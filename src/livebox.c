/*
 * Copyright 2012  Samsung Electronics Co., Ltd
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

#include <dlog.h>
#include <livebox-service.h>
#include <provider.h>
#include <provider_buffer.h>

#include "debug.h"
#include "livebox.h"
#include "dlist.h"
#include "util.h"

#define EAPI __attribute__((visibility("default")))

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
	char *group;
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

EAPI const int DONE = 0x00;
EAPI const int NEED_TO_SCHEDULE = 0x01;
EAPI const int OUTPUT_UPDATED = 0x02;
EAPI const int NEED_TO_CREATE = 0x01;
EAPI const int NEED_TO_DESTROY = 0x01;
EAPI const int LB_SYS_EVENT_FONT_CHANGED = 0x01;
EAPI const int LB_SYS_EVENT_LANG_CHANGED = 0x02;
EAPI const int LB_SYS_EVENT_TIME_CHANGED = 0x04;
EAPI const int LB_SYS_EVENT_PAUSED = 0x0100;
EAPI const int LB_SYS_EVENT_RESUMED = 0x0200;

EAPI struct livebox_desc *livebox_desc_open(const char *filename, int for_pd)
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

	DbgPrint("Open a new file: %s\n", new_fname);
	handle->fp = fopen(new_fname, "w+t");
	free(new_fname);
	if (!handle->fp) {
		ErrPrint("Failed to open a file: %s\n", strerror(errno));
		free(handle);
		return NULL;
	}

	return handle;
}

EAPI int livebox_desc_close(struct livebox_desc *handle)
{
	struct dlist *l;
	struct dlist *n;
	struct block *block;

	if (!handle)
		return -EINVAL;

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

		if (block->group) {
			fprintf(handle->fp, "group=%s\n", block->group);
			DbgPrint("group=%s\n", block->group);
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
		free(block->group);
		free(block->id);
		free(block->target_id);
		free(block);
	}

	fclose(handle->fp);
	free(handle);
	return 0;
}

EAPI int livebox_desc_set_category(struct livebox_desc *handle, const char *id, const char *category)
{
	struct block *block;

	if (!handle || !category)
		return -EINVAL;

	block = calloc(1, sizeof(*block));
	if (!block)
		return -ENOMEM;

	block->type = strdup(LB_DESC_TYPE_INFO);
	if (!block->type) {
		free(block);
		return -ENOMEM;
	}

	block->part = strdup("category");
	if (!block->part) {
		free(block->type);
		free(block);
		return -ENOMEM;
	}

	block->data = strdup(category);
	if (!block->data) {
		free(block->type);
		free(block->part);
		free(block);
		return -ENOMEM;
	}

	if (id) {
		block->id = strdup(id);
		if (!block->id) {
			free(block->data);
			free(block->type);
			free(block->part);
			free(block);
			return -ENOMEM;
		}
	}

	block->idx = handle->last_idx++;
	handle->block_list = dlist_append(handle->block_list, block);
	return block->idx;
}

EAPI int livebox_desc_set_size(struct livebox_desc *handle, const char *id, int w, int h)
{
	struct block *block;
	char buffer[BUFSIZ];

	if (!handle)
		return -EINVAL;

	block = calloc(1, sizeof(*block));
	if (!block)
		return -ENOMEM;

	block->type = strdup(LB_DESC_TYPE_INFO);
	if (!block->type) {
		free(block);
		return -ENOMEM;
	}

	block->part = strdup("size");
	if (!block->part) {
		free(block->type);
		free(block);
		return -ENOMEM;
	}

	if (id) {
		block->id = strdup(id);
		if (!block->id) {
			free(block->part);
			free(block->type);
			free(block);
			return -ENOMEM;
		}
	}

	snprintf(buffer, sizeof(buffer), "%dx%d", w, h);
	block->data = strdup(buffer);
	if (!block->data) {
		free(block->part);
		free(block->type);
		free(block);
		return -ENOMEM;
	}

	block->idx = handle->last_idx++;
	handle->block_list = dlist_append(handle->block_list, block);
	return block->idx;
}

EAPI char *livebox_util_nl2br(const char *str)
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

EAPI int livebox_desc_set_id(struct livebox_desc *handle, int idx, const char *id)
{
	struct dlist *l;
	struct block *block;

	dlist_foreach(handle->block_list, l, block) {
		if (block->idx == idx) {
			if (strcasecmp(block->type, LB_DESC_TYPE_SCRIPT)) {
				ErrPrint("Invalid block is used\n");
				return -EINVAL;
			}

			free(block->target_id);
			block->target_id = NULL;

			if (!id || !strlen(id))
				return 0;

			block->target_id = strdup(id);
			if (!block->target_id) {
				ErrPrint("Heap: %s\n", strerror(errno));
				return -ENOMEM;
			}

			return 0;
		}
	}

	return -ENOENT;
}

/*!
 * \return idx
 */
