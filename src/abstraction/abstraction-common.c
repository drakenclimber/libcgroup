#include <libcgroup.h>
#include <libcgroup-internal.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "abstraction-common.h"

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
	fprintf(stdout, "ctrlr = %s\n", *controller);
	return 0;
}

static int get_value_from_name(const char * const name, char **value)
{
	char *copy = strdup(name);
	char *tok, *saveptr = NULL;
	int ret = 0;

	tok = strtok_r(copy, "=", &saveptr);
	if (tok == NULL) {
		/* The name string doesn't contain a value */
		*value = NULL;
		ret = 0;
		goto out;
	}

	*value = strdup(tok);
	fprintf(stdout, "%s: value = %s\n", __func__, *value);

out:
	if (copy)
		free(copy);

	return ret;
}

static int convert_v1_to_v2(struct cgroup_name_map * const map)
{
	int ret = 0;

	if (strcmp(map->controller, "cpu") == 0)
		ret = cgroup_cpu_v1_to_v2(map);
	else
		/* currently unsupported controller */
		ret = ECGINVAL;

	return ret;
}

static int convert_v2_to_v1(struct cgroup_name_map * const map)
{
	int ret = 0;

	if (strcmp(map->controller, "cpu") == 0)
		ret = cgroup_cpu_v2_to_v1(map);
	else
		/* currently unsupported controller */
		ret = ECGINVAL;

	return ret;
}

int cgroup_map_convert_name(struct cgroup_name_map * const map)
{
	enum cg_version_t ctrl_version;
	int ret = 0;

	ret = get_controller_from_name(map->prev_name, &map->controller);
	if (ret)
		goto err;

	ret = get_value_from_name(map->prev_name, &map->prev_value);
	if (ret)
		goto err;

	ret = cgroup_get_controller_version(map->controller, &ctrl_version);
	if (ret)
		goto err;
	fprintf(stdout, "controller %s version = %d\n", map->controller, ctrl_version);

	if (ctrl_version == map->prev_version) {
		/* No conversion necessary.  Use the setting as is */
		ret = cgroup_map_insert_new_name(map, map->prev_name,
						 map->prev_value);
		if (ret)
			goto err;
	} else {
		/* We need to convert from one version to another */
		switch (map->prev_version) {
		case CGROUP_UNK:
			break;
		case CGROUP_V1:
			ret = convert_v1_to_v2(map);
			if (ret)
				goto err;
			break;
		case CGROUP_V2:
			ret = convert_v2_to_v1(map);
			if (ret)
				goto err;
			break;
		default:
			cgroup_err("Unsupported cgroup version: %d\n",
				   map->prev_version);
			goto err;
		}
	}

err:
	return ret;
}

int cgroup_map_free_new(struct cgroup_name_map * const map)
{
	int i;

	for (i = 0; i < map->new_len; i++) {
		if (map->new_names[i] != NULL)
			free(map->new_names[i]);
		if (map->new_values[i] != NULL)
			free(map->new_values[i]);
	}

	map->new_len = 0;
	return 0;
}

int cgroup_map_free(struct cgroup_name_map * const map)
{
	int ret;

	if (map->prev_name != NULL)
		free(map->prev_name);
	if (map->prev_value != NULL)
		free(map->prev_value);

	ret = cgroup_map_free_new(map);

	return ret;
}

int cgroup_map_insert_new_name(struct cgroup_name_map * const map,
			       const char * const new_name,
			       const char * const new_value)
{
	int ret;

	map->new_names = reallocarray(map->new_names, sizeof(char *),
				      map->new_len + 1);
	if (map->new_names == NULL) {
		ret = ECGOTHER;
		goto delete_new;
	}

	map->new_values = reallocarray(map->new_values, sizeof(char *),
				       map->new_len + 1);
	if (map->new_values == NULL) {
		ret = ECGOTHER;
		goto delete_new;
	}

	map->new_names[map->new_len] = NULL;
	if (new_name) {
		map->new_names[map->new_len] = strdup(new_name);
		if (map->new_names[map->new_len] == NULL) {
			ret = ECGOTHER;
			goto delete_new;
		}
	}

	map->new_values[map->new_len] = NULL;
	if (new_value) {
		map->new_values[map->new_len] = strdup(new_value);
		if (map->new_values[map->new_len] == NULL) {
			ret = ECGOTHER;
			goto delete_new;
		}
	}

	map->new_len++;
	return 0;

delete_new:
	/* one of the reallocs failed.  delete both arrays and reset the
	 * length to zero
	 */
	cgroup_map_free_new(map);
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
