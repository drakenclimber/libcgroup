#include <libcgroup.h>
#include <libcgroup-internal.h>

#include <string.h>

static const char * const cpu_shares = "cpu.shares";

static int v2_weight_to_v1(const char * const prev_setting,
			   char *new_settings[])
{
	new_settings[0] = strdup(cpu_shares);
	if (new_settings[0] == NULL)
		return ECGINVAL;

	return 0;
}

int cpu_v1_to_v2(const char * const prev_setting, char *new_settings[])
{
	return ECGFAIL;
}

int cpu_v2_to_v1(const char * const prev_setting, char *new_settings[])
{
	int ret = ECGFAIL;

	if (strcmp(prev_setting, "cpu.weight") == 0)
		ret = v2_weight_to_v1(prev_setting, new_settings);

	return ret;
}
