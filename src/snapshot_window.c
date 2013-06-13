#include <Ecore_Evas.h>
#include <Evas.h>
#include <Ecore.h>
#include <unistd.h>

#include <dlog.h>
#include <livebox-service.h>
#include <livebox-errno.h>

#include "livebox.h"
#include "debug.h"

#if !defined(SECURE_LOGD)
#define SECURE_LOGD LOGD
#endif

#if !defined(SECURE_LOGE)
#define SECURE_LOGE LOGE
#endif

#if !defined(SECURE_LOGW)
#define SECURE_LOGW LOGW
#endif

#define QUALITY_N_COMPRESS "quality=100 compress=1"
#define PUBLIC __attribute__((visibility("default")))

struct snapshot_info {
	char *id;
	void (*flush_cb)(Evas_Object *snapshot_window, const char *id, int status, void *data);
	void *data;

	Ecore_Timer *flush_timer;

	int render_cnt;
	double timeout;
};

static void post_render_cb(void *data, Evas *e, void *event_info);
static void pre_render_cb(void *data, Evas *e, void *event_info);



static inline Evas *create_virtual_canvas(int w, int h)
{
        Ecore_Evas *internal_ee;
        Evas *internal_e;

        // Create virtual canvas
        internal_ee = ecore_evas_buffer_new(w, h);
        if (!internal_ee) {
                LOGD("Failed to create a new canvas buffer\n");
                return NULL;
        }

	ecore_evas_alpha_set(internal_ee, EINA_TRUE);
	ecore_evas_manual_render_set(internal_ee, EINA_FALSE);

        // Get the "Evas" object from a virtual canvas
        internal_e = ecore_evas_get(internal_ee);
        if (!internal_e) {
                ecore_evas_free(internal_ee);
                LOGD("Faield to get Evas object\n");
                return NULL;
        }

        return internal_e;
}



static inline int flush_data_to_file(Evas *e, char *data, const char *filename, int w, int h)
{
        Evas_Object *output;

        output = evas_object_image_add(e);
        if (!output) {
		LOGD("Failed to create an image object\n");
                return LB_STATUS_ERROR_FAULT;
        }

        evas_object_image_data_set(output, NULL);
        evas_object_image_colorspace_set(output, EVAS_COLORSPACE_ARGB8888);
        evas_object_image_alpha_set(output, EINA_TRUE);
        evas_object_image_size_set(output, w, h);
        evas_object_image_smooth_scale_set(output, EINA_TRUE);
        evas_object_image_data_set(output, data);
	evas_object_image_fill_set(output, 0, 0, w, h);
        evas_object_image_data_update_add(output, 0, 0, w, h);

        if (evas_object_image_save(output, filename, NULL, QUALITY_N_COMPRESS) == EINA_FALSE) {
                evas_object_del(output);
		SECURE_LOGD("Faield to save a captured image (%s)\n", filename);
                return LB_STATUS_ERROR_IO;
        }

	evas_object_del(output);

        if (access(filename, F_OK) != 0) {
		SECURE_LOGD("File %s is not found\n", filename);
                return LB_STATUS_ERROR_IO;
        }

	SECURE_LOGD("Flush data to a file (%s)\n", filename);
	return LB_STATUS_SUCCESS;
}



static inline int destroy_virtual_canvas(Evas *e)
{
        Ecore_Evas *ee;

        ee = ecore_evas_ecore_evas_get(e);
        if (!ee) {
		LOGD("Failed to ecore evas object\n");
                return LB_STATUS_ERROR_FAULT;
        }

        ecore_evas_free(ee);
        return LB_STATUS_SUCCESS;
}



