/**
 *  @brief self-test for liboauth.
 *  @file selftest.c
 *  @author Robin Gareus <robin@gareus.org>
 *
 * Copyright 2007, 2008 Robin Gareus <robin@gareus.org>
 *
 * This code contains examples from http://wiki.oauth.net/ may they be blessed.
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
  char *b64d;
  char *testurl, *testkey;
 #ifdef TEST_UNICODE
  wchar_t src[] = {0x000A, 0};
 #endif

  if (loglevel) printf("\n *** testing liboauth against http://wiki.oauth.net/TestCases (july 2008) ***\n");

  // http://wiki.oauth.net/TestCases
  fail|=test_encoding("abcABC123","abcABC123");
  fail|=test_encoding("-._~","-._~");
  fail|=test_encoding("%","%25");
  fail|=test_encoding("+","%2B");
  fail|=test_encoding("&=*","%26%3D%2A");

 #ifdef TEST_UNICODE
  src[0] = 0x000A; fail|=test_uniencoding(src,"%0A");
  src[0] = 0x0020; fail|=test_uniencoding(src,"%20");
  src[0] = 0x007F; fail|=test_uniencoding(src,"%7F");
  src[0] = 0x0080; fail|=test_uniencoding(src,"%C2%80");
  src[0] = 0x3001; fail|=test_uniencoding(src,"%E3%80%81");
 #endif

  fail|=test_normalize("name", "name=");
  fail|=test_normalize("a=b", "a=b");
  fail|=test_normalize("a=b&c=d", "a=b&c=d");
  fail|=test_normalize("a=x!y&a=x+y", "a=x%20y&a=x%21y");
  fail|=test_normalize("x!y=a&x=a", "x=a&x%21y=a");

  fail|=test_request("GET", "http://example.com/" "?" 
      "n=v",
  // expect:
      "GET&http%3A%2F%2Fexample.com%2F&n%3Dv");

  fail|=test_request("GET", "http://example.com" "?" 
      "n=v",
  // expect:
      "GET&http%3A%2F%2Fexample.com%2F&n%3Dv");

  fail|=test_request("POST", "https://photos.example.net/request_token" "?" 
      "oauth_version=1.0"
      "&oauth_consumer_key=dpf43f3p2l4k3l03"
      "&oauth_timestamp=1191242090"
      "&oauth_nonce=hsu94j3884jdopsl"
      "&oauth_signature_method=PLAINTEXT"
      "&oauth_signature=ignored",
  // expect:
      "POST&https%3A%2F%2Fphotos.example.net%2Frequest_token&oauth_consumer_key%3Ddpf43f3p2l4k3l03%26oauth_nonce%3Dhsu94j3884jdopsl%26oauth_signature_method%3DPLAINTEXT%26oauth_timestamp%3D1191242090%26oauth_version%3D1.0");

  fail|=test_request("GET", "http://photos.example.net/photos" "?" 
      "file=vacation.jpg&size=original"
      "&oauth_version=1.0"
      "&oauth_consumer_key=dpf43f3p2l4k3l03"
      "&oauth_token=nnch734d00sl2jdk"
      "&oauth_timestamp=1191242096"
      "&oauth_nonce=kllo9940pd9333jh"
      "&oauth_signature=ignored"
      "&oauth_signature_method=HMAC-SHA1",
  // expect:
      "GET&http%3A%2F%2Fphotos.example.net%2Fphotos&file%3Dvacation.jpg%26oauth_consumer_key%3Ddpf43f3p2l4k3l03%26oauth_nonce%3Dkllo9940pd9333jh%26oauth_signature_method%3DHMAC-SHA1%26oauth_timestamp%3D1191242096%26oauth_token%3Dnnch734d00sl2jdk%26oauth_version%3D1.0%26size%3Doriginal");

  fail|=test_sha1("cs","","bs","egQqG5AJep5sJ7anhXju1unge2I=");
  fail|=test_sha1("cs","ts","bs","VZVjXceV7JgPq/dOTnNmEfO0Fv8=");
  fail|=test_sha1("kd94hf93k423kf44","pfkkdhi9sl3r4s00","GET&http%3A%2F%2Fphotos.example.net%2Fphotos&file%3Dvacation.jpg%26oauth_consumer_key%3Ddpf43f3p2l4k3l03%26oauth_nonce%3Dkllo9940pd9333jh%26oauth_signature_method%3DHMAC-SHA1%26oauth_timestamp%3D1191242096%26oauth_token%3Dnnch734d00sl2jdk%26oauth_version%3D1.0%26size%3Doriginal","tR3+Ty81lMeYAr/Fid0kMTYa/WM=");

  // HMAC-SHA1 selftest.
  // see http://oauth.net/core/1.0/#anchor25 
  testurl = "GET&http%3A%2F%2Fphotos.example.net%2Fphotos&file%3D"
      "vacation.jpg%26oauth_consumer_key%3Ddpf43f3p2l4k3l03%26oauth_nonce"
      "%3Dkllo9940pd9333jh%26oauth_signature_method%3DHMAC-SHA1%26o"
      "auth_timestamp%3D1191242096%26oauth_token%3Dnnch734d00sl2jdk"
      "%26oauth_version%3D1.0%26size%3Doriginal";

  testkey = "kd94hf93k423kf44&pfkkdhi9sl3r4s00";
  b64d = oauth_sign_hmac_sha1(testurl , testkey);
  if (strcmp(b64d,"tR3+Ty81lMeYAr/Fid0kMTYa/WM=")) {
    printf("HMAC-SHA1 signature test failed.\n");
    fail|=1;
  } else if (loglevel)
    printf("HMAC-SHA1 signature test successful.\n");
  free(b64d);

  // rsa-signature based on http://wiki.oauth.net/TestCases example
  b64d = oauth_sign_rsa_sha1(
    "GET&http%3A%2F%2Fphotos.example.net%2Fphotos&file%3Dvacaction.jpg%26oauth_consumer_key%3Ddpf43f3p2l4k3l03%26oauth_nonce%3D13917289812797014437%26oauth_signature_method%3DRSA-SHA1%26oauth_timestamp%3D1196666512%26oauth_version%3D1.0%26size%3Doriginal",

    "-----BEGIN PRIVATE KEY-----\n"
    "MIICdgIBADANBgkqhkiG9w0BAQEFAASCAmAwggJcAgEAAoGBALRiMLAh9iimur8V\n"
    "A7qVvdqxevEuUkW4K+2KdMXmnQbG9Aa7k7eBjK1S+0LYmVjPKlJGNXHDGuy5Fw/d\n"
    "7rjVJ0BLB+ubPK8iA/Tw3hLQgXMRRGRXXCn8ikfuQfjUS1uZSatdLB81mydBETlJ\n"
    "hI6GH4twrbDJCR2Bwy/XWXgqgGRzAgMBAAECgYBYWVtleUzavkbrPjy0T5FMou8H\n"
    "X9u2AC2ry8vD/l7cqedtwMPp9k7TubgNFo+NGvKsl2ynyprOZR1xjQ7WgrgVB+mm\n"
    "uScOM/5HVceFuGRDhYTCObE+y1kxRloNYXnx3ei1zbeYLPCHdhxRYW7T0qcynNmw\n"
    "rn05/KO2RLjgQNalsQJBANeA3Q4Nugqy4QBUCEC09SqylT2K9FrrItqL2QKc9v0Z\n"
    "zO2uwllCbg0dwpVuYPYXYvikNHHg+aCWF+VXsb9rpPsCQQDWR9TT4ORdzoj+Nccn\n"
    "qkMsDmzt0EfNaAOwHOmVJ2RVBspPcxt5iN4HI7HNeG6U5YsFBb+/GZbgfBT3kpNG\n"
    "WPTpAkBI+gFhjfJvRw38n3g/+UeAkwMI2TJQS4n8+hid0uus3/zOjDySH3XHCUno\n"
    "cn1xOJAyZODBo47E+67R4jV1/gzbAkEAklJaspRPXP877NssM5nAZMU0/O/NGCZ+\n"
    "3jPgDUno6WbJn5cqm8MqWhW1xGkImgRk+fkDBquiq4gPiT898jusgQJAd5Zrr6Q8\n"
    "AO/0isr/3aa6O6NLQxISLKcPDk2NOccAfS/xOtfOz4sJYM3+Bs4Io9+dZGSDCA54\n"
    "Lw03eHTNQghS0A==\n"
    "-----END PRIVATE KEY-----");

  if (strcmp(b64d,"jvTp/wX1TYtByB1m+Pbyo0lnCOLIsyGCH7wke8AUs3BpnwZJtAuEJkvQL2/9n4s5wUmUl4aCI4BwpraNx4RtEXMe5qg5T1LVTGliMRpKasKsW//e+RinhejgCuzoH26dyF8iY2ZZ/5D1ilgeijhV/vBka5twt399mXwaYdCwFYE=")) {
    printf("RSA-SHA1 signature test failed.\n");
    fail|=1;
  } else if (loglevel)
    printf("RSA-SHA1 signature test successful.\n");
  free(b64d);

  if (oauth_verify_rsa_sha1(
    "GET&http%3A%2F%2Fphotos.example.net%2Fphotos&file%3Dvacaction.jpg%26oauth_consumer_key%3Ddpf43f3p2l4k3l03%26oauth_nonce%3D13917289812797014437%26oauth_signature_method%3DRSA-SHA1%26oauth_timestamp%3D1196666512%26oauth_version%3D1.0%26size%3Doriginal",

    "-----BEGIN CERTIFICATE-----\n"
    "MIIBpjCCAQ+gAwIBAgIBATANBgkqhkiG9w0BAQUFADAZMRcwFQYDVQQDDA5UZXN0\n"
    "IFByaW5jaXBhbDAeFw03MDAxMDEwODAwMDBaFw0zODEyMzEwODAwMDBaMBkxFzAV\n"
    "BgNVBAMMDlRlc3QgUHJpbmNpcGFsMIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKB\n"
    "gQC0YjCwIfYoprq/FQO6lb3asXrxLlJFuCvtinTF5p0GxvQGu5O3gYytUvtC2JlY\n"
    "zypSRjVxwxrsuRcP3e641SdASwfrmzyvIgP08N4S0IFzEURkV1wp/IpH7kH41Etb\n"
    "mUmrXSwfNZsnQRE5SYSOhh+LcK2wyQkdgcMv11l4KoBkcwIDAQABMA0GCSqGSIb3\n"
    "DQEBBQUAA4GBAGZLPEuJ5SiJ2ryq+CmEGOXfvlTtEL2nuGtr9PewxkgnOjZpUy+d\n"
    "4TvuXJbNQc8f4AMWL/tO9w0Fk80rWKp9ea8/df4qMq5qlFWlx6yOLQxumNOmECKb\n"
    "WpkUQDIDJEoFUzKMVuJf4KO/FJ345+BNLGgbJ6WujreoM1X/gYfdnJ/J\n"
    "-----END CERTIFICATE-----\n", 
    "jvTp/wX1TYtByB1m+Pbyo0lnCOLIsyGCH7wke8AUs3BpnwZJtAuEJkvQL2/9n4s5wUmUl4aCI4BwpraNx4RtEXMe5qg5T1LVTGliMRpKasKsW//e+RinhejgCuzoH26dyF8iY2ZZ/5D1ilgeijhV/vBka5twt399mXwaYdCwFYE=")
      != 1) {
    printf("RSA-SHA1 verify-signature test failed.\n");
    fail|=1;
  } else if (loglevel)
    printf("RSA-SHA1 verify-signature test successful.\n");

  // report
  if (fail) {
    printf("\n !!! One or more tests from http://wiki.oauth.net/TestCases failed.\n\n");
  } else {
    printf(" *** http://wiki.oauth.net/TestCases verified sucessfully.\n");
  }

  return (fail?1:0);
}
