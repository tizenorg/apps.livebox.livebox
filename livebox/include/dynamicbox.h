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

#ifndef __DYNAMICBOX_UILITY_H
#define __DYNAMICBOX_UILITY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <Evas.h>

/**
 * @file dynamicbox.h
 * @brief This file declares API of libdynamicbox library
 * @since_tizen 2.3
 */

/**
 * @addtogroup CAPI_DYNAMICBOX_UTILITY_MODULE
 * @{
 */

/**
 * @brief Definition for COLOR BLOCK.
 * @since_tizen 2.3
 */
#define DBOX_DESC_TYPE_COLOR "color"

/**
 * @brief Definition for TEXT BLOCK.
 * @since_tizen 2.3
 */
#define DBOX_DESC_TYPE_TEXT "text"

/**
 * @brief Definition for IMAGE BLOCK.
 * @since_tizen 2.3
 */
#define DBOX_DESC_TYPE_IMAGE "image"

/**
 * @brief Definition for SIGNAL BLOCK.
 * @since_tizen 2.3
 */
#define DBOX_DESC_TYPE_SIGNAL "signal"

/**
 * @brief Definition for INFO BLOCK.
 * @since_tizen 2.3
 */
#define DBOX_DESC_TYPE_INFO "info"

/**
 * @brief Definition for DRAG BLOCK.
 * @since_tizen 2.3
 */
#define DBOX_DESC_TYPE_DRAG "drag"

/**
 * @brief Definition for SCRIPT SCRIPT.
 * @since_tizen 2.3
 */
#define DBOX_DESC_TYPE_SCRIPT "script"

/**
 * @brief Definition for ACCESSIBILITY INFORMATION BLOCK.
 * @since_tizen 2.3
 */
#define DBOX_DESC_TYPE_ACCESS "access"

/**
 * @brief Definition for accessibility operation.
 * @since_tizen 2.3
 */
#define DBOX_DESC_TYPE_ACCESS_OPERATION	"access,operation"

/**
 * @brief Definition for set highlight on specified part.
 * @since_tizen 2.3
 */
#define DBOX_DESC_DATA_ACCESS_OPERATION_HL_SET "set,hl"

/**
 * @brief Definition for remove highlight from specified part.
 * @since_tizen 2.3
 */
#define DBOX_DESC_DATA_ACCESS_OPERATION_HL_UNSET "unset,hl"

/**
 * @brief Definition for move the highlight to next part.
 * @since_tizen 2.3
 */
#define DBOX_DESC_DATA_ACCESS_OPERATION_HL_NEXT "next,hl"

/**
 * @brief Definition for move the highlight to previous part.
 * @since_tizen 2.3
 */
#define DBOX_DESC_DATA_ACCESS_OPERATION_HL_PREV "prev,hl"

/**
 * @brief Definition for reset the chain of accessibility highlight list.
 * @since_tizen 2.3
 */
#define DBOX_DESC_DATA_ACCESS_OPERATION_RESET "reset,focus"

/**
 * @brief Definition for specify the highlight move option.
 * @details If the highlight list reaches to the last object, move back to the first object.
 * @since_tizen 2.3
 */
#define DBOX_DESC_OPTION_ACCESS_HL_LOOP "cycle"

/**
 * @brief Definition for specify the highlight move option.
 * @details If the highlight list reaches to the last object(or the first object),
 * the "prev,hl", "next,hl" will be failed.
 * @since_tizen 2.3
 */
#define DBOX_DESC_OPTION_ACCESS_HL_NOLOOP "no,cycle"

/**
 * @brief Dynamic Box description data handle.
 * @since_tizen 2.3
 */
struct dynamicbox_desc;

/**
 * @brief Updates a description file.
 * @since_tizen 2.3
 * @remarks Must be used only by Inhouse Dynamic Box.
 * @param[in] id Dynamic Box Instance Id
 * @param[in] gbar 1 for Glance Bar or 0
 * @privlevel public
 * @privilege %http://tizen.org/privilege/core/dynamicbox.provider
 * @return Handle of desc instance
 * @pre Should be loaded by data-provider-slave.
 * @post Should be destroyed(flushed) using dynamicbox_desc_close() API.
 * @see dynamicbox_desc_close()
 */
extern struct dynamicbox_desc *dynamicbox_desc_open(const char *id, int gbar);

