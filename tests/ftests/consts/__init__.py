# SPDX-License-Identifier: LGPL-2.1-only
#
# Common constants for the libcgroup functional tests
#
# Copyright (c) 2019-2025 Oracle and/or its affiliates.
#
# Author: Tom Hromatka <tom.hromatka@oracle.com>
# Author: Kamalesh Babulal <kamalesh.babulal@oracle.com>

from consts.consts_common import ConstsCommon
from consts.consts_oracle import ConstsOracle
from consts.consts_ubuntu import ConstsUbuntu
import os


class Consts(ConstsCommon):
    def __init__(self):
        self.distro_name = Consts.get_distro()
        if self.distro_name == 'ubuntu':
            self.distro = ConstsUbuntu()
        elif self.distro_name == 'oracle':
            self.distro = ConstsOracle()


    @staticmethod
    def get_distro():
        with open('/etc/os-release', 'r') as relfile:
            buf = relfile.read()
            if "Oracle Linux" in buf:
                return "oracle"
            elif "Ubuntu" in buf:
                return "ubuntu"
            else:
                raise NotImplementedError("Unsupported Distro")

# vim: set et ts=4 sw=4:
