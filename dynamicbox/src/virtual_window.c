/*
 * Copyright 2013  Samsung Electronics Co., Ltd
 *
 * Licensed under the Flora License, Version 1.1 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://floralicense.org/license/
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define _GNU_SOURCE

#include <Elementary.h>
#include <string.h>
#include <Ecore_Evas.h>
#include <Ecore_X.h>
#include <Evas.h>
#include <dlfcn.h>
#include <Eina.h>

#include <X11/Xlib.h>

#include <dlog.h>
#include <dynamicbox_errno.h>
#include <dynamicbox_service.h>
#include <dynamicbox_conf.h>
#include <dynamicbox_buffer.h>

#include "dynamicbox.h"
#include "internal/dynamicbox.h"
#include "debug.h"

#define IS_GBAR 1

#define PUBLIC __attribute__((visibility("default")))
#define DBOX_WIN_TAG "dynamic,box,win"

#define DBOX_DEFAULT_WIDTH 1
#define DBOX_DEFAULT_HEIGHT 1
#define GL_ENGINE "opengl_x11"

static struct static_info {
    Ecore_Evas *(*alloc_canvas)(int w, int h, void *(*a)(void *data, int size), void (*f)(void *data, void *ptr), void *data);
    Ecore_Evas *(*alloc_canvas_with_stride)(int w, int h, void *(*a)(void *data, int size, int *stride, int *bpp), void (*f)(void *data, void *ptr), void *data);
    Ecore_Evas *(*alloc_canvas_with_pixmap)(const char *disp_name, Ecore_X_Window parent, int x, int y, int w, int h, Ecore_X_Pixmap (*alloc_cb)(void *data, Ecore_X_Window parent, int w, int h, int depth), void (*free_cb)(void *data, Ecore_X_Pixmap pixmap), void *data);
} s_info = {
    .alloc_canvas = NULL,
    .alloc_canvas_with_stride = NULL,
    .alloc_canvas_with_pixmap = NULL,
};

/**
 * @brief
 * Abstracted Data Type of Virtual Window
 */
typedef struct virtual_window_info {
    char *id; /**< Identification */
    dynamicbox_buffer_h handle; /**< Livebox buffer handle */
    enum win_type {
	VWIN_SW_BUF = 0x00, /**< S/W buffer */
	VWIN_GEM    = 0x01, /**< GEM buffer */
	VWIN_PIXMAP = 0x02, /**< PIXMAP */
	VWIN_ERROR  = 0x03  /**< Unknown */
    } type;
    Ecore_Evas *ee;
    Evas *e;
    int is_gbar;
    int deleted;
    int w;
    int h;
    unsigned int *resource_array;
    int resource_cnt;

    int pressed;
} *vwin_info_t;

static inline Evas_Object *get_highlighted_object(Evas_Object *obj)
{
    Evas_Object *o, *ho;

    o = evas_object_name_find(evas_object_evas_get(obj), "_elm_access_disp");
    if (!o) return NULL;

    ho = evas_object_data_get(o, "_elm_access_target");
    return ho;
}

/**
 * @note
 * Every user event (mouse) on the buffer will be passed via this event callback
 */
