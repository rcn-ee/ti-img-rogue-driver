/* -*- mode: c; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* vi: set ts=8 sw=8 sts=8: */
/*************************************************************************/ /*!
@File
@Codingstyle    LinuxKernel
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@License        Dual MIT/GPLv2

The contents of this file are subject to the MIT license as set out below.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

Alternatively, the contents of this file may be used under the terms of
the GNU General Public License Version 2 ("GPL") in which case the provisions
of GPL are applicable instead of those above.

If you wish to allow use of your version of this file only under the terms of
GPL, and not to allow others to use your version of this file under the terms
of the MIT license, indicate your decision by deleting the provisions above
and replace them with the notice and other provisions required by GPL as set
out in the file called "GPL-COPYING" included in this distribution. If you do
not delete the provisions above, a recipient may use your version of this file
under the terms of either the MIT license or GPL.

This License is also included in this distribution in the file called
"MIT-COPYING".

EXCEPT AS OTHERWISE STATED IN A NEGOTIATED AGREEMENT: (A) THE SOFTWARE IS
PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
PURPOSE AND NONINFRINGEMENT; AND (B) IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/ /**************************************************************************/

#include <linux/atomic.h>
#include <linux/module.h>
#include <linux/pagemap.h>
#include <linux/jiffies.h>
#include "pvr_linux_fence.h"
#include <linux/reservation.h>
#include <linux/workqueue.h>
#include <linux/dma-mapping.h>
#include <linux/version.h>
#include <linux/mutex.h>
#include <linux/capability.h>
#include <linux/completion.h>
#include <linux/dma-buf.h>

#include <drm/drmP.h>
#include <drm/drm_crtc.h>
#include <drm/drm_crtc_helper.h>
#include <drm/drm_edid.h>
#include <drm/drm_fb_helper.h>
#include <drm/drm_modes.h>
#include <drm/drm_plane_helper.h>

#include "img_drm_fourcc_internal.h"
#include <pvr_drm_display_external.h>
#include <pvrversion.h>

#include <drm/drm_fourcc.h>

#include "drm_nulldisp_drv.h"
#if defined(LMA)
#include "tc_drv.h"
#include "drm_pdp_gem.h"
#include "pdp_drm.h"
#else
#include "drm_nulldisp_gem.h"
#endif
#include "nulldisp_drm.h"
#include "drm_netlink_gem.h"
#include "drm_nulldisp_netlink.h"

#include "kernel_compatibility.h"

#define DRIVER_NAME "nulldisp"
#define DRIVER_DESC "Imagination Technologies Null DRM Display Driver"
#define DRIVER_DATE "20150612"

#define NULLDISP_FB_WIDTH_MIN 0
#define NULLDISP_FB_WIDTH_MAX 4096
#define NULLDISP_FB_HEIGHT_MIN 0
#define NULLDISP_FB_HEIGHT_MAX 2160

#define NULLDISP_DEFAULT_WIDTH 640
#define NULLDISP_DEFAULT_HEIGHT 480
#define NULLDISP_DEFAULT_REFRESH_RATE 60

#define NULLDISP_MAX_PLANES 3

enum nulldisp_crtc_flip_status {
	NULLDISP_CRTC_FLIP_STATUS_NONE = 0,
	NULLDISP_CRTC_FLIP_STATUS_PENDING,
	NULLDISP_CRTC_FLIP_STATUS_DONE,
};

struct nulldisp_flip_data {
	struct dma_fence_cb base;
	struct drm_crtc *crtc;
	struct dma_fence *wait_fence;
};

struct nulldisp_crtc {
	struct drm_crtc base;
	struct delayed_work vb_work;
	struct work_struct flip_work;
	struct delayed_work flip_to_work;
	struct delayed_work copy_to_work;

	struct completion flip_scheduled;
	struct completion copy_done;

	/* Reuse the drm_device event_lock to protect these */
	atomic_t flip_status;
	struct drm_pending_vblank_event *flip_event;
	struct drm_framebuffer *old_fb;
	struct nulldisp_flip_data *flip_data;
	bool flip_async;
};

struct nulldisp_display_device {
	struct drm_device *dev;

	struct workqueue_struct *workqueue;
	struct nulldisp_crtc *nulldisp_crtc;
	struct nlpvrdpy *nlpvrdpy;
#if defined(LMA)
	struct pdp_gem_private *pdp_gem_priv;
#endif
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 8, 0))
	struct drm_connector *connector;
#endif
};

struct nulldisp_framebuffer {
	struct drm_framebuffer base;
	struct drm_gem_object *obj[NULLDISP_MAX_PLANES];
};

struct nulldisp_module_params {
	uint32_t hdisplay;
	uint32_t vdisplay;
	uint32_t vrefresh;
};

#define to_nulldisp_crtc(crtc) \
	container_of(crtc, struct nulldisp_crtc, base)
#define to_nulldisp_framebuffer(framebuffer) \
	container_of(framebuffer, struct nulldisp_framebuffer, base)

#if defined(LMA)
#define	obj_to_resv(obj) pdp_gem_get_resv(obj)
#else
#define	obj_to_resv(obj) nulldisp_gem_get_resv(obj)
#endif

static const uint32_t nulldisp_modeset_formats[] = {
	DRM_FORMAT_NV12,
	DRM_FORMAT_NV21,
	DRM_FORMAT_YUYV,
	DRM_FORMAT_YUV444,
	DRM_FORMAT_XRGB8888,
	DRM_FORMAT_ARGB8888,
	DRM_FORMAT_RGB565,
	DRM_FORMAT_ABGR2101010,
#ifdef DRM_FORMAT_ABGR16_IMG
	DRM_FORMAT_ABGR16_IMG,
#endif
};

/*
 * Note that nulldisp, being a no-hardware display controller driver,
 * "supports" a number different decompression hardware
 * versions (V0, V1, V2 ...). Real, hardware display controllers are
 * likely to support only a single version.
 */
static const uint64_t nulldisp_primary_plane_modifiers[] = {
	DRM_FORMAT_MOD_LINEAR,
	DRM_FORMAT_MOD_PVR_FBCDC_8x8_V0,
	DRM_FORMAT_MOD_PVR_FBCDC_8x8_V0_FIX,
	DRM_FORMAT_MOD_PVR_FBCDC_8x8_V1,
	DRM_FORMAT_MOD_PVR_FBCDC_8x8_V2,
	DRM_FORMAT_MOD_PVR_FBCDC_8x8_V3,
	DRM_FORMAT_MOD_PVR_FBCDC_8x8_V7,
	DRM_FORMAT_MOD_PVR_FBCDC_16x4_V0,
	DRM_FORMAT_MOD_PVR_FBCDC_16x4_V0_FIX,
	DRM_FORMAT_MOD_PVR_FBCDC_16x4_V1,
	DRM_FORMAT_MOD_PVR_FBCDC_16x4_V2,
	DRM_FORMAT_MOD_PVR_FBCDC_16x4_V3,
	DRM_FORMAT_MOD_PVR_FBCDC_16x4_V7,
	DRM_FORMAT_MOD_PVR_FBCDC_32x2_V1,
	DRM_FORMAT_MOD_PVR_FBCDC_32x2_V3,
	DRM_FORMAT_MOD_INVALID
};

static struct nulldisp_module_params module_params = {
	.hdisplay = NULLDISP_DEFAULT_WIDTH,
	.vdisplay = NULLDISP_DEFAULT_HEIGHT,
	.vrefresh = NULLDISP_DEFAULT_REFRESH_RATE,
};
module_param_named(width, module_params.hdisplay, uint, 0444);
module_param_named(height, module_params.vdisplay, uint, 0444);
module_param_named(refreshrate, module_params.vrefresh, uint, 0444);
MODULE_PARM_DESC(width, "Preferred display width in pixels");
MODULE_PARM_DESC(height, "Preferred display height in pixels");
MODULE_PARM_DESC(refreshrate, "Preferred display refresh rate");

/*
 * Please use this function to obtain the module parameters instead of
 * accessing the global "module_params" structure directly.
 */
static inline const struct nulldisp_module_params *
nulldisp_get_module_params(void)
{
	return &module_params;
}

/******************************************************************************
 * Linux compatibility functions
 ******************************************************************************/
static inline void
nulldisp_drm_fb_set_format(struct drm_framebuffer *fb,
                           u32 pixel_format)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0))
	fb->format = drm_format_info(pixel_format);
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 10, 0))
	const struct drm_format_info *format = drm_format_info(pixel_format);

	fb->pixel_format = pixel_format;

	fb->depth = format->depth;
	fb->bits_per_pixel = format->depth ? (format->cpp[0] * 8) : 0;
#else
	fb->pixel_format = pixel_format;

	switch (pixel_format) {
	case DRM_FORMAT_NV12:
	case DRM_FORMAT_YUYV:
		/* Unused for YUV formats */
		fb->depth = 0;
		fb->bits_per_pixel = 0;
		break;

	default: /* RGB */
		drm_fb_get_bpp_depth(pixel_format,
				     &fb->depth,
				     &fb->bits_per_pixel);
	}
