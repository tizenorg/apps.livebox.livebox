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

#include <livebox-service.h>
#include <livebox-errno.h>

#include "livebox.h"

#define PUBLIC __attribute__((visibility("default")))

struct livebox_desc {
	FILE *fp;
	int for_pd;

	unsigned int last_idx;

	//struct dlist *block_list;
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
PUBLIC const int LB_SYS_EVENT_TTS_CHANGED = 0x10;
PUBLIC const int LB_SYS_EVENT_PAUSED = 0x0100;
PUBLIC const int LB_SYS_EVENT_RESUMED = 0x0200;
PUBLIC const int LB_SYS_EVENT_MMC_STATUS_CHANGED = 0x0400;
PUBLIC const int LB_SYS_EVENT_DELETED = 0x0800;

PUBLIC struct livebox_desc *livebox_desc_open(const char *filename, int for_pd)
{
    return NULL;
}

PUBLIC int livebox_desc_close(struct livebox_desc *handle)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

PUBLIC int livebox_desc_set_category(struct livebox_desc *handle, const char *id, const char *category)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

PUBLIC int livebox_desc_set_size(struct livebox_desc *handle, const char *id, int w, int h)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

PUBLIC char *livebox_util_nl2br(const char *str)
{
    return NULL;
}

PUBLIC int livebox_desc_set_id(struct livebox_desc *handle, int idx, const char *id)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

/*!
 * \return idx
 */
PUBLIC int livebox_desc_add_block(struct livebox_desc *handle, const char *id, const char *type, const char *part, const char *data, const char *option)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

PUBLIC int livebox_desc_del_block(struct livebox_desc *handle, int idx)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

PUBLIC struct livebox_buffer *livebox_acquire_buffer(const char *filename, int is_pd, int width, int height, int pixels, int auto_align, int (*handler)(struct livebox_buffer *, struct buffer_event_data *, void *), void *data)
{
    return NULL;
}

PUBLIC int livebox_request_update(const char *filename)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

PUBLIC int livebox_conf_premultiplied_alpha(void)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

PUBLIC unsigned long livebox_pixmap_id(struct livebox_buffer *handle)
{
    return 0lu;
}

PUBLIC int livebox_release_buffer(struct livebox_buffer *handle)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

PUBLIC void *livebox_ref_buffer(struct livebox_buffer *handle)
{
    return NULL;
}

PUBLIC int livebox_unref_buffer(void *buffer)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

PUBLIC int livebox_sync_buffer(struct livebox_buffer *handle)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

PUBLIC int livebox_support_hw_buffer(struct livebox_buffer *handle)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

PUBLIC int livebox_create_hw_buffer(struct livebox_buffer *handle)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

PUBLIC int livebox_destroy_hw_buffer(struct livebox_buffer *handle)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

PUBLIC void *livebox_buffer_hw_buffer(struct livebox_buffer *handle)
{
    return NULL;
}

PUBLIC int livebox_buffer_pre_render(struct livebox_buffer *handle)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

PUBLIC int livebox_buffer_post_render(struct livebox_buffer *handle)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

PUBLIC int livebox_buffer_stride(struct livebox_buffer *handle)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

PUBLIC int livebox_content_is_updated(const char *filename, int is_pd)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

PUBLIC int livebox_request_close_pd(const char *pkgname, const char *id, int reason)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

PUBLIC int livebox_freeze_scroller(const char *pkgname, const char *id)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

PUBLIC int livebox_release_scroller(const char *pkgname, const char *id)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

PUBLIC int livebox_set_extra_info(const char *id, const char *content, const char *title, const char *icon, const char *name)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

/* End of a file */
