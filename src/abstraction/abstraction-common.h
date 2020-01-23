#ifndef __ABSTRACTION_COMMON
#define __ABSTRACTION_COMMON

#include "config.h"
#include <libcgroup.h>
#include "../libcgroup-internal.h"

// TODO - delete this
#define MAX_NEW_SETTINGS	10

struct cgroup_name_map {
	enum cg_version_t in_version;
	char *prev_name;
	char *prev_value;
	char **new_names;
	char **new_values;
	int new_len;
};

int cgroup_append_to_argv(int * const argc, char ***argv,
			  const char * const new_arg);

int cgroup_map_convert_name(struct cgroup_name_map * const map);
int cgroup_map_delete_new(struct cgroup_name_map * const map);
int cgroup_map_insert_new_name(struct cgroup_name_map * const map,
			       const char * const new_name,
			       const char * const new_value);

int cgroup_convert_setting(enum cg_version_t in_version,
			   const char * const prev_setting,
			   char *new_settings[]);

/* ctrl-cpu.c functions */
int cpu_v1_to_v2(const char * const prev_setting, char *new_settings[]);
int cpu_v2_to_v1(const char * const prev_setting, char *new_settings[]);

#endif /* __ABSTRACTION_COMMON */