static int event_handler_cb(dynamicbox_buffer_h handler, struct dynamicbox_buffer_event_data *event_info, void *data)
{
    vwin_info_t info = data;
    Elm_Access_Action_Info action_info;
    Elm_Access_Action_Type action_type;
    int ret = 0;
    Evas_Object *parent_elm;
    KeySym *key_symbol;
    unsigned int flags = 0;
    double timestamp;

    if (!info->handle) {
	/* Just ignore this event */
	return 0;
    }

    if (DYNAMICBOX_CONF_USE_GETTIMEOFDAY) {
	if (DYNAMICBOX_CONF_EVENT_FILTER > 0.0f && (info->pressed == 0 || event_info->type == DBOX_BUFFER_EVENT_MOVE)) {
	    struct timeval tv;

	    if (gettimeofday(&tv, NULL) < 0) {
		ErrPrint("gettimeofday: %s\n", strerror(errno));
	    } else {
		timestamp = (double)tv.tv_sec + (double)tv.tv_usec / 1000000.0f;
		timestamp -= event_info->timestamp;

		if (timestamp > DYNAMICBOX_CONF_EVENT_FILTER) {
		    DbgPrint("Dropped %lf\n", timestamp);
		    return 0;
		}
	    }
	}

	/**
	 * If the device doesn't use the clock monotic time, we have to emulate it for EVAS
	 * Evas only use the monotic time for animating objects
	 */
	timestamp = ecore_time_get() * 1000.0f;
    } else {
	if (DYNAMICBOX_CONF_EVENT_FILTER > 0.0f && (info->pressed == 0 || event_info->type == DBOX_BUFFER_EVENT_MOVE)) {
	    timestamp = ecore_time_get();

	    timestamp -= event_info->timestamp;
	    if (timestamp > DYNAMICBOX_CONF_EVENT_FILTER) {
		DbgPrint("Dropped %lf\n", timestamp);
		return 0;
	    }
	}

	timestamp = event_info->timestamp * 1000.0f;
    }

    /**
     * @note
     * Feed up events
     */
    switch (event_info->type) {
    case DBOX_BUFFER_EVENT_ON_HOLD:
	flags = evas_event_default_flags_get(info->e);
	flags |= EVAS_EVENT_FLAG_ON_HOLD;
	evas_event_default_flags_set(info->e, flags);
	ErrPrint("ON_HOLD[%s] %dx%d - %lf\n", info->id, event_info->info.pointer.x, event_info->info.pointer.y, timestamp);
	break;
    case DBOX_BUFFER_EVENT_OFF_HOLD:
	flags = evas_event_default_flags_get(info->e);
	flags &= ~EVAS_EVENT_FLAG_ON_HOLD;
	evas_event_default_flags_set(info->e, flags);
	break;
    case DBOX_BUFFER_EVENT_ON_SCROLL:
	flags = evas_event_default_flags_get(info->e);
	flags |= EVAS_EVENT_FLAG_ON_SCROLL;
	evas_event_default_flags_set(info->e, flags);
	break;
    case DBOX_BUFFER_EVENT_OFF_SCROLL:
	flags = evas_event_default_flags_get(info->e);
	flags &= ~EVAS_EVENT_FLAG_ON_SCROLL;
	evas_event_default_flags_set(info->e, flags);
	break;
    case DBOX_BUFFER_EVENT_ENTER:
	evas_event_feed_mouse_in(info->e, timestamp, NULL);
	break;
    case DBOX_BUFFER_EVENT_LEAVE:
	evas_event_feed_mouse_out(info->e, timestamp, NULL);
	break;
    case DBOX_BUFFER_EVENT_DOWN:
	if (info->pressed) {
	    ErrPrint("MOUSE UP is not called\n");
	    ErrPrint("UP[%s] %dx%d - %lf\n", info->id, event_info->info.pointer.x, event_info->info.pointer.y, timestamp);
	    evas_event_feed_mouse_move(info->e, event_info->info.pointer.x, event_info->info.pointer.y, timestamp, NULL);
	    evas_event_feed_mouse_up(info->e, 1, EVAS_BUTTON_NONE, timestamp, NULL);
	}

	/**
	 * @note
	 * Before processing the DOWN event,
	 * Reset the evas event flags regarding ON_HOLD option.
	 * It can be re-enabled while processing down-move-up events.
	 */
	flags = evas_event_default_flags_get(info->e);
	flags &= ~EVAS_EVENT_FLAG_ON_SCROLL;
	flags &= ~EVAS_EVENT_FLAG_ON_HOLD;
	evas_event_default_flags_set(info->e, flags);
	/**
	 * @note
	 * Calculate the event occurred X & Y on the buffer
	 */
	evas_event_feed_mouse_move(info->e, event_info->info.pointer.x, event_info->info.pointer.y, timestamp, NULL);
	evas_event_feed_mouse_down(info->e, 1, EVAS_BUTTON_NONE, timestamp, NULL); /* + 0.2f just for fake event */
	info->pressed = 1;
	ErrPrint("DOWN[%s] %dx%d - %lf\n", info->id, event_info->info.pointer.x, event_info->info.pointer.y, timestamp);
	break;
    case DBOX_BUFFER_EVENT_MOVE:
	/**
	 * @note
	 * Calculate the event occurred X & Y on the buffer
	 */
	evas_event_feed_mouse_move(info->e, event_info->info.pointer.x, event_info->info.pointer.y, timestamp, NULL);
	break;
    case DBOX_BUFFER_EVENT_UP:
	evas_event_feed_mouse_move(info->e, event_info->info.pointer.x, event_info->info.pointer.y, timestamp, NULL);
	evas_event_feed_mouse_up(info->e, 1, EVAS_BUTTON_NONE, timestamp, NULL);
	info->pressed = 0;
	/**
	 * @note
	 * We have to keep the event flags, so we should not clear them from here.
	 * Sometimes, asynchronously callable Callbacks can refer the evas event flags after up event.
	 * so if we reset them from here, those kind of callbacks will fails to do their job properly.
	 */
	ErrPrint("UP[%s] %dx%d - %lf\n", info->id, event_info->info.pointer.x, event_info->info.pointer.y, timestamp);
	break;
    case DBOX_BUFFER_EVENT_ACCESS_HIGHLIGHT:
	parent_elm = ecore_evas_data_get(info->ee, DBOX_WIN_TAG);
	if (!parent_elm) {
	    ret = DBOX_ACCESS_STATUS_ERROR;
	    break;
	}
	memset(&action_info, 0, sizeof(action_info));
	action_type = ELM_ACCESS_ACTION_HIGHLIGHT;
	/**
	 * @note
	 * Calculate the event occurred X & Y on the buffer
	 */
	action_info.x = event_info->info.access.x;
	action_info.y = event_info->info.access.y;
	ret = elm_access_action(parent_elm, action_type, &action_info);
	if (ret == EINA_TRUE) {
	    if (!get_highlighted_object(parent_elm)) {
		ErrPrint("Highlighted object is not found\n");
		ret = DBOX_ACCESS_STATUS_ERROR;
	    } else {
		DbgPrint("Highlighted object is found\n");
		ret = DBOX_ACCESS_STATUS_DONE;
	    }
	} else {
	    ErrPrint("Action error\n");
	    ret = DBOX_ACCESS_STATUS_ERROR;
	}
	break;
    case DBOX_BUFFER_EVENT_ACCESS_HIGHLIGHT_NEXT:
	parent_elm = ecore_evas_data_get(info->ee, DBOX_WIN_TAG);
	if (!parent_elm) {
	    ret = DBOX_ACCESS_STATUS_ERROR;
	    break;
	}
	memset(&action_info, 0, sizeof(action_info));
	action_type = ELM_ACCESS_ACTION_HIGHLIGHT_NEXT;
	action_info.highlight_cycle = EINA_FALSE;
	ret = elm_access_action(parent_elm, action_type, &action_info);
	ret = (ret == EINA_FALSE) ? DBOX_ACCESS_STATUS_LAST : DBOX_ACCESS_STATUS_DONE;
	break;
    case DBOX_BUFFER_EVENT_ACCESS_HIGHLIGHT_PREV:
	parent_elm = ecore_evas_data_get(info->ee, DBOX_WIN_TAG);
	if (!parent_elm) {
	    ret = DBOX_ACCESS_STATUS_ERROR;
	    break;
	}
	memset(&action_info, 0, sizeof(action_info));
	action_type = ELM_ACCESS_ACTION_HIGHLIGHT_PREV;
	action_info.highlight_cycle = EINA_FALSE;
	ret = elm_access_action(parent_elm, action_type, &action_info);
	ret = (ret == EINA_FALSE) ? DBOX_ACCESS_STATUS_FIRST : DBOX_ACCESS_STATUS_DONE;
	break;
    case DBOX_BUFFER_EVENT_ACCESS_ACTIVATE:
	parent_elm = ecore_evas_data_get(info->ee, DBOX_WIN_TAG);
	if (!parent_elm) {
	    ret = DBOX_ACCESS_STATUS_ERROR;
	    break;
	}
	memset(&action_info, 0, sizeof(action_info));
	action_type = ELM_ACCESS_ACTION_ACTIVATE;
	ret = elm_access_action(parent_elm, action_type, &action_info);
	ret = (ret == EINA_FALSE) ? DBOX_ACCESS_STATUS_ERROR : DBOX_ACCESS_STATUS_DONE;
	break;
    case DBOX_BUFFER_EVENT_ACCESS_ACTION_UP:
	parent_elm = ecore_evas_data_get(info->ee, DBOX_WIN_TAG);
	if (!parent_elm) {
	    ret = DBOX_ACCESS_STATUS_ERROR;
	    break;
	}
	memset(&action_info, 0, sizeof(action_info));
	action_type = ELM_ACCESS_ACTION_UP;
	ret = elm_access_action(parent_elm, action_type, &action_info);
	ret = (ret == EINA_FALSE) ? DBOX_ACCESS_STATUS_ERROR : DBOX_ACCESS_STATUS_DONE;
	break;
    case DBOX_BUFFER_EVENT_ACCESS_ACTION_DOWN:
	parent_elm = ecore_evas_data_get(info->ee, DBOX_WIN_TAG);
	if (!parent_elm) {
	    ret = DBOX_ACCESS_STATUS_ERROR;
	    break;
	}
	memset(&action_info, 0, sizeof(action_info));
	action_type = ELM_ACCESS_ACTION_DOWN;
	ret = elm_access_action(parent_elm, action_type, &action_info);
	ret = (ret == EINA_FALSE) ? DBOX_ACCESS_STATUS_ERROR : DBOX_ACCESS_STATUS_DONE;
	break;
    case DBOX_BUFFER_EVENT_ACCESS_SCROLL_UP:
	parent_elm = ecore_evas_data_get(info->ee, DBOX_WIN_TAG);
	if (!parent_elm) {
	    ret = DBOX_ACCESS_STATUS_ERROR;
	    break;
	}
	memset(&action_info, 0, sizeof(action_info));
	action_type = ELM_ACCESS_ACTION_SCROLL;
	action_info.x = event_info->info.access.x;
	action_info.y = event_info->info.access.y;
	action_info.mouse_type = event_info->info.access.mouse_type;
	ret = elm_access_action(parent_elm, action_type, &action_info);
	ret = (ret == EINA_FALSE) ? DBOX_ACCESS_STATUS_ERROR : DBOX_ACCESS_STATUS_DONE;
	break;
    case DBOX_BUFFER_EVENT_ACCESS_SCROLL_MOVE:
	parent_elm = ecore_evas_data_get(info->ee, DBOX_WIN_TAG);
	if (!parent_elm) {
	    ret = DBOX_ACCESS_STATUS_ERROR;
	    break;
	}
	memset(&action_info, 0, sizeof(action_info));
	action_type = ELM_ACCESS_ACTION_SCROLL;
	action_info.x = event_info->info.access.x;
	action_info.y = event_info->info.access.y;
	action_info.mouse_type = event_info->info.access.mouse_type;
	ret = elm_access_action(parent_elm, action_type, &action_info);
	ret = (ret == EINA_FALSE) ? DBOX_ACCESS_STATUS_ERROR : DBOX_ACCESS_STATUS_DONE;
	break;
    case DBOX_BUFFER_EVENT_ACCESS_SCROLL_DOWN:
	parent_elm = ecore_evas_data_get(info->ee, DBOX_WIN_TAG);
	if (!parent_elm) {
	    ret = DBOX_ACCESS_STATUS_ERROR;
	    break;
	}
	memset(&action_info, 0, sizeof(action_info));
	action_type = ELM_ACCESS_ACTION_SCROLL;
	action_info.x = event_info->info.access.x;
	action_info.y = event_info->info.access.y;
	action_info.mouse_type = event_info->info.access.mouse_type;
	ret = elm_access_action(parent_elm, action_type, &action_info);
	ret = (ret == EINA_FALSE) ? DBOX_ACCESS_STATUS_ERROR : DBOX_ACCESS_STATUS_DONE;
	break;
    case DBOX_BUFFER_EVENT_ACCESS_UNHIGHLIGHT:
	parent_elm = ecore_evas_data_get(info->ee, DBOX_WIN_TAG);
	if (!parent_elm) {
	    ret = DBOX_ACCESS_STATUS_ERROR;
	    break;
	}
	memset(&action_info, 0, sizeof(action_info));
	action_type = ELM_ACCESS_ACTION_UNHIGHLIGHT;
	ret = elm_access_action(parent_elm, action_type, &action_info);
	ret = (ret == EINA_FALSE) ? DBOX_ACCESS_STATUS_ERROR : DBOX_ACCESS_STATUS_DONE;
	break;
    case DBOX_BUFFER_EVENT_ACCESS_VALUE_CHANGE:
	parent_elm = ecore_evas_data_get(info->ee, DBOX_WIN_TAG);
	if (!parent_elm) {
	    ret = DBOX_ACCESS_STATUS_ERROR;
	    break;
	}
	memset(&action_info, 0, sizeof(action_info));
	action_type = ELM_ACCESS_ACTION_VALUE_CHANGE;
	action_info.x = event_info->info.access.x;
	action_info.y = event_info->info.access.y;
	action_info.mouse_type = event_info->info.access.mouse_type;
	ret = elm_access_action(parent_elm, action_type, &action_info);
	ret = (ret == EINA_FALSE) ? DBOX_ACCESS_STATUS_ERROR : DBOX_ACCESS_STATUS_DONE;
	break;
    case DBOX_BUFFER_EVENT_ACCESS_MOUSE:
	parent_elm = ecore_evas_data_get(info->ee, DBOX_WIN_TAG);
	if (!parent_elm) {
	    ret = DBOX_ACCESS_STATUS_ERROR;
	    break;
	}
	memset(&action_info, 0, sizeof(action_info));
	action_type = ELM_ACCESS_ACTION_MOUSE;
	action_info.x = event_info->info.access.x;
	action_info.y = event_info->info.access.y;
	action_info.mouse_type = event_info->info.access.mouse_type;
	ret = elm_access_action(parent_elm, action_type, &action_info);
	ret = (ret == EINA_FALSE) ? DBOX_ACCESS_STATUS_ERROR : DBOX_ACCESS_STATUS_DONE;
	break;
    case DBOX_BUFFER_EVENT_ACCESS_BACK:
	parent_elm = ecore_evas_data_get(info->ee, DBOX_WIN_TAG);
	if (!parent_elm) {
	    ret = DBOX_ACCESS_STATUS_ERROR;
	    break;
	}
	memset(&action_info, 0, sizeof(action_info));
	action_type = ELM_ACCESS_ACTION_BACK;
	action_info.x = event_info->info.access.x;
	action_info.y = event_info->info.access.y;
	action_info.mouse_type = event_info->info.access.mouse_type;
	ret = elm_access_action(parent_elm, action_type, &action_info);
	ret = (ret == EINA_FALSE) ? DBOX_ACCESS_STATUS_ERROR : DBOX_ACCESS_STATUS_DONE;
	break;
    case DBOX_BUFFER_EVENT_ACCESS_OVER:
	parent_elm = ecore_evas_data_get(info->ee, DBOX_WIN_TAG);
	if (!parent_elm) {
	    ret = DBOX_ACCESS_STATUS_ERROR;
	    break;
	}
	memset(&action_info, 0, sizeof(action_info));
	action_type = ELM_ACCESS_ACTION_OVER;
	action_info.x = event_info->info.access.x;
	action_info.y = event_info->info.access.y;
	action_info.mouse_type = event_info->info.access.mouse_type;
	ret = elm_access_action(parent_elm, action_type, &action_info);
	ret = (ret == EINA_FALSE) ? DBOX_ACCESS_STATUS_ERROR : DBOX_ACCESS_STATUS_DONE;
	break;
    case DBOX_BUFFER_EVENT_ACCESS_READ:
	parent_elm = ecore_evas_data_get(info->ee, DBOX_WIN_TAG);
	if (!parent_elm) {
	    ret = DBOX_ACCESS_STATUS_ERROR;
	    break;
	}
	memset(&action_info, 0, sizeof(action_info));
	action_type = ELM_ACCESS_ACTION_READ;
	action_info.x = event_info->info.access.x;
	action_info.y = event_info->info.access.y;
	action_info.mouse_type = event_info->info.access.mouse_type;
	ret = elm_access_action(parent_elm, action_type, &action_info);
	ret = (ret == EINA_FALSE) ? DBOX_ACCESS_STATUS_ERROR : DBOX_ACCESS_STATUS_DONE;
	break;
    case DBOX_BUFFER_EVENT_ACCESS_ENABLE:
	parent_elm = ecore_evas_data_get(info->ee, DBOX_WIN_TAG);
	if (!parent_elm) {
	    ret = DBOX_ACCESS_STATUS_ERROR;
	    break;
	}
	memset(&action_info, 0, sizeof(action_info));
	action_type = ELM_ACCESS_ACTION_ENABLE;
	action_info.x = event_info->info.access.x;
	action_info.y = event_info->info.access.y;
	action_info.mouse_type = event_info->info.access.mouse_type;
	ret = elm_access_action(parent_elm, action_type, &action_info);
	ret = (ret == EINA_FALSE) ? DBOX_ACCESS_STATUS_ERROR : DBOX_ACCESS_STATUS_DONE;
	break;
    case DBOX_BUFFER_EVENT_ACCESS_DISABLE:
	parent_elm = ecore_evas_data_get(info->ee, DBOX_WIN_TAG);
	if (!parent_elm) {
	    ret = DBOX_ACCESS_STATUS_ERROR;
	    break;
	}
	memset(&action_info, 0, sizeof(action_info));
	action_type = ELM_ACCESS_ACTION_DISABLE;
	action_info.x = event_info->info.access.x;
	action_info.y = event_info->info.access.y;
	action_info.mouse_type = event_info->info.access.mouse_type;
	ret = elm_access_action(parent_elm, action_type, &action_info);
	ret = (ret == EINA_FALSE) ? DBOX_ACCESS_STATUS_ERROR : DBOX_ACCESS_STATUS_DONE;
	break;
    case DBOX_BUFFER_EVENT_KEY_DOWN:
	parent_elm = ecore_evas_data_get(info->ee, DBOX_WIN_TAG);
	if (!parent_elm) {
	    ret = DBOX_ACCESS_STATUS_ERROR;
	    break;
	}

	key_symbol = XGetKeyboardMapping(ecore_x_display_get(), event_info->info.keycode, 1, &ret);
	if (key_symbol) {
	    char *key_name;
	    char *key_string;

	    key_string = XKeysymToString(*key_symbol);
	    key_name = XKeysymToString(*key_symbol);
	    DbgPrint("Key symbol: %s, name: %s\n", key_string, key_name);
	    XFree(key_symbol);
	    XFree(key_name);
	    XFree(key_string);
	}
	ret = DBOX_KEY_STATUS_ERROR;
	break;
    case DBOX_BUFFER_EVENT_KEY_UP:
	parent_elm = ecore_evas_data_get(info->ee, DBOX_WIN_TAG);
	if (!parent_elm) {
	    ret = DBOX_ACCESS_STATUS_ERROR;
	    break;
	}

	key_symbol = XGetKeyboardMapping(ecore_x_display_get(), event_info->info.keycode, 1, &ret);
	if (key_symbol) {
	    char *key_name;
	    char *key_string;

	    key_string = XKeysymToString(*key_symbol);
	    key_name = XKeysymToString(*key_symbol);
	    DbgPrint("Key symbol: %s, name: %s\n", key_string, key_name);
	    XFree(key_symbol);
	    XFree(key_name);
	    XFree(key_string);
	}
	ret = DBOX_KEY_STATUS_ERROR;
	break;
    case DBOX_BUFFER_EVENT_KEY_FOCUS_IN:
	parent_elm = ecore_evas_data_get(info->ee, DBOX_WIN_TAG);
	if (!parent_elm) {
	    ret = DBOX_ACCESS_STATUS_ERROR;
	    break;
	}

	key_symbol = XGetKeyboardMapping(ecore_x_display_get(), event_info->info.keycode, 1, &ret);
	if (key_symbol) {
	    char *key_name;
	    char *key_string;

	    key_string = XKeysymToString(*key_symbol);
	    key_name = XKeysymToString(*key_symbol);
	    DbgPrint("Key symbol: %s, name: %s\n", key_string, key_name);
	    XFree(key_symbol);
	    XFree(key_name);
	    XFree(key_string);
	}
	ret = DBOX_KEY_STATUS_ERROR;
	break;
    case DBOX_BUFFER_EVENT_KEY_FOCUS_OUT:
	parent_elm = ecore_evas_data_get(info->ee, DBOX_WIN_TAG);
	if (!parent_elm) {
	    ret = DBOX_ACCESS_STATUS_ERROR;
	    break;
	}

	key_symbol = XGetKeyboardMapping(ecore_x_display_get(), event_info->info.keycode, 1, &ret);
	if (key_symbol) {
	    char *key_name;
	    char *key_string;

	    key_string = XKeysymToString(*key_symbol);
	    key_name = XKeysymToString(*key_symbol);
	    DbgPrint("Key symbol: %s, name: %s\n", key_string, key_name);
	    XFree(key_symbol);
	    XFree(key_name);
	    XFree(key_string);
	}
	ret = DBOX_KEY_STATUS_ERROR;
	break;
    default:
	DbgPrint("Unhandled buffer event (%d)\n", event_info->type);
	break;
    }

    return ret;
}

