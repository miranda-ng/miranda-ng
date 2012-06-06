/**
 *  @brief self-test and example code for liboauth 
 *  @file oauthtest.c
 *  @author Robin Gareus <robin@gareus.org>
 *
 * Copyright 2007, 2008 Robin Gareus <robin@gareus.org>
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
 * excercising the oauth-HTTP GET function. - it is almost the same as 
 * \ref request_token_example_post below. 
 */
void request_token_example_get(void) {
#if 0
  const char *request_token_uri = "http://oauth-sandbox.mediamatic.nl/module/OAuth/request_token";
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
  char *reply;

  req_url = oauth_sign_url2(request_token_uri, NULL, OA_HMAC, NULL, req_c_key, req_c_secret, NULL, NULL);

  printf("request URL:%s\n\n", req_url);
  reply = oauth_http_get(req_url,NULL);
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
  if(reply) free(reply);
  if(res_t_key) free(res_t_key);
  if(res_t_secret) free(res_t_secret);
}

/*
 * a example requesting and parsing a request-token from an OAuth service-provider
 * using the oauth-HTTP POST function.
 */
void request_token_example_post(void) {
#if 0
  const char *request_token_uri = "http://oauth-sandbox.mediamatic.nl/module/OAuth/request_token";
  const char *req_c_key         = "17b09ea4c9a4121145936f0d7d8daa28047583796"; //< consumer key
  const char *req_c_secret      = "942295b08ffce77b399419ee96ac65be"; //< consumer secret
#else
  const char *request_token_uri = "http://term.ie/oauth/example/request_token.php";
  const char *req_c_key         = "key"; //< consumer key
  const char *req_c_secret      = "secret"; //< consumer secret
#endif
  char *res_t_key    = NULL; //< replied key
  char *res_t_secret = NULL; //< replied secret

  char *postarg = NULL;
  char *req_url;
  char *reply;

  req_url = oauth_sign_url2(request_token_uri, &postarg, OA_HMAC, NULL, req_c_key, req_c_secret, NULL, NULL);

  printf("request URL:%s\n\n", req_url);
  reply = oauth_http_post(req_url,postarg);
  if (!reply) 
    printf("HTTP request for an oauth request-token failed.\n");
  else {
    //parse reply - example:
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
  if(postarg) free(postarg);
  if(reply) free(reply);
  if(res_t_key) free(res_t_key);
  if(res_t_secret) free(res_t_secret);
}


/*
 * Main Test and Example Code.
 * 
 * compile:
 *  gcc -lssl -loauth -o oauthtest oauthtest.c
 */
int main (int argc, char **argv) {
  int fail=0;

  const char *url      = "http://base.url/&just=append?post=or_get_parameters"
                         "&arguments=will_be_formatted_automatically?&dont_care"
			 "=about_separators";
			 //< the url to sign
  const char *c_key    = "1234567890abcdef1234567890abcdef123456789";
  			//< consumer key
  const char *c_secret = "01230123012301230123012301230123";
  			//< consumer secret
  const char *t_key    = "0987654321fedcba0987654321fedcba098765432";
  			//< token key
  const char *t_secret = "66666666666666666666666666666666";
  			//< token secret

#if 1 // example sign GET request and print the signed request URL
  {
    char *geturl = NULL;
    geturl = oauth_sign_url2(url, NULL, OA_HMAC, NULL, c_key, c_secret, t_key, t_secret);
    printf("GET: URL:%s\n\n", geturl);
    if(geturl) free(geturl);
  }
#endif

#if 1 // sign POST ;) example 
  {
    char *postargs = NULL, *post = NULL;
    post = oauth_sign_url2(url, &postargs, OA_HMAC, NULL, c_key, c_secret, t_key, t_secret);
    printf("POST: URL:%s\n      PARAM:%s\n\n", post, postargs);
    if(post) free(post);
    if(postargs) free(postargs);
  }
#endif

  printf(" *** sending HTTP request *** \n\n");

// These two will perform a HTTP request, requesting an access token. 
// it's intended both as test (verify signature) 
// and example code.
#if 1 // POST a request-token request
  request_token_example_post();
#endif
#if 1 // GET a request-token
  request_token_example_get();
#endif

  return (fail?1:0);
}