static inline int flush_to_file(void *canvas, const char *filename, int w, int h)
{
	int status;
	Evas *shot_e;
	Ecore_Evas *shot_ee;

	shot_e = create_virtual_canvas(w, h);
	if (!shot_e) {
		LOGE("Unable to create a new virtual window\n");
		return LB_STATUS_ERROR_FAULT;
	}

	shot_ee = ecore_evas_ecore_evas_get(shot_e);
	if (!shot_ee) {
		LOGE("Unable to get Ecore_Evas\n");
		destroy_virtual_canvas(shot_e);
		return LB_STATUS_ERROR_FAULT;
	}

	ecore_evas_manual_render_set(shot_ee, EINA_TRUE);

	status = flush_data_to_file(shot_e, canvas, filename, w, h);
	destroy_virtual_canvas(shot_e);

	return status;
}



static void del_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	struct snapshot_info *info;

	info = evas_object_data_del(obj, "snapshot,info");
	if (!info)
		return;

	SECURE_LOGD("Delete object (%s)\n", info->id);

	if (info->flush_cb) {
		info->flush_cb(obj, info->id, LB_STATUS_ERROR_CANCEL, info->data);
		LOGD("Flush is canceled\n");
	}

	/*!
	 * \note
	 * Render callback will be deleted.
	 */
	destroy_virtual_canvas(e);
	free(info->id);
	free(info);
}



static Eina_Bool direct_snapshot_cb(void *data)
{
	Evas *e;
	Ecore_Evas *ee;
	void *canvas;
	int status;
	int w;
	int h;
	void (*flush_cb)(Evas_Object *snapshot_window, const char *id, int status, void *data);
	Evas_Object *snapshot_win = data;
	struct snapshot_info *info;

	info = evas_object_data_get(snapshot_win, "snapshot,info");
	if (!info) {
		LOGE("Unable to get snapshot info\n");
		return ECORE_CALLBACK_CANCEL;
	}

	info->flush_timer = NULL;
	flush_cb = info->flush_cb;
	info->flush_cb = NULL; /* To prevent call this from the delete callback */

	e = evas_object_evas_get(snapshot_win);
	if (!e) {
		LOGE("Invalid object, failed to get Evas\n");
		if (flush_cb)
			flush_cb(snapshot_win, info->id, LB_STATUS_ERROR_FAULT, info->data);
		return ECORE_CALLBACK_CANCEL;
	}

	ee = ecore_evas_ecore_evas_get(e);
	if (!ee) {
		LOGE("Unable to get Ecore_Evas object\n");
		if (flush_cb)
			flush_cb(snapshot_win, info->id, LB_STATUS_ERROR_FAULT, info->data);
		return ECORE_CALLBACK_CANCEL;
	}

	ecore_evas_geometry_get(ee, NULL, NULL, &w, &h);
	ecore_evas_manual_render_set(ee, EINA_TRUE);

	canvas = ecore_evas_buffer_pixels_get(ee);
	if (!canvas) {
		LOGE("Failed to get canvas\n");
		if (flush_cb)
			flush_cb(snapshot_win, info->id, LB_STATUS_ERROR_FAULT, info->data);
		return ECORE_CALLBACK_CANCEL;
	}

	status = flush_to_file(canvas, info->id, w, h);
	if (flush_cb)
		flush_cb(snapshot_win, info->id, status, info->data);
	return ECORE_CALLBACK_CANCEL;
}