/**
 * @note
 * This callback can be called twice (or more) to get a several pixmaps
 * Acquired pixmaps are used for double/tripple buffering for canvas
 */
static Ecore_X_Pixmap alloc_pixmap_cb(void *data, Ecore_X_Window parent, int w, int h, int depth)
{
    vwin_info_t info = data;
    Ecore_X_Pixmap pixmap;

    if (!info->handle) {
	ErrPrint("Invalid handle\n");
	return 0u;
    }

    info->w = w;
    info->h = h;
    DbgPrint("Size of ee is updated: %dx%d - %d (info: %p)\n", info->w, info->h, depth, info);
    depth >>= 3;

    if (dynamicbox_resource_id(info->handle, DBOX_PRIMARY_BUFFER) == 0u) {
	/**
	 * @note
	 * Need to allocate a primary buffer
	 */
	dynamicbox_acquire_buffer(info->handle, DBOX_PRIMARY_BUFFER, info->w, info->h, depth);
	if (!info->handle) {
	    ErrPrint("Failed to get the buffer\n");
	    return 0u;
	}

	pixmap = (Ecore_X_Pixmap)dynamicbox_resource_id(info->handle, DBOX_PRIMARY_BUFFER);
    } else if (DYNAMICBOX_CONF_EXTRA_BUFFER_COUNT > 0) {
	int idx;

	if (!info->resource_array) {
	    info->resource_array = calloc(DYNAMICBOX_CONF_EXTRA_BUFFER_COUNT, sizeof(*info->resource_array));
	    if (!info->resource_array) {
		ErrPrint("Out of memory: %s\n", strerror(errno));
		return 0u;
	    }

	    idx = 0;
	} else {
	    for (idx = 0; idx < DYNAMICBOX_CONF_EXTRA_BUFFER_COUNT; idx++) {
		if (info->resource_array[idx] == 0u) {
		    break;
		}
	    }

	    if (idx == DYNAMICBOX_CONF_EXTRA_BUFFER_COUNT) {
		ErrPrint("Out of index: %d\n", idx);
		return 0u;
	    }
	}

	if (dynamicbox_acquire_buffer(info->handle, idx, info->w, info->h, depth) < 0) {
	    ErrPrint("Failed to acquire a buffer for %d\n", idx);
	    return 0u;
	}

	info->resource_array[idx] = dynamicbox_resource_id(info->handle, idx);
	if (info->resource_array[idx] == 0u) {
	    ErrPrint("Failed to allocate pixmap\n");
	}

	DbgPrint("Allocated index: %d/%d - %u\n", idx, DYNAMICBOX_CONF_EXTRA_BUFFER_COUNT, info->resource_array[idx]);
	pixmap = info->resource_array[idx];
    } else {
	ErrPrint("Unable to allocate pixmap\n");
	pixmap = 0u;
    }

    /**
     * Acquire a buffer for canvas.
     */
    info->type = VWIN_PIXMAP;
    info->resource_cnt += !!(unsigned int)pixmap;
    return pixmap;
}

static void free_pixmap_cb(void *data, Ecore_X_Pixmap pixmap)
{
    vwin_info_t info = data;

    if (!info->handle) {
	return;
    }

    if (info->type != VWIN_PIXMAP) {
	ErrPrint("Impossible\n");
    }

    if (dynamicbox_resource_id(info->handle, DBOX_PRIMARY_BUFFER) == pixmap) {
	if (dynamicbox_release_buffer(info->handle, DBOX_PRIMARY_BUFFER) < 0) {
	    DbgPrint("Failed to release buffer\n");
	}
	info->resource_cnt--;
    } else {
	int idx;

	for (idx = 0; idx < DYNAMICBOX_CONF_EXTRA_BUFFER_COUNT; idx++) {
	    /**
	     * @note
	     * Find a index to release it
	     */
	    if (info->resource_array[idx] == pixmap) {
		if (dynamicbox_release_buffer(info->handle, idx) < 0) {
		    DbgPrint("Failed to release buffer\n");
		}
		info->resource_array[idx] = 0u;
		info->resource_cnt--;
		break;
	    }
	}
    }

    if (info->deleted && info->resource_cnt == 0) {
	DbgPrint("Destroy buffer handle\n");

	dynamicbox_destroy_buffer(info->handle);
	free(info->resource_array);
	free(info->id);
	free(info);
    }
}

