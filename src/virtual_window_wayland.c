#include <Elementary.h>
#include <string.h>
#include <Ecore_Evas.h>
#include <Ecore_X.h>
#include <Evas.h>

#include <dlog.h>
#include <livebox-errno.h>
#include <livebox-service.h>

#include "livebox.h"
#include "debug.h"

#define IS_PD 1

#define PUBLIC __attribute__((visibility("default")))

/*!
 * \brief
 * Abstracted Data Type of Virtual Window
 */
struct info {
	char *id; /*!< Identification */
	struct livebox_buffer *handle; /*!< Livebox buffer handle */
	int is_hw; /*!< 1 if a buffer is created on the H/W accelerated place or 0 */
	Ecore_Evas *ee;
	Evas *e;
	int is_pd;
};

static inline Evas_Object *get_highlighted_object(Evas_Object *obj)
{
	Evas_Object *o, *ho;

	o = evas_object_name_find(evas_object_evas_get(obj), "_elm_access_disp");
	if (!o) return NULL;

	ho = evas_object_data_get(o, "_elm_access_target");
	return ho;
}

/*!
 * \note
 * Every user event (mouse) on the buffer will be passed via this event callback
 */
static int event_handler_cb(struct livebox_buffer *handler, struct buffer_event_data *event_info, void *data)
{
	struct info *info = data;
	Elm_Access_Action_Info action_info;
	Elm_Access_Action_Type action_type;
	int ret = 0;
	Evas_Object *parent_elm;


	if (!info->handle) {
		/* Just ignore this event */
		return 0;
	}

	/*!
	 * \note
	 * Feed up events
	 */
	switch (event_info->type) {
	case BUFFER_EVENT_ENTER:
		evas_event_feed_mouse_in(info->e, event_info->timestamp * 1000, NULL);
		break;
	case BUFFER_EVENT_LEAVE:
		evas_event_feed_mouse_out(info->e, event_info->timestamp * 1000, NULL);
		break;
	case BUFFER_EVENT_DOWN:
		/*!
		 * \note
		 * Calculate the event occurred X & Y on the buffer
		 */
		evas_event_feed_mouse_move(info->e, event_info->info.pointer.x, event_info->info.pointer.y, (event_info->timestamp - 0.001f) * 1000, NULL); /* + 0.1f just for fake event */
		evas_event_feed_mouse_down(info->e, 1, EVAS_BUTTON_NONE, event_info->timestamp * 1000, NULL); /* + 0.2f just for fake event */
		break;
	case BUFFER_EVENT_MOVE:
		/*!
		 * \note
		 * Calculate the event occurred X & Y on the buffer
		 */
		evas_event_feed_mouse_move(info->e, event_info->info.pointer.x, event_info->info.pointer.y, event_info->timestamp * 1000, NULL);
		break;
	case BUFFER_EVENT_UP:
		evas_event_feed_mouse_move(info->e, event_info->info.pointer.x, event_info->info.pointer.y, event_info->timestamp * 1000, NULL);
		evas_event_feed_mouse_up(info->e, 1, EVAS_BUTTON_NONE, (event_info->timestamp + 0.001f) * 1000, NULL);
		break;
	case BUFFER_EVENT_HIGHLIGHT:
		parent_elm = ecore_evas_data_get(info->ee, "dynamic,box,win");
		if (!parent_elm) {
			ret = LB_ACCESS_STATUS_ERROR;
			break;
		}
		memset(&action_info, 0, sizeof(action_info));
		action_type = ELM_ACCESS_ACTION_HIGHLIGHT;
		/*!
		 * \note
		 * Calculate the event occurred X & Y on the buffer
		 */
		action_info.x = event_info->info.pointer.x;
		action_info.y = event_info->info.pointer.y;
		ret = elm_access_action(parent_elm, action_type, &action_info);
		if (ret == EINA_TRUE) {
			if (!get_highlighted_object(parent_elm)) {
				LOGE("Highlighted object is not found\n");
				ret = LB_ACCESS_STATUS_ERROR;
			} else {
				LOGD("Highlighted object is found\n");
				ret = LB_ACCESS_STATUS_DONE;
			}
		} else {
			LOGE("Action error\n");
			ret = LB_ACCESS_STATUS_ERROR;
		}
		break;
	case BUFFER_EVENT_HIGHLIGHT_NEXT:
		parent_elm = ecore_evas_data_get(info->ee, "dynamic,box,win");
		if (!parent_elm) {
			ret = LB_ACCESS_STATUS_ERROR;
			break;
		}
		memset(&action_info, 0, sizeof(action_info));
		action_type = ELM_ACCESS_ACTION_HIGHLIGHT_NEXT;
		action_info.highlight_cycle = EINA_FALSE;
		ret = elm_access_action(parent_elm, action_type, &action_info);
		ret = (ret == EINA_FALSE) ? LB_ACCESS_STATUS_LAST : LB_ACCESS_STATUS_DONE;
		break;
	case BUFFER_EVENT_HIGHLIGHT_PREV:
		parent_elm = ecore_evas_data_get(info->ee, "dynamic,box,win");
		if (!parent_elm) {
			ret = LB_ACCESS_STATUS_ERROR;
			break;
		}
		memset(&action_info, 0, sizeof(action_info));
		action_type = ELM_ACCESS_ACTION_HIGHLIGHT_PREV;
		action_info.highlight_cycle = EINA_FALSE;
		ret = elm_access_action(parent_elm, action_type, &action_info);
		ret = (ret == EINA_FALSE) ? LB_ACCESS_STATUS_FIRST : LB_ACCESS_STATUS_DONE;
		break;
	case BUFFER_EVENT_ACTIVATE:
		parent_elm = ecore_evas_data_get(info->ee, "dynamic,box,win");
		if (!parent_elm) {
			ret = LB_ACCESS_STATUS_ERROR;
			break;
		}
		memset(&action_info, 0, sizeof(action_info));
		action_type = ELM_ACCESS_ACTION_ACTIVATE;
		ret = elm_access_action(parent_elm, action_type, &action_info);
		ret = (ret == EINA_FALSE) ? LB_ACCESS_STATUS_ERROR : LB_ACCESS_STATUS_DONE;
		break;
	case BUFFER_EVENT_ACTION_UP:
		parent_elm = ecore_evas_data_get(info->ee, "dynamic,box,win");
		if (!parent_elm) {
			ret = LB_ACCESS_STATUS_ERROR;
			break;
		}
		memset(&action_info, 0, sizeof(action_info));
		action_type = ELM_ACCESS_ACTION_UP;
		ret = elm_access_action(parent_elm, action_type, &action_info);
		ret = (ret == EINA_FALSE) ? LB_ACCESS_STATUS_ERROR : LB_ACCESS_STATUS_DONE;
		break;
	case BUFFER_EVENT_ACTION_DOWN:
		parent_elm = ecore_evas_data_get(info->ee, "dynamic,box,win");
		if (!parent_elm) {
			ret = LB_ACCESS_STATUS_ERROR;
			break;
		}
		memset(&action_info, 0, sizeof(action_info));
		action_type = ELM_ACCESS_ACTION_DOWN;
		ret = elm_access_action(parent_elm, action_type, &action_info);
		ret = (ret == EINA_FALSE) ? LB_ACCESS_STATUS_ERROR : LB_ACCESS_STATUS_DONE;
		break;
	case BUFFER_EVENT_SCROLL_UP:
		parent_elm = ecore_evas_data_get(info->ee, "dynamic,box,win");
		if (!parent_elm) {
			ret = LB_ACCESS_STATUS_ERROR;
			break;
		}
		memset(&action_info, 0, sizeof(action_info));
		action_type = ELM_ACCESS_ACTION_SCROLL;
		action_info.x = event_info->info.pointer.x;
		action_info.y = event_info->info.pointer.y;
		action_info.mouse_type = 2;
		ret = elm_access_action(parent_elm, action_type, &action_info);
		ret = (ret == EINA_FALSE) ? LB_ACCESS_STATUS_ERROR : LB_ACCESS_STATUS_DONE;
		break;
	case BUFFER_EVENT_SCROLL_MOVE:
		parent_elm = ecore_evas_data_get(info->ee, "dynamic,box,win");
		if (!parent_elm) {
			ret = LB_ACCESS_STATUS_ERROR;
			break;
		}
		memset(&action_info, 0, sizeof(action_info));
		action_type = ELM_ACCESS_ACTION_SCROLL;
		action_info.x = event_info->info.pointer.x;
		action_info.y = event_info->info.pointer.y;
		action_info.mouse_type = 1;
		ret = elm_access_action(parent_elm, action_type, &action_info);
		ret = (ret == EINA_FALSE) ? LB_ACCESS_STATUS_ERROR : LB_ACCESS_STATUS_DONE;
		break;
	case BUFFER_EVENT_SCROLL_DOWN:
		parent_elm = ecore_evas_data_get(info->ee, "dynamic,box,win");
		if (!parent_elm) {
			ret = LB_ACCESS_STATUS_ERROR;
			break;
		}
		memset(&action_info, 0, sizeof(action_info));
		action_type = ELM_ACCESS_ACTION_SCROLL;
		action_info.x = event_info->info.pointer.x;
		action_info.y = event_info->info.pointer.y;
		action_info.mouse_type = 0;
		ret = elm_access_action(parent_elm, action_type, &action_info);
		ret = (ret == EINA_FALSE) ? LB_ACCESS_STATUS_ERROR : LB_ACCESS_STATUS_DONE;
		break;
	case BUFFER_EVENT_UNHIGHLIGHT:
		parent_elm = ecore_evas_data_get(info->ee, "dynamic,box,win");
		if (!parent_elm) {
			ret = LB_ACCESS_STATUS_ERROR;
			break;
		}
		memset(&action_info, 0, sizeof(action_info));
		action_type = ELM_ACCESS_ACTION_UNHIGHLIGHT;
		ret = elm_access_action(parent_elm, action_type, &action_info);
		ret = (ret == EINA_FALSE) ? LB_ACCESS_STATUS_ERROR : LB_ACCESS_STATUS_DONE;
		break;
	case BUFFER_EVENT_KEY_DOWN:
		parent_elm = ecore_evas_data_get(info->ee, "dynamic,box,win");
		if (!parent_elm) {
			ret = LB_ACCESS_STATUS_ERROR;
			break;
		}

		ret = LB_KEY_STATUS_ERROR;
		break;
	case BUFFER_EVENT_KEY_UP:
		parent_elm = ecore_evas_data_get(info->ee, "dynamic,box,win");
		if (!parent_elm) {
			ret = LB_ACCESS_STATUS_ERROR;
			break;
		}

		ret = LB_KEY_STATUS_ERROR;
		break;
	case BUFFER_EVENT_KEY_FOCUS_IN:
		parent_elm = ecore_evas_data_get(info->ee, "dynamic,box,win");
		if (!parent_elm) {
			ret = LB_ACCESS_STATUS_ERROR;
			break;
		}

		ret = LB_KEY_STATUS_ERROR;
		break;
	case BUFFER_EVENT_KEY_FOCUS_OUT:
		parent_elm = ecore_evas_data_get(info->ee, "dynamic,box,win");
		if (!parent_elm) {
			ret = LB_ACCESS_STATUS_ERROR;
			break;
		}

		ret = LB_KEY_STATUS_ERROR;
		break;
	default:
		LOGD("Unhandled buffer event (%d)\n", event_info->type);
		break;
	}

	return ret;
}

