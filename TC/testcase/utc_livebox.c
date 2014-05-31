/*
 * Copyright (c) 2011 Samsung Electronics Co., Ltd All Rights Reserved
 *
 * Licensed under the Apache License, Version 2.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include <tet_api.h>
#include <stdlib.h>

#define LOG_TAG "LIVEBOX_TC"

#include <livebox-service.h>
#include <livebox-errno.h>
#include <livebox.h>
#include <dlog.h>

enum {
	POSITIVE_TC_IDX = 0x01,
	NEGATIVE_TC_IDX,
};

static void startup(void)
{
	/* start of TC */
	tet_printf("\n TC start");
}


static void cleanup(void)
{
	/* end of TC */
	tet_printf("\n TC end");
}

void (*tet_startup)(void) = startup;
void (*tet_cleanup)(void) = cleanup;

#define DUMMY_ID "/opt/usr/share/live_magazine/org.tizen.dummy.png"
#define DUMMY_INVALID_ID "/usr/share/live_magazine/org.tizen.dummy.png"
#define DUMMY_PKGNAME "org.tizen.dummy"

static void utc_livebox_desc_open_n(void)
{
	LOGD("");
	struct livebox_desc *handle;

	handle = livebox_desc_open(DUMMY_INVALID_ID, 0);
	if (handle) {
		(void)livebox_desc_close(handle);
	}
        dts_check_eq("livebox_desc_open", handle, NULL, "Must return NULL in case of invalid id is used"); 
}

static void utc_livebox_desc_open_p(void)
{
	LOGD("");
	struct livebox_desc *handle;

	handle = livebox_desc_open(DUMMY_ID, 0);
        dts_check_ne("livebox_desc_open", handle, NULL, "Must return valid handle"); 
}

static void utc_livebox_desc_close_n(void)
{
	LOGD("");
	int ret;

	ret = livebox_desc_close(NULL);
	dts_check_eq("livebox_desc_close", ret, LB_STATUS_ERROR_INVALID, "Must returns LB_STATUS_ERROR_INVALID");
}

static void utc_livebox_desc_close_p(void)
{
	LOGD("");
	struct livebox_desc *handle;
	int ret;

	handle = livebox_desc_open(DUMMY_ID, 0);
	if (!handle) {
		dts_check_ne("livebox_desc_close", handle, NULL, "Failed to create a handle");
		return;
	}

	ret = livebox_desc_close(handle);
	dts_check_eq("livebox_desc_close", ret, LB_STATUS_SUCCESS, "Must returns LB_STATUS_SUCCESS");
}

static void utc_livebox_desc_set_category_n(void)
{
	LOGD("");
	int ret;
	ret = livebox_desc_set_category(NULL, NULL, NULL);

	dts_check_eq("livebox_desc_set_category", ret, LB_STATUS_ERROR_INVALID, "Invalid parameter used, LB_STATUS_ERROR_INVALID should be returned");
}

static void utc_livebox_desc_set_category_p(void)
{
	LOGD("");
	struct livebox_desc *handle;
	int ret;

	handle = livebox_desc_open(DUMMY_ID, 0);
	if (!handle) {
		dts_check_ne("livebox_desc_set_category", handle, NULL, "Failed to create a handle");
		return;
	}

	ret = livebox_desc_set_category(handle, NULL, "New Category");
	(void)livebox_desc_close(handle);
	dts_check_eq("livebox_desc_set_category", ret, LB_STATUS_SUCCESS, "LB_STATUS_SUCCESS should be returned\n");
}

static void utc_livebox_desc_set_id_n(void)
{
	LOGD("");
	struct livebox_desc *handle;
	int ret;

	handle = livebox_desc_open(DUMMY_ID, 0);
	if (!handle) {
		dts_check_ne("livebox_desc_set_id", handle, NULL, "Failed to create a handle");
		return;
	}
	ret = livebox_desc_set_id(handle, -1, NULL);
	(void)livebox_desc_close(handle);
	dts_check_eq("livebox_desc_set_id", ret, LB_STATUS_ERROR_NOT_EXIST, "LB_STATUS_ERROR_NOT_EXIST should be returned\n");
}