static void *alloc_fb(void *data, int size)
{
    vwin_info_t info = data;
    void *buffer;

    if (info->ee) {
	ecore_evas_geometry_get(info->ee, NULL, NULL, &info->w, &info->h);
	DbgPrint("Size of ee is updated: %dx%d (info: %p)\n", info->w, info->h, info);
    }

    if (!info->handle) {
	ErrPrint("Failed to create a buffer\n");
	return NULL;
    }

    if (dynamicbox_acquire_buffer(info->handle, DBOX_PRIMARY_BUFFER, info->w, info->h, sizeof(int)) < 0) {
	ErrPrint("Failed to acquire buffer\n");
	return NULL;
    }

    /**
     * If it supports the H/W accelerated buffer,
     * Use it.
     */
    if (dynamicbox_support_hw_buffer(info->handle)) {
	if (dynamicbox_create_hw_buffer(info->handle) == 0) {
	    buffer = dynamicbox_buffer_hw_buffer(info->handle);
	    if (buffer) {
		DbgPrint("HW Accelerated buffer is created %p, (%dx%d)\n", info, info->w, info->h);
		info->type = VWIN_GEM;
		return buffer;
	    }
	}

	ErrPrint("Failed to allocate HW Accelerated buffer\n");
    }

    /**
     * Or use the buffer of a S/W backend.
     */
    buffer = dynamicbox_ref_buffer(info->handle);
    DbgPrint("SW buffer is created (%dx%d)\n", info->w, info->h);
    info->type = VWIN_SW_BUF;
    return buffer;
}

