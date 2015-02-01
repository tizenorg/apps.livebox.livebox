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

/**
 * Export by dynamicbox_service
 */
#include <dynamicbox_buffer.h>
#include <dynamicbox_script.h>

#ifndef __DYNAMICBOX_UILITY_INTERNAL_H
#define __DYNAMICBOX_UILITY_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file dynamicbox_internal.h
 * @brief This file declares API of libdynamicbox library
 * @since_tizen 2.3
 */

/**
 * @addtogroup CAPI_DYNAMICBOX_UTILITY_MODULE
 * @{
 */

/**
 * @internal
 * @brief Definition for COLOR BLOCK.
 * @since_tizen 2.3
 */
#define DBOX_DESC_TYPE_COLOR "color"

/**
 * @internal
 * @brief Definition for TEXT BLOCK.
 * @since_tizen 2.3
 */
#define DBOX_DESC_TYPE_TEXT "text"

/**
 * @internal
 * @brief Definition for IMAGE BLOCK.
 * @since_tizen 2.3
 */
#define DBOX_DESC_TYPE_IMAGE "image"

/**
 * @internal
 * @brief Definition for SIGNAL BLOCK.
 * @since_tizen 2.3
 */
#define DBOX_DESC_TYPE_SIGNAL "signal"

/**
 * @internal
 * @brief Definition for INFO BLOCK.
 * @since_tizen 2.3
 */
#define DBOX_DESC_TYPE_INFO "info"

/**
 * @internal
 * @brief Definition for DRAG BLOCK.
 * @since_tizen 2.3
 */
#define DBOX_DESC_TYPE_DRAG "drag"

/**
 * @internal
 * @brief Definition for SCRIPT SCRIPT.
 * @since_tizen 2.3
 */
#define DBOX_DESC_TYPE_SCRIPT "script"

/**
 * @internal
 * @brief Definition for ACCESSIBILITY INFORMATION BLOCK.
 * @since_tizen 2.3
 */
#define DBOX_DESC_TYPE_ACCESS "access"

/**
 * @internal
 * @brief Definition for accessibility operation.
 * @since_tizen 2.3
 */
#define DBOX_DESC_TYPE_ACCESS_OPERATION "access,operation"

/**
 * @internal
 * @brief Definition for set highlight on specified part.
 * @since_tizen 2.3
 */
#define DBOX_DESC_DATA_ACCESS_OPERATION_HL_SET "set,hl"

/**
 * @internal
 * @brief Definition for remove highlight from specified part.
 * @since_tizen 2.3
 */
#define DBOX_DESC_DATA_ACCESS_OPERATION_HL_UNSET "unset,hl"

/**
 * @internal
 * @brief Definition for move the highlight to next part.
 * @since_tizen 2.3
 */
#define DBOX_DESC_DATA_ACCESS_OPERATION_HL_NEXT "next,hl"

/**
 * @internal
 * @brief Definition for move the highlight to previous part.
 * @since_tizen 2.3
 */
#define DBOX_DESC_DATA_ACCESS_OPERATION_HL_PREV "prev,hl"

/**
 * @internal
 * @brief Definition for reset the chain of accessibility highlight list.
 * @since_tizen 2.3
 */
#define DBOX_DESC_DATA_ACCESS_OPERATION_RESET "reset,focus"

/**
 * @internal
 * @brief Definition for specify the highlight move option.
 * @details If the highlight list reaches to the last object, move back to the first object.
 * @since_tizen 2.3
 */
#define DBOX_DESC_OPTION_ACCESS_HL_LOOP "cycle"

/**
 * @internal
 * @brief Definition for specify the highlight move option.
 * @details If the highlight list reaches to the last object(or the first object),
 * the "prev,hl", "next,hl" will be failed.
 * @since_tizen 2.3
 */
#define DBOX_DESC_OPTION_ACCESS_HL_NOLOOP "no,cycle"

/**
 * @internal
 * @brief Dynamic Box description data handle.
 * @since_tizen 2.3
 */
typedef struct dynamicbox_desc *dynamicbox_desc_h;

/**
 * @internal
 * @brief Flush Callback for snapshot window
 * @since_tizen 2.3
 */
typedef void (*dynamicbox_flush_cb)(void *snapshot_window, const char *id, int status, void *data);

/**
 * @internal
 * @brief Updates a description file.
 * @since_tizen 2.3
 * @remarks Must be used only by Inhouse Dynamic Box.
 * @param[in] id Dynamic Box Instance Id
 * @param[in] gbar 1 for Glance Bar or 0
 * @privlevel public
 * @privilege %http://developer.samsung.com/tizen/privilege/dynamicbox.provider
 * @feature http://developer.samsung.com/tizen/feature/samsung_extension
 * @feature http://developer.samsung.com/tizen/feature/in_house/shell.appwidget
 * @return Handle of desc instance
 * @retval @c NULL Failed to create a desc instance, dynamicbox_last_status() will returns reason of failure
 * @pre Should be loaded by data-provider-slave.
 * @post Should be destroyed(flushed) using dynamicbox_desc_close() API.
 * @see dynamicbox_desc_close()
 */
extern dynamicbox_desc_h dynamicbox_desc_open(const char *id, int gbar);

/**
 * @internal
 * @brief Completes the description file update.
 * @details Only if this API is called, the description data will be applied to the content of Dynamic Box (or GBar).
 * @since_tizen 2.3
 * @remarks Must be used only by Inhouse Dynamic Box.
 * @param[in] handle Handle which is created by dynamicbox_desc_open() function
 * @privlevel public
 * @privilege %http://developer.samsung.com/tizen/privilege/dynamicbox.provider
 * @feature http://developer.samsung.com/tizen/feature/samsung_extension
 * @feature http://developer.samsung.com/tizen/feature/in_house/shell.appwidget
 * @return int type
 * @retval #DBOX_STATUS_ERROR_NONE If the flushing description data is successfully done
 * @retval #DBOX_STATUS_ERROR_INVALID_PARAMETER If the given handle is not valid
 * @retval #DBOX_STATUS_ERROR_PERMISSION_DENIED Permission denied
 * @pre Handle must be created by dynamicbox_desc_open()
 * @post Handle must not be used after return from this function.
 * @see dynamicbox_desc_open()
 */
extern int dynamicbox_desc_close(dynamicbox_desc_h handle);

/**
 * @internal
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
extern int dynamicbox_desc_set_category(dynamicbox_desc_h handle, const char *id, const char *category);

/**
 * @internal
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
extern int dynamicbox_desc_set_size(dynamicbox_desc_h handle, const char *id, int w, int h);

/**
 * @internal
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
extern int dynamicbox_desc_set_id(dynamicbox_desc_h handle, int idx, const char *id);

/**
 * @internal
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
extern int dynamicbox_desc_add_block(dynamicbox_desc_h handle, const char *id, const char *type, const char *part, const char *data, const char *option);

/**
 * @internal
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
extern int dynamicbox_desc_del_block(dynamicbox_desc_h handle, int idx);

/**
 * @internal
 * @brief Structure for a Dynamic Box buffer defined by a provider.
 * @since_tizen 2.3
 */
typedef struct dynamicbox_buffer *dynamicbox_buffer_h;

/**
 * @internal
 * @brief Return values of dynamicbox programming interfaces: Using network.
 * @since_tizen 2.3
 */
extern const int DBOX_USE_NET;

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
 * @privilege %http://developer.samsung.com/tizen/privilege/dynamicbox.provider
 * @feature http://developer.samsung.com/tizen/feature/samsung_extension
 * @feature http://developer.samsung.com/tizen/feature/in_house/shell.appwidget
 * @return Buffer handle
 * @retval @c NULL Failed to acquire buffer, dynamicbox_last_status() will returns reason of failure.
 * @retval handler Handle object
 * @pre Given instance must be specify its type as buffer. or this API will be fail.
 * @post Allocated buffer object must be released via dynamicbox_release_buffer().
 * @see dynamicbox_release_buffer()
 */
extern dynamicbox_buffer_h dynamicbox_create_buffer(const char *id, int gbar, int auto_align, int (*handler)(dynamicbox_buffer_h, dynamicbox_buffer_event_data_t, void *), void *data);

