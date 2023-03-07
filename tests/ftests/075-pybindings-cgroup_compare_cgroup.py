#!/usr/bin/env python3
# SPDX-License-Identifier: LGPL-2.1-only
#
# cgroup_compare_cgroup() test using the python bindings
#
# Copyright (c) 2023 Oracle and/or its affiliates.
# Author: Tom Hromatka <tom.hromatka@oracle.com>
#

from cgroup import Cgroup as CgroupCli, Mode
from libcgroup import Cgroup, Version
import consts
import ftests
import sys
import os

CGNAME = '075cgcompare/compare'
CONTROLLERS = ['cpu', 'memory', 'io', 'pids']

def prereqs(config):
    result = consts.TEST_PASSED
    cause = None

    if config.args.container:
        result = consts.TEST_SKIPPED
        cause = 'This test cannot be run within a container'
        return result, cause

    if Cgroup.cgroup_mode() != Mode.CGROUP_MODE_UNIFIED:
        result = consts.TEST_SKIPPED
        cause = 'This test requires the unified cgroup v2 hierarchy'
        return result, cause

    return result, cause


def setup(config):
    CgroupCli.create(config, CONTROLLERS, CGNAME)


def test(config):
    result = consts.TEST_PASSED
    cause = None

    #
    # Test 1 - Compare matching empty cgroup instances
    #
    cgempty1 = Cgroup(CGNAME, Version.CGROUP_V2)
    cgempty2 = Cgroup(CGNAME, Version.CGROUP_V2)

    if cgempty1 != cgempty2:
        result = consts.TEST_FAILED
        tmp_cause = 'Empty cgroups do not match'
        cause = '\n'.join(filter(None, [cause, tmp_cause]))

    #
    # Test 2 - Compare different empty cgroup instances
    #
    cgempty3 = Cgroup('foo', Version.CGROUP_V2)
    cgempty4 = Cgroup('bar', Version.CGROUP_V2)

    if cgempty3 == cgempty4:
        result = consts.TEST_FAILED
        tmp_cause = 'Empty cgroups erroneously match'
        cause = '\n'.join(filter(None, [cause, tmp_cause]))

    #
    # Test 3 - Compare different empty cgroup instances
    #
    cgempty5 = Cgroup('baz', Version.CGROUP_V1)
    cgempty6 = Cgroup('baz', Version.CGROUP_V2)

    if cgempty5 == cgempty6:
        result = consts.TEST_FAILED
        tmp_cause = 'Empty cgroups erroneously match'
        cause = '\n'.join(filter(None, [cause, tmp_cause]))

    #
    # Test 4 - Compare cgroups with only the controllers populated
    #
    cgctrl1 = Cgroup(CGNAME, Version.CGROUP_V2)
#    cgctrl1.add_all_controllers()
#
#    cgctrl2 = Cgroup(CGNAME, Version.CGROUP_V2)
#    for controller in CONTROLLERS:
#        cgctrl2.add_controller(controller)
#
#    if cgctrl1 != cgctrl2:
#        result = consts.TEST_FAILED
#        tmp_cause = 'Controller-only cgroups do not match'
#        cause = '\n'.join(filter(None, [cause, tmp_cause]))
    return result, cause


def teardown(config):
    CgroupCli.delete(config, CONTROLLERS, CGNAME, recursive=True)


def main(config):
    [result, cause] = prereqs(config)
    if result != consts.TEST_PASSED:
        return [result, cause]

    setup(config)
    [result, cause] = test(config)
    teardown(config)

    return [result, cause]


if __name__ == '__main__':
    config = ftests.parse_args()
    # this test was invoked directly.  run only it
    config.args.num = int(os.path.basename(__file__).split('-')[0])
    sys.exit(ftests.main(config))

# vim: set et ts=4 sw=4:
