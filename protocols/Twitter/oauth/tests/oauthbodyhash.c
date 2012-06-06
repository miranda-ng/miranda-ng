/**
 *  @brief experimental code to sign data uploads
 *  @file oauthbodysign.c
 *  @author Robin Gareus <robin@gareus.org>
 *
 * Copyright 2009 Robin Gareus <robin@gareus.org>
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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <oauth.h>

int my_data_post(char *url, char *data) {
  const char *c_key         = "key"; //< consumer key
  const char *c_secret      = "secret"; //< consumer secret
  char *t_key               = "tkey"; //< access token key
  char *t_secret            = "tsecret"; //< access token secret

  char *postarg = NULL;
  char *req_url = NULL;
  char *reply   = NULL;
  char *bh;
  char *uh;
  char *sig_url;

  bh=oauth_body_hash_data(strlen(data), data);
  uh = oauth_catenc(2, url, bh);
  req_url = oauth_sign_url2(uh, &postarg, OA_HMAC, NULL, c_key, c_secret, t_key, t_secret);
  printf("POST: %s?%s\n", req_url, postarg);
  if (uh) free(uh);

  sig_url = malloc(2+strlen(req_url)+strlen(postarg));
  sprintf(sig_url,"%s?%s",req_url, postarg);
  reply = oauth_post_data(sig_url, data, strlen(data), "Content-Type: application/json");
  if(sig_url) free(sig_url);

  printf("REPLY: %s\n", reply);
  if(reply) free(reply);
  return 0;
}

int main (int argc, char **argv) {
  char *base_url = "http://localhost/oauthtest.php";
  char *teststring="Hello World!";

  /* TEST_BODY_HASH_FILE and TEST_BODY_HASH_DATA are only
   * here as examples and for testing during development.
   *
   * the my_data_post() function above uses oauth_body_hash_data() 
   */

#if defined TEST_BODY_HASH_FILE || defined TEST_BODY_HASH_DATA
  char *bh=NULL;
#endif

#ifdef TEST_BODY_HASH_FILE // example hash file
  char *filename="/tmp/test";
  bh=oauth_body_hash_file(filename);
  if (bh) printf("%s\n", bh);
  if (bh) free(bh);
#endif

#ifdef TEST_BODY_HASH_DATA // example hash data
  bh=oauth_body_hash_data(strlen(teststring), teststring);
  if (bh) printf("%s\n", bh);
  if (bh) free(bh);
#endif

  my_data_post(base_url, teststring);
  return(0);
}
