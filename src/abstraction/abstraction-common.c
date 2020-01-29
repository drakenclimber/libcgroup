#include <libcgroup.h>
#include <libcgroup-internal.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "abstraction-common.h"

int cgroup_strtol(const char * const in_str, int base,
		  long int * const out_value)
{
	char *endptr;
	int ret = 0;

	*out_value = strtol(in_str, &endptr, base);

	/* taken directly from strtol's man page */
	if ((errno == ERANGE &&
	     (*out_value == LONG_MAX || *out_value == LONG_MIN))
	    || (errno != 0 && *out_value == 0)) {
		cgroup_err("Error: Failed to convert %s from strtol: %s\n",
			   in_str);
		ret = ECGFAIL;
		goto out;
	}

	if (endptr == in_str) {
		cgroup_err("Error: No long value found in %s\n",
			   in_str);
		ret = ECGFAIL;
		goto out;
	}

out:
	return ret;
}

int cgroup_append_to_argv(int * const argc, char ***argv,
			  const char * const new_arg)
{
	(*argv) = reallocarray((*argv), sizeof(char *), (*argc) + 1);
	if (*argv == NULL)
		return ECGOTHER;

	(*argv)[*argc] = strdup(new_arg);
	if ((*argv)[*argc] == NULL)
		return ECGOTHER;

	(*argc)++;

	return 0;
}

static int get_controller_from_name(const char * const name,
				    char **controller)
{
	char *dot;

	*controller = strdup(name);
	if (*controller == NULL)
		return ECGOTHER;

	dot = strchr(*controller, '.');
	if (dot == NULL)
		return ECGINVAL;

	*dot = '\0';
	return 0;
}

static int get_value_from_name(const char * const name, char **value)
{
	char *copy = strdup(name);
	char *tok, *saveptr = NULL;
	int ret = 0;

	tok = strtok_r(copy, "=", &saveptr);
	if (strlen(tok) == strlen(copy)) {
		/* The name string doesn't contain an =, thus no value */
		*value = NULL;
		ret = 0;
		goto out;
	}

	*value = strdup(tok);
out:
	if (copy)
		free(copy);

	return ret;
}

int cgroup_map_convert(struct cgroup_name_map * const map)
{
	bool converted_cpu = false;
	bool converted_cpuset = false;
	enum cg_version_t ctrl_version;
	char *controller;
	int i, ret = 0;

	for (i = 0; i < map->cgx_len; i++) {
		ret = get_controller_from_name(map->cgx_names[i], &controller);
		if (ret)
			goto err;

		if (map->cgx_values[i] == NULL) {
			/* attempt to extract the value from the name */
			ret = get_value_from_name(map->cgx_names[i],
						  &map->cgx_values[i]);
			if (ret)
				goto err;
		}

		ret = cgroup_get_controller_version(controller,
						    &ctrl_version);
		if (ret)
			goto err;

		if (ctrl_version == map->cgx_version) {
			/* No conversion necessary.  Use the setting as is */
			ret = cgroup_map_insert_disk_name_value(
				map, map->cgx_names[i], map->cgx_values[i]);
			if (ret)
				goto err;

			/* no more processing is necessary on this name.  move
			 * on to the next one
			 */
			continue;
		}

		/* the versions don't match.  we need to convert */
		if (converted_cpu == false && strcmp(controller, "cpu") == 0) {
			converted_cpu = true;
			ret = cgroup_cpu_convert(map, ctrl_version);
			if (ret)
				goto err;
		}
		else if (converted_cpuset == false &&
			 strcmp(controller, "cpuset") == 0) {
			converted_cpuset = true;
			ret = cgroup_cpuset_convert(map, ctrl_version);
			if (ret)
				goto err;
		}
	}

err:
	return ret;
}

void cgroup_map_free_cgx(struct cgroup_name_map * const map)
{
	int i;

	for (i = 0; i < map->cgx_len; i++) {
		if (map->cgx_names[i] != NULL)
			free(map->cgx_names[i]);
		if (map->cgx_values[i] != NULL)
			free(map->cgx_values[i]);
	}

	map->cgx_len = 0;
}

void cgroup_map_free_disk(struct cgroup_name_map * const map)
{
	int i;

	for (i = 0; i < map->disk_len; i++) {
		if (map->disk_names[i] != NULL)
			free(map->disk_names[i]);
		if (map->disk_values[i] != NULL)
			free(map->disk_values[i]);
	}

	map->disk_len = 0;
}

void cgroup_map_free(struct cgroup_name_map * const map)
{
	cgroup_map_free_cgx(map);
	cgroup_map_free_disk(map);
}

int cgroup_map_insert_cgx_name_value(struct cgroup_name_map * const map,
				     const char * const cgx_name,
				     const char * const cgx_value)
{
	int ret;

	map->cgx_names = reallocarray(map->cgx_names, sizeof(char *),
				      map->cgx_len + 1);
	if (map->cgx_names == NULL) {
		ret = ECGOTHER;
		goto delete_cgx;
	}

	map->cgx_values = reallocarray(map->cgx_values, sizeof(char *),
				       map->cgx_len + 1);
	if (map->cgx_values == NULL) {
		ret = ECGOTHER;
		goto delete_cgx;
	}

	map->cgx_names[map->cgx_len] = NULL;
	if (cgx_name) {
		map->cgx_names[map->cgx_len] = strdup(cgx_name);
		if (map->cgx_names[map->cgx_len] == NULL) {
			ret = ECGOTHER;
			goto delete_cgx;
		}
	}

	map->cgx_values[map->cgx_len] = NULL;
	if (cgx_value) {
		map->cgx_values[map->cgx_len] = strdup(cgx_value);
		if (map->cgx_values[map->cgx_len] == NULL) {
			ret = ECGOTHER;
			goto delete_cgx;
		}
	}

	map->cgx_len++;
	return 0;

delete_cgx:
	/* one of the reallocs failed.  delete both arrays and reset the
	 * length to zero
	 */
	cgroup_map_free_cgx(map);
	return ret;
}

int cgroup_map_insert_disk_name_value(struct cgroup_name_map * const map,
				      const char * const disk_name,
				      const char * const disk_value)
{
	int ret;

	map->disk_names = reallocarray(map->disk_names, sizeof(char *),
				      map->disk_len + 1);
	if (map->disk_names == NULL) {
		ret = ECGOTHER;
		goto delete_disk;
	}

	map->disk_values = reallocarray(map->disk_values, sizeof(char *),
				        map->disk_len + 1);
	if (map->disk_values == NULL) {
		ret = ECGOTHER;
		goto delete_disk;
	}

	map->disk_names[map->disk_len] = NULL;
	if (disk_name) {
		map->disk_names[map->disk_len] = strdup(disk_name);
		if (map->disk_names[map->disk_len] == NULL) {
			ret = ECGOTHER;
			goto delete_disk;
		}
	}

	map->disk_values[map->disk_len] = NULL;
	if (disk_value) {
		map->disk_values[map->disk_len] = strdup(disk_value);
		if (map->disk_values[map->disk_len] == NULL) {
			ret = ECGOTHER;
			goto delete_disk;
		}
	}

	map->disk_len++;
	return 0;

delete_disk:
	/* one of the reallocs failed.  delete both arrays and reset the
	 * length to zero
	 */
	cgroup_map_free_disk(map);
	return ret;
}
