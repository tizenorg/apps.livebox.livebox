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

#define DbgPrint(format, arg...)    SECURE_LOGD(format, ##arg)
#define ErrPrint(format, arg...)    SECURE_LOGE(format, ##arg)
#define WarnPrint(format, arg...)   SECURE_LOGW(format, ##arg)

#if defined(_ENABLE_PERF)
#define PERF_INIT() \
	struct timeval __stv; \
	struct timeval __etv; \
	struct timeval __rtv

#define PERF_BEGIN() do { \
	if (gettimeofday(&__stv, NULL) < 0) { \
		ErrPrint("gettimeofday: %s\n", strerror(errno)); \
	} \
} while (0)

#define PERF_MARK(tag) do { \
	if (gettimeofday(&__etv, NULL) < 0) { \
		ErrPrint("gettimeofday: %s\n", strerror(errno)); \
	} \
	timersub(&__etv, &__stv, &__rtv); \
	DbgPrint("[%s] %u.%06u\n", tag, __rtv.tv_sec, __rtv.tv_usec); \
} while (0)
#else
#define PERF_INIT()
#define PERF_BEGIN()
#define PERF_MARK(tag)
#endif

/* End of a file */
