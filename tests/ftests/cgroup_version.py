# SPDX-License-Identifier: LGPL-2.1-only
#
# Cgroup Version class for the libcgroup functional tests
#
# Copyright (c) 2019-2025 Oracle and/or its affiliates.
# Author: Tom Hromatka <tom.hromatka@oracle.com>
#

from enum import Enum
import os


class CgroupVersion(Enum):
    CGROUP_UNK = 0
    CGROUP_V1 = 1
    CGROUP_V2 = 2

    # given a controller name, get the cgroup version of the controller
    @staticmethod
    def get_version(controller):
        with open('/proc/mounts', 'r') as mntf:
            for line in mntf.readlines():
                mnt_path = line.split()[1]

                if line.split()[0] == 'cgroup':
                    for option in line.split()[3].split(','):
                        if option == controller:
                            return CgroupVersion.CGROUP_V1
                elif line.split()[0] == 'cgroup2':
                    ctrlf_path = os.path.join(mnt_path, 'cgroup.controllers')
                    with open(ctrlf_path, 'r') as ctrlf:
                        controllers = ctrlf.readline()
                        for ctrl in controllers.split():
                            if ctrl == controller:
                                return CgroupVersion.CGROUP_V2

        raise IndexError(
                            'Unknown version for controller {}'
                            ''.format(controller)
                        )


# vim: set et ts=4 sw=4:
