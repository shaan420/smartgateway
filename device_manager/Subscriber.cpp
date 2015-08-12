#include "Subscriber.hpp"

CURL *Subscriber::sm_curl = NULL;
ts_context_t *Subscriber::sm_cloud = NULL;

using namespace std;

static const char *payload_text[] = {
	"Date: %s\r\n",
	"From: asu.smarthome@gmail.com (ASU SmartHome)\r\n",
	"Subject: ASU SmartGateway Event Trigger\r\n",
	"\r\n", /* empty line to divide headers from body, see RFC5322 */
	"Event: %s\r\n",
	"\r\n",
	NULL
};

struct UserData
{
	const char *message;
	int lines_read;
};

static size_t payload_source(void *ptr, size_t size, size_t nmemb, void *userp)
{
	const char *data = NULL;
	char custom_data[256];
	struct UserData *udata = (struct UserData *)userp;

	if((size == 0) || (nmemb == 0) || ((size*nmemb) < 1)) 
	{
		return 0;
	}

	if (payload_text[udata->lines_read])
	{
		if (0 == strncmp(payload_text[udata->lines_read], "Date:", strlen("Date:")))
		{
			char time_str[128];
			time_t now = time(NULL);
			struct tm *t = localtime(&now);

			strftime(time_str, sizeof(time_str)-1, "%d %m %Y %H:%M", t);
			snprintf(custom_data, 256, payload_text[udata->lines_read], time_str);

			data = custom_data;
		}
		else if (0 == strncmp(payload_text[udata->lines_read], "Event:", strlen("Event:")))
		{
			snprintf(custom_data, 256, payload_text[udata->lines_read], udata->message);

			data = custom_data;
		}
		else
		{
			data = payload_text[udata->lines_read];
		}
	}

	if(data) 
	{
		size_t len = strlen(data);
		memcpy(ptr, data, len);
		udata->lines_read++;

		return len;
	}

	return 0;
}

int Subscriber::Publish(const char *message, const char *val)
{
	switch (m_type)
	{
		case SUBSCRIBER_NOTIFICATION_TYPE_SMS:
			cout << "SMS not yet supported\n";
			break;

		case SUBSCRIBER_NOTIFICATION_TYPE_EMAIL:
			Sendmail(m_params.c_str(), message, val);
			break;

		case SUBSCRIBER_NOTIFICATION_TYPE_CLOUD:
			SendToCloud(m_params.c_str(), message, val);
			break;

		default:
			cout << "ERROR: invalid notification type\n";
			break;
	}

	return 0;
}

static int prepare_udata(struct UserData *udata, const char *m)
{
	if ((udata == NULL) || (m == NULL))
	{
		return -1;
	}

	udata->lines_read = 0;
	udata->message = m;

	return 0;
}

int Subscriber::Sendmail(const char *to, 
			 			 const char *message,
						 const char *val)
{
	struct UserData udata;
	CURLcode res = CURLE_OK;
	struct curl_slist *recipients = NULL;
	char msg[256];

	snprintf(msg, 256, "EventName=%s&Val=%s", message, val);

	if (sm_curl != NULL) 
	{
		recipients = curl_slist_append(recipients, to);
		curl_easy_setopt(sm_curl, CURLOPT_MAIL_RCPT, recipients);

		/* We're using a callback function to specify the payload (the headers and
		 * body of the message). You could just use the CURLOPT_READDATA option to
		 * specify a FILE pointer to read from. */
		curl_easy_setopt(sm_curl, CURLOPT_READFUNCTION, payload_source);

		prepare_udata(&udata, msg);
		curl_easy_setopt(sm_curl, CURLOPT_READDATA, &udata);
		curl_easy_setopt(sm_curl, CURLOPT_UPLOAD, 1L);

		/* Since the traffic will be encrypted, it is very useful to turn on debug
		 * information within libcurl to see what is happening during the transfer.
		 */
		curl_easy_setopt(sm_curl, CURLOPT_VERBOSE, 1L);

		/* Send the message */
		cout << "Sending to " << to << endl;
		res = curl_easy_perform(sm_curl);

		/* Check for errors */
		if(res != CURLE_OK)
			fprintf(stderr, "curl_easy_perform() failed: %s\n",
					curl_easy_strerror(res));

		/* Free the list of recipients */
		curl_slist_free_all(recipients);
	}
	else 
	{
		perror("ERROR: Failed to send email");
	}

	return 0;
}

int Subscriber::SendToCloud(const char *field_id, 
							const char *message, 
							const char *val)
{
	char field_str[32];
	ts_datapoint_t data;
	memset(&data, 0, sizeof(ts_datapoint_t));

	float value = strtof(val, NULL);
	ts_set_value_f32(&data, value);

	snprintf(field_str, 32, "field%s", field_id);
	ts_datastream_update(sm_cloud, 0, field_str, &data);
	return 0;
}


