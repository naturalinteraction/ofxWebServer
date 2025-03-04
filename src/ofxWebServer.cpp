#ifdef __linux__

/*
 *  ofxWebServer.cpp
 *  webServerExample
 *
 *  Created by Marek Bereza on 26/03/2010.
 *  Copyright 2010 Marek Bereza. All rights reserved.
 *
 */

#include "ofxWebServer.h"

#ifdef TARGET_WIN32
#include "winsock.h"
#else
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

void ipLongToString(long ipLong, char *ipString) {
	unsigned short a, b, c, d;
	a = (ipLong & (0xff << 24)) >> 24;
	b = (ipLong & (0xff << 16)) >> 16;
	c = (ipLong & (0xff << 8)) >> 8;
	d = ipLong & 0xff;
	sprintf(ipString, "%hu.%hu.%hu.%hu", a, b, c, d);
}


void handleMultipart(ofxWSRequestHandler *handler, const struct mg_request_info *info) {
	printf("It's multipart! - post data length %d\n",info->post_data_len);

	string type = "";
	for(int i = 0; i < info->num_headers; i++) {
		string name = info->http_headers[i].name;
		name = ofToLower(name);
		if(name.find("content-type")==0) {
			type = info->http_headers[i].value;
			break;
			//printf("%s -> %s\n", info->http_headers[i].name, info->http_headers[i].value);
		}
	}
	if(type=="") return;



	int bp = type.find("boundary=");

	if (bp==-1) {
		return;
	}

	string boundary = std::string("--") + type.substr(bp + 9, type.length() - bp);
	printf("boundary: %s\n", boundary.c_str());

	/*char *dat = new char [info->post_data_len+1];


	memcpy(dat, info->post_data, info->post_data_len);
	printf("%s\n", dat);
	 */
	for(int i = 0; i < info->num_headers; i++) {
		printf("%s -> %s\n", info->http_headers[i].name, info->http_headers[i].value);
	}
	/*
	delete dat;*/
}

// callback from server
void webserverCallback(struct mg_connection *conn,
					   const struct mg_request_info *info,
					   void *user_data) {


	string query = "";
	if(info->query_string!=NULL) query = info->query_string;
	
	
	char ip[16];
	
	ipLongToString(info->remote_ip, ip);
	((ofxWSRequestHandler*)user_data)->setConnection(conn, ip, query);

	string method = info->request_method;
	method = ofToLower(method);

	if(method=="get") {




//		printf("--->Get\n");

		// if it's not multipart data, we've just got a normal GET going on.
		((ofxWSRequestHandler*)user_data)->httpGet(info->uri);




	} else if(strcmp(info->request_method,"POST")==0 || strcmp(info->request_method,"post")) {
		for(int i = 0; i < info->num_headers; i++) {
			if(strcmp(info->http_headers[i].name, "Content-Type")==0) {
				string type = info->http_headers[i].value;
				if(type.find("multipart/form-data")==0) {
					handleMultipart(((ofxWSRequestHandler*)user_data), info);
				}
			}
		}
		((ofxWSRequestHandler*)user_data)->httpPost(info->uri, info->post_data, info->post_data_len);
	} else {
		printf("ofxWebServer: unhandled '%s' request method\n", info->request_method);
	}
}

ofxWebServer::ofxWebServer() {

}

void ofxWebServer::start(string root, int port) {
	ctx = mg_start();     // Start Mongoose serving thread
	mg_set_option(ctx, "root", ofToDataPath(root, true).c_str());  // Set document root
	mg_set_option(ctx, "ports", ofToString(port).c_str());    // Listen on port XXXX
}

void ofxWebServer::stop() {
	mg_stop(ctx);
}

void ofxWebServer::addHandler(ofxWSRequestHandler *handler, string pattern) {

	// make sure there's a forward slash at the beginning
	if(pattern.size()==0) {
		pattern = "/";
	} else if(pattern[0]!='/') {
		pattern = "/" + pattern;
	}

	mg_set_uri_callback(ctx, pattern.c_str(), webserverCallback, handler);
}

string ofxWebServer::getIPAddress() {
	char baseHostName[255 - 6];
	gethostname(baseHostName, 255 - 6);

	// Adjust for iPhone -- add .local to the host name
	char hn[255];
	sprintf(hn, "%s.local", baseHostName);

	struct hostent *host = gethostbyname(hn);
	if (host == NULL)
	{
		printf("resolv error\n");
		return NULL;
	} else {
		struct in_addr **list = (struct in_addr **)host->h_addr_list;
		return inet_ntoa(*list[0]);
	}
}
/*
void ofxWebServer::callback(struct mg_connection *conn,
			  const struct mg_request_info *info) {
	mg_printf(conn, "<html><body>chrcxdflfkjThis is <b>HTML</b>!</body></html>\n");
}

*/

#endif