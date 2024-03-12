# SPDX-License-Identifier: LGPL-2.1-only
#
# Libcgroup tree class
#
# Copyright (c) 2021-2024 Oracle and/or its affiliates.
# Author: Tom Hromatka <tom.hromatka@oracle.com>
#

# cython: language_level = 3str

# pip install treelib
# https://treelib.readthedocs.io/en/latest/
#
# Is the treelib print broken?  See this link:
# https://stackoverflow.com/questions/46345677/treelib-prints-garbage-instead-of-pseudographics-in-python3
#
from treelib import Node, Tree
import os

from libcgroup import Cgroup, CgroupFile, CgroupError, Version

class LibcgroupTree(object):
    def __get_mount(self):
        mount_list = Cgroup.mount_points(self.version)

        for mount in mount_list:
            if self.version == Version.CGROUP_V2:
                return mount
            else:
                if mount.endswith(controller) or mount.find('{},'.format(controller)):
                    return mount

        return None

    def __init__(self, name=None, version=Version.CGROUP_V2, controller='cpu', depth=None,
                 files=None):
        self.version = version
        self.controller = controller
        self.mount = self.__get_mount()

        if name:
            self.name = name
            self.start_path = os.path.join(self.mount, self.name)
        else:
            self.name = '/'
            self.start_path = self.mount

        self.rootcg = Cgroup(name=self.name, path=self.start_path, version=version)

        self.files = files
        self.depth = depth

    def walk(self):
        if self.depth is not None:
            max_slash_cnt = self.start_path.count('/') + self.depth + 1
        else:
            max_slash_cnt = os.pathconf('/', 'PC_NAME_MAX')

        for root, dirs, filenames in os.walk(self.start_path):
            if root == self.start_path:
                continue
            if root.count('/') >= max_slash_cnt:
                continue

            cg = Cgroup(name=root[len(self.mount) + 1:], path=root, version=self.version)
            self.walk_action(cg)

            if self.files:
                for filename in filenames:
                    cgf = CgroupFile(name=filename, path=os.path.join(root, filename))
                    self.walk_action(cgf)

    # This function can be overridden to do custom actions
    def walk_action(self, cg):
        parent_path = os.path.dirname(cg.path)
        parentcg = self.find_parent_by_path(parent_path)
        parentcg.children.append(cg)

    def find_cg_by_path(self, cg_path, cg):
        for child in cg.children:
            if child.path == cg_path:
                return child

            next = self.find_cg_by_path(cg_path, child)
            if next:
                return next

        return None

    def find_parent_by_path(self, parent_path):
        if parent_path == self.start_path:
            return self.rootcg

        parent = self.find_cg_by_path(parent_path, self.rootcg)
        if not parent:
            raise CgroupError('Failed to find cgroup with path {}'.format(parent_path))

        return parent

    # This function can be overridden to display custom data in the tree
    def node_label(self, cg):
        name = os.path.basename(cg.name)
        if not len(name):
            name = '/'

        return name

    def build(self):
        self.tree = Tree()

        self.tree.create_node(self.node_label(self.rootcg), self.rootcg.path)

        self.add_nodes(self.rootcg)

    def add_nodes(self, cg):
        for child in cg.children:
            self.tree.create_node(self.node_label(child), child.path, parent=cg.path)

            self.add_nodes(child)

    def show(self, ascii=False):
        if (ascii):
            self.tree.show(line_type='ascii')
        else:
            self.tree.show()

class LibcgroupPsiTree(LibcgroupTree):
    def __init__(self, name, controller='cpu', depth=None, psi_field='some-avg10'):
        super().__init__(name, version=Version.CGROUP_V2, files=False, depth=depth)

        self.rootcg.get_psi(controller)
        self.psi_field = psi_field

    def walk_action(self, cg):
        cg.get_psi(self.controller)
        super().walk_action(cg)

    def node_label(self, cg):
        name = os.path.basename(cg.name)
        if not len(name):
            name = '/'

        return '{}: {}'.format(name, cg.psi[self.psi_field])
