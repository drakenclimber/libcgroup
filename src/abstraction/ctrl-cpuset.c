#include <libcgroup.h>
#include <libcgroup-internal.h>

#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "abstraction-common.h"

static const char * const cpuset_cpu_exclusive = "cpuset.cpu_exclusive";
static const char * const cpuset_cpus_partition = "cpuset.cpus.partition";
static const char * const zero = "0";
static const char * const one = "1";
static const char * const member = "member";
static const char * const root = "root";

static int v1_exclusive_to_v2(struct cgroup_name_map * const map,
			      const char * const exclusive_value)
{
	const char *partition_str = NULL;
	long int exclusive;
	int ret = 0;

	if (exclusive_value) {
		ret = cgroup_strtol(exclusive_value, 10, &exclusive);
		if (ret)
			goto out;

		switch (exclusive) {
		case 0:
			partition_str = member;
			break;
		case 1:
			partition_str = root;
			break;
		default:
			ret = ECGINVAL;
			break;
		}
	}

	ret = cgroup_map_insert_disk_name_value(map, cpuset_cpus_partition,
						partition_str);

out:
	return ret;
}

static int v2_partition_to_v1(struct cgroup_name_map * const map,
			      const char * const partition_value)
{
	const char *exclusive_str = NULL;
	int ret = 0;

	if (partition_value) {
		if (strcmp(partition_value, member) == 0) {
			exclusive_str = zero;
		} else if (strcmp(partition_value, root) == 0) {
			exclusive_str = one;
		} else {
			cgroup_warn("Unsupported partition type: %s\n",
				    partition_value);
			ret = ECGROUPPARSEFAIL;
			goto out;
		}
	}

	ret = cgroup_map_insert_disk_name_value(map, cpuset_cpu_exclusive,
						exclusive_str);

out:
	return ret;
}
static int v1_to_v2(struct cgroup_name_map * const map)
{
	int i, ret = ECGFAIL;

	/* At this time, I believe cpuset cgroup v1 settings have a one-to-one
	 * mapping to cpuset cgroup v2 settings.  If/When this no longe
	 * becomes the case, we can make this function smarter.
	 */
	for (i = 0; i < map->cgx_len; i++) {
		if (strcmp(map->cgx_names[i], cpuset_cpu_exclusive) == 0)
			ret = v1_exclusive_to_v2(map, map->cgx_values[i]);
		if (ret)
			goto out;
	}

out:
	return ret;
}

static int v2_to_v1(struct cgroup_name_map * const map)
{
	int i, ret = ECGFAIL;

	/* At this time, I believe cpuset cgroup v1 settings have a one-to-one
	 * mapping to cpuset cgroup v2 settings.  If/When this no longe
	 * becomes the case, we can make this function smarter.
	 */
	for (i = 0; i < map->cgx_len; i++) {
		if (strcmp(map->cgx_names[i], cpuset_cpus_partition) == 0)
			ret = v2_partition_to_v1(map, map->cgx_values[i]);
		if (ret)
			goto out;
	}

out:
	return ret;
}

int cgroup_cpuset_convert(struct cgroup_name_map * const map,
			  enum cg_version_t ctrl_version)
{
	switch (ctrl_version) {
	case CGROUP_V1:
		return v2_to_v1(map);
	case CGROUP_V2:
		return v1_to_v2(map);
	default:
		return ECGFAIL;
	}
}