static void *alloc_stride_fb(void *data, int size, int *stride, int *bpp)
{
    void *buffer;

    buffer = alloc_fb(data, size);
    if (buffer) {
	vwin_info_t info = data;
	int _stride;

	*bpp = sizeof(int);
	_stride = dynamicbox_buffer_stride(info->handle);
	if (_stride < 0) {
	    _stride = info->w * *bpp;
	}

	*stride = _stride;
	*bpp <<= 3;
	DbgPrint("bpp: %d, stride: %d\n", *bpp, *stride);
    }

    return buffer;
}

static void free_fb(void *data, void *ptr)
{
    vwin_info_t info = data;

    if (!info->handle) {
	return;
    }

    if (info->type == VWIN_GEM) {
	if (dynamicbox_destroy_hw_buffer(info->handle) == 0) {
	    DbgPrint("HW Accelerated buffer is destroyed\n");
	}
    } else if (info->type == VWIN_SW_BUF) {
	DbgPrint("SW buffer is destroyed, %p\n", info);
	dynamicbox_unref_buffer(ptr);
    } else if (info->type == VWIN_PIXMAP) {
	ErrPrint("Unable to reach to here\n");
    }

    if (dynamicbox_release_buffer(info->handle, DBOX_PRIMARY_BUFFER) < 0) {
	ErrPrint("Failed to release buffer\n");
    }

    if (info->deleted) {
	dynamicbox_destroy_buffer(info->handle);
	free(info->resource_array);
	free(info->id);
	free(info);
    }
}