static Eina_Bool snapshot_cb(void *data)
{
	Evas_Object *snapshot_win = data;
	struct snapshot_info *info;
	Evas *e;
	Ecore_Evas *ee;
	void *canvas;
	void (*flush_cb)(Evas_Object *snapshot_window, const char *id, int status, void *data);

	info = evas_object_data_get(snapshot_win, "snapshot,info");
	if (!info) {
		LOGE("Invalid object\n");
		return ECORE_CALLBACK_CANCEL;
	}

	info->flush_timer = NULL;
	flush_cb = info->flush_cb;
	info->flush_cb = NULL; /* To prevent call this from the delete callback */

	e = evas_object_evas_get(snapshot_win);
	if (!e) {
		LOGE("Invalid object\n");
		if (flush_cb)
			flush_cb(snapshot_win, info->id, LB_STATUS_ERROR_FAULT, info->data);
		return ECORE_CALLBACK_CANCEL;
	}

	ee = ecore_evas_ecore_evas_get(e);
	if (!ee) {
		LOGE("Invalid object (ee)\n");
		if (flush_cb)
			flush_cb(snapshot_win, info->id, LB_STATUS_ERROR_FAULT, info->data);
		return ECORE_CALLBACK_CANCEL;
	}

	canvas = (void*)ecore_evas_buffer_pixels_get(ee);
	if (!canvas) {
		LOGD("Failed to get pixel canvas\n");
		if (flush_cb)
			flush_cb(snapshot_win, info->id, LB_STATUS_ERROR_FAULT, info->data);
		return ECORE_CALLBACK_CANCEL;
	}

	if (flush_cb) {
		int w;
		int h;
		int status;

		ecore_evas_geometry_get(ee, NULL, NULL, &w, &h);

		SECURE_LOGD("Flush size: %dx%d\n", w, h);
		status = flush_to_file(canvas, info->id, w, h);

		flush_cb(snapshot_win, info->id, status, info->data);
		/*!
		 * Do not access info after this.
		 */
	}

	return ECORE_CALLBACK_CANCEL;
}



static void post_render_cb(void *data, Evas *e, void *event_info)
{
	Evas_Object *snapshot_win = data;
	struct snapshot_info *info;

	info = evas_object_data_get(snapshot_win, "snapshot,info");
	if (!info) {
		LOGE("snapshot info is not valid\n");
		return;
	}

	info->render_cnt++;

	if (info->flush_timer) {
		/*!
		 * This has not to be happens.
		 */
		LOGE("Flush timer is not cleared\n");
		ecore_timer_del(info->flush_timer);
		info->flush_timer = NULL;
	}

	if (!info->flush_cb) {
		LOGD("Flush request is not initiated yet\n");
		return;
	}

	/*!
	 * \NOTE
	 * Even if tehre is no timer registered, we should capture the content
	 * from out of this callback.
	 * Or we can met unexpected problems.
	 * To avoid it, use the 0.0001f to get timer callback ASAP, not in this function.
	 */
	LOGD("Fire the flush timer %lf (%d)\n", info->timeout, info->render_cnt);
	info->flush_timer = ecore_timer_add(info->timeout, snapshot_cb, snapshot_win);
	if (!info->flush_timer) {
		void (*flush_cb)(Evas_Object *snapshot_window, const char *id, int status, void *data);

		LOGE("Unalbe to add timer for getting the snapshot\n");
		flush_cb = info->flush_cb;
		info->flush_cb = NULL;

		flush_cb(snapshot_win, info->id, LB_STATUS_ERROR_FAULT, info->data);
		/*!
		 * \note
		 * Do not access info after from here.
		 */
	}
}



static void pre_render_cb(void *data, Evas *e, void *event_info)
{
	Evas_Object *snapshot_win = data;
	struct snapshot_info *info;

	info = evas_object_data_get(snapshot_win, "snapshot,info");
	if (!info) {
		LOGE("snapshot info is not valid\n");
		return;
	}

	if (info->flush_timer) {
		ecore_timer_del(info->flush_timer);
		info->flush_timer = NULL;
		LOGD("Clear the flush timer\n");
	}

	LOGD("Pre-render callback\n");
}



static void resize_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Ecore_Evas *ee;
	int w;
	int h;
	int ow;
	int oh;

	ee = ecore_evas_ecore_evas_get(e);
	if (!ee)
		return;

	ecore_evas_geometry_get(ee, NULL, NULL, &w, &h);
	evas_object_geometry_get(obj, NULL, NULL, &ow, &oh);
	if (ow == w && oh == h) {
		SECURE_LOGD("Size is not changed: %dx%d\n", w, h);
		return;
	}

	/*!
	 * Box(parent object) is resized.
	 * Try to resize the canvas too.
	 */
	ecore_evas_resize(ee, w, h);
	SECURE_LOGD("Canvas is resized to %dx%d\n", w, h);
}



