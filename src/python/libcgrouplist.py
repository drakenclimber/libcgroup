# SPDX-License-Identifier: LGPL-2.1-only
#
# Libcgroup list class
#
# Copyright (c) 2021-2024 Oracle and/or its affiliates.
# Author: Tom Hromatka <tom.hromatka@oracle.com>
#

#
#!/usr/bin/env python3
#
# Libcgroup list class
#
# Copyright (c) 2021-2024 Oracle and/or its affiliates.
# Author: Tom Hromatka <tom.hromatka@oracle.com>
#

# pip install treelib
# https://treelib.readthedocs.io/en/latest/
from treelib import Node, Tree
import os

from libcgroup import Version

from libcgrouptree import LibcgroupTree
from libcgrouputils import LibcgroupPid

float_metrics = ['%usr', '%system', '%guest', '%wait', '%CPU', '%MEM', 'minflt/s', 'majflt/s']
int_metrics = ['Time', 'UID', 'PID', 'CPU', 'RSS', 'threads', 'fd-nr']
str_metrics = ['Command']

class LibcgroupList(LibcgroupTree):
    def __init__(self, name, version=Version.CGROUP_V2, controller='cpu', depth=None,
                 metric='%CPU', threshold=1.0, limit=None):
        super().__init__(name, version, controller, depth=depth, files=False)

        self.metric = metric
        self.threshold = threshold
        self.cgpid_list = list()
        self.limit = limit

    def walk_action(self, cg):
        cg.get_pids()

        for pid in cg.pids:
            cgpid = LibcgroupPid.create_from_pidstat(pid)

            try:
                cgpid.cgroup = cg.path[len(self.start_path):]

                if self.metric in float_metrics:
                   if float(cgpid.pidstats[self.metric]) >= self.threshold:
                        self.cgpid_list.append(cgpid)

                elif self.metric in int_metrics:
                    if int(cgpid.pidstats[self.metric]) >= self.threshold:
                        self.cgpid_list.append(cgpid)

                else:
                    self.cgpid_list.append(cgpid)

            except AttributeError:
                # The pid could have been deleted between when we read cgroup.procs
                # and when we ran pidstat.  Ignore it and move on
                pass

    def sort(self):
        if self.metric in float_metrics:
            self.cgpid_list = sorted(self.cgpid_list, reverse=True,
                                     key=lambda cgpid: float(cgpid.pidstats[self.metric]))

        elif self.metric in int_metrics:
            self.cgpid_list = sorted(self.cgpid_list, reverse=True,
                                     key=lambda cgpid: int(cgpid.pidstats[self.metric]))

        else:
            self.cgpid_list = sorted(self.cgpid_list, reverse=True,
                                     key=lambda cgpid: cgpid.pidstats[self.metric])

    def show(self, sort=True):
        if sort:
            self.sort()

        print('{0: >10} {1: >16}  {2: >8} {3: <50}'.format(
            'PID', 'COMMAND', self.metric, 'CGROUP'))

        for i, cgpid in enumerate(self.cgpid_list):
            if self.limit and i >= self.limit:
                break

            if self.metric in float_metrics:
                print('{0: >10} {1: >16} {2: 9.2f} {3: <50}'.format(cgpid.pid,
                      cgpid.pidstats['Command'], float(cgpid.pidstats[self.metric]),
                      cgpid.cgroup))
            elif self.metric in int_metrics:
                print('{0: >10} {1: >16}    {2: 7d} {3: <50}'.format(cgpid.pid,
                      cgpid.pidstats['Command'], int(cgpid.pidstats[self.metric]),
                      cgpid.cgroup))
            else:
                print('{0: >10} {1: >16}    {2: >6} {3: <50}'.format(cgpid.pid,
                      cgpid.pidstats['Command'], cgpid.pidstats[self.metric],
                      cgpid.cgroup))