/**
 * @brief Completes the description file update.
 * @details Only if this API is called, the description data will be applied to the content of Dynamic Box (or GBar).
 * @since_tizen 2.3
 * @remarks Must be used only by Inhouse Dynamic Box.
 * @param[in] handle Handle which is created by dynamicbox_desc_open() function
 * @privlevel public
 * @privilege %http://tizen.org/privilege/core/dynamicbox.provider
 * @return int type
 * @retval #DBOX_STATUS_ERROR_NONE If the flushing description data is successfully done
 * @retval #DBOX_STATUS_ERROR_INVALID_PARAMETER If the given handle is not valid
 * @pre Handle must be created by dynamicbox_desc_open()
 * @post Handle must not be used after return from this function.
 * @see dynamicbox_desc_open()
 */
extern int dynamicbox_desc_close(struct dynamicbox_desc *handle);

/**
 * @brief Sets the category information of current description data.
 * @details Creates a new description block for updating category information of layout object.
 * @since_tizen 2.3
 * @param[in] handle Handle which is created by dynamicbox_desc_open() function
 * @param[in] id Id string of target object
 * @param[in] category Category string that will be used by layout object
 * @return int type
 * @retval #DBOX_STATUS_ERROR_INVALID_PARAMETER Invalid handle
 * @retval #DBOX_STATUS_ERROR_OUT_OF_MEMORY Memory is not enough to add this block
 * @retval index Index(>=0) of added block, which can be used by dynamicbox_desc_set_id()
 * @pre Must be called after create a handle using dynamicbox_desc_open()
 * @see dynamicbox_desc_set_id()
 */
extern int dynamicbox_desc_set_category(struct dynamicbox_desc *handle, const char *id, const char *category);

/**
 * @brief Sets the content size.
 * @details Sets the content size of layout.
 * @since_tizen 2.3
 * @param[in] handle Handle which is created by dynamicbox_desc_open() function
 * @param[in] id Id string of target object
 * @param[in] w Width in pixel
 * @param[in] h Height in pixel
 * @return int type
 * @retval #DBOX_STATUS_ERROR_INVALID_PARAMETER Invalid argument
 * @retval #DBOX_STATUS_ERROR_OUT_OF_MEMORY Not enough memory to add a new block
 * @retval index Index(>=0) of added block Successfully added
 * @see dynamicbox_desc_open()
 * @see dynamicbox_desc_set_id()
 */
extern int dynamicbox_desc_set_size(struct dynamicbox_desc *handle, const char *id, int w, int h);

/**
 * @brief Sets the target id of the given block.
 * @details Only available for the script block.
 * @since_tizen 2.3
 * @param[in] handle Handle which is created by dynamicbox_desc_open() function
 * @param[in] idx Index of target block
 * @param[in] id Id String which will be used by other dynamicbox_desc_XXX functions
 * @return int type
 * @retval #DBOX_STATUS_ERROR_NONE Id is successfully updated for given idx of desc block
 * @retval #DBOX_STATUS_ERROR_INVALID_PARAMETER Invalid argument
 * @retval #DBOX_STATUS_ERROR_NOT_EXIST Given index of desc block is not exists
 * @pre Desc block should be exists which has given index "idx".
 * @post Specified Id string("id") can be used by other dynamicbox_desc_XXXX functions.
 * @see dynamicbox_desc_open()
 * @see dynamicbox_desc_set_id()
 */
extern int dynamicbox_desc_set_id(struct dynamicbox_desc *handle, int idx, const char *id);

/**
 * @brief Adds a new block.
 * @since_tizen 2.3
 * @param[in] handle Handle which is created by the dynamicbox_desc_open() function
 * @param[in] id ID of source script object
 * @param[in] type Type (image|text|script|signal|.. etc)
 * @param[in] part Target part to update with given content(data)
 * @param[in] data Content for specified part
 * @param[in] option Option for the block. (script: group, image: option, ...)
 * @return Index of current description block
 * @retval #DBOX_STATUS_ERROR_INVALID_PARAMETER Invalid argument
 * @retval #DBOX_STATUS_ERROR_OUT_OF_MEMORY Not enough memory to add a new desc block
 * @retval index Index(>=0) of added desc block
 * @pre Handle must be created using dynamicbox_desc_open.
 * @see dynamicbox_desc_set_id()
 * @see dynamicbox_desc_del_block()
 */
extern int dynamicbox_desc_add_block(struct dynamicbox_desc *handle, const char *id, const char *type, const char *part, const char *data, const char *option);

