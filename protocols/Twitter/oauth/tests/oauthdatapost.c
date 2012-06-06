/**
 *  @brief experimental code to sign data uploads
 *  @file oauthimageupload.c
 *  @author Robin Gareus <robin@gareus.org>
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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <oauth.h>

#ifdef USE_MMAP
#include <sys/mman.h>
#endif

/** 
 * example oauth body signature and HTTP-POST.
 * WARNING: <b> This is request type is not part of the 
 * oauth core 1.0</b>. 
 *
 * This is an experimental extension.
 */
int oauth_image_post(char *filename, char *url) {
  const char *c_key         = "key"; //< consumer key
  const char *c_secret      = "secret"; //< consumer secret
  char *t_key               = NULL; //< access token key
  char *t_secret            = NULL; //< access token secret

  char *postarg = NULL;
  char *req_url = NULL;
  char *reply   = NULL;

  char *filedata = NULL;
  size_t filelen = 0;

  FILE *F;

  char *okey, *sign;
  char *sig_url;

  // get acces token - see oautexample.c
  t_key    = strdup("key"); //< access token key
  t_secret = strdup("secret"); //< access token secret

  // read raw data to sign and send from file.
  F= fopen(filename, "r");
  if (!F) return 1;
  fseek(F, 0L, SEEK_END);
  filelen= ftell(F);
  rewind(F);

  #ifdef USE_MMAP
  filedata=mmap(NULL,filelen,PROT_READ,MAP_SHARED,fileno(F),0L);
  #else 
  filedata=malloc(filelen*sizeof(char));
  if (filelen != fread(filedata,sizeof(char), filelen, F)) {
  	fclose(F);
  	return 2;
  }
  fclose(F);
  #endif

  // sign the body
  okey = oauth_catenc(2, c_secret, t_secret);
  sign = oauth_sign_hmac_sha1_raw(filedata,filelen,okey,strlen(okey));
  free(okey);
  sig_url = malloc(63+strlen(url)+strlen(sign));
  sprintf(sig_url,"%s&xoauth_body_signature=%s&xoauth_body_signature_method=HMAC_SHA1",url, sign);

  // sign a POST request
  req_url = oauth_sign_url2(sig_url, &postarg, OA_HMAC, NULL, c_key, c_secret, t_key, t_secret);
  free(sig_url);

  // append the oauth [post] parameters to the request-URL!!
  sig_url = malloc(2+strlen(req_url)+strlen(postarg));
  sprintf(sig_url,"%s?%s",req_url, postarg);

  printf("POST:'%s'\n",sig_url);
  //reply = oauth_post_file(sig_url,filename,filelen,"Content-Type: image/jpeg;");
  printf("reply:'%s'\n",reply);

  if(req_url) free(req_url);
  if(postarg) free(postarg);
  if(reply) free(reply);
  if(t_key) free(t_key);
  if(t_secret) free(t_secret);

  #ifdef USE_MMAP
  munmap(filedata,filelen);
  fclose(F);
  #else 
  if(filedata) free(filedata);
  #endif
  return(0);
}


/**
 * Main Test and Example Code.
 * 
 * compile:
 *  gcc -lssl -loauth -o oauthdatapost oauthdatapost.c
 */

int main (int argc, char **argv) {
  char *base_url = "http://mms06.test.mediamatic.nl";
  char *filename = "/tmp/test.jpg";
  int anyid = 18704;
  char *title = "test";
  char *url;

  if (argc>4) base_url = argv[4];
  if (argc>3) title = argv[3];
  if (argc>2) anyid = atoi(argv[2]);
  if (argc>1) filename = argv[1];

  // TODO check if file exists; also read oauth-params from args or file

  // anyMeta.nl image-post module URL
  url = malloc(1024*sizeof(char));
  if (anyid<1 && !title) 
    sprintf(url,"%s/module/ImagePost/",base_url);
  else if (anyid>0 && !title) 
    sprintf(url,"%s/module/ImagePost/%i?echoid=1",base_url,anyid);
  else if (anyid<1 && title) {
    char *tp = oauth_url_escape(title);
    sprintf(url,"%s/module/ImagePost/?title=%s",base_url,tp);
    free(tp);
  }
  else if (anyid>0 && title) {
    char *tp = oauth_url_escape(title);
    sprintf(url,"%s/module/ImagePost/%i?echoid=1&title=%s",base_url,anyid,tp);
    free(tp);
  }

  // doit
  switch(oauth_image_post(filename, url)) {
    case 0:
      printf("request ok.\n");
      break;
    default:
      printf("upload failed.\n");
      break;
  }
  return(0);
}
