/**
 * Libcgroup abstraction layer for the memory controller
 *
 * Copyright (c) 2021 Oracle and/or its affiliates.
 * Author: Tom Hromatka <tom.hromatka@oracle.com>
 */

/*
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of version 2.1 of the GNU Lesser General Public License as
 * published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, see <http://www.gnu.org/licenses>.
 */

#include <libcgroup.h>
#include <libcgroup-internal.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "abstraction-common.h"
#include "abstraction-map.h"

static const char * const V1_MAX = "9223372036854771712";
static const char * const V2_MAX = "max";

int cgroup_convert_memory_max_to_limit(
	struct cgroup_controller * const dst_cgc,
	const char * const in_value,
	const char * const out_setting,
	void *in_dflt, void *out_dflt)
{
	if (strlen(in_value) == 0) {
		/* There's no value to convert.  Populate the setting */
		return cgroup_add_value_string(dst_cgc, out_setting, NULL);
	} else {
		if (strcmp(in_value, V2_MAX) == 0)
			return cgroup_add_value_string(dst_cgc, out_setting,
				V1_MAX);
		else
			return cgroup_add_value_string(dst_cgc, out_setting,
				in_value);
	}
}

int cgroup_convert_memory_limit_to_max(
	struct cgroup_controller * const dst_cgc,
	const char * const in_value,
	const char * const out_setting,
	void *in_dflt, void *out_dflt)
{
	if (strlen(in_value) == 0) {
		/* There's no value to convert.  Populate the setting */
		return cgroup_add_value_string(dst_cgc, out_setting, NULL);
	} else {
		if (strcmp(in_value, V1_MAX) == 0)
			return cgroup_add_value_string(dst_cgc, out_setting,
				V2_MAX);
		else
			return cgroup_add_value_string(dst_cgc, out_setting,
				in_value);
	}
}
