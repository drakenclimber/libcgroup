#include <libcgroup.h>
#include <libcgroup-internal.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tools-common.h"

int main(int argc, char *argv[])
{
	int len;

	fprintf(stdout, "cgget argc = %d\n", argc);
	len = 0;
	while (len < argc) {
		fprintf(stdout, " %s", argv[len]);
		len++;
	}
	fprintf(stdout, "\n");

	return cgget_main(argc, argv, CGROUP_UNK);
}
