#include "include.h"
#include "../cmd.h"

struct protocol1 *sbuf_protocol1_alloc(void)
{
	return (struct protocol1 *)calloc_w(1,
		sizeof(struct protocol1), __func__);
}

void sbuf_protocol1_free_content(struct protocol1 *protocol1)
{
	if(!protocol1) return;
	memset(&(protocol1->rsbuf), 0, sizeof(protocol1->rsbuf));
	if(protocol1->sigjob)
		{ rs_job_free(protocol1->sigjob); protocol1->sigjob=NULL; }
	if(protocol1->infb)
		{ rs_filebuf_free(protocol1->infb); protocol1->infb=NULL; }
	if(protocol1->outfb)
		{ rs_filebuf_free(protocol1->outfb); protocol1->outfb=NULL; }
	close_fp(&protocol1->sigfp); protocol1->sigfp=NULL;
	gzclose_fp(&protocol1->sigzp); protocol1->sigzp=NULL;
	close_fp(&protocol1->fp); protocol1->fp=NULL;
	gzclose_fp(&protocol1->zp); protocol1->zp=NULL;
	iobuf_free_content(&protocol1->datapth);
	iobuf_free_content(&protocol1->endfile);
	protocol1->datapth.cmd=CMD_DATAPTH;
	protocol1->endfile.cmd=CMD_END_FILE;
}
