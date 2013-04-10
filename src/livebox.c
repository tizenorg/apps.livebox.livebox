/*
 * Copyright 2013  Samsung Electronics Co., Ltd
 *
 * Licensed under the Flora License, Version 1.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.tizenopensource.org/license
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

/*!
 * \brief This function is defined by the data-provider-slave
 */
extern const char *livebox_find_pkgname(const char *filename);
extern int livebox_request_update_by_id(const char *uri);

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
};

PUBLIC const int DONE = 0x00;
PUBLIC const int OUTPUT_UPDATED = 0x02;
PUBLIC const int USE_NET = 0x04;

PUBLIC const int NEED_TO_SCHEDULE = 0x01;
PUBLIC const int NEED_TO_CREATE = 0x01;
PUBLIC const int NEED_TO_DESTROY = 0x01;
PUBLIC const int NEED_TO_UPDATE = 0x01;

PUBLIC const int LB_SYS_EVENT_FONT_CHANGED = 0x01;
PUBLIC const int LB_SYS_EVENT_LANG_CHANGED = 0x02;
PUBLIC const int LB_SYS_EVENT_TIME_CHANGED = 0x04;
PUBLIC const int LB_SYS_EVENT_REGION_CHANGED = 0x08;
PUBLIC const int LB_SYS_EVENT_PAUSED = 0x0100;
PUBLIC const int LB_SYS_EVENT_RESUMED = 0x0200;

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

	if (!handle)
		return LB_STATUS_ERROR_INVALID;

	DbgPrint("Close and flush\n");
	dlist_foreach_safe(handle->block_list, l, n, block) {
		handle->block_list = dlist_remove(handle->block_list, l);

		DbgPrint("{\n");
		fprintf(handle->fp, "{\n");
		if (block->type) {
			fprintf(handle->fp, "type=%s\n", block->type);
			DbgPrint("type=%s\n", block->type);
		}

		if (block->part) {
			fprintf(handle->fp, "part=%s\n", block->part);
			DbgPrint("part=%s\n", block->part);
		}

		if (block->data) {
			fprintf(handle->fp, "data=%s\n", block->data);
			DbgPrint("data=%s\n", block->data);
		}

		if (block->option) {
			fprintf(handle->fp, "option=%s\n", block->option);
			DbgPrint("option=%s\n", block->option);
		}

		if (block->id) {
			fprintf(handle->fp, "id=%s\n", block->id);
			DbgPrint("id=%s\n", block->id);
		}

		if (block->target_id) {
			fprintf(handle->fp, "target=%s\n", block->target_id);
			DbgPrint("target=%s\n", block->target_id);
		}

		fprintf(handle->fp, "}\n");
		DbgPrint("}\n");

		free(block->type);
		free(block->part);
		free(block->data);
		free(block->option);
		free(block->id);
		free(block->target_id);
		free(block);
	}

	fclose(handle->fp);
	free(handle);
	return LB_STATUS_SUCCESS;
}

