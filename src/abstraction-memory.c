// SPDX-License-Identifier: LGPL-2.1-only
/**
 * Libcgroup abstraction layer for the cpu controller
 *
 * Copyright (c) 2021-2022 Oracle and/or its affiliates.
 * Author: Tom Hromatka <tom.hromatka@oracle.com>
 */

#include "abstraction-common.h"
#include "abstraction-map.h"

#include <libcgroup.h>
#include <libcgroup-internal.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>

#define LL_MAX 8192

static const char * const V2_MAX = "max";
static const char * const V1_MAX = "9223372036854771712";

static int read_setting(const char * const cgrp_name, const char * const controller_name,
			const char * const setting_name, char ** const value)
{
	char tmp_line[LL_MAX];
	void *handle;
	int ret;

	//fprintf(stderr, "%s:%d setting = %s\n", __func__, __LINE__, setting_name);
	ret = cgroup_read_value_begin(controller_name, cgrp_name, setting_name, &handle,
				      tmp_line, LL_MAX);
	//fprintf(stderr, "%s:%d, ret = %d\n", __func__, __LINE__, ret);
	if (ret == ECGEOF)
		goto read_end;
	else if (ret != 0)
		goto end;
	//fprintf(stderr, "%s:%d\n", __func__, __LINE__);

	*value = strdup(tmp_line);
	if ((*value) == NULL)
		ret = ECGOTHER;
	//fprintf(stderr, "%s:%d\n", __func__, __LINE__);

read_end:
	//fprintf(stderr, "%s:%d\n", __func__, __LINE__);
	cgroup_read_value_end(&handle);
	if (ret == ECGEOF)
		ret = 0;
end:
	//fprintf(stderr, "%s:%d\n", __func__, __LINE__);
	return ret;
}

static int convert_limit_to_max(struct cgroup_controller * const dst_cgc,
		const char * const in_setting, const char * const in_value,
		const char * const out_setting, void *in_dflt, void *out_dflt)
{
	char max_line[LL_MAX] = {0};
	char *limit = NULL;
	int ret;

	//fprintf(stderr, "%s:%d\n", __func__, __LINE__);
	if (strlen(in_value) == 0) {
		//fprintf(stderr, "%s:%d\n", __func__, __LINE__);
		/* There's no value to convert.  Populate the setting */
		ret = cgroup_add_value_string(dst_cgc, out_setting, NULL);
		if (ret)
			goto out;
		//fprintf(stderr, "%s:%d\n", __func__, __LINE__);
	} else {
		ret = read_setting(dst_cgc->cgroup->name, "memory", in_setting, &limit);
		if (ret)
			goto out;

		//fprintf(stderr, "in dflt = %s\n", (char *)in_dflt);
		//fprintf(stderr, "limit = %s\n", limit);
		if (strcmp(limit, in_dflt) == 0)
			snprintf(max_line, LL_MAX, "%s", V2_MAX);
		else
			snprintf(max_line, LL_MAX, "%s", limit);

		ret = cgroup_add_value_string(dst_cgc, out_setting, max_line);
		if (ret)
			goto out;
	}

out:
	//fprintf(stderr, "%s:%d\n", __func__, __LINE__);
	if (limit)
		free(limit);

	return ret;
}
int cgroup_convert_memory_max_limit_to_max(struct cgroup_controller * const dst_cgc,
				const char * const in_value, const char * const out_setting,
				void *in_dflt, void *out_dflt)
{
	return convert_limit_to_max(dst_cgc, "memory.limit_in_bytes", in_value, out_setting,
				    in_dflt, out_dflt);
}

int cgroup_convert_memory_high_limit_to_max(struct cgroup_controller * const dst_cgc,
				const char * const in_value, const char * const out_setting,
				void *in_dflt, void *out_dflt)
{
	return convert_limit_to_max(dst_cgc, "memory.soft_limit_in_bytes", in_value, out_setting,
				    in_dflt, out_dflt);
}

static int convert_max_to_limit(struct cgroup_controller * const dst_cgc,
		const char * const in_setting,  const char * const in_value,
		const char * const out_setting, void *in_dflt, void *out_dflt)
{
	char max_line[LL_MAX] = {0};
	char *limit = NULL;
	int ret;

	//fprintf(stderr, "%s:%d\n", __func__, __LINE__);
	if (strlen(in_value) == 0) {
		/* There's no value to convert.  Populate the setting and return */
		return cgroup_add_value_string(dst_cgc, out_setting, NULL);
	} else {
		//fprintf(stderr, "%s:%d\n", __func__, __LINE__);
		ret = read_setting(dst_cgc->cgroup->name, "memory", in_setting, &limit);
		if (ret)
			goto out;

		//fprintf(stderr, "%s:%d\n", __func__, __LINE__);
		//fprintf(stderr, "in dflt = %s\n", (char *)in_dflt);
		//fprintf(stderr, "limit = %s\n", limit);
		if (strcmp(limit, in_dflt) == 0)
			snprintf(max_line, LL_MAX, "%s", V1_MAX);
		else
			snprintf(max_line, LL_MAX, "%s", limit);

		ret = cgroup_add_value_string(dst_cgc, out_setting, max_line);
		if (ret)
			goto out;
	}

out:
	if (limit)
		free(limit);

	return ret;
}

int cgroup_convert_memory_max_max_to_limit(struct cgroup_controller * const dst_cgc,
				const char * const in_value, const char * const out_setting,
				void *in_dflt, void *out_dflt)
{
	return convert_max_to_limit(dst_cgc, "memory.max", in_value, out_setting,
				    in_dflt, out_dflt);
}

int cgroup_convert_memory_high_max_to_limit(struct cgroup_controller * const dst_cgc,
				const char * const in_value, const char * const out_setting,
				void *in_dflt, void *out_dflt)
{
	return convert_max_to_limit(dst_cgc, "memory.high", in_value, out_setting,
				    in_dflt, out_dflt);
}
