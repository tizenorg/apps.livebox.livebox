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

#include <Elementary.h>
#include <Ecore_Evas.h>
#include <Evas.h>
#include <Ecore.h>
#include <unistd.h>

#include <dlog.h>
#include <dynamicbox_service.h>
#include <dynamicbox_errno.h>

#include "dynamicbox.h"
#include "internal/dynamicbox.h"
#include "debug.h"

#define QUALITY_N_COMPRESS "quality=100 compress=1"
#define PUBLIC __attribute__((visibility("default")))

struct snapshot_info {
    char *id;
    dynamicbox_flush_cb flush_cb;
    void *data;

    Ecore_Timer *flush_timer;

    int render_cnt;
    double timeout;
};

static void post_render_cb(void *data, Evas *e, void *event_info);
static void pre_render_cb(void *data, Evas *e, void *event_info);

static inline Evas *create_virtual_canvas(int w, int h)
{
    Ecore_Evas *internal_ee;
    Evas *internal_e;

    // Create virtual canvas
    internal_ee = ecore_evas_buffer_new(w, h);
    if (!internal_ee) {
	DbgPrint("Failed to create a new canvas buffer\n");
	return NULL;
    }

    ecore_evas_alpha_set(internal_ee, EINA_TRUE);
    ecore_evas_manual_render_set(internal_ee, EINA_FALSE);

    // Get the "Evas" object from a virtual canvas
    internal_e = ecore_evas_get(internal_ee);
    if (!internal_e) {
	ecore_evas_free(internal_ee);
	DbgPrint("Faield to get Evas object\n");
	return NULL;
    }

    return internal_e;
}

static inline int flush_data_to_file(Evas *e, const char *data, const char *filename, int w, int h)
{
    Evas_Object *output;

    output = evas_object_image_add(e);
    if (!output) {
	DbgPrint("Failed to create an image object\n");
	return DBOX_STATUS_ERROR_FAULT;
    }

    evas_object_image_data_set(output, NULL);
    evas_object_image_colorspace_set(output, EVAS_COLORSPACE_ARGB8888);
    evas_object_image_alpha_set(output, EINA_TRUE);
    evas_object_image_size_set(output, w, h);
    evas_object_image_smooth_scale_set(output, EINA_TRUE);
    evas_object_image_data_set(output, (void *)data);
    evas_object_image_fill_set(output, 0, 0, w, h);
    evas_object_image_data_update_add(output, 0, 0, w, h);

    if (evas_object_image_save(output, filename, NULL, QUALITY_N_COMPRESS) == EINA_FALSE) {
	evas_object_del(output);
	DbgPrint("Faield to save a captured image (%s)\n", filename);
	return DBOX_STATUS_ERROR_IO_ERROR;
    }

    evas_object_del(output);

    if (access(filename, F_OK) != 0) {
	DbgPrint("File %s is not found\n", filename);
	return DBOX_STATUS_ERROR_IO_ERROR;
    }

    DbgPrint("Flush data to a file (%s)\n", filename);
    return DBOX_STATUS_ERROR_NONE;
}

static inline int destroy_virtual_canvas(Evas *e)
{
    Ecore_Evas *ee;

    ee = ecore_evas_ecore_evas_get(e);
    if (!ee) {
	DbgPrint("Failed to ecore evas object\n");
	return DBOX_STATUS_ERROR_FAULT;
    }

    ecore_evas_free(ee);
    return DBOX_STATUS_ERROR_NONE;
}

static inline int flush_to_file(const void *canvas, const char *filename, int w, int h)
{
    int status;
    Evas *shot_e;
    Ecore_Evas *shot_ee;

    shot_e = create_virtual_canvas(w, h);
    if (!shot_e) {
	ErrPrint("Unable to create a new virtual window\n");
	return DBOX_STATUS_ERROR_FAULT;
    }

    shot_ee = ecore_evas_ecore_evas_get(shot_e);
    if (!shot_ee) {
	ErrPrint("Unable to get Ecore_Evas\n");
	destroy_virtual_canvas(shot_e);
	return DBOX_STATUS_ERROR_FAULT;
    }

    ecore_evas_manual_render_set(shot_ee, EINA_TRUE);

    status = flush_data_to_file(shot_e, canvas, filename, w, h);
    destroy_virtual_canvas(shot_e);

    return status;
}