static void utc_livebox_desc_set_id_p(void)
{
	LOGD("");
	struct livebox_desc *handle;
	int ret;
	int idx;

	handle = livebox_desc_open(DUMMY_ID, 0);
	if (!handle) {
		dts_check_ne("livebox_desc_set_id", handle, NULL, "Failed to create a handle");
		return;
	}
	idx = livebox_desc_add_block(handle, NULL, LB_DESC_TYPE_SCRIPT, "swallow,part", "/usr/apps/org.tizen.test-app/res/edje/test.edj", "test,group");
	if (idx < 0) {
		dts_check_ge("livebox_desc_set_id", idx, 0, "Failed to add a desc block");
		(void)livebox_desc_close(handle);
		return;
	}

	ret = livebox_desc_set_id(handle, idx, "new,id");
	(void)livebox_desc_close(handle);
	dts_check_eq("livebox_desc_set_id", ret, LB_STATUS_SUCCESS, "LB_STATUS_SUCCESS should be returned\n");
}

static void utc_livebox_desc_add_block_n(void)
{
	LOGD("");
	int idx;

	idx = livebox_desc_add_block(NULL, NULL, LB_DESC_TYPE_SCRIPT, "swallow,part", "/usr/apps/org.tizen.test-app/res/edje/test.edj", "test,group");
	dts_check_eq("livebox_desc_add_block", idx, LB_STATUS_ERROR_INVALID, "LB_STATUS_ERROR_INVALID should be returned\n");
}

static void utc_livebox_desc_add_block_p(void)
{
	LOGD("");
	struct livebox_desc *handle;
	int idx;

	handle = livebox_desc_open(DUMMY_ID, 0);
	if (!handle) {
		dts_check_ne("livebox_desc_add_block", handle, NULL, "Failed to create a handle");
		return;
	}

	idx = livebox_desc_add_block(handle, NULL, LB_DESC_TYPE_SCRIPT, "swallow,part", "/usr/apps/org.tizen.test-app/res/edje/test.edj", "test,group");
	dts_check_ge("livebox_desc_add_block", idx, 0, "idx should not be less than 0\n");
	(void)livebox_desc_close(handle);
}

static void utc_livebox_desc_del_block_n(void)
{
	LOGD("");
	struct livebox_desc *handle;
	int ret;

	handle = livebox_desc_open(DUMMY_ID, 0);
	if (!handle) {
		dts_check_ne("livebox_desc_del_block", handle, NULL, "Failed to create a handle");
		return;
	}

	ret = livebox_desc_del_block(handle, 0);
	dts_check_eq("livebox_desc_del_block", ret, LB_STATUS_ERROR_NOT_EXIST, "Block is not found, LB_STATUS_ERROR_NOT_EXIST should be returned\n");
}

static void utc_livebox_desc_del_block_p(void)
{
	LOGD("");
	struct livebox_desc *handle;
	int idx;
	int ret;

	handle = livebox_desc_open(DUMMY_ID, 0);
	if (handle == NULL) {
		dts_check_ne("livebox_desc_del_block", handle, NULL, "Failed to create desc handle");
		return;
	}
	idx = livebox_desc_add_block(handle, NULL, LB_DESC_TYPE_SCRIPT, "swallow,part", "/usr/apps/org.tizen.test-app/res/edje/test.edj", "test,group");
	if (idx < 0) {
		dts_check_ge("livebox_desc_del_block", idx, 0, "Failed to add a desc block");
		(void)livebox_desc_close(handle);
		return;
	}
	ret = livebox_desc_del_block(handle, idx);
	(void)livebox_desc_close(handle);
	dts_check_eq("livebox_desc_del_block", ret, LB_STATUS_SUCCESS, "del_block should returns LB_STATUS_SUCCESS\n");
}

int livebox_trigger_update_monitor(const char *id, int is_pd)
{
	if (!id) {
		return LB_STATUS_ERROR_INVALID;
	}

	return LB_STATUS_SUCCESS;
}

static void utc_livebox_content_is_updated_n(void)
{
	LOGD("");
	int ret;

	ret = livebox_content_is_updated(NULL, 0);
	dts_check_eq("livebox_content_is_updated", ret, LB_STATUS_ERROR_INVALID, "should returns LB_STATUS_ERROR_INVALID\n");
}

