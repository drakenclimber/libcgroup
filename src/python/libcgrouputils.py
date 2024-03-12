# SPDX-License-Identifier: LGPL-2.1-only
#
# Utilities for understanding and evaluating cgroup usage on a machine
#
# Copyright (c) 2021-2024 Oracle and/or its affiliates.
# Author: Tom Hromatka <tom.hromatka@oracle.com>
#

# cython: language_level = 3str


import subprocess

def run(command, run_in_shell=False):
    if run_in_shell:
        if isinstance(command, str):
            # nothing to do.  command is already formatted as a string
            pass
        elif isinstance(command, list):
            command = ' '.join(command)
        else:
            raise ValueError('Unsupported command type')

    subproc = subprocess.Popen(command, shell=run_in_shell,
                               stdout=subprocess.PIPE,
                               stderr=subprocess.PIPE)
    out, err = subproc.communicate()
    ret = subproc.returncode

    out = out.strip().decode('UTF-8')
    err = err.strip().decode('UTF-8')

    if ret != 0 or len(err) > 0:
        raise RunError("Command '{}' failed".format(''.join(command)),
                       command, ret, out, err)

    return out

def humanize(value):
    if type(value) is not int:
        raise TypeError('Unsupported type {}'.format(type(value)))

    if value < 1024:
        return value
    elif value < 1024 ** 2:
        value = value / 1024
        return '{}K'.format(int(value))
    elif value < 1024 ** 3:
        value = value / (1024 ** 2)
        return '{}M'.format(int(value))
    elif value < 1024 ** 4:
        value = value / (1024 ** 3)
        return '{}G'.format(int(value))
    elif value < 1024 ** 5:
        value = value / (1024 ** 4)
        return '{}T'.format(int(value))
    else:
        return value

class RunError(Exception):
    def __init__(self, message, command, ret, stdout, stderr):
        super(RunError, self).__init__(message)

        self.command = command
        self.ret = ret
        self.stdout = stdout
        self.stderr = stderr

    def __str__(self):
        out_str = 'RunError:\n\tcommand = {}\n\tret = {}'.format(
                  self.command, self.ret)
        out_str += '\n\tstdout = {}\n\tstderr = {}'.format(self.stdout,
                                                           self.stderr)
        return out_str
