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
#include <livebox-service.h>
#include <provider.h>
#include <provider_buffer.h>
#include <livebox-errno.h>

#include "debug.h"
#include "livebox.h"
#include "dlist.h"
#include "util.h"

#define PUBLIC __attribute__((visibility("default")))

#define FILE_SCHEMA	"file://"

/* Must has to sync'd with data-provider-slave */
struct conf {
	int width;
	int height;

	int base_width;
	int base_height;
	double minimum_period;

	struct {
		char *script;
		char *abi;
		char *pd_group;
		double period;
	} default_conf;

	struct {
		char *name;
		char *secured;
		char *abi;
	} launch_key;

	double default_packet_time;

	char *empty_content;
	char *empty_title;

	char *default_content;
	char *default_title;

	unsigned long minimum_space;

	char *replace_tag;

	double slave_ttl;
	double slave_activate_time;
	double slave_relaunch_time;
	int slave_relaunch_count;

	int max_log_line;
	int max_log_file;

	unsigned long sqlite_flush_max;

	struct {
		char *conf;
		char *image;
		char *script;
		char *root;
		char *script_port;
		char *slave_log;
		char *reader;
		char *always;
		char *db;
		char *module;
		char *input;
	} path;

	int max_size_type;

	int slave_max_load;

	double ping_time;

	int use_sw_backend;
	char *provider_method;
	int debug_mode;
	int overwrite_content;
	int com_core_thread;
	int use_xmonitor;
	int premultiplied;
};

/*!
 * \brief This function is defined by the data-provider-slave
 */
static struct info {
	const char *(*find_pkgname)(const char *filename);
	int (*request_update_by_id)(const char *uri);
	int (*trigger_update_monitor)(const char *id, int is_pd);
	int (*update_extra_info)(const char *id, const char *content, const char *title, const char *icon, const char *name);
	struct conf *conf;
} s_info = {
	.find_pkgname = NULL,
	.request_update_by_id = NULL,
	.trigger_update_monitor = NULL,
	.update_extra_info = NULL,
	.conf = NULL,
};

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

struct livebox_desc {
	FILE *fp;
	int for_pd;

	unsigned int last_idx;

	struct dlist *block_list;
};

struct livebox_buffer_data {
	int is_pd;
	int accelerated;

	/* for Buffer event wrapper */
	int (*handler_NEW)(struct livebox_buffer *, struct buffer_event_data *, void *);
	int (*handler)(struct livebox_buffer *, enum buffer_event, double, double, double, void *);
	void *cbdata;

	char *content;
	char *title;
	char *icon;
	char *name;
};

PUBLIC const int DONE = 0x00;
PUBLIC const int OUTPUT_UPDATED = 0x02;
PUBLIC const int USE_NET = 0x04;

PUBLIC const int NEED_TO_SCHEDULE = 0x01;
PUBLIC const int NEED_TO_CREATE = 0x01;
PUBLIC const int NEED_TO_DESTROY = 0x01;
PUBLIC const int NEED_TO_UPDATE = 0x01;
PUBLIC const int FORCE_TO_SCHEDULE = 0x08;

PUBLIC const int LB_SYS_EVENT_FONT_CHANGED = 0x01;
PUBLIC const int LB_SYS_EVENT_LANG_CHANGED = 0x02;
PUBLIC const int LB_SYS_EVENT_TIME_CHANGED = 0x04;
PUBLIC const int LB_SYS_EVENT_REGION_CHANGED = 0x08;
PUBLIC const int LB_SYS_EVENT_PAUSED = 0x0100;
PUBLIC const int LB_SYS_EVENT_RESUMED = 0x0200;
PUBLIC const int LB_SYS_EVENT_MMC_STATUS_CHANGED = 0x0400;

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

