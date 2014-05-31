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

/**
 * \addtogroup CAPI_LIVEBOX_HELPER_MODULE
 * \{
 */

/*!
 * \brief Defined by provider
 */
struct livebox_buffer;

/*!
 * \brief
 * Return values of livebox programming interfaces.
 */
/*!
 * \brief
 * Operation is successfully done
 */
extern const int DONE;

/*!
 * \brief
 * Contents is updated
 */
extern const int OUTPUT_UPDATED;

/*!
 * \brief
 * Need to call the livebox_need_to_update and livebox_update_content
 */
extern const int NEED_TO_SCHEDULE;

/*!
 * Need to do force update
 */
extern const int FORCE_TO_SCHEDULE;

/*!
 * \brief
 * Need to create a new instance
 */
extern const int NEED_TO_CREATE;

/*!
 * \brief
 * Need to destroy this instance
 */
extern const int NEED_TO_DESTROY;

/*!
 * \brief
 * Need to update
 */
extern const int NEED_TO_UPDATE;

/*!
 * \brief
 * Using network
 */
extern const int USE_NET;

/*!
 * \brief
 * System font is changed
 */
extern const int LB_SYS_EVENT_FONT_CHANGED;

/*!
 * \brief
 * System language is changed
 */
extern const int LB_SYS_EVENT_LANG_CHANGED;

/*!
 * \brief
 * System time is changed
 */
extern const int LB_SYS_EVENT_TIME_CHANGED;

/*!
 * \brief
 * Region changed
 */
extern const int LB_SYS_EVENT_REGION_CHANGED;

/*!
 * \brief
 * Livebox is paused
 */
extern const int LB_SYS_EVENT_PAUSED;

/*!
 * \brief
 * Livebox is resumed
 */
extern const int LB_SYS_EVENT_RESUMED;

/*!
 * \brief
 * MMC Status change event
 */
extern const int LB_SYS_EVENT_MMC_STATUS_CHANGED;

/*!
 * \brief
 * COLOR BLOCK
 */
#define LB_DESC_TYPE_COLOR "color"

/*!
 * \brief
 * TEXT BLOCK
 */
#define LB_DESC_TYPE_TEXT "text"

/*!
 * \brief
 * IMAGE BLOCK
 */
#define LB_DESC_TYPE_IMAGE "image"

/*!
 * \brief
 * SIGNAL BLOCK
 */
#define LB_DESC_TYPE_SIGNAL "signal"

/*!
 * \brief
 * INFO BLOCK
 */
#define LB_DESC_TYPE_INFO "info"

/*!
 * \brief
 * DRAG BLOCK
 */
#define LB_DESC_TYPE_DRAG "drag"

/*!
 * \brief
 * SCRIPT SCRIPT
 */
#define LB_DESC_TYPE_SCRIPT "script"

/*!
 * \brief
 * ACCESSIBILITY INFORMATION BLOCK
 */
#define LB_DESC_TYPE_ACCESS "access"

/*!
 * \brief
 * Accessibility operation.
 */
#define LB_DESC_TYPE_ACCESS_OPERATION	"access,operation"

/*!
 * \brief
 * Set highlight on specified part
 */
#define LB_DESC_DATA_ACCESS_OPERATION_HL_SET "set,hl"

/*!
 * \brief
 * Remove highlight from specified part
 */
#define LB_DESC_DATA_ACCESS_OPERATION_HL_UNSET "unset,hl"

/*!
 * \brief
 * Move the highlight to next part
 */
#define LB_DESC_DATA_ACCESS_OPERATION_HL_NEXT "next,hl"

/*!
 * \brief
 * Move the highlight to previous part
 */
#define LB_DESC_DATA_ACCESS_OPERATION_HL_PREV "prev,hl"

/*!
 * \brief
 * Reset the chain of accessibility highlight list.
 */
#define LB_DESC_DATA_ACCESS_OPERATION_RESET "reset,focus"

/*!
 * \brief
 * Specify the highligt move option.
 * If the highlight list reaches to the last object, move back to the first object.
 */
#define LB_DESC_OPTION_ACCESS_HL_LOOP "cycle"