EAPI int livebox_desc_add_block(struct livebox_desc *handle, const char *id, const char *type, const char *part, const char *data, const char *group)
{
	struct block *block;

	if (!handle || !type)
		return -EINVAL;

	if (!part)
		part = "";

	if (!data)
		data = "";

	block = calloc(1, sizeof(*block));
	if (!block)
		return -ENOMEM;

	block->type = strdup(type);
	if (!block->type) {
		free(block);
		return -ENOMEM;
	}

	block->part = strdup(part);
	if (!block->part) {
		free(block->type);
		free(block);
		return -ENOMEM;
	}

	block->data = strdup(data);
	if (!block->data) {
		free(block->type);
		free(block->part);
		free(block);
		return -ENOMEM;
	}

	if (group) {
		block->group = strdup(group);
		if (!block->group) {
			free(block->data);
			free(block->type);
			free(block->part);
			free(block);
			return -ENOMEM;
		}
	}

	if (id) {
		block->id = strdup(id);
		if (!block->id) {
			free(block->group);
			free(block->data);
			free(block->type);
			free(block->part);
			free(block);
			return -ENOMEM;
		}
	}

	block->idx = handle->last_idx++;
	handle->block_list = dlist_append(handle->block_list, block);
	return block->idx;
}

EAPI int livebox_desc_del_block(struct livebox_desc *handle, int idx)
{
	struct dlist *l;
	struct block *block;

	dlist_foreach(handle->block_list, l, block) {
		if (block->idx == idx) {
			handle->block_list = dlist_remove(handle->block_list, l);
			free(block->type);
			free(block->part);
			free(block->data);
			free(block->group);
			free(block->id);
			free(block->target_id);
			free(block);
			return 0;
		}
	}

	return -ENOENT;
}

EAPI struct livebox_buffer *livebox_acquire_buffer(const char *filename, int is_pd, int width, int height, int (*handler)(struct livebox_buffer *, enum buffer_event, double, double, double, void *), void *data)
{
	const char *pkgname;
	struct livebox_buffer *handle;
	char *uri;
	int uri_len;

	if (!filename || !width || !height) {
		ErrPrint("Invalid argument: %p(%dx%d)\n", filename, width, height);
		return NULL;
	}

	uri_len = strlen(filename) + strlen(FILE_SCHEMA) + 1;
	uri = malloc(uri_len);
	if (!uri) {
		ErrPrint("Heap: %s\n", strerror(errno));
		return NULL;
	}

	snprintf(uri, uri_len, FILE_SCHEMA "%s", filename);
	pkgname = livebox_find_pkgname(uri);
	if (!pkgname) {
		ErrPrint("Invalid Request\n");
		free(uri);
		return NULL;
	}

	handle = provider_buffer_acquire((!!is_pd) ? TYPE_PD : TYPE_LB, pkgname, uri, width, height, sizeof(int), handler, data);
	DbgPrint("Acquire buffer for PD(%s), %s, %p\n", pkgname, uri, handle);
	free(uri);
	return handle;
}

EAPI int livebox_request_update(const char *filename)
{
	int uri_len;
	char *uri;
	int ret;

	if (!filename) {
		ErrPrint("Invalid argument\n");
		return -EINVAL;
	}

	uri_len = strlen(filename) + strlen(FILE_SCHEMA) + 1;
	uri = malloc(uri_len);
	if (!uri) {
		ErrPrint("Heap: %s\n", strerror(errno));
		return -ENOMEM;
	}

	snprintf(uri, uri_len, FILE_SCHEMA "%s", filename);
	ret = livebox_request_update_by_id(uri);
	free(uri);
	return ret;
}

EAPI unsigned long livebox_pixmap_id(struct livebox_buffer *handle)
{
	return provider_buffer_pixmap_id(handle);
}

EAPI int livebox_release_buffer(struct livebox_buffer *handle)
{
	if (!handle)
		return -EINVAL;

	DbgPrint("Release buffer\n");
	return provider_buffer_release(handle);
}

EAPI void *livebox_ref_buffer(struct livebox_buffer *handle)
{
	if (!handle)
		return -EINVAL;

	DbgPrint("Ref buffer\n");
	return provider_buffer_ref(handle);
}

EAPI int livebox_unref_buffer(void *buffer)
{
	if (!buffer)
		return -EINVAL;

	DbgPrint("Unref buffer\n");
	return provider_buffer_unref(buffer);
}

EAPI int livebox_sync_buffer(struct livebox_buffer *handle)
{
	const char *pkgname;
	const char *id;

	if (!handle)
		return -EINVAL;

	pkgname = provider_buffer_pkgname(handle);
	id = provider_buffer_id(handle);
	if (!pkgname || !id) {
		ErrPrint("Invalid buffer handler\n");
		return -EINVAL;
	}

	DbgPrint("Sync buffer\n");
	provider_buffer_sync(handle);
	provider_send_desc_updated(pkgname, id, NULL);
	return 0;
}

/* End of a file */
