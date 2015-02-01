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

#include <dynamicbox_service.h>

#ifndef __DYNAMICBOX_UILITY_H
#define __DYNAMICBOX_UILITY_H

#ifdef __cplusplus
extern "C" {
#endif

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
 * @brief Event information type for dynamicbox_content_event()
 * @details definition is in dynamicbox_service.h file
 * @since_tizen 2.3
 */
typedef struct dynamicbox_event_info *dynamicbox_event_info_t;

/**
 * @brief Creates an Object to create an elm_win object.
 * @details Creating a new Object using Dynamic Box frame buffer.
 * @since_tizen 2.3
 * @param[in] id Instance Id
 * @param[in] gbar 1 if this object is created for Glance Bar or 0 (for Dynamic Box)
 * @privlevel public
 * @privilege %http://developer.samsung.com/tizen/privilege/dynamicbox.provider
 * @feature http://developer.samsung.com/tizen/feature/samsung_extension
 * @feature http://developer.samsung.com/tizen/feature/in_house/shell.appwidget
 * @return void* Object type
 * @retval Address Valid evas object
 * @retval @c NULL failed to create, dynamicbox_last_status() will returns reason of failure
 */
extern void *dynamicbox_get_evas_object(const char *id, int gbar);

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
 * @feature http://developer.samsung.com/tizen/feature/samsung_extension
 * @feature http://developer.samsung.com/tizen/feature/in_house/shell.appwidget
 * @retval #DBOX_STATUS_ERROR_OUT_OF_MEMORY Out of memory
 * @retval #DBOX_STATUS_ERROR_INVALID_PARAMETER Invalid parameters
 * @retval #DBOX_STATUS_ERROR_FAULT Unrecoverable error
 * @retval #DBOX_STATUS_ERROR_NONE Successfully sent
 */
extern int dynamicbox_set_extra_info(const char *id, const char *content, const char *title, const char *icon, const char *name);

/**
 * @brief Return values of dynamicbox programming interfaces: Operation is successfully done.
 * @since_tizen 2.3
 * @feature http://developer.samsung.com/tizen/feature/samsung_extension
 * @feature http://developer.samsung.com/tizen/feature/in_house/shell.appwidget
 */
extern const int DBOX_DONE;

/**
 * @brief Return values of dynamicbox programming interfaces: Contents are updated.
 * @since_tizen 2.3
 * @feature http://developer.samsung.com/tizen/feature/samsung_extension
 * @feature http://developer.samsung.com/tizen/feature/in_house/shell.appwidget
 */
extern const int DBOX_OUTPUT_UPDATED;

/**
 * @brief Return values of dynamicbox programming interfaces: Need to call the dynamicbox_need_to_update and dynamicbox_update_content.
 * @since_tizen 2.3
 * @feature http://developer.samsung.com/tizen/feature/samsung_extension
 * @feature http://developer.samsung.com/tizen/feature/in_house/shell.appwidget
 */
extern const int DBOX_NEED_TO_SCHEDULE;

/**
 * @internal
 * @brief Return values of dynamicbox programming interfaces: Need to create a new instance.
 * @since_tizen 2.3
 */
extern const int DBOX_NEED_TO_CREATE;

/**
 * @brief Return values of dynamicbox programming interfaces: Need to do force update.
 * @since_tizen 2.3
 * @feature http://developer.samsung.com/tizen/feature/samsung_extension
 * @feature http://developer.samsung.com/tizen/feature/in_house/shell.appwidget
 */
extern const int DBOX_FORCE_TO_SCHEDULE;

/**
 * @brief Return values of dynamicbox programming interfaces: Need to destroy this instance.
 * @since_tizen 2.3
 * @feature http://developer.samsung.com/tizen/feature/samsung_extension
 * @feature http://developer.samsung.com/tizen/feature/in_house/shell.appwidget
 */
extern const int DBOX_NEED_TO_DESTROY;

/**
 * @brief Return values of dynamicbox programming interfaces: Need to update.
 * @since_tizen 2.3
 * @feature http://developer.samsung.com/tizen/feature/samsung_extension
 * @feature http://developer.samsung.com/tizen/feature/in_house/shell.appwidget
 */
extern const int DBOX_NEED_TO_UPDATE;

/**
 * @brief System event type: System font is changed.
 * @since_tizen 2.3
 * @feature http://developer.samsung.com/tizen/feature/samsung_extension
 * @feature http://developer.samsung.com/tizen/feature/in_house/shell.appwidget
 */
extern const int DBOX_SYS_EVENT_FONT_CHANGED;

/**
 * @brief System event type: System language is changed.
 * @since_tizen 2.3
 * @feature http://developer.samsung.com/tizen/feature/samsung_extension
 * @feature http://developer.samsung.com/tizen/feature/in_house/shell.appwidget
 */
extern const int DBOX_SYS_EVENT_LANG_CHANGED;

/**
 * @brief System event type: System time is changed.
 * @since_tizen 2.3
 * @feature http://developer.samsung.com/tizen/feature/samsung_extension
 * @feature http://developer.samsung.com/tizen/feature/in_house/shell.appwidget
 */
extern const int DBOX_SYS_EVENT_TIME_CHANGED;

/**
 * @brief System event type: MMC Status change event.
 * @since_tizen 2.3
 * @feature http://developer.samsung.com/tizen/feature/samsung_extension
 * @feature http://developer.samsung.com/tizen/feature/in_house/shell.appwidget
 */
extern const int DBOX_SYS_EVENT_MMC_STATUS_CHANGED;

/**
 * @brief System event type: Region changed.
 * @since_tizen 2.3
 * @feature http://developer.samsung.com/tizen/feature/samsung_extension
 * @feature http://developer.samsung.com/tizen/feature/in_house/shell.appwidget
 */
extern const int DBOX_SYS_EVENT_REGION_CHANGED;

/**
 * @brief System event type: Accessibility mode changed.
 * @since_tizen 2.3
 * @feature http://developer.samsung.com/tizen/feature/samsung_extension
 * @feature http://developer.samsung.com/tizen/feature/in_house/shell.appwidget
 */
extern const int DBOX_SYS_EVENT_TTS_CHANGED;

/**
 * @brief System event type: Dynamic Box is paused.
 * @since_tizen 2.3
 * @feature http://developer.samsung.com/tizen/feature/samsung_extension
 * @feature http://developer.samsung.com/tizen/feature/in_house/shell.appwidget
 */
extern const int DBOX_SYS_EVENT_PAUSED;

/**
 * @brief System event type: Dynamic Box is resumed.
 * @since_tizen 2.3
 * @feature http://developer.samsung.com/tizen/feature/samsung_extension
 * @feature http://developer.samsung.com/tizen/feature/in_house/shell.appwidget
 */
extern const int DBOX_SYS_EVENT_RESUMED;

/**
 * @brief System event type: Dynamic Box instance is removed from a viewer.
 * @since_tizen 2.3
 * @feature http://developer.samsung.com/tizen/feature/samsung_extension
 * @feature http://developer.samsung.com/tizen/feature/in_house/shell.appwidget
 */
extern const int DBOX_SYS_EVENT_DELETED;

/**
 * @brief Requests to schedule the update operation to a provider.
 *        Your DBox will get the update request, if the @a id is specified, specific box only will get it.
 * @since_tizen 2.3
 * @param[in] id Instance Id which is passed to you via the first parameter of every dynamicbox_XXXX interface functions
 * @feature http://developer.samsung.com/tizen/feature/samsung_extension
 * @feature http://developer.samsung.com/tizen/feature/in_house/shell.appwidget
 * @return If succes returns 0 or return less than 0
 * @retval #DBOX_STATUS_ERROR_NONE Successfully triggered
 * @retval #DBOX_STATUS_ERROR_INVALID_PARAMETER Invalid argument
 * @retval #DBOX_STATUS_ERROR_OUT_OF_MEMORY Not enough memory
 * @retval #DBOX_STATUS_ERROR_NOT_EXIST Given id instance is not exist
 * @retval #DBOX_STATUS_ERROR_PERMISSION_DENIED Permission denied
 * @retval #DBOX_STATUS_ERROR_DISABLED in case of direct request for updating is disabled
 */
extern int dynamicbox_request_update(const char *id);

/**
 * @brief Send a freeze request to the viewer (homescreen)
 * @details
 *        The viewer will get this request via event callback.
 *        Then it should freeze its scroller or stop moving the dynamicbox.
 * @since_tizen 2.3
 * @remarks If the viewer doesn't care this request, this will has no effect.
 * @param[in] dboxid Dynamic Box Package Id
 * @param[in] id Instance Id which is passed to you via the first parameter of every dynamicbox_XXXX interface functions
 * @privlevel public
 * @privilege %http://developer.samsung.com/tizen/privilege/dynamicbox.provider
 * @feature http://developer.samsung.com/tizen/feature/samsung_extension
 * @feature http://developer.samsung.com/tizen/feature/in_house/shell.appwidget
 * @return int type
 * @retval #DBOX_STATUS_ERROR_INVALID_PARAMETER Invalid parameters
 * @retval #DBOX_STATUS_ERROR_OUT_OF_MEMORY Out of memory
 * @retval #DBOX_STATUS_ERROR_PERMISSION_DENIED Permission denied
 * @retval #DBOX_STATUS_ERROR_FAULT Failed to send requet
 * @retval #DBOX_STATUS_ERROR_NONE Successfully requested
 * @see dynamicbox_release_scroller()
 */
extern int dynamicbox_freeze_scroller(const char *dboxid, const char *id);

/**
 * @brief Send a release request to the viewer (homescreen)
 * @details
 *        The viewer will get this request via event callback.
 *        Then it should release its scroller or continue moving the dynamicbox.
 * @since_tizen 2.3
 * @remarks If the viewer doesn't care this request, this will has no effect.
 * @param[in] dboxid Dynamic Box Package Id
 * @param[in] id Instance Id which is passed to you via the first parameter of every dynamicbox_XXXX interface functions
 * @privlevel public
 * @privilege %http://developer.samsung.com/tizen/privilege/dynamicbox.provider
 * @feature http://developer.samsung.com/tizen/feature/samsung_extension
 * @feature http://developer.samsung.com/tizen/feature/in_house/shell.appwidget
 * @return int type
 * @retval #DBOX_STATUS_ERROR_PERMISSION_DENIED Permission denied
 * @retval #DBOX_STATUS_ERROR_INVALID_PARAMETER Invalid parameters
 * @retval #DBOX_STATUS_ERROR_OUT_OF_MEMORY Out of memory
 * @retval #DBOX_STATUS_ERROR_FAULT Failed to send requet
 * @retval #DBOX_STATUS_ERROR_NONE Successfully requested
 * @see dynamicbox_freeze_scroller()
 */
extern int dynamicbox_thaw_scroller(const char *dboxid, const char *id);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
/* End of a file */
