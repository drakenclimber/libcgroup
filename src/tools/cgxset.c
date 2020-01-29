#include <libcgroup.h>
#include <libcgroup-internal.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

#include "abstraction-common.h"
#include "../tools/tools-common.h"

#define CGSET "cgset"

enum {
	COPY_FROM_OPTION = CHAR_MAX + 1
};

static struct option const long_options[] =
{
	{"v1", no_argument, NULL, '1'},
	{"v2", no_argument, NULL, '2'},
	{"rule", required_argument, NULL, 'r'},
	{"help", no_argument, NULL, 'h'},
	{"copy-from", required_argument, NULL, COPY_FROM_OPTION},
	{NULL, 0, NULL, 0}
};

static void usage(const char *program_name)
{
	printf("Usage: %s [-1] [-2] [-r <name=value>] <cgroup_path> ...\n"
		"   or: %s --copy-from <source_cgroup_path> "\
		"<cgroup_path> ...\n", program_name, program_name);
	printf("Set the parameters of given cgroup(s)\n");
	printf("  -1, --v1			Provided parameters are in "
		"v1 format\n");
	printf("  -2, --v2			Provided parameters are in "
		"v2 format\n");
	printf("  -a, --all			Print info about all relevant "\
		"controllers\n");
	printf("  -r, --variable <name>			Define parameter "\
		"to set\n");
	printf("  --copy-from <source_cgroup_path>	Control group whose "\
		"parameters will be copied\n");
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

static int process_r_flag(struct cgroup_name_map * const map,
			  const char * const in_name)
{
	int ret = 0;

	if (in_name == NULL) {
		usage(CGSET);
		return ECGFAIL;
	}

	/* We will extract the values field later */
	ret = cgroup_map_insert_in_name_value(map, in_name, NULL);
	if (ret)
		return ret;

	return ret;
}

static int parse_cgset_opts(int argc, char *argv[],
			    int * const cgset_argc, char ***cgset_argv,
			    enum cg_version_t args_version)
{
	struct cgroup_name_map map = {0};
	int c, i, ret;
	char *tmp;

	map.in_version = args_version;
	map.out_version = CGROUP_DISK;

	ret = cgroup_append_to_argv(cgset_argc, cgset_argv, CGSET);
	if (ret)
		goto err;

	/* Rebuild the options list without our parameters in it */
	while ((c = getopt_long(argc, argv, "r:hnvg:a12", long_options,
				NULL)) > 0) {
		switch (c) {
		case 'h':
			tmp = malloc(3);
			tmp[0] = '-';
			tmp[1] = c;
			tmp[2] = '\0';
			ret = cgroup_append_to_argv(cgset_argc, cgset_argv,
						    tmp);
			if (ret)
				goto err;

			if (optarg) {
				ret = cgroup_append_to_argv(cgset_argc,
							    cgset_argv,
							    optarg);
				if (ret)
					goto err;
			}
			break;

		case 'r':
			ret = process_r_flag(&map, optarg);
			if (ret)
				goto err;
			break;

		default:
			break;
		}
	}

	ret = cgroup_map_convert(&map);
	if (ret)
		goto err;

	for (i = 0; i < map.out_len; i++) {
		ret = cgroup_append_to_argv(cgset_argc, cgset_argv, "-r");
		if (ret)
			goto err;

		ret = cgroup_append_to_argv(cgset_argc, cgset_argv,
					    map.out_names[i]);
		if (ret)
			goto err;
	}

	/* append any non-getopt parameters */
	while(argv[optind] != NULL) {
		ret = cgroup_append_to_argv(cgset_argc, cgset_argv,
					    argv[optind]);
		if (ret)
			goto err;

		optind++;
	}

err:
	cgroup_map_free(&map);
	return ret;
}

int main(int argc, char *argv[])
{
	enum cg_version_t version;
	int result = 0;
	int cgset_argc = 0;
	char **cgset_argv = NULL;

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

	result = parse_cgset_opts(argc, argv, &cgset_argc, &cgset_argv,
				  version);
	if (result)
		goto err;

	int i;
	fprintf(stdout, "cgset argc = %d\n", cgset_argc);
	fprintf(stdout, "cgset");
	for (i = 1; i < cgset_argc; i++)
		fprintf(stdout, " %s", cgset_argv[i]);
	fprintf(stdout, "\n");

	/* reset the getopt index back to the start */
	optind = 0;
	//result = cgset_main(cgset_argc, cgset_argv, NULL, NULL, &i);
	result = ECGINVAL;
err:
	return (result < 0) ? -result : result;
}
