#include "burp.h"
#include "prog.h"
#include "msg.h"
#include "rs_buf.h"
#include "lock.h"
#include "handy.h"
#include "asyncio.h"
#include "counter.h"
#include "sbuf.h"
#include "list_client.h"

/* Note: The chars in this function are not the same as in the CMD_ set.
   These are for printing to the screen only. */
static char *encode_mode(mode_t mode, char *buf)
{
  char *cp = buf;

  *cp++ = S_ISDIR(mode) ? 'd' : S_ISBLK(mode)  ? 'b' : S_ISCHR(mode)  ? 'c' :
          S_ISLNK(mode) ? 'l' : S_ISFIFO(mode) ? 'f' : S_ISSOCK(mode) ? 's' : '-';
  *cp++ = mode & S_IRUSR ? 'r' : '-';
  *cp++ = mode & S_IWUSR ? 'w' : '-';
  *cp++ = (mode & S_ISUID
               ? (mode & S_IXUSR ? 's' : 'S')
               : (mode & S_IXUSR ? 'x' : '-'));
  *cp++ = mode & S_IRGRP ? 'r' : '-';
  *cp++ = mode & S_IWGRP ? 'w' : '-';
  *cp++ = (mode & S_ISGID
               ? (mode & S_IXGRP ? 's' : 'S')
               : (mode & S_IXGRP ? 'x' : '-'));
  *cp++ = mode & S_IROTH ? 'r' : '-';
  *cp++ = mode & S_IWOTH ? 'w' : '-';
  *cp++ = (mode & S_ISVTX
               ? (mode & S_IXOTH ? 't' : 'T')
               : (mode & S_IXOTH ? 'x' : '-'));
  *cp = '\0';
  return cp;
}

static char *encode_time(utime_t utime, char *buf)
{
   const struct tm *tm;
   int n = 0;
   time_t time = utime;

#if defined(HAVE_WIN32)
   /*
    * Avoid a seg fault in Microsoft's CRT localtime_r(),
    *  which incorrectly references a NULL returned from gmtime() if
    *  time is negative before or after the timezone adjustment.
    */
   struct tm *gtm;

   if ((gtm = gmtime(&time)) == NULL) {
      return buf;
   }

   if (gtm->tm_year == 1970 && gtm->tm_mon == 1 && gtm->tm_mday < 3) {
      return buf;
   }
#endif

   if((tm=localtime(&time)))
   {
      n = sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d",
                   tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
                   tm->tm_hour, tm->tm_min, tm->tm_sec);
   }
   return buf+n;
}

void ls_output(char *buf, const char *fname, struct stat *statp)
{
	char *p;
	const char *f;
	int n;
	time_t time;

	p = encode_mode(statp->st_mode, buf);
	n = sprintf(p, " %2d ", (uint32_t)statp->st_nlink);
	p += n;
	n = sprintf(p, "%5d %5d",
		(uint32_t)statp->st_uid,
		(uint32_t)statp->st_gid);
	p += n;
	n = sprintf(p, " %7lu ", (unsigned long) statp->st_size);
	p += n;
	if (statp->st_ctime > statp->st_mtime)
		time = statp->st_ctime;
	else
		time = statp->st_mtime;

	/* Display most recent time */
	p = encode_time(time, p);
	*p++ = ' ';
	for (f=fname; *f; )
		*p++ = *f++;
	*p = 0;
}

void ls_output_json(char *buf, const int first_entry, const char fcmd, const char *fname, const char *lname, struct stat *statp)
{
	sprintf(buf,
		"%s"
		"%s"
		"\t\t\t{\n"
		"\t\t\t\t\"type\": \"%c\",\n"
		"\t\t\t\t\"name\": \"%s\",\n"
		"\t\t\t\t\"link\": \"%s\",\n"
		"\t\t\t\t\"st_dev\": %lu,\n"
		"\t\t\t\t\"st_ino\": %lu,\n"
		"\t\t\t\t\"st_mode\": %lu,\n"
		"\t\t\t\t\"st_nlink\": %lu,\n"
		"\t\t\t\t\"st_uid\": %d,\n"
		"\t\t\t\t\"st_gid\": %d,\n"
		"\t\t\t\t\"st_rdev\": %lu,\n"
		"\t\t\t\t\"st_size\": %lu,\n"
		"\t\t\t\t\"st_atime\": %lu,\n"
		"\t\t\t\t\"st_mtime\": %lu,\n"
		"\t\t\t\t\"st_ctime\": %lu\n"
		"\t\t\t}",
		first_entry? ("\t\"items\":\n"
			      "\t\t[\n"):"",
		first_entry? "":"\t\t\t,\n",
		fcmd,
		fname,
		lname? lname:"",
		statp->st_dev,
		statp->st_ino,
		statp->st_mode,
		statp->st_nlink,
		(uint32_t)statp->st_uid,
		(uint32_t)statp->st_gid,
		statp->st_rdev,
		statp->st_size,
		statp->st_atime,
		statp->st_mtime,
		statp->st_ctime);
}

