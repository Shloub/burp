#ifndef _CHAMP_SERVER_H
#define _CHAMP_SERVER_H

#define FILENO_LEN	8

extern int champ_chooser_server(struct sdirs *sdirs, struct conf **confs);
extern int champ_chooser_server_standalone(struct conf **globalcs);

#endif