static Eina_Bool snapshot_cb(void *data)
{
    Evas_Object *snapshot_win = data;
    struct snapshot_info *info;
    Evas *e;
    Ecore_Evas *ee;
    void *canvas;
    dynamicbox_flush_cb flush_cb;
    int status = DBOX_STATUS_ERROR_NONE;

    info = evas_object_data_get(snapshot_win, "snapshot,info");
    if (!info) {
	ErrPrint("Invalid object\n");
	return ECORE_CALLBACK_CANCEL;
    }

    if (info->flush_timer) {
	info->flush_timer = NULL;
    } else {
	status = DBOX_STATUS_ERROR_CANCEL;
    }

    flush_cb = info->flush_cb;
    info->flush_cb = NULL; /* To prevent call this from the delete callback */

    e = evas_object_evas_get(snapshot_win);
    if (!e) {
	ErrPrint("Invalid object\n");
	if (flush_cb) {
	    flush_cb(snapshot_win, info->id, DBOX_STATUS_ERROR_FAULT, info->data);
	}
	return ECORE_CALLBACK_CANCEL;
    }

    ee = ecore_evas_ecore_evas_get(e);
    if (!ee) {
	ErrPrint("Invalid object (ee)\n");
	if (flush_cb) {
	    flush_cb(snapshot_win, info->id, DBOX_STATUS_ERROR_FAULT, info->data);
	}
	return ECORE_CALLBACK_CANCEL;
    }

    canvas = (void*)ecore_evas_buffer_pixels_get(ee);
    if (!canvas) {
	DbgPrint("Failed to get pixel canvas\n");
	if (flush_cb) {
	    flush_cb(snapshot_win, info->id, DBOX_STATUS_ERROR_FAULT, info->data);
	}
	return ECORE_CALLBACK_CANCEL;
    }

    if (flush_cb) {
	int w;
	int h;
	int ret;

	ecore_evas_geometry_get(ee, NULL, NULL, &w, &h);

	DbgPrint("Flush size: %dx%d\n", w, h);
	ret = flush_to_file(canvas, info->id, w, h);
	if (status == DBOX_STATUS_ERROR_NONE) {
	    status = ret;
	}

	flush_cb(snapshot_win, info->id, status, info->data);
	/**
	 * Do not access info after this.
	 */
    }

    return ECORE_CALLBACK_CANCEL;
}

static void del_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
    struct snapshot_info *info;

    info = evas_object_data_del(obj, "snapshot,info");
    if (!info) {
	return;
    }

    DbgPrint("Delete object (%s)\n", info->id);

    evas_event_callback_del(e, EVAS_CALLBACK_RENDER_POST, post_render_cb);
    evas_event_callback_del(e, EVAS_CALLBACK_RENDER_PRE, pre_render_cb);

    if (info->flush_timer) {
	ecore_timer_del(info->flush_timer);
	info->flush_timer = NULL;

	(void)snapshot_cb(obj);
	DbgPrint("Flush is canceled\n");
    }

    /**
     * @note
     * Render callback will be deleted.
     */
    free(info->id);
    free(info);
}

static Eina_Bool direct_snapshot_cb(void *data)
{
    Evas *e;
    Ecore_Evas *ee;
    const void *canvas;
    int status;
    int w;
    int h;
    dynamicbox_flush_cb flush_cb;
    Evas_Object *snapshot_win = data;
    struct snapshot_info *info;

    info = evas_object_data_get(snapshot_win, "snapshot,info");
    if (!info) {
	ErrPrint("Unable to get snapshot info\n");
	return ECORE_CALLBACK_CANCEL;
    }

    info->flush_timer = NULL;
    flush_cb = info->flush_cb;
    info->flush_cb = NULL; /* To prevent call this from the delete callback */

    e = evas_object_evas_get(snapshot_win);
    if (!e) {
	ErrPrint("Invalid object, failed to get Evas\n");
	if (flush_cb) {
	    flush_cb(snapshot_win, info->id, DBOX_STATUS_ERROR_FAULT, info->data);
	}
	return ECORE_CALLBACK_CANCEL;
    }

    ee = ecore_evas_ecore_evas_get(e);
    if (!ee) {
	ErrPrint("Unable to get Ecore_Evas object\n");
	if (flush_cb) {
	    flush_cb(snapshot_win, info->id, DBOX_STATUS_ERROR_FAULT, info->data);
	}
	return ECORE_CALLBACK_CANCEL;
    }

    ecore_evas_geometry_get(ee, NULL, NULL, &w, &h);
    ecore_evas_manual_render_set(ee, EINA_TRUE);

    canvas = ecore_evas_buffer_pixels_get(ee);
    if (!canvas) {
	ErrPrint("Failed to get canvas\n");
	if (flush_cb) {
	    flush_cb(snapshot_win, info->id, DBOX_STATUS_ERROR_FAULT, info->data);
	}
	return ECORE_CALLBACK_CANCEL;
    }

    status = flush_to_file(canvas, info->id, w, h);
    if (flush_cb) {
	flush_cb(snapshot_win, info->id, status, info->data);
    }
    return ECORE_CALLBACK_CANCEL;
}

