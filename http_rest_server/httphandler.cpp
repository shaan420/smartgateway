#include <signal.h>
#include <pthread.h>
#include "platform.h"
#include <microhttpd.h>
#include <iostream>
#include <map>
#include <string>

#include "RestAPI.hpp"

using std::map;
using std::string;

#define PAGE "<html><head><title>Error</title></head><body>Bad data</body></html>"
#define PORT            8888
#define POSTBUFFERSIZE  512
#define MAXNAMESIZE     20
#define MAXANSWERSIZE   512

#define GET             0
#define POST            1

REST::RestAPI g_rest_api;

static int shouldNotExit = 1;

struct connection_info_struct
{
	struct MHD_Connection *con;
	const char *url;
	int connectiontype;
	char *answerstring;
};

static int send_page (struct MHD_Connection *connection, const char *page)
{
	int ret;
	struct MHD_Response *response;


	response =
		MHD_create_response_from_buffer (strlen (page), (void *) page,
				MHD_RESPMEM_PERSISTENT);
	if (!response)
		return MHD_NO;

	ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
	MHD_destroy_response (response);

	return ret;
}

static int get_url_args_map(void *cls, MHD_ValueKind kind,
		const char *key , const char* value)
{
	map<string, string> * url_args = static_cast<map<string, string> *>(cls);

	printf("key=%s value=%s\n", key, value);
	if (url_args->find(key) == url_args->end()) {
		if (!value)
			(*url_args)[key] = "";
		else 
			(*url_args)[key] = value;
	}
	return MHD_YES;

}

static int get_url_args_string(void *cls, MHD_ValueKind kind,
		const char *key , const char* value)
{
	string *url_args = static_cast<string*>(cls);

	printf("key=%s value=%s\n", key, value);

	if ((*url_args).length() > 0)
		*url_args += '&';

	*url_args += key;
	*url_args += '=';
	*url_args += value;
	
	return MHD_YES;

}

static int process_post_request(struct connection_info_struct *con_info, 
							    const char *upload_data, 
								int upload_data_size)
{
	string url_string;
	map<string, string> url_args;
	string respdata;

	if (MHD_get_connection_values (con_info->con, MHD_GET_ARGUMENT_KIND, 
				get_url_args_map, &url_args) < 0) 
	{
		return MHD_NO;
	}

	if (MHD_get_connection_values (con_info->con, MHD_GET_ARGUMENT_KIND, 
				get_url_args_string, &url_string) < 0) 
	{
		return MHD_NO;
	}

	printf("url=<%s>\n", con_info->url);

	g_rest_api.executeRestAPI(con_info->url, url_string, url_args, upload_data, upload_data_size, respdata);
	std::cout << "RespData: " << respdata << std::endl;

	if ((respdata.length() > 0) && (respdata.length() <= MAXNAMESIZE))
	{
		char *answerstring;
		answerstring = (char *)malloc(MAXANSWERSIZE);
		if (!answerstring)
			return MHD_NO;

		snprintf (answerstring, MAXANSWERSIZE, "%s", respdata.c_str());
		con_info->answerstring = answerstring;
	}
	else
		con_info->answerstring = NULL;

	return MHD_NO;
}

static int url_handler (void *cls,
		struct MHD_Connection *connection,
		const char *url,
		const char *method,
		const char *version,
		const char *upload_data, size_t *upload_data_size, void **con_cls)
{
	if (NULL == *con_cls)
	{
		struct connection_info_struct *con_info;

		con_info = (struct connection_info_struct *)malloc(sizeof (struct connection_info_struct));
		if (NULL == con_info)
			return MHD_NO;
		con_info->answerstring = NULL;
		con_info->url = url;
		con_info->con = connection;
		con_info->connectiontype = -1;

		if (0 == strcmp (method, "POST"))
		{
			con_info->connectiontype = POST;
		}
		else
		{
			con_info->connectiontype = GET;
		}

		*con_cls = (void *) con_info;

		return MHD_YES;
	}

	struct connection_info_struct *con_info = *(struct connection_info_struct **)con_cls;

	// Support only GET and POST for demonstration
	if (con_info->connectiontype == -1)
	{
		return MHD_NO; 
	}

	if (con_info->connectiontype == POST)
	{
		if (*upload_data_size != 0)
		{
			process_post_request (con_info, upload_data,
					*upload_data_size);
			*upload_data_size = 0;

			return MHD_YES;
		}
		else if (NULL != con_info->answerstring)
		{
			return send_page(connection, con_info->answerstring);
		}
	}

	return send_page(connection, PAGE);
}

void handle_term(int signo)
{
	shouldNotExit = 0;
}

void request_completed (void *cls, struct MHD_Connection *connection, 
					    void **con_cls,
						enum MHD_RequestTerminationCode toe)
{
	struct connection_info_struct *con_info = *(struct connection_info_struct **)con_cls;

	if (NULL == con_info) return;
	if (con_info->connectiontype == POST)
	{
		if (con_info->answerstring) free (con_info->answerstring);
	}

	free (con_info);
	*con_cls = NULL;   
}

void* http(void *arg)
{
	int *port = (int *)arg;
	struct MHD_Daemon *d;

	d = MHD_start_daemon (MHD_USE_SELECT_INTERNALLY, *port, NULL, NULL,
						  &url_handler, (void *)PAGE, 
						  MHD_OPTION_NOTIFY_COMPLETED, &request_completed, NULL,
						  MHD_OPTION_END);

	if (d == 0)
	{
		return 0;
	}

	while(shouldNotExit) 
	{
		sleep(1);
	}

	MHD_stop_daemon (d);
	return 0;
}

int main (int argc, char *const *argv)
{
	if (argc != 2)
	{
		printf ("%s PORT\n", argv[0]);
		exit(1);
	}

	daemon(0,1);
	signal(SIGTERM, handle_term);
	int port = atoi(argv[1]);
	pthread_t  thread;

	if ( 0 != pthread_create(&thread, 0 , http, &port))
	{
		printf("thread error\n");
		exit(1);
	}

	pthread_join(thread, 0);
	return 0;
}
