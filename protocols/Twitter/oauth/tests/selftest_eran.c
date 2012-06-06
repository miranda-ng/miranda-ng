/**
 *  @brief self-test for liboauth.
 *  @file selftest.c
 *  @author Robin Gareus <robin@gareus.org>
 *
 * This code contains examples provided by Eran Hammer-Lahav 
 * on the oauth.net mailing list.
 *
 * Copyright 2008 Robin Gareus <robin@gareus.org>
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

#ifdef TEST_UNICODE
#include <locale.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <oauth.h>

#include "commontest.h"

int loglevel = 1; //< report each successful test

int main (int argc, char **argv) {
  int fail=0;
  char *tmptst;

  if (loglevel) printf("\n *** testing liboauth against Eran's Test cases ***\n http://groups.google.com/group/oauth/browse_frm/thread/243f4da439fd1f51?hl=en\n");

  // Eran's test-cases - http://groups.google.com/group/oauth/browse_frm/thread/243f4da439fd1f51?hl=en
  fail|=test_encoding("1234=asdf=4567","1234%3Dasdf%3D4567");
  fail|=test_encoding("asdf-4354=asew-5698","asdf-4354%3Dasew-5698");
  fail|=test_encoding("erks823*43=asd&123ls%23","erks823%2A43%3Dasd%26123ls%2523");
  fail|=test_encoding("dis9$#$Js009%==","dis9%24%23%24Js009%25%3D%3D");
  fail|=test_encoding("3jd834jd9","3jd834jd9");
  fail|=test_encoding("12303202302","12303202302");
  fail|=test_encoding("taken with a 30% orange filter","taken%20with%20a%2030%25%20orange%20filter");
  fail|=test_encoding("mountain & water view","mountain%20%26%20water%20view");

  fail|=test_request("GET", "http://example.com:80/photo" "?" 
      "oauth_version=1.0"
      "&oauth_consumer_key=1234=asdf=4567"
      "&oauth_timestamp=12303202302"
      "&oauth_nonce=3jd834jd9"
      "&oauth_token=asdf-4354=asew-5698"
      "&oauth_signature_method=HMAC-SHA1"
      "&title=taken with a 30% orange filter"
      "&file=mountain \001 water view"
      "&format=jpeg"
      "&include=date"
      "&include=aperture",
  "GET&http%3A%2F%2Fexample.com%2Fphoto&file%3Dmountain%2520%2526%2520water%2520view%26format%3Djpeg%26include%3Daperture%26include%3Ddate%26oauth_consumer_key%3D1234%253Dasdf%253D4567%26oauth_nonce%3D3jd834jd9%26oauth_signature_method%3DHMAC-SHA1%26oauth_timestamp%3D12303202302%26oauth_token%3Dasdf-4354%253Dasew-5698%26oauth_version%3D1.0%26title%3Dtaken%2520with%2520a%252030%2525%2520orange%2520filter" );

  tmptst = oauth_sign_url2(
      "http://example.com:80/photo" "?" 
      "oauth_version=1.0"
      "&oauth_timestamp=12303202302"
      "&oauth_nonce=3jd834jd9"
      "&title=taken with a 30% orange filter"
      "&file=mountain \001 water view"
      "&format=jpeg"
      "&include=date"
      "&include=aperture",
   NULL, OA_HMAC, NULL, "1234=asdf=4567", "erks823*43=asd&123ls%23", "asdf-4354=asew-5698", "dis9$#$Js009%==");
  if (strcmp(tmptst,"http://example.com/photo?file=mountain%20%26%20water%20view&format=jpeg&include=aperture&include=date&oauth_consumer_key=1234%3Dasdf%3D4567&oauth_nonce=3jd834jd9&oauth_signature_method=HMAC-SHA1&oauth_timestamp=12303202302&oauth_token=asdf-4354%3Dasew-5698&oauth_version=1.0&title=taken%20with%20a%2030%25%20orange%20filter&oauth_signature=jMdUSR1vOr3SzNv3gZ5DDDuGirA%3D")) {
  	printf(" got '%s'\n expected: '%s'\n",tmptst, "http://example.com/photo?file=mountain%20%26%20water%20view&format=jpeg&include=aperture&include=date&oauth_consumer_key=1234%3Dasdf%3D4567&oauth_nonce=3jd834jd9&oauth_signature_method=HMAC-SHA1&oauth_timestamp=12303202302&oauth_token=asdf-4354%3Dasew-5698&oauth_version=1.0&title=taken%20with%20a%2030%25%20orange%20filter&oauth_signature=jMdUSR1vOr3SzNv3gZ5DDDuGirA%3D");
	fail|=1;
  } else if (loglevel) printf("request signature ok.\n");
  if(tmptst) free(tmptst);

  // report
  if (fail) {
    printf("\n !!! One or more of Eran's Test Cases failed.\n\n");
  } else {
    printf(" *** Eran's Test-Cases verified sucessfully.\n");
  }

  return (fail?1:0);
}
