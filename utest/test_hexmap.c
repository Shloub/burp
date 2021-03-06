#include <check.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <openssl/md5.h>
#include "test.h"
#include "../src/hexmap.h"
#include "../src/protocol2/blk.h"

START_TEST(test_md5sum_of_empty_string)
{
	MD5_CTX md5;
	uint8_t checksum[MD5_DIGEST_LENGTH];

	MD5_Init(&md5);
	MD5_Final(checksum, &md5);
	hexmap_init();
	fail_unless(!memcmp(md5sum_of_empty_string, &md5, MD5_DIGEST_LENGTH));
}
END_TEST

struct md5data
{
        const char *str;
	uint8_t bytes[MD5_DIGEST_LENGTH];
};

static struct md5data m[] = {
	{ "d41d8cd98f00b204e9800998ecf8427e",
		{ 0xD4, 0x1D, 0x8C, 0xD9, 0x8F, 0x00, 0xB2, 0x04,
		  0xE9, 0x80, 0x09, 0x98, 0xEC, 0xF8, 0x42, 0x7E } },
	{ "00000000000000000000000000000000",
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
	{ "ffffffffffffffffffffffffffffffff",
		{ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF } },
	{ "0123456789abcdef0123456789abcdef",
		{ 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
		  0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF } },
};

START_TEST(test_md5str_to_bytes)
{
	hexmap_init();
	FOREACH(m)
	{
		uint8_t bytes[MD5_DIGEST_LENGTH];
		md5str_to_bytes(m[i].str, bytes);
		fail_unless(!memcmp(bytes, m[i].bytes, MD5_DIGEST_LENGTH));
	}
}
END_TEST

START_TEST(test_bytes_to_md5str)
{
	hexmap_init();
	FOREACH(m)
	{
		const char *str;
		str=bytes_to_md5str(m[i].bytes);
		fail_unless(!strcmp(m[i].str, str));
	}
}
END_TEST

struct savepathdata
{
        const char *str;
	uint8_t bytes[SAVE_PATH_LEN];
};

static struct savepathdata ssavepath[] = {
	{ "0011/2233/4455", { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55 } },
	{ "0000/0000/0000", { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
	{ "FFFF/FFFF/FFFF", { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF } },
};
static struct savepathdata ssavepathsig[] = {
	{ "0000/0000/0000/0000",
			{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
	{ "0011/2233/4455/6677",
			{ 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77 } },
	{ "AA00/BB11/CC22/DD33",
			{ 0xAA, 0x00, 0xBB, 0x11, 0xCC, 0x22, 0xDD, 0x33 } },
	{ "FFFF/FFFF/FFFF/FFFF",
			{ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF } },
};

static void do_savepath_str_to_bytes(struct savepathdata *d, size_t s)
{
	FOREACH(d)
	{
		uint8_t bytes[SAVE_PATH_LEN];
		savepathstr_to_bytes(d[i].str, bytes);
		fail_unless(memcmp(bytes, d[i].bytes, SAVE_PATH_LEN));
	}
}

START_TEST(test_savepathstr_to_bytes)
{
	hexmap_init();
	do_savepath_str_to_bytes(ssavepath,
		sizeof(ssavepath)/sizeof(*ssavepath));
	do_savepath_str_to_bytes(ssavepathsig,
		sizeof(ssavepathsig)/sizeof(*ssavepathsig));
}
END_TEST

START_TEST(test_bytes_to_savepathstr)
{
	hexmap_init();
	FOREACH(ssavepath)
	{
		const char *str;
		str=bytes_to_savepathstr(ssavepath[i].bytes);
		fail_unless(!strcmp(ssavepath[i].str, str));
	}
}
END_TEST

START_TEST(test_bytes_to_savepathstr_with_sig)
{
	hexmap_init();
	FOREACH(ssavepathsig)
	{
		const char *str;
		str=bytes_to_savepathstr_with_sig(ssavepathsig[i].bytes);
		fail_unless(!strcmp(ssavepathsig[i].str, str));
	}
}
END_TEST

Suite *suite_hexmap(void)
{
	Suite *s;
	TCase *tc_core;

	s=suite_create("hexmap");

	tc_core=tcase_create("Core");

	tcase_add_test(tc_core, test_md5sum_of_empty_string);
	tcase_add_test(tc_core, test_md5str_to_bytes);
	tcase_add_test(tc_core, test_bytes_to_md5str);
	tcase_add_test(tc_core, test_savepathstr_to_bytes);
	tcase_add_test(tc_core, test_bytes_to_savepathstr);
	tcase_add_test(tc_core, test_bytes_to_savepathstr_with_sig);
	suite_add_tcase(s, tc_core);

	return s;
}