/*!
 * \brief
 * [Default] Specify the highlight move option.
 * If the highlight list reaches to the last object(or the first object),
 * the "prev,hl", "next,hl" will be failed.
 */
#define LB_DESC_OPTION_ACCESS_HL_NOLOOP "no,cycle"

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

/*!
 * Livebox description data handle structure.
 */
struct livebox_desc;

/*!
 * \brief Update a description file.
 * \details explain api more detail
 * \remarks must be used only by Inhouse livebox
 * \param[in] filename
 * \param[in] for_pd
 * \return handle
 * \retval Handle of desc instance
 * \pre Should be loaded by data-provider-slave
 * \post Should be destroyed(flushed) using livebox_desc_close API
 * \see livebox_desc_close
 */
extern struct livebox_desc *livebox_desc_open(const char *filename, int for_pd);

/*!
 * \brief Complete the description file updating
 * \details Only if this API is called, the description data will be applied to the content of livebox(or PD).
 * \remarks must be used only by Inhouse livebox
 * \param[in] handle Handle which is created by livebox_desc_open function
 * \return int
 * \retval LB_STATUS_SUCCESS If the flushing description data is successfully done.
 * \retval LB_STATUS_ERROR_INVALID If the given handle is not valid.
 * \pre handle must be created by livebox_desc_open
 * \post Handle must not be used after return from this function
 * \see livebox_desc_open
 */
extern int livebox_desc_close(struct livebox_desc *handle);

/*!
 * \brief
 *    Set the category information of current description data
 * \details
 *    Create a new description block for updating category information of layout object
 * \remarks N/A
 * \param[in] handle Handle which is created by livebox_desc_open function
 * \param[in] id Id string of target object.
 * \param[in] category Category string that will be used by layout object
 * \return int
 * \retval LB_STATUS_ERROR_INVALID Invalid handle
 * \retval LB_STATUS_ERROR_MEMORY Memory is not enough to add this block
 * \retval index Index(>=0) of added block, which can be used by livebox_desc_set_id
 * \pre Must be called after create a handle using livebox_desc_open.
 * \post N/A
 * \see livebox_desc_set_id
 */
extern int livebox_desc_set_category(struct livebox_desc *handle, const char *id, const char *category);

/*!
 * \brief
 *    Set the content size
 * \details
 *    Set the content size of layout
 * \remarks N/A
 * \param[in] handle Handle which is created by livebox_desc_open function
 * \param[in] id Id string of target object.
 * \param[in] w Width in pixel
 * \param[in] h Height in pixel
 * \return int
 * \retval LB_STATUS_ERROR_INVALID Invalid argument
 * \retval LB_STATUS_ERROR_MEMORY Not enough memory to add a new block
 * \retval index Index(>=0) of added block Successfully added
 * \pre N/A
 * \post N/A
 * \see livebox_desc_open
 * \see livebox_desc_set_id
 */
extern int livebox_desc_set_size(struct livebox_desc *handle, const char *id, int w, int h);

/*!
 * \brief
 *    Set the target id of given block
 *    Only available for the script block
 * \details
 *    N/A
 * \remarks N/A
 * \param[in] handle Handle which is created by livebox_desc_open function
 * \param[in] idx Index of target block
 * \param[in] id Id String which will be used by other livebox_desc_XXX functions
 * \return int
 * \retval LB_STATUS_SUCCESS Id is successfully updated for given idx of desc block
 * \retval LB_STATUS_ERROR_INVALID Invalid argument
 * \retval LB_STATUS_ERROR_NOT_EXIST Given index of desc block is not exists
 * \pre desc block should be exists which has given index "idx"
 * \post specified Id string("id") can be used by other livebox_desc_XXXX functions
 * \see livebox_desc_open
 * \see livebox_desc_set_id
 */
extern int livebox_desc_set_id(struct livebox_desc *handle, int idx, const char *id);