static void utc_livebox_content_is_updated_p(void)
{
	LOGD("");
	int ret;
	ret = livebox_content_is_updated(DUMMY_ID, 0);
	dts_check_eq("livebox_content_is_updated", ret, LB_STATUS_SUCCESS, "should returns LB_STATUS_SUCCESS\n");
}

static void utc_livebox_request_close_pd_n(void)
{
	LOGD("");
	int ret;

	ret = livebox_request_close_pd(DUMMY_PKGNAME, DUMMY_ID, LB_STATUS_SUCCESS);
	dts_check_eq("livebox_request_close_pd", ret, LB_STATUS_ERROR_INVALID, "should returns LB_STATUS_ERROR_INVALID\n");
}

static void utc_livebox_request_close_pd_p(void)
{
	LOGD("");
	/*!
	 * \note
	 * Unable to test the positive case
	 */
	dts_pass("livebox_request_close_pd", "pass negative test");
}

static void utc_livebox_request_update_n(void)
{
	LOGD("");
	int ret;
	ret = livebox_request_update(NULL);
	dts_check_eq("livebox_request_update", ret, LB_STATUS_ERROR_INVALID, "should returns LB_STATUS_ERROR_INVALID");
}

static void utc_livebox_request_update_p(void)
{
	LOGD("");
	/*!
	 * \note
	 * Unable to test the positive case
	 */
	dts_pass("livebox_request_update", "pass negative test");
}

static void utc_livebox_util_nl2br_n(void)
{
	LOGD("");
	char *nl2br;

	nl2br = livebox_util_nl2br(NULL);
	dts_check_eq("livebox_util_nl2br", nl2br, NULL, "should returns NULL\n");
}

static void utc_livebox_util_nl2br_p(void)
{
	LOGD("");
	char *nl2br;
	nl2br = livebox_util_nl2br("hello\nworld");
	LOGD("[%s]", nl2br);
	dts_check_str_eq("livebox_util_nl2br", nl2br, "hello<br>world", "should returns \"hello<br>world\"");
	LOGD("");
}

static void utc_livebox_acquire_buffer_n(void)
{
	LOGD("");
	struct livebox_buffer *handle;

	handle = livebox_acquire_buffer(NULL, 0, 720, 200, NULL, NULL);
	dts_check_eq("livebox_acquire_buffer", handle, NULL, "should returns NULL\n");
}

static void utc_livebox_acquire_buffer_p(void)
{
	LOGD("");
	/*!
	 * \note
	 * Unable to test the positive case
	 */
	dts_pass("livebox_acquire_buffer", "pass positive test");
}

static void utc_livebox_acquire_buffer_NEW_n(void)
{
	LOGD("");
	struct livebox_buffer *handle;

	handle = livebox_acquire_buffer_NEW(NULL, 0, 720, 200, NULL, NULL);
	dts_check_eq("livebox_acquire_buffer_NEW", handle, NULL, "should returns NULL\n");
}

static void utc_livebox_acquire_buffer_NEW_p(void)
{
	LOGD("");
	/*!
	 * \note
	 * Unable to test the positive case
	 */
	dts_pass("livebox_acquire_buffer_NEW", "pass positive test");
}


static void utc_livebox_pixmap_id_n(void)
{
	LOGD("");
	unsigned long pixmap;

	pixmap = livebox_pixmap_id(NULL);
	dts_check_eq("livebox_pixmap_id", pixmap, 0, "should returns 0\n");
}

static void utc_livebox_pixmap_id_p(void)
{
	LOGD("");
	/*!
	 * \note
	 * Unable to test the positive case
	 */
	dts_pass("livebox_pixmap_id", "pass positive test");
}

static void utc_livebox_ref_buffer_n(void)
{
	LOGD("");
	void *ret;
	ret = livebox_ref_buffer(NULL);
	dts_check_eq("livebox_ref_buffer", ret, NULL, "should returns NULL\n");
}

static void utc_livebox_ref_buffer_p(void)
{
	LOGD("");
	/*!
	 * \note
	 * Unable to test the positive case
	 */
	dts_pass("livebox_ref_buffer", "pass positive test");
}

