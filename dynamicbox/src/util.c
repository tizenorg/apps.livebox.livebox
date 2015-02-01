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

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>

#include <Ecore.h>
#include <dlog.h>

#include "debug.h"
#include "util.h"

int errno;

int util_check_extension(const char *filename, const char *check_ptr)
{
    int name_len;

    name_len = strlen(filename);
    while (--name_len >= 0 && *check_ptr) {
	if (filename[name_len] != *check_ptr)
	    return -EINVAL;

	check_ptr ++;
    }

    return 0;
}

double util_timestamp(void)
{
#if defined(_USE_ECORE_TIME_GET)
    return ecore_time_get();
#else
    struct timeval tv;

    gettimeofday(&tv, NULL);

    return (double)tv.tv_sec + (double)tv.tv_usec / 1000000.0f;
#endif
}

/* End of a file */
