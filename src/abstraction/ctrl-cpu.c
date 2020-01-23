#include <libcgroup.h>
#include <libcgroup-internal.h>

#include <string.h>

#include "abstraction-common.h"

static const char * const cpu_shares = "cpu.shares";
static const char * const cpu_weight = "cpu.weight";

static int v1_shares_to_v2(struct cgroup_name_map * const map)
{
	// TODO - convert map->prev_value to shares
	return cgroup_map_insert_new_name(map, cpu_weight, map->prev_value);
}

static int v2_weight_to_v1(struct cgroup_name_map * const map)
{
	// TODO - convert map->prev_value to weight
	return cgroup_map_insert_new_name(map, cpu_shares, map->prev_value);
}

int cgroup_cpu_v1_to_v2(struct cgroup_name_map * const map)
{
	int ret = ECGFAIL;

	if (strcmp(map->prev_name, cpu_shares) == 0)
		ret = v1_shares_to_v2(map);

	return ret;
}

int cgroup_cpu_v2_to_v1(struct cgroup_name_map * const map)
{
	int ret = ECGFAIL;

	if (strcmp(map->prev_name, cpu_weight) == 0)
		ret = v2_weight_to_v1(map);

	return ret;
}
