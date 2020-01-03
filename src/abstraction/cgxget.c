#include <libcgroup.h>
#include <libcgroup-internal.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

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
	printf("Usage: %s [-1] [-2]", program_name);
	printf("  -1, --v1			Provided parameters are in "
		"v1 format\n");
	printf("  -2, --v2			Provided parameters are in "
		"v2 format\n");
}

static int parse_abstract_opts(int argc, char *argv[],
			       bool * const v1, bool * const v2)
{
	int c;

	while ((c = getopt_long(argc, argv, "r:hnvg:a12", long_options,
				NULL)) > 0) {
		switch (c) {
		case '1':
			*v1 = true;
			break;
		case '2':
			*v2 = true;
			break;

		/* ignore all other options at this time */
		default:
			break;
		}
	}

	if ((*v1) && (*v2)) {
		/* both v1 and v2 simultaneously doesn't make sense */
		usage(argv[0]);
		return -1;
	}

	/* reset the getopt index back to the start */
	optind = 0;

	return 0;
}

int main(int argc, char *argv[])
{
	bool v1 = false, v2 = false;
	int result = 0, c;
	char *cgget = CGGET;
	int cgget_argc = 0;
	char *cgget_argv[100] = {0};
	char *tmp;

	if (argc < 2) {
		usage(argv[0]);
		/*
		 * don't return here so that we can return the usage from
		 * cgget as well
		 */
	}

	/*
	 * Parse the options for the abstraction layer
	 */
	result = parse_abstract_opts(argc, argv, &v1, &v2);
	if (result < 0)
		goto err;

	/*
	 * Rebuild the options list without our parameters in it
	 */
	while ((c = getopt_long(argc, argv, "r:hnvg:a12", long_options,
				NULL)) > 0) {
		switch (c) {
		case 'r':
		case 'g':
			tmp = malloc(3);
			tmp[0] = '-';
			tmp[1] = c;
			tmp[2] = '\0';
			cgget_argv[cgget_argc] = tmp;
			cgget_argc++;

			if (optarg) {
				cgget_argv[cgget_argc] = optarg;
				cgget_argc++;
			}
			break;

		case 'h':
		case 'n':
		case 'v':
		case 'a':
			tmp = malloc(3);
			tmp[0] = '-';
			tmp[1] = c;
			tmp[2] = '\0';
			cgget_argv[cgget_argc] = tmp;
			cgget_argc++;
			break;

		default:
			break;
		}
	}

	/* append any non-getopt parameters */
	while(argv[optind] != NULL) {
		cgget_argv[cgget_argc] = argv[optind];
		cgget_argc++;

		optind++;
	}

	result = execvp(cgget, cgget_argv);

err:
	return (result < 0) ? -result : result;
}
