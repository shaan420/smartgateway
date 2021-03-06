#ifndef  _TS_HTTP_H_
# define _TS_HTTP_


#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>


#define SA      struct sockaddr
#define MAXLINE 8192
#define MAXSUB  1024


#define LISTENQ         1024
#define HOST_API        "api.thingspeak.com"

extern int h_errno;


ssize_t ts_http_post(ts_context_t *, char *, char *, char *);
ssize_t ts_http_put(ts_context_t *, char *, char *, char *);
ssize_t ts_http_delete(ts_context_t *, char *, char *);
char   *ts_http_get(ts_context_t *, char *, char *, char *);

#endif /*_TS_HTTP_*/
