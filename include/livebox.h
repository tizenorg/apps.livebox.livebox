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

#ifndef __LIVEBOX_HELPER_H
#define __LIVEBOX_HELPER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <Evas.h>

struct livebox_buffer; /* Defined by provider */

/*!
 * \brief
 * Return values of livebox programming interfaces.
 */
extern const int DONE; /*!< Operation is successfully done */
extern const int OUTPUT_UPDATED; /*!< Contents is updated */

extern const int NEED_TO_SCHEDULE; /*!< Need to call the livebox_need_to_update and livebox_update_content */
extern const int NEED_TO_CREATE; /*!< Need to create a new instance */
extern const int NEED_TO_DESTROY; /*!< Need to destroy this instance */
extern const int NEED_TO_UPDATE;

extern const int USE_NET; /*!< Using network */

extern const int LB_SYS_EVENT_FONT_CHANGED; /*!< System font is changed */
extern const int LB_SYS_EVENT_LANG_CHANGED; /*!< System language is changed */
extern const int LB_SYS_EVENT_TIME_CHANGED; /*!< System time is changed */
extern const int LB_SYS_EVENT_REGION_CHANGED; /*!< Region changed */
extern const int LB_SYS_EVENT_PAUSED;
extern const int LB_SYS_EVENT_RESUMED;
extern const int LB_SYS_EVENT_MMC_STATUS_CHANGED; /*!< MMC Status change event */

#define LB_DESC_TYPE_COLOR "color"
#define LB_DESC_TYPE_TEXT "text"
#define LB_DESC_TYPE_IMAGE "image"
#define LB_DESC_TYPE_SIGNAL "signal"
#define LB_DESC_TYPE_INFO "info"
#define LB_DESC_TYPE_DRAG "drag"
#define LB_DESC_TYPE_SCRIPT "script"
#define LB_DESC_TYPE_ACCESS "access"

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
 * \param[in] option option for the block. (script: group, image: option, ...)
 * \return idx Index of current description block
 */
extern int livebox_desc_add_block(struct livebox_desc *handle, const char *id, const char *type, const char *part, const char *data, const char *option);

/*!
 * \brief Delete a added block
 * \param[in] handle
 * \param[in] idx Index of added block
 * \return int
 */
extern int livebox_desc_del_block(struct livebox_desc *handle, int idx);

/*!
 * \brief Notify the updated content to the provider.
 * \param[in] id Instance Id
 * \param[in] is_pd 1 if call for PD or 0(LB).
 * \return int
 */
extern int livebox_content_is_updated(const char *id, int is_pd);

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
	BUFFER_EVENT_ENTER, /*!< Mouse cursor enter */
	BUFFER_EVENT_LEAVE, /*!< Mouse cursor leave */
	BUFFER_EVENT_DOWN, /*!< Mouse down */
	BUFFER_EVENT_MOVE, /*!< Mouse move */
	BUFFER_EVENT_UP, /*!< Mouse up */

	BUFFER_EVENT_KEY_DOWN, /*!< Key down */
	BUFFER_EVENT_KEY_UP, /*!< Key up */

	BUFFER_EVENT_HIGHLIGHT,
	BUFFER_EVENT_HIGHLIGHT_NEXT,
	BUFFER_EVENT_HIGHLIGHT_PREV,
	BUFFER_EVENT_ACTIVATE,
	BUFFER_EVENT_ACTION_UP,
	BUFFER_EVENT_ACTION_DOWN,
	BUFFER_EVENT_SCROLL_UP,
	BUFFER_EVENT_SCROLL_MOVE,
	BUFFER_EVENT_SCROLL_DOWN,
	BUFFER_EVENT_UNHIGHLIGHT
};
#endif

/*!
 * \brief Acquire a buffer for PD or LB, Currently, we only supporting the PD.
 * \param[in] id Id of a livebox instance
 * \param[in] is_pd 1 for PD or 0 for livebox
 * \param[in] width Width
 * \param[in] height Height
 * \param[in] handler Event handling callback
 * \param[in] data user data for event handling callback
 * \return handler Buffer handle
 */
