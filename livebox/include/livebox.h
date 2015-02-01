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
 * @file livebox.h
 * @brief This file declares API of liblivebox library
 */

/**
 * @addtogroup CAPI_LIVEBOX_HELPER_MODULE
 * @{
 */

/**
 * @brief Definition for COLOR BLOCK.
 */
#define LB_DESC_TYPE_COLOR "color"

/**
 * @brief Definition for TEXT BLOCK.
 */
#define LB_DESC_TYPE_TEXT "text"

/**
 * @brief Definition for IMAGE BLOCK.
 */
#define LB_DESC_TYPE_IMAGE "image"

/**
 * @brief Definition for SIGNAL BLOCK.
 */
#define LB_DESC_TYPE_SIGNAL "signal"

/**
 * @brief Definition for INFO BLOCK.
 */
#define LB_DESC_TYPE_INFO "info"

/**
 * @brief Definition for DRAG BLOCK.
 */
#define LB_DESC_TYPE_DRAG "drag"

/**
 * @brief Definition for SCRIPT SCRIPT.
 */
#define LB_DESC_TYPE_SCRIPT "script"

/**
 * @brief Definition for ACCESSIBILITY INFORMATION BLOCK.
 */
#define LB_DESC_TYPE_ACCESS "access"

/**
 * @brief Definition for accessibility operation.
 */
#define LB_DESC_TYPE_ACCESS_OPERATION	"access,operation"

/**
 * @brief Definition for set highlight on specified part.
 */
#define LB_DESC_DATA_ACCESS_OPERATION_HL_SET "set,hl"

/**
 * @brief Definition for remove highlight from specified part.
 */
#define LB_DESC_DATA_ACCESS_OPERATION_HL_UNSET "unset,hl"

/**
 * @brief Definition for move the highlight to next part.
 */
#define LB_DESC_DATA_ACCESS_OPERATION_HL_NEXT "next,hl"

/**
 * @brief Definition for move the highlight to previous part.
 */
#define LB_DESC_DATA_ACCESS_OPERATION_HL_PREV "prev,hl"

/**
 * @brief Definition for reset the chain of accessibility highlight list.
 */
#define LB_DESC_DATA_ACCESS_OPERATION_RESET "reset,focus"

/**
 * @brief Definition for specify the highlight move option.
 * @details If the highlight list reaches to the last object, move back to the first object.
 */
#define LB_DESC_OPTION_ACCESS_HL_LOOP "cycle"

/**
 * @brief Definition for specify the highlight move option.
 * @details If the highlight list reaches to the last object(or the first object),
 * the "prev,hl", "next,hl" will be failed.
 */
#define LB_DESC_OPTION_ACCESS_HL_NOLOOP "no,cycle"

/**
 * @brief Livebox description data handle.
 */
struct livebox_desc;

/**
 * @brief Updates a description file.
 * @remarks Must be used only by Inhouse livebox.
 * @param[in] filename
 * @param[in] for_pd
 * @privlevel public
 * @privilege %http://tizen.org/privilege/core/dynamicbox.provider
 * @return Handle of desc instance
 * @pre Should be loaded by data-provider-slave.
 * @post Should be destroyed(flushed) using livebox_desc_close() API.
 * @see livebox_desc_close()
 */
extern struct livebox_desc *livebox_desc_open(const char *filename, int for_pd);

/**
 * @brief Completes the description file update.
 * @details Only if this API is called, the description data will be applied to the content of livebox (or PD).
 * @remarks Must be used only by Inhouse livebox.
 * @param[in] handle Handle which is created by livebox_desc_open() function
 * @privlevel public
 * @privilege %http://tizen.org/privilege/core/dynamicbox.provider
 * @return int type
 * @retval #LB_STATUS_SUCCESS If the flushing description data is successfully done
 * @retval #LB_STATUS_ERROR_INVALID If the given handle is not valid
 * @pre Handle must be created by livebox_desc_open()
 * @post Handle must not be used after return from this function.
 * @see livebox_desc_open()
 */
extern int livebox_desc_close(struct livebox_desc *handle);

/**
 * @brief Sets the category information of current description data.
 * @details Creates a new description block for updating category information of layout object.
 * @param[in] handle Handle which is created by livebox_desc_open() function
 * @param[in] id Id string of target object
 * @param[in] category Category string that will be used by layout object
 * @privlevel N/P
 * @return int type
 * @retval #LB_STATUS_ERROR_INVALID Invalid handle
 * @retval #LB_STATUS_ERROR_MEMORY Memory is not enough to add this block
 * @retval index Index(>=0) of added block, which can be used by livebox_desc_set_id()
 * @pre Must be called after create a handle using livebox_desc_open()
 * @see livebox_desc_set_id()
 */
extern int livebox_desc_set_category(struct livebox_desc *handle, const char *id, const char *category);

/**
 * @brief Sets the content size.
 * @details Sets the content size of layout.
 * @param[in] handle Handle which is created by livebox_desc_open() function
 * @param[in] id Id string of target object
 * @param[in] w Width in pixel
 * @param[in] h Height in pixel
 * @privlevel N/P
 * @return int type
 * @retval #LB_STATUS_ERROR_INVALID Invalid argument
 * @retval #LB_STATUS_ERROR_MEMORY Not enough memory to add a new block
 * @retval index Index(>=0) of added block Successfully added
 * @see livebox_desc_open()
 * @see livebox_desc_set_id()
 */
extern int livebox_desc_set_size(struct livebox_desc *handle, const char *id, int w, int h);

/**
 * @brief Sets the target id of the given block.
 * @details Only available for the script block.
 * @param[in] handle Handle which is created by livebox_desc_open() function
 * @param[in] idx Index of target block
 * @param[in] id Id String which will be used by other livebox_desc_XXX functions
 * @privlevel N/P
 * @return int type
 * @retval #LB_STATUS_SUCCESS Id is successfully updated for given idx of desc block
 * @retval #LB_STATUS_ERROR_INVALID Invalid argument
 * @retval #LB_STATUS_ERROR_NOT_EXIST Given index of desc block is not exists
 * @pre Desc block should be exists which has given index "idx".
 * @post Specified Id string("id") can be used by other livebox_desc_XXXX functions.
 * @see livebox_desc_open()
 * @see livebox_desc_set_id()
 */
extern int livebox_desc_set_id(struct livebox_desc *handle, int idx, const char *id);

/**
 * @brief Adds a new block.
 * @param[in] handle Handle which is created by the livebox_desc_open() function
 * @param[in] id ID of source script object
 * @param[in] type Type (image|text|script|signal|.. etc)
 * @param[in] part Target part to update with given content(data)
 * @param[in] data Content for specified part
 * @param[in] option Option for the block. (script: group, image: option, ...)
 * @privlevel N/P
 * @return Index of current description block
 * @retval #LB_STATUS_ERROR_INVALID Invalid argument
 * @retval #LB_STATUS_ERROR_MEMORY Not enough memory to add a new desc block
 * @retval index Index(>=0) of added desc block
 * @pre Handle must be created using livebox_desc_open.
 * @see livebox_desc_set_id()
 * @see livebox_desc_del_block()
 */
extern int livebox_desc_add_block(struct livebox_desc *handle, const char *id, const char *type, const char *part, const char *data, const char *option);

/**
 * @brief Deletes an added block.
 * @details If you discard the added desc block, use this API and the index of the created desc block.
 * @param[in] handle Handle which is created by livebox_desc_open() function
 * @param[in] idx Index of added block, returned from livebox_desc_add_block(), livebox_desc_set_size(), livebox_desc_set_category(), etc
 * @privlevel N/P
 * @return int type
 * @retval #LB_STATUS_SUCCESS Successfully deleted
 * @retval #LB_STATUS_ERROR_NOT_EXIST Given index of desc block is not exists
 * @pre Index must be exists.
 * @see livebox_desc_add_block()
 * @see livebox_desc_open()
 */
extern int livebox_desc_del_block(struct livebox_desc *handle, int idx);

/**
 * @brief Creates an Evas_Object to create an elm_win object.
 * @details Creating a new Evas_Object using livebox frame buffer.
 * @param[in] id Instance Id
 * @param[in] is_pd 1 if this object is created for PD or 0 (for LB)
 * @privlevel public
 * @privilege %http://tizen.org/privilege/core/dynamicbox.provider
 * @return Evas_Object* Evas Object type
 * @retval Address Valid evas object
 * @retval @c NULL failed to create
 */
extern Evas_Object *livebox_get_evas_object(const char *id, int is_pd);

/**
 * @brief Set extra information for updated content.
 * @details
 *      If the provider should give some hints to the viewer,
 *      such as title, icon and name, this function can set them.
 *      provider will send those information to the viewer when the
 *      content is updated.
 * @remarks This function only works with the buffer type inhouse provider.
 * @param[in] id Livebox Instance Id
 * @param[in] content Content string that will pass to a livebox via livebox_create interface when it is re-created.
 * @param[in] title Human readable text for accessitility, summarized text for representing the content of a box.
 * @param[in] icon Path of icon image file - alternative image content
 * @param[in] name Name of a box - alternative text content
 * @privlevel N/P
 * @return int type
 * @retval #LB_STATUS_ERROR_MEMORY Out of memory
 * @retval #LB_STATUS_ERROR_INVALID Invalid parameters
 * @retval #LB_STATUS_ERROR_FAULT Unrecoverable error
 * @retval #LB_STATUS_SUCCESS Successfully sent
 */
extern int livebox_set_extra_info(const char *id, const char *content, const char *title, const char *icon, const char *name);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
/* End of a file */