/**
 * @internal
 * @brief Acquire a buffer for GBar or DBox.
 * @since_tizen 2.3
 * @param[in] handle Buffer handle
 * @param[in] idx Index of a buffer, DBOX_PRIMARY_BUFFER is used for accessing primary buffer
 * @param[in] width Width of a buffer
 * @param[in] height Height of a buffer
 * @param[in] pixels Pixel size in byte
 * @privlevel public
 * @privilege %http://developer.samsung.com/tizen/privilege/dynamicbox.provider
 * @feature http://developer.samsung.com/tizen/feature/samsung_extension
 * @feature http://developer.samsung.com/tizen/feature/in_house/shell.appwidget
 * @return int status
 * @retval #DBOX_STATUS_ERROR_NONE Successfully allocated
 * @retval #DBOX_STATUS_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #DBOX_STATUS_ERROR_FAULT Unrecoverable error occurred
 * @retval #DBOX_STATUS_ERROR_PERMISSION_DENIED Permission denied
 * @pre dynamicbox_create_buffer() must be called
 * @see dynamicbox_create_buffer()
 * @see dynamicbox_release_buffer()
 */
extern int dynamicbox_acquire_buffer(dynamicbox_buffer_h handle, int idx, int width, int height, int pixels);

/**
 * @internal
 * @brief Acquires the Id of Resource (window system).
 * @details Only if the provider uses Resource for providing render buffer.
 * @since_tizen 2.3
 * @remarks Pixmap Id can be destroyed if you call the dynamicbox_release_buffer(). Then this Resource Id is not guaranteed to use.
 * @param[in] handle Buffer handle
 * @param[in] idx Index of a buffer, #c DBOX_PRIMARY_BUFFER is used for accessing the primary buffer
 * @return unsigned int Resource Id
 * @retval >0 Resource Id
 * @retval 0 Failed to get Resource Id
 * @pre The system must be set to use the Resource Id method for content sharing.
 * @see dynamicbox_acquire_buffer()
 */
extern unsigned int dynamicbox_resource_id(dynamicbox_buffer_h handle, int idx);

/**
 * @internal
 * @brief Releases the buffer of dynamicbox.
 * @since_tizen 2.3
 * @param[in] handle Buffer handle
 * @param[in] idx Index of a buffer, #c DBOX_PRIMARY_BUFFER is used for accessing the primary buffer
 * @privlevel public
 * @privilege %http://developer.samsung.com/tizen/privilege/dynamicbox.provider
 * @feature http://developer.samsung.com/tizen/feature/samsung_extension
 * @feature http://developer.samsung.com/tizen/feature/in_house/shell.appwidget
 * @return int status
 * @retval #DBOX_STATUS_ERROR_INVALID_PARAMETER Invalid argument
 * @retval #DBOX_STATUS_ERROR_NONE Successfully released
 * @retval #DBOX_STATUS_ERROR_PERMISSION_DENIED Permission denied
 * @pre Handle must be created using dynamicbox_acquire_buffer().
 * @see dynamicbox_acquire_buffer()
 */
extern int dynamicbox_release_buffer(dynamicbox_buffer_h handle, int idx);

/**
 * @internal
 * @brief Destroy a buffer of dynamicbox
 * @since_tizen 2.3
 * @param[in] handle buffer handle
 * @privlevel public
 * @privilege %http://developer.samsung.com/tizen/privilege/dynamicbox.provider
 * @feature http://developer.samsung.com/tizen/feature/samsung_extension
 * @feature http://developer.samsung.com/tizen/feature/in_house/shell.appwidget
 * @return int status
 * @retval #DBOX_STATUS_ERROR_INVALID_PARAMTER Invalid argument
 * @retval #DBOX_STATUS_ERROR_PERMISSION_DENIED Permission denied
 * @retval #DBOX_STATUS_ERROR_NONE Successfully destroyed
 * @see dynamicbox_create_buffer()
 */
extern int dynamicbox_destroy_buffer(dynamicbox_buffer_h handle);

