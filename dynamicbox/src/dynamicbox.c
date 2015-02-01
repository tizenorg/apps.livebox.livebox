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

#include <stdio.h>
#include <errno.h>
#include <stdlib.h> /* malloc */
#include <string.h> /* strdup */
#include <libgen.h>
#include <unistd.h> /* access */
#define __USE_GNU
#include <dlfcn.h>

#include <dlog.h>
#include <dynamicbox_errno.h>
#include <dynamicbox_service.h>
#include <dynamicbox_provider.h>
#include <dynamicbox_provider_buffer.h>
#include <dynamicbox_conf.h>
#include <dynamicbox_buffer.h>
#include <Evas.h>
#include <Ecore.h>

#include "debug.h"
#include "dlist.h"
#include "util.h"
#include "dynamicbox.h"
#include "internal/dynamicbox.h"

#define PUBLIC __attribute__((visibility("default")))

#define FILE_SCHEMA    "file://"

/**
 * @note
 * This value is configurable value.
 */
#define FRAME_WAIT_INTERVAL (1.0f/6.0f)

struct block {
    unsigned int idx;

    char *type;
    char *part;
    char *data;
    char *option;
    char *id;
    char *file;
    char *target_id;
};

struct dynamicbox_desc {
    FILE *fp;
    int for_pd;

    unsigned int last_idx;

    struct dlist *block_list;
};

struct dynamicbox_buffer_data {
    int is_gbar;
    int accelerated;

    /* for Buffer event wrapper */
    int (*handler)(dynamicbox_buffer_h , dynamicbox_buffer_event_data_t, void *);
    void *cbdata;

    char *content;
    char *title;
    char *icon;
    char *name;

    Ecore_Timer *frame_wait_timer;
};

/**
 * @brief These functions are defined in the data-provider-slave
 */
static struct info {
    const char *(*find_pkgname)(const char *filename);
    int (*request_update_by_id)(const char *uri);
    int (*trigger_update_monitor)(const char *id, int is_gbar);
    int (*update_extra_info)(const char *id, const char *content, const char *title, const char *icon, const char *name);

    enum load_type {
	LOAD_TYPE_UNKNOWN = -1,
	LOAD_TYPE_SLAVE   = 0,
	LOAD_TYPE_APP     = 1
    } type;

    union _updated {
	struct _slave {
	    int (*send)(const char *pkgname, const char *id, dynamicbox_buffer_h handle, int idx, int x, int y, int w, int h, int gbar, const char *descfile);
	} slave;

	struct _app {
	    int (*send)(dynamicbox_buffer_h handle, int idx, int x, int y, int w, int h, int gbar);
	} app;
    } updated;
} s_info = {
    .find_pkgname = NULL,
    .request_update_by_id = NULL,
    .trigger_update_monitor = NULL,
    .update_extra_info = NULL,
    .type = LOAD_TYPE_UNKNOWN,    /* Not initialized */
};

#define FUNC_PREFIX                               "dynamicbox_"
#define FUNC_DYNAMICBOX_SEND_UPDATED              FUNC_PREFIX "send_buffer_updated"
#define FUNC_DYNAMICBOX_PROVIDER_APP_UPDATED      FUNC_PREFIX "provider_app_buffer_updated"
#define FUNC_DYNAMICBOX_FIND_PKGNAME              FUNC_PREFIX "find_pkgname"
#define FUNC_DYNAMICBOX_REQUEST_UPDATE_BY_ID      FUNC_PREFIX "request_update_by_id"
#define FUNC_DYNAMICBOX_TRIGGER_UPDATE_MONITOR    FUNC_PREFIX "trigger_update_monitor"
#define FUNC_DYNAMICBOX_UPDATE_EXTRA_INFO         FUNC_PREFIX "update_extra_info"

static inline void load_update_function(void)
{
    /* Must to be checked the slave function first. */
    s_info.updated.slave.send = dlsym(RTLD_DEFAULT, FUNC_DYNAMICBOX_SEND_UPDATED);
    if (s_info.updated.slave.send) {
	s_info.type = LOAD_TYPE_SLAVE;
	DbgPrint("Slave detected\n");
    } else {
	s_info.updated.app.send = dlsym(RTLD_DEFAULT, FUNC_DYNAMICBOX_PROVIDER_APP_UPDATED);
	if (s_info.updated.app.send) {
	    s_info.type = LOAD_TYPE_APP;
	    DbgPrint("App detected\n");
	}
    }
}

static int send_updated(const char *pkgname, const char *id, dynamicbox_buffer_h handle, int idx, int x, int y, int w, int h, int gbar, const char *descfile)
{
    int ret = DBOX_STATUS_ERROR_INVALID_PARAMETER;

    if (s_info.type == LOAD_TYPE_UNKNOWN) {
	load_update_function();
    }

    if (s_info.type == LOAD_TYPE_APP) {
	ret = s_info.updated.app.send(handle, idx, x, y, w, h, gbar);
    } else if (s_info.type == LOAD_TYPE_SLAVE) {
	/**
	 * pkgname, id are used for finding handle of direct connection.
	 */
	ret = s_info.updated.slave.send(pkgname, id, handle, idx, x, y, w, h, gbar, descfile);
    } else {
	dynamicbox_damage_region_t region = {
	    .x = x,
	    .y = y,
	    .w = w,
	    .h = h,
	};
	ret = dynamicbox_provider_send_buffer_updated(handle, idx, &region, gbar, descfile);
    }

    return ret;
}

