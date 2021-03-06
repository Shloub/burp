#ifndef DPTH_PROTOCOL1_H
#define DPTH_PROTOCOL1_H

struct dpthl
{
	int prim;
	int seco;
	int tert;
	char path[32];
};

extern int init_dpthl(struct dpthl *dpthl,
	struct sdirs *sdirs, struct conf **cconfs);
extern int incr_dpthl(struct dpthl *dpthl, struct conf **cconfs);
extern int set_dpthl_from_string(struct dpthl *dpthl, const char *datapath);
extern void mk_dpthl(struct dpthl *dpthl, struct conf **cconfs, enum cmd cmd);

#endif
