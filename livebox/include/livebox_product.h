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

#ifndef __LIVEBOX_HELPER_PRODUCT_H
#define __LIVEBOX_HELPER_PRODUCT_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file livebox_product.h
 * @brief This file declares API of liblivebox library for product
 */

/**
 * @addtogroup CAPI_LIVEBOX_HELPER_PRODUCT_MODULE
 * @{
 */

/**
 * @brief Structure for a livebox buffer defined by a provider.
 */
struct livebox_buffer;

extern const int DONE; /**<Return values of livebox programming interfaces: Operation is successfully done.*/

extern const int OUTPUT_UPDATED; /**<Return values of livebox programming interfaces: Contents are updated.*/

extern const int NEED_TO_SCHEDULE; /**<Return values of livebox programming interfaces: Need to call the livebox_need_to_update and livebox_update_content.*/

extern const int FORCE_TO_SCHEDULE; /**<Return values of livebox programming interfaces: Need to do force update.*/

extern const int NEED_TO_CREATE; /**<Return values of livebox programming interfaces: Need to create a new instance.*/

extern const int NEED_TO_DESTROY; /**<Return values of livebox programming interfaces: Need to destroy this instance.*/

extern const int NEED_TO_UPDATE; /**<Return values of livebox programming interfaces: Need to update.*/

extern const int USE_NET; /**<Return values of livebox programming interfaces: Using network.*/

extern const int LB_SYS_EVENT_FONT_CHANGED; /**<System event type: System font is changed.*/

extern const int LB_SYS_EVENT_LANG_CHANGED; /**<System event type: System language is changed.*/

extern const int LB_SYS_EVENT_TIME_CHANGED; /**<System event type: System time is changed.*/

extern const int LB_SYS_EVENT_REGION_CHANGED; /**<System event type: Region changed. */

extern const int LB_SYS_EVENT_TTS_CHANGED; /**< System event type: Accessibility changed. */

extern const int LB_SYS_EVENT_PAUSED; /**<System event type: Livebox is paused.*/

extern const int LB_SYS_EVENT_RESUMED; /**<System event type: Livebox is resumed.*/

extern const int LB_SYS_EVENT_MMC_STATUS_CHANGED; /**<System event type: MMC Status change event.*/

extern const int LB_SYS_EVENT_DELETED;/**<System event type: Livebox instance is removed from a viewer.*/

/**
 * @brief Structure for extra event information for livebox_content_event interface function.
 */
#if !defined(__SCRIPT_EVENT_INFO)
#define __SCRIPT_EVENT_INFO
/**
 * @brief
 * Text signal & Content event uses this data structure.
 */
struct event_info {
        struct pointer {
                double x; /**< X value of current mouse(touch) position */
                double y; /**< Y value of current mouse(touch) position */
                int down; /**< Is it pressed(1) or not(0) */
        } pointer;

        struct part {
                double sx; /**< Pressed object's left top X */
                double sy; /**< Pressed object's left top Y */
                double ex; /**< Pressed object's right bottom X */
                double ey; /**< Pressed object's right bottom Y */
        } part;
};
#endif

/**
 * @brief Notifies the updated content to the provider.
 * @details Forcedly notifies the updated contents.
 *    This function can be used from async callback function to notify the updated content.
 * @param[in] id Instance Id which is passed to you via the first parameter of every livebox_XXXX interface functions
 * @param[in] is_pd 1 for updating content of PD or 0(for content of LB)
 * @privlevel N/P
 * @return int type
 * @retval #LB_STATUS_ERROR_MEMORY Not enough memory
 * @retval #LB_STATUS_ERROR_INVALID Invalid argument
 * @retval #LB_STATUS_ERROR_IO I/O failed, Cannot access given resource file(id)
 * @retval #LB_STATUS_SUCCESS Successfully notified
 * @pre This API only can be used after loaded into the data-provider-slave process.
 */
extern int livebox_content_is_updated(const char *id, int is_pd);

