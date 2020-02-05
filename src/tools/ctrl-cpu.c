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

static int v1_shares_to_v2(struct cgroup_name_map * const map,
			   const char * const shares_value)
{
#define WEIGHT_STR_LEN 20

	long int weight;
	char *weight_str = NULL;
	int ret = 0;

	if (shares_value) {
		ret = cgroup_strtol(shares_value, 10, &weight);
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

	ret = cgroup_map_insert_out_name_value(map, cpu_weight, weight_str);

out:
	if (weight_str)
		free(weight_str);

	return ret;
}

static int v2_weight_to_v1(struct cgroup_name_map * const map,
			   const char * const weight_value)
{
#define SHARES_STR_LEN 20

	long int shares;
	char *shares_str = NULL;
	int ret = 0;

	if (weight_value) {
		ret = cgroup_strtol(weight_value, 10, &shares);
		if (ret)
			goto out;

		/* now scale from cpu.shares to cpu.weight */
		shares = shares * DEFAULT_SHARES_VALUE / DEFAULT_WEIGHT_VALUE;

		shares_str = calloc(sizeof(char), SHARES_STR_LEN);
		ret = snprintf(shares_str, SHARES_STR_LEN, "%ld\n", shares);
		if (ret == SHARES_STR_LEN) {
			/* we ran out of room in the string. throw an error */
			cgroup_err("Error: shares too large for string: %d\n",
				   shares);
			ret = ECGFAIL;
			goto out;
		}
	}

	ret = cgroup_map_insert_out_name_value(map, cpu_shares, shares_str);

out:
	if (shares_str)
		free(shares_str);

	return ret;
}

static int v1_to_v2(struct cgroup_name_map * const map)
{
	int i, ret = ECGFAIL;

	/* At this time, I believe cpu cgroup v1 settings have a one-to-one
	 * mapping to cpu cgroup v2 settings.  If/When this no longer becomes
	 * the case, we can make this function smarter.
	 */
	for (i = 0; i < map->in_len; i++) {
		if (strcmp(map->in_names[i], cpu_shares) == 0)
			ret = v1_shares_to_v2(map, map->in_values[i]);
		if (ret)
			goto out;
	}

out:
	return ret;
}

static int v2_to_v1(struct cgroup_name_map * const map)
{
	int i, ret = ECGFAIL;

	/* At this time, I believe cpu cgroup v1 settings have a one-to-one
	 * mapping to cpu cgroup v2 settings.  If/When this no longer becomes
	 * the case, we can make this function smarter.
	 */
	for (i = 0; i < map->in_len; i++) {
		if (strcmp(map->in_names[i], cpu_weight) == 0)
			ret = v2_weight_to_v1(map, map->in_values[i]);
		if (ret)
			goto out;
	}

out:
	return ret;
}

int cgroup_cpu_convert(struct cgroup_name_map * const map,
		       enum cg_version_t out_version)
{
	switch (out_version) {
	case CGROUP_V1:
		return v2_to_v1(map);
	case CGROUP_V2:
		return v1_to_v2(map);
	default:
		return ECGFAIL;
	}
}

static int v1_to_v2_2(struct cgroup_controller * const out_cgc,
		    const struct cgroup_controller * const in_cgc)
{
	int ret = 0;

	return ret;
}

static int v2_to_v1_2(struct cgroup_controller * const out_cgc,
		    const struct cgroup_controller * const in_cgc)
{
	int ret = 0;

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

	switch (out_cgc->version) {
	case CGROUP_V1:
		ret = v2_to_v1_2(out_cgc, in_cgc);
	case CGROUP_V2:
		return v1_to_v2_2(out_cgc, in_cgc);
	default:
		return ECGFAIL;
	}

out:
	return ret;
}