/**
 * @brief Deletes an added block.
 * @since_tizen 2.3
 * @details If you discard the added desc block, use this API and the index of the created desc block.
 * @param[in] handle Handle which is created by dynamicbox_desc_open() function
 * @param[in] idx Index of added block, returned from dynamicbox_desc_add_block(), dynamicbox_desc_set_size(), dynamicbox_desc_set_category(), etc
 * @return int type
 * @retval #DBOX_STATUS_ERROR_NONE Successfully deleted
 * @retval #DBOX_STATUS_ERROR_NOT_EXIST Given index of desc block is not exists
 * @pre Index must be exists.
 * @see dynamicbox_desc_add_block()
 * @see dynamicbox_desc_open()
 */
extern int dynamicbox_desc_del_block(struct dynamicbox_desc *handle, int idx);

/**
 * @brief Creates an Evas_Object to create an elm_win object.
 * @details Creating a new Evas_Object using Dynamic Box frame buffer.
 * @since_tizen 2.3
 * @param[in] id Instance Id
 * @param[in] gbar 1 if this object is created for Glance Bar or 0 (for Dynamic Box)
 * @privlevel public
 * @privilege %http://tizen.org/privilege/core/dynamicbox.provider
 * @return Evas_Object* Evas Object type
 * @retval Address Valid evas object
 * @retval @c NULL failed to create
 */
extern Evas_Object *dynamicbox_get_evas_object(const char *id, int gbar);

/**
 * @brief Set extra information for updated content.
 * @details
 *      If the provider should give some hints to the viewer,
 *      such as title, icon and name, this function can set them.
 *      provider will send those information to the viewer when the
 *      content is updated.
 * @since_tizen 2.3
 * @remarks This function only works with the buffer type inhouse provider.
 * @param[in] id Dynamic Box Instance Id
 * @param[in] content Content string that will pass to a Dynamic Box via dynamicbox_create interface when it is re-created.
 * @param[in] title Human readable text for accessitility, summarized text for representing the content of a box.
 * @param[in] icon Path of icon image file - alternative image content
 * @param[in] name Name of a box - alternative text content
 * @return int type
 * @retval #DBOX_STATUS_ERROR_OUT_OF_MEMORY Out of memory
 * @retval #DBOX_STATUS_ERROR_INVALID_PARAMETER Invalid parameters
 * @retval #DBOX_STATUS_ERROR_FAULT Unrecoverable error
 * @retval #DBOX_STATUS_ERROR_NONE Successfully sent
 */
extern int dynamicbox_set_extra_info(const char *id, const char *content, const char *title, const char *icon, const char *name);


/**
 * @internal
 * @brief Structure for a Dynamic Box buffer defined by a provider.
 * @since_tizen 2.3
 */
struct dynamicbox_buffer;

/**
 * @internal
 * @brief Return values of dynamicbox programming interfaces: Operation is successfully done.
 * @since_tizen 2.3
 */
extern const int DONE;

/**
 * @internal
 * @brief Return values of dynamicbox programming interfaces: Contents are updated.
 * @since_tizen 2.3
 */
extern const int OUTPUT_UPDATED;

/**
 * @internal
 * @brief Return values of dynamicbox programming interfaces: Need to call the dynamicbox_need_to_update and dynamicbox_update_content.
 * @since_tizen 2.3
 */
extern const int NEED_TO_SCHEDULE;

/**
 * @internal
 * @brief Return values of dynamicbox programming interfaces: Need to do force update.
 * @since_tizen 2.3
 */
extern const int FORCE_TO_SCHEDULE;

/**
 * @internal
 * @brief Return values of dynamicbox programming interfaces: Need to create a new instance.
 * @since_tizen 2.3
 */
extern const int NEED_TO_CREATE;

/**
 * @internal
 * @brief Return values of dynamicbox programming interfaces: Need to destroy this instance.
 * @since_tizen 2.3
 */
extern const int NEED_TO_DESTROY;

/**
 * @internal
 * @brief Return values of dynamicbox programming interfaces: Need to update.
 * @since_tizen 2.3
 */
extern const int NEED_TO_UPDATE;

/**
 * @internal
 * @brief Return values of dynamicbox programming interfaces: Using network.
 * @since_tizen 2.3
 */
extern const int USE_NET;

/**
 * @internal
 * @brief System event type: System font is changed.
 * @since_tizen 2.3
 */
extern const int DBOX_SYS_EVENT_FONT_CHANGED;