#endif
}

static inline void nulldisp_drm_fb_set_modifier(struct drm_framebuffer *fb,
						uint64_t value)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 10, 0))
	fb->modifier = value;
#elif(LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
	/* FB modifier values must be the same for all planes */
	fb->modifier[0] = value;
	fb->modifier[1] = value;
	fb->modifier[2] = value;
	fb->modifier[3] = value;
#else
	/* Modifiers are not supported */
#endif
}

/******************************************************************************
 * Plane functions
 ******************************************************************************/

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
static bool nulldisp_primary_format_mod_supported(struct drm_plane *plane,
						  uint32_t format,
						  uint64_t modifier)
{
	/*
	 * All 'nulldisp_modeset_formats' are supported for every modifier in the
	 * 'nulldisp_primary_plane_modifiers' array.
	 */
	return true;
}
#endif

static const struct drm_plane_funcs nulldisp_primary_plane_funcs = {
	.update_plane = drm_primary_helper_update,
	.disable_plane = drm_primary_helper_disable,
	.destroy = drm_primary_helper_destroy,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
	.format_mod_supported = nulldisp_primary_format_mod_supported,
#endif
};

/******************************************************************************
 * CRTC functions
 ******************************************************************************/

static void nulldisp_crtc_helper_dpms(struct drm_crtc *crtc,
				      int mode)
{
	/*
	 * Change the power state of the display/pipe/port/etc. If the mode
	 * passed in is unsupported, the provider must use the next lowest
	 * power level.
	 */
}

static void nulldisp_crtc_helper_prepare(struct drm_crtc *crtc)
{
	drm_crtc_vblank_off(crtc);

	/*
	 * Prepare the display/pipe/port/etc for a mode change e.g. put them
	 * in a low power state/turn them off
	 */
}

static void nulldisp_crtc_helper_commit(struct drm_crtc *crtc)
{
	/* Turn the display/pipe/port/etc back on */

	drm_crtc_vblank_on(crtc);
}

static bool
nulldisp_crtc_helper_mode_fixup(struct drm_crtc *crtc,
				const struct drm_display_mode *mode,
				struct drm_display_mode *adjusted_mode)
{
	/*
	 * Fix up mode so that it's compatible with the hardware. The results
	 * should be stored in adjusted_mode (i.e. mode should be untouched).
	 */
	return true;
}

static int
nulldisp_crtc_helper_mode_set_base_atomic(struct drm_crtc *crtc,
					  struct drm_framebuffer *fb,
					  int x, int y,
					  enum mode_set_atomic atomic)
{
	/* Set the display base address or offset from the base address */
	return 0;
}

static int nulldisp_crtc_helper_mode_set_base(struct drm_crtc *crtc,
					      int x, int y,
					      struct drm_framebuffer *old_fb)
{
	return nulldisp_crtc_helper_mode_set_base_atomic(crtc,
							 crtc->primary->fb,
							 x,
							 y,
							 0);
}

static int
nulldisp_crtc_helper_mode_set(struct drm_crtc *crtc,
			      struct drm_display_mode *mode,
			      struct drm_display_mode *adjusted_mode,
			      int x, int y,
			      struct drm_framebuffer *old_fb)
{
	/* Setup the the new mode and/or framebuffer */
	return nulldisp_crtc_helper_mode_set_base(crtc, x, y, old_fb);
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
static void nulldisp_crtc_helper_load_lut(struct drm_crtc *crtc)
{
}
#endif

static void nulldisp_crtc_destroy(struct drm_crtc *crtc)
{
	struct nulldisp_crtc *nulldisp_crtc = to_nulldisp_crtc(crtc);

	DRM_DEBUG_DRIVER("[CRTC:%d]\n", crtc->base.id);

	drm_crtc_cleanup(crtc);

	BUG_ON(atomic_read(&nulldisp_crtc->flip_status) !=
	       NULLDISP_CRTC_FLIP_STATUS_NONE);

	kfree(nulldisp_crtc);
}

static void nulldisp_crtc_flip_complete(struct drm_crtc *crtc)
{
	struct nulldisp_crtc *nulldisp_crtc = to_nulldisp_crtc(crtc);
	unsigned long flags;

	spin_lock_irqsave(&crtc->dev->event_lock, flags);

	/* The flipping process has been completed so reset the flip status */
	atomic_set(&nulldisp_crtc->flip_status, NULLDISP_CRTC_FLIP_STATUS_NONE);

	dma_fence_put(nulldisp_crtc->flip_data->wait_fence);
	kfree(nulldisp_crtc->flip_data);
	nulldisp_crtc->flip_data = NULL;

	if (nulldisp_crtc->flip_event) {
		drm_crtc_send_vblank_event(crtc, nulldisp_crtc->flip_event);
		nulldisp_crtc->flip_event = NULL;
	}

	spin_unlock_irqrestore(&crtc->dev->event_lock, flags);
}

static void nulldisp_crtc_flip_done(struct nulldisp_crtc *nulldisp_crtc)
{
	struct drm_crtc *crtc = &nulldisp_crtc->base;

	struct drm_framebuffer *old_fb;

	WARN_ON(atomic_read(&nulldisp_crtc->flip_status) !=
		NULLDISP_CRTC_FLIP_STATUS_PENDING);

	old_fb = nulldisp_crtc->old_fb;
	nulldisp_crtc->old_fb = NULL;

	(void) nulldisp_crtc_helper_mode_set_base(crtc, crtc->x, crtc->y,
						  old_fb);

	atomic_set(&nulldisp_crtc->flip_status, NULLDISP_CRTC_FLIP_STATUS_DONE);

	if (nulldisp_crtc->flip_async)
		nulldisp_crtc_flip_complete(crtc);
}

static inline unsigned long nulldisp_netlink_timeout(void)
{
	return msecs_to_jiffies(30000);
}

static bool nulldisp_set_flip_to(struct nulldisp_crtc *nulldisp_crtc)
{
	struct drm_crtc *crtc = &nulldisp_crtc->base;
	struct nulldisp_display_device *nulldisp_dev = crtc->dev->dev_private;

	/* Returns false if work already queued, else true */
	return queue_delayed_work(nulldisp_dev->workqueue,
				  &nulldisp_crtc->flip_to_work,
				  nulldisp_netlink_timeout());
}

static bool nulldisp_set_copy_to(struct nulldisp_crtc *nulldisp_crtc)
{
	struct drm_crtc *crtc = &nulldisp_crtc->base;
	struct nulldisp_display_device *nulldisp_dev = crtc->dev->dev_private;

	/* Returns false if work already queued, else true */
	return queue_delayed_work(nulldisp_dev->workqueue,
				  &nulldisp_crtc->copy_to_work,
				  nulldisp_netlink_timeout());
}

static void nulldisp_flip_to_work(struct work_struct *w)
{
	struct delayed_work *dw =
		container_of(w, struct delayed_work, work);
	struct nulldisp_crtc *nulldisp_crtc =
		container_of(dw, struct nulldisp_crtc, flip_to_work);

	if (atomic_read(&nulldisp_crtc->flip_status) ==
	    NULLDISP_CRTC_FLIP_STATUS_PENDING)
		nulldisp_crtc_flip_done(nulldisp_crtc);
}

static void nulldisp_copy_to_work(struct work_struct *w)
{
	struct delayed_work *dw =
		container_of(w, struct delayed_work, work);
	struct nulldisp_crtc *nulldisp_crtc =
		container_of(dw, struct nulldisp_crtc, copy_to_work);

	complete(&nulldisp_crtc->copy_done);
}

static void nulldisp_flip_work(struct work_struct *w)
{
	struct nulldisp_crtc *nulldisp_crtc =
		container_of(w, struct nulldisp_crtc, flip_work);
	struct drm_crtc *crtc = &nulldisp_crtc->base;
	struct drm_device *dev = crtc->dev;
	struct nulldisp_display_device *nulldisp_dev = dev->dev_private;
	struct nulldisp_framebuffer *nulldisp_fb =
		to_nulldisp_framebuffer(crtc->primary->fb);
	u64 addr[NULLDISP_MAX_PLANES],
	    size[NULLDISP_MAX_PLANES];
	int i;

	/*
	 * To prevent races with disconnect requests from user space,
	 * set the timeout before sending the flip request.
	 */
	for (i = 0; i < nulldisp_drm_fb_num_planes(crtc->primary->fb); i++) {
		struct drm_gem_object *obj = nulldisp_fb->obj[i];

		if (drm_gem_create_mmap_offset(obj)) {
			DRM_ERROR("Failed to get mmap offset for buffer[%d] = %p\n", i, obj);
			goto fail_cancel;
		}

		addr[i] = drm_vma_node_offset_addr(&obj->vma_node);
		size[i] = obj->size;
	}

	nulldisp_set_flip_to(nulldisp_crtc);

	if (nlpvrdpy_send_flip(nulldisp_dev->nlpvrdpy,
			       &nulldisp_fb->base,
			       &addr[0],
			       &size[0]))
		goto fail_cancel;

	return;

fail_cancel:
	/*
	 * We can't flush the work, as we are running on the same
	 * single threaded workqueue as the work to be flushed.
	 */
	cancel_delayed_work(&nulldisp_crtc->flip_to_work);

	nulldisp_crtc_flip_done(nulldisp_crtc);
}

static void nulldisp_crtc_flip_cb(struct dma_fence *fence,
				  struct dma_fence_cb *cb)
{
	struct nulldisp_flip_data *flip_data =
		container_of(cb, struct nulldisp_flip_data, base);
	struct drm_crtc *crtc = flip_data->crtc;
	struct nulldisp_crtc *nulldisp_crtc = to_nulldisp_crtc(crtc);
	struct drm_device *dev = crtc->dev;
	struct nulldisp_display_device *nulldisp_dev = dev->dev_private;

