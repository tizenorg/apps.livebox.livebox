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

#ifndef __LIVEBOX_HELPER_H
#define __LIVEBOX_HELPER_H

#ifdef __cplusplus
extern "C" {
#endif

struct livebox_buffer; /* Defined by provider */

/*!
 * \brief
 * Return values of livebox programming interfaces.
 */
extern const int DONE; /*!< Operation is successfully done */
extern const int NEED_TO_SCHEDULE; /*!< Need to call the livebox_need_to_update and livebox_update_content */
extern const int OUTPUT_UPDATED; /*!< Contents is updated */
extern const int NEED_TO_CREATE; /*!< Need to create a new instance */
extern const int NEED_TO_DESTROY; /*!< Need to destroy this instance */

extern const int LB_SYS_EVENT_FONT_CHANGED; /*!< System font is changed */
extern const int LB_SYS_EVENT_LANG_CHANGED; /*!< System language is changed */
extern const int LB_SYS_EVENT_PAUSED;
extern const int LB_SYS_EVENT_RESUMED;

#define LB_DESC_TYPE_TEXT "text"
#define LB_DESC_TYPE_IMAGE "image"
#define LB_DESC_TYPE_SIGNAL "signal"
#define LB_DESC_TYPE_INFO "info"
#define LB_DESC_TYPE_DRAG "drag"
#define LB_DESC_TYPE_SCRIPT "script"

/*!
 * \brief
 * Extra event informatino for livebox_content_event interface function
 */
#ifndef __PROVIDER_H
struct event_info {
	struct {
		double x;
		double y;
		int down;
	} pointer; /*!< Down/Up state and mouse position */

	struct {
		double sx;
		double sy;
		double ex;
		double ey;
	} part; /*!< Geometry of event received part */
};
#endif

struct livebox_desc;

/*!
 * \brief Update a description file.
 * \param[in] filename
 * \param[in] for_pd
 * \return handle
 */
extern struct livebox_desc *livebox_desc_open(const char *filename, int for_pd);

/*!
 * \brief Complete the description file updating
 * \param[in] handle
 * \return int
 */
extern int livebox_desc_close(struct livebox_desc *handle);

/*!
 * \brief Set the category information of current description data
 * \param[in] handle
 * \param[in] id
 * \param[in] category
 * \return int
 */
extern int livebox_desc_set_category(struct livebox_desc *handle, const char *id, const char *category);

/*!
 * \brief Set the content size
 * \param[in] handle
 * \param[in] id
 * \param[in] w
 * \param[in] h
 * \return int
 */
extern int livebox_desc_set_size(struct livebox_desc *handle, const char *id, int w, int h);

/*!
 * \breif Set the target id of given block
 *        Only available for the script block
 * \param[in] handle
 * \param[in] idx
 * \param[in] id
 * \return ret
 */
extern int livebox_desc_set_id(struct livebox_desc *handle, int idx, const char *id);

/*!
 * \brief Add a new block
 * \param[in] handle
 * \param[in] id ID of source script object
 * \param[in] type image|text|script|signal|...
 * \param[in] part target part to update with given content(data)
 * \param[in] data content for specified part
 * \param[in] group script group name
 * \return idx Index of current description block
 */
extern int livebox_desc_add_block(struct livebox_desc *handle, const char *id, const char *type, const char *part, const char *data, const char *group);

/*!
 * \brief Delete a added block
 * \param[in] handle
 * \param[in] idx Index of added block
 * \return int
 */
extern int livebox_desc_del_block(struct livebox_desc *handle, int idx);

/*!
 * \brief Replace '\n' with '<br>'
 * \param[in] str Source string
 * \return char* allocated string
 */
extern char *livebox_util_nl2br(const char *str);


/*!
 * Interfaces for direct buffer management (Available only for the PD area)
 */


#ifndef __PROVIDER_BUFFER_H
/*!
 * \NOTE
 * This enumeration value should be sync'd with provider
 */
enum buffer_event {
	BUFFER_EVENT_ENTER, /*!< */
	BUFFER_EVENT_LEAVE, /*!< */
	BUFFER_EVENT_DOWN, /*!< */
	BUFFER_EVENT_MOVE, /*!< */
	BUFFER_EVENT_UP, /*!< */
};
#endif

/*!
 * \brief
 * \param[in] filename
 * \param[in] width
 * \param[in] height
 * \param[in] handler
 * \param[in] data
 * \return handler
 */
extern struct livebox_buffer *livebox_acquire_buffer(const char *filename, int is_pd, int width, int height, int (*handler)(struct livebox_buffer *, enum buffer_event, double, double, double, void *), void *data);

/*!
 * \param[in] filename
 * \return pixmap ID
 */
extern unsigned long livebox_pixmap_id(struct livebox_buffer *handle);

/*!
 * \brief
 * \param[in] handle
 * \return int
 */
extern int livebox_release_buffer(struct livebox_buffer *handle);

/*!
 * \brief
 * \param[in] handle
 * \return void* buffer
 */
extern void *livebox_ref_buffer(struct livebox_buffer *handle);

/*!
 * \brief
 * \param[in] buffer
 * \return int
 */
extern int livebox_unref_buffer(void *buffer);

/*!
 * \brief
 * \param[in] handler
 * \return int
 */
extern int livebox_sync_buffer(struct livebox_buffer *handle);

/*!
 * \brief
 * \param[in] filename
 * \return int
 */
extern int livebox_request_update(const char *filename);

#ifdef __cplusplus
}
#endif

#endif
/* End of a file */
