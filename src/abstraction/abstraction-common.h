#ifndef __ABSTRACTION_COMMON
#define __ABSTRACTION_COMMON

#include "config.h"
#include <libcgroup.h>
#include "../libcgroup-internal.h"

struct cgroup_name_map {
	char *controller;
	enum cg_version_t cgx_version;
	char *cgx_name;
	char *cgx_value;
	char **disk_names;
	char **disk_values;
	int disk_len;
};

int cgroup_append_to_argv(int * const argc, char ***argv,
			  const char * const new_arg);

int cgroup_map_convert_name(struct cgroup_name_map * const map);
int cgroup_map_free_new(struct cgroup_name_map * const map);
int cgroup_map_free(struct cgroup_name_map * const map);
int cgroup_map_insert_disk_name(struct cgroup_name_map * const map,
			        const char * const disk_name,
			        const char * const disk_value);

/* ctrl-cpu.c functions */
int cgroup_cpu_v1_to_v2(struct cgroup_name_map * const map);
int cgroup_cpu_v2_to_v1(struct cgroup_name_map * const map);

#endif /* __ABSTRACTION_COMMON */