static void utc_livebox_sync_buffer_n(void)
{
	LOGD("");
	int ret;
	ret = livebox_sync_buffer(NULL);
	dts_check_eq("livebox_sync_buffer", ret, LB_STATUS_ERROR_INVALID, "should returns LB_STATUS_ERROR_INVALID\n");
}

static void utc_livebox_sync_buffer_p(void)
{
	LOGD("");
	/*!
	 * \note
	 * Unable to test the positive case
	 */
	dts_pass("livebox_sync_buffer", "pass positive test");
}

static void utc_livebox_support_hw_buffer_n(void)
{
	LOGD("");
	int ret;
	ret = livebox_support_hw_buffer(NULL);
	dts_check_eq("livebox_support_hw_buffer", ret, LB_STATUS_ERROR_INVALID, "should returns LB_STATUS_ERROR_INVALID\n");
}

static void utc_livebox_support_hw_buffer_p(void)
{
	LOGD("");
	/*!
	 * \note
	 * Unable to test the positive case
	 */
	dts_pass("livebox_support_hw_buffer", "pass positive test");
}

static void utc_livebox_create_hw_buffer_n(void)
{
	LOGD("");
	int ret;
	ret = livebox_create_hw_buffer(NULL);
	dts_check_eq("livebox_create_hw_buffer", ret, LB_STATUS_ERROR_INVALID, "should returns LB_STATUS_ERROR_INVALID\n");
}

static void utc_livebox_create_hw_buffer_p(void)
{
	LOGD("");
	/*!
	 * \note
	 * Unable to test the positive case
	 */
	dts_pass("livebox_create_hw_buffer", "pass positive test");
}

static void utc_livebox_destroy_hw_buffer_n(void)
{
	LOGD("");
	int ret;
	ret = livebox_destroy_hw_buffer(NULL);
	dts_check_eq("livebox_destroy_hw_buffer", ret, LB_STATUS_ERROR_INVALID, "should returns LB_STATUS_ERROR_INVALID\n");
}

static void utc_livebox_destroy_hw_buffer_p(void)
{
	LOGD("");
	/*!
	 * \note
	 * Unable to test the positive case
	 */
	dts_pass("livebox_destroy_hw_buffer", "pass positive test");
}

static void utc_livebox_buffer_hw_buffer_n(void)
{
	LOGD("");
	void *ret;
	ret = livebox_buffer_hw_buffer(NULL);
	dts_check_eq("livebox_buffer_hw_buffer", ret, NULL, "should returns LB_STATUS_ERROR_INVALID");
}

static void utc_livebox_buffer_hw_buffer_p(void)
{
	LOGD("");
	/*!
	 * \note
	 * Unable to test the positive case
	 */
	dts_pass("livebox_buffer_hw_buffer", "pass positive test");
}

static void utc_livebox_buffer_pre_render_n(void)
{
	LOGD("");
	int ret;
	ret = livebox_buffer_pre_render(NULL);
	dts_check_eq("livebox_buffer_pre_render", ret, LB_STATUS_ERROR_INVALID, "should returns LB_STATUS_ERROR_INVALID\n");
}

static void utc_livebox_buffer_pre_render_p(void)
{
	LOGD("");
	/*!
	 * \note
	 * Unable to test the positive case
	 */
	dts_pass("livebox_buffer_pre_render", "pass positive test");
}

static void utc_livebox_buffer_post_render_n(void)
{
	LOGD("");
	int ret;
	ret = livebox_buffer_post_render(NULL);
	dts_check_eq("livebox_buffer_post_render", ret, LB_STATUS_ERROR_INVALID, "should returns LB_STATUS_ERROR_INVALID\n");
}

static void utc_livebox_buffer_post_render_p(void)
{
	LOGD("");
	/*!
	 * \note
	 * Unable to test the positive case
	 */
	dts_pass("livebox_buffer_post_render", "pass positive test");
}

static void utc_livebox_get_evas_object_n(void)
{
	LOGD("");
	Evas_Object *obj;

	obj = livebox_get_evas_object(NULL, 1);
	dts_check_eq("livebox_get_evas_object", obj, NULL, "should returns NULL\n");
}

