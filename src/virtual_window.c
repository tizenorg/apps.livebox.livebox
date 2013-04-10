#include <Ecore_Evas.h>
#include <Evas.h>

#include <dlog.h>
#include <livebox-errno.h>

#include "livebox.h"

#define IS_PD 1

#define PUBLIC __attribute__((visibility("default")))
/*!
 * \brief
 * Abstracted Data Type of Virtual Window
 */
struct info {
	char *id; /*!< Identification */
	int width; /*!< Width */
	int height; /*!< Height */
	struct livebox_buffer *handle; /*!< Livebox buffer handle */
	Evas_Object *window; /*!< Parent evas object - WARN: Incompatible with the elm_win object */
	int is_hw; /*!< 1 if a buffer is created on the H/W accelerated place or 0 */
};

/*!
 * \note
 * Every user event (mouse) on the buffer will be passed via this event callback
 */
static int event_handler_cb(struct livebox_buffer *handler, enum buffer_event evt, double timestamp, double x, double y, void *data)
{
	struct info *info = data;
	Evas *e;
	int ix;
	int iy;

	if (!info->handle) {
		/* Just ignore this event */
		return 0;
	}

	/*!
	 * \note
	 * Calculate the event occurred X & Y on the buffer
	 */
	ix = info->width * x;
	iy = info->height * y;

	e = evas_object_evas_get(info->window);

	/*!
	 * \note
	 * Feed up events
	 */
	switch (evt) {
	case BUFFER_EVENT_ENTER:
		evas_event_feed_mouse_in(e, timestamp, NULL);
		break;
	case BUFFER_EVENT_LEAVE:
		evas_event_feed_mouse_out(e, timestamp, NULL);
		break;
	case BUFFER_EVENT_DOWN:
		evas_event_feed_mouse_in(e, timestamp, NULL);
		evas_event_feed_mouse_move(e, ix, iy, timestamp + 0.01f, NULL); /* + 0.1f just for fake event */
		evas_event_feed_mouse_down(e, 1, EVAS_BUTTON_NONE, timestamp + 0.02f, NULL); /* + 0.2f just for fake event */
		break;
	case BUFFER_EVENT_MOVE:
		evas_event_feed_mouse_move(e, ix, iy, timestamp, NULL);
		break;
	case BUFFER_EVENT_UP:
		evas_event_feed_mouse_up(e, 1, EVAS_BUTTON_NONE, timestamp, NULL);
		evas_event_feed_mouse_out(e, timestamp + 0.01f, NULL); /* + 0.1f just for fake event */
		break;
	default:
		LOGD("Unhandled buffer event (%d)\n", evt);
		break;
	}

	return 0;
}

static void *alloc_fb(void *data, int size)
{
	struct info *info = data;
	void *buffer;

	/*!
	 * Acquire a buffer for canvas.
	 */
	info->handle = livebox_acquire_buffer(info->id, IS_PD,
					info->width, info->height,
					event_handler_cb, info);

	/*!
	 * If it supports the H/W accelerated buffer,
	 * Use it.
	 */
	if (livebox_support_hw_buffer(info->handle)) {
		if (livebox_create_hw_buffer(info->handle) == 0) {
			buffer = livebox_buffer_hw_buffer(info->handle);
			if (buffer) {
				LOGD("HW Accelerated buffer is created\n");
				info->is_hw = 1;
				return buffer;
			}
		}

		LOGE("Failed to allocate HW Accelerated buffer\n");
	}

	/*!
	 * Or use the buffer of a S/W backend.
	 */
	buffer = livebox_ref_buffer(info->handle);
	LOGD("SW buffer is created\n");
	info->is_hw = 0;
	return buffer;
}

static void free_fb(void *data, void *ptr)
{
	struct info *info = data;

	if (!info->handle)
		return;

	if (info->is_hw) {
		if (livebox_destroy_hw_buffer(info->handle) == 0) {
			LOGD("HW Accelerated buffer is destroyed\n");
			goto out;
		}
	}

	livebox_unref_buffer(ptr);
	LOGD("SW buffer is destroyed\n");
out:
	livebox_release_buffer(info->handle);
	info->handle = NULL;
}

static void resize_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	struct info *info = data;
	Ecore_Evas *ee;

	ee = ecore_evas_ecore_evas_get(e);
	if (!ee)
		return;

	evas_object_geometry_get(obj, NULL, NULL, &info->width, &info->height);
	LOGD("Resize to %dx%d\n", info->width, info->height);
	/*!
	 * Box(parent object) is resized.
	 * Try to resize the canvas too.
	 */
	ecore_evas_resize(ee, info->width, info->height);
}

/*!
 * If a canvas is destroyed,
 * Free all buffer of canvas.
 */
static void del_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Ecore_Evas *ee;
	struct info *info = data;

	ee = ecore_evas_ecore_evas_get(e);
	if (!ee)
		return;

	LOGD("Try to release the ECORE_EVAS\n");
	ecore_evas_free(ee);
	free(info->id);
	free(info);
	LOGD("ECORE_EVAS is released\n");
}

static void pre_render_cb(void *data, Evas *e, void *event_info)
{
	struct info *info = data;

	if (!info->handle)
		return;

	if (info->is_hw)
		livebox_buffer_pre_render(info->handle);
}

static void post_render_cb(void *data, Evas *e, void *event_info)
{
	struct info *info = data;

	if (!info->handle)
		return;

	if (info->is_hw)
		livebox_buffer_post_render(info->handle);
	else
		livebox_sync_buffer(info->handle);
}

PUBLIC Evas_Object *livebox_virtual_window_add(const char *id, int width, int height)
{
	Ecore_Evas *ee;
	Evas *e;
	struct info *info;

	info = calloc(1, sizeof(*info));
	if (!info) {
		return NULL;
	}

	info->id = strdup(id);
	if (!info->id) {
		free(info);
		return NULL;
	}

	info->width = width;
	info->height = height;

	ee = ecore_evas_buffer_allocfunc_new(width, height, alloc_fb, free_fb, info);
	if (!ee) {
		free(info->id);
		free(info);
		return NULL;
	}

	pre_render_cb(info, NULL, NULL);
	ecore_evas_alpha_set(ee, EINA_TRUE);
	post_render_cb(info, NULL, NULL);
	
	ecore_evas_manual_render_set(ee, EINA_FALSE);
	ecore_evas_resize(ee, width, height);

	e = ecore_evas_get(ee);
	if (!e) {
		ecore_evas_free(ee);
		return NULL;
	}

	evas_event_callback_add(e, EVAS_CALLBACK_RENDER_POST, post_render_cb, info);
	evas_event_callback_add(e, EVAS_CALLBACK_RENDER_PRE, pre_render_cb, info);

	info->window = evas_object_rectangle_add(e);
	if (!info->window) {
		ecore_evas_free(ee);
		return NULL;
	}

	evas_object_resize(info->window, width, height);
	evas_object_color_set(info->window, 0, 0, 0, 0);
	evas_object_event_callback_add(info->window, EVAS_CALLBACK_DEL, del_cb, info);
	evas_object_event_callback_add(info->window, EVAS_CALLBACK_RESIZE, resize_cb, info);

	return info->window;
}

PUBLIC int livebox_virtual_window_del(Evas_Object *virtual_win)
{
	evas_object_del(virtual_win);
	return LB_STATUS_SUCCESS;
}

/* End of a file */
