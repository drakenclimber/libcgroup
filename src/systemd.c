// SPDX-License-Identifier: LGPL-2.1-only
/**
 * Libcgroup systemd interfaces
 *
 * Copyright (c) 2022 Oracle and/or its affiliates.
 * Author: Tom Hromatka <tom.hromatka@oracle.com>
 */
#ifdef SYSTEMD

int cgroup_create_scope(const char * const scope_name, const char * const slice_name,
			int delegated)
{
	return 0;
}

#endif /* SYSTEMD */
