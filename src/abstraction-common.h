/**
 * Libcgroup abstraction layer prototypes and structs
 *
 * Copyright (c) 2021 Oracle and/or its affiliates.
 * Author: Tom Hromatka <tom.hromatka@oracle.com>
 */

/*
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of version 2.1 of the GNU Lesser General Public License as
 * published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, see <http://www.gnu.org/licenses>.
 */
#ifndef __ABSTRACTION_COMMON
#define __ABSTRACTION_COMMON

__BEGIN_DECLS

#include "config.h"
#include <libcgroup.h>
#include "libcgroup-internal.h"

/**
 * Convert a string to a long
 *
 * @param in_str String to be converted
 * @param base Integer base
 * @param out_value Pointer to hold the output long
 *
 * @return 0 on success, ECGFAIL if the conversion to long failed
 */
int cgroup_strtol(const char * const in_str, int base,
		  long int * const out_value);

/**
 * Convert from one cgroup version to another version
 *
 * @param out_cgroup Destination cgroup
 * @param out_version Destination cgroup version
 * @param in_cgroup Source cgroup
 * @param in_version Source cgroup version, only used if set to v1 or v2
 *
 * @return 0 on success
 *         ECGFAIL conversion failed
 *         ECGCONTROLLERNOTEQUAL incorrect controller version provided
 */
int cgroup_convert_cgroup(struct cgroup * const out_cgroup,
			  enum cg_version_t out_version,
			  const struct cgroup * const in_cgroup,
			  enum cg_version_t in_version);

/**
 * Convert from one cpu controller version to another version
 *
 * @param out_cgc Destination controller
 * @param in_cgc Source controller
 *
 * @return 0 on success
 *         ECGFAIL conversion failed
 *         ECGCONTROLLERNOTEQUAL incorrect controller version provided
 */
int cgroup_convert_cpu(struct cgroup_controller * const out_cgc,
		       const struct cgroup_controller * const in_cgc);

/**
 * Functions that are defined as STATIC can be placed within the UNIT_TEST
 * ifdef.  This will allow them to be included in the unit tests while
 * remaining static in a normal libcgroup library build.
 */
#ifdef UNIT_TEST
int v1_shares_to_v2(struct cgroup_controller * const dst_cgc,
		    const char * const shares_val);
#endif /* UNIT_TEST */

__END_DECLS

#endif /* __ABSTRACTION_COMMON */
