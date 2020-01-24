#ifndef __ABSTRACTION_COMMON
#define __ABSTRACTION_COMMON

#include "config.h"
#include <libcgroup.h>
#include "../libcgroup-internal.h"

struct cgroup_name_map {
	/* The cgx_* fields contain the name/value pairs of cgroup settings
	 * as requested by the user.  The cgx* tools should respond in the
	 * same cgroup format that the user made the request in.
	 */
	enum cg_version_t cgx_version;
	char **cgx_names;
	char **cgx_values;
	int cgx_len;

	/* The disk_* fields contain the name/value pairs of cgroup settings
	 * on the native filesystem.  These pairs may not have a one-to-one
	 * mapping with the user-facing parameters cgx_*.
	 */
	char **disk_names;
	char **disk_values;
	int disk_len;
};

int cgroup_append_to_argv(int * const argc, char ***argv,
			  const char * const new_arg);

int cgroup_map_convert(struct cgroup_name_map * const map);
void cgroup_map_free_cgx(struct cgroup_name_map * const map);
void cgroup_map_free_disk(struct cgroup_name_map * const map);
void cgroup_map_free(struct cgroup_name_map * const map);
int cgroup_map_insert_cgx_name_value(struct cgroup_name_map * const map,
				     const char * const cgx_name,
				     const char * const cgx_value);
int cgroup_map_insert_disk_name_value(struct cgroup_name_map * const map,
				      const char * const disk_name,
				      const char * const disk_value);

/* ctrl-cpu.c functions */
int cgroup_cpu_convert(struct cgroup_name_map * const map,
		       enum cg_version_t ctrl_version);

#endif /* __ABSTRACTION_COMMON */
