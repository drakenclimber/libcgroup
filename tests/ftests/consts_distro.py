# SPDX-License-Identifier: LGPL-2.1-only
#
# Constants for the libcgroup functional tests
#
# Copyright (c) 2019-2021 Oracle and/or its affiliates.
# Author: Tom Hromatka <tom.hromatka@oracle.com>
#

import consts_oracle
import os

consts = None

with open('/etc/os-release', 'r') as relfile:
    buf = relfile.read()
    if "Oracle Linux" in buf:
        consts = consts_oracle.ConstsOracle()
    else:
        raise NotImplementedError("TODO")


# vim: set et ts=4 sw=4:
