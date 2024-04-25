#!/usr/bin/env python3
# SPDX-License-Identifier: LGPL-2.1-only
#
# Display a list of cgroups and their realtime usage
#
# Copyright (c) 2021-2024 Oracle and/or its affiliates.
# Author: Tom Hromatka <tom.hromatka@oracle.com>
#

from libcgroup import LibcgroupRealtimeList
import argparse
import os

def parse_args():
    parser = argparse.ArgumentParser('Libcgroup PSI List')
    parser.add_argument('-C', '--cgroup', type=str, required=False, default=None,
                        help='Relative path to the cgroup of interest, e.g. machine.slice/foo.scope')
    parser.add_argument('-d', '--depth', type=int, required=False, default=None,
                        help='Depth to recurse into the cgroup path.  0 == only this cgroup, 1 == this cgroup and its children, ...')
    parser.add_argument('-l', '--limit', type=int, required=False, default=None,
                        help='Only display the first N cgroups. If not provided, all cgroups that match are displayed')

    args = parser.parse_args()

    return args

def main(args):
    cglist = LibcgroupRealtimeList(args.cgroup, depth=args.depth, limit=args.limit)

    cglist.walk()
    cglist.show()

if __name__ == '__main__':
    args = parse_args()
    main(args)
