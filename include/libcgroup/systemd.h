/* SPDX-License-Identifier: LGPL-2.1-only */
#ifndef _LIBCGROUP_SYSTEMD_H
#define _LIBCGROUP_SYSTEMD_H

#ifndef _LIBCGROUP_H_INSIDE
#error "Only <libcgroup.h> should be included directly."
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef SYSTEMD

/**
 * Create a systemd scope
 *
 * @param scope_name Name of the scope, must end in .scope
 * @param slice_name Name of the slice, must end in .slice
 * @param delegated Instruct systemd that this cgroup is delegated and should not be managed
 * 	  by systemd
 */
int cgroup_create_scope(const char * const scope_name, const char * const slice_name,
			int delegated);

#endif /* SYSTEMD */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _LIBCGROUP_SYSTEMD_H */
