/**
 * Libcgroup cpu controller abstraction layer
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
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "abstraction-common.h"

#define DEFAULT_SHARES_VALUE 1024
#define DEFAULT_WEIGHT_VALUE 100

static const char * const cpu_shares = "cpu.shares";
static const char * const cpu_weight = "cpu.weight";

STATIC int v1_shares_to_v2(struct cgroup_controller * const dst_cgc,
			   const char * const shares_val)
{
#define WEIGHT_STR_LEN 20

	long int weight;
	char *weight_str = NULL;
	int ret = 0;

	if (!shares_val)
		return ECGINVAL;

	if (strlen(shares_val) > 0) {
		ret = cgroup_strtol(shares_val, 10, &weight);
		if (ret)
			goto out;

		/* now scale from cpu.shares to cpu.weight */
		weight = weight * DEFAULT_WEIGHT_VALUE / DEFAULT_SHARES_VALUE;

		weight_str = calloc(sizeof(char), WEIGHT_STR_LEN);
		ret = snprintf(weight_str, WEIGHT_STR_LEN, "%ld\n", weight);
		if (ret == WEIGHT_STR_LEN) {
			/* we ran out of room in the string. throw an error */
			cgroup_err("Error: weight too large for string: %d\n",
				   weight);
			ret = ECGFAIL;
			goto out;
		}
	}

	ret = cgroup_add_value_string(dst_cgc, cpu_weight, weight_str);

out:
	if (weight_str)
		free(weight_str);

	return ret;
}

static int v1_to_v2(struct cgroup_controller * const out_cgc,
		    const struct cgroup_controller * const in_cgc)
{
	int i, ret = 0;

	if (in_cgc->version != CGROUP_V1) {
		cgroup_err("Invalid cgroup version");
		return ECGCONTROLLERNOTEQUAL;
	}

	/* At this time, I believe cpu cgroup v1 settings have a one-to-one
	 * mapping to cpu cgroup v2 settings.  If/When this no longer becomes
	 * the case, we can make this function smarter.
	 */
	for (i = 0; i < in_cgc->index; i++) {
		if (strcmp(in_cgc->values[i]->name, cpu_shares) == 0)
			ret = v1_shares_to_v2(out_cgc,
				in_cgc->values[i]->value);
		if (ret)
			goto out;
	}

	if (out_cgc->index == 0)
		/* no settings/values were successfully converted */
		ret = ECGFAIL;

out:
	return ret;
}

static int v2_to_v1(struct cgroup_controller * const out_cgc,
		    const struct cgroup_controller * const in_cgc)
{
	int i, ret = 0;

	if (in_cgc->version != CGROUP_V2) {
		cgroup_err("Invalid cgroup version");
		return ECGCONTROLLERNOTEQUAL;
	}

	/* At this time, I believe cpu cgroup v1 settings have a one-to-one
	 * mapping to cpu cgroup v2 settings.  If/When this no longer becomes
	 * the case, we can make this function smarter.
	 */
	for (i = 0; i < in_cgc->index; i++) {
		/* todo - add conversions here */
	}

	if (out_cgc->index == 0)
		/* no settings/values were successfully converted */
		ret = ECGFAIL;

out:
	return ret;
}

int cgroup_convert_cpu(struct cgroup_controller * const out_cgc,
		       const struct cgroup_controller * const in_cgc)
{
	int ret;

	if (out_cgc->version == CGROUP_UNK ||
	    out_cgc->version == CGROUP_DISK)
	{
		ret = cgroup_get_controller_version(out_cgc->name,
						    &out_cgc->version);
		if (ret)
			goto out;
	}

	if (in_cgc->version == out_cgc->version) {
		ret = cgroup_copy_controller_values(out_cgc, in_cgc);
		/* regardless of success/failure, there's nothing more to do */
		goto out;
	}

	switch (out_cgc->version) {
	case CGROUP_V1:
		ret = v2_to_v1(out_cgc, in_cgc);
		break;
	case CGROUP_V2:
		ret = v1_to_v2(out_cgc, in_cgc);
		break;
	default:
		ret = ECGFAIL;
		break;
	}

out:
	return ret;
}