/*!
 * \brief Add a new block
 * \details N/A
 * \remarks N/A
 * \param[in] handle Handle which is created by livebox_desc_open function
 * \param[in] id ID of source script object
 * \param[in] type image|text|script|signal|...
 * \param[in] part target part to update with given content(data)
 * \param[in] data content for specified part
 * \param[in] option option for the block. (script: group, image: option, ...)
 * \return idx Index of current description block
 * \retval LB_STATUS_ERROR_INVALID Invalid argument
 * \retval LB_STATUS_ERROR_MEMORY Not enough memory to add a new desc block
 * \retval Index index(>=0) of added desc block
 * \pre handle must be created using livebox_desc_open
 * \post N/A
 * \see livebox_desc_set_id
 * \see livebox_desc_del_block
 */
extern int livebox_desc_add_block(struct livebox_desc *handle, const char *id, const char *type, const char *part, const char *data, const char *option);

/*!
 * \brief Delete a added block
 * \details
 *    If you discard the added desc block, use this API and the index of created desc block.
 * \remarks N/A
 * \param[in] handle Handle which is created by livebox_desc_open function
 * \param[in] idx Index of added block, returned from livebox_desc_add_block, livebox_desc_set_size, livebox_desc_set_category, ...
 * \return int
 * \retval LB_STATUS_SUCCESS Successfully deleted
 * \retval LB_STATUS_ERROR_NOT_EXIST Given index of desc block is not exists
 * \pre index must be exists.
 * \post N/A
 * \see livebox_desc_add_block
 * \see livebox_desc_open
 */
extern int livebox_desc_del_block(struct livebox_desc *handle, int idx);

/*!
 * \brief Notify the updated content to the provider.
 * \details
 *    Forcely notify the updated contents.
 *    This function can be used from async callback function to notify the updated content.
 * \remarks N/A
 * \param[in] id Instance Id which is passed to you via the first parameter of every livebox_XXXX interface functions.
 * \param[in] is_pd 1 for updating content of PD or 0(for content of LB).
 * \return int
 * \retval LB_STATUS_ERROR_MEMORY Not enough memory
 * \retval LB_STATUS_ERROR_INVALID Invalid argument
 * \retval LB_STATUS_ERROR_IO I/O failed, Cannot access given resource file(id)
 * \retval LB_STATUS_SUCCESS Successfully notified
 * \pre This API only can be used after loaded into the data-provider-slave process.
 * \post N/A
 * \see N/A
 */
extern int livebox_content_is_updated(const char *id, int is_pd);

/*!
 * \brief Replace "\n"(new line character) with &lt;br&gt;
 * \details
 *    This utility function is used to replace all NEW LINE CHARACTER("\n") with &lt;br&gt; tag.
 * \remarks N/A
 * \param[in] str Source string
 * \return char *
 * \retval String that is allocted in the heap
 * \retval NULL if it fails to replace string
 * \pre N/A
 * \post Returned string must has to be free'd by user manually.
 * \see N/A
 */
extern char *livebox_util_nl2br(const char *str);


/*!
 * \brief
 * Interfaces for direct buffer management (Available only for the PD area)
 */
#ifndef __PROVIDER_BUFFER_H
/*!
 * \note
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
	BUFFER_EVENT_KEY_FOCUS_IN, /*!< Key focus in */
	BUFFER_EVENT_KEY_FOCUS_OUT, /*!< Key focus out */

	BUFFER_EVENT_HIGHLIGHT, /*!< Accessibility - Highlight */
	BUFFER_EVENT_HIGHLIGHT_NEXT, /*!< Accessibility - Move highlight to next */
	BUFFER_EVENT_HIGHLIGHT_PREV, /*!< Accessibility - Move highlight to prev */
	BUFFER_EVENT_ACTIVATE, /*!< Accessibility - Activate the highlighted object */
	BUFFER_EVENT_ACTION_UP, /*!< Accessibility - Make UP action */
	BUFFER_EVENT_ACTION_DOWN, /*!< Accessibility - Make Down action */
	BUFFER_EVENT_SCROLL_UP, /*!< Accessibility - Scroll up */
	BUFFER_EVENT_SCROLL_MOVE, /*!< Accessibility - Scroll move */
	BUFFER_EVENT_SCROLL_DOWN, /*!< Accessibility - Scroll down */
	BUFFER_EVENT_UNHIGHLIGHT, /*!< Accessibility - Remove highlight */

	BUFFER_EVENT_ON_HOLD,	/*!< To prevent from generating mouse clicked event */
	BUFFER_EVENT_OFF_HOLD,	/*!< Stopped holding. */
	BUFFER_EVENT_ON_SCROLL, /*!< On scrolling */
	BUFFER_EVENT_OFF_SCROLL /*!< Scrolling stopped */
};