static void post_render_cb(void *data, Evas *e, void *event_info)
{
    Evas_Object *snapshot_win = data;
    struct snapshot_info *info;

    info = evas_object_data_get(snapshot_win, "snapshot,info");
    if (!info) {
	ErrPrint("snapshot info is not valid\n");
	return;
    }

    info->render_cnt++;

    if (info->flush_timer) {
	/**
	 * @note
	 * This has not to be happens.
	 */
	ErrPrint("Flush timer is not cleared\n");
	ecore_timer_del(info->flush_timer);
	info->flush_timer = NULL;
    }

    if (!info->flush_cb) {
	DbgPrint("Flush request is not initiated yet\n");
	return;
    }

    /*!
     * \NOTE
     * Even if tehre is no timer registered, we should capture the content
     * from out of this callback.
     * Or we can met unexpected problems.
     * To avoid it, use the 0.0001f to get timer callback ASAP, not in this function.
     */
    DbgPrint("Fire the flush timer %lf (%d)\n", info->timeout, info->render_cnt);
    info->flush_timer = ecore_timer_add(info->timeout, snapshot_cb, snapshot_win);
    if (!info->flush_timer) {
	dynamicbox_flush_cb flush_cb;

	ErrPrint("Unalbe to add timer for getting the snapshot\n");
	flush_cb = info->flush_cb;
	info->flush_cb = NULL;

	if (flush_cb) {
	    flush_cb(snapshot_win, info->id, DBOX_STATUS_ERROR_FAULT, info->data);
	}
	/*!
	 * \note
	 * Do not access info after from here.
	 */
    }
}

static void pre_render_cb(void *data, Evas *e, void *event_info)
{
    Evas_Object *snapshot_win = data;
    struct snapshot_info *info;

    info = evas_object_data_get(snapshot_win, "snapshot,info");
    if (!info) {
	ErrPrint("snapshot info is not valid\n");
	return;
    }

    if (info->flush_timer) {
	ecore_timer_del(info->flush_timer);
	info->flush_timer = NULL;
	DbgPrint("Clear the flush timer\n");
    }

    DbgPrint("Pre-render callback\n");
}

PUBLIC void *dynamicbox_snapshot_window_add(const char *id, int size_type)
{
    struct snapshot_info *info;
    Evas_Object *snapshot_win;
    Evas_Object *parent;
    Evas *e;
    int w;
    int h;

    if (dynamicbox_service_get_size(size_type, &w, &h) != DBOX_STATUS_ERROR_NONE) {
	ErrPrint("Invalid size\n");
	return NULL;
    }

    info = malloc(sizeof(*info));
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

    info->flush_cb = NULL;
    info->data = NULL;
    info->flush_timer = NULL;
    info->render_cnt = 0;

    e = create_virtual_canvas(w, h);
    if (!e) {
	free(info->id);
	free(info);
	return NULL;
    }

    parent = evas_object_rectangle_add(e);
    if (!parent) {
	destroy_virtual_canvas(e);
	free(info->id);
	free(info);
	return NULL;
    }

    snapshot_win = elm_win_add(parent, "DBox,Snapshot", ELM_WIN_DYNAMIC_BOX);
    evas_object_del(parent);
    if (!snapshot_win) {
	destroy_virtual_canvas(e);
	free(info->id);
	free(info);
	return NULL;
    }

    DbgPrint("Create a new window %dx%d for %s\n", w, h, id);
    evas_object_event_callback_add(snapshot_win, EVAS_CALLBACK_DEL, del_cb, NULL);
    evas_object_resize(snapshot_win, w, h);
    evas_object_show(snapshot_win);

    evas_object_data_set(snapshot_win, "snapshot,info", info);

    evas_event_callback_add(e, EVAS_CALLBACK_RENDER_POST, post_render_cb, snapshot_win);
    evas_event_callback_add(e, EVAS_CALLBACK_RENDER_PRE, pre_render_cb, snapshot_win);

    return snapshot_win;
}

PUBLIC int dynamicbox_snapshot_window_flush(void *snapshot_win, double timeout, dynamicbox_flush_cb flush_cb, void *data)
{
    struct snapshot_info *info;

    if (!flush_cb || timeout < 0.0f) {
	ErrPrint("Invalid argument (%p, %lf)\n", flush_cb, timeout);
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    info = evas_object_data_get(snapshot_win, "snapshot,info");
    if (!info) {
	ErrPrint("Invalid argument\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    info->timeout = timeout;

    if (timeout == 0.0f) {
	/*!
	 * This timer is just used for guarantees same behavious even if it flushes content directly,
	 * The callback should be called from next loop.
	 * or the developer will get confused
	 */
	info->flush_timer = ecore_timer_add(0.0001f, direct_snapshot_cb, snapshot_win);
	if (!info->flush_timer) {
	    return DBOX_STATUS_ERROR_FAULT;
	}
    } else if (info->render_cnt) {
	/*!
	 * Try to watit pre-render callback.
	 * If there is rendered contents.
	 */
	DbgPrint("Rendered %d times already\n", info->render_cnt);
	info->flush_timer = ecore_timer_add(info->timeout, snapshot_cb, snapshot_win);
	if (!info->flush_timer) {
	    return DBOX_STATUS_ERROR_FAULT;
	}
    }

    info->flush_cb = flush_cb;
    info->data = data;

    return DBOX_STATUS_ERROR_NONE;
}

/* End of a file */
