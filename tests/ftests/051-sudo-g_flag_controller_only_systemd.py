#!/usr/bin/env python3
# SPDX-License-Identifier: LGPL-2.1-only
#
# Advanced cgget functionality test - '-b' '-g' <controller>
#
# Copyright (c) 2022 Oracle and/or its affiliates.
# Author: Tom Hromatka <tom.hromatka@oracle.com>
#

from cgroup import Cgroup, CgroupVersion
from run import Run, RunError
import consts
import ftests
import sys
import os

CONTROLLER = 'cpu'
SYSTEMD_CGNAME = 'cg_in_scope'
OTHER_CGNAME = 'cg_not_in_scope'

SLICE = 'libcgroup'
SCOPE = 'test051'

CONFIG_FILE = '''systemd {{
    slice = {};
    scope = {};
    setdefault = yes;
}}'''.format(SLICE, SCOPE)

CONFIG_FILE_NAME = os.path.join(os.getcwd(), '051cgconfig.conf')

def prereqs(config):
    result = consts.TEST_PASSED
    cause = None

    if config.args.container:
        result = consts.TEST_SKIPPED
        cause = 'This test cannot be run within a container'
        return result, cause

    return result, cause


def setup(config):
    f = open(CONFIG_FILE_NAME, 'w')
    f.write(CONFIG_FILE)
    f.close()

    Cgroup.configparser(config, load_file=CONFIG_FILE_NAME)

    Cgroup.create(config, CONTROLLER, SYSTEMD_CGNAME)
    Cgroup.create(config, CONTROLLER, OTHER_CGNAME, ignore_systemd=True)


def test(config):
    result = consts.TEST_PASSED
    cause = None

    out = Cgroup.get(config, controller=CONTROLLER, cgname=SYSTEMD_CGNAME)
    if len(out.splitlines()) < 10:
        # This cgget command gets all of the settings/values within the cgroup.  We don't care
        # about the exact data, but there should be at least 10 lines of settings/values
        result = consts.TEST_FAILED
        cause = 'cgget failed to read at least 10 lines from cgroup {}: {}'.format(
                SYSTEMD_CGNAME, out)
        return result, cause

    out = Cgroup.get(config, controller=CONTROLLER, cgname=OTHER_CGNAME, ignore_systemd=True)
    if len(out.splitlines()) < 10:
        result = consts.TEST_FAILED
        cause = 'cgget failed to read at least 10 lines from cgroup {}: {}'.format(
                SYSTEMD_CGNAME, out)
        return result, cause

    # This should fail because the wrong path should be built up
    out = Cgroup.get(config, controller=CONTROLLER, cgname=SYSTEMD_CGNAME,
            ignore_systemd=True, print_headers=False)
    if len(out) > 0:
        result = consts.TEST_FAILED
        cause = 'cgget erroneously read cgroup {} at the wrong path: {}'.format(
                SYSTEMD_CGNAME, out)
        return result, cause

    # This should fail because the wrong path should be built up
    out = Cgroup.get(config, controller=CONTROLLER, cgname=OTHER_CGNAME,
            ignore_systemd=False, print_headers=False)
    if len(out) > 0:
        result = consts.TEST_FAILED
        cause = 'cgget erroneously read cgroup {} at the wrong path: {}'.format(
                OTHER_CGNAME, out)
        return result, cause

    return result, cause


def teardown(config):
    Cgroup.delete(config, CONTROLLER, SYSTEMD_CGNAME)
    Cgroup.delete(config, CONTROLLER, OTHER_CGNAME, ignore_systemd=True)

    os.remove(CONFIG_FILE_NAME)

    try:
        Run.run(['systemctl', 'stop', '{}.scope'.format(SCOPE)], shell_bool=True)
    except RunError:
        pass


def main(config):
    [result, cause] = prereqs(config)
    if result != consts.TEST_PASSED:
        return [result, cause]

    try:
        result = consts.TEST_FAILED
        setup(config)
        [result, cause] = test(config)
    finally:
        teardown(config)

    return [result, cause]


if __name__ == '__main__':
    config = ftests.parse_args()
    # this test was invoked directly.  run only it
    config.args.num = int(os.path.basename(__file__).split('-')[0])
    sys.exit(ftests.main(config))

# vim: set et ts=6 sw=4:
