/**
 *  @brief self-test for liboauth.
 *  @file selftest.c
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <oauth.h>

#include "commontest.h"

int loglevel = 1; //< report each successful test

int main (int argc, char **argv) {
  int fail=0;

  if (loglevel) printf("\n *** Testing query parameter array encoding.\n");

  fail|=test_request("GET", "http://example.com" 
      "?k1=v1"
      "&a1[ak1]=av1"
      "&a1[aa1][aak2]=aav2"
      "&a1[aa1][aak1]=aav1"
      "&k2=v2"
      "&a1[ak2]=av2",
  "GET&http%3A%2F%2Fexample.com%2F&a1%255Baa1%255D%255Baak1%255D%3Daav1%26a1%255Baa1%255D%255Baak2%255D%3Daav2%26a1%255Bak1%255D%3Dav1%26a1%255Bak2%255D%3Dav2%26k1%3Dv1%26k2%3Dv2");

  if (loglevel) printf("\n *** Testing http://tools.ietf.org/html/rfc5849 example.\n");

  fail|=test_request("GET", "http://example.com" 
      "/request?b5=%3D%253D&a3=a&c%40=&a2=r%20b"
      "&c2&a3=2+q"
      "&oauth_consumer_key=9djdj82h48djs9d2"
      "&oauth_token=kkk9d7dh3k39sjv7"
      "&oauth_signature_method=HMAC-SHA1"
      "&oauth_timestamp=137131201"
      "&oauth_nonce=7d8f3e4a"
      "&oauth_signature=djosJKDKJSD8743243%2Fjdk33klY%3D",
   "GET&http%3A%2F%2Fexample.com%2Frequest&a2%3Dr%2520b%26a3%3D2%2520q%26a3%3Da%26b5%3D%253D%25253D%26c%2540%3D%26c2%3D%26oauth_consumer_key%3D9djdj82h48djs9d2%26oauth_nonce%3D7d8f3e4a%26oauth_signature_method%3DHMAC-SHA1%26oauth_timestamp%3D137131201%26oauth_token%3Dkkk9d7dh3k39sjv7");

  if (loglevel) printf("\n *** Testing body hash calculation.\n");

  char *bh;
  const char *teststring="Hello World!";
  bh=oauth_body_hash_data(strlen(teststring), teststring);
  if (bh) { 
    if (strcmp(bh,"oauth_body_hash=Lve95gjOVATpfV8EL5X4nxwjKHE=")) fail|=1;
    free(bh);
  } else { 
    fail|=1;
  }

  // report
  if (fail) {
    printf("\n !!! One or more test cases failed.\n\n");
  } else {
    printf(" *** Test cases verified sucessfully.\n");
  }

  return (fail?1:0);
}
