#include "include.h"

#include <netdb.h>
#include <dirent.h>

static int get_link(const char *dir, const char *lnk, char real[], size_t r)
{
	ssize_t len=0;
	char *tmp=NULL;
	if(!(tmp=prepend_s(dir, lnk)))
		return -1;
	if((len=readlink(tmp, real, r-1))<0) len=0;
	real[len]='\0';
	free(tmp);
	return 0;
}

static void have_backup_file_name(struct bu *bu,
        const char *file, uint32_t bit)
{
	struct stat statp;
	static char path[256]="";
	snprintf(path, sizeof(path), "%s/%s", bu->path, file);
	if(lstat(path, &statp)) return;
	bu->flags|=bit;
}

static void have_backup_file_name_w(struct bu *bu,
	const char *file, uint32_t bit)
{
	char compressed[32];
	snprintf(compressed, sizeof(compressed), "%s.gz", file);
	have_backup_file_name(bu, file, bit);
	have_backup_file_name(bu, compressed, bit);
}

static int maybe_add_ent(const char *dir, const char *d_name,
	struct bu **bu_list, uint16_t flags, struct cstat *include_working)
{
	int ret=-1;
	char buf[32]="";
	struct stat statp;
	char *fullpath=NULL;
	char *timestamp=NULL;
	char *timestampstr=NULL;
	char *hlinkedpath=NULL;
	char *basename=NULL;
	struct bu *bu=NULL;

	if(!(basename=prepend("", d_name, strlen(d_name), ""))
	 || !(fullpath=prepend_s(dir, basename))
	 || !(timestamp=prepend_s(fullpath, "timestamp"))
	 || !(hlinkedpath=prepend_s(fullpath, "hardlinked")))
		goto error;

	if((!lstat(fullpath, &statp) && !S_ISDIR(statp.st_mode))
	  || lstat(timestamp, &statp) || !S_ISREG(statp.st_mode)
	  || timestamp_read(timestamp, buf, sizeof(buf)))
	{
		ret=0; // For resilience.
		goto error;
	}
	free_w(&timestamp);

	if(!(timestampstr=strdup_w(buf, __func__)))
		goto error;

	if(!lstat(hlinkedpath, &statp)) flags|=BU_HARDLINKED;

	if(!(bu=bu_alloc())
	  || bu_init(bu, fullpath, basename, timestampstr, flags))
		goto error;

	if(*bu_list) bu->next=*bu_list;
	*bu_list=bu;
	have_backup_file_name_w(bu, "manifest", BU_MANIFEST);
	if(include_working)
	{
		have_backup_file_name_w(bu, "log", BU_LOG_BACKUP);
		have_backup_file_name_w(bu, "restorelog", BU_LOG_RESTORE);
		have_backup_file_name_w(bu, "verifylog", BU_LOG_VERIFY);
		// Hack to include option for live counters.
		if(include_working->run_status==RUN_STATUS_RUNNING)
		{
			switch(include_working->cntr->cntr_status)
			{
				case CNTR_STATUS_SCANNING:
				case CNTR_STATUS_BACKUP:
				case CNTR_STATUS_MERGING:
				case CNTR_STATUS_SHUFFLING:
					bu->flags|=BU_STATS_BACKUP;
					break;
				case CNTR_STATUS_VERIFYING:
					bu->flags|=BU_STATS_VERIFY;
					break;
				case CNTR_STATUS_RESTORING:
					bu->flags|=BU_STATS_VERIFY;
					break;
				default:
					break;
			}
		}
		if(!(bu->flags & BU_STATS_BACKUP))
		  have_backup_file_name(bu, "backup_stats", BU_STATS_BACKUP);
		if(!(bu->flags & BU_STATS_RESTORE))
		  have_backup_file_name(bu, "restore_stats", BU_STATS_RESTORE);
		if(!(bu->flags & BU_STATS_VERIFY))
		  have_backup_file_name(bu, "verify_stats", BU_STATS_VERIFY);
	}