	(void) queue_work(nulldisp_dev->workqueue,
			  &nulldisp_crtc->flip_work);

	complete_all(&nulldisp_crtc->flip_scheduled);
}

static void nulldisp_crtc_flip_schedule_cb(struct dma_fence *fence,
					   struct dma_fence_cb *cb)
{
	struct nulldisp_flip_data *flip_data =
		container_of(cb, struct nulldisp_flip_data, base);
	int err = 0;

	if (flip_data->wait_fence)
		err = dma_fence_add_callback(flip_data->wait_fence,
					     &flip_data->base,
					     nulldisp_crtc_flip_cb);

	if (!flip_data->wait_fence || err) {
		if (err && err != -ENOENT)
			DRM_ERROR("flip failed to wait on old buffer\n");
		nulldisp_crtc_flip_cb(flip_data->wait_fence, &flip_data->base);
	}
}

static int nulldisp_crtc_flip_schedule(struct drm_crtc *crtc,
				       struct drm_gem_object *obj,
				       struct drm_gem_object *old_obj)
{
	struct nulldisp_crtc *nulldisp_crtc = to_nulldisp_crtc(crtc);
	struct reservation_object *resv = obj_to_resv(obj);
	struct reservation_object *old_resv = obj_to_resv(old_obj);
	struct nulldisp_flip_data *flip_data;
	struct dma_fence *fence;
	int err;

	flip_data = kmalloc(sizeof(*flip_data), GFP_KERNEL);
	if (!flip_data)
		return -ENOMEM;

	flip_data->crtc = crtc;

	ww_mutex_lock(&old_resv->lock, NULL);
	flip_data->wait_fence =
		dma_fence_get(reservation_object_get_excl(old_resv));

	if (old_resv != resv) {
		ww_mutex_unlock(&old_resv->lock);
		ww_mutex_lock(&resv->lock, NULL);
	}

	fence = dma_fence_get(reservation_object_get_excl(resv));
	ww_mutex_unlock(&resv->lock);

	nulldisp_crtc->flip_data = flip_data;
	reinit_completion(&nulldisp_crtc->flip_scheduled);
	atomic_set(&nulldisp_crtc->flip_status,
		   NULLDISP_CRTC_FLIP_STATUS_PENDING);

	if (fence) {
		err = dma_fence_add_callback(fence, &flip_data->base,
					     nulldisp_crtc_flip_schedule_cb);
		dma_fence_put(fence);
		if (err && err != -ENOENT)
			goto err_set_flip_status_none;
	}

	if (!fence || err == -ENOENT) {
		nulldisp_crtc_flip_schedule_cb(fence, &flip_data->base);
		err = 0;
	}

	return err;

err_set_flip_status_none:
	atomic_set(&nulldisp_crtc->flip_status, NULLDISP_CRTC_FLIP_STATUS_NONE);
	dma_fence_put(flip_data->wait_fence);
	kfree(flip_data);
	return err;
}

static int nulldisp_crtc_page_flip(struct drm_crtc *crtc,
				   struct drm_framebuffer *fb,
				   struct drm_pending_vblank_event *event,
				   uint32_t page_flip_flags
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 12, 0))
				   , struct drm_modeset_acquire_ctx *ctx
#endif
				   )
{
	struct nulldisp_crtc *nulldisp_crtc = to_nulldisp_crtc(crtc);
	struct nulldisp_framebuffer *nulldisp_fb = to_nulldisp_framebuffer(fb);
	struct nulldisp_framebuffer *nulldisp_old_fb =
		to_nulldisp_framebuffer(crtc->primary->fb);
	enum nulldisp_crtc_flip_status status;
	unsigned long flags;
	int err;

	spin_lock_irqsave(&crtc->dev->event_lock, flags);
	status = atomic_read(&nulldisp_crtc->flip_status);
	spin_unlock_irqrestore(&crtc->dev->event_lock, flags);

	if (status != NULLDISP_CRTC_FLIP_STATUS_NONE)
		return -EBUSY;

	if (!(page_flip_flags & DRM_MODE_PAGE_FLIP_ASYNC)) {
		err = drm_crtc_vblank_get(crtc);
		if (err)
			return err;
	}

	nulldisp_crtc->old_fb = crtc->primary->fb;
	nulldisp_crtc->flip_event = event;
	nulldisp_crtc->flip_async = !!(page_flip_flags &
				       DRM_MODE_PAGE_FLIP_ASYNC);

	/* Set the crtc to point to the new framebuffer */
	crtc->primary->fb = fb;

	err = nulldisp_crtc_flip_schedule(crtc, nulldisp_fb->obj[0],
					  nulldisp_old_fb->obj[0]);
	if (err) {
		crtc->primary->fb = nulldisp_crtc->old_fb;
		nulldisp_crtc->old_fb = NULL;
		nulldisp_crtc->flip_event = NULL;
		nulldisp_crtc->flip_async = false;

		DRM_ERROR("failed to schedule flip (err=%d)\n", err);
		goto err_vblank_put;
	}

	return 0;

err_vblank_put:
	if (!(page_flip_flags & DRM_MODE_PAGE_FLIP_ASYNC))
		drm_crtc_vblank_put(crtc);
	return err;
}

static void nulldisp_crtc_helper_disable(struct drm_crtc *crtc)
{
	struct nulldisp_crtc *nulldisp_crtc = to_nulldisp_crtc(crtc);

	if (atomic_read(&nulldisp_crtc->flip_status) ==
	    NULLDISP_CRTC_FLIP_STATUS_PENDING)
		wait_for_completion(&nulldisp_crtc->flip_scheduled);

	/*
	 * Flush any outstanding page flip related work. The order this
	 * is done is important, to ensure there are no outstanding
	 * page flips.
	 */
	flush_work(&nulldisp_crtc->flip_work);
	flush_delayed_work(&nulldisp_crtc->flip_to_work);
	flush_delayed_work(&nulldisp_crtc->vb_work);

	drm_crtc_vblank_off(crtc);
	flush_delayed_work(&nulldisp_crtc->vb_work);

	/*
	 * Vblank has been disabled, so the vblank handler shouldn't be
	 * able to reschedule itself.
	 */
	BUG_ON(cancel_delayed_work(&nulldisp_crtc->vb_work));

	BUG_ON(atomic_read(&nulldisp_crtc->flip_status) !=
	       NULLDISP_CRTC_FLIP_STATUS_NONE);

	/* Flush any remaining dirty FB work */
	flush_delayed_work(&nulldisp_crtc->copy_to_work);
}

static const struct drm_crtc_helper_funcs nulldisp_crtc_helper_funcs = {
	.dpms = nulldisp_crtc_helper_dpms,
	.prepare = nulldisp_crtc_helper_prepare,
	.commit = nulldisp_crtc_helper_commit,
	.mode_fixup = nulldisp_crtc_helper_mode_fixup,
	.mode_set = nulldisp_crtc_helper_mode_set,
	.mode_set_base = nulldisp_crtc_helper_mode_set_base,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
	.load_lut = nulldisp_crtc_helper_load_lut,
#endif
	.mode_set_base_atomic = nulldisp_crtc_helper_mode_set_base_atomic,
	.disable = nulldisp_crtc_helper_disable,
};

static const struct drm_crtc_funcs nulldisp_crtc_funcs = {
	.reset = NULL,
	.cursor_set = NULL,
	.cursor_move = NULL,
	.gamma_set = NULL,
	.destroy = nulldisp_crtc_destroy,
	.set_config = drm_crtc_helper_set_config,
	.page_flip = nulldisp_crtc_page_flip,
};

static bool nulldisp_queue_vblank_work(struct nulldisp_crtc *nulldisp_crtc)
{
	struct drm_crtc *crtc = &nulldisp_crtc->base;
	struct nulldisp_display_device *nulldisp_dev = crtc->dev->dev_private;
	int vrefresh;
	const int vrefresh_default = 60;

	if (crtc->hwmode.vrefresh) {
		vrefresh = crtc->hwmode.vrefresh;
	} else {
		vrefresh = vrefresh_default;
		DRM_ERROR("vertical refresh rate is zero, defaulting to %d\n",
			  vrefresh);
	}

	/* Returns false if work already queued, else true */
	return queue_delayed_work(nulldisp_dev->workqueue,
				  &nulldisp_crtc->vb_work,
				  usecs_to_jiffies(1000000/vrefresh));
}