PUBLIC int livebox_desc_set_category(struct livebox_desc *handle, const char *id, const char *category)
{
	struct block *block;

	if (!handle || !category)
		return LB_STATUS_ERROR_INVALID;

	block = calloc(1, sizeof(*block));
	if (!block)
		return LB_STATUS_ERROR_MEMORY;

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

	if (!handle)
		return LB_STATUS_ERROR_INVALID;

	block = calloc(1, sizeof(*block));
	if (!block)
		return LB_STATUS_ERROR_MEMORY;

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

	if (!str)
		return NULL;

	len = strlen(str);
	if (!len)
		return NULL;

	ret = malloc(len + 1);
	if (!ret)
		return NULL;

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

			if (!id || !strlen(id))
				return LB_STATUS_SUCCESS;

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

	if (!handle || !type)
		return LB_STATUS_ERROR_INVALID;

	if (!part)
		part = "";

	if (!data)
		data = "";

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

PUBLIC struct livebox_buffer *livebox_acquire_buffer(const char *filename, int is_pd, int width, int height, int (*handler)(struct livebox_buffer *, enum buffer_event, double, double, double, void *), void *data)
{
	struct livebox_buffer_data *user_data;
	const char *pkgname;
	struct livebox_buffer *handle;
	char *uri;
	int uri_len;

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

	uri_len = strlen(filename) + strlen(FILE_SCHEMA) + 1;
	uri = malloc(uri_len);
	if (!uri) {
		ErrPrint("Heap: %s\n", strerror(errno));
		free(user_data);
		return NULL;
	}

	snprintf(uri, uri_len, FILE_SCHEMA "%s", filename);
	pkgname = livebox_find_pkgname(uri);
	if (!pkgname) {
		ErrPrint("Invalid Request\n");
		free(user_data);
		free(uri);
		return NULL;
	}

	handle = provider_buffer_acquire((!!is_pd) ? TYPE_PD : TYPE_LB, pkgname, uri, width, height, sizeof(int), handler, data);
	DbgPrint("Acquire buffer for PD(%s), %s, %p\n", pkgname, uri, handle);
	free(uri);

	(void)provider_buffer_set_user_data(handle, user_data);
	return handle;
}

PUBLIC int livebox_request_update(const char *filename)
{
	int uri_len;
	char *uri;
	int ret;

	if (!filename) {
		ErrPrint("Invalid argument\n");
		return LB_STATUS_ERROR_INVALID;
	}

	uri_len = strlen(filename) + strlen(FILE_SCHEMA) + 1;
	uri = malloc(uri_len);
	if (!uri) {
		ErrPrint("Heap: %s\n", strerror(errno));
		return LB_STATUS_ERROR_MEMORY;
	}

	snprintf(uri, uri_len, FILE_SCHEMA "%s", filename);
	ret = livebox_request_update_by_id(uri);
	free(uri);
	return ret;
}

PUBLIC unsigned long livebox_pixmap_id(struct livebox_buffer *handle)
{
	return provider_buffer_pixmap_id(handle);
}

PUBLIC int livebox_release_buffer(struct livebox_buffer *handle)
{
	struct livebox_buffer_data *user_data;

	if (!handle)
		return LB_STATUS_ERROR_INVALID;

	user_data = provider_buffer_user_data(handle);
	if (user_data) {
		free(user_data);
		provider_buffer_set_user_data(handle, NULL);
	}

	DbgPrint("Release buffer\n");
	return provider_buffer_release(handle);
}

PUBLIC void *livebox_ref_buffer(struct livebox_buffer *handle)
{
	struct livebox_buffer_data *user_data;
	void *data;
	int w, h, size;
	int ret;

	if (!handle)
		return NULL;

	user_data = provider_buffer_user_data(handle);
	if (!user_data)
		return NULL;

	if (user_data->accelerated) {
		DbgPrint("H/W accelerated buffer is allocated\n");
		return NULL;
	}

	ret = provider_buffer_get_size(handle, &w, &h, &size);

	data = provider_buffer_ref(handle);
	if (data && !ret && w > 0 && h > 0 && size > 0) {
		memset(data, 0, w * h * size);
		provider_buffer_sync(handle);
	}

	DbgPrint("Ref buffer %ds%d(%d)\n", w, h, size);
	return data;
}

PUBLIC int livebox_unref_buffer(void *buffer)
{
	if (!buffer)
		return LB_STATUS_ERROR_INVALID;

	DbgPrint("Unref buffer\n");
	return provider_buffer_unref(buffer);
}

PUBLIC int livebox_sync_buffer(struct livebox_buffer *handle)
{
	struct livebox_buffer_data *user_data;
	const char *pkgname;
	const char *id;

	if (!handle)
		return LB_STATUS_ERROR_INVALID;

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

	provider_buffer_sync(handle);

	if (user_data->is_pd) {
		if (provider_send_desc_updated(pkgname, id, NULL) < 0)
			ErrPrint("Failed to send PD updated (%s)\n", id);
	} else {
		int w;
		int h;
		int pixel_size;

		if (provider_buffer_get_size(handle, &w, &h, &pixel_size) < 0)
			ErrPrint("Failed to get size (%s)\n", id);

		if (provider_send_updated(pkgname, id, w, h, -1.0f, NULL, NULL) < 0)
			ErrPrint("Failed to send updated (%s)\n", id);
	}

	return LB_STATUS_SUCCESS;
}

PUBLIC int livebox_support_hw_buffer(struct livebox_buffer *handle)
{
	if (!handle)
		return LB_STATUS_ERROR_INVALID;

	return provider_buffer_pixmap_is_support_hw(handle);
}

PUBLIC int livebox_create_hw_buffer(struct livebox_buffer *handle)
{
	struct livebox_buffer_data *user_data;
	int ret;

	if (!handle)
		return LB_STATUS_ERROR_INVALID;

	user_data = provider_buffer_user_data(handle);
	if (!user_data)
		return LB_STATUS_ERROR_INVALID;

	if (user_data->accelerated)
		return -EALREADY;

	ret = provider_buffer_pixmap_create_hw(handle);
	user_data->accelerated = (ret == 0);
	return ret;
}

PUBLIC int livebox_destroy_hw_buffer(struct livebox_buffer *handle)
{
	struct livebox_buffer_data *user_data;
	if (!handle)
		return LB_STATUS_ERROR_INVALID;

	user_data = provider_buffer_user_data(handle);
	if (!user_data || !user_data->accelerated)
		return LB_STATUS_ERROR_INVALID;

	user_data->accelerated = 0;

	return provider_buffer_pixmap_destroy_hw(handle);
}

PUBLIC void *livebox_buffer_hw_buffer(struct livebox_buffer *handle)
{
	struct livebox_buffer_data *user_data;

	if (!handle)
		return NULL;

	user_data = provider_buffer_user_data(handle);
	if (!user_data || !user_data->accelerated)
		return NULL;

	return provider_buffer_pixmap_hw_addr(handle);
}

PUBLIC int livebox_buffer_pre_render(struct livebox_buffer *handle)
{
	struct livebox_buffer_data *user_data;

	if (!handle)
		return LB_STATUS_ERROR_INVALID;

	user_data = provider_buffer_user_data(handle);
	if (!user_data)
		return LB_STATUS_ERROR_INVALID;

	if (!user_data->accelerated)
		return LB_STATUS_SUCCESS;

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

	if (!handle)
		return LB_STATUS_ERROR_INVALID;

	user_data = provider_buffer_user_data(handle);
	if (!user_data)
		return LB_STATUS_ERROR_INVALID;

	if (!user_data->accelerated)
		return LB_STATUS_SUCCESS;

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
		if (provider_send_desc_updated(pkgname, id, NULL) < 0)
			ErrPrint("Failed to send PD updated (%s)\n", id);
	} else {
		int w;
		int h;
		int pixel_size;

		if (provider_buffer_get_size(handle, &w, &h, &pixel_size) < 0)
			ErrPrint("Failed to get size (%s)\n", id);

		if (provider_send_updated(pkgname, id, w, h, -1.0f, NULL, NULL) < 0)
			ErrPrint("Failed to send updated (%s)\n", id);
	}

	return LB_STATUS_SUCCESS;
}

/* End of a file */