/**
 * @internal
 * @brief System event type: System language is changed.
 * @since_tizen 2.3
 */
extern const int DBOX_SYS_EVENT_LANG_CHANGED;

/**
 * @internal
 * @brief System event type: System time is changed.
 * @since_tizen 2.3
 */
extern const int DBOX_SYS_EVENT_TIME_CHANGED;

/**
 * @internal
 * @brief System event type: Region changed.
 * @since_tizen 2.3
 */
extern const int DBOX_SYS_EVENT_REGION_CHANGED;

/**
 * @internal
 * @brief System event type: Accessibility mode changed.
 * @since_tizen 2.3
 */
extern const int DBOX_SYS_EVENT_TTS_CHANGED;

/**
 * @internal
 * @brief System event type: Dynamic Box is paused.
 * @since_tizen 2.3
 */
extern const int DBOX_SYS_EVENT_PAUSED;

/**
 * @internal
 * @brief System event type: Dynamic Box is resumed.
 * @since_tizen 2.3
 */
extern const int DBOX_SYS_EVENT_RESUMED;

/**
 * @internal
 * @brief System event type: MMC Status change event.
 * @since_tizen 2.3
 */
extern const int DBOX_SYS_EVENT_MMC_STATUS_CHANGED;

/**
 * @internal
 * @brief System event type: Dynamic Box instance is removed from a viewer.
 * @since_tizen 2.3
 */
extern const int DBOX_SYS_EVENT_DELETED;

/**
 * @internal
 * @brief Structure for extra event information for dynamicbox_content_event interface function.
 * @since_tizen 2.3
 */
#if !defined(__SCRIPT_EVENT_INFO)
#define __SCRIPT_EVENT_INFO

/**
 * @internal
 * @brief
 * Text signal & Content event uses this data structure.
 * @since_tizen 2.3
 */
struct event_info {
        struct _pointer {
                double x; /**< X value of current mouse(touch) position */
                double y; /**< Y value of current mouse(touch) position */
                int down; /**< Is it pressed(1) or not(0) */
        } pointer;

        struct _part {
                double sx; /**< Pressed object's left top X */
                double sy; /**< Pressed object's left top Y */
                double ex; /**< Pressed object's right bottom X */
                double ey; /**< Pressed object's right bottom Y */
        } part;
};
#endif

/**
 * @internal
 * @brief Notifies the updated content to the provider.
 * @details Forcedly notifies the updated contents.
 *    This function can be used from async callback function to notify the updated content.
 * @since_tizen 2.3
 * @param[in] id Instance Id which is passed to you via the first parameter of every dynamicbox_XXXX interface functions
 * @param[in] gbar 1 for updating content of Glance Bar or 0(for content of Dynamic Box)
 * @return int type
 * @retval #DBOX_STATUS_ERROR_OUT_OF_MEMORY Not enough memory
 * @retval #DBOX_STATUS_ERROR_INVALID_PARAMETER Invalid argument
 * @retval #DBOX_STATUS_ERROR_IO_ERROR I/O failed, Cannot access given resource file(id)
 * @retval #DBOX_STATUS_ERROR_NONE Successfully notified
 * @pre This API only can be used after loaded into the data-provider-slave process.
 */
extern int dynamicbox_content_is_updated(const char *id, int gbar);

/**
 * @internal
 * @brief Replaces "\n" (new line character) with &lt;br&gt; tag.
 * @details This utility function is used to replace all NEW LINE CHARACTER ("\n") with &lt;br&gt; tag.
 * @since_tizen 2.3
 * @param[in] str Source string
 * @return char * type
 * @retval string Allocted in the heap
 * @retval @c NULL If it fails to replace string
 * @post Returned string must has to be free'd by user manually.
 */
extern char *dynamicbox_util_nl2br(const char *str);

/**
 * @internal
 * @brief Interfaces for direct buffer management (Available only for the GBar area).
 * @since_tizen 2.3
 */
#ifndef __DBOX_PROVIDER_BUFFER_H
/**
 * @internal
 * @brief Enumeration for value should be sync'd with provider.
 * @since_tizen 2.3
 */
enum dbox_buffer_event {
	DBOX_BUFFER_EVENT_ENTER, /**< Mouse cursor enter */
	DBOX_BUFFER_EVENT_LEAVE, /**< Mouse cursor leave */
	DBOX_BUFFER_EVENT_DOWN, /**< Mouse down */
	DBOX_BUFFER_EVENT_MOVE, /**< Mouse move */
	DBOX_BUFFER_EVENT_UP, /**< Mouse up */

