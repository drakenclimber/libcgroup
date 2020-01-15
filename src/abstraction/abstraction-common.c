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
	fprintf(stdout, "ctrlr = %s\n", *controller);
	return 0;
}

static int convert_v1_to_v2(const char * const controller,
			    const char * const prev_setting,
			    char *new_settings[])
{
	int ret = 0;

	if (strcmp(controller, "cpu") == 0)
		ret = cpu_v1_to_v2(prev_setting, new_settings);
	else
		/* currently unsupported controller */
		ret = ECGINVAL;

	return ret;
}

static int convert_v2_to_v1(const char * const controller,
			    const char * const prev_setting,
			    char *new_settings[])
{
	int ret = 0;

	if (strcmp(controller, "cpu") == 0)
		ret = cpu_v2_to_v1(prev_setting, new_settings);
	else
		/* currently unsupported controller */
		ret = ECGINVAL;

	return ret;
}

int cgroup_convert_setting(enum cg_version_t in_version,
			   const char * const prev_setting,
			   char *new_settings[])
{
	enum cg_version_t ctrl_version;
	char *controller;
	int ret = 0;
	int new_settings_idx = 0;

	ret = get_controller_from_setting(prev_setting,	&controller);
	if (ret)
		goto err;

	ret = cgroup_get_controller_version(controller, &ctrl_version);
	if (ret)
		goto err;
	fprintf(stdout, "controller %s version = %d\n", controller, ctrl_version);

	if (ctrl_version == in_version) {
		/* No conversion necessary.  Use the setting as is */
		new_settings[new_settings_idx] = strdup(prev_setting);
		fprintf(stdout, "yo\n");
		if (new_settings[new_settings_idx] == NULL) {
			ret = ECGFAIL;
		}
		new_settings_idx++;
	} else {
		/* We need to convert from one version to another */
		switch (in_version) {
		case CGROUP_UNK:
			break;
		case CGROUP_V1:
			if (ctrl_version == CGROUP_V2) {
				ret = convert_v1_to_v2(controller,
						       prev_setting,
						       new_settings);
				if (ret)
					goto err;
			}
			break;
		case CGROUP_V2:
			if (ctrl_version == CGROUP_V1) {
				ret = convert_v2_to_v1(controller,
						       prev_setting,
						       new_settings);
				if (ret)
					goto err;
			}
			break;
		default:
			cgroup_err("Unsupported cgroup version: %d\n",
				   in_version);
			goto err;
		}
	}

err:
	if (controller)
		free(controller);

	fprintf(stdout, "yo2\n");
	return ret;
}