struct buffer_event_data {
	enum buffer_event type; /*!< Mouse, Keyboard, Accessibility event type */
	double timestamp; /*!< Timestamp of events */

	union input_data {
		struct mouse {
			int x; /*!< If the event type is for accessibility or mouse, coordinates of X for pointer */
			int y; /*!< If the event type is for accessibility or mouse, coordinates of Y for pointer */
		} pointer;

		unsigned int keycode; /*!< If the event type is for keyboard, the value of keycode */
	} info;
};
#endif

/*!
 * \brief
 *    Acquire a buffer for PD or LB, Currently, we only supporting the PD.
 * \details N/A
 * \remarks N/A
 * \param[in] id Id of a livebox instance
 * \param[in] is_pd 1 for PD or 0 for livebox
 * \param[in] width Width in pixel
 * \param[in] height Height in pixel
 * \param[in] handler Event handling callback
 * \param[in] data user data for event handling callback
 * \return handler Buffer handle
 * \retval NULL Failed to acquire buffer
 * \retval Handle object
 * \pre
 *    Given instance must be specify its type as buffer. or this API will be fail
 * \post
 *    Allocated buffer object must be released via livebox_release_Buffer
 * \see livebox_release_buffer
 * \sa livebox_acquire_buffer_NEW
 */
extern struct livebox_buffer *livebox_acquire_buffer(const char *id, int is_pd, int width, int height, int (*handler)(struct livebox_buffer *, enum buffer_event, double, double, double, void *), void *data);
extern struct livebox_buffer *livebox_acquire_buffer_NEW(const char *id, int is_pd, int width, int height, int pixels, int (*handler)(struct livebox_buffer *, struct buffer_event_data *, void *), void *data);

/*!
 * \brief Acquire the ID of pixmap resource
 *        Only if the provider uses pixmap for providing render buffer.
 * \details N/A
 * \remarks Pixmap Id can be destroyed if you call the livebox_release_buffer. than this pixmap Id is not guaranteed to use.
 * \param[in] handle Buffer handle
 * \return unsigned
 * \retval positive Pixmap Id
 * \retval 0 failed to get pixmap Id.
 * \pre The system must be set to use the pixmap method for content sharing
 * \post N/A
 * \see livebox_acquire_buffer
 */
extern unsigned long livebox_pixmap_id(struct livebox_buffer *handle);

/*!
 * \brief Release the buffer of livebox
 * \details N/A
 * \remarks N/A
 * \param[in] handle Buffer handle
 * \return int
 * \retval LB_STATUS_ERROR_INVALID Invalid argument
 * \retval LB_STATUS_SUCCESS Successfully released
 * \pre handle must be created using livebox_acquire_buffer.
 * \post N/A
 * \see livebox_acquire_buffer
 * \sa livebox_release_buffer_NEW
 */
extern int livebox_release_buffer(struct livebox_buffer *handle);
extern int livebox_release_buffer_NEW(struct livebox_buffer *handle);

/*!
 * \brief Get the address of S/W render buffer.
 *        If you try to use this, after create_hw_buffer, you will get NULL
 * \details N/A
 * \remarks N/A
 * \param[in] handle Buffer handle
 * \return void* address of the render buffer
 * \retval NULL if it falis to get buffer address
 * \retval address if it succeed to get the buffer address
 * \pre N/A
 * \post N/A
 * \see livebox_unref_buffer
 */
extern void *livebox_ref_buffer(struct livebox_buffer *handle);

/*!
 * \brief Release the S/W render buffer.
 * \details N/A
 * \remarks N/A
 * \param[in] buffer Address of render buffer
 * \return int
 * \retval LB_STATUS_ERROR_INVALID Invalid handle
 * \retval LB_STATUS_SUCCESS Successfully unref'd
 * \pre livebox_ref_buffer must be called
 * \post N/A
 * \see livebox_ref_buffer
 */