	DBOX_BUFFER_EVENT_KEY_DOWN, /**< Key down */
	DBOX_BUFFER_EVENT_KEY_UP, /**< Key up */
	DBOX_BUFFER_EVENT_KEY_FOCUS_IN, /**< Key focus in */
	DBOX_BUFFER_EVENT_KEY_FOCUS_OUT, /**< Key focus out */

	DBOX_BUFFER_EVENT_HIGHLIGHT, /**< Accessibility - Highlight */
	DBOX_BUFFER_EVENT_HIGHLIGHT_NEXT, /**< Accessibility - Move highlight to next */
	DBOX_BUFFER_EVENT_HIGHLIGHT_PREV, /**< Accessibility - Move highlight to prev */
	DBOX_BUFFER_EVENT_ACTIVATE, /**< Accessibility - Activate the highlighted object */
	DBOX_BUFFER_EVENT_ACTION_UP, /**< Accessibility - Make UP action */
	DBOX_BUFFER_EVENT_ACTION_DOWN, /**< Accessibility - Make Down action */
	DBOX_BUFFER_EVENT_SCROLL_UP, /**< Accessibility - Scroll up */
	DBOX_BUFFER_EVENT_SCROLL_MOVE, /**< Accessibility - Scroll move */
	DBOX_BUFFER_EVENT_SCROLL_DOWN, /**< Accessibility - Scroll down */
	DBOX_BUFFER_EVENT_UNHIGHLIGHT, /**< Accessibility - Remove highlight */

	DBOX_BUFFER_EVENT_ON_HOLD,	/**< To prevent from generating mouse clicked event */
	DBOX_BUFFER_EVENT_OFF_HOLD,	/**< Stopped holding. */
	DBOX_BUFFER_EVENT_ON_SCROLL, /**< On scrolling */
	DBOX_BUFFER_EVENT_OFF_SCROLL, /**< Scrolling stopped */

	DBOX_BUFFER_EVENT_ACCESS_VALUE_CHANGE, /**< */
	DBOX_BUFFER_EVENT_ACCESS_MOUSE, /**< give mouse event to highlight object */
	DBOX_BUFFER_EVENT_ACCESS_BACK, /**< go back to a previous view ex: pop naviframe item */
	DBOX_BUFFER_EVENT_ACCESS_OVER, /**< mouse over an object */
	DBOX_BUFFER_EVENT_ACCESS_READ, /**< highlight an object */
	DBOX_BUFFER_EVENT_ACCESS_ENABLE, /**< enable highlight and read ability */
	DBOX_BUFFER_EVENT_ACCESS_DISABLE /**< disable highlight and read ability */
};

/**
 * @internal
 * @brief Event info
 * @since_tizen 2.3
 */
struct dbox_buffer_event_data {
	enum dbox_buffer_event type; /**< Mouse, Keyboard, Accessibility event type */
	double timestamp; /**< Timestamp of events */

	union _input_data {
		struct _mouse {
			int x; /**< If the event type is for accessibility or mouse, coordinates of X for pointer */
			int y; /**< If the event type is for accessibility or mouse, coordinates of Y for pointer */
		} pointer;

		struct _access {
			int x; /**< Accessibility event X coordinate */
			int y; /**< Accessibility event Y coordinate */
			unsigned int mouse_type; /**< 0: down, 1: move, 2: up | 0: cur, 1: next, 2: prev, 3: off */
			unsigned int action_type; /**< reserved for protocol */
			unsigned int action_by; /**< reserved for protocol */
			int cycle; /**< reserved for protocol */
		} access;

		unsigned int keycode; /**< If the event type is for keyboard, the value of keycode */
	} info;
};
#endif

/**
 * @internal
 * @brief Acquires a buffer for GBar or DBox, currently only the GBar is supported.
 * @since_tizen 2.3
 * @param[in] id Instance Id which is passed to you via the first parameter of every dynamicbox_XXXX interface functions
 * @param[in] gbar 1 for Glance Bar or 0 for Dynamic Box
 * @param[in] width Width in pixel
 * @param[in] height Height in pixel
 * @param[in] pixels Size of a pixel in bytes
 * @param[in] handler Event handling callback
 * @param[in] data User data for event handling callback
 * @privlevel public
 * @privilege %http://tizen.org/privilege/core/dynamicbox.provider
 * @return Buffer handle
 * @retval @c NULL Failed to acquire buffer
 * @retval handler Handle object
 * @pre Given instance must be specify its type as buffer. or this API will be fail.
 * @post Allocated buffer object must be released via dynamicbox_release_buffer().
 * @see dynamicbox_release_buffer()
 */