extern struct livebox_buffer *livebox_acquire_buffer(const char *id, int is_pd, int width, int height, int (*handler)(struct livebox_buffer *, enum buffer_event, double, double, double, void *), void *data);

/*!
 * \brief Acquire the ID of pixmap resource
 *        Only if the provider uses pixmap for providing render buffer.
 * \param[in] handle Buffer handle
 * \return pixmap ID if succeed or 0lu
 * \see livebox_acquire_buffer
 */
extern unsigned long livebox_pixmap_id(struct livebox_buffer *handle);

/*!
 * \brief
 * \param[in] handle Buffer handle
 * \return int
 * \see livebox_acquire_buffer
 */
extern int livebox_release_buffer(struct livebox_buffer *handle);

/*!
 * \brief Get the address of S/W render buffer.
 *        If you try to use this, after create_hw_buffer, you will get NULL
 * \param[in] handle Buffer handle
 * \return void* address of the render buffer
 * \see livebox_unref_buffer
 */
extern void *livebox_ref_buffer(struct livebox_buffer *handle);

/*!
 * \brief Release the S/W render buffer.
 * \param[in] void* Address of render buffer
 * \return int 0 if succeed or errno < 0
 * \see livebox_ref_buffer
 */
extern int livebox_unref_buffer(void *buffer);

/*!
 * \brief Sync the updated buffer
 *        This is only needed for non-H/W accelerated buffer
 * \param[in] handler Buffer handle
 * \return int 0 if succeed or errno < 0
 * \see livebox_acquire_buffer
 */
extern int livebox_sync_buffer(struct livebox_buffer *handle);

/*!
 * \brief Request schedule the update operation to a provider.
 * \param[in] id Livebox Id
 * \return int 0 if succeed or errno < 0
 */
extern int livebox_request_update(const char *id);

/*!
 * \brief Checking wether the livebox support H/W acceleration or not.
 * \param[in] handle Buffer handle.
 * \return 1 if support or 0
 * \see livebox_acquire_buffer
 */
extern int livebox_support_hw_buffer(struct livebox_buffer *handle);

/*!
 * \brief Create the H/W accelerated buffer.
 * \param[in] handle Buffer handle
 * \return 0 if succeed to create it or errno < 0
 * \see livebox_support_hw_buffer
 */
extern int livebox_create_hw_buffer(struct livebox_buffer *handle);

/*!
 * \brief Destroy the H/W accelerated buffer.
 * \param[in] handle Buffer handle
 * \return 0 if succeed to destroy it or errno < 0
 * \see livebox_create_hw_buffer
 */
extern int livebox_destroy_hw_buffer(struct livebox_buffer *handle);

/*!
 * \brief Get the address of accelerated H/W buffer
 * \param[in] handle Buffer handle
 * \return void
 * \see livebox_create_hw_buffer
 */
extern void *livebox_buffer_hw_buffer(struct livebox_buffer *handle);

/*!
 * \brief Pre-processing for rendering content.
 *        This is only needed for accessing H/W accelerated buffer.
 * \param[in] handle Buffer handle
 * \return 0 if succeed or errno < 0
 * \see livebox_support_hw_buffer
 */
extern int livebox_buffer_pre_render(struct livebox_buffer *handle);

/*!
 * \brief Post-processing for rendering content.
 * \param[in] handle Buffer handle
 * \return 0 if succeed or errno < 0
 * \see livebox_support_hw_buffer
 */
extern int livebox_buffer_post_render(struct livebox_buffer *handle);

/*
extern Evas_Object *livebox_snapshot_window_add(const char *id, int size_type);
extern int livebox_snapshot_window_flush(Evas_Object *snapshot_win, double timeout, void (*flush_cb)(Evas_Object *snapshot_window, const char *id, int status, void *data), void *data);
extern int livebox_snapshot_window_del(Evas_Object *snapshot_win);
extern Evas_Object *livebox_virtual_window_add(const char *id, int width, int height);
extern int livebox_virtual_window_del(Evas_Object *virtual_win);
*/

// End of a file
#ifdef __cplusplus
}
#endif

#endif
/* End of a file */