/**
 * @brief Replaces "\n" (new line character) with &lt;br&gt; tag.
 * @details This utility function is used to replace all NEW LINE CHARACTER ("\n") with &lt;br&gt; tag.
 * @param[in] str Source string
 * @privlevel N/P
 * @return char * type
 * @retval string Allocted in the heap
 * @retval @c NULL If it fails to replace string
 * @post Returned string must has to be free'd by user manually.
 */
extern char *livebox_util_nl2br(const char *str);

/*
 * Interfaces for direct buffer management (Available only for the PD area).
 */
#ifndef __PROVIDER_BUFFER_H
/**
 * @brief Enumeration for value should be sync'd with provider.
 */
enum buffer_event {
	BUFFER_EVENT_ENTER, /**< Mouse cursor enter */
	BUFFER_EVENT_LEAVE, /**< Mouse cursor leave */
	BUFFER_EVENT_DOWN, /**< Mouse down */
	BUFFER_EVENT_MOVE, /**< Mouse move */
	BUFFER_EVENT_UP, /**< Mouse up */

	BUFFER_EVENT_KEY_DOWN, /**< Key down */
	BUFFER_EVENT_KEY_UP, /**< Key up */
	BUFFER_EVENT_KEY_FOCUS_IN, /**< Key focus in */
	BUFFER_EVENT_KEY_FOCUS_OUT, /**< Key focus out */

	BUFFER_EVENT_HIGHLIGHT, /**< Accessibility - Highlight */
	BUFFER_EVENT_HIGHLIGHT_NEXT, /**< Accessibility - Move highlight to next */
	BUFFER_EVENT_HIGHLIGHT_PREV, /**< Accessibility - Move highlight to prev */
	BUFFER_EVENT_ACTIVATE, /**< Accessibility - Activate the highlighted object */
	BUFFER_EVENT_ACTION_UP, /**< Accessibility - Make UP action */
	BUFFER_EVENT_ACTION_DOWN, /**< Accessibility - Make Down action */
	BUFFER_EVENT_SCROLL_UP, /**< Accessibility - Scroll up */
	BUFFER_EVENT_SCROLL_MOVE, /**< Accessibility - Scroll move */
	BUFFER_EVENT_SCROLL_DOWN, /**< Accessibility - Scroll down */
	BUFFER_EVENT_UNHIGHLIGHT, /**< Accessibility - Remove highlight */

	BUFFER_EVENT_ON_HOLD,	/**< To prevent from generating mouse clicked event */
	BUFFER_EVENT_OFF_HOLD,	/**< Stopped holding. */
	BUFFER_EVENT_ON_SCROLL, /**< On scrolling */
	BUFFER_EVENT_OFF_SCROLL, /**< Scrolling stopped */

	/* Accessibility event */
	BUFFER_EVENT_VALUE_CHANGE,
	BUFFER_EVENT_MOUSE,
	BUFFER_EVENT_BACK,
	BUFFER_EVENT_OVER,
	BUFFER_EVENT_READ,
	BUFFER_EVENT_ENABLE,
	BUFFER_EVENT_DISABLE
};

struct buffer_event_data {
	enum buffer_event type; /**< Mouse, Keyboard, Accessibility event type */
	double timestamp; /**< Timestamp of events */

	union input_data {
		struct mouse {
			int x; /**< If the event type is for accessibility or mouse, coordinates of X for pointer */
			int y; /**< If the event type is for accessibility or mouse, coordinates of Y for pointer */
		} pointer;

		struct access {
			int x;
			int y;
			unsigned int mouse_type;
			unsigned int action_type;
			unsigned int action_by;
			int cycle;
		} access;

		unsigned int keycode; /**< If the event type is for keyboard, the value of keycode */
	} info;
};
#endif