extern struct dynamicbox_buffer *dynamicbox_acquire_buffer(const char *id, int gbar, int width, int height, int pixels, int auto_align, int (*handler)(struct dynamicbox_buffer *, struct dbox_buffer_event_data *, void *), void *data);

/**
 * @internal
 * @brief Acquires the Id of Resource (window system).
 * @details Only if the provider uses Resource for providing render buffer.
 * @since_tizen 2.3
 * @remarks Pixmap Id can be destroyed if you call the dynamicbox_release_buffer(). Then this Resource Id is not guaranteed to use.
 * @param[in] handle Buffer handle
 * @return unsigned int Resource Id
 * @retval >0 Resource Id
 * @retval 0 Failed to get Resource Id
 * @pre The system must be set to use the Resource Id method for content sharing.
 * @see dynamicbox_acquire_buffer()
 */
extern unsigned int dynamicbox_resource_id(struct dynamicbox_buffer *handle);

/**
 * @internal
 * @brief Releases the buffer of dynamicbox.
 * @since_tizen 2.3
 * @param[in] handle Buffer handle
 * @privlevel public
 * @privilege %http://tizen.org/privilege/core/dynamicbox.provider
 * @return int type
 * @retval #DBOX_STATUS_ERROR_INVALID_PARAMETER Invalid argument
 * @retval #DBOX_STATUS_ERROR_NONE Successfully released
 * @pre Handle must be created using dynamicbox_acquire_buffer().
 * @see dynamicbox_acquire_buffer()
 */
extern int dynamicbox_release_buffer(struct dynamicbox_buffer *handle);

/**
 * @internal
 * @brief Gets the address of buffer for S/W rendering.
 * @details If you try to use this, after dynamicbox_create_hw_buffer(), you will get @c NULL.
 * @since_tizen 2.3
 * @param[in] handle Buffer handle
 * @privlevel public
 * @privilege %http://tizen.org/privilege/core/dynamicbox.provider
 * @return void* address of the render buffer
 * @retval @c NULL If it falis to get buffer address
 * @retval address If it succeed to get the buffer address
 * @see dynamicbox_unref_buffer()
 */
extern void *dynamicbox_ref_buffer(struct dynamicbox_buffer *handle);

/**
 * @internal
 * @brief Releases the S/W render buffer.
 * @since_tizen 2.3
 * @param[in] buffer Address of render buffer
 * @privlevel public
 * @privilege %http://tizen.org/privilege/core/dynamicbox.provider
 * @return int type
 * @retval #DBOX_STATUS_ERROR_INVALID_PARAMETER Invalid handle
 * @retval #DBOX_STATUS_ERROR_NONE Successfully unreference
 * @pre dynamicbox_ref_buffer() must be called.
 * @see dynamicbox_ref_buffer()
 */
extern int dynamicbox_unref_buffer(void *buffer);

/**
 * @internal
 * @brief Synchronizes the updated buffer.
 * @details This is only needed for non-H/W accelerated buffer.
 * @since_tizen 2.3
 * @param[in] handle Buffer handle
 * @privlevel public
 * @privilege %http://tizen.org/privilege/core/dynamicbox.provider
 * @return int type
 * @retval #DBOX_STATUS_ERROR_NONE Successfully sync'd
 * @retval #DBOX_STATUS_ERROR_INVALID_PARAMETER Invalid argument
 * @see dynamicbox_acquire_buffer()
 */
extern int dynamicbox_sync_buffer(struct dynamicbox_buffer *handle);

/**
 * @internal
 * @brief Requests to schedule the update operation to a provider.
 * @since_tizen 2.3
 * @param[in] id Instance Id which is passed to you via the first parameter of every dynamicbox_XXXX interface functions
 * @privlevel public
 * @privilege %http://tizen.org/privilege/core/dynamicbox.provider
 * @return If succes returns 0 or return less than 0
 * @retval #DBOX_STATUS_ERROR_NONE Successfully triggered
 * @retval #DBOX_STATUS_ERROR_INVALID_PARAMETER Invalid argument
 * @retval #DBOX_STATUS_ERROR_OUT_OF_MEMORY Not enough memory
 * @retval #DBOX_STATUS_ERROR_NOT_EXIST Given id instance is not exist
 */
