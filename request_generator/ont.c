/***************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * Copyright (C) 1998 - 2011, Daniel Stenberg, <daniel@haxx.se>, et al.
 *
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this distribution. The terms
 * are also available at http://curl.haxx.se/docs/copyright.html.
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ***************************************************************************/
#include <stdio.h>
#include <curl/curl.h>

int main(int argc, char *argv[])
{
	CURL *curl;
	CURLcode res;

	/* In windows, this will init the winsock stuff */
	curl_global_init(CURL_GLOBAL_ALL);

	/* get a curl handle */
	curl = curl_easy_init();
	if(curl) {
		/* First set the URL that is about to receive our POST. This URL can
		   just as well be a https:// URL if that is what should receive the
		   data. */
		//curl_easy_setopt(curl, CURLOPT_URL, "http://10.218.100.213:8080/rule");
		//curl_easy_setopt(curl, CURLOPT_URL, "http://10.218.100.213:8080/event");
		curl_easy_setopt(curl, CURLOPT_URL, "http://192.168.0.33:8080/getOntData");
		/* Now specify the POST data */
		//curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "Action=insert_event&IsOneShot=false&EventName=lightOn&Event=event(lightOn, event, 0, Ts) :- get(lighting1, get_status, X1, Ts1), get(lighting1, get_status, X2, Ts) , X1=:=0, X2 =:= 1, (Ts =:= (Ts1+1))");
		//curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "Action=insert_rule&IsOneShot=true&Rule=do(lighting1, set_status, 0, Ts) :- get(lighting1, get_status, X1, Ts), X1 =:= 1");
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "DeviceClass=Lighting&hasLocation=room1");

		/* Perform the request, res will get the return code */
		res = curl_easy_perform(curl);
		/* Check for errors */
		if(res != CURLE_OK)
			fprintf(stderr, "curl_easy_perform() failed: %s\n",
					curl_easy_strerror(res));

		/* always cleanup */
		curl_easy_cleanup(curl);
	}
	curl_global_cleanup();
	return 0;
}
