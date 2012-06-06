/**
 *  @brief self-test and example code for liboauth using 
 *         HTTP Authorization header.
 *  @file oauthtest.c
 *  @author Robin Gareus <robin@gareus.org>
 *
 * Copyright 2010 Robin Gareus <robin@gareus.org>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <oauth.h>

/* 
 * a example requesting and parsing a request-token from an OAuth service-provider
 * using OAuth HTTP Authorization header:
 * see http://oauth.net/core/1.0a/#auth_header
 * and http://oauth.net/core/1.0a/#consumer_req_param
 */
void request_token_example_get(void) {
#if 0
  const char *request_token_uri = "http://localhost/oauthtest.php?test=test";
  const char *req_c_key         = "17b09ea4c9a4121145936f0d7d8daa28047583796"; //< consumer key
  const char *req_c_secret      = "942295b08ffce77b399419ee96ac65be"; //< consumer secret
#else
  const char *request_token_uri = "http://term.ie/oauth/example/request_token.php";
  const char *req_c_key         = "key"; //< consumer key
  const char *req_c_secret      = "secret"; //< consumer secret
#endif
  char *res_t_key    = NULL; //< replied key
  char *res_t_secret = NULL; //< replied secret

  char *req_url = NULL;
  char *req_hdr = NULL;
  char *http_hdr= NULL;
  char *reply;


  //req_url = oauth_sign_url2(request_token_uri, NULL, OA_HMAC, NULL, req_c_key, req_c_secret, NULL, NULL);

  // oauth_sign_url2 (see oauth.h) in steps
  int  argc;
  char **argv = NULL;

  argc = oauth_split_url_parameters(request_token_uri, &argv);
  if (1) {
    int i;
    for (i=0;i<argc; i++)
      printf("%d:%s\n", i, argv[i]);
  }

  oauth_sign_array2_process(&argc, &argv,
          NULL, //< postargs (unused)
          OA_HMAC,
          NULL, //< HTTP method (defaults to "GET")
          req_c_key, req_c_secret, NULL, NULL);

  // 'default' oauth_sign_url2 would do:
  // req_url = oauth_serialize_url(argc, 0, argv);
 
  // we split [x_]oauth_ parameters (for use in HTTP Authorization header)
  req_hdr = oauth_serialize_url_sep(argc, 1, argv, ", ", 6);
  // and other URL parameters 
  req_url = oauth_serialize_url_sep(argc, 0, argv, "&", 1);

  oauth_free_array(&argc, &argv);

  // done with OAuth stuff, now perform the HTTP request.
  http_hdr = malloc(strlen(req_hdr) + 55);

  // Note that (optional) 'realm' is not to be 
  // included in the oauth signed parameters and thus only added here.
  // see 9.1.1 in http://oauth.net/core/1.0/#anchor14
  sprintf(http_hdr, "Authorization: OAuth realm=\"http://example.org/\", %s", req_hdr);

  printf("request URL=%s\n", req_url);
  printf("request header=%s\n\n", http_hdr);
  reply = oauth_http_get2(req_url,NULL, http_hdr);
  if (!reply) 
    printf("HTTP request for an oauth request-token failed.\n");
  else {
    // parse reply - example:
    //"oauth_token=2a71d1c73d2771b00f13ca0acb9836a10477d3c56&oauth_token_secret=a1b5c00c1f3e23fb314a0aa22e990266"
    int rc;
    char **rv = NULL;

    printf("HTTP-reply: %s\n", reply);
    rc = oauth_split_url_parameters(reply, &rv);
    qsort(rv, rc, sizeof(char *), oauth_cmpstringp);
    if( rc==2 
	&& !strncmp(rv[0],"oauth_token=",11)
	&& !strncmp(rv[1],"oauth_token_secret=",18) ){
	  res_t_key=strdup(&(rv[0][12]));
	  res_t_secret=strdup(&(rv[1][19]));
	  printf("key:    '%s'\nsecret: '%s'\n",res_t_key, res_t_secret);
    }
    if(rv) free(rv);
  }

  if(req_url) free(req_url);
  if(req_hdr) free(req_hdr);
  if(http_hdr)free(http_hdr);
  if(reply) free(reply);
  if(res_t_key) free(res_t_key);
  if(res_t_secret) free(res_t_secret);
}

/*
 * Main Test and Example Code.
 */
int main (int argc, char **argv) {
  request_token_example_get();
  return (0);
}
