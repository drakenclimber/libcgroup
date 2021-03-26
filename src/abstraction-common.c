/**
 * Libcgroup abstraction layer
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

int cgroup_strtol(const char * const in_str, int base,
		  long int * const out_value)
{
	char *endptr;
	int ret = 0;

	*out_value = strtol(in_str, &endptr, base);

	/* taken directly from strtol's man page */
	if ((errno == ERANGE &&
	     (*out_value == LONG_MAX || *out_value == LONG_MIN))
	    || (errno != 0 && *out_value == 0)) {
		cgroup_err("Error: Failed to convert %s from strtol: %s\n",
			   in_str);
		ret = ECGFAIL;
		goto out;
	}

	if (endptr == in_str) {
		cgroup_err("Error: No long value found in %s\n",
			   in_str);
		ret = ECGFAIL;
		goto out;
	}

out:
	return ret;
}

int cgroup_convert_cgroup(struct cgroup * const out_cgroup,
			  enum cg_version_t out_version,
			  const struct cgroup * const in_cgroup,
			  enum cg_version_t in_version)
{
	const struct cgroup_abstraction_map *convert_tbl;
	struct cgroup_controller *cgc;
	int tbl_sz = 0;
	int ret = 0;
	int i, j, k;

	for (i = 0; i < in_cgroup->index; i++) {
		cgc = cgroup_add_controller(out_cgroup,
					    in_cgroup->controller[i]->name);
		if (cgc == NULL) {
			ret = ECGFAIL;
			goto out;
		}

		/* the user has overridden the version */
		if (in_version == CGROUP_V1 || in_version == CGROUP_V2) {
			in_cgroup->controller[i]->version = in_version;
		}

		cgc->version = out_version;

		if (cgc->version == CGROUP_UNK ||
		    cgc->version == CGROUP_DISK) {
			ret = cgroup_get_controller_version(cgc->name,
				&cgc->version);
			if (ret)
				goto out;
		}

		if (in_version == cgc->version) {
			ret = cgroup_copy_controller_values(cgc,
				in_cgroup->controller[i]);
			/* regardless of success/failure, there's nothing
			 * more to do */
			goto out;
		}

		switch (cgc->version) {
		case CGROUP_V1:
			convert_tbl = cgroup_v2_to_v1_map;
			tbl_sz = cgroup_v2_to_v1_map_sz;
			break;
		case CGROUP_V2:
			convert_tbl = cgroup_v1_to_v2_map;
			tbl_sz = cgroup_v1_to_v2_map_sz;
			break;
		default:
			ret = ECGFAIL;
			goto out;
		}

		for (j = 0; j < in_cgroup->controller[i]->index; j++) {
			for (k = 0; k < tbl_sz; k++) {
				if (strcmp(convert_tbl[k].in_setting,
				    in_cgroup->controller[i]->values[j]->name) == 0) {
					ret = convert_tbl[k].cgroup_convert(cgc,
						in_cgroup->controller[i]->values[j]->value,
						convert_tbl[k].out_setting,
						convert_tbl[k].in_dflt,
						convert_tbl[k].out_dflt);
				}
			}
		}
	}

out:
	return ret;
}

int cgroup_convert_int(struct cgroup_controller * const dst_cgc,
		       const char * const in_value,
		       const char * const out_setting,
		       void *in_dflt, void *out_dflt)
{
#define OUT_VALUE_STR_LEN 20

	long int in_dflt_int = (long int)in_dflt;
	long int out_dflt_int = (long int)out_dflt;
	char *out_value_str = NULL;
	long int out_value;
	int ret;

	if (!in_value)
		return ECGINVAL;

	if (strlen(in_value) > 0) {
		ret = cgroup_strtol(in_value, 10, &out_value);
		if (ret)
			goto out;

		/* now scale from the input range to the output range */
		out_value = out_value * out_dflt_int / in_dflt_int;

		out_value_str = calloc(sizeof(char), OUT_VALUE_STR_LEN);
		ret = snprintf(out_value_str, OUT_VALUE_STR_LEN, "%ld\n", out_value);
		if (ret == OUT_VALUE_STR_LEN) {
			/* we ran out of room in the string. throw an error */
			cgroup_err("Error: output value too large for string: %d\n",
				   out_value);
			ret = ECGFAIL;
			goto out;
		}
	}

	ret = cgroup_add_value_string(dst_cgc, out_setting, out_value_str);

out:
	if (out_value_str)
		free(out_value_str);

	return ret;
}
