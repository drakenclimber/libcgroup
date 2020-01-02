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
	{"executable", required_argument, NULL, 'x'},
};

static void usage(const char *program_name)
{
	printf("Usage: %s [-1] [-2]", program_name);
	printf("  -1, --v1			Provided parameters are in "
		"v1 format\n");
	printf("  -2, --v2			Provided parameters are in "
		"v2 format\n");
}

int main(int argc, char *argv[])
{
	bool v1 = false, v2 = false;
	int result = 0, c;
	char *cgget = CGGET;

	if (argc < 2) {
		usage(argv[0]);
		/*
		 * don't return here so that we can return the usage from
		 * cgget as well
		 */
	}

	/*
	 * extract our options from argv[] but leave the remaining options
	 * intact for cgget to parse
	 */
	while ((c = getopt_long(argc, argv, ":12x:", long_options, NULL)) > 0) {
		switch (c) {
		case '1':
			v1 = true;
			break;
		case '2':
			v2 = true;
			break;
		case 'x':
			cgget = optarg;
			break;
		}
	}

	if (v1 && v2) {
		/* both v1 and v2 simultaneously doesn't make sense */
		usage(argv[0]);
		return 1;
	}

	result = execvp(cgget, argv);

	return result;
}