extern int livebox_unref_buffer(void *buffer);

/*!
 * \brief Sync the updated buffer
 *        This is only needed for non-H/W accelerated buffer
 * \details N/A
 * \remarks N/A
 * \param[in] handle Buffer handle
 * \return int
 * \retval LB_STATUS_SUCCESS Successfully sync'd
 * \retval LB_STATUS_ERROR_INVALID Invalid argument
 * \pre N/A
 * \post N/A
 * \see livebox_acquire_buffer
 */
extern int livebox_sync_buffer(struct livebox_buffer *handle);

/*!
 * \brief Request schedule the update operation to a provider.
 * \details
 * \remarks N/A
 * \param[in] id Livebox Id
 * \return int 0 if succeed or errno < 0
 * \retval LB_STATUS_SUCCESS Successfully triggered
 * \retval LB_STATUS_ERROR_INVALID Invalid argument
 * \retval LB_STATUS_ERROR_MEMORY Not enough memory
 * \retval LB_STATUS_ERROR_NOT_EXIST Given id instance is not exist
 * \pre N/A
 * \post N/A
 * \see N/A
 */
extern int livebox_request_update(const char *id);

/*!
 * \brief Checking wether the livebox support H/W acceleration or not.
 * \details
 * \remarks N/A
 * \param[in] handle Buffer handle.
 * \return int
 * \retval 1 if the buffer handle support the H/W acceleration buffer
 * \retval 0 if it doesn not supported
 * \pre N/A
 * \post N/A
 * \see livebox_acquire_buffer
 */
extern int livebox_support_hw_buffer(struct livebox_buffer *handle);

/*!
 * \brief Create the H/W accelerated buffer.
 * \details Create the H/W accelerated buffer
 * \remarks N/A
 * \param[in] handle Buffer handle
 * \return int
 * \retval LB_STATUS_ERROR_ALREADY H/W buffer is already created
 * \retval LB_STATUS_ERROR_INVALID Invalid argument
 * \retval LB_STATUS_SUCCESS Successfully created
 * \pre Must be checked whether the H/W buffer is supported or not.
 * \post Must be destroyed if it is not necessary
 * \see livebox_support_hw_buffer
 */
extern int livebox_create_hw_buffer(struct livebox_buffer *handle);

/*!
 * \brief Destroy the H/W accelerated buffer.
 * \details N/A
 * \remarks N/A
 * \param[in] handle Buffer handle
 * \return int
 * \retval LB_STATUS_SUCCESS Successfully destroyed
 * \retval LB_STATUS_ERROR_INVALID Invalid argument
 * \pre Must be created h/w buffer using livebox_create_hw_buffer
 * \post N/A
 * \see livebox_create_hw_buffer
 */
extern int livebox_destroy_hw_buffer(struct livebox_buffer *handle);

/*!
 * \brief Get the address of accelerated H/W buffer
 * \details N/A
 * \remarks N/A
 * \param[in] handle Buffer handle
 * \return void *
 * \retval NULL failed to get H/W accelerated buffer address
 * \retval addr H/W accelerated buffer address
 * \pre N/A
 * \post N/A
 * \see livebox_create_hw_buffer
 */
extern void *livebox_buffer_hw_buffer(struct livebox_buffer *handle);

/*!
 * \brief Pre-processing for rendering content.
 *        This is only needed for accessing H/W accelerated buffer.
 * \details N/A
 * \remarks N/A
 * \param[in] handle Buffer handle
 * \return int
 * \retval LB_STATUS_ERROR_INVALID Invalid handle
 * \retval LB_STATUS_SUCCESS Successfully done
 * \pre N/A
 * \post livebox_buffer_post_render must be called after the rendering (accessing buffer) is completed.
 * \see livebox_support_hw_buffer
 * \see livebox_buffer_post_render
 */
extern int livebox_buffer_pre_render(struct livebox_buffer *handle);

/*!
 * \brief Post-processing for rendering content.
 * \details N/A
 * \remarks N/A
 * \param[in] handle Buffer handle
 * \return integer 0 if succeed or errno < 0
 * \retval LB_STATUS_SUCCESS if succeed
 * \retval LB_STATUS_ERROR_INVALID Invalid argument
 * \pre livebox_buffer_pre_render must be called
 * \post N/A
 * \see livebox_support_hw_buffer
 * \see livebox_buffer_pre_render
 */