/**
 * @internal
 * @brief Gets the address of buffer for S/W rendering.
 * @details If you try to use this, after dynamicbox_create_hw_buffer(), you will get @c NULL.
 *          This API only be used for primary buffer. you cannot access the extra buffer
 * @since_tizen 2.3
 * @param[in] handle Buffer handle
 * @privlevel public
 * @privilege %http://developer.samsung.com/tizen/privilege/dynamicbox.provider
 * @feature http://developer.samsung.com/tizen/feature/samsung_extension
 * @feature http://developer.samsung.com/tizen/feature/in_house/shell.appwidget
 * @return void* address of the render buffer
 * @retval @c NULL If it falis to get buffer address, dynamicbox_last_status() will returns reason of failure.
 * @retval address If it succeed to get the buffer address
 * @see dynamicbox_unref_buffer()
 */
extern void *dynamicbox_ref_buffer(dynamicbox_buffer_h handle);

/**
 * @internal
 * @brief Releases the S/W render buffer.
 * @since_tizen 2.3
 * @param[in] buffer Address of render buffer
 * @privlevel public
 * @privilege %http://developer.samsung.com/tizen/privilege/dynamicbox.provider
 * @feature http://developer.samsung.com/tizen/feature/samsung_extension
 * @feature http://developer.samsung.com/tizen/feature/in_house/shell.appwidget
 * @return int type
 * @retval #DBOX_STATUS_ERROR_INVALID_PARAMETER Invalid handle
 * @retval #DBOX_STATUS_ERROR_PERMISSION_DENIED Permission denied
 * @retval #DBOX_STATUS_ERROR_NONE Successfully unreference
 * @pre dynamicbox_ref_buffer() must be called.
 * @see dynamicbox_ref_buffer()
 */
extern int dynamicbox_unref_buffer(void *buffer);

/**
 * @internal
 * @brief Synchronizes the updated buffer.
 * @details This is only needed for non-H/W accelerated buffer.
 *          This API is only supported for primary buffer (#c DBOX_PRIMARY_BUFFER)
 * @since_tizen 2.3
 * @param[in] handle Buffer handle
 * @privlevel public
 * @privilege %http://developer.samsung.com/tizen/privilege/dynamicbox.provider
 * @feature http://developer.samsung.com/tizen/feature/samsung_extension
 * @feature http://developer.samsung.com/tizen/feature/in_house/shell.appwidget
 * @return int type
 * @retval #DBOX_STATUS_ERROR_NONE Successfully sync'd
 * @retval #DBOX_STATUS_ERROR_PERMISSION_DENIED Permission denied
 * @retval #DBOX_STATUS_ERROR_INVALID_PARAMETER Invalid argument
 * @see dynamicbox_acquire_buffer()
 */
extern int dynamicbox_sync_buffer(dynamicbox_buffer_h handle);

/**
 * @internal
 * @brief Checks whether the dynamicbox supports H/W acceleration or not.
 *        This API is only supported for primary buffer (#c DBOX_PRIMARY_BUFFER)
 * @since_tizen 2.3
 * @param[in] handle Buffer handle
 * @return int type
 * @retval 1 If the buffer handle support the H/W acceleration buffer
 * @retval 0 If it doesn not supported
 * @see dynamicbox_acquire_buffer()
 */
extern int dynamicbox_support_hw_buffer(dynamicbox_buffer_h handle);

/**
 * @internal
 * @brief Creates the H/W accelerated buffer.
 *        This API is only supported for primary buffer (#c DBOX_PRIMARY_BUFFER)
 * @since_tizen 2.3
 * @param[in] handle Buffer handle
 * @privlevel public
 * @privilege %http://developer.samsung.com/tizen/privilege/dynamicbox.provider
 * @feature http://developer.samsung.com/tizen/feature/samsung_extension
 * @feature http://developer.samsung.com/tizen/feature/in_house/shell.appwidget
 * @return int type
 * @retval #DBOX_STATUS_ERROR_ALREADY H/W buffer is already created
 * @retval #DBOX_STATUS_ERROR_INVALID_PARAMETER Invalid argument
 * @retval #DBOX_STATUS_ERROR_PERMISSION_DENIED Permission denied
 * @retval #DBOX_STATUS_ERROR_NONE Successfully created
 * @pre Must be checked whether the H/W buffer is supported or not.
 * @post Must be destroyed if it is not necessary.
 * @see dynamicbox_support_hw_buffer()
 */