	return 0;
error:
	free_w(&basename);
	free_w(&fullpath);
	free_w(&timestamp);
	free_w(&timestampstr);
	free_w(&hlinkedpath);
	return ret;
}

static void setup_indices(struct bu *bu_list)
{
	int i;
	int tr=0;
	struct bu *bu=NULL;
	struct bu *last=NULL;

	i=1;
	for(bu=bu_list; bu; bu=bu->next)
	{
		// Enumerate the position of each entry.
		bu->index=i++;

		// Backups that come after hardlinked backups are deletable.
		if((bu->flags & BU_HARDLINKED) && bu->next)
			bu->next->flags|=BU_DELETABLE;

		// Also set up reverse linkage.
		bu->prev=last;
		last=bu;
	}

	// The oldest backup is deletable.
	if(bu_list) bu_list->flags|=BU_DELETABLE;

	if(last)
	{

		if((tr=last->bno))
		{
			// Transpose bnos so that the oldest bno is set to 1.
			for(bu=bu_list; bu; bu=bu->next)
				bu->trbno=tr-bu->bno+1;
		}
	}
}

static int rev_alphasort(const struct dirent **a, const struct dirent **b)
{
	static int s;
	if((s=strcmp((*a)->d_name, (*b)->d_name))>0)
		return -1;
	if(s<0)
		return 1;
	return 0;
}

static int do_bu_get_list(struct sdirs *sdirs,
	struct bu **bu_list, struct cstat *include_working)
{
	int i=0;
	int n=0;
	int ret=-1;
	char realwork[32]="";
	char realfinishing[32]="";
	char realcurrent[32]="";
	struct dirent **dp=NULL;
	const char *dir=sdirs->client;
	uint16_t flags=0;

	if(get_link(dir, "working", realwork, sizeof(realwork))
	  || get_link(dir, "finishing", realfinishing, sizeof(realfinishing))
	  || get_link(dir, "current", realcurrent, sizeof(realcurrent)))
		goto end;

	if((n=scandir(dir, &dp, NULL, rev_alphasort))<0)
	{
		logp("scandir failed in %s: %s\n", __func__, strerror(errno));
		goto end;
	}
	for(i=0; i<n; i++)
	{
		if(!dp[i]->d_ino
		  || !strcmp(dp[i]->d_name, ".")
		  || !strcmp(dp[i]->d_name, ".."))
			continue;
		flags=0;
		if(!strcmp(dp[i]->d_name, realcurrent))
		{
			flags|=BU_CURRENT;
		}
		else if(!strcmp(dp[i]->d_name, realwork))
		{
			if(!include_working) continue;
			flags|=BU_WORKING;
		}
		else if(!strcmp(dp[i]->d_name, realfinishing))
		{
			if(!include_working) continue;
			flags|=BU_FINISHING;
		}
		if(maybe_add_ent(dir, dp[i]->d_name, bu_list, flags,
			include_working)) goto end;
	}

	setup_indices(*bu_list);

	ret=0;
end:
	if(dp)
	{
		for(i=0; i<n; i++) free(dp[i]);
		free(dp);
	}
	return ret;
}

int bu_get_list(struct sdirs *sdirs, struct bu **bu_list)
{
	return do_bu_get_list(sdirs, bu_list, NULL);
}

int bu_get_list_with_working(struct sdirs *sdirs, struct bu **bu_list,
	struct cstat *cstat)
{
	return do_bu_get_list(sdirs, bu_list, cstat);
}

int bu_get_current(struct sdirs *sdirs, struct bu **bu_list)
{
	char real[32]="";
	// FIX THIS: should not need to specify "current".
	if(get_link(sdirs->client, "current", real, sizeof(real)))
		return -1;
	return maybe_add_ent(sdirs->client, real, bu_list, BU_CURRENT, NULL);
}
