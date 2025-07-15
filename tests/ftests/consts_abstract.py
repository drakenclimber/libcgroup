# SPDX-License-Identifier: LGPL-2.1-only
#
# Constants for the libcgroup functional tests
#
# Copyright (c) 2019-2021 Oracle and/or its affiliates.
# Author: Tom Hromatka <tom.hromatka@oracle.com>
#

import abc

class ConstsAbstract(abc.ABC):
	@property
	@abc.abstractmethod
	def expected_cpu_out_009(self, controller='cpu'):
		raise NotImplementedError("Must be implemented in the distro class")

# vim: set et ts=4 sw=4:
