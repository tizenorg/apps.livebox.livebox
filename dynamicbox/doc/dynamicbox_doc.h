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


#ifndef __DYNAMICBOX_DOC_H__
#define __DYNAMICBOX_DOC_H__

/**
 * @defgroup DYNAMICBOX_UTILITY_MODULE dynamicbox
 * @brief Supports APIs for development of inhouse dynamic box
 * @ingroup CAPI_DYNAMICBOX_FRAMEWORK 
 * @section DYNAMICBOX_UTILITY_MODULE_HEADER Required Header
 *   \#include <dynamicbox.h>
 * @section DYNAMICBOX_UTILITY_MODULE_OVERVIEW Overview
<H1>Dynamic Box Utility functions</H1>

<H2>1. Script type Dynamic Box</H2>
If you choose the Script Type DBox, you have to use this.

Dynamic Box defines its own syntax to share the content between home application and provider application.
It is called "Description Data Syntax"

To generate it you have to use following functions.

@code
struct dynamicbox_desc *desc_handle;
int idx;

desc_handle = dynamicbox_desc_open(id, 0);
if (!desc_handle) {
    // Error
}

dynamicbox_desc_set_size(desc_handle, id, 720, 360);

// Loads sub-layout object into main layout
idx = dynamicbox_desc_add_block(desc_handle, NULL, DBOX_DESC_TYPE_SCRIPT, "sub,layout", "/usr/apps/com.samsung.my-app/shared/res/dbox.edj", "sub,group");
dynamicbox_desc_set_id(desc_handle, idx, "sub,layout");

// Set a text for sub-layout object
dynamicbox_desc_add_block(desc_handle, "sub,layout", DBOX_DESC_TYPE_TEXT, "sub,layout,text", "Hello World", NULL);

// Flushes changes, the content will be changed after close this handle.
dynamicbox_desc_close(desc_handle);
desc_handle = NULL;
@endcode

Only after you close the desc_handle, the provider will send changes to the dynamic box manager service.
And if it needs to propagate events to the home application, the home application will get changes event.

<H2>2. Window(buffer) type Dynamic Box</H2>

@code
Evas_Object *parent;
Evas_Object *win;

parent = dynamicbox_get_evas_object(id, 0);
if (!parent) {
    // Error
}

win = elm_win_add(parent, "Dynamic Box Window", ELM_WIN_DYNAMIC_BOX);
evas_object_del(parent);
if (!win) {
    // Error
}
@endcode

To create a window for dynamic box,
You have to get the parent object using dynamicbox_get_evas_object().
And after creating a window for dynamic box, you have to delete it.
Its attributes will be passed to the newly created window. So you don't need keep the parent object anymore.

After creating a window, you can do what you want like an application.
Creatig any core-control such as button, list, etc, ....

<H2>3. Image type Dynamic Box</H2>
This kind of dynamic box should create an image file using given Id.
The Id's syntax is "file://ABS_PATH_OF_OUTPUT_FILE", so you should create an image file using this URI.
The Id will be assigned to every dynamic box instances.
And those are unique.

If you create an image file, you should notify it to the viewer using dynamicbox_provider_app_dbox_updated()
it is provided by libdynamicbox_provider_app package.

<H2>4. Text type Dynamic Box (Experimental)</H2>
In case of text type, you may have no window or script file.
The text type dynamic box only needs to generate content data using dynamicbox_desc_XXXX series APIs.
Of course, after you prepare the desc file, you have to call dynamicbox_provider_app_dbox_updated() function too.
Then the viewer will get your updated contents and try to parse it to locate content of dynamic box to its screen.
But, unfortunately, this type of dynamic box is not fully supported yet.
Because it very highly depends on the viewer implementations.
So if the viewer doesn't support this type of dynamic box, you cannot do anything for user.

To generate the text data, you can use below API set.

 - dynamicbox_desc_open()
 - dynamicbox_desc_set_category()
 - dynamicbox_desc_set_size()
 - dynamicbox_desc_set_id()
 - dynamicbox_desc_add_block()
 - dynamicbox_desc_del_block()
 - dynamicbox_desc_close()

Here is a sample code for you.
\code
#define FOR_GBAR 1
#define FOR_DBOX 0

struct dynamicbox_desc *handle;
int idx;

handle = dynamicbox_desc_open(handle, FOR_GBAR); // The second parameter will help you choose to target, for glance bar or dynamic box?
idx = dynamicbox_desc_add_block(handle, NULL, DBOX_DESC_TYPE_SCRIPT, "/opt/usr/apps/your.company.application/shared/resource/edje/script.edj", "sample", NULL);
dynamicbox_desc_set_id(handle, idx, "sample");
dynamicbox_desc_close(handle);
dynamicbox_provider_app_dbox_updated(dbox_id, 0, 0);
\endocde

 */

#endif /* __DYNAMICBOX_DOC_H__ */
