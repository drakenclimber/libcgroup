# SPDX-License-Identifier: LGPL-2.1-only
#
# Constants for the libcgroup functional tests (Oracle)
#
# Copyright (c) 2025 Oracle and/or its affiliates.
# Author: Kamalesh Babulal <kamalesh.babulal@oracle.com>
#

from cgroup import CgroupVersion
from consts_abstract import ConstsAbstract

class ConstsOracle(ConstsAbstract):
    def expected_cpu_out_009(self, controller='cpu'):
        version = CgroupVersion.get_version(controller)

        if version == CgroupVersion.CGROUP_V1:
            return [
                # OL8 4.18.0-553.58.1
                """cpu.cfs_period_us: 100000
                cpu.stat: nr_periods 0
                        nr_throttled 0
                        throttled_time 0
                cpu.shares: 1024
                cpu.cfs_quota_us: -1
                cpu.rt_runtime_us: 0
                cpu.rt_period_us: 1000000""",
                # OL8 5.15.0-309.180.4
                """cpu.cfs_burst_us: 0
                cpu.cfs_period_us: 100000
                cpu.stat: nr_periods 0
                        nr_throttled 0
                        throttled_time 0
                cpu.shares: 1024
                cpu.idle: 0
                cpu.cfs_quota_us: -1"""
            ]
        else:
            return [
                # OL8 4.18.0-553.58.1
                """cpu.weight: 100
                cpu.stat: usage_usec 0
                        user_usec 0
                        system_usec 0
                        nr_periods 0
                        nr_throttled 0
                        throttled_usec 0
                cpu.weight.nice: 0
                cpu.max: max 100000""",
                # OL8 5.15.0-309.180.4
                """cpu.weight: 100
                cpu.stat: usage_usec 0
                        user_usec 0
                        system_usec 0
                        nr_periods 0
                        nr_throttled 0
                        throttled_usec 0
                cpu.weight.nice: 0
                cpu.pressure:
                cpu.idle: 0
                cpu.max.burst: 0
                cpu.max: max 100000"""
            ]

# vim: set et ts=4 sw=4:
