#include <Elementary.h>
#include <string.h>
#include <Ecore_Evas.h>
#include <Evas.h>

#include <dlog.h>
#include <livebox-errno.h>
#include <livebox-service.h>

#include "livebox.h"
#if !defined(SECURE_LOGD)
#define SECURE_LOGD LOGD
#endif

#if !defined(SECURE_LOGE)
#define SECURE_LOGE LOGE
#endif

#if !defined(SECURE_LOGW)
#define SECURE_LOGW LOGW
#endif


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
	Elm_Access_Action_Info action_info;
	Elm_Access_Action_Type action_type;
	Evas *e;
	Ecore_Evas *ee;
	Evas_Object *parent_elm;
	int ix;
	int iy;
	int ret = 0;

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

	memset(&action_info, 0, sizeof(action_info));

	e = evas_object_evas_get(info->window);
	ee = ecore_evas_ecore_evas_get(e);
	parent_elm = ecore_evas_data_get(ee, "parent,elm");

	/*!
	 * \note
	 * Feed up events
	 */
	switch (evt) {
	case BUFFER_EVENT_ENTER:
		evas_event_feed_mouse_in(e, timestamp * 1000, NULL);
		break;
	case BUFFER_EVENT_LEAVE:
		evas_event_feed_mouse_out(e, timestamp * 1000, NULL);
		break;
	case BUFFER_EVENT_DOWN:
		evas_event_feed_mouse_in(e, timestamp * 1000, NULL);
		evas_event_feed_mouse_move(e, ix, iy, (timestamp + 0.01f) * 1000, NULL); /* + 0.1f just for fake event */
		evas_event_feed_mouse_down(e, 1, EVAS_BUTTON_NONE, (timestamp + 0.02f) * 1000, NULL); /* + 0.2f just for fake event */
		break;
	case BUFFER_EVENT_MOVE:
		evas_event_feed_mouse_move(e, ix, iy, timestamp * 1000, NULL);
		break;
	case BUFFER_EVENT_UP:
		evas_event_feed_mouse_up(e, 1, EVAS_BUTTON_NONE, timestamp * 1000, NULL);
		evas_event_feed_mouse_out(e, (timestamp + 0.01f) * 1000, NULL); /* + 0.1f just for fake event */
		break;
	case BUFFER_EVENT_HIGHLIGHT:
		if (!parent_elm) {
			ret = LB_ACCESS_STATUS_ERROR;
			break;
		}
		action_type = ELM_ACCESS_ACTION_HIGHLIGHT;
		action_info.x = ix;
		action_info.y = iy;
		ret = elm_access_action(parent_elm, action_type, &action_info);
		ret = (ret == EINA_FALSE) ? LB_ACCESS_STATUS_ERROR : LB_ACCESS_STATUS_DONE;
		break;
	case BUFFER_EVENT_HIGHLIGHT_NEXT:
		if (!parent_elm) {
			ret = LB_ACCESS_STATUS_ERROR;
			break;
		}
		action_type = ELM_ACCESS_ACTION_HIGHLIGHT_NEXT;
		action_info.highlight_cycle = EINA_FALSE;
		ret = elm_access_action(parent_elm, action_type, &action_info);
		ret = (ret == EINA_FALSE) ? LB_ACCESS_STATUS_LAST : LB_ACCESS_STATUS_DONE;
		break;
	case BUFFER_EVENT_HIGHLIGHT_PREV:
		if (!parent_elm) {
			ret = LB_ACCESS_STATUS_ERROR;
			break;
		}
		action_type = ELM_ACCESS_ACTION_HIGHLIGHT_PREV;
		action_info.highlight_cycle = EINA_FALSE;
		ret = elm_access_action(parent_elm, action_type, &action_info);
		ret = (ret == EINA_FALSE) ? LB_ACCESS_STATUS_FIRST : LB_ACCESS_STATUS_DONE;
		break;
	case BUFFER_EVENT_ACTIVATE:
		if (!parent_elm) {
			ret = LB_ACCESS_STATUS_ERROR;
			break;
		}
		action_type = ELM_ACCESS_ACTION_ACTIVATE;
		ret = elm_access_action(parent_elm, action_type, &action_info);
		ret = (ret == EINA_FALSE) ? LB_ACCESS_STATUS_ERROR : LB_ACCESS_STATUS_DONE;
		break;
	case BUFFER_EVENT_ACTION_UP:
		if (!parent_elm) {
			ret = LB_ACCESS_STATUS_ERROR;
			break;
		}
		action_type = ELM_ACCESS_ACTION_UP;
		ret = elm_access_action(parent_elm, action_type, &action_info);
		ret = (ret == EINA_FALSE) ? LB_ACCESS_STATUS_ERROR : LB_ACCESS_STATUS_DONE;
		break;
	case BUFFER_EVENT_ACTION_DOWN:
		if (!parent_elm) {
			ret = LB_ACCESS_STATUS_ERROR;
			break;
		}
		action_type = ELM_ACCESS_ACTION_DOWN;
		ret = elm_access_action(parent_elm, action_type, &action_info);
		ret = (ret == EINA_FALSE) ? LB_ACCESS_STATUS_ERROR : LB_ACCESS_STATUS_DONE;
		break;
	case BUFFER_EVENT_SCROLL_UP:
		if (!parent_elm) {
			ret = LB_ACCESS_STATUS_ERROR;
			break;
		}
		action_type = ELM_ACCESS_ACTION_SCROLL;
		action_info.x = ix;
		action_info.y = iy;
		action_info.mouse_type = 2;
		ret = elm_access_action(parent_elm, action_type, &action_info);
		ret = (ret == EINA_FALSE) ? LB_ACCESS_STATUS_ERROR : LB_ACCESS_STATUS_DONE;
		break;
	case BUFFER_EVENT_SCROLL_MOVE:
		if (!parent_elm) {
			ret = LB_ACCESS_STATUS_ERROR;
			break;
		}
		action_type = ELM_ACCESS_ACTION_SCROLL;
		action_info.x = ix;
		action_info.y = iy;
		action_info.mouse_type = 1;
		ret = elm_access_action(parent_elm, action_type, &action_info);
		ret = (ret == EINA_FALSE) ? LB_ACCESS_STATUS_ERROR : LB_ACCESS_STATUS_DONE;
		break;
	case BUFFER_EVENT_SCROLL_DOWN:
		if (!parent_elm) {
			ret = LB_ACCESS_STATUS_ERROR;
			break;
		}
		action_type = ELM_ACCESS_ACTION_SCROLL;
		action_info.x = ix;
		action_info.y = iy;
		action_info.mouse_type = 0;
		ret = elm_access_action(parent_elm, action_type, &action_info);
		ret = (ret == EINA_FALSE) ? LB_ACCESS_STATUS_ERROR : LB_ACCESS_STATUS_DONE;
		break;
	case BUFFER_EVENT_UNHIGHLIGHT:
		if (!parent_elm) {
			ret = LB_ACCESS_STATUS_ERROR;
			break;
		}
		action_type = ELM_ACCESS_ACTION_UNHIGHLIGHT;
		ret = elm_access_action(parent_elm, action_type, &action_info);
		ret = (ret == EINA_FALSE) ? LB_ACCESS_STATUS_ERROR : LB_ACCESS_STATUS_DONE;
		break;
	default:
		LOGD("Unhandled buffer event (%d)\n", evt);
		break;
	}

	return ret;
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

	if (!info->handle) {
		return;
	}

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
	if (!ee) {
		return;
	}

	evas_object_geometry_get(obj, NULL, NULL, &info->width, &info->height);
	SECURE_LOGD("Resize to %dx%d\n", info->width, info->height);
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
	if (!ee) {
		return;
	}

	ecore_evas_free(ee);
	free(info->id);
	free(info);
}

static void pre_render_cb(void *data, Evas *e, void *event_info)
{
	struct info *info = data;

	if (!info->handle) {
		return;
	}

	if (info->is_hw) {
		livebox_buffer_pre_render(info->handle);
	}
}

static void post_render_cb(void *data, Evas *e, void *event_info)
{
	struct info *info = data;

	if (!info->handle) {
		return;
	}

	if (info->is_hw) {
		livebox_buffer_post_render(info->handle);
	} else {
		livebox_sync_buffer(info->handle);
	}
}

PUBLIC Evas_Object *virtual_window_create(const char *id, int width, int height)
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

PUBLIC int virtual_window_set_parent_elm(Evas_Object *win, Evas_Object *parent)
{
	Evas *e;
	Ecore_Evas *ee;

	if (!win) {
		return LB_STATUS_ERROR_INVALID;
	}

	e = evas_object_evas_get(win);
	if (!e) {
		return LB_STATUS_ERROR_FAULT;
	}

	ee = ecore_evas_ecore_evas_get(e);
	if (!ee) {
		return LB_STATUS_ERROR_FAULT;
	}

	ecore_evas_data_set(ee, "parent,elm", parent);
	return 0;
}

/* End of a file */