class LibcgroupPsiList(LibcgroupTree):
    def __init__(self, name, controller='cpu', depth=None, psi_field='some-avg10',
                 threshold=None, limit=None):
        super().__init__(name, version=Version.CGROUP_V2, controller=controller,
                         depth=depth)

        self.controller = controller
        self.psi_field = psi_field
        self.cglist = list()
        self.threshold = threshold
        self.limit = limit

    def walk_action(self, cg):
        cg.get_psi(self.controller)

        if not self.threshold:
            self.cglist.append(cg)
        elif cg.psi[self.psi_field] >= self.threshold:
            self.cglist.append(cg)

    def sort(self):
        self.cglist = sorted(self.cglist, reverse=True,
                             key=lambda cg: cg.psi[self.psi_field])

    def _show_float(self):
        print('{0: >10} {1: >3} {2: <16}'.format(self.psi_field, 'PSI', 'CGROUP'))

        for i, cg in enumerate(self.cglist):
            if self.limit and i >= self.limit:
                break

            print('        {0: 6.2f} {1: <16}'.format(cg.psi[self.psi_field],
                                                      cg.path[len(self.start_path):]))

    def _show_int(self):
        print('{0: >10} {1: >3} {2: <16}'.format(self.psi_field, 'PSI', 'CGROUP'))

        for i, cg in enumerate(self.cglist):
            if self.limit and i >= self.limit:
                break

            print('   {0: 11d} {1: <16}'.format(cg.psi[self.psi_field],
                                                cg.path[len(self.start_path):]))

    def show(self, sort=True):
        if sort:
            self.sort()

        if 'total' in self.psi_field:
            self._show_int()
        else:
            self._show_float()

class LibcgroupRealtimeList(LibcgroupTree):
    def __init__(self, name, depth=None, limit=None):
        super().__init__(name, version=Version.CGROUP_V1, controller='cpu',
                         depth=depth)

        self.rootcg.get_realtime()
        self.cglist = list()
        self.limit = limit

    def walk_action(self, cg):
        cg.get_realtime()

        if cg.settings['cpu.rt_runtime_us']:
            self.cglist.append(cg)

    def sort(self):
        self.cglist = sorted(self.cglist, reverse=True,
                             key=lambda cg: cg.realtime_pct)

    def show(self, sort=True, verbose=True):
        total_pct = 0.0

        if sort:
            self.sort()

        print('{0: >7} {1: >7} {2: >7} {3: <20}'.format('RUNTIME', 'PERIOD', 'PERCENT', 'CGROUP'))

        for i, cg in enumerate(self.cglist):
            # Only add the realtime of the direct children of the root cgroup.
            # Grandchildren realtime allocations are accounted for in their
            # parents' realtime allocations.
            if cg.path[len(self.rootcg.path):].count('/') == 1:
                total_pct += cg.realtime_pct

            if self.limit and i >= self.limit:
                  continue

            print('{0: >7} {1: >7}  {2: >5.2f}% {3: <20}'.format(
                  cg.settings['cpu.rt_runtime_us'],
                  cg.settings['cpu.rt_period_us'],
                  cg.realtime_pct,
                  cg.path[len(self.start_path):]))

        if verbose:
            print('\n{0:,} / {1:,} microseconds ({2: >5.2f}%) of the CPU '
                  'cycles have been allocated to realtime in the root '
                  'cgroup.'.format(
                  self.rootcg.settings['cpu.rt_runtime_us'],
                  self.rootcg.settings['cpu.rt_period_us'],
                  self.rootcg.realtime_pct))

            if self.mount == self.start_path:
                alloc_path = 'the root cgroup'
                tmpcg = self.rootcg
            else:
                alloc_path = self.rootcg.path[len(self.mount):]
                tmpcg = self.rootcg

            percent_consumed = 100 * total_pct / tmpcg.realtime_pct
            print('\n{0:,} of the {1:,} realtime cycles ({2: >5.2f}%) for {3:} '
                  'have been assigned to children cgroups'.format(
                  int(percent_consumed * tmpcg.settings['cpu.rt_runtime_us'] / 100),
                  tmpcg.settings['cpu.rt_runtime_us'],
                  percent_consumed,
                  alloc_path))

            print('\n{0:,} (cpu.rt_runtime_us) / {1:,} (cpu.rt_period_us) '
                  'microseconds can still be assigned to a child of {2:}'.format(
                  max(int((tmpcg.realtime_pct - total_pct) * 10000), 0),
                  1000000,
                  alloc_path))

            print('\nNote that the remaining cpu.rt_runtime_us is estimated '
                  'and could be off by 1 or 2 in either direction.\n')