static void nulldisp_handle_vblank(struct work_struct *w)
{
	struct delayed_work *dw =
		container_of(w, struct delayed_work, work);
	struct nulldisp_crtc *nulldisp_crtc =
		container_of(dw, struct nulldisp_crtc, vb_work);
	struct drm_crtc *crtc = &nulldisp_crtc->base;
	struct drm_device *dev = crtc->dev;
	enum nulldisp_crtc_flip_status status;

	/*
	 * Reschedule the handler, if necessary. This is done before
	 * calling drm_crtc_vblank_put, so that the work can be cancelled
	 * if vblank events are disabled.
	 */
	if (drm_handle_vblank(dev, 0))
		(void) nulldisp_queue_vblank_work(nulldisp_crtc);

	status = atomic_read(&nulldisp_crtc->flip_status);
	if (status == NULLDISP_CRTC_FLIP_STATUS_DONE) {
		if (!nulldisp_crtc->flip_async)
			nulldisp_crtc_flip_complete(crtc);
		drm_crtc_vblank_put(crtc);
	}

}

static struct nulldisp_crtc *
nulldisp_crtc_create(struct nulldisp_display_device *nulldisp_dev)
{
	struct nulldisp_crtc *nulldisp_crtc;
	struct drm_crtc *crtc;
	struct drm_plane *primary;

	nulldisp_crtc = kzalloc(sizeof(*nulldisp_crtc), GFP_KERNEL);
	if (!nulldisp_crtc)
		goto err_return;

	primary = kzalloc(sizeof(*primary), GFP_KERNEL);
	if (!primary)
		goto err_free_crtc;

	crtc = &nulldisp_crtc->base;

	atomic_set(&nulldisp_crtc->flip_status, NULLDISP_CRTC_FLIP_STATUS_NONE);
	init_completion(&nulldisp_crtc->flip_scheduled);
	init_completion(&nulldisp_crtc->copy_done);

	if (drm_universal_plane_init(nulldisp_dev->dev, primary, 0,
				     &nulldisp_primary_plane_funcs,
				     nulldisp_modeset_formats,
				     ARRAY_SIZE(nulldisp_modeset_formats),
				     nulldisp_primary_plane_modifiers,
				     DRM_PLANE_TYPE_PRIMARY, NULL)) {
		goto err_free_primary;
	}

	if (drm_crtc_init_with_planes(nulldisp_dev->dev, crtc, primary,
				      NULL, &nulldisp_crtc_funcs, NULL)) {
		goto err_cleanup_plane;
	}

	drm_crtc_helper_add(crtc, &nulldisp_crtc_helper_funcs);

	INIT_DELAYED_WORK(&nulldisp_crtc->vb_work, nulldisp_handle_vblank);
	INIT_WORK(&nulldisp_crtc->flip_work, nulldisp_flip_work);
	INIT_DELAYED_WORK(&nulldisp_crtc->flip_to_work, nulldisp_flip_to_work);
	INIT_DELAYED_WORK(&nulldisp_crtc->copy_to_work, nulldisp_copy_to_work);

	DRM_DEBUG_DRIVER("[CRTC:%d]\n", crtc->base.id);

	return nulldisp_crtc;

err_cleanup_plane:
	drm_plane_cleanup(primary);
err_free_primary:
	kfree(primary);
err_free_crtc:
	kfree(nulldisp_crtc);
err_return:
	return NULL;
}


/******************************************************************************
 * Connector functions
 ******************************************************************************/

static int
nulldisp_validate_module_parameters(void)
{
	const struct nulldisp_module_params *module_params =
		nulldisp_get_module_params();

	if (!module_params->hdisplay ||
	    !module_params->vdisplay ||
	    !module_params->vrefresh ||
	    (module_params->hdisplay > NULLDISP_FB_WIDTH_MAX) ||
	    (module_params->vdisplay > NULLDISP_FB_HEIGHT_MAX))
		return -EINVAL;

	return 0;
}

static bool
nulldisp_set_preferred_mode(struct drm_connector *connector,
			    uint32_t hdisplay,
			    uint32_t vdisplay,
			    uint32_t vrefresh)
{
	struct drm_display_mode *mode;

	/*
	 * Mark the first mode, matching the hdisplay, vdisplay and
	 * vrefresh, preferred.
	 */
	list_for_each_entry(mode, &connector->probed_modes, head)
		if (mode->hdisplay == hdisplay &&
		    mode->vdisplay == vdisplay &&
		    drm_mode_vrefresh(mode) == vrefresh) {
			mode->type |= DRM_MODE_TYPE_PREFERRED;
			return true;
		}

	return false;
}

static bool
nulldisp_connector_add_preferred_mode(struct drm_connector *connector,
				      uint32_t hdisplay,
				      uint32_t vdisplay,
				      uint32_t vrefresh)
{
	/*
	 * The DRM core validates the mode information, so to create
	 * a mode with an arbitrary width, height and vrefresh values,
	 * we duplicate a valid mode and then override the relevant
	 * values.
	 *
	 * The mode parameters are based on the 'drm_dmt_modes[]'
	 * mode 4096x2160@60Hz from 'drm_edid.c'. This is the highest
	 * standard resolution mode in the 4.14 kernel. These mode parameters
	 * are also valid for any lower resolution mode.
	 */
	struct drm_display_mode base_mode = {
		DRM_MODE("", DRM_MODE_TYPE_PREFERRED, 556744, hdisplay,
			 4104, 4136, 4176, 0, vdisplay, 2208, 2216, 2222, 0, 0)
	};
	struct drm_display_mode *preferred_mode;
	struct drm_device *dev = connector->dev;

	/* Override the base mode parameters */
	snprintf(&base_mode.name[0], ARRAY_SIZE(base_mode.name), "%dx%d",
		 hdisplay, vdisplay);
	base_mode.vrefresh = vrefresh;

	preferred_mode = drm_mode_duplicate(dev, &base_mode);
	if (!preferred_mode) {
		DRM_DEBUG_DRIVER("[CONNECTOR:%s]:create mode %dx%d@%d failed\n",
				 connector->name,
				 hdisplay,
				 vdisplay,
				 vrefresh);

		return false;
	}

	drm_mode_probed_add(connector, preferred_mode);

	return true;
}

/*
 * Gather modes. Here we can get the EDID data from the monitor and
 * turn it into drm_display_mode structures.
 */
static int
nulldisp_connector_helper_get_modes(struct drm_connector *connector)
{
	int modes_count;
	struct drm_device *dev = connector->dev;
	const struct nulldisp_module_params *module_params =
		nulldisp_get_module_params();
	uint32_t hdisplay = module_params->hdisplay;
	uint32_t vdisplay = module_params->vdisplay;
	uint32_t vrefresh = module_params->vrefresh;

	/* Add common modes */
	modes_count = drm_add_modes_noedid(connector,
					   dev->mode_config.max_width,
					   dev->mode_config.max_height);

	/*
	 * Check if any of the connector modes match the preferred mode
	 * criteria specified by the module parameters. If the mode is
	 * found - flag it as preferred. Otherwise create the preferred
	 * mode based on the module parameters criteria, and flag it as
	 * preferred.
	 */
	if (!nulldisp_set_preferred_mode(connector,
					 hdisplay,
					 vdisplay,
					 vrefresh))
		if (nulldisp_connector_add_preferred_mode(connector,
							  hdisplay,
							  vdisplay,
							  vrefresh))
			modes_count++;

	/* Sort the connector modes by relevance */
	drm_mode_sort(&connector->probed_modes);
	
	return modes_count;
}

static int
nulldisp_connector_helper_mode_valid(struct drm_connector *connector,
				     struct drm_display_mode *mode)
{
	/*
	 * This function is called on each gathered mode (e.g. via EDID)
	 * and gives the driver a chance to reject it if the hardware
	 * cannot support it.
	 */
	return MODE_OK;
}

static struct drm_encoder *
nulldisp_connector_helper_best_encoder(struct drm_connector *connector)
{
	/* Pick the first encoder we find */
	if (connector->encoder_ids[0] != 0) {
		struct drm_encoder *encoder;

		encoder = drm_encoder_find(connector->dev,
					   NULL,
					   connector->encoder_ids[0]);
		if (encoder) {
			DRM_DEBUG_DRIVER("[ENCODER:%d:%s] best for "
					 "[CONNECTOR:%d:%s]\n",
					 encoder->base.id,
					 encoder->name,
					 connector->base.id,
					 connector->name);
			return encoder;
		}
	}

	return NULL;
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 10, 0))
static enum drm_connector_status
nulldisp_connector_detect(struct drm_connector *connector,
			  bool force)
{
	/* Return whether or not a monitor is attached to the connector */
	return connector_status_connected;
}
#endif

static void nulldisp_connector_destroy(struct drm_connector *connector)
{
	DRM_DEBUG_DRIVER("[CONNECTOR:%d:%s]\n",
			 connector->base.id,
			 connector->name);

	drm_mode_connector_update_edid_property(connector, NULL);
	drm_connector_cleanup(connector);

	kfree(connector);
}

static void nulldisp_connector_force(struct drm_connector *connector)
{
}

static const struct drm_connector_helper_funcs
nulldisp_connector_helper_funcs = {
	.get_modes = nulldisp_connector_helper_get_modes,
	.mode_valid = nulldisp_connector_helper_mode_valid,
	.best_encoder = nulldisp_connector_helper_best_encoder,
};

static const struct drm_connector_funcs nulldisp_connector_funcs = {
	.dpms = drm_helper_connector_dpms,
	.reset = NULL,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 10, 0))
	.detect = nulldisp_connector_detect,
#endif
	.fill_modes = drm_helper_probe_single_connector_modes,
	.destroy = nulldisp_connector_destroy,
	.force = nulldisp_connector_force,
};

static struct drm_connector *
nulldisp_connector_create(struct nulldisp_display_device *nulldisp_dev,
			  int type)
{
	struct drm_connector *connector;

	connector = kzalloc(sizeof(*connector), GFP_KERNEL);
	if (!connector)
		return NULL;

	drm_connector_init(nulldisp_dev->dev,
			   connector,
			   &nulldisp_connector_funcs,
			   type);
	drm_connector_helper_add(connector, &nulldisp_connector_helper_funcs);

	connector->dpms = DRM_MODE_DPMS_OFF;
	connector->interlace_allowed = false;
	connector->doublescan_allowed = false;
	connector->display_info.subpixel_order = SubPixelUnknown;

	DRM_DEBUG_DRIVER("[CONNECTOR:%d:%s]\n",
			 connector->base.id,
			 connector->name);

	return connector;
}


/******************************************************************************
 * Encoder functions
 ******************************************************************************/

static void nulldisp_encoder_helper_dpms(struct drm_encoder *encoder,
					 int mode)
{
	/*
	 * Set the display power state or active encoder based on the mode. If
	 * the mode passed in is unsupported, the provider must use the next
	 * lowest power level.
	 */
}

static bool
nulldisp_encoder_helper_mode_fixup(struct drm_encoder *encoder,
				   const struct drm_display_mode *mode,
				   struct drm_display_mode *adjusted_mode)
{
	/*
	 * Fix up mode so that it's compatible with the hardware. The results
	 * should be stored in adjusted_mode (i.e. mode should be untouched).
	 */
	return true;
}

static void nulldisp_encoder_helper_prepare(struct drm_encoder *encoder)
{
	/*
	 * Prepare the encoder for a mode change e.g. set the active encoder
	 * accordingly/turn the encoder off
	 */
}

static void nulldisp_encoder_helper_commit(struct drm_encoder *encoder)
{
	/* Turn the encoder back on/set the active encoder */
}

static void
nulldisp_encoder_helper_mode_set(struct drm_encoder *encoder,
				 struct drm_display_mode *mode,
				 struct drm_display_mode *adjusted_mode)
{
	/* Setup the encoder for the new mode */
}

static void nulldisp_encoder_destroy(struct drm_encoder *encoder)
{
	DRM_DEBUG_DRIVER("[ENCODER:%d:%s]\n", encoder->base.id, encoder->name);

	drm_encoder_cleanup(encoder);
	kfree(encoder);
}

static const struct drm_encoder_helper_funcs nulldisp_encoder_helper_funcs = {
	.dpms = nulldisp_encoder_helper_dpms,
	.mode_fixup = nulldisp_encoder_helper_mode_fixup,
	.prepare = nulldisp_encoder_helper_prepare,
	.commit = nulldisp_encoder_helper_commit,
	.mode_set = nulldisp_encoder_helper_mode_set,
	.get_crtc = NULL,
	.detect = NULL,
	.disable = NULL,
};

static const struct drm_encoder_funcs nulldisp_encoder_funcs = {
	.reset = NULL,
	.destroy = nulldisp_encoder_destroy,
};

static struct drm_encoder *
nulldisp_encoder_create(struct nulldisp_display_device *nulldisp_dev,
			int type)
{
	struct drm_encoder *encoder;
	int err;

	encoder = kzalloc(sizeof(*encoder), GFP_KERNEL);
	if (!encoder)
		return ERR_PTR(-ENOMEM);

	err = drm_encoder_init(nulldisp_dev->dev,
			       encoder,
			       &nulldisp_encoder_funcs,
			       type,
			       NULL);
	if (err) {
		DRM_ERROR("Failed to initialise encoder\n");
		return ERR_PTR(err);
	}
	drm_encoder_helper_add(encoder, &nulldisp_encoder_helper_funcs);

	/*
	 * This is a bit field that's used to determine which
	 * CRTCs can drive this encoder.
	 */
	encoder->possible_crtcs = 0x1;

	DRM_DEBUG_DRIVER("[ENCODER:%d:%s]\n", encoder->base.id, encoder->name);

	return encoder;
}


/******************************************************************************
 * Framebuffer functions
 ******************************************************************************/

static void nulldisp_framebuffer_destroy(struct drm_framebuffer *framebuffer)
{
	struct nulldisp_framebuffer *nulldisp_framebuffer =
		to_nulldisp_framebuffer(framebuffer);
	int i;

	DRM_DEBUG_DRIVER("[FB:%d]\n", framebuffer->base.id);

	drm_framebuffer_cleanup(framebuffer);

	for (i = 0; i < nulldisp_drm_fb_num_planes(framebuffer); i++)
		drm_gem_object_put_unlocked(nulldisp_framebuffer->obj[i]);

	kfree(nulldisp_framebuffer);
}

static int
nulldisp_framebuffer_create_handle(struct drm_framebuffer *framebuffer,
				   struct drm_file *file_priv,
				   unsigned int *handle)
{
	struct nulldisp_framebuffer *nulldisp_framebuffer =
		to_nulldisp_framebuffer(framebuffer);

	DRM_DEBUG_DRIVER("[FB:%d]\n", framebuffer->base.id);

	return drm_gem_handle_create(file_priv,
				     nulldisp_framebuffer->obj[0],
				     handle);
}

static int
nulldisp_framebuffer_dirty(struct drm_framebuffer *framebuffer,
			   struct drm_file *file_priv,
			   unsigned flags,
			   unsigned color,
			   struct drm_clip_rect *clips,
			   unsigned num_clips)
{
	struct nulldisp_framebuffer *nulldisp_fb =
		to_nulldisp_framebuffer(framebuffer);
	struct nulldisp_display_device *nulldisp_dev =
		framebuffer->dev->dev_private;
	struct nulldisp_crtc *nulldisp_crtc = nulldisp_dev->nulldisp_crtc;
	u64 addr[NULLDISP_MAX_PLANES],
	    size[NULLDISP_MAX_PLANES];
	int i;

	/*
	 * To prevent races with disconnect requests from user space,
	 * set the timeout before sending the copy request.
	 */
	for (i = 0; i < nulldisp_drm_fb_num_planes(framebuffer); i++) {
		struct drm_gem_object *obj = nulldisp_fb->obj[i];

		if (drm_gem_create_mmap_offset(obj)) {
			DRM_ERROR("Failed to get mmap offset for buffer[%d] = %p\n", i, obj);
			goto fail_flush;
		}

		addr[i] = drm_vma_node_offset_addr(&obj->vma_node);
		size[i] = obj->size;
	}

	nulldisp_set_copy_to(nulldisp_crtc);

	if (nlpvrdpy_send_copy(nulldisp_dev->nlpvrdpy,
			       &nulldisp_fb->base,
			       &addr[0],
			       &size[0]))
		goto fail_flush;

	wait_for_completion(&nulldisp_crtc->copy_done);

	return 0;

fail_flush:
	flush_delayed_work(&nulldisp_crtc->copy_to_work);

	wait_for_completion(&nulldisp_crtc->copy_done);

	return 0;

}

static const struct drm_framebuffer_funcs nulldisp_framebuffer_funcs = {
	.destroy = nulldisp_framebuffer_destroy,
	.create_handle = nulldisp_framebuffer_create_handle,
	.dirty = nulldisp_framebuffer_dirty,
};

static int
nulldisp_framebuffer_init(struct drm_device *dev,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 5, 0)) || \
	(defined(CHROMIUMOS_KERNEL) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 18, 0)))
			  const
#endif
			  struct drm_mode_fb_cmd2 *mode_cmd,
			  struct nulldisp_framebuffer *nulldisp_framebuffer,
			  struct drm_gem_object **obj)
{
	struct drm_framebuffer *fb = &nulldisp_framebuffer->base;
	int err;
	int i;

	fb->dev = dev;

	nulldisp_drm_fb_set_format(fb, mode_cmd->pixel_format);

	fb->width        = mode_cmd->width;
	fb->height       = mode_cmd->height;
	fb->flags        = mode_cmd->flags;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))	
	nulldisp_drm_fb_set_modifier(fb, mode_cmd->modifier[0]);
#endif

	for (i = 0; i < nulldisp_drm_fb_num_planes(fb); i++) {
		fb->pitches[i]  = mode_cmd->pitches[i];
		fb->offsets[i]  = mode_cmd->offsets[i];

		nulldisp_framebuffer->obj[i] = obj[i];
	}

	err = drm_framebuffer_init(dev, fb, &nulldisp_framebuffer_funcs);
	if (err) {
		DRM_ERROR("failed to initialise framebuffer structure (%d)\n",
			  err);
		return err;
	}

	DRM_DEBUG_DRIVER("[FB:%d]\n", fb->base.id);

	return 0;
}

static struct drm_framebuffer *
nulldisp_fb_create(struct drm_device *dev,
		   struct drm_file *file_priv,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 5, 0)) || \
	(defined(CHROMIUMOS_KERNEL) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 18, 0)))
		   const
#endif
		   struct drm_mode_fb_cmd2 *mode_cmd)
{
	struct drm_gem_object *obj[NULLDISP_MAX_PLANES];
	struct nulldisp_framebuffer *nulldisp_framebuffer;
	int err;
	int i;

	nulldisp_framebuffer = kzalloc(sizeof(*nulldisp_framebuffer),
				       GFP_KERNEL);
	if (!nulldisp_framebuffer) {
		err = -ENOMEM;
		goto fail_exit;
	}

	for (i = 0; i < drm_format_num_planes(mode_cmd->pixel_format); i++) {
		obj[i] = drm_gem_object_lookup(file_priv, mode_cmd->handles[i]);
		if (!obj[i]) {
			DRM_ERROR("failed to find buffer with handle %u\n",
				  mode_cmd->handles[i]);
			err = -ENOENT;
			goto fail_unreference;
		}
	}

	err = nulldisp_framebuffer_init(dev,
					mode_cmd,
					nulldisp_framebuffer,
					obj);
	if (err)
		goto fail_unreference;

	DRM_DEBUG_DRIVER("[FB:%d]\n", nulldisp_framebuffer->base.base.id);

	return &nulldisp_framebuffer->base;

fail_unreference:
	kfree(nulldisp_framebuffer);

	while (i--)
		drm_gem_object_put_unlocked(obj[i]);

fail_exit:
	return ERR_PTR(err);
}

static const struct drm_mode_config_funcs nulldisp_mode_config_funcs = {
	.fb_create = nulldisp_fb_create,
	.output_poll_changed = NULL,
};

static int nulldisp_nl_flipped_cb(void *data)
{
	struct nulldisp_crtc *nulldisp_crtc = data;

	flush_delayed_work(&nulldisp_crtc->flip_to_work);
	flush_delayed_work(&nulldisp_crtc->vb_work);

	return 0;
}

static int nulldisp_nl_copied_cb(void *data)
{
	struct nulldisp_crtc *nulldisp_crtc = data;

	flush_delayed_work(&nulldisp_crtc->copy_to_work);

	return 0;
}

static void nulldisp_nl_disconnect_cb(void *data)
{
	struct nulldisp_crtc *nulldisp_crtc = data;

	flush_delayed_work(&nulldisp_crtc->flip_to_work);
	flush_delayed_work(&nulldisp_crtc->copy_to_work);
}

static int nulldisp_early_load(struct drm_device *dev)
{
	struct nulldisp_display_device *nulldisp_dev;
	struct drm_connector *connector;
	struct drm_encoder *encoder;
	int err;

	platform_set_drvdata(to_platform_device(dev->dev), dev);

	nulldisp_dev = kzalloc(sizeof(*nulldisp_dev), GFP_KERNEL);
	if (!nulldisp_dev)
		return -ENOMEM;

	dev->dev_private = nulldisp_dev;
	nulldisp_dev->dev = dev;

	drm_mode_config_init(dev);

	dev->mode_config.funcs = (void *)&nulldisp_mode_config_funcs;
	dev->mode_config.min_width = NULLDISP_FB_WIDTH_MIN;
	dev->mode_config.max_width = NULLDISP_FB_WIDTH_MAX;
	dev->mode_config.min_height = NULLDISP_FB_HEIGHT_MIN;
	dev->mode_config.max_height = NULLDISP_FB_HEIGHT_MAX;
	dev->mode_config.fb_base = 0;
	dev->mode_config.async_page_flip = true;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
	dev->mode_config.allow_fb_modifiers = true;
#endif

	nulldisp_dev->nulldisp_crtc = nulldisp_crtc_create(nulldisp_dev);
	if (!nulldisp_dev->nulldisp_crtc) {
		DRM_ERROR("failed to create a CRTC.\n");

		err = -ENOMEM;
		goto err_config_cleanup;
	}

	connector = nulldisp_connector_create(nulldisp_dev,
					      DRM_MODE_CONNECTOR_Unknown);
	if (!connector) {
		DRM_ERROR("failed to create a connector.\n");

		err = -ENOMEM;
		goto err_config_cleanup;
	}
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 8, 0))
	nulldisp_dev->connector = connector;
#endif
	encoder = nulldisp_encoder_create(nulldisp_dev,
					  DRM_MODE_ENCODER_NONE);
	if (IS_ERR(encoder)) {
		DRM_ERROR("failed to create an encoder.\n");

		err = PTR_ERR(encoder);
		goto err_config_cleanup;
	}

	err = drm_mode_connector_attach_encoder(connector, encoder);
	if (err) {
		DRM_ERROR("failed to attach [ENCODER:%d:%s] to "
			  "[CONNECTOR:%d:%s] (err=%d)\n",
			  encoder->base.id,
			  encoder->name,
			  connector->base.id,
			  connector->name,
			  err);
		goto err_config_cleanup;
	}

#if defined(LMA)
	nulldisp_dev->pdp_gem_priv = pdp_gem_init(dev);
	if (!nulldisp_dev->pdp_gem_priv) {
		err = -ENOMEM;
		goto err_config_cleanup;
	}
#endif
	nulldisp_dev->workqueue =
		create_singlethread_workqueue(DRIVER_NAME);
	if (!nulldisp_dev->workqueue) {
		DRM_ERROR("failed to create work queue\n");
		goto err_gem_cleanup;
	}

	err = drm_vblank_init(nulldisp_dev->dev, 1);
	if (err) {
		DRM_ERROR("failed to complete vblank init (err=%d)\n", err);
		goto err_workqueue_cleanup;
	}

	dev->irq_enabled = true;

	nulldisp_dev->nlpvrdpy = nlpvrdpy_create(dev,
						 nulldisp_nl_disconnect_cb,
						 nulldisp_dev->nulldisp_crtc,
						 nulldisp_nl_flipped_cb,
						 nulldisp_dev->nulldisp_crtc,
						 nulldisp_nl_copied_cb,
						 nulldisp_dev->nulldisp_crtc);
	if (!nulldisp_dev->nlpvrdpy) {
		DRM_ERROR("Netlink initialisation failed (err=%d)\n", err);
		goto err_vblank_cleanup;
	}

	return 0;

err_vblank_cleanup:
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
	/* Called by drm_dev_fini in Linux 4.11.0 and later */
	drm_vblank_cleanup(dev);
#endif
err_workqueue_cleanup:
	destroy_workqueue(nulldisp_dev->workqueue);
	dev->irq_enabled = false;
err_gem_cleanup:
#if defined(LMA)
	pdp_gem_cleanup(nulldisp_dev->pdp_gem_priv);
#endif
err_config_cleanup:
	drm_mode_config_cleanup(dev);
	kfree(nulldisp_dev);
	return err;
}

static int nulldisp_late_load(struct drm_device *dev)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 8, 0))
	struct nulldisp_display_device *nulldisp_dev = dev->dev_private;
	int err;

	err = drm_connector_register(nulldisp_dev->connector);
	if (err) {
		DRM_ERROR("[CONNECTOR:%d:%s] failed to register (err=%d)\n",
			  nulldisp_dev->connector->base.id,
			  nulldisp_dev->connector->name,
			  err);
		return err;
	}
#endif
	return 0;
}

static void nulldisp_early_unload(struct drm_device *dev)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 8, 0))
	struct nulldisp_display_device *nulldisp_dev = dev->dev_private;

	drm_connector_unregister(nulldisp_dev->connector);
#endif
}