extern int dynamicbox_create_hw_buffer(dynamicbox_buffer_h handle);

/**
 * @internal
 * @brief Destroys the H/W accelerated buffer.
 *        This API is only supported for primary buffer (#c DBOX_PRIMARY_BUFFER)
 * @since_tizen 2.3
 * @param[in] handle Buffer handle
 * @privlevel public
 * @privilege %http://developer.samsung.com/tizen/privilege/dynamicbox.provider
 * @feature http://developer.samsung.com/tizen/feature/samsung_extension
 * @feature http://developer.samsung.com/tizen/feature/in_house/shell.appwidget
 * @return int type
 * @retval #DBOX_STATUS_ERROR_NONE Successfully destroyed
 * @retval #DBOX_STATUS_ERROR_PERMISSION_DENIED Permission denied
 * @retval #DBOX_STATUS_ERROR_INVALID_PARAMETER Invalid argument
 * @pre Must be created h/w buffer using dynamicbox_create_hw_buffer.
 * @see dynamicbox_create_hw_buffer()
 */
extern int dynamicbox_destroy_hw_buffer(dynamicbox_buffer_h handle);

/**
 * @internal
 * @brief Gets the address of the accelerated H/W buffer.
 *        This API is only supported for primary buffer (#c DBOX_PRIMARY_BUFFER)
 * @since_tizen 2.3
 * @param[in] handle Buffer handle
 * @privlevel public
 * @privilege %http://developer.samsung.com/tizen/privilege/dynamicbox.provider
 * @feature http://developer.samsung.com/tizen/feature/samsung_extension
 * @feature http://developer.samsung.com/tizen/feature/in_house/shell.appwidget
 * @return void* type
 * @retval @c NULL Failed to get H/W accelerated buffer address, dynamicbox_last_status() will returns reason of failure.
 * @retval addr H/W accelerated buffer address
 * @see dynamicbox_create_hw_buffer()
 */
extern void *dynamicbox_buffer_hw_buffer(dynamicbox_buffer_h handle);

/**
 * @internal
 * @brief Get the stride information from the buffer info
 *        This API is only supported for primary buffer (#c DBOX_PRIMARY_BUFFER)
 * @since_tizen 2.3
 * @param[in] handle Buffer handle
 * @return int stride size
 * @retval positive_value length of stride
 * @return #DBOX_STATUS_ERROR_INVALID_PARAMETER Invalid parameter
 * @return #DBOX_STATUS_ERROR_FAULT Unrecoverable error
 * @see dynamicbox_create_hw_buffer()
 */
extern int dynamicbox_buffer_stride(dynamicbox_buffer_h handle);

/**
 * @internal
 * @brief Pre-processes for rendering content.
 * @details This is only needed for accessing H/W accelerated buffer.
 * @since_tizen 2.3
 * @param[in] handle Buffer handle
 * @privlevel public
 * @privilege %http://developer.samsung.com/tizen/privilege/dynamicbox.provider
 * @feature http://developer.samsung.com/tizen/feature/samsung_extension
 * @feature http://developer.samsung.com/tizen/feature/in_house/shell.appwidget
 * @return int type
 * @retval #DBOX_STATUS_ERROR_INVALID_PARAMETER Invalid handle
 * @retval #DBOX_STATUS_ERROR_PERMISSION_DENIED Permission denied
 * @retval #DBOX_STATUS_ERROR_NONE Successfully done
 * @post dynamicbox_buffer_post_render must be called after the rendering (accessing buffer) is completed.
 * @see dynamicbox_support_hw_buffer()
 * @see dynamicbox_buffer_post_render()
 */
extern int dynamicbox_buffer_pre_render(dynamicbox_buffer_h handle);