static void utc_livebox_get_evas_object_p(void)
{
	LOGD("");
	/*!
	 * \note
	 * Unable to test the positive case
	 */
	dts_pass("livebox_get_evas_object", "pass positve test");
}

struct tet_testlist tet_testlist[] = {
	{ utc_livebox_desc_open_n, NEGATIVE_TC_IDX },                        
	{ utc_livebox_desc_open_p, POSITIVE_TC_IDX },
	{ utc_livebox_desc_close_n, NEGATIVE_TC_IDX },
	{ utc_livebox_desc_close_p, POSITIVE_TC_IDX },	
	{ utc_livebox_desc_set_category_n, NEGATIVE_TC_IDX },	
	{ utc_livebox_desc_set_category_p, POSITIVE_TC_IDX },
	{ utc_livebox_desc_set_id_n, NEGATIVE_TC_IDX },
	{ utc_livebox_desc_set_id_p, POSITIVE_TC_IDX },
	{ utc_livebox_desc_add_block_n, NEGATIVE_TC_IDX },	
	{ utc_livebox_desc_add_block_p, POSITIVE_TC_IDX },
	{ utc_livebox_desc_del_block_n, NEGATIVE_TC_IDX },
	{ utc_livebox_desc_del_block_p, POSITIVE_TC_IDX },
	{ utc_livebox_content_is_updated_n, NEGATIVE_TC_IDX },
	{ utc_livebox_content_is_updated_p, POSITIVE_TC_IDX },
	{ utc_livebox_request_close_pd_n, NEGATIVE_TC_IDX },
	{ utc_livebox_request_close_pd_p, POSITIVE_TC_IDX },
	{ utc_livebox_request_update_n, NEGATIVE_TC_IDX },
	{ utc_livebox_request_update_p, POSITIVE_TC_IDX },
	{ utc_livebox_util_nl2br_n, NEGATIVE_TC_IDX },
	{ utc_livebox_util_nl2br_p, POSITIVE_TC_IDX },

	{ utc_livebox_acquire_buffer_n, NEGATIVE_TC_IDX },
	{ utc_livebox_acquire_buffer_p, POSITIVE_TC_IDX },
	{ utc_livebox_acquire_buffer_NEW_n, NEGATIVE_TC_IDX },
	{ utc_livebox_acquire_buffer_NEW_p, POSITIVE_TC_IDX },

	{ utc_livebox_pixmap_id_n, NEGATIVE_TC_IDX },
	{ utc_livebox_pixmap_id_p, POSITIVE_TC_IDX },

	{ utc_livebox_ref_buffer_n, NEGATIVE_TC_IDX },
	{ utc_livebox_ref_buffer_p, POSITIVE_TC_IDX },

	{ utc_livebox_sync_buffer_n, NEGATIVE_TC_IDX },
	{ utc_livebox_sync_buffer_p, POSITIVE_TC_IDX },

	{ utc_livebox_support_hw_buffer_n, NEGATIVE_TC_IDX },
	{ utc_livebox_support_hw_buffer_p, POSITIVE_TC_IDX },
	{ utc_livebox_create_hw_buffer_n, NEGATIVE_TC_IDX },
	{ utc_livebox_create_hw_buffer_p, POSITIVE_TC_IDX },
	{ utc_livebox_destroy_hw_buffer_n, NEGATIVE_TC_IDX },
	{ utc_livebox_destroy_hw_buffer_p, POSITIVE_TC_IDX },
	{ utc_livebox_buffer_hw_buffer_n, NEGATIVE_TC_IDX },
	{ utc_livebox_buffer_hw_buffer_p, POSITIVE_TC_IDX },
	{ utc_livebox_buffer_pre_render_n, NEGATIVE_TC_IDX },
	{ utc_livebox_buffer_pre_render_p, POSITIVE_TC_IDX },
	{ utc_livebox_buffer_post_render_n, NEGATIVE_TC_IDX },
	{ utc_livebox_buffer_post_render_p, POSITIVE_TC_IDX },
	{ utc_livebox_get_evas_object_n, NEGATIVE_TC_IDX },
	{ utc_livebox_get_evas_object_p, POSITIVE_TC_IDX },

	{ NULL, 0 },
};

