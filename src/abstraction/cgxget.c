#include <libcgroup.h>
#include <libcgroup-internal.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

#include "abstraction-common.h"
#include "../tools/tools-common.h"

#define CGGET "cgget"

static struct option const long_options[] =
{
	{"v1", no_argument, NULL, '1'},
	{"v2", no_argument, NULL, '2'},
	{"variable", required_argument, NULL, 'r'},
	{"help", no_argument, NULL, 'h'},
	{"all",  no_argument, NULL, 'a'},
	{"values-only", no_argument, NULL, 'v'},
	{NULL, 0, NULL, 0}
};

static void usage(const char *program_name)
{
	printf("Usage: %s [-1] [-2]\n", program_name);
	printf("  -1, --v1			Provided parameters are in "
		"v1 format\n");
	printf("  -2, --v2			Provided parameters are in "
		"v2 format\n");
	printf("  -a, --all			Print info about all relevant "\
		"controllers\n");
	printf("  -g <controllers>		Controller which info should "\
		"be displayed\n");
	printf("  -g <controllers>:<path>	Control group which info "\
		"should be displayed\n");
	printf("  -h, --help			Display this help\n");
	printf("  -n				Do not print headers\n");
	printf("  -r, --variable  <name>	Define parameter to display\n");
	printf("  -v, --values-only		Print only values, not "\
		"parameter names\n");
}

static int parse_abstract_opts(int argc, char *argv[],
			       enum cg_version_t * const version)
{
	int c;
	bool version_set = false;

	*version = CGROUP_UNK;

	while ((c = getopt_long(argc, argv, "r:hnvg:a12", long_options,
				NULL)) > 0) {
		switch (c) {
		case '1':
			if (version_set) {
				usage(argv[0]);
				exit(0);
			}
			version_set = true;
			*version = CGROUP_V1;
			break;
		case '2':
			if (version_set) {
				usage(argv[0]);
				exit(0);
			}
			version_set = true;
			*version = CGROUP_V2;
			break;
		case 'h':
			usage(argv[0]);
			exit(0);

		/* ignore all other options at this time */
		default:
			break;
		}
	}

	/* reset the getopt index back to the start */
	optind = 0;

	return 0;
}

static int process_r_flag(int * const argc, char ***argv,
			  const char * const in_name,
			  enum cg_version_t version)
{
	struct cgroup_name_map map = {0};
	int i, ret = 0;

	if (in_name == NULL) {
		usage(CGGET);
		return ECGFAIL;
	}

	map.prev_name = strdup(in_name);
	if (map.prev_name == NULL)
		return ECGOTHER;

	map.prev_version = version;
	map.prev_value = NULL;

	ret = cgroup_map_convert_name(&map);
	if (ret)
		goto out;

	for (i = 0; i < map.new_len; i++) {
		ret = cgroup_append_to_argv(argc, argv, "-r");
		if (ret)
			goto out;

		ret = cgroup_append_to_argv(argc, argv, map.new_names[i]);
		if (ret)
			goto out;
	}

out:
	if (map.prev_name != NULL)
		free(map.prev_name);
	if (map.prev_value != NULL)
		free(map.prev_value);

	return ret;
}

static int parse_cgget_opts(int argc, char *argv[],
			    int * const cgget_argc, char ***cgget_argv,
			    enum cg_version_t args_version)
{
	int c, ret;
	char *tmp;

	ret = cgroup_append_to_argv(cgget_argc, cgget_argv, CGGET);
	if (ret)
		goto err;

	/* Rebuild the options list without our parameters in it */
	while ((c = getopt_long(argc, argv, "r:hnvg:a12", long_options,
				NULL)) > 0) {
		switch (c) {
		case 'a':
		case 'g':
		case 'h':
		case 'n':
		case 'v':
			tmp = malloc(3);
			tmp[0] = '-';
			tmp[1] = c;
			tmp[2] = '\0';
			ret = cgroup_append_to_argv(cgget_argc, cgget_argv,
						    tmp);
			if (ret)
				goto err;

			if (optarg) {
				ret = cgroup_append_to_argv(cgget_argc,
							    cgget_argv,
							    optarg);
				if (ret)
					goto err;
			}
			break;

		case 'r':
			ret = process_r_flag(cgget_argc, cgget_argv, optarg,
					     args_version);
			if (ret)
				goto err;
			break;

		default:
			break;
		}
	}

	/* append any non-getopt parameters */
	while(argv[optind] != NULL) {
		ret = cgroup_append_to_argv(cgget_argc, cgget_argv,
					    argv[optind]);
		if (ret)
			goto err;

		optind++;
	}

err:
	return ret;
}

int main(int argc, char *argv[])
{
	enum cg_version_t version;
	int result = 0;
	int cgget_argc = 0;
	char **cgget_argv = NULL;

	if (argc < 2) {
		usage(argv[0]);
		goto err;
	}

	/* Parse the options for the abstraction layer */
	result = parse_abstract_opts(argc, argv, &version);
	if (result)
		goto err;

	result = cgroup_init();
	if (result) {
		fprintf(stderr, "%s: libcgroup initialization failed: %s\n",
			argv[0], cgroup_strerror(result));
		goto err;
	}

	result = parse_cgget_opts(argc, argv, &cgget_argc, &cgget_argv,
				  version);
	if (result)
		goto err;

	int i;
	fprintf(stdout, "cgget argc = %d\n", cgget_argc);
	fprintf(stdout, "cgget");
	for (i = 1; i < cgget_argc; i++)
		fprintf(stdout, " %s", cgget_argv[i]);
	fprintf(stdout, "\n");

	/* reset the getopt index back to the start */
	optind = 0;
	result = cgget_main(cgget_argc, cgget_argv, NULL, NULL, &i);
err:
	return (result < 0) ? -result : result;
}
