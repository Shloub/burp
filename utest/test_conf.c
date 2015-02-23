#include <check.h>
#include <stdio.h>
#include <stdlib.h>
#include "../src/alloc.h"
#include "../src/conf.h"

void logp(const char *fmt, ...) { }
void log_oom_w(const char *func, const char *orig_func) { }
// Stuff pulled in from cntr.c:
// FIX THIS: Most of it can be deleted if cntr_stats_to_file did not have all
// the async stuff in it.
void logc(const char *fmt, ...) { }
const char *getdatestr(time_t t) { return ""; }
const char *time_taken(time_t d) { return ""; }
struct async *async_alloc(void) { struct async *a=NULL; return a; }
struct asfd *asfd_alloc(void) { struct asfd *v=NULL; return v; }
char *prepend_s(const char *prep, const char *fname) { return (char *)""; }
void close_fd(int *fd) { }
void async_free(struct async **as) { }
void asfd_free(struct asfd **asfd) { }
int json_cntr_to_file(struct asfd *asfd, struct cntr *cntr) { return 0; }
// Stuff pulled in from strlist.c:
#include "../src/regexp.h"
int pathcmp(const char *a, const char *b) { return 0; }
int compile_regex(regex_t **regex, const char *str) { return 0; }

static void check_default(struct conf **c, enum conf_opt o)
{
	switch(o)
	{
		case OPT_BURP_MODE:
			ck_assert_int_eq(get_e_burp_mode(c[o]),
				BURP_MODE_UNSET);
			break;
		case OPT_LOCKFILE:
		case OPT_ADDRESS:
		case OPT_PORT:
		case OPT_STATUS_ADDRESS:
		case OPT_STATUS_PORT:
        	case OPT_SSL_CERT_CA:
		case OPT_SSL_CERT:
		case OPT_SSL_KEY:
		case OPT_SSL_KEY_PASSWORD:
		case OPT_SSL_PEER_CN:
		case OPT_SSL_CIPHERS:
		case OPT_SSL_DHFILE:
		case OPT_CA_CONF:
		case OPT_CA_NAME:
		case OPT_CA_SERVER_NAME:
		case OPT_CA_BURP_CA:
		case OPT_CA_CSR_DIR:
		case OPT_PEER_VERSION:
		case OPT_CLIENT_LOCKDIR:
		case OPT_MONITOR_LOGFILE:
		case OPT_CNAME:
		case OPT_PASSWORD:
		case OPT_PASSWD:
		case OPT_SERVER:
		case OPT_ENCRYPTION_PASSWORD:
		case OPT_AUTOUPGRADE_OS:
		case OPT_AUTOUPGRADE_DIR:
		case OPT_BACKUP:
		case OPT_BACKUP2:
		case OPT_RESTOREPREFIX:
		case OPT_RESTORE_SPOOL:
		case OPT_BROWSEFILE:
		case OPT_BROWSEDIR:
		case OPT_B_SCRIPT_PRE:
		case OPT_B_SCRIPT_POST:
		case OPT_R_SCRIPT_PRE:
		case OPT_R_SCRIPT_POST:
		case OPT_B_SCRIPT:
		case OPT_R_SCRIPT:
		case OPT_RESTORE_PATH:
		case OPT_ORIG_CLIENT:
		case OPT_CNTR:
		case OPT_CONFFILE:
		case OPT_USER:
		case OPT_GROUP:
		case OPT_DIRECTORY:
		case OPT_TIMESTAMP_FORMAT:
		case OPT_CLIENTCONFDIR:
		case OPT_S_SCRIPT_PRE:
		case OPT_S_SCRIPT_POST:
		case OPT_MANUAL_DELETE:
		case OPT_S_SCRIPT:
		case OPT_TIMER_SCRIPT:
		case OPT_N_SUCCESS_SCRIPT:
		case OPT_N_FAILURE_SCRIPT:
		case OPT_DEDUP_GROUP:
		case OPT_VSS_DRIVES:
		case OPT_REGEX:
		case OPT_RESTORE_CLIENT:
			ck_assert_int_eq(get_string(c[o])==NULL, 1);
			break;
		case OPT_RATELIMIT:
			ck_assert_int_eq(get_float(c[o]), 0);
			break;
		case OPT_CLIENT_IS_WINDOWS:
		case OPT_RANDOMISE:
		case OPT_B_SCRIPT_POST_RUN_ON_FAIL:
		case OPT_R_SCRIPT_POST_RUN_ON_FAIL:
		case OPT_SEND_CLIENT_CNTR:
		case OPT_BREAKPOINT:
		case OPT_SYSLOG:
		case OPT_PROGRESS_COUNTER:
		case OPT_MONITOR_BROWSE_CACHE:
		case OPT_S_SCRIPT_PRE_NOTIFY:
		case OPT_S_SCRIPT_POST_RUN_ON_FAIL:
		case OPT_S_SCRIPT_POST_NOTIFY:
		case OPT_S_SCRIPT_NOTIFY:
		case OPT_HARDLINKED_ARCHIVE:
        	case OPT_N_SUCCESS_WARNINGS_ONLY:
        	case OPT_N_SUCCESS_CHANGES_ONLY:
		case OPT_CROSS_ALL_FILESYSTEMS:
		case OPT_READ_ALL_FIFOS:
		case OPT_READ_ALL_BLOCKDEVS:
		case OPT_SPLIT_VSS:
		case OPT_STRIP_VSS:
		case OPT_ATIME:
		case OPT_OVERWRITE:
		case OPT_STRIP:
			ck_assert_int_eq(get_int(c[o]), 0);
			break;
		case OPT_DAEMON:
		case OPT_STDOUT:
		case OPT_FORK:
		case OPT_DIRECTORY_TREE:
		case OPT_PASSWORD_CHECK:
		case OPT_LIBRSYNC:
		case OPT_VERSION_WARN:
		case OPT_PATH_LENGTH_WARN:
		case OPT_CLIENT_CAN_DELETE:
		case OPT_CLIENT_CAN_DIFF:
		case OPT_CLIENT_CAN_FORCE_BACKUP:
		case OPT_CLIENT_CAN_LIST:
		case OPT_CLIENT_CAN_RESTORE:
		case OPT_CLIENT_CAN_VERIFY:
		case OPT_SERVER_CAN_RESTORE:
			ck_assert_int_eq(get_int(c[o]), 1);
			break;
		case OPT_NETWORK_TIMEOUT:
			ck_assert_int_eq(get_int(c[o]), 60*60*2);
			break;
		case OPT_SSL_COMPRESSION:
		case OPT_MAX_CHILDREN:
		case OPT_MAX_STATUS_CHILDREN:
			ck_assert_int_eq(get_int(c[o]), 5);
			break;
        	case OPT_COMPRESSION:
			ck_assert_int_eq(get_int(c[o]), 9);
			break;
		case OPT_MAX_STORAGE_SUBDIRS:
			ck_assert_int_eq(get_int(c[o]), 30000);
			break;
		case OPT_MAX_HARDLINKS:
			ck_assert_int_eq(get_int(c[o]), 10000);
			break;
		case OPT_UMASK:
			ck_assert_int_eq(get_mode_t(c[o]), 0022);
			break;
		case OPT_STARTDIR:
		case OPT_B_SCRIPT_PRE_ARG:
		case OPT_B_SCRIPT_POST_ARG:
		case OPT_R_SCRIPT_PRE_ARG:
		case OPT_R_SCRIPT_POST_ARG:
		case OPT_B_SCRIPT_ARG:
		case OPT_R_SCRIPT_ARG:
		case OPT_S_SCRIPT_PRE_ARG:
		case OPT_S_SCRIPT_POST_ARG:
		case OPT_S_SCRIPT_ARG:
		case OPT_TIMER_ARG:
		case OPT_N_SUCCESS_ARG:
		case OPT_N_FAILURE_ARG:
		case OPT_RESTORE_CLIENTS:
		case OPT_KEEP:
		case OPT_INCLUDE:
		case OPT_EXCLUDE:
		case OPT_FSCHGDIR:
		case OPT_NOBACKUP:
		case OPT_INCEXT:
		case OPT_EXCEXT:
		case OPT_INCREG:
		case OPT_EXCREG:
		case OPT_EXCFS:
		case OPT_EXCOM:
		case OPT_INCGLOB:
        	case OPT_FIFOS:
        	case OPT_BLOCKDEVS:
			ck_assert_int_eq(get_strlist(c[o])==NULL, 1);
			break;
		case OPT_PROTOCOL:
			ck_assert_int_eq(get_e_protocol(c[o]), PROTO_AUTO);
			break;
		case OPT_HARD_QUOTA:
		case OPT_SOFT_QUOTA:
		case OPT_MIN_FILE_SIZE:
		case OPT_MAX_FILE_SIZE:
			ck_assert_int_eq(get_ssize_t(c[o]), 0);
			break;
        	case OPT_WORKING_DIR_RECOVERY_METHOD:
			ck_assert_int_eq(get_e_recovery_method(c[o]),
				RECOVERY_METHOD_DELETE);
			break;
        	case OPT_MAX:
			break;
		// No default, so we get compiler warnings if something was
		// missed.
	}
}

START_TEST(test_conf_defaults)
{
	int i=0;
        struct conf **confs=NULL;
        confs=confs_alloc();
	confs_init(confs);
	for(i=0; i<OPT_MAX; i++)
		check_default(confs, (enum conf_opt)i);
	confs_free(&confs);
	ck_assert_int_eq(confs==NULL, 1);
	ck_assert_int_eq(alloc_count, free_count);
}
END_TEST

Suite *conf_suite(void)
{
	Suite *s;
	TCase *tc_core;

	s=suite_create("conf");

	tc_core=tcase_create("Core");

	tcase_add_test(tc_core, test_conf_defaults);
	suite_add_tcase(s, tc_core);

	return s;
}

int main(void)
{
	int number_failed;
	Suite *s;
	SRunner *sr;

	s=conf_suite();
	sr=srunner_create(s);

	srunner_run_all(sr, CK_NORMAL);
	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);
	return number_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}