static void nulldisp_late_unload(struct drm_device *dev)
{
	struct nulldisp_display_device *nulldisp_dev = dev->dev_private;

	nlpvrdpy_send_disconnect(nulldisp_dev->nlpvrdpy);
	nlpvrdpy_destroy(nulldisp_dev->nlpvrdpy);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
	/* Called by drm_dev_fini in Linux 4.11.0 and later */
	drm_vblank_cleanup(dev);
#endif
	destroy_workqueue(nulldisp_dev->workqueue);

	dev->irq_enabled = false;

#if defined(LMA)
	pdp_gem_cleanup(nulldisp_dev->pdp_gem_priv);
#endif
	drm_mode_config_cleanup(dev);

	kfree(nulldisp_dev);
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 18, 0))
static int nulldisp_load(struct drm_device *dev, unsigned long flags)
{
	int err;

	err = nulldisp_early_load(dev);
	if (err)
		return err;

	err = nulldisp_late_load(dev);
	if (err) {
		nulldisp_late_unload(dev);
		return err;
	}

	return 0;
}

static int nulldisp_unload(struct drm_device *dev)
{
	nulldisp_early_unload(dev);
	nulldisp_late_unload(dev);

	return 0;
}
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 6, 0))
static void
nulldisp_crtc_flip_event_cancel(struct drm_crtc *crtc, struct drm_file *file)
{
	struct nulldisp_crtc *nulldisp_crtc = to_nulldisp_crtc(crtc);
	unsigned long flags;

	spin_lock_irqsave(&crtc->dev->event_lock, flags);

	if (nulldisp_crtc->flip_event &&
	    nulldisp_crtc->flip_event->base.file_priv == file) {
		struct drm_pending_event *pending_event =
			&nulldisp_crtc->flip_event->base;

		pending_event->destroy(pending_event);
		nulldisp_crtc->flip_event = NULL;
	}

	spin_unlock_irqrestore(&crtc->dev->event_lock, flags);
}
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 6, 0))
static void nulldisp_preclose(struct drm_device *dev, struct drm_file *file)
{
	struct drm_crtc *crtc;

	list_for_each_entry(crtc, &dev->mode_config.crtc_list, head)
		nulldisp_crtc_flip_event_cancel(crtc, file);
}
#endif

static void nulldisp_lastclose(struct drm_device *dev)
{
	struct drm_crtc *crtc;

	drm_modeset_lock_all(dev);
	list_for_each_entry(crtc, &dev->mode_config.crtc_list, head) {
		if (crtc->primary->fb) {
			struct drm_mode_set mode_set = { .crtc = crtc };
			int err;

			err = drm_mode_set_config_internal(&mode_set);
			if (err)
				DRM_ERROR("failed to disable crtc %p (err=%d)\n",
					  crtc, err);
		}
	}
	drm_modeset_unlock_all(dev);
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0)) || \
	(defined(CHROMIUMOS_KERNEL) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 18, 0)))
static int nulldisp_enable_vblank(struct drm_device *dev, unsigned int crtc)
#else
static int nulldisp_enable_vblank(struct drm_device *dev, int crtc)
#endif
{
	struct nulldisp_display_device *nulldisp_dev = dev->dev_private;

	switch (crtc) {
	case 0:
		break;
	default:
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0))
		DRM_ERROR("invalid crtc %u\n", crtc);
#else
		DRM_ERROR("invalid crtc %d\n", crtc);
#endif
		return -EINVAL;
	}

	if (!nulldisp_queue_vblank_work(nulldisp_dev->nulldisp_crtc)) {
		DRM_ERROR("work already queued\n");
		return -1;
	}

	return 0;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0)) || \
	(defined(CHROMIUMOS_KERNEL) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 18, 0)))
static void nulldisp_disable_vblank(struct drm_device *dev, unsigned int crtc)
#else
static void nulldisp_disable_vblank(struct drm_device *dev, int crtc)
#endif
{
	struct nulldisp_display_device *nulldisp_dev = dev->dev_private;

	switch (crtc) {
	case 0:
		break;
	default:
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0))
		DRM_ERROR("invalid crtc %u\n", crtc);
#else
		DRM_ERROR("invalid crtc %d\n", crtc);
#endif
		return;
	}

	/*
	 * Vblank events may be disabled from within the vblank handler,
	 * so don't wait for the work to complete.
	 */
	(void) cancel_delayed_work(&nulldisp_dev->nulldisp_crtc->vb_work);
}

static const struct vm_operations_struct nulldisp_gem_vm_ops = {
#if defined(LMA)
	.fault	= pdp_gem_object_vm_fault,
	.open	= drm_gem_vm_open,
	.close	= drm_gem_vm_close,
#else
	.fault	= nulldisp_gem_object_vm_fault,
	.open	= nulldisp_gem_vm_open,
	.close	= nulldisp_gem_vm_close,
#endif
};

#if defined(LMA)
static int pdp_gem_dumb_create(struct drm_file *file,
			       struct drm_device *dev,
			       struct drm_mode_create_dumb *args)
{
	struct nulldisp_display_device *nulldisp_dev = dev->dev_private;

	return pdp_gem_dumb_create_priv(file,
					dev,
					nulldisp_dev->pdp_gem_priv,
					args);
}

static int nulldisp_gem_object_create_ioctl(struct drm_device *dev,
					    void *data,
					    struct drm_file *file)
{
	struct drm_nulldisp_gem_create *args = data;
	struct nulldisp_display_device *nulldisp_dev = dev->dev_private;	
	struct drm_pdp_gem_create pdp_args;
	int err;
	
	if (args->flags) {
		DRM_ERROR("invalid flags: %#08x\n", args->flags);
		return -EINVAL;
	}

	if (args->handle) {
		DRM_ERROR("invalid handle (this should always be 0)\n");
		return -EINVAL;
	}

	/* 
	 * Remapping of nulldisp create args to pdp create args.
	 *
	 * Note: even though the nulldisp and pdp args are identical
	 * in this case, they may potentially change in future.
	 */
	pdp_args.size = args->size;
	pdp_args.flags = args->flags;
	pdp_args.handle = args->handle;

	err = pdp_gem_object_create_ioctl_priv(dev,
					       nulldisp_dev->pdp_gem_priv,
					       &pdp_args,
					       file);
	
	if (!err)
		args->handle = pdp_args.handle;

	return err;
}

static int nulldisp_gem_object_mmap_ioctl(struct drm_device *dev,
					  void *data,
					  struct drm_file *file)
{
	struct drm_nulldisp_gem_mmap *args = data;
	struct drm_pdp_gem_mmap pdp_args;
	int err;

	pdp_args.handle = args->handle;
	pdp_args.pad = args->pad;
	pdp_args.offset = args->offset;

	err = pdp_gem_object_mmap_ioctl(dev, &pdp_args, file);

	if (!err)
		args->offset = pdp_args.offset;

	return err;
}

static int nulldisp_gem_object_cpu_prep_ioctl(struct drm_device *dev,
					      void *data,
					      struct drm_file *file)
{
	struct drm_nulldisp_gem_cpu_prep *args =
		(struct drm_nulldisp_gem_cpu_prep *)data;
	struct drm_pdp_gem_cpu_prep pdp_args;

	pdp_args.handle = args->handle;
	pdp_args.flags = args->flags;

	return pdp_gem_object_cpu_prep_ioctl(dev, &pdp_args, file);
}

static int nulldisp_gem_object_cpu_fini_ioctl(struct drm_device *dev,
				       void *data,
				       struct drm_file *file)
{
	struct drm_nulldisp_gem_cpu_fini *args =
		(struct drm_nulldisp_gem_cpu_fini *)data;
	struct drm_pdp_gem_cpu_fini pdp_args;

	pdp_args.handle = args->handle;
	pdp_args.pad = args->pad;

	return pdp_gem_object_cpu_fini_ioctl(dev, &pdp_args, file);
}

static void pdp_gem_object_free(struct drm_gem_object *obj)
{
	struct nulldisp_display_device *nulldisp_dev = obj->dev->dev_private;

	pdp_gem_object_free_priv(nulldisp_dev->pdp_gem_priv, obj);
}
#endif

static const struct drm_ioctl_desc nulldisp_ioctls[] = {
	DRM_IOCTL_DEF_DRV(NULLDISP_GEM_CREATE, nulldisp_gem_object_create_ioctl, DRM_AUTH | DRM_UNLOCKED),
	DRM_IOCTL_DEF_DRV(NULLDISP_GEM_MMAP, nulldisp_gem_object_mmap_ioctl, DRM_AUTH | DRM_UNLOCKED),
	DRM_IOCTL_DEF_DRV(NULLDISP_GEM_CPU_PREP, nulldisp_gem_object_cpu_prep_ioctl, DRM_AUTH | DRM_UNLOCKED),
	DRM_IOCTL_DEF_DRV(NULLDISP_GEM_CPU_FINI, nulldisp_gem_object_cpu_fini_ioctl, DRM_AUTH | DRM_UNLOCKED),
};