/**
 * @brief Acquires a buffer for PD or LB, currently only the PD is supported.
 * @param[in] id Instance Id which is passed to you via the first parameter of every livebox_XXXX interface functions
 * @param[in] is_pd 1 for PD or 0 for livebox
 * @param[in] width Width in pixel
 * @param[in] height Height in pixel
 * @param[in] pixels Size of a pixel in bytes
 * @param[in] handler Event handling callback
 * @param[in] data User data for event handling callback
 * @privlevel platform
 * @privilege %http://developer.samsung.com/privilege/core/dynamicbox.provider
 * @return Buffer handle
 * @retval @c NULL Failed to acquire buffer
 * @retval handler Handle object
 * @pre Given instance must be specify its type as buffer. or this API will be fail.
 * @post Allocated buffer object must be released via livebox_release_buffer().
 * @see livebox_acquire_buffer()
 */
extern struct livebox_buffer *livebox_acquire_buffer(const char *id, int is_pd, int width, int height, int pixels, int auto_align, int (*handler)(struct livebox_buffer *, struct buffer_event_data *, void *), void *data);

/**
 * @brief Acquires the ID of pixmap resource.
 * @details Only if the provider uses pixmap for providing render buffer.
 * @remarks Pixmap Id can be destroyed if you call the livebox_release_buffer(). Then this pixmap Id is not guaranteed to use.
 * @param[in] handle Buffer handle
 * @privlevel N/P
 * @return unsigned log type
 * @retval positive Pixmap Id
 * @retval 0 Failed to get pixmap Id
 * @pre The system must be set to use the pixmap method for content sharing.
 * @see livebox_acquire_buffer()
 */
extern unsigned long livebox_pixmap_id(struct livebox_buffer *handle);

/**
 * @brief Releases the buffer of livebox.
 * @param[in] handle Buffer handle
 * @privlevel platform
 * @privilege %http://developer.samsung.com/privilege/core/dynamicbox.provider
 * @return int type
 * @retval #LB_STATUS_ERROR_INVALID Invalid argument
 * @retval #LB_STATUS_SUCCESS Successfully released
 * @pre Handle must be created using livebox_acquire_buffer().
 * @see livebox_acquire_buffer()
 */
extern int livebox_release_buffer(struct livebox_buffer *handle);

/**
 * @brief Gets the address of S/W render buffer.
 * @details If you try to use this, after livebox_create_hw_buffer(), you will get @c NULL.
 * @param[in] handle Buffer handle
 * @privlevel platform
 * @privilege %http://developer.samsung.com/privilege/core/dynamicbox.provider
 * @return void* address of the render buffer
 * @retval @c NULL If it falis to get buffer address
 * @retval address If it succeed to get the buffer address
 * @see livebox_unref_buffer()
 */
extern void *livebox_ref_buffer(struct livebox_buffer *handle);

/**
 * @brief Releases the S/W render buffer.
 * @param[in] buffer Address of render buffer
 * @privlevel platform
 * @privilege %http://developer.samsung.com/privilege/core/dynamicbox.provider
 * @return int type
 * @retval #LB_STATUS_ERROR_INVALID Invalid handle
 * @retval #LB_STATUS_SUCCESS Successfully unreference
 * @pre livebox_ref_buffer() must be called.
 * @see livebox_ref_buffer()
 */
extern int livebox_unref_buffer(void *buffer);

/**
 * @brief Synchronizes the updated buffer.
 * @details This is only needed for non-H/W accelerated buffer.
 * @param[in] handle Buffer handle
 * @privlevel platform
 * @privilege %http://developer.samsung.com/privilege/core/dynamicbox.provider
 * @return int type
 * @retval #LB_STATUS_SUCCESS Successfully sync'd
 * @retval #LB_STATUS_ERROR_INVALID Invalid argument
 * @see livebox_acquire_buffer()
 */
extern int livebox_sync_buffer(struct livebox_buffer *handle);

/**
 * @brief Requests to schedule the update operation to a provider.
 * @param[in] id Instance Id which is passed to you via the first parameter of every livebox_XXXX interface functions
 * @privlevel platform
 * @privilege %http://developer.samsung.com/privilege/core/dynamicbox.provider
 * @return If succes returns 0 or return less than 0
 * @retval #LB_STATUS_SUCCESS Successfully triggered
 * @retval #LB_STATUS_ERROR_INVALID Invalid argument
 * @retval #LB_STATUS_ERROR_MEMORY Not enough memory
 * @retval #LB_STATUS_ERROR_NOT_EXIST Given id instance is not exist
 */