static void *alloc_fb(void *data, int size)
{
	struct info *info = data;
	void *buffer;
	int width = 0;
	int height = 0;

	if (info->ee == NULL) {
		width = 1;
		height = 1;
	} else {
		ecore_evas_geometry_get(info->ee, NULL, NULL, &width, &height);
	}

	/*!
	 * Acquire a buffer for canvas.
	 */
	info->handle = livebox_acquire_buffer_NEW(info->id, info->is_pd,
					width, height,
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

PUBLIC Evas_Object *livebox_get_evas_object(const char *id, int is_pd)
{
	struct info *info;
	Evas_Object *rect;

	if (!id) {
		ErrPrint("Invalid parameter\n");
		return NULL;
	}

	info = calloc(1, sizeof(*info));
	if (!info) {
		ErrPrint("Heap: %s\n", strerror(errno));
		return NULL;
	}

	info->id = strdup(id);
	if (!info->id) {
		ErrPrint("Heap: %s\n", strerror(errno));
		free(info);
		return NULL;
	}

	info->is_pd = is_pd;

	info->ee = ecore_evas_buffer_allocfunc_new(1, 1, alloc_fb, free_fb, info);
	if (!info->ee) {
		ErrPrint("Failed to create ecore_evas (%dx%d)\n", 1, 1);
		free(info->id);
		free(info);
		return NULL;
	}

	pre_render_cb(info, NULL, NULL);
	ecore_evas_alpha_set(info->ee, EINA_TRUE);
	post_render_cb(info, NULL, NULL);

	ecore_evas_data_set(info->ee, "dynamic,box,info", info);

	ecore_evas_manual_render_set(info->ee, EINA_FALSE);
	ecore_evas_resize(info->ee, 1, 1);

	info->e = ecore_evas_get(info->ee);
	if (!info->e) {
		ErrPrint("Failed to get evas\n");
		ecore_evas_free(info->ee);
		return NULL;
	}

	evas_event_callback_add(info->e, EVAS_CALLBACK_RENDER_POST, post_render_cb, info);
	evas_event_callback_add(info->e, EVAS_CALLBACK_RENDER_PRE, pre_render_cb, info);

	rect = evas_object_rectangle_add(info->e);
	if (!rect) {
		ErrPrint("Failed to create evas_object\n");
		ecore_evas_free(info->ee);
		return NULL;
	}

	evas_object_resize(rect, 1, 1);
	evas_object_color_set(rect, 0, 0, 0, 0);
	return rect;
}

/* End of a file */