int do_list_client(struct config *conf, enum action act, int json)
{
	int ret=0;
	size_t slen=0;
	char msg[64]="";
	char scmd;
	struct stat statp;
	char *statbuf=NULL;
	char ls[2048]="";
	char *dpth=NULL;
	int first_entry=1;
	// format long list as JSON
	int emit_json = (act==ACTION_LONG_LIST && conf->backup && json);
//logp("in do_list\n");

	if(conf->browsedir)
	  snprintf(msg, sizeof(msg), "listb %s:%s",
		conf->backup?conf->backup:"", conf->browsedir);
	else
	  snprintf(msg, sizeof(msg), "list %s:%s",
		conf->backup?conf->backup:"", conf->regex?conf->regex:"");
	if(async_write_str(CMD_GEN, msg)
	  || async_read_expect(CMD_GEN, "ok"))
		return -1;

	if(emit_json)
	{
		printf("{\n");
	}
	
	// This should probably should use the sbuf stuff.
	while(1)
	{
		char fcmd;
		size_t flen=0;
		int64_t winattr=0;
		int compression=-1;
		char *fname=NULL;
		if(async_read(&scmd, &statbuf, &slen))
		{
			//ret=-1; break;
			break;
		}
		if(scmd==CMD_TIMESTAMP)
		{
			// A backup timestamp, just print it.
			if(emit_json)
			{
				printf("\t\"backup\":\n"
				       "\t\t{\n"
				       "\t\t\t\"timestamp\": \"%s\",\n"
				       "\t\t\t\"directory\": \"%s\",\n"
				       "\t\t\t\"regex\": \"%s\"\n"
				       "\t\t},\n",
				       statbuf,
				       conf->browsedir? conf->browsedir:"",
				       conf->regex? conf->regex:"");
			}
			else
			{
				printf("Backup: %s\n", statbuf);
				if(conf->browsedir)
					printf("Listing directory: %s\n",
					       conf->browsedir);
				if(conf->regex)
					printf("With regex: %s\n",
					       conf->regex);
			}
			if(statbuf) { free(statbuf); statbuf=NULL; }
			continue;
		}
		else if(scmd==CMD_DATAPTH)
		{
			if(statbuf) { free(statbuf); statbuf=NULL; }
			continue;
		}
		else if(scmd!=CMD_STAT)
		{
			logp("expected %c cmd - got %c:%s\n",
				CMD_STAT, scmd, statbuf);
			ret=-1; break;
		}

		decode_stat(statbuf, &statp, &winattr, &compression);

		if(async_read(&fcmd, &fname, &flen))
		{
			logp("got stat without an object\n");
			ret=-1; break;
		}
		else if(fcmd==CMD_DIRECTORY
			|| fcmd==CMD_FILE
			|| fcmd==CMD_ENC_FILE
			|| fcmd==CMD_EFS_FILE
			|| fcmd==CMD_SPECIAL)
		{
			*ls='\0';
			if(act==ACTION_LONG_LIST)
			{
				if(emit_json)
				{
					ls_output_json(ls, first_entry, fcmd, fname, NULL, &statp);
				}
				else
				{
					ls_output(ls, fname, &statp);
				}
				printf("%s\n", ls);
			}
			else
			{
				printf("%s\n", fname);
			}
			if (first_entry)
			{
				first_entry = 0;
			}
		}
		else if(cmd_is_link(fcmd)) // symlink or hardlink
		{
			char lcmd;
			size_t llen=0;
			char *lname=NULL;
			if(async_read(&lcmd, &lname, &llen)
			  || lcmd!=fcmd)
			{
				logp("could not get second part of link %c:%s\n", fcmd, fname);
				ret=-1;
			}
			else
			{
				if(act==ACTION_LONG_LIST)
				{
					*ls='\0';
					if(emit_json) {
						ls_output_json(ls, first_entry, fcmd, fname, lname, &statp);
						printf("%s\n", ls);
					}
					else {
						ls_output(ls, fname, &statp);
						printf("%s -> %s\n", ls, lname);
					}
				}
				else
				{
					printf("%s\n", fname);
				}
				if (first_entry)
				{
					first_entry = 0;
				}
			}
			if(lname) free(lname);
		}
		if(fname) free(fname);
		if(statbuf) { free(statbuf); statbuf=NULL; }
	}
	if(statbuf) free(statbuf);
	if(dpth) free(dpth);
	if(emit_json)
	{
		if(!first_entry)
		{
			printf("\t\t]\n");
		}
		printf("}\n");
	}
	if(!ret) logp("List finished ok\n");
	return ret;
}
