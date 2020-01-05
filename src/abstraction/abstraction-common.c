#include <libcgroup.h>
#include <libcgroup-internal.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "abstraction-common.h"

static int get_controller_from_setting(const char * const setting,
				       char **controller)
{
	char *dot;

	*controller = strdup(setting);
	if (*controller == NULL)
		return ECGOTHER;

	dot = strchr(*controller, '.');
	if (dot == NULL)
		return ECGINVAL;

	*dot = '\0';
	return 0;
}

int cgroup_convert_setting(const char * const prev_setting,
			   char *new_settings[])
{
	enum cg_version_t version;
	char *controller;
	int ret = 0;

	ret = get_controller_from_setting(prev_setting,	&controller);
	if (ret)
		goto err;

	ret = cgroup_get_controller_version(controller, &version);
	fprintf(stdout, "controller %s version = %d\n", controller, version);

err:
	if (controller)
		free(controller);

	return ret;
}