static char *id_to_uri(const char *id)
{
    char *uri;
    int uri_len;

    uri_len = strlen(id) + strlen(FILE_SCHEMA) + 1;

    uri = malloc(uri_len);
    if (!uri) {
	return NULL;
    }

    snprintf(uri, uri_len, FILE_SCHEMA "%s", id);
    return uri;
}

static inline int event_handler_wrapper(dynamicbox_buffer_h buffer, dynamicbox_buffer_event_data_t event_info, void *data)
{
    const char *pkgname;
    const char *id;
    struct dynamicbox_buffer_data *cbdata = data;
    int ret;

    pkgname = dynamicbox_provider_buffer_pkgname(buffer);
    if (!pkgname) {
	ErrPrint("pkgname is not valid\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    id = dynamicbox_provider_buffer_id(buffer);
    if (!id) {
	ErrPrint("id is not valid[%s]\n", pkgname);
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (cbdata->handler) {
	ret = cbdata->handler(buffer, event_info, cbdata->cbdata);
    } else {
	ret = DBOX_STATUS_ERROR_FAULT;
    }

    switch (event_info->type) {
	case DBOX_BUFFER_EVENT_ACCESS_HIGHLIGHT:
	case DBOX_BUFFER_EVENT_ACCESS_HIGHLIGHT_NEXT:
	case DBOX_BUFFER_EVENT_ACCESS_HIGHLIGHT_PREV:
	case DBOX_BUFFER_EVENT_ACCESS_ACTIVATE:
	case DBOX_BUFFER_EVENT_ACCESS_ACTION_UP:
	case DBOX_BUFFER_EVENT_ACCESS_ACTION_DOWN:
	case DBOX_BUFFER_EVENT_ACCESS_SCROLL_UP:
	case DBOX_BUFFER_EVENT_ACCESS_SCROLL_MOVE:
	case DBOX_BUFFER_EVENT_ACCESS_SCROLL_DOWN:
	case DBOX_BUFFER_EVENT_ACCESS_UNHIGHLIGHT:
	case DBOX_BUFFER_EVENT_ACCESS_VALUE_CHANGE:
	case DBOX_BUFFER_EVENT_ACCESS_MOUSE:
	case DBOX_BUFFER_EVENT_ACCESS_BACK:
	case DBOX_BUFFER_EVENT_ACCESS_OVER:
	case DBOX_BUFFER_EVENT_ACCESS_READ:
	case DBOX_BUFFER_EVENT_ACCESS_ENABLE:
	case DBOX_BUFFER_EVENT_ACCESS_DISABLE:
	    DbgPrint("Accessibility event: %d\n", event_info->type);
	    if (ret < 0) {
		(void)dynamicbox_provider_send_access_status(pkgname, id, DBOX_ACCESS_STATUS_ERROR);
	    } else {
		(void)dynamicbox_provider_send_access_status(pkgname, id, ret);
	    }
	    break;
	case DBOX_BUFFER_EVENT_KEY_UP:
	case DBOX_BUFFER_EVENT_KEY_DOWN:
	case DBOX_BUFFER_EVENT_KEY_FOCUS_IN:
	case DBOX_BUFFER_EVENT_KEY_FOCUS_OUT:
	    DbgPrint("Key event: %d\n", event_info->type);
	    if (ret < 0) {
		(void)dynamicbox_provider_send_key_status(pkgname, id, DBOX_KEY_STATUS_ERROR);
	    } else {
		(void)dynamicbox_provider_send_key_status(pkgname, id, ret);
	    }
	    break;
	default:
	    break;
    }

    return ret;
}

static inline int default_event_handler(dynamicbox_buffer_h buffer, dynamicbox_buffer_event_data_t event_info, void *data)
{
    /* NOP */
    return 0;
}

PUBLIC const int DBOX_DONE = 0x00;
PUBLIC const int DBOX_OUTPUT_UPDATED = 0x02;
PUBLIC const int DBOX_USE_NET = 0x04;

PUBLIC const int DBOX_NEED_TO_SCHEDULE = 0x01;
PUBLIC const int DBOX_NEED_TO_CREATE = 0x01;
PUBLIC const int DBOX_NEED_TO_DESTROY = 0x01;
PUBLIC const int DBOX_FORCE_TO_SCHEDULE = 0x08;

PUBLIC const int DBOX_SYS_EVENT_FONT_CHANGED = 0x01;
PUBLIC const int DBOX_SYS_EVENT_LANG_CHANGED = 0x02;
PUBLIC const int DBOX_SYS_EVENT_TIME_CHANGED = 0x04;
PUBLIC const int DBOX_SYS_EVENT_REGION_CHANGED = 0x08;
PUBLIC const int DBOX_SYS_EVENT_TTS_CHANGED = 0x10;
PUBLIC const int DBOX_SYS_EVENT_PAUSED = 0x0100;
PUBLIC const int DBOX_SYS_EVENT_RESUMED = 0x0200;
PUBLIC const int DBOX_SYS_EVENT_MMC_STATUS_CHANGED = 0x0400;
PUBLIC const int DBOX_SYS_EVENT_DELETED = 0x0800;

PUBLIC struct dynamicbox_desc *dynamicbox_desc_open(const char *filename, int for_pd)
{
    struct dynamicbox_desc *handle;
    char *new_fname;

    handle = calloc(1, sizeof(*handle));
    if (!handle) {
	ErrPrint("Error: %s\n", strerror(errno));
	return NULL;
    }

    if (for_pd) {
	int len;
	len = strlen(filename) + strlen(".desc") + 1;
	new_fname = malloc(len);
	if (!new_fname) {
	    ErrPrint("Error: %s\n", strerror(errno));
	    free(handle);
	    return NULL;
	}
	snprintf(new_fname, len, "%s.desc", filename);
    } else {
	new_fname = strdup(filename);
	if (!new_fname) {
	    ErrPrint("Error: %s\n", strerror(errno));
	    free(handle);
	    return NULL;
	}
    }

    DbgPrint("Open a file %s with merge mode %s\n",
	    new_fname,
	    access(new_fname, F_OK) == 0 ? "enabled" : "disabled");

    handle->fp = fopen(new_fname, "at");
    free(new_fname);
    if (!handle->fp) {
	ErrPrint("Failed to open a file: %s\n", strerror(errno));
	free(handle);
	return NULL;
    }

    return handle;
}

PUBLIC int dynamicbox_desc_close(struct dynamicbox_desc *handle)
{
    struct dlist *l;
    struct dlist *n;
    struct block *block;

    if (!handle) {
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    dlist_foreach_safe(handle->block_list, l, n, block) {
	handle->block_list = dlist_remove(handle->block_list, l);

	fprintf(handle->fp, "{\n");
	if (block->type) {
	    fprintf(handle->fp, "type=%s\n", block->type);
	}

	if (block->part) {
	    fprintf(handle->fp, "part=%s\n", block->part);
	}

	if (block->data) {
	    fprintf(handle->fp, "data=%s\n", block->data);
	}

	if (block->option) {
	    fprintf(handle->fp, "option=%s\n", block->option);
	}

	if (block->id) {
	    fprintf(handle->fp, "id=%s\n", block->id);
	}

	if (block->target_id) {
	    fprintf(handle->fp, "target=%s\n", block->target_id);
	}
	fprintf(handle->fp, "}\n");

	free(block->type);
	free(block->part);
	free(block->data);
	free(block->option);
	free(block->id);
	free(block->target_id);
	free(block);
    }

    if (fclose(handle->fp) != 0) {
	ErrPrint("fclose: %s\n", strerror(errno));
    }
    free(handle);
    return DBOX_STATUS_ERROR_NONE;
}

PUBLIC int dynamicbox_desc_set_category(struct dynamicbox_desc *handle, const char *id, const char *category)
{
    struct block *block;

    if (!handle || !category) {
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    block = calloc(1, sizeof(*block));
    if (!block) {
	return DBOX_STATUS_ERROR_OUT_OF_MEMORY;
    }

    block->type = strdup(DBOX_DESC_TYPE_INFO);
    if (!block->type) {
	free(block);
	return DBOX_STATUS_ERROR_OUT_OF_MEMORY;
    }

    block->part = strdup("category");
    if (!block->part) {
	free(block->type);
	free(block);
	return DBOX_STATUS_ERROR_OUT_OF_MEMORY;
    }

    block->data = strdup(category);
    if (!block->data) {
	free(block->type);
	free(block->part);
	free(block);
	return DBOX_STATUS_ERROR_OUT_OF_MEMORY;
    }

    if (id) {
	block->id = strdup(id);
	if (!block->id) {
	    free(block->data);
	    free(block->type);
	    free(block->part);
	    free(block);
	    return DBOX_STATUS_ERROR_OUT_OF_MEMORY;
	}
    }

    block->idx = handle->last_idx++;
    handle->block_list = dlist_append(handle->block_list, block);
    return block->idx;
}

PUBLIC int dynamicbox_desc_set_size(struct dynamicbox_desc *handle, const char *id, int w, int h)
{
    struct block *block;
    char buffer[BUFSIZ];

    if (!handle) {
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    block = calloc(1, sizeof(*block));
    if (!block) {
	return DBOX_STATUS_ERROR_OUT_OF_MEMORY;
    }

    block->type = strdup(DBOX_DESC_TYPE_INFO);
    if (!block->type) {
	free(block);
	return DBOX_STATUS_ERROR_OUT_OF_MEMORY;
    }

    block->part = strdup("size");
    if (!block->part) {
	free(block->type);
	free(block);
	return DBOX_STATUS_ERROR_OUT_OF_MEMORY;
    }

    if (id) {
	block->id = strdup(id);
	if (!block->id) {
	    free(block->part);
	    free(block->type);
	    free(block);
	    return DBOX_STATUS_ERROR_OUT_OF_MEMORY;
	}
    }

    snprintf(buffer, sizeof(buffer), "%dx%d", w, h);
    block->data = strdup(buffer);
    if (!block->data) {
	free(block->id);
	free(block->part);
	free(block->type);
	free(block);
	return DBOX_STATUS_ERROR_OUT_OF_MEMORY;
    }

    block->idx = handle->last_idx++;
    handle->block_list = dlist_append(handle->block_list, block);
    return block->idx;
}

PUBLIC int dynamicbox_desc_set_id(struct dynamicbox_desc *handle, int idx, const char *id)
{
    struct dlist *l;
    struct block *block;

    dlist_foreach(handle->block_list, l, block) {
	if (block->idx == idx) {
	    if (strcasecmp(block->type, DBOX_DESC_TYPE_SCRIPT)) {
		ErrPrint("Invalid block is used\n");
		return DBOX_STATUS_ERROR_INVALID_PARAMETER;
	    }

	    free(block->target_id);
	    block->target_id = NULL;

	    if (!id || !strlen(id)) {
		return DBOX_STATUS_ERROR_NONE;
	    }

	    block->target_id = strdup(id);
	    if (!block->target_id) {
		ErrPrint("Heap: %s\n", strerror(errno));
		return DBOX_STATUS_ERROR_OUT_OF_MEMORY;
	    }

	    return DBOX_STATUS_ERROR_NONE;
	}
    }

    return DBOX_STATUS_ERROR_NOT_EXIST;
}

/*!
 * \return idx
 */
PUBLIC int dynamicbox_desc_add_block(struct dynamicbox_desc *handle, const char *id, const char *type, const char *part, const char *data, const char *option)
{
    struct block *block;

    if (!handle || !type) {
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!part) {
	part = "";
    }

    if (!data) {
	data = "";
    }

    block = calloc(1, sizeof(*block));
    if (!block) {
	ErrPrint("Heap: %s\n", strerror(errno));
	return DBOX_STATUS_ERROR_OUT_OF_MEMORY;
    }

    block->type = strdup(type);
    if (!block->type) {
	ErrPrint("Heap: %s\n", strerror(errno));
	free(block);
	return DBOX_STATUS_ERROR_OUT_OF_MEMORY;
    }

    block->part = strdup(part);
    if (!block->part) {
	ErrPrint("Heap: %s\n", strerror(errno));
	free(block->type);
	free(block);
	return DBOX_STATUS_ERROR_OUT_OF_MEMORY;
    }

    block->data = strdup(data);
    if (!block->data) {
	ErrPrint("Heap: %s\n", strerror(errno));
	free(block->type);
	free(block->part);
	free(block);
	return DBOX_STATUS_ERROR_OUT_OF_MEMORY;
    }

    if (option) {
	block->option = strdup(option);
	if (!block->option) {
	    ErrPrint("Heap: %s\n", strerror(errno));
	    free(block->data);
	    free(block->type);
	    free(block->part);
	    free(block);
	    return DBOX_STATUS_ERROR_OUT_OF_MEMORY;
	}
    }

    if (id) {
	block->id = strdup(id);
	if (!block->id) {
	    ErrPrint("Heap: %s\n", strerror(errno));
	    free(block->option);
	    free(block->data);
	    free(block->type);
	    free(block->part);
	    free(block);
	    return DBOX_STATUS_ERROR_OUT_OF_MEMORY;
	}
    }

    block->idx = handle->last_idx++;
    handle->block_list = dlist_append(handle->block_list, block);
    return block->idx;
}

PUBLIC int dynamicbox_desc_del_block(struct dynamicbox_desc *handle, int idx)
{
    struct dlist *l;
    struct block *block;

    if (!handle || idx < 0) {
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    dlist_foreach(handle->block_list, l, block) {
	if (block->idx == idx) {
	    handle->block_list = dlist_remove(handle->block_list, l);
	    free(block->type);
	    free(block->part);
	    free(block->data);
	    free(block->option);
	    free(block->id);
	    free(block->target_id);
	    free(block);
	    return DBOX_STATUS_ERROR_NONE;
	}
    }

    return DBOX_STATUS_ERROR_NOT_EXIST;
}

PUBLIC int dynamicbox_acquire_buffer(dynamicbox_buffer_h handle, int idx, int width, int height, int pixel_size)
{
    int ret;

    if (!handle || width <= 0 || height <= 0 || pixel_size <= 0 || idx > DYNAMICBOX_CONF_EXTRA_BUFFER_COUNT) {
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (idx >= 0) {
	int _width = 0;
	int _height = 0;
	int _pixel_size = 0;
	/* To validate the buffer */
	if (dynamicbox_provider_buffer_get_size(handle, &_width, &_height, &_pixel_size) < 0) {
	    ErrPrint("Failed to get buffer size\n");
	}

	if (_width != width || _height != height || pixel_size != _pixel_size) {
	    DbgPrint("Extra buffer's geometry is not matched with primary one\n");
	}

	ret = dynamicbox_provider_buffer_extra_acquire(handle, idx, width, height, pixel_size);
    } else {
	ret = dynamicbox_provider_buffer_acquire(handle, width, height, pixel_size);
    }

    return ret;
}

PUBLIC dynamicbox_buffer_h dynamicbox_create_buffer(const char *filename, int is_gbar, int auto_align, int (*handler)(dynamicbox_buffer_h , dynamicbox_buffer_event_data_t, void *), void *data)
{
    struct dynamicbox_buffer_data *user_data;
    const char *pkgname;
    dynamicbox_buffer_h handle;
    char *uri;

    if (!filename) {
	ErrPrint("Invalid argument: %p(%dx%d)\n", filename);
	return NULL;
    }

    user_data = calloc(1, sizeof(*user_data));
    if (!user_data) {
	ErrPrint("Heap: %s\n", strerror(errno));
	return NULL;
    }

    user_data->is_gbar = is_gbar;
    user_data->handler = handler ? handler : default_event_handler;
    user_data->cbdata = data;

    uri = id_to_uri(filename);
    if (!uri) {
	ErrPrint("Heap: %s\n", strerror(errno));
	free(user_data);
	return NULL;
    }

    if (!s_info.find_pkgname) {
	s_info.find_pkgname = dlsym(RTLD_DEFAULT, FUNC_DYNAMICBOX_FIND_PKGNAME);
	if (!s_info.find_pkgname) {
	    ErrPrint("Failed to find a \"dynamicbox_find_pkgname\"\n");
	    free(user_data);
	    free(uri);
	    return NULL;
	}
    }

    pkgname = s_info.find_pkgname(uri);
    if (!pkgname) {
	ErrPrint("Invalid Request\n");
	free(user_data);
	free(uri);
	return NULL;
    }

    handle = dynamicbox_provider_buffer_create((!!is_gbar) ? DBOX_TYPE_GBAR : DBOX_TYPE_DBOX, pkgname, uri, auto_align, event_handler_wrapper, user_data);
    free(uri);
    if (!handle) {
	free(user_data);
	return NULL;
    }

    (void)dynamicbox_provider_buffer_set_user_data(handle, user_data);
    return handle;
}

PUBLIC int dynamicbox_request_update(const char *filename)
{
    char *uri;
    int ret;

    if (!filename) {
	ErrPrint("Invalid argument\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    uri = id_to_uri(filename);
    if (!uri) {
	ErrPrint("Heap: %s\n", strerror(errno));
	return DBOX_STATUS_ERROR_OUT_OF_MEMORY;
    }

    if (!s_info.request_update_by_id) {
	s_info.request_update_by_id = dlsym(RTLD_DEFAULT, FUNC_DYNAMICBOX_REQUEST_UPDATE_BY_ID);
	if (!s_info.request_update_by_id) {
	    ErrPrint("\"dynamicbox_request_update_by_id\" is not exists\n");
	    free(uri);
	    return DBOX_STATUS_ERROR_FAULT;
	}
    }

    ret = s_info.request_update_by_id(uri);
    free(uri);
    return ret;
}

PUBLIC unsigned int dynamicbox_resource_id(dynamicbox_buffer_h handle, int idx)
{
    int ret;

    if (!handle || idx > DYNAMICBOX_CONF_EXTRA_BUFFER_COUNT) {
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (idx >= 0) {
	ret = dynamicbox_provider_buffer_extra_resource_id(handle, idx);
    } else {
	ret = dynamicbox_provider_buffer_resource_id(handle);
    }

    return ret;
}

PUBLIC int dynamicbox_release_buffer(dynamicbox_buffer_h handle, int idx)
{
    int ret;

    if (!handle || idx > DYNAMICBOX_CONF_EXTRA_BUFFER_COUNT) {
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (idx >= 0) {
	ret = dynamicbox_provider_buffer_extra_release(handle, idx);
    } else {
	ret = dynamicbox_provider_buffer_release(handle);
    }

    DbgPrint("Release buffer: %d (%d)\n", idx, ret);
    return ret;
}

PUBLIC int dynamicbox_destroy_buffer(dynamicbox_buffer_h handle)
{
    struct dynamicbox_buffer_data *user_data;

    if (!handle) {
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    user_data = dynamicbox_provider_buffer_user_data(handle);
    if (user_data) {
	if (user_data->frame_wait_timer) {
	    ecore_timer_del(user_data->frame_wait_timer);
	    user_data->frame_wait_timer = NULL;
	}
	free(user_data->content);
	free(user_data->title);
	free(user_data->icon);
	free(user_data->name);
	free(user_data);
	dynamicbox_provider_buffer_set_user_data(handle, NULL);
    }

    DbgPrint("Destroy buffer\n");
    return dynamicbox_provider_buffer_destroy(handle);
}

PUBLIC void *dynamicbox_ref_buffer(dynamicbox_buffer_h handle)
{
    struct dynamicbox_buffer_data *user_data;
    void *data;
    int w, h, size;
    int ret;

    if (!handle) {
	return NULL;
    }

    user_data = dynamicbox_provider_buffer_user_data(handle);
    if (!user_data) {
	return NULL;
    }

    if (user_data->accelerated) {
	DbgPrint("H/W accelerated buffer is allocated\n");
	return NULL;
    }

    ret = dynamicbox_provider_buffer_get_size(handle, &w, &h, &size);

    data = dynamicbox_provider_buffer_ref(handle);
    if (data && !ret && w > 0 && h > 0 && size > 0) {
	memset(data, 0, w * h * size);
	(void)dynamicbox_provider_buffer_sync(handle);
    }

    DbgPrint("Ref buffer %ds%d(%d)\n", w, h, size);
    return data;
}

PUBLIC int dynamicbox_unref_buffer(void *buffer)
{
    if (!buffer) {
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    DbgPrint("Unref buffer\n");
    return dynamicbox_provider_buffer_unref(buffer);
}

PUBLIC int dynamicbox_sync_buffer(dynamicbox_buffer_h handle)
{
    struct dynamicbox_buffer_data *user_data;
    const char *pkgname;
    const char *id;
    int w;
    int h;
    int pixel_size;
    int ret;

    if (!handle) {
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    user_data = dynamicbox_provider_buffer_user_data(handle);
    if (!user_data) {
	ErrPrint("Invalid buffer\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (user_data->accelerated) {
	DbgPrint("H/W Buffer allocated. skip the sync buffer\n");
	return DBOX_STATUS_ERROR_NONE;
    }

    pkgname = dynamicbox_provider_buffer_pkgname(handle);
    if (!pkgname) {
	ErrPrint("Invalid buffer handler\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    id = dynamicbox_provider_buffer_id(handle);
    if (!id) {
	ErrPrint("Invalid buffer handler\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    (void)dynamicbox_provider_buffer_sync(handle);

    if (dynamicbox_provider_buffer_get_size(handle, &w, &h, &pixel_size) < 0) {
	ErrPrint("Failed to get size (%s)\n", id);
    }

    /**
     * @todo
     * manipulate the damaged region, so send update event only for the damaged region.
     * to make more efficient updates
     */
    ret = send_updated(pkgname, id, handle, DBOX_PRIMARY_BUFFER, 0, 0, w, h, user_data->is_gbar, NULL);
    if (ret < 0) {
	ErrPrint("Failed to send%supdated (%s)\n", user_data->is_gbar ? " GBAR " : " ", id);
    }

    return ret;
}

PUBLIC int dynamicbox_send_updated_by_idx(dynamicbox_buffer_h handle, int idx)
{
    struct dynamicbox_buffer_data *user_data;
    const char *pkgname;
    const char *id;
    int w;
    int h;
    int pixel_size;
    int ret;

    user_data = dynamicbox_provider_buffer_user_data(handle);
    if (!user_data) {
	ErrPrint("Invalid buffer\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    pkgname = dynamicbox_provider_buffer_pkgname(handle);
    if (!pkgname) {
	ErrPrint("Invalid buffer handler\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    id = dynamicbox_provider_buffer_id(handle);
    if (!id) {
	ErrPrint("Invalid buffer handler\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (dynamicbox_provider_buffer_get_size(handle, &w, &h, &pixel_size) < 0) {
	ErrPrint("Failed to get size (%s)\n", id);
    }

    ret = send_updated(pkgname, id, handle, idx, 0, 0, w, h, user_data->is_gbar, NULL);
    if (ret < 0) {
	ErrPrint("Failed to send%supdated (%s) %d\n", user_data->is_gbar ? " GBAR " : " ", id, idx);
    }

    return ret;
}

PUBLIC int dynamicbox_support_hw_buffer(dynamicbox_buffer_h handle)
{
    if (!handle) {
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    return dynamicbox_provider_buffer_is_support_hw(handle);
}

PUBLIC int dynamicbox_create_hw_buffer(dynamicbox_buffer_h handle)
{
    struct dynamicbox_buffer_data *user_data;
    int ret;

    if (!handle) {
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    user_data = dynamicbox_provider_buffer_user_data(handle);
    if (!user_data) {
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (user_data->accelerated) {
	return DBOX_STATUS_ERROR_ALREADY;
    }

    ret = dynamicbox_provider_buffer_create_hw(handle);
    user_data->accelerated = (ret == 0);
    return ret;
}

PUBLIC int dynamicbox_destroy_hw_buffer(dynamicbox_buffer_h handle)
{
    struct dynamicbox_buffer_data *user_data;

    if (!handle) {
	LOGD("handle is NULL\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    user_data = dynamicbox_provider_buffer_user_data(handle);
    if (!user_data || !user_data->accelerated) {
	LOGD("user_data is NULL\n");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    user_data->accelerated = 0;

    return dynamicbox_provider_buffer_destroy_hw(handle);
}

PUBLIC void *dynamicbox_buffer_hw_buffer(dynamicbox_buffer_h handle)
{
    struct dynamicbox_buffer_data *user_data;

    if (!handle) {
	return NULL;
    }

    user_data = dynamicbox_provider_buffer_user_data(handle);
    if (!user_data || !user_data->accelerated) {
	return NULL;
    }

    return dynamicbox_provider_buffer_hw_addr(handle);
}

PUBLIC int dynamicbox_buffer_pre_render(dynamicbox_buffer_h handle)
{
    struct dynamicbox_buffer_data *user_data;

    if (!handle) {
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    user_data = dynamicbox_provider_buffer_user_data(handle);
    if (!user_data) {
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!user_data->accelerated) {
	return DBOX_STATUS_ERROR_NONE;
    }

    /*!
     * \note
     * Do preprocessing for accessing the H/W render buffer
     */
    return dynamicbox_provider_buffer_pre_render(handle);
}

static Eina_Bool frame_wait_cb(void *data)
{
    dynamicbox_buffer_h handle = data;
    struct dynamicbox_buffer_data *user_data;
    const char *pkgname;
    const char *id;
    int pixel_size;
    int ret;
    int w;
    int h;

    user_data = dynamicbox_provider_buffer_user_data(handle);
    if (!user_data) {
	ErrPrint("Failed to get a user data\n");
	return ECORE_CALLBACK_CANCEL;
    }

    pkgname = dynamicbox_provider_buffer_pkgname(handle);
    id = dynamicbox_provider_buffer_id(handle);

    if (!pkgname || !id) {
	ErrPrint("Failed to get instance information (%s), (%s)\n", pkgname, id);
	user_data->frame_wait_timer = NULL;
	return ECORE_CALLBACK_CANCEL;
    }

    if (dynamicbox_provider_buffer_get_size(handle, &w, &h, &pixel_size) < 0) {
	ErrPrint("Failed to get size (%s)\n", id);
	w = 0;
	h = 0;
	pixel_size = sizeof(int);
    }

    DbgPrint("Frame skip waiting timer is fired (%s)\n", id);

    /**
     * @todo
     * manipulate the damaged region, so send update event only for the damaged region.
     * to make more efficient updates
     */
    ret = send_updated(pkgname, id, handle, DBOX_PRIMARY_BUFFER, 0, 0, w, h, user_data->is_gbar, NULL);
    if (ret < 0) {
	ErrPrint("Failed to send%supdated (%s)\n", user_data->is_gbar ? " GBAR " : " ", id);
    }

    (void)dynamicbox_provider_buffer_clear_frame_skip(handle);

    user_data->frame_wait_timer = NULL;
    return ECORE_CALLBACK_CANCEL;
}

PUBLIC int dynamicbox_buffer_post_render(dynamicbox_buffer_h handle)
{
    int ret;
    const char *pkgname;
    const char *id;
    struct dynamicbox_buffer_data *user_data;
    int w;
    int h;
    int pixel_size;
    PERF_INIT();
    PERF_BEGIN();

    if (!handle) {
	PERF_MARK("post_render");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    user_data = dynamicbox_provider_buffer_user_data(handle);
    if (!user_data) {
	PERF_MARK("post_render");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!user_data->accelerated) {
	PERF_MARK("post_render");
	return DBOX_STATUS_ERROR_NONE;
    }

    pkgname = dynamicbox_provider_buffer_pkgname(handle);
    if (!pkgname) {
	ErrPrint("Invalid buffer handle\n");
	PERF_MARK("post_render");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    id = dynamicbox_provider_buffer_id(handle);
    if (!id) {
	ErrPrint("Invalid buffer handler\n");
	PERF_MARK("post_render");
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    ret = dynamicbox_provider_buffer_post_render(handle);
    if (ret < 0) {
	ErrPrint("Failed to post render processing\n");
	PERF_MARK("post_render");
	return ret;
    }

    if (dynamicbox_provider_buffer_frame_skip(handle) == 0) {
	if (dynamicbox_provider_buffer_get_size(handle, &w, &h, &pixel_size) < 0) {
	    ErrPrint("Failed to get size (%s)\n", id);
	}

	/**
	 * @todo
	 * manipulate the damaged region, so send update event only for the damaged region.
	 * to make more efficient updates
	 */
	ret = send_updated(pkgname, id, handle, DBOX_PRIMARY_BUFFER, 0, 0, w, h, user_data->is_gbar, NULL);
	if (ret < 0) {
	    ErrPrint("Failed to send%supdated (%s)\n", user_data->is_gbar ? " GBAR " : " ", id);
	}

	if (user_data->frame_wait_timer) {
	    ecore_timer_del(user_data->frame_wait_timer);
	    user_data->frame_wait_timer = NULL;
	}
    } else {
	if (user_data->frame_wait_timer) {
	    ecore_timer_reset(user_data->frame_wait_timer);
	} else {
	    user_data->frame_wait_timer = ecore_timer_add(FRAME_WAIT_INTERVAL, frame_wait_cb, handle);
	    if (!user_data->frame_wait_timer) {
		ErrPrint("Failed to add waiting timer for next frame\n");
	    }
	}
    }

    PERF_MARK("post_render");
    return ret;
}

PUBLIC int dynamicbox_buffer_stride(dynamicbox_buffer_h handle)
{
    if (!handle) {
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    return dynamicbox_provider_buffer_stride(handle);
}

PUBLIC int dynamicbox_content_is_updated(const char *filename, int is_gbar)
{
    if (!s_info.trigger_update_monitor) {
	s_info.trigger_update_monitor = dlsym(RTLD_DEFAULT, FUNC_DYNAMICBOX_TRIGGER_UPDATE_MONITOR);
	if (!s_info.trigger_update_monitor) {
	    ErrPrint("Trigger update monitor function is not exists\n");
	    return DBOX_STATUS_ERROR_FAULT;
	}
    }

    return s_info.trigger_update_monitor(filename, is_gbar);
}

PUBLIC int dynamicbox_request_close_glance_bar(const char *pkgname, const char *id, int reason)
{
    char *uri;
    int schema_len = strlen(FILE_SCHEMA);
    int ret;

    if (!pkgname || !id) {
	ErrPrint("Invalid parameters (%s) (%s)\n", pkgname, id);
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (strncmp(id, FILE_SCHEMA, schema_len)) {
	uri = id_to_uri(id);
	if (!uri) {
	    ErrPrint("Heap: %s\n", strerror(errno));
	    return DBOX_STATUS_ERROR_OUT_OF_MEMORY;
	}
    } else {
	uri = strdup(id);
	if (!uri) {
	    ErrPrint("Heap: %s\n", strerror(errno));
	    return DBOX_STATUS_ERROR_OUT_OF_MEMORY;
	}
    }

    ret = dynamicbox_provider_send_request_close_gbar(pkgname, uri, reason);
    free(uri);
    return ret;
}

PUBLIC int dynamicbox_freeze_scroller(const char *pkgname, const char *id)
{
    char *uri;
    int ret;

    uri = id_to_uri(id);
    if (!uri) {
	ErrPrint("Heap: %s\n", strerror(errno));
	return DBOX_STATUS_ERROR_OUT_OF_MEMORY;
    }

    ret = dynamicbox_provider_send_hold_scroll(pkgname, uri, 1);
    free(uri);
    return ret;
}

PUBLIC int dynamicbox_thaw_scroller(const char *pkgname, const char *id)
{
    char *uri;
    int ret;

    uri = id_to_uri(id);
    if (!uri) {
	ErrPrint("Heap: %s\n", strerror(errno));
	return DBOX_STATUS_ERROR_OUT_OF_MEMORY;
    }

    ret = dynamicbox_provider_send_hold_scroll(pkgname, uri, 0);
    free(uri);
    return ret;
}

PUBLIC int dynamicbox_set_extra_info(const char *id, const char *content, const char *title, const char *icon, const char *name)
{
    dynamicbox_buffer_h handle;
    const char *pkgname;
    char *uri;

    uri = id_to_uri(id);
    if (!uri) {
	ErrPrint("Heap: %s\n", strerror(errno));
	return DBOX_STATUS_ERROR_OUT_OF_MEMORY;
    }

    if (!s_info.find_pkgname) {
	s_info.find_pkgname = dlsym(RTLD_DEFAULT, FUNC_DYNAMICBOX_FIND_PKGNAME);
	if (!s_info.find_pkgname) {
	    ErrPrint("Failed to find a \"dynamicbox_find_pkgname\"\n");
	    free(uri);
	    return DBOX_STATUS_ERROR_FAULT;
	}
    }

    pkgname = s_info.find_pkgname(uri);
    if (!pkgname) {
	ErrPrint("Failed to find a package (%s)\n", uri);
	free(uri);
	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
    }

    handle = dynamicbox_provider_buffer_find_buffer(DBOX_TYPE_DBOX, pkgname, uri);
    if (handle) {
	struct dynamicbox_buffer_data *user_data;

	user_data = dynamicbox_provider_buffer_user_data(handle);
	if (!user_data) {
	    ErrPrint("User data is not available\n");
	    free(uri);
	    return DBOX_STATUS_ERROR_FAULT;
	}

	if (content && strlen(content)) {
	    char *_content;

	    _content = strdup(content);
	    if (_content) {
		if (user_data->content) {
		    free(user_data->content);
		}

		user_data->content = _content;
	    } else {
		ErrPrint("Heap: %s\n", strerror(errno));
	    }
	}

	if (title && strlen(title)) {
	    char *_title;

	    _title = strdup(title);
	    if (_title) {
		if (user_data->title) {
		    free(user_data->title);
		}

		user_data->title = _title;
	    } else {
		ErrPrint("Heap: %s\n", strerror(errno));
	    }
	}

	if (icon && strlen(icon)) {
	    char *_icon;

	    _icon = strdup(icon);
	    if (_icon) {
		if (user_data->icon) {
		    free(user_data->icon);
		}

		user_data->icon = _icon;
	    } else {
		ErrPrint("Heap: %s\n", strerror(errno));
	    }
	}

	if (name && strlen(name)) {
	    char *_name;

	    _name = strdup(name);
	    if (_name) {
		if (user_data->name) {
		    free(user_data->name);
		}

		user_data->name = _name;
	    } else {
		ErrPrint("Heap: %s\n", strerror(errno));
	    }
	}

	if (dynamicbox_provider_send_extra_info(pkgname, uri, -1.0f, user_data->content, user_data->title, user_data->icon, user_data->name) < 0) {
	    ErrPrint("Failed to send extra info (%s)\n", id);
	}

	free(uri);
	return DBOX_STATUS_ERROR_NONE;
    }
    free(uri);

    if (!s_info.update_extra_info) {
	s_info.update_extra_info = dlsym(RTLD_DEFAULT, FUNC_DYNAMICBOX_UPDATE_EXTRA_INFO);
	if (!s_info.update_extra_info) {
	    ErrPrint("Failed to find a \"dynamicbox_update_extra_info\"\n");
	    return DBOX_STATUS_ERROR_INVALID_PARAMETER;
	}
    }

    return s_info.update_extra_info(id, content, title, icon, name);
}

/* End of a file */
