#!/usr/bin/env python3
# SPDX-License-Identifier: LGPL-2.1-only
#
# Create a systemd scope
#
# Copyright (c) 2025 Oracle and/or its affiliates.
# Author: Tom Hromatka <tom.hromatka@oracle.com>
#

from cgroup import Cgroup as CgroupCli
from libcgroup import Cgroup, BusType
from cgroup import CgroupVersion
from run import RunError, Run
from systemd import Systemd
from process import Process
import ftests
import consts
import sys
import os

import time
from run import Run

SLICE = 'libcgtests.slice'
SCOPE = '094test.scope'
SLICE_AND_SCOPE='{}/{}'.format(SLICE, SCOPE)

# Which controller isn't all that important, but it is important that we
# have a cgroup v2 controller
CONTROLLER = 'cpu'

TABLE = [
    # setting, value, sdbus_type, cgroup setting, expected value
    [ 'CPUWeight', 1234, BusType.BUS_TYPE_UINT64, 'cpu.weight', '1234' ],
    [ 'CPUQuotaPerSecUSec', 10000, BusType.BUS_TYPE_UINT64, 'cpu.max', '1000 100000' ],
    [ 'CPUQuotaPeriodUSec', 500000, BusType.BUS_TYPE_UINT64, 'cpu.max', '5000 500000' ],
    [ 'MemoryMin', 40960, BusType.BUS_TYPE_UINT64, 'memory.min', '40960' ],
    [ 'MemoryLow', 409600, BusType.BUS_TYPE_UINT64, 'memory.low', '409600' ],
    [ 'MemoryHigh', 8192000, BusType.BUS_TYPE_UINT64, 'memory.high', '8192000' ],
    [ 'MemoryMax', 10240000, BusType.BUS_TYPE_UINT64, 'memory.max', '10240000' ],
    [ 'TasksMax', 12345, BusType.BUS_TYPE_UINT64, 'pids.max', '12345' ],
]

def prereqs(config):
    result = consts.TEST_PASSED
    cause = None

    if config.args.container:
        result = consts.TEST_SKIPPED
        cause = 'This test cannot be run within a container'
        return result, cause

    if CgroupVersion.get_version(CONTROLLER) != CgroupVersion.CGROUP_V2:
        result = consts.TEST_SKIPPED
        cause = 'This test requires cgroup v2'

    if not Systemd.is_systemd_enabled():
        result = consts.TEST_SKIPPED
        cause = 'Systemd support not compiled in'

    return result, cause


def setup(config):
    Cgroup.create_scope(scope_name=SCOPE, slice_name=SLICE, delegated=False)


def test_cpuset(config):
    result = consts.TEST_PASSED
    cause = None

    cpu_bitfield = Cgroup.cpuset_str_to_bitfield('0,1,3,4,5,7-11,12')
    Cgroup.set_systemd_property(SCOPE, 'AllowedCPUs', cpu_bitfield, BusType.BUS_TYPE_ARRAY,
                                array_type=BusType.BUS_TYPE_BYTE)

    cmd = list()
    cmd.append('cat')
    cmd.append('/run/systemd/transient/{}.d/50-AllowedCPUs.conf'.format(SCOPE))
    ret = Run.run(cmd)

    write_passed = False
    for line in ret.splitlines():
        if line.strip() == 'AllowedCPUs=0-1 3-5 7-12':
            write_passed = True

    if not write_passed:
        result = consts.TEST_FAILED
        cause = 'Failed to set the cpuset list: {}'.format(ret)

    return result, cause

def test(config):
    result = consts.TEST_PASSED
    cause = None

    Cgroup.set_systemd_property(SCOPE, 'CPUAccounting', True, BusType.BUS_TYPE_BOOLEAN)
    Cgroup.set_systemd_property(SCOPE, 'IOAccounting', True, BusType.BUS_TYPE_BOOLEAN)
    Cgroup.set_systemd_property(SCOPE, 'TasksAccounting', True, BusType.BUS_TYPE_BOOLEAN)

    for entry in TABLE:
        Cgroup.set_systemd_property(SCOPE, entry[0], entry[1], entry[2])

        out = CgroupCli.get(config, cgname=SLICE_AND_SCOPE, setting=entry[3],
                            values_only=True, print_headers=False)

        if out != entry[4]:
            result = consts.TEST_FAILED
            cause = (
                        'After setting {}={}, expected {}={}, but received '
                        '{}={}'
                        ''.format(entry[0], entry[1], entry[3], entry[4],
                                  entry[3], out)
                    )
            return result, cause

    result, cause = test_cpuset(config)

    return result, cause


def teardown(config, result):
    pid = CgroupCli.get(config, cgname=os.path.join(SLICE, SCOPE), setting='cgroup.procs',
                        print_headers=False, values_only=True)
    Process.kill(config, pid)

    if result != consts.TEST_PASSED:
        # Something went wrong.  Let's force the removal of the cgroups just to be safe.
        # Note that this should remove the cgroup, but it won't remove it from systemd's
        # internal caches, so the system may not return to its 'pristine' prior-to-this-test
        # state
        try:
            CgroupCli.delete(config, None, os.path.join(SLICE, SCOPE))
        except RunError:
            pass
    else:
        # There is no need to remove the scope.  systemd should automatically remove it
        # once there are no processes inside of it
        pass

    return consts.TEST_PASSED, None


def main(config):
    [result, cause] = prereqs(config)
    if result != consts.TEST_PASSED:
        return [result, cause]

    try:
        result = consts.TEST_FAILED
        setup(config)
        [result, cause] = test(config)
    finally:
        teardown(config, result)

    return [result, cause]


if __name__ == '__main__':
    config = ftests.parse_args()
    # this test was invoked directly.  run only it
    config.args.num = int(os.path.basename(__file__).split('-')[0])
    sys.exit(ftests.main(config))

# vim: set et ts=4 sw=4:
