/**
 *  @brief test and example code for liboauth.
 *  @file commontest.c
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

#ifdef TEST_UNICODE
#include <locale.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <oauth.h>

#include "commontest.h"

extern int loglevel; //< report each successful test

/*
 * test parameter encoding
 */
int test_encoding(char *param, char *expected) {
  int rv=0;
  char *testcase=NULL;
  testcase = oauth_url_escape(param);
  if (strcmp(testcase,expected)) {
    rv=1;
    printf("parameter encoding test for '%s' failed.\n"
           " got: '%s' expected: '%s'\n", param, testcase, expected);
  } 
  else if (loglevel) printf("parameter encoding ok. ('%s')\n", testcase);
  if (testcase) free(testcase);
  return (rv);
}

#ifdef TEST_UNICODE
/*
 * test unicode paramter encoding
 */
int test_uniencoding(wchar_t *src, char *expected) {
  size_t n;
  char *dst;
// check unicode: http://www.thescripts.com/forum/thread223350.html
  const char *encoding = "en_US.UTF-8"; // or try en_US.ISO-8859-1 etc.
  //wchar_t src[] = {0x0080, 0};

  if(setlocale(LC_CTYPE, encoding) == NULL) {
    printf("requested encoding unavailable\n");
    return -1;
  }

  n = wcstombs(NULL, src, 0);
  dst = malloc(n + 1);
  if(dst == NULL) {
    printf("memory allocation failed\n");
    return -2;
  }
  if(wcstombs(dst, src, n + 1) != n) {
    printf("conversion failed\n");
    free(dst);
    return -3;
  }
  return test_encoding(dst, expected);
}
#endif

/*
 * test request normalization
 */
int test_normalize(char *param, char *expected) {
  int rv=2;
  int  i, argc;
  char **argv = NULL;
  char *testcase;

  argc = oauth_split_url_parameters(param, &argv);
  qsort(argv, argc, sizeof(char *), oauth_cmpstringp);
  testcase= oauth_serialize_url(argc,0, argv);

  rv=strcmp(testcase,expected);
  if (rv) {
    printf("parameter normalization test failed for: '%s'.\n"
           " got: '%s' expected: '%s'\n", param, testcase, expected);
  }
  else if (loglevel) printf("parameter normalization ok. ('%s')\n", testcase);
  for (i=0;i<argc;i++) free(argv[i]);
  if (argv) free(argv);
  if (testcase) free(testcase);
  return (rv);
}

/*
 * test request concatenation
 */
int test_request(char *http_method, char *request, char *expected) {
  int rv=2;
  int  i, argc;
  char **argv = NULL;
  char *query, *testcase;

  argc = oauth_split_url_parameters(request, &argv);
  qsort(&argv[1], argc-1, sizeof(char *), oauth_cmpstringp);
  query= oauth_serialize_url(argc,1, argv);
  testcase = oauth_catenc(3, http_method, argv[0], query);

  rv=strcmp(testcase,expected);
  if (rv) {
    printf("request concatenation test failed for: '%s'.\n"
           " got:      '%s'\n expected: '%s'\n", request, testcase, expected);
  }
  else if (loglevel) printf("request concatenation ok.\n");
  for (i=0;i<argc;i++) free(argv[i]);
  if (argv) free(argv);
  if (query) free(query);
  if (testcase) free(testcase);
  return (rv);
}

/*
 * test hmac-sha1 checksum
 */
int test_sha1(char *c_secret, char *t_secret, char *base, char *expected) {
  int rv=0;
  char *okey = oauth_catenc(2, c_secret, t_secret);
  char *b64d = oauth_sign_hmac_sha1(base, okey);
  if (strcmp(b64d,expected)) {
    printf("HMAC-SHA1 invalid. base:'%s' secrets:'%s'\n"
           " got: '%s' expected: '%s'\n", base, okey, b64d, expected);
    rv=1;
  } else if (loglevel) printf("HMAC-SHA1 test sucessful.\n");
  free(b64d);
  free(okey);
  return (rv);
}
