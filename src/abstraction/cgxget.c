#include <libcgroup.h>
#include <libcgroup-internal.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

#define CGGET "cgget"

static void usage(const char *program_name)
{
	printf("Usage: %s [-1] [-2]", program_name);
	printf("  -1, --v1			Provided parameters are in "
		"v1 format\n");
	printf("  -2, --v2			Provided parameters are in "
		"v2 format\n");
}

static void remove_arg(int * const argc, char *argv[], int arg_to_remove)
{
	int i;

	for (i = arg_to_remove; i < (*argc) - 1; i++) {
		argv[i] = argv[i + 1];
	}

	argv[(*argc) - 1] = NULL;
	(*argc)--;
}

int main(int argc, char *argv[])
{
	int result = 0, cur_arg = 0;
	bool v1 = false, v2 = false;

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
	while (cur_arg < argc) {
		if ((strncmp("-1", argv[cur_arg], strlen("-1")) == 0) ||
		    (strncmp("--v1", argv[cur_arg], strlen("--v1")) == 0)) {
			v1 = true;

			/* don't pass this argument into cgget */
			remove_arg(&argc, argv, cur_arg);
		}

		if ((strncmp("-2", argv[cur_arg], strlen("-2")) == 0) ||
		    (strncmp("--v2", argv[cur_arg], strlen("--v2")) == 0)) {
			v2 = true;

			/* don't pass this argument into cgget */
			remove_arg(&argc, argv, cur_arg);
		}

		cur_arg++;
	}

	if (v1 && v2) {
		/* both v1 and v2 simultaneously doesn't make sense */
		usage(argv[0]);
		return 1;
	}

	result = execvp(CGGET, argv);

	return result;
}