extern int dynamicbox_request_update(const char *id);

/**
 * @internal
 * @brief Checks whether the dynamicbox supports H/W acceleration or not.
 * @since_tizen 2.3
 * @param[in] handle Buffer handle
 * @return int type
 * @retval 1 If the buffer handle support the H/W acceleration buffer
 * @retval 0 If it doesn not supported
 * @see dynamicbox_acquire_buffer()
 */
extern int dynamicbox_support_hw_buffer(struct dynamicbox_buffer *handle);

/**
 * @internal
 * @brief Creates the H/W accelerated buffer.
 * @since_tizen 2.3
 * @param[in] handle Buffer handle
 * @privlevel public
 * @privilege %http://tizen.org/privilege/core/dynamicbox.provider
 * @return int type
 * @retval #DBOX_STATUS_ERROR_ALREADY H/W buffer is already created
 * @retval #DBOX_STATUS_ERROR_INVALID_PARAMETER Invalid argument
 * @retval #DBOX_STATUS_ERROR_NONE Successfully created
 * @pre Must be checked whether the H/W buffer is supported or not.
 * @post Must be destroyed if it is not necessary.
 * @see dynamicbox_support_hw_buffer()
 */
extern int dynamicbox_create_hw_buffer(struct dynamicbox_buffer *handle);

/**
 * @internal
 * @brief Destroys the H/W accelerated buffer.
 * @since_tizen 2.3
 * @param[in] handle Buffer handle
 * @privlevel public
 * @privilege %http://tizen.org/privilege/core/dynamicbox.provider
 * @return int type
 * @retval #DBOX_STATUS_ERROR_NONE Successfully destroyed
 * @retval #DBOX_STATUS_ERROR_INVALID_PARAMETER Invalid argument
 * @pre Must be created h/w buffer using dynamicbox_create_hw_buffer.
 * @see dynamicbox_create_hw_buffer()
 */
extern int dynamicbox_destroy_hw_buffer(struct dynamicbox_buffer *handle);

/**
 * @internal
 * @brief Gets the address of the accelerated H/W buffer.
 * @since_tizen 2.3
 * @param[in] handle Buffer handle
 * @privlevel public
 * @privilege %http://tizen.org/privilege/core/dynamicbox.provider
 * @return void* type
 * @retval @c NULL Failed to get H/W accelerated buffer address
 * @retval addr H/W accelerated buffer address
 * @see dynamicbox_create_hw_buffer()
 */
extern void *dynamicbox_buffer_hw_buffer(struct dynamicbox_buffer *handle);

/**
 * @internal
 * @brief Get the stride information from the buffer info
 * @since_tizen 2.3
 * @param[in] handle Buffer handle
 * @return int stride size
 * @retval positive_value length of stride
 * @return DBOX_STATUS_ERROR_INVALID_PARAMETER Invalid parameter
 * @return DBOX_STATUS_ERROR_FAULT Unrecoverable error
 * @see dynamicbox_create_hw_buffer()
 */
extern int dynamicbox_buffer_stride(struct dynamicbox_buffer *handle);

/**
 * @internal
 * @brief Pre-processes for rendering content.
 * @details This is only needed for accessing H/W accelerated buffer.
 * @since_tizen 2.3
 * @param[in] handle Buffer handle
 * @privlevel public
 * @privilege %http://tizen.org/privilege/core/dynamicbox.provider
 * @return int type
 * @retval #DBOX_STATUS_ERROR_INVALID_PARAMETER Invalid handle
 * @retval #DBOX_STATUS_ERROR_NONE Successfully done
 * @post dynamicbox_buffer_post_render must be called after the rendering (accessing buffer) is completed.
 * @see dynamicbox_support_hw_buffer()
 * @see dynamicbox_buffer_post_render()
 */
extern int dynamicbox_buffer_pre_render(struct dynamicbox_buffer *handle);

/**
 * @internal
 * @brief Post-processes for rendering content.
 * @since_tizen 2.3
 * @param[in] handle Buffer handle
 * @privlevel public
 * @privilege %http://tizen.org/privilege/core/dynamicbox.provider
 * @return If succes returns 0 or return less than 0
 * @retval #DBOX_STATUS_ERROR_NONE If succeed
 * @retval #DBOX_STATUS_ERROR_INVALID_PARAMETER Invalid argument
 * @pre dynamicbox_buffer_pre_render must be called.
 * @see dynamicbox_support_hw_buffer()
 * @see dynamicbox_buffer_pre_render()
 */
extern int dynamicbox_buffer_post_render(struct dynamicbox_buffer *handle);

/**
 * @internal
 * @brief Sends a request to the viewer (homescreen) to close the GBar.
 * @details Even though you call this function. the homescreen can ignore it.
 * @since_tizen 2.3
 * @param[in] dboxid Dynamic Box Package Id
 * @param[in] id Instance Id which is passed to you via the first parameter of every dynamicbox_XXXX interface functions
 * @param[in] reason DBOX_STATUS_ERROR_NONE(0)
 * @privlevel public
 * @privilege %http://tizen.org/privilege/core/dynamicbox.provider
 * @return int type
 * @retval #DBOX_STATUS_ERROR_INVALID_PARAMETER Invalid parameters
 * @retval #DBOX_STATUS_ERROR_OUT_OF_MEMORY Out of memory
 * @retval #DBOX_STATUS_ERROR_FAULT Failed to send a request
 * @retval #DBOX_STATUS_ERROR_NONE Successfully requested
 */
extern int dynamicbox_request_close_gbar(const char *dboxid, const char *id, int reason);

/**
 * @internal
 * @brief Send a freeze request to the viewer (homescreen)
 * @details
 *        The viewer will get this request via event callback.
 *        Then it should freeze its scroller or stop moving the dynamicbox.
 * @since_tizen 2.3
 * @remarks If the viewer doesn't care this request, this will has no effect.
 * @param[in] dboxid Dynamic Box Package Id
 * @param[in] id Instance Id which is passed to you via the first parameter of every dynamicbox_XXXX interface functions
 * @privlevel public
 * @privilege %http://tizen.org/privilege/core/dynamicbox.provider
 * @return int type
 * @retval #DBOX_STATUS_ERROR_INVALID_PARAMETER Invalid parameters
 * @retval #DBOX_STATUS_ERROR_OUT_OF_MEMORY Out of memory
 * @retval #DBOX_STATUS_ERROR_FAULT Failed to send requet
 * @retval #DBOX_STATUS_ERROR_NONE Successfully requested
 * @see dynamicbox_release_scroller()
 */
extern int dynamicbox_freeze_scroller(const char *dboxid, const char *id);

/**
 * @internal
 * @brief Send a release request to the viewer (homescreen)
 * @details
 *        The viewer will get this request via event callback.
 *        Then it should release its scroller or continue moving the dynamicbox.
 * @since_tizen 2.3
 * @remarks If the viewer doesn't care this request, this will has no effect.
 * @param[in] dboxid Dynamic Box Package Id
 * @param[in] id Instance Id which is passed to you via the first parameter of every dynamicbox_XXXX interface functions
 * @privlevel public
 * @privilege %http://tizen.org/privilege/core/dynamicbox.provider
 * @return int type
 * @retval #DBOX_STATUS_ERROR_INVALID_PARAMETER Invalid parameters
 * @retval #DBOX_STATUS_ERROR_OUT_OF_MEMORY Out of memory
 * @retval #DBOX_STATUS_ERROR_FAULT Failed to send requet
 * @retval #DBOX_STATUS_ERROR_NONE Successfully requested
 * @see dynamicbox_freeze_scroller()
 */
extern int dynamicbox_thaw_scroller(const char *dboxid, const char *id);

/**
 * @internal
 * @brief Get the configuration value for premultiplied alpha.
 * @details
 *	If the system uses the premultiplied alpha for content,
 *	This function will returns 1.
 * @since_tizen 2.3
 * @remarks This function only works with the inhouse provider.
 * @return status of premultiplied alpha.
 * @retval 1 Content rendered with premultiplied alpha.
 * @retval 0 Content doesn't rendered with premultiplied alpha.
 */
extern int dynamicbox_conf_premultiplied_alpha(void);

/**
 * @internal
 * @brief Get the configuration value for AUTO Alignment of canvas buffer.
 * @details
 *      If the UIFW does not support the STRIDE information for rendering frame,
 *      This function will returns 1, then the dynamicbox will automatically aligning the buffer.
 * @since_tizen 2.3
 * @return status of auto alignment.
 * @retval 1 Content will be automatically aligned by dynamicbox.
 * @retval 0 Content will be aligned by UIFW, using stride information.
 */
extern int dynamicbox_conf_auto_align(void);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
/* End of a file */