extern int livebox_request_update(const char *id);

/**
 * @brief Checks whether the livebox supports H/W acceleration or not.
 * @param[in] handle Buffer handle
 * @privlevel N/P
 * @return int type
 * @retval 1 If the buffer handle support the H/W acceleration buffer
 * @retval 0 If it doesn not supported
 * @see livebox_acquire_buffer()
 */
extern int livebox_support_hw_buffer(struct livebox_buffer *handle);

/**
 * @brief Creates the H/W accelerated buffer.
 * @param[in] handle Buffer handle
 * @privlevel platform
 * @privilege %http://developer.samsung.com/privilege/core/dynamicbox.provider
 * @return int type
 * @retval #LB_STATUS_ERROR_ALREADY H/W buffer is already created
 * @retval #LB_STATUS_ERROR_INVALID Invalid argument
 * @retval #LB_STATUS_SUCCESS Successfully created
 * @pre Must be checked whether the H/W buffer is supported or not.
 * @post Must be destroyed if it is not necessary.
 * @see livebox_support_hw_buffer()
 */
extern int livebox_create_hw_buffer(struct livebox_buffer *handle);

/**
 * @brief Destroys the H/W accelerated buffer.
 * @param[in] handle Buffer handle
 * @privlevel platform
 * @privilege %http://developer.samsung.com/privilege/core/dynamicbox.provider
 * @return int type
 * @retval #LB_STATUS_SUCCESS Successfully destroyed
 * @retval #LB_STATUS_ERROR_INVALID Invalid argument
 * @pre Must be created h/w buffer using livebox_create_hw_buffer.
 * @see livebox_create_hw_buffer()
 */
extern int livebox_destroy_hw_buffer(struct livebox_buffer *handle);

/**
 * @brief Gets the address of the accelerated H/W buffer.
 * @param[in] handle Buffer handle
 * @privlevel platform
 * @privilege %http://developer.samsung.com/privilege/core/dynamicbox.provider
 * @return void* type
 * @retval @c NULL Failed to get H/W accelerated buffer address
 * @retval addr H/W accelerated buffer address
 * @see livebox_create_hw_buffer()
 */
extern void *livebox_buffer_hw_buffer(struct livebox_buffer *handle);

/**
 * @brief Get the stride information from the buffer info
 * @param[in] handle Buffer handle
 * @privlevel N/P
 * @return int stride size
 * @retval positive_value length of stride
 * @return LB_STATUS_ERROR_INVALID Invalid parameter
 * @return LB_STATUS_ERROR_FAULT Unrecoverable error
 * @see livebox_create_hw_buffer()
 */
extern int livebox_buffer_stride(struct livebox_buffer *handle);

/**
 * @brief Pre-processes for rendering content.
 * @details This is only needed for accessing H/W accelerated buffer.
 * @param[in] handle Buffer handle
 * @privlevel platform
 * @privilege %http://developer.samsung.com/privilege/core/dynamicbox.provider
 * @return int type
 * @retval #LB_STATUS_ERROR_INVALID Invalid handle
 * @retval #LB_STATUS_SUCCESS Successfully done
 * @post livebox_buffer_post_render must be called after the rendering (accessing buffer) is completed.
 * @see livebox_support_hw_buffer()
 * @see livebox_buffer_post_render()
 */
extern int livebox_buffer_pre_render(struct livebox_buffer *handle);

/**
 * @brief Post-processes for rendering content.
 * @param[in] handle Buffer handle
 * @privlevel platform
 * @privilege %http://developer.samsung.com/privilege/core/dynamicbox.provider
 * @return If succes returns 0 or return less than 0
 * @retval #LB_STATUS_SUCCESS If succeed
 * @retval #LB_STATUS_ERROR_INVALID Invalid argument
 * @pre livebox_buffer_pre_render must be called.
 * @see livebox_support_hw_buffer()
 * @see livebox_buffer_pre_render()
 */
