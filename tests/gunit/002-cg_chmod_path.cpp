/**
 * libcgroup googletest for cg_chmod_path()
 *
 * Copyright (c) 2019 Oracle and/or its affiliates.  All rights reserved.
 * Author: Tom Hromatka <tom.hromatka@oracle.com>
 */

/*
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of version 2.1 of the GNU Lesser General Public License as
 * published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, see <http://www.gnu.org/licenses>.
 */

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "gtest/gtest.h"

#include "libcgroup-internal.h"

class ChmodPathV1Test : public ::testing::Test {
	protected:

	const char *DIR1 = "/tmp/__dir1";
	const char *DIR2 = "/tmp/__dir2";
	const char *DIR3 = "/tmp/__dir3";
	const char *INV_DIR = "/tmp/__notvalid";
	mode_t prev_umask;

	void SetUp() override {
		int ret;

		// clear the existing umask
		prev_umask = umask(0);

		ret = mkdir(DIR1, S_IRWXU | S_IRWXG | S_IRWXO);
		ASSERT_EQ(ret, 0);

		ret = mkdir(DIR2, S_IRWXU | S_IRWXG | S_IRWXO);
		ASSERT_EQ(ret, 0);
	}

	void TearDown() override {
		(void)rmdir(DIR1);
		(void)rmdir(DIR2);
		(void)rmdir(DIR3);
		(void)umask(prev_umask);
	}
};

/**
 * Helper function to get mode bits
 *
 * Note that googletest doesn't support asserts in functions that
 * have a non-void return.
 */
void GetModeBits(const char *pathname, mode_t * const mode) {
	struct stat statbuf;
	int ret;

	ret = stat(pathname, &statbuf);
	ASSERT_EQ(ret, 0);

	*mode = statbuf.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO);

	return;
}

/**
 * Basic chmod directory test
 * @param ChmodPathV1Test googletest test case name
 * @param ChmodPathV1_SimpleChmod test name
 */
TEST_F(ChmodPathV1Test, ChmodPathV1_SimpleChmod)
{
	int ret;
	mode_t setmode = S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH,
	       getmode;

	ret = cg_chmod_path(DIR1, setmode, 0);
	ASSERT_EQ(ret, 0);

	GetModeBits(DIR1, &getmode);
	ASSERT_EQ(setmode, getmode);
}

/**
 * Chmod directory with umask test
 * @param ChmodPathV1Test googletest test case name
 * @param ChmodPathV1_UmaskChmod test name
 */
TEST_F(ChmodPathV1Test, ChmodPathV1_UmaskChmod)
{
	int ret;
	mode_t setmode = S_IRWXU | S_IRWXG | S_IRWXO,
	       getmode;

	ret = cg_chmod_path(DIR2, setmode, 1);
	ASSERT_EQ(ret, 0);

	GetModeBits(DIR1, &getmode);
	ASSERT_EQ(setmode, getmode);
}

/**
 * Chmod directory with a non-zero umask
 * @param ChmodPathV1Test googletest test case name
 * @param ChmodPathV1_NonZeroUmask test name
 */
TEST_F(ChmodPathV1Test, ChmodPathV1_NonZeroUmask)
{
	int ret;
	mode_t setmode = S_IRWXU | S_IRWXG | S_IRWXO,
	       expected_getmode, getmode,
	       new_umask = S_IWGRP | S_IWOTH, prev_umask;

	expected_getmode = setmode & ~(new_umask);

	prev_umask = umask(new_umask);

	ret = mkdir(DIR3, setmode);
	ASSERT_EQ(ret, 0);

	// The umask setting means the dir should be 0755
	GetModeBits(DIR3, &getmode);
	ASSERT_EQ(getmode, expected_getmode);

	// Change the dir's permissions to 0777
	ret = cg_chmod_path(DIR3, setmode, 1);
	ASSERT_EQ(ret, 0);

	GetModeBits(DIR3, &getmode);
	ASSERT_EQ(getmode, setmode);

	/* remove the directory in TearDown() to ensure it is always
	 * destroyed
	 */

	(void)umask(prev_umask);
}

/**
 * Chmod directory with umask, but stat fails
 * @param ChmodPathV1Test googletest test case name
 * @param ChmodPathV1_UmaskStatFails test name
 */
TEST_F(ChmodPathV1Test, ChmodPathV1_UmaskStatFails)
{
	int ret;
	mode_t setmode = S_IRWXU | S_IRWXG | S_IRWXO;

	ret = cg_chmod_path(INV_DIR, setmode, 1);
	ASSERT_EQ(ret, ECGOTHER);
}
