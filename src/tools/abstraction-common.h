#ifndef __ABSTRACTION_COMMON
#define __ABSTRACTION_COMMON

#include "config.h"
#include <libcgroup.h>
#include "libcgroup-internal.h"

struct cgroup_name_map {
	enum cg_version_t in_version;
	char **in_names;
	char **in_values;
	int in_len;

	enum cg_version_t out_version;
	char **out_names;
	char **out_values;
	int out_len;
};

int cgroup_strtol(const char * const in_str, int base,
		  long int * const out_value);
int cgroup_append_to_argv(int * const argc, char ***argv,
			  const char * const new_arg);

int cgroup_map_convert(struct cgroup_name_map * const map);
void cgroup_map_free_in(struct cgroup_name_map * const map);
void cgroup_map_free_out(struct cgroup_name_map * const map);
void cgroup_map_free(struct cgroup_name_map * const map);
int cgroup_map_insert_in_name_value(struct cgroup_name_map * const map,
				    const char * const in_name,
				    const char * const in_value);
int cgroup_map_insert_out_name_value(struct cgroup_name_map * const map,
				     const char * const out_name,
				     const char * const out_value);

/* ctrl-cpu.c functions */
int cgroup_cpu_convert(struct cgroup_name_map * const map,
		       enum cg_version_t out_version);

/* ctrl-cpuset.c functions */
int cgroup_cpuset_convert(struct cgroup_name_map * const map,
			  enum cg_version_t out_version);

#endif /* __ABSTRACTION_COMMON */
