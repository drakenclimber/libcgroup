/* SPDX-License-Identifier: LGPL-2.1-only */
/**
 * libcgroup googletest for cgroup_systemd_cpuset_str_to_byte_array()
 *
 * Copyright (c) 2025 Oracle and/or its affiliates.
 * Author: Tom Hromatka <tom.hromatka@oracle.com>
 */

#include <ftw.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "gtest/gtest.h"

#include "libcgroup-internal.h"

class CgroupSystemdCpusetStrToByteArray : public ::testing::Test {
	protected:
};

TEST_F(CgroupSystemdCpusetStrToByteArray, InvalidParameters)
{
	unsigned char *bytes;
	char str[] = "1,2";
	int ret, len;

	ret = cgroup_systemd_cpuset_str_to_byte_array(NULL, &bytes, &len);
	ASSERT_EQ(ret, ECGINVAL);

	ret = cgroup_systemd_cpuset_str_to_byte_array(str, NULL, &len);
	ASSERT_EQ(ret, ECGINVAL);

	ret = cgroup_systemd_cpuset_str_to_byte_array(str, &bytes, NULL);
	ASSERT_EQ(ret, ECGINVAL);
}

TEST_F(CgroupSystemdCpusetStrToByteArray, OneByte)
{
	unsigned char *bytes;
	char str[] = "0,1,6,7";
	int ret, len;

	ret = cgroup_systemd_cpuset_str_to_byte_array(str, &bytes, &len);
	ASSERT_EQ(ret, 0);
	ASSERT_EQ(len, 1);
	ASSERT_EQ(bytes[0], 0xC3);

	free(bytes);
}

TEST_F(CgroupSystemdCpusetStrToByteArray, OneByteHyphen)
{
	unsigned char *bytes;
	char str[] = "1,3-6";
	int ret, len;

	ret = cgroup_systemd_cpuset_str_to_byte_array(str, &bytes, &len);
	ASSERT_EQ(ret, 0);
	ASSERT_EQ(len, 1);
	ASSERT_EQ(bytes[0], 0x7A);

	free(bytes);
}

TEST_F(CgroupSystemdCpusetStrToByteArray, TwoBytes)
{
	unsigned char *bytes;
	char str[] = "1,3,5,7,8,10,12,14";
	int ret, len;

	ret = cgroup_systemd_cpuset_str_to_byte_array(str, &bytes, &len);
	ASSERT_EQ(ret, 0);
	ASSERT_EQ(len, 2);
	ASSERT_EQ(bytes[0], 0xAA);
	ASSERT_EQ(bytes[1], 0x55);

	free(bytes);
}

TEST_F(CgroupSystemdCpusetStrToByteArray, TwoBytesHyphens)
{
	unsigned char *bytes;
	char str[] = "0-4,7,9-15";
	int ret, len;

	ret = cgroup_systemd_cpuset_str_to_byte_array(str, &bytes, &len);
	ASSERT_EQ(ret, 0);
	ASSERT_EQ(len, 2);
	ASSERT_EQ(bytes[0], 0x9F);
	ASSERT_EQ(bytes[1], 0xFE);

	free(bytes);
}

TEST_F(CgroupSystemdCpusetStrToByteArray, ThreeBytes)
{
	unsigned char *bytes;
	char str[] = "1,5,7-12,16,20-22";
	int ret, len;

	ret = cgroup_systemd_cpuset_str_to_byte_array(str, &bytes, &len);
	ASSERT_EQ(ret, 0);
	ASSERT_EQ(len, 3);
	ASSERT_EQ(bytes[0], 0xA2);
	ASSERT_EQ(bytes[1], 0x1F);
	ASSERT_EQ(bytes[2], 0x71);

	free(bytes);
}

TEST_F(CgroupSystemdCpusetStrToByteArray, OutOfOrder)
{
	unsigned char *bytes;
	char str[] = "24,6,13-15,0,9,21";
	int ret, len;

	ret = cgroup_systemd_cpuset_str_to_byte_array(str, &bytes, &len);
	ASSERT_EQ(ret, 0);
	ASSERT_EQ(len, 4);
	ASSERT_EQ(bytes[0], 0x41);
	ASSERT_EQ(bytes[1], 0xE2);
	ASSERT_EQ(bytes[2], 0x20);
	ASSERT_EQ(bytes[3], 0x01);

	free(bytes);
}