PUBLIC Evas_Object *livebox_snapshot_window_add(const char *id, int size_type)
{
	struct snapshot_info *info;
	Evas_Object *snapshot_win;
	Evas *e;
	int w;
	int h;

	if (livebox_service_get_size(size_type, &w, &h) != LB_STATUS_SUCCESS) {
		LOGE("Invalid size\n");
		return NULL;
	}

	info = malloc(sizeof(*info));
	if (!info) {
		LOGE("Heap: %s\n", strerror(errno));
		return NULL;
	}

	info->id = strdup(id);
	if (!info->id) {
		LOGE("Heap: %s\n", strerror(errno));
		free(info);
		return NULL;
	}

	info->flush_cb = NULL;
	info->data = NULL;
	info->flush_timer = NULL;
	info->render_cnt = 0;

	e = create_virtual_canvas(w, h);
	if (!e) {
		free(info->id);
		free(info);
		return NULL;
	}

	snapshot_win = evas_object_rectangle_add(e);
	if (!snapshot_win) {
		destroy_virtual_canvas(e);
		free(info->id);
		free(info);
		return NULL;
	}

	SECURE_LOGD("Add new window %dx%d\n", w, h);
	evas_object_event_callback_add(snapshot_win, EVAS_CALLBACK_DEL, del_cb, NULL);
	evas_object_event_callback_add(snapshot_win, EVAS_CALLBACK_RESIZE, resize_cb, NULL);
	evas_object_resize(snapshot_win, w, h);
	evas_object_show(snapshot_win);

	evas_object_data_set(snapshot_win, "snapshot,info", info);

	evas_event_callback_add(e, EVAS_CALLBACK_RENDER_POST, post_render_cb, snapshot_win);
	evas_event_callback_add(e, EVAS_CALLBACK_RENDER_PRE, pre_render_cb, snapshot_win);

	return snapshot_win;
}



PUBLIC int livebox_snapshot_window_flush(Evas_Object *snapshot_win, double timeout, void (*flush_cb)(Evas_Object *snapshot_window, const char *id, int status, void *data), void *data)
{
	struct snapshot_info *info;

	if (!flush_cb || timeout < 0.0f) {
		LOGE("Invalid argument (%p, %lf)\n", flush_cb, timeout);
		return LB_STATUS_ERROR_INVALID;
	}

	info = evas_object_data_get(snapshot_win, "snapshot,info");
	if (!info) {
		LOGE("Invalid argument\n");
		return LB_STATUS_ERROR_INVALID;
	}

	info->timeout = timeout;

	if (timeout == 0.0f) {
		/*!
		 * This timer is just used for guarantees same behavious even if it flushes content directly,
		 * The callback should be called from next loop.
		 * or the developer will get confused
		 */
		info->flush_timer = ecore_timer_add(0.0001f, direct_snapshot_cb, snapshot_win);
		if (!info->flush_timer)
			return LB_STATUS_ERROR_FAULT;
	} else if (info->render_cnt) {
		/*!
		 * Try to watit pre-render callback.
		 * If there is rendered contents.
		 */
		DbgPrint("Rendered %d times already\n", info->render_cnt);
		info->flush_timer = ecore_timer_add(info->timeout, snapshot_cb, snapshot_win);
		if (!info->flush_timer)
			return LB_STATUS_ERROR_FAULT;
	}

	info->flush_cb = flush_cb;
	info->data = data;

	return LB_STATUS_SUCCESS;
}



PUBLIC int livebox_snapshot_window_del(Evas_Object *snapshot_win)
{
	Evas *e;
	if (!snapshot_win || !evas_object_data_get(snapshot_win, "snapshot,info"))
		return LB_STATUS_ERROR_INVALID;

	e = evas_object_evas_get(snapshot_win);
	evas_event_callback_del(e, EVAS_CALLBACK_RENDER_POST, post_render_cb);
	evas_event_callback_del(e, EVAS_CALLBACK_RENDER_PRE, pre_render_cb);

	evas_object_del(snapshot_win);
	return LB_STATUS_SUCCESS;
}



// End of a file