/**
 * @internal
 * @brief Post-processes for rendering content.
 * @since_tizen 2.3
 * @param[in] handle Buffer handle
 * @privlevel public
 * @privilege %http://developer.samsung.com/tizen/privilege/dynamicbox.provider
 * @feature http://developer.samsung.com/tizen/feature/samsung_extension
 * @feature http://developer.samsung.com/tizen/feature/in_house/shell.appwidget
 * @return If succes returns 0 or return less than 0
 * @retval #DBOX_STATUS_ERROR_NONE If succeed
 * @retval #DBOX_STATUS_ERROR_PERMISSION_DENIED Permission denied
 * @retval #DBOX_STATUS_ERROR_INVALID_PARAMETER Invalid argument
 * @pre dynamicbox_buffer_pre_render must be called.
 * @see dynamicbox_support_hw_buffer()
 * @see dynamicbox_buffer_pre_render()
 */
extern int dynamicbox_buffer_post_render(dynamicbox_buffer_h handle);

/**
 * @internal
 * @brief Sends a request to the viewer (homescreen) to close the GBar.
 * @details Even though you call this function. the homescreen can ignore it.
 * @since_tizen 2.3
 * @param[in] dboxid Dynamic Box Package Id
 * @param[in] id Instance Id which is passed to you via the first parameter of every dynamicbox_XXXX interface functions
 * @param[in] reason #DBOX_STATUS_ERROR_NONE(0)
 * @privlevel public
 * @privilege %http://developer.samsung.com/tizen/privilege/dynamicbox.provider
 * @feature http://developer.samsung.com/tizen/feature/samsung_extension
 * @feature http://developer.samsung.com/tizen/feature/in_house/shell.appwidget
 * @return int type
 * @retval #DBOX_STATUS_ERROR_PERMISSION_DENIED Permission denied
 * @retval #DBOX_STATUS_ERROR_INVALID_PARAMETER Invalid parameters
 * @retval #DBOX_STATUS_ERROR_OUT_OF_MEMORY Out of memory
 * @retval #DBOX_STATUS_ERROR_FAULT Failed to send a request
 * @retval #DBOX_STATUS_ERROR_NONE Successfully requested
 */
extern int dynamicbox_request_close_glance_bar(const char *dboxid, const char *id, int reason);

/**
 * @internal
 * @brief Create a snapshot window
 * @details
 *         If you want create a snapshot image of contents, create this snapshot window
 *         But it will not renders object asynchronously,
 *         So you have to render the objects immediately, the delayed rendering object will not be on the canvas
 *         After flush it to a file.
 * @since_tizen 2.3
 * @param[in] id
 * @param[in] size_type
 * @return void* window
 * @retval @c NULL failed to create a snapshot window, dynamicbox_last_status() will returns reason of failure.
 * @retval Object window object (ex, elementary window)
 * @see dynamicbox_snapshot_window_flush()
 */
extern void *dynamicbox_snapshot_window_add(const char *id, int size_type);

/**
 * @internal
 * @brief Flush to a file for snapshot window
 * @since_tizen 2.3
 * @param[in] snapshot_win Snapshot window
 * @param[in] timeout After this, the image file will be created
 * @param[in] flush_cb Right after flush an image file, this callback will be called
 * @param[in] data Callback data for flush_cb
 * @return int status
 * @retval #DBOX_STATUS_ERROR_NONE Successfully flushed (flush timer added)
 * @retval #DBOX_STATUS_ERROR_INVALID_PARAMETER Invalid paramter
 * @retval #DBOX_STATUS_ERROR_FAULT Failed to create a flush timer
 * @see dynamicbox_snapshot_window_add()
 */
extern int dynamicbox_snapshot_window_flush(void *snapshot_win, double timeout, dynamicbox_flush_cb flush_cb, void *data);

/**
 * @internal
 * @brief Sending the updated event to the viewer
 * @since_tizen 2.3
 * @param[in] handle Dynamicbox Buffer Handle
 * @param[in] idx Index of a buffer
 * @privlevel public
 * @privilege %http://developer.samsung.com/tizen/privilege/dynamicbox.provider
 * @feature http://developer.samsung.com/tizen/feature/samsung_extension
 * @feature http://developer.samsung.com/tizen/feature/in_house/shell.appwidget
 * @return int status
 * @retval #DBOX_STATUS_ERROR_NONE Successfully sent
 */
extern int dynamicbox_send_updated_by_idx(dynamicbox_buffer_h handle, int idx);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
/* End of a file */