static void pre_render_cb(void *data, Evas *e, void *event_info)
{
    vwin_info_t info = data;

    if (!info->handle) {
	return;
    }

    if (dynamicbox_conf_premultiplied_alpha()) {
	Evas_Coord w;
	Evas_Coord h;

	ecore_evas_geometry_get(info->ee, NULL, NULL, &w, &h);
	evas_damage_rectangle_add(e, 0, 0, w, h);
    }

    if (info->type == VWIN_GEM) {
	dynamicbox_buffer_pre_render(info->handle);
    } else if (info->type == VWIN_PIXMAP) {
	/**
	 * Only the pixmap type Ecore_Evas uses this variable
	 */
    } else if (info->type == VWIN_SW_BUF) {
	/* Do nothing */
    }
}

static void post_render_cb(void *data, Evas *e, void *event_info)
{
    vwin_info_t info = data;

    if (!info->handle) {
	return;
    }

    if (dynamicbox_conf_premultiplied_alpha()) {
	void *canvas;
	int x, y, w, h;

	// Get a pointer of a buffer of the virtual canvas
	canvas = (void *)ecore_evas_buffer_pixels_get(info->ee);
	if (!canvas) {
	    ErrPrint("Failed to get pixel canvas\n");
	    return;
	}

	ecore_evas_geometry_get(info->ee, &x, &y, &w, &h);
	evas_data_argb_unpremul(canvas, w * h);
    }

    if (info->type == VWIN_GEM) {
	dynamicbox_buffer_post_render(info->handle);
    } else if (info->type == VWIN_PIXMAP) {
	int idx;
	unsigned int front_resource_id;

	front_resource_id = ecore_evas_gl_x11_pixmap_get(info->ee);

	for (idx = 0; idx < DYNAMICBOX_CONF_EXTRA_BUFFER_COUNT; idx++) {
	    if (front_resource_id == info->resource_array[idx]) {
		/**
		 */
		dynamicbox_send_updated_by_idx(info->handle, idx);
		break;
	    }
	}

	if (idx == DYNAMICBOX_CONF_EXTRA_BUFFER_COUNT) {
	    /* Send updated event for PRIMARY BUFFER */
	    if (front_resource_id == dynamicbox_resource_id(info->handle, DBOX_PRIMARY_BUFFER)) {
		dynamicbox_send_updated_by_idx(info->handle, DBOX_PRIMARY_BUFFER);
	    } else {
		DbgPrint("Unable to send updated: %u (%u)\n", front_resource_id, dynamicbox_resource_id(info->handle, DBOX_PRIMARY_BUFFER));
	    }
	}
    } else if (info->type == VWIN_SW_BUF) {
	dynamicbox_sync_buffer(info->handle);
    }
}

