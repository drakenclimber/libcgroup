#ifndef __ABSTRACTION_COMMON
#define __ABSTRACTION_COMMON

#include "config.h"
#include <libcgroup.h>
#include "../libcgroup-internal.h"

struct cgroup_name_map {
	char *controller;
	enum cg_version_t prev_version;
	char *prev_name;
	char *prev_value;
	char **new_names;
	char **new_values;
	int new_len;
};

int cgroup_append_to_argv(int * const argc, char ***argv,
			  const char * const new_arg);

int cgroup_map_convert_name(struct cgroup_name_map * const map);
int cgroup_map_free_new(struct cgroup_name_map * const map);
int cgroup_map_free(struct cgroup_name_map * const map);
int cgroup_map_insert_new_name(struct cgroup_name_map * const map,
			       const char * const new_name,
			       const char * const new_value);

/* ctrl-cpu.c functions */
int cgroup_cpu_v1_to_v2(struct cgroup_name_map * const map);
int cgroup_cpu_v2_to_v1(struct cgroup_name_map * const map);

#endif /* __ABSTRACTION_COMMON */
