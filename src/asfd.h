#ifndef _ASFD_H
#define _ASFD_H

#include "burp.h"
#include "cmd.h"
#include "ssl.h"

// Return values for simple_loop().
enum asl_ret
{
	ASL_END_ERROR=-1,
	ASL_CONTINUE=0,
	ASL_END_OK=1,
	ASL_END_OK_RETURN_1=2
};

enum asfd_streamtype
{
	ASFD_STREAM_STANDARD=0,
	ASFD_STREAM_LINEBUF,
	ASFD_STREAM_NCURSES_STDIN
};

enum asfd_fdtype
{
	ASFD_FD_UNSET=0,
	ASFD_FD_SERVER_LISTEN_MAIN,
	ASFD_FD_SERVER_LISTEN_STATUS,
	ASFD_FD_SERVER_PIPE_READ,
	ASFD_FD_SERVER_PIPE_WRITE,
	ASFD_FD_SERVER_CHILD_MAIN,
	ASFD_FD_SERVER_TO_CHAMP_CHOOSER,
	ASFD_FD_CHILD_MAIN,
	ASFD_FD_CHILD_PIPE_READ,
	ASFD_FD_CHILD_PIPE_WRITE,
	ASFD_FD_CLIENT_MONITOR_READ,
	ASFD_FD_CLIENT_MONITOR_WRITE,
	ASFD_FD_CLIENT_NCURSES_READ
};

enum append_ret
{
	APPEND_ERROR=-1,
	APPEND_OK=0,
	APPEND_BLOCKED=1
};

// Async file descriptor. Can add these to a struct async.
struct asfd
{
	int fd;
	SSL *ssl;
	struct async *as;
	char *desc;
	enum asfd_streamtype streamtype;

	int network_timeout;
	int max_network_timeout;

	float ratelimit;
	time_t rlstart;
	int rlsleeptime;
	unsigned long long rlbytes;

	struct iobuf *rbuf;

	int doread;
	char *readbuf;
	size_t readbuflen;
	int read_blocked_on_write;

	int dowrite;
	char *writebuf;
	size_t writebuflen;
	int write_blocked_on_read;

	struct asfd *next;

	// Stuff for the champ chooser server.
	struct incoming *in;
	struct blist *blist;
	int blkcnt;
	uint64_t wrap_up;
	uint8_t want_to_remove;

	// For the champ chooser server main socket.
	uint8_t listening_for_new_clients;
	uint8_t new_client;

	// For the main server process.
	pid_t pid;
	enum asfd_fdtype fdtype;

	// Function pointers.
	int (*init)(struct asfd *, const char *,
		struct async *, int, SSL *,
		enum asfd_streamtype, struct conf **);
	int (*parse_readbuf)(struct asfd *);
	int (*parse_readbuf_specific)(struct asfd *);
	enum append_ret
		(*append_all_to_write_buffer)(struct asfd *, struct iobuf *);
	int (*set_bulk_packets)(struct asfd *);
	int (*do_read)(struct asfd *);
	int (*do_write)(struct asfd *);
	int (*read)(struct asfd *);
	int (*read_expect)(struct asfd *, enum cmd, const char *);
	int (*simple_loop)(struct asfd *, struct conf **, void *,
		const char *, enum asl_ret callback(struct asfd *,
			struct conf **, void *));
	int (*write)(struct asfd *, struct iobuf *);
	int (*write_str)(struct asfd *, enum cmd, const char *);
	int (*write_strn)(struct asfd *, enum cmd, const char *, size_t);
};

extern struct asfd *asfd_alloc(void);
extern void asfd_close(struct asfd *asfd); // Maybe should be in the struct.
extern void asfd_free(struct asfd **asfd);

extern struct asfd *setup_asfd(struct async *as,
	const char *desc, int *fd, SSL *ssl,
	enum asfd_streamtype asfd_streamtype, enum asfd_fdtype fdtype,
	pid_t pid, struct conf **conf);

#endif