static void ecore_evas_free_cb(Ecore_Evas *ee)
{
    vwin_info_t info;

    info = ecore_evas_data_get(ee, "dynamic,box,info");
    if (!info) {
	DbgPrint("Info is not valid\n");
	return;
    }

    if (info->e) {
	evas_event_callback_del(info->e, EVAS_CALLBACK_RENDER_POST, post_render_cb);
	evas_event_callback_del(info->e, EVAS_CALLBACK_RENDER_PRE, pre_render_cb);
    }

    info->deleted = 1;
    info->ee = NULL;
}

PUBLIC void *dynamicbox_get_evas_object(const char *id, int is_gbar)
{
    vwin_info_t info;
    Evas_Object *rect;
    const char *engine;

    if (!s_info.alloc_canvas && !s_info.alloc_canvas_with_stride && !s_info.alloc_canvas_with_pixmap) {
	s_info.alloc_canvas_with_pixmap = dlsym(RTLD_DEFAULT, "ecore_evas_gl_x11_pixmap_allocfunc_new");
	if (!s_info.alloc_canvas_with_pixmap) {
	    DbgPrint("pixmap_allocfunc_new is not found\n");
	}

	s_info.alloc_canvas_with_stride = dlsym(RTLD_DEFAULT, "ecore_evas_buffer_allocfunc_with_stride_new");
	if (!s_info.alloc_canvas_with_stride) {
	    DbgPrint("allocfunc_with_stirde_new is not found\n");
	}

	s_info.alloc_canvas = dlsym(RTLD_DEFAULT, "ecore_evas_buffer_allocfunc_new");
	if (!s_info.alloc_canvas) {
	    ErrPrint("allocfunc_new is not found\n");
	}

	if (!s_info.alloc_canvas_with_stride && !s_info.alloc_canvas && !s_info.alloc_canvas_with_pixmap) {
	    ErrPrint("No way to allocate canvas\n");
	    return NULL;
	}
    }

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

    info->is_gbar = is_gbar;

    /**
     * Acquire a buffer for canvas.
     */
    info->handle = dynamicbox_create_buffer(info->id, info->is_gbar,
	    (dynamicbox_conf_auto_align() || !s_info.alloc_canvas_with_stride),
	    event_handler_cb, info);

    if (!info->handle) {
	ErrPrint("Failed to create a dynamicbox buffer\n");
	free(info->id);
	free(info);
	return NULL;
    }

    /**
     * Size information must be initialized before call the ecore_evas_buffer_new.
     */
    info->w = DBOX_DEFAULT_WIDTH;
    info->h = DBOX_DEFAULT_HEIGHT;

    engine = elm_config_preferred_engine_get();
    DbgPrint("Preferred engine: %s (%s)\n", engine, GL_ENGINE);
    if (engine && !strcmp(engine, GL_ENGINE)) {
	if (s_info.alloc_canvas_with_pixmap) {
	    info->ee = s_info.alloc_canvas_with_pixmap(NULL, 0u, 0, 0, info->w, info->h, alloc_pixmap_cb, free_pixmap_cb, info);
	    if (!info->ee) {
		ErrPrint("Unable to create a ee for pixmap\n");
	    }
	}
    }

    if (!info->ee) {
	if (!dynamicbox_conf_auto_align() && s_info.alloc_canvas_with_stride) {
	    info->ee = s_info.alloc_canvas_with_stride(info->w, info->h, alloc_stride_fb, free_fb, info);
	} else if (s_info.alloc_canvas) {
	    info->ee = s_info.alloc_canvas(info->w, info->h, alloc_fb, free_fb, info);
	}
    }

    if (!info->ee) {
	ErrPrint("Failed to create ecore_evas (%dx%d)\n", info->w, info->h);
	dynamicbox_destroy_buffer(info->handle);
	free(info->id);
	free(info);
	return NULL;
    }

    ecore_evas_data_set(info->ee, "dynamic,box,info", info);

    /**
     * @note
     * Free callback must be prepared before use the ecore_evas_free()
     */
    ecore_evas_callback_pre_free_set(info->ee, ecore_evas_free_cb);

    info->e = ecore_evas_get(info->ee);
    if (!info->e) {
	ErrPrint("Failed to get evas\n");
	ecore_evas_free(info->ee);
	return NULL;
    }

    pre_render_cb(info, NULL, NULL);
    ecore_evas_alpha_set(info->ee, EINA_TRUE);
    post_render_cb(info, NULL, NULL);

    ecore_evas_manual_render_set(info->ee, EINA_FALSE);
    ecore_evas_resize(info->ee, info->w, info->h);

    evas_event_callback_add(info->e, EVAS_CALLBACK_RENDER_POST, post_render_cb, info);
    evas_event_callback_add(info->e, EVAS_CALLBACK_RENDER_PRE, pre_render_cb, info);

    rect = evas_object_rectangle_add(info->e);
    if (!rect) {
	ErrPrint("Failed to create evas_object\n");
	ecore_evas_free(info->ee);
	return NULL;
    }

    evas_object_resize(rect, info->w, info->h);
    evas_object_color_set(rect, 0, 0, 0, 0);
    return rect;
}

/* End of a file */
