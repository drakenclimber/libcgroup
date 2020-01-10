#ifndef __ABSTRACTION_COMMON
#define __ABSTRACTION_COMMON

#include "config.h"
#include <libcgroup.h>
#include "../libcgroup-internal.h"

#define MAX_NEW_SETTINGS	10

int cgroup_convert_setting(enum cg_version_t in_version,
			   const char * const prev_setting,
			   char *new_settings[]);

#endif /* __ABSTRACTION_COMMON */
