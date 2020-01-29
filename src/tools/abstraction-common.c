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
	enum cg_version_t in_version = map->in_version;
	enum cg_version_t out_version = map->out_version;
	char *controller;
	int i, ret = 0;

	for (i = 0; i < map->in_len; i++) {
		ret = get_controller_from_name(map->in_names[i], &controller);
		if (ret)
			goto err;

		if (map->in_values[i] == NULL) {
			/* attempt to extract the value from the name */
			ret = get_value_from_name(map->in_names[i],
						  &map->in_values[i]);
			if (ret)
				goto err;
		}

		ret = cgroup_get_controller_version(controller,
						    &ctrl_version);
		if (ret)
			goto err;

		if (map->in_version == CGROUP_DISK ||
		    map->in_version == CGROUP_UNK)
			in_version = ctrl_version;
		if (map->out_version == CGROUP_DISK ||
		    map->out_version == CGROUP_UNK)
			out_version = ctrl_version;

		if (in_version == out_version) {
			/* No conversion necessary.  Use the setting as is */
			ret = cgroup_map_insert_out_name_value(
				map, map->in_names[i], map->in_values[i]);
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
			ret = cgroup_cpu_convert(map, out_version);
			if (ret)
				goto err;
		}
		else if (converted_cpuset == false &&
			 strcmp(controller, "cpuset") == 0) {
			converted_cpuset = true;
			ret = cgroup_cpuset_convert(map, out_version);
			if (ret)
				goto err;
		}
	}

err:
	return ret;
}

void cgroup_map_free_in(struct cgroup_name_map * const map)
{
	int i;

	for (i = 0; i < map->in_len; i++) {
		if (map->in_names[i] != NULL)
			free(map->in_names[i]);
		if (map->in_values[i] != NULL)
			free(map->in_values[i]);
	}

	map->in_len = 0;
}

void cgroup_map_free_out(struct cgroup_name_map * const map)
{
	int i;

	for (i = 0; i < map->out_len; i++) {
		if (map->out_names[i] != NULL)
			free(map->out_names[i]);
		if (map->out_values[i] != NULL)
			free(map->out_values[i]);
	}

	map->out_len = 0;
}

void cgroup_map_free(struct cgroup_name_map * const map)
{
	cgroup_map_free_in(map);
	cgroup_map_free_out(map);
}

int cgroup_map_insert_in_name_value(struct cgroup_name_map * const map,
				    const char * const in_name,
				    const char * const in_value)
{
	int ret;

	map->in_names = reallocarray(map->in_names, sizeof(char *),
				      map->in_len + 1);
	if (map->in_names == NULL) {
		ret = ECGOTHER;
		goto delete_in;
	}

	map->in_values = reallocarray(map->in_values, sizeof(char *),
				       map->in_len + 1);
	if (map->in_values == NULL) {
		ret = ECGOTHER;
		goto delete_in;
	}

	map->in_names[map->in_len] = NULL;
	if (in_name) {
		map->in_names[map->in_len] = strdup(in_name);
		if (map->in_names[map->in_len] == NULL) {
			ret = ECGOTHER;
			goto delete_in;
		}
	}

	map->in_values[map->in_len] = NULL;
	if (in_value) {
		map->in_values[map->in_len] = strdup(in_value);
		if (map->in_values[map->in_len] == NULL) {
			ret = ECGOTHER;
			goto delete_in;
		}
	}

	map->in_len++;
	return 0;

delete_in:
	/* one of the reallocs failed.  delete both arrays and reset the
	 * length to zero
	 */
	cgroup_map_free_in(map);
	return ret;
}

int cgroup_map_insert_out_name_value(struct cgroup_name_map * const map,
				     const char * const out_name,
				     const char * const out_value)
{
	int ret;

	map->out_names = reallocarray(map->out_names, sizeof(char *),
				      map->out_len + 1);
	if (map->out_names == NULL) {
		ret = ECGOTHER;
		goto delete_out;
	}

	map->out_values = reallocarray(map->out_values, sizeof(char *),
				       map->out_len + 1);
	if (map->out_values == NULL) {
		ret = ECGOTHER;
		goto delete_out;
	}

	map->out_names[map->out_len] = NULL;
	if (out_name) {
		map->out_names[map->out_len] = strdup(out_name);
		if (map->out_names[map->out_len] == NULL) {
			ret = ECGOTHER;
			goto delete_out;
		}
	}

	map->out_values[map->out_len] = NULL;
	if (out_value) {
		map->out_values[map->out_len] = strdup(out_value);
		if (map->out_values[map->out_len] == NULL) {
			ret = ECGOTHER;
			goto delete_out;
		}
	}

	map->out_len++;
	return 0;

delete_out:
	/* one of the reallocs failed.  delete both arrays and reset the
	 * length to zero
	 */
	cgroup_map_free_out(map);
	return ret;
}
