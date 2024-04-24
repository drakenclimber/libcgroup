#!/usr/bin/env python3
# SPDX-License-Identifier: LGPL-2.1-only
#
# Display the realtime usage in a cgroup hierarchy
#
# Copyright (c) 2021-2024 Oracle and/or its affiliates.
# Author: Tom Hromatka <tom.hromatka@oracle.com>
#

from libcgrouptree import LibcgroupRealtimeTree
import argparse
import os

def parse_args():
    parser = argparse.ArgumentParser("Libcgroup Realtime Tree")
    parser.add_argument('-C', '--cgroup', type=str, required=False, default=None,
                        help='Relative path to the cgroup of interest, e.g. machine.slice/foo.scope')
    parser.add_argument('-d', '--depth', type=int, required=False, default=None,
                        help='Depth to recurse into the cgroup path.  0 == only this cgroup, 1 == this cgroup and its children, ...')

    args = parser.parse_args()

    return args

def main(args):
    cgtree = LibcgroupRealtimeTree(args.cgroup, depth=args.depth)

    cgtree.walk()
    cgtree.build()

    if not args.cgroup:
        args.cgroup = '/'

    print('Realtime allocation for {}'.format(args.cgroup))
    cgtree.show()

if __name__ == '__main__':
    args = parse_args()
    main(args)