extern int livebox_buffer_post_render(struct livebox_buffer *handle);

/**
 * @brief Sends a request to the viewer (homescreen) to close the PD.
 * @details Even though you call this function. the homescreen can ignore it.
 * @param[in] pkgname Livebox Package Id
 * @param[in] id Instance Id which is passed to you via the first parameter of every livebox_XXXX interface functions
 * @param[in] reason LB_STATUS_SUCCESS(0)
 * @privlevel platform
 * @privilege %http://developer.samsung.com/privilege/core/dynamicbox.provider
 * @return int type
 * @retval #LB_STATUS_ERROR_INVALID Invalid parameters
 * @retval #LB_STATUS_ERROR_MEMORY Out of memory
 * @retval #LB_STATUS_ERROR_FAULT Failed to send a request
 * @retval #LB_STATUS_SUCCESS Successfully requested
 */
extern int livebox_request_close_pd(const char *pkgname, const char *id, int reason);

/**
 * @brief Send a freeze request to the viewer (homescreen)
 * @details
 *        The viewer will get this request via event callback.
 *        Then it should freeze its scroller or stop moving the livebox.
 * @remarks If the viewer doesn't care this request, this will has no effect.
 * @param[in] pkgname Livebox Package Id
 * @param[in] id Instance Id which is passed to you via the first parameter of every livebox_XXXX interface functions
 * @privlevel platform
 * @privilege %http://developer.samsung.com/privilege/core/dynamicbox.provider
 * @return int type
 * @retval #LB_STATUS_ERROR_INVALID Invalid parameters
 * @retval #LB_STATUS_ERROR_MEMORY Out of memory
 * @retval #LB_STATUS_ERROR_FAULT Failed to send requet
 * @retval #LB_STATUS_SUCCESS Successfully requested
 * @see livebox_release_scroller()
 */
extern int livebox_freeze_scroller(const char *pkgname, const char *id);

/**
 * @brief Send a release request to the viewer (homescreen)
 * @details
 *        The viewer will get this request via event callback.
 *        Then it should release its scroller or continue moving the livebox.
 * @remarks If the viewer doesn't care this request, this will has no effect.
 * @param[in] pkgname Livebox Package Id
 * @param[in] id Instance Id which is passed to you via the first parameter of every livebox_XXXX interface functions
 * @privlevel platform
 * @privilege %http://developer.samsung.com/privilege/core/dynamicbox.provider
 * @return int type
 * @retval #LB_STATUS_ERROR_INVALID Invalid parameters
 * @retval #LB_STATUS_ERROR_MEMORY Out of memory
 * @retval #LB_STATUS_ERROR_FAULT Failed to send requet
 * @retval #LB_STATUS_SUCCESS Successfully requested
 * @see livebox_freeze_scroller()
 */
extern int livebox_release_scroller(const char *pkgname, const char *id);

/**
 * @brief Get the configuration value for premultiplied alpha.
 * @details
 *	If the system uses the premultiplied alpha for content,
 *	This function will returns 1.
 * @remarks This function only works with the inhouse provider.
 * @privlevel N/P
 * @return status of premultiplied alpha.
 * @retval 1 Content rendered with premultiplied alpha.
 * @retval 0 Content doesn't rendered with premultiplied alpha.
 */
extern int livebox_conf_premultiplied_alpha(void);

/**
 * @brief Get the configuration value for AUTO Alignment of canvas buffer.
 * @details
 *      If the UIFW does not support the STRIDE information for rendering frame,
 *      This function will returns 1, then the livebox will automatically aligning the buffer.
 * @privlevel N/P
 * @return status of auto alignment.
 * @retval 1 Content will be automatically aligned by livebox.
 * @retval 0 Content will be aligned by UIFW, using stride information.
 */
extern int livebox_conf_auto_align(void);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
/* End of a file */