static int nulldisp_gem_mmap(struct file *file, struct vm_area_struct *vma)
{
	int err;

	err = netlink_gem_mmap(file, vma);
#if !defined(LMA)
	if (!err) {
		struct drm_file *file_priv = file->private_data;
		struct drm_device *dev = file_priv->minor->dev;
		struct drm_gem_object *obj;

		mutex_lock(&dev->struct_mutex);
		obj = vma->vm_private_data;

		if (obj->import_attach)
			err = dma_buf_mmap(obj->dma_buf, vma, 0);
		else
			err = nulldisp_gem_object_get_pages(obj);

		mutex_unlock(&dev->struct_mutex);
	}
#endif
	return err;
}

static const struct file_operations nulldisp_driver_fops = {
	.owner		= THIS_MODULE,
	.open		= drm_open,
	.release	= drm_release,
	.unlocked_ioctl	= drm_ioctl,
	.mmap		= nulldisp_gem_mmap,
	.poll		= drm_poll,
	.read		= drm_read,
	.llseek		= noop_llseek,
#ifdef CONFIG_COMPAT
	.compat_ioctl	= drm_compat_ioctl,
#endif
};

static struct drm_driver nulldisp_drm_driver = {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 18, 0))
	.load				= NULL,
	.unload				= NULL,
#else
	.load				= nulldisp_load,
	.unload				= nulldisp_unload,
#endif
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 6, 0))
	.preclose			= nulldisp_preclose,
#endif
	.lastclose			= nulldisp_lastclose,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 18, 0)) && \
	(LINUX_VERSION_CODE < KERNEL_VERSION(4, 5, 0))
	.set_busid			= drm_platform_set_busid,
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 12, 0))
	.get_vblank_counter		= NULL,
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0))
	.get_vblank_counter		= drm_vblank_no_hw_counter,
#else
	.get_vblank_counter		= drm_vblank_count,
#endif
	.enable_vblank			= nulldisp_enable_vblank,
	.disable_vblank			= nulldisp_disable_vblank,


	.prime_handle_to_fd		= drm_gem_prime_handle_to_fd,
	.prime_fd_to_handle		= drm_gem_prime_fd_to_handle,

#if defined(LMA)
	.gem_free_object		= pdp_gem_object_free,
	.gem_prime_export		= pdp_gem_prime_export,
	.gem_prime_import		= pdp_gem_prime_import,
	.gem_prime_import_sg_table	= pdp_gem_prime_import_sg_table,

	.dumb_create			= pdp_gem_dumb_create,
	.dumb_map_offset		= pdp_gem_dumb_map_offset,
#else
	.gem_free_object		= nulldisp_gem_object_free,
	.gem_prime_export		= nulldisp_gem_prime_export,
	.gem_prime_import		= drm_gem_prime_import,
	.gem_prime_pin			= nulldisp_gem_prime_pin,
	.gem_prime_unpin		= nulldisp_gem_prime_unpin,
	.gem_prime_get_sg_table		= nulldisp_gem_prime_get_sg_table,
	.gem_prime_import_sg_table	= nulldisp_gem_prime_import_sg_table,
	.gem_prime_vmap			= nulldisp_gem_prime_vmap,
	.gem_prime_vunmap		= nulldisp_gem_prime_vunmap,
	.gem_prime_mmap			= nulldisp_gem_prime_mmap,
	.gem_prime_res_obj		= nulldisp_gem_prime_res_obj,

	.dumb_create			= nulldisp_gem_dumb_create,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
	.dumb_map_offset		= nulldisp_gem_dumb_map_offset,
#endif
#endif
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
	.dumb_destroy			= drm_gem_dumb_destroy,
#endif

	.gem_vm_ops			= &nulldisp_gem_vm_ops,

	.name				= DRIVER_NAME,
	.desc				= DRIVER_DESC,
	.date				= DRIVER_DATE,
	.major				= PVRVERSION_MAJ,
	.minor				= PVRVERSION_MIN,
	.patchlevel			= PVRVERSION_BUILD,

	.driver_features		= DRIVER_GEM |
					  DRIVER_MODESET |
					  DRIVER_PRIME,
	.ioctls				= nulldisp_ioctls,
	.num_ioctls			= ARRAY_SIZE(nulldisp_ioctls),
	.fops				= &nulldisp_driver_fops,
};

static int nulldisp_probe(struct platform_device *pdev)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 18, 0))
	struct drm_device *ddev;
	int ret;

	ddev = drm_dev_alloc(&nulldisp_drm_driver, &pdev->dev);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 0))
	if (IS_ERR(ddev))
		return PTR_ERR(ddev);
#else
	if (!ddev)
		return -ENOMEM;
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 5, 0))
	/* Needed by drm_platform_set_busid */
	ddev->platformdev = pdev;
#endif
	/*
	 * The load callback, called from drm_dev_register, is deprecated,
	 * because of potential race conditions.
	 */
	BUG_ON(nulldisp_drm_driver.load != NULL);

	ret = nulldisp_early_load(ddev);
	if (ret)
		goto err_drm_dev_unref;

	ret = drm_dev_register(ddev, 0);
	if (ret)
		goto err_drm_dev_late_unload;

	ret = nulldisp_late_load(ddev);
	if (ret)
		goto err_drm_dev_unregister;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 11, 0))
	DRM_INFO("Initialized %s %d.%d.%d %s on minor %d\n",
		nulldisp_drm_driver.name,
		nulldisp_drm_driver.major,
		nulldisp_drm_driver.minor,
		nulldisp_drm_driver.patchlevel,
		nulldisp_drm_driver.date,
		ddev->primary->index);
#endif
	return 0;

err_drm_dev_unregister:
	drm_dev_unregister(ddev);
err_drm_dev_late_unload:
	nulldisp_late_unload(ddev);
err_drm_dev_unref:
	drm_dev_unref(ddev);
	return	ret;
#else
	return drm_platform_init(&nulldisp_drm_driver, pdev);
#endif
}

static int nulldisp_remove(struct platform_device *pdev)
{
	struct drm_device *ddev = platform_get_drvdata(pdev);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 18, 0))
	/*
	 * The unload callback, called from drm_dev_unregister, is
	 * deprecated.
	 */
	BUG_ON(nulldisp_drm_driver.unload != NULL);

	nulldisp_early_unload(ddev);

	drm_dev_unregister(ddev);

	nulldisp_late_unload(ddev);

	drm_dev_unref(ddev);
#else
	drm_put_dev(ddev);
#endif
	return 0;
}

static void nulldisp_shutdown(struct platform_device *pdev)
{
}

static struct platform_device_id nulldisp_platform_device_id_table[] = {
#if defined(LMA)
	{ .name = APOLLO_DEVICE_NAME_PDP, .driver_data = 0 },
	{ .name = ODN_DEVICE_NAME_PDP, .driver_data = 0 },
#else
	{ .name = "nulldisp", .driver_data = 0 },
#endif
	{ },
};

static struct platform_driver nulldisp_platform_driver = {
	.probe		= nulldisp_probe,
	.remove		= nulldisp_remove,
	.shutdown	= nulldisp_shutdown,
	.driver		= {
		.owner  = THIS_MODULE,
		.name	= DRIVER_NAME,
	},
	.id_table	= nulldisp_platform_device_id_table,
};


static struct platform_device_info nulldisp_device_info = {
	.name		= "nulldisp",
	.id		= -1,
#if defined(LMA)
	/*
	 * The display hardware does not access system memory, so there
	 * is no DMA limitation.
	 */
	.dma_mask	= DMA_BIT_MASK(64),
#elif defined(NO_HARDWARE)
	/*
	* Not all cores have 40 bit physical support, but this
	* will work unless > 32 bit address is returned on those cores.
	* In the future this will be fixed more correctly.
	*/
	.dma_mask	= DMA_BIT_MASK(40),
#else
	.dma_mask	= DMA_BIT_MASK(32),
#endif
};

static struct platform_device *nulldisp_dev;

static int __init nulldisp_init(void)
{
	int err;

	err = nulldisp_validate_module_parameters();
	if (err) {
		DRM_ERROR("invalid module parameters (err=%d)\n", err);
		return err;
	}

	err = nlpvrdpy_register();
	if (err) {
		DRM_ERROR("failed to register with netlink (err=%d)\n", err);
		return err;
	}

	nulldisp_dev = platform_device_register_full(&nulldisp_device_info);
	if (IS_ERR(nulldisp_dev)) {
		err = PTR_ERR(nulldisp_dev);
		nulldisp_dev = NULL;
		goto err_unregister_family;
	}

	err = platform_driver_register(&nulldisp_platform_driver);
	if (err)
		goto err_unregister_family;

	return 0;

err_unregister_family:
		(void) nlpvrdpy_unregister();
		return err;
}

static void __exit nulldisp_exit(void)
{
	int err;

	err = nlpvrdpy_unregister();
	BUG_ON(err);

	if (nulldisp_dev)
		platform_device_unregister(nulldisp_dev);

	platform_driver_unregister(&nulldisp_platform_driver);
}

module_init(nulldisp_init);
module_exit(nulldisp_exit);

MODULE_AUTHOR("Imagination Technologies Ltd. <gpl-support@imgtec.com>");
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("Dual MIT/GPL");