PUBLIC struct livebox_desc *livebox_desc_open(const char *filename, int for_pd)
{
	struct livebox_desc *handle;
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

PUBLIC int livebox_desc_close(struct livebox_desc *handle)
{
	struct dlist *l;
	struct dlist *n;
	struct block *block;

	if (!handle) {
		return LB_STATUS_ERROR_INVALID;
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
	return LB_STATUS_SUCCESS;
}

PUBLIC int livebox_desc_set_category(struct livebox_desc *handle, const char *id, const char *category)
{
	struct block *block;

	if (!handle || !category) {
		return LB_STATUS_ERROR_INVALID;
	}

	block = calloc(1, sizeof(*block));
	if (!block) {
		return LB_STATUS_ERROR_MEMORY;
	}

	block->type = strdup(LB_DESC_TYPE_INFO);
	if (!block->type) {
		free(block);
		return LB_STATUS_ERROR_MEMORY;
	}

	block->part = strdup("category");
	if (!block->part) {
		free(block->type);
		free(block);
		return LB_STATUS_ERROR_MEMORY;
	}

	block->data = strdup(category);
	if (!block->data) {
		free(block->type);
		free(block->part);
		free(block);
		return LB_STATUS_ERROR_MEMORY;
	}

	if (id) {
		block->id = strdup(id);
		if (!block->id) {
			free(block->data);
			free(block->type);
			free(block->part);
			free(block);
			return LB_STATUS_ERROR_MEMORY;
		}
	}

	block->idx = handle->last_idx++;
	handle->block_list = dlist_append(handle->block_list, block);
	return block->idx;
}

PUBLIC int livebox_desc_set_size(struct livebox_desc *handle, const char *id, int w, int h)
{
	struct block *block;
	char buffer[BUFSIZ];

	if (!handle) {
		return LB_STATUS_ERROR_INVALID;
	}

	block = calloc(1, sizeof(*block));
	if (!block) {
		return LB_STATUS_ERROR_MEMORY;
	}

	block->type = strdup(LB_DESC_TYPE_INFO);
	if (!block->type) {
		free(block);
		return LB_STATUS_ERROR_MEMORY;
	}

	block->part = strdup("size");
	if (!block->part) {
		free(block->type);
		free(block);
		return LB_STATUS_ERROR_MEMORY;
	}

	if (id) {
		block->id = strdup(id);
		if (!block->id) {
			free(block->part);
			free(block->type);
			free(block);
			return LB_STATUS_ERROR_MEMORY;
		}
	}

	snprintf(buffer, sizeof(buffer), "%dx%d", w, h);
	block->data = strdup(buffer);
	if (!block->data) {
		free(block->part);
		free(block->type);
		free(block);
		return LB_STATUS_ERROR_MEMORY;
	}

	block->idx = handle->last_idx++;
	handle->block_list = dlist_append(handle->block_list, block);
	return block->idx;
}

PUBLIC char *livebox_util_nl2br(const char *str)
{
	int len;
	register int i;
	char *ret;
	char *ptr;

	if (!str) {
		return NULL;
	}

	len = strlen(str);
	if (!len) {
		return NULL;
	}

	ret = malloc(len + 1);
	if (!ret) {
		return NULL;
	}

	ptr = ret;
	i = 0;
	while (*str) {
		switch (*str) {
		case '\n':
			if (len - i < 5) {
				char *tmp;
				len += len;

				tmp = realloc(ret, len + 1);
				if (!tmp) {
					free(ret);
					return NULL;
				}

				ret = tmp;
				ptr = tmp + i;
			}

			strcpy(ptr, "<br>");
			ptr += 4;
			i += 4;
			break;
		default:
			*ptr++ = *str;
			i++;
			break;
		}

		str++;
	}
	*ptr = '\0';

	return ret;
}

PUBLIC int livebox_desc_set_id(struct livebox_desc *handle, int idx, const char *id)
{
	struct dlist *l;
	struct block *block;

	dlist_foreach(handle->block_list, l, block) {
		if (block->idx == idx) {
			if (strcasecmp(block->type, LB_DESC_TYPE_SCRIPT)) {
				ErrPrint("Invalid block is used\n");
				return LB_STATUS_ERROR_INVALID;
			}

			free(block->target_id);
			block->target_id = NULL;

			if (!id || !strlen(id)) {
				return LB_STATUS_SUCCESS;
			}

			block->target_id = strdup(id);
			if (!block->target_id) {
				ErrPrint("Heap: %s\n", strerror(errno));
				return LB_STATUS_ERROR_MEMORY;
			}

			return LB_STATUS_SUCCESS;
		}
	}

	return LB_STATUS_ERROR_NOT_EXIST;
}

/*!
 * \return idx
 */
PUBLIC int livebox_desc_add_block(struct livebox_desc *handle, const char *id, const char *type, const char *part, const char *data, const char *option)
{
	struct block *block;

	if (!handle || !type) {
		return LB_STATUS_ERROR_INVALID;
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
		return LB_STATUS_ERROR_MEMORY;
	}

	block->type = strdup(type);
	if (!block->type) {
		ErrPrint("Heap: %s\n", strerror(errno));
		free(block);
		return LB_STATUS_ERROR_MEMORY;
	}

	block->part = strdup(part);
	if (!block->part) {
		ErrPrint("Heap: %s\n", strerror(errno));
		free(block->type);
		free(block);
		return LB_STATUS_ERROR_MEMORY;
	}

	block->data = strdup(data);
	if (!block->data) {
		ErrPrint("Heap: %s\n", strerror(errno));
		free(block->type);
		free(block->part);
		free(block);
		return LB_STATUS_ERROR_MEMORY;
	}

	if (option) {
		block->option = strdup(option);
		if (!block->option) {
			ErrPrint("Heap: %s\n", strerror(errno));
			free(block->data);
			free(block->type);
			free(block->part);
			free(block);
			return LB_STATUS_ERROR_MEMORY;
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
			return LB_STATUS_ERROR_MEMORY;
		}
	}

	block->idx = handle->last_idx++;
	handle->block_list = dlist_append(handle->block_list, block);
	return block->idx;
}

PUBLIC int livebox_desc_del_block(struct livebox_desc *handle, int idx)
{
	struct dlist *l;
	struct block *block;

	if (!handle || idx < 0) {
		return LB_STATUS_ERROR_INVALID;
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
			return LB_STATUS_SUCCESS;
		}
	}

	return LB_STATUS_ERROR_NOT_EXIST;
}

static inline int event_handler_wrapper_NEW(struct livebox_buffer *buffer, struct buffer_event_data *event_info, void *data)
{
	const char *pkgname;
	const char *id;
	struct livebox_buffer_data *cbdata = data;
	int ret;

	pkgname = provider_buffer_pkgname(buffer);
	if (!pkgname) {
		ErrPrint("pkgname is not valid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	id = provider_buffer_id(buffer);
	if (!id) {
		ErrPrint("id is not valid[%s]\n", pkgname);
		return LB_STATUS_ERROR_INVALID;
	}

	if (cbdata->handler_NEW) {
		ret = cbdata->handler_NEW(buffer, event_info, cbdata->cbdata);
	} else {
		ret = LB_STATUS_ERROR_FAULT;
	}

	switch (event_info->type) {
	case BUFFER_EVENT_HIGHLIGHT:
	case BUFFER_EVENT_HIGHLIGHT_NEXT:
	case BUFFER_EVENT_HIGHLIGHT_PREV:
	case BUFFER_EVENT_ACTIVATE:
	case BUFFER_EVENT_ACTION_UP:
	case BUFFER_EVENT_ACTION_DOWN:
	case BUFFER_EVENT_SCROLL_UP:
	case BUFFER_EVENT_SCROLL_MOVE:
	case BUFFER_EVENT_SCROLL_DOWN:
	case BUFFER_EVENT_UNHIGHLIGHT:
		DbgPrint("Accessibility event: %d\n", event_info->type);
		if (ret < 0) {
			(void)provider_send_access_status(pkgname, id, LB_ACCESS_STATUS_ERROR);
		} else {
			(void)provider_send_access_status(pkgname, id, ret);
		}
		break;
	case BUFFER_EVENT_KEY_UP:
	case BUFFER_EVENT_KEY_DOWN:
	case BUFFER_EVENT_KEY_FOCUS_IN:
	case BUFFER_EVENT_KEY_FOCUS_OUT:
		DbgPrint("Key event: %d\n", event_info->type);
		if (ret < 0) {
			(void)provider_send_key_status(pkgname, id, LB_KEY_STATUS_ERROR);
		} else {
			(void)provider_send_key_status(pkgname, id, ret);
		}
		break;
	default:
		break;
	}

	return ret;
}

/*!
 * \note
 * The last "data" argument is same with "user_data" which is managed by "provider_set_user_data).
 */
static inline int event_handler_wrapper(struct livebox_buffer *buffer, enum buffer_event type, double timestamp, double x, double y, void *data)
{
	const char *pkgname;
	const char *id;
	struct livebox_buffer_data *cbdata = data;
	int ret;

	pkgname = provider_buffer_pkgname(buffer);
	if (!pkgname) {
		ErrPrint("pkgname is not valid\n");
		return LB_STATUS_ERROR_INVALID;
	}

	id = provider_buffer_id(buffer);
	if (!id) {
		ErrPrint("id is not valid[%s]\n", pkgname);
		return LB_STATUS_ERROR_INVALID;
	}

	if (cbdata->handler) {
		ret = cbdata->handler(buffer, type, timestamp, x, y, cbdata->cbdata);
	} else {
		ret = LB_STATUS_ERROR_FAULT;
	}

	switch (type) {
	case BUFFER_EVENT_HIGHLIGHT:
	case BUFFER_EVENT_HIGHLIGHT_NEXT:
	case BUFFER_EVENT_HIGHLIGHT_PREV:
	case BUFFER_EVENT_ACTIVATE:
	case BUFFER_EVENT_ACTION_UP:
	case BUFFER_EVENT_ACTION_DOWN:
	case BUFFER_EVENT_SCROLL_UP:
	case BUFFER_EVENT_SCROLL_MOVE:
	case BUFFER_EVENT_SCROLL_DOWN:
	case BUFFER_EVENT_UNHIGHLIGHT:
		DbgPrint("Accessibility event: %d\n", type);
		if (ret < 0) {
			(void)provider_send_access_status(pkgname, id, LB_ACCESS_STATUS_ERROR);
		} else {
			(void)provider_send_access_status(pkgname, id, ret);
		}
		break;
	case BUFFER_EVENT_KEY_UP:
	case BUFFER_EVENT_KEY_DOWN:
	case BUFFER_EVENT_KEY_FOCUS_IN:
	case BUFFER_EVENT_KEY_FOCUS_OUT:
		DbgPrint("Key event: %d\n", type);
		if (ret < 0) {
			(void)provider_send_key_status(pkgname, id, LB_KEY_STATUS_ERROR);
		} else {
			(void)provider_send_key_status(pkgname, id, ret);
		}
		break;
	default:
		break;
	}

	return ret;
}

static inline int default_event_handler_NEW(struct livebox_buffer *buffer, struct buffer_event_data *event_info, void *data)
{
	/* NOP */
	return 0;
}

static inline int default_event_handler(struct livebox_buffer *buffer, enum buffer_event type, double timestamp, double x, double y, void *data)
{
	/* NOP */
	return 0;
}

PUBLIC struct livebox_buffer *livebox_acquire_buffer_NEW(const char *filename, int is_pd, int width, int height, int pixels, int (*handler)(struct livebox_buffer *, struct buffer_event_data *, void *), void *data)
{
	struct livebox_buffer_data *user_data;
	const char *pkgname;
	struct livebox_buffer *handle;
	char *uri;

	if (!filename || !width || !height) {
		ErrPrint("Invalid argument: %p(%dx%d)\n", filename, width, height);
		return NULL;
	}

	user_data = calloc(1, sizeof(*user_data));
	if (!user_data) {
		ErrPrint("Heap: %s\n", strerror(errno));
		return NULL;
	}

	user_data->is_pd = is_pd;
	user_data->handler_NEW = handler ? handler : default_event_handler_NEW;
	user_data->handler = NULL;
	user_data->cbdata = data;

	uri = id_to_uri(filename);
	if (!uri) {
		ErrPrint("Heap: %s\n", strerror(errno));
		free(user_data);
		return NULL;
	}

	if (!s_info.find_pkgname) {
		s_info.find_pkgname = dlsym(RTLD_DEFAULT, "livebox_find_pkgname");
		if (!s_info.find_pkgname) {
			ErrPrint("Failed to find a \"livebox_find_pkgname\"\n");
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

	handle = provider_buffer_create((!!is_pd) ? TYPE_PD : TYPE_LB, pkgname, uri, event_handler_wrapper_NEW, user_data);
	free(uri);
	if (!handle) {
		free(user_data);
		return NULL;
	}

	if (provider_buffer_acquire_NEW(handle, width, height, pixels) < 0) {
		provider_buffer_destroy(handle);
		free(user_data);
		return NULL;
	}

	(void)provider_buffer_set_user_data(handle, user_data);
	return handle;
}

PUBLIC struct livebox_buffer *livebox_acquire_buffer(const char *filename, int is_pd, int width, int height, int (*handler)(struct livebox_buffer *, enum buffer_event, double, double, double, void *), void *data)
{
	struct livebox_buffer_data *user_data;
	const char *pkgname;
	struct livebox_buffer *handle;
	char *uri;

	if (!filename || !width || !height) {
		ErrPrint("Invalid argument: %p(%dx%d)\n", filename, width, height);
		return NULL;
	}

	user_data = calloc(1, sizeof(*user_data));
	if (!user_data) {
		ErrPrint("Heap: %s\n", strerror(errno));
		return NULL;
	}

	user_data->is_pd = is_pd;
	user_data->handler = handler ? handler : default_event_handler;
	user_data->handler_NEW = NULL;
	user_data->cbdata = data;

	uri = id_to_uri(filename);
	if (!uri) {
		ErrPrint("Heap: %s\n", strerror(errno));
		free(user_data);
		return NULL;
	}

	if (!s_info.find_pkgname) {
		s_info.find_pkgname = dlsym(RTLD_DEFAULT, "livebox_find_pkgname");
		if (!s_info.find_pkgname) {
			ErrPrint("Failed to find a \"livebox_find_pkgname\"\n");
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

	handle = provider_buffer_acquire((!!is_pd) ? TYPE_PD : TYPE_LB, pkgname, uri, width, height, sizeof(int), event_handler_wrapper, user_data);
	DbgPrint("Acquire buffer for PD(%s), %s, %p\n", pkgname, uri, handle);
	free(uri);
	if (!handle) {
		free(user_data);
		return NULL;
	}

	(void)provider_buffer_set_user_data(handle, user_data);
	return handle;
}

PUBLIC int livebox_request_update(const char *filename)
{
	char *uri;
	int ret;

	if (!filename) {
		ErrPrint("Invalid argument\n");
		return LB_STATUS_ERROR_INVALID;
	}

	uri = id_to_uri(filename);
	if (!uri) {
		ErrPrint("Heap: %s\n", strerror(errno));
		return LB_STATUS_ERROR_MEMORY;
	}

	if (!s_info.request_update_by_id) {
		s_info.request_update_by_id = dlsym(RTLD_DEFAULT, "livebox_request_update_by_id");
		if (!s_info.request_update_by_id) {
			ErrPrint("\"livebox_request_update_by_id\" is not exists\n");
			free(uri);
			return LB_STATUS_ERROR_FAULT;
		}
	}
	ret = s_info.request_update_by_id(uri);
	free(uri);
	return ret;
}

PUBLIC int livebox_conf_premultiplied_alpha(void)
{
	if (!s_info.conf) {
		s_info.conf = dlsym(RTLD_DEFAULT, "g_conf");
		if (!s_info.conf) {
			ErrPrint("g_conf is not found\n");
			return 1;
		}

		DbgPrint("Premultiplied alpha: %d\n", s_info.conf->premultiplied);
	}

	return s_info.conf->premultiplied;
}

PUBLIC unsigned long livebox_pixmap_id(struct livebox_buffer *handle)
{
	return provider_buffer_pixmap_id(handle);
}

PUBLIC int livebox_release_buffer(struct livebox_buffer *handle)
{
	struct livebox_buffer_data *user_data;

	if (!handle) {
		return LB_STATUS_ERROR_INVALID;
	}

	user_data = provider_buffer_user_data(handle);
	if (user_data) {
		free(user_data->content);
		free(user_data->title);
		free(user_data->icon);
		free(user_data->name);
		free(user_data);
		provider_buffer_set_user_data(handle, NULL);
	}

	DbgPrint("Release buffer\n");
	return provider_buffer_release(handle);
}

PUBLIC int livebox_release_buffer_NEW(struct livebox_buffer *handle)
{
	struct livebox_buffer_data *user_data;

	if (!handle) {
		return LB_STATUS_ERROR_INVALID;
	}

	user_data = provider_buffer_user_data(handle);
	if (user_data) {
		free(user_data->content);
		free(user_data->title);
		free(user_data->icon);
		free(user_data->name);
		free(user_data);
		provider_buffer_set_user_data(handle, NULL);
	}

	(void)provider_buffer_release_NEW(handle);
	(void)provider_buffer_destroy(handle);

	DbgPrint("Release buffer\n");
	return 0;
}

PUBLIC void *livebox_ref_buffer(struct livebox_buffer *handle)
{
	struct livebox_buffer_data *user_data;
	void *data;
	int w, h, size;
	int ret;

	if (!handle) {
		return NULL;
	}

	user_data = provider_buffer_user_data(handle);
	if (!user_data) {
		return NULL;
	}

	if (user_data->accelerated) {
		DbgPrint("H/W accelerated buffer is allocated\n");
		return NULL;
	}

	ret = provider_buffer_get_size(handle, &w, &h, &size);

	data = provider_buffer_ref(handle);
	if (data && !ret && w > 0 && h > 0 && size > 0) {
		memset(data, 0, w * h * size);
		(void)provider_buffer_sync(handle);
	}

	DbgPrint("Ref buffer %ds%d(%d)\n", w, h, size);
	return data;
}

PUBLIC int livebox_unref_buffer(void *buffer)
{
	if (!buffer) {
		return LB_STATUS_ERROR_INVALID;
	}

	DbgPrint("Unref buffer\n");
	return provider_buffer_unref(buffer);
}

PUBLIC int livebox_sync_buffer(struct livebox_buffer *handle)
{
	struct livebox_buffer_data *user_data;
	const char *pkgname;
	const char *id;

	if (!handle) {
		return LB_STATUS_ERROR_INVALID;
	}

	user_data = provider_buffer_user_data(handle);
	if (!user_data) {
		ErrPrint("Invalid buffer\n");
		return LB_STATUS_ERROR_INVALID;
	}

	if (user_data->accelerated) {
		DbgPrint("H/W Buffer allocated. skip the sync buffer\n");
		return LB_STATUS_SUCCESS;
	}

	pkgname = provider_buffer_pkgname(handle);
	if (!pkgname) {
		ErrPrint("Invalid buffer handler\n");
		return LB_STATUS_ERROR_INVALID;
	}

	id = provider_buffer_id(handle);
	if (!id) {
		ErrPrint("Invalid buffer handler\n");
		return LB_STATUS_ERROR_INVALID;
	}

	(void)provider_buffer_sync(handle);

	if (user_data->is_pd == 1) {
		if (provider_send_desc_updated(pkgname, id, NULL) < 0) {
			ErrPrint("Failed to send PD updated (%s)\n", id);
		}
	} else {
		int w;
		int h;
		int pixel_size;

		if (provider_buffer_get_size(handle, &w, &h, &pixel_size) < 0) {
			ErrPrint("Failed to get size (%s)\n", id);
		}

		if (provider_send_updated_NEW(pkgname, id, w, h, -1.0f, user_data->content, user_data->title, user_data->icon, user_data->name) < 0) {
			ErrPrint("Failed to send updated (%s)\n", id);
		}
	}

	return LB_STATUS_SUCCESS;
}

PUBLIC int livebox_support_hw_buffer(struct livebox_buffer *handle)
{
	if (!handle) {
		return LB_STATUS_ERROR_INVALID;
	}

	return provider_buffer_pixmap_is_support_hw(handle);
}

PUBLIC int livebox_create_hw_buffer(struct livebox_buffer *handle)
{
	struct livebox_buffer_data *user_data;
	int ret;

	if (!handle) {
		return LB_STATUS_ERROR_INVALID;
	}

	user_data = provider_buffer_user_data(handle);
	if (!user_data) {
		return LB_STATUS_ERROR_INVALID;
	}

	if (user_data->accelerated) {
		return LB_STATUS_ERROR_ALREADY;
	}

	ret = provider_buffer_pixmap_create_hw(handle);
	user_data->accelerated = (ret == 0);
	return ret;
}

PUBLIC int livebox_destroy_hw_buffer(struct livebox_buffer *handle)
{
	struct livebox_buffer_data *user_data;

	if (!handle) {
		LOGD("handle is NULL\n");
		return LB_STATUS_ERROR_INVALID;
	}

	user_data = provider_buffer_user_data(handle);
	if (!user_data || !user_data->accelerated) {
		LOGD("user_data is NULL\n");
		return LB_STATUS_ERROR_INVALID;
	}

	user_data->accelerated = 0;

	return provider_buffer_pixmap_destroy_hw(handle);
}

PUBLIC void *livebox_buffer_hw_buffer(struct livebox_buffer *handle)
{
	struct livebox_buffer_data *user_data;

	if (!handle) {
		return NULL;
	}

	user_data = provider_buffer_user_data(handle);
	if (!user_data || !user_data->accelerated) {
		return NULL;
	}

	return provider_buffer_pixmap_hw_addr(handle);
}

PUBLIC int livebox_buffer_pre_render(struct livebox_buffer *handle)
{
	struct livebox_buffer_data *user_data;

	if (!handle) {
		return LB_STATUS_ERROR_INVALID;
	}

	user_data = provider_buffer_user_data(handle);
	if (!user_data) {
		return LB_STATUS_ERROR_INVALID;
	}

	if (!user_data->accelerated) {
		return LB_STATUS_SUCCESS;
	}

	/*!
	 * \note
	 * Do preprocessing for accessing the H/W render buffer
	 */
	return provider_buffer_pre_render(handle);
}

PUBLIC int livebox_buffer_post_render(struct livebox_buffer *handle)
{
	int ret;
	const char *pkgname;
	const char *id;
	struct livebox_buffer_data *user_data;

	if (!handle) {
		return LB_STATUS_ERROR_INVALID;
	}

	user_data = provider_buffer_user_data(handle);
	if (!user_data) {
		return LB_STATUS_ERROR_INVALID;
	}

	if (!user_data->accelerated) {
		return LB_STATUS_SUCCESS;
	}

	pkgname = provider_buffer_pkgname(handle);
	if (!pkgname) {
		ErrPrint("Invalid buffer handle\n");
		return LB_STATUS_ERROR_INVALID;
	}

	id = provider_buffer_id(handle);
	if (!id) {
		ErrPrint("Invalid buffer handler\n");
		return LB_STATUS_ERROR_INVALID;
	}

	ret = provider_buffer_post_render(handle);
	if (ret < 0) {
		ErrPrint("Failed to post render processing\n");
		return ret;
	}

	if (user_data->is_pd == 1) {
		if (provider_send_desc_updated(pkgname, id, NULL) < 0) {
			ErrPrint("Failed to send PD updated (%s)\n", id);
		}
	} else {
		int w;
		int h;
		int pixel_size;

		if (provider_buffer_get_size(handle, &w, &h, &pixel_size) < 0) {
			ErrPrint("Failed to get size (%s)\n", id);
		}

		if (provider_send_updated_NEW(pkgname, id, w, h, -1.0f, user_data->content, user_data->title, user_data->icon, user_data->name) < 0) {
			ErrPrint("Failed to send updated (%s)\n", id);
		}
	}

	return LB_STATUS_SUCCESS;
}

PUBLIC int livebox_content_is_updated(const char *filename, int is_pd)
{
	if (!s_info.trigger_update_monitor) {
		s_info.trigger_update_monitor = dlsym(RTLD_DEFAULT, "livebox_trigger_update_monitor");
		if (!s_info.trigger_update_monitor) {
			ErrPrint("Trigger update monitor function is not exists\n");
			return LB_STATUS_ERROR_FAULT;
		}
	}

	return s_info.trigger_update_monitor(filename, is_pd);
}

PUBLIC int livebox_request_close_pd(const char *pkgname, const char *id, int reason)
{
	char *uri;
	int schema_len = strlen(FILE_SCHEMA);
	int ret;

	if (!pkgname || !id) {
		ErrPrint("Invalid parameters (%s) (%s)\n", pkgname, id);
		return LB_STATUS_ERROR_INVALID;
	}

	if (strncmp(id, FILE_SCHEMA, schema_len)) {
		uri = id_to_uri(id);
		if (!uri) {
			ErrPrint("Heap: %s\n", strerror(errno));
			return LB_STATUS_ERROR_MEMORY;
		}
	} else {
		uri = strdup(id);
		if (!uri) {
			ErrPrint("Heap: %s\n", strerror(errno));
			return LB_STATUS_ERROR_MEMORY;
		}
	}

	ret = provider_send_request_close_pd(pkgname, uri, reason);
	free(uri);
	return ret;
}

PUBLIC int livebox_freeze_scroller(const char *pkgname, const char *id)
{
	char *uri;
	int ret;

	uri = id_to_uri(id);
	if (!uri) {
		ErrPrint("Heap: %s\n", strerror(errno));
		return LB_STATUS_ERROR_MEMORY;
	}

	ret = provider_send_hold_scroll(pkgname, uri, 1);
	free(uri);
	return ret;
}

PUBLIC int livebox_release_scroller(const char *pkgname, const char *id)
{
	char *uri;
	int ret;

	uri = id_to_uri(id);
	if (!uri) {
		ErrPrint("Heap: %s\n", strerror(errno));
		return LB_STATUS_ERROR_MEMORY;
	}

	ret = provider_send_hold_scroll(pkgname, uri, 0);
	free(uri);
	return ret;
}

PUBLIC int livebox_set_extra_info(const char *id, const char *content, const char *title, const char *icon, const char *name)
{
	struct livebox_buffer *handle;
	const char *pkgname;
	char *uri;

	uri = id_to_uri(id);
	if (!uri) {
		ErrPrint("Heap: %s\n", strerror(errno));
		return LB_STATUS_ERROR_MEMORY;
	}

	pkgname = s_info.find_pkgname(uri);
	if (!pkgname) {
		ErrPrint("Failed to find a package (%s)\n", uri);
		free(uri);
		return LB_STATUS_ERROR_INVALID;
	}

	handle = provider_buffer_find_buffer(TYPE_LB, pkgname, uri);
	free(uri);
	if (handle) {
		struct livebox_buffer_data *user_data;

		user_data = provider_buffer_user_data(handle);
		if (!user_data) {
			ErrPrint("User data is not available\n");
			return LB_STATUS_ERROR_FAULT;
		}

		if (content && strlen(content)) {
			char *_content;

			_content = strdup(content);
			if (_content) {
				if (user_data->content) {
					free(user_data->content);
					user_data->content = NULL;
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
					user_data->title = NULL;
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
					user_data->icon = NULL;
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
					user_data->name = NULL;
				}

				user_data->name = _name;
			} else {
				ErrPrint("Heap: %s\n", strerror(errno));
			}
		}

		return LB_STATUS_SUCCESS;
	}

	if (!s_info.update_extra_info) {
		s_info.update_extra_info = dlsym(RTLD_DEFAULT, "livebox_update_extra_info");
		if (!s_info.update_extra_info) {
			ErrPrint("Failed to find a \"livebox_update_extra_info\"\n");
			return LB_STATUS_ERROR_INVALID;
		}
	}

	return s_info.update_extra_info(id, content, title, icon, name);
}

/* End of a file */