extern int livebox_buffer_post_render(struct livebox_buffer *handle);

/*!
 * \brief
 * Deprecated API set.
extern Evas_Object *livebox_snapshot_window_add(const char *id, int size_type);
extern int livebox_snapshot_window_flush(Evas_Object *snapshot_win, double timeout, void (*flush_cb)(Evas_Object *snapshot_window, const char *id, int status, void *data), void *data);
extern int livebox_snapshot_window_del(Evas_Object *snapshot_win);
*/

/*!
 * \brief Create an Evas_Object to create a elm_win object
 * \details Creating a new Evas_Object using livebox frame buffer.
 * \remarks N/A
 * \param[in] id Instance Id
 * \param[in] is_pd 1 if this object is created for PD or 0 (for LB)
 * \return Evas_Object* Evas Object
 * \retval Address Valid evas object
 * \retval NULL failed to create
 * \pre N/A
 * \post N/A
 * \see livebox_set_elm_window
 */
extern Evas_Object *livebox_get_evas_object(const char *id, int is_pd);

/*!
 * \brief Send a request to the viewer (homescreen) to close the PD.
 * \details Even though you call this function. the homescreen can ignore it.
 * \remarks
 * \param[in] pkgname Livebox Package Id
 * \param[in] id Livebox Instance Id
 * \param[in] reason LB_STATUS_SUCCESS(0)
 * \return int
 * \retval LB_STATUS_ERROR_INVALID Invalid parameters
 * \retval LB_STATUS_ERROR_MEMORY Out of memory
 * \retval LB_STATUS_ERROR_FAULT Failed to send a request
 * \retval LB_STATUS_SUCCESS Successfully requested
 * \pre N/A
 * \post N/A
 * \see
 */
extern int livebox_request_close_pd(const char *pkgname, const char *id, int reason);

/*!
 * \brief Send a freeze request to the viewer (homescreen)
 * \details
 *        The viewer will get this request via event callback.
 *        Then it should freeze its scroller or stop moving the livebox.
 * \remarks If the viewer doesn't care this request, this will has no effect.
 * \param[in] pkgname Livebox Package Id
 * \param[in] id Livebox Instance Id
 * \return int
 * \retval LB_STATUS_ERROR_INVALID Invalid parameters
 * \retval LB_STATUS_ERROR_MEMORY Out of memory
 * \retval LB_STATUS_ERROR_FAULT Failed to send requet
 * \retval LB_STATUS_SUCCESS Successfully requested
 * \pre N/A
 * \post N/A
 * \see livebox_release_scroller
 */
extern int livebox_freeze_scroller(const char *pkgname, const char *id);

/*!
 * \brief Send a release request to the viewer (homescreen)
 * \details
 *        The viewer will get this request via event callback.
 *        Then it should release its scroller or continue moving the livebox.
 * \remarks If the viewer doesn't care this request, this will has no effect.
 * \param[in] pkgname Livebox Package Id
 * \param[in] id Livebox Instance Id
 * \retval LB_STATUS_ERROR_INVALID Invalid parameters
 * \retval LB_STATUS_ERROR_MEMORY Out of memory
 * \retval LB_STATUS_ERROR_FAULT Failed to send requet
 * \retval LB_STATUS_SUCCESS Successfully requested
 * \pre N/A
 * \post N/A
 * \see livebox_freeze_scroller
 */
extern int livebox_release_scroller(const char *pkgname, const char *id);

/*!
 * \brief Get the configuration status of premultiplied alpha.
 * \details
 *	If the system uses the premultiplied alpha for content,
 *	This function will returns 1.
 * \remarks This function only works with the inhouse provider.
 * \return status of premultiplied alpha.
 * \retval 1 Content rendered with premultiplied alpha.
 * \retval 0 Content doesn't rendered with premultiplied alpha.
 */
extern int livebox_conf_premultiplied_alpha(void);

/*!
 * \}
 */

#ifdef __cplusplus
}
#endif

#endif
/* End of a file */
