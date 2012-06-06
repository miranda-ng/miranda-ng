#include <stdio.h>
#include <stdlib.h>
#include <oauth.h>
#include <strings.h>

static void usage (char *program_name) {
  printf(" usage: %s mode url ckey tkey csec tsec\n", program_name);
  exit (1);
}

/**
 * 
 * compile:
 *  gcc -loauth -o oauthsign oauthsign.c
 */
int main (int argc, char **argv) {

  char *url;     //< the url to sign
  char *c_key;    //< consumer key
  char *c_secret;  //< consumer secret
  char *t_key;    //< token key
  char *t_secret ;  //< token secret

  int mode = 0;   //< mode: 0=GET 1=POST

  // TODO: use getopt to parse parameters

  // FIXME: read secrets from stdin - they show up in ps(1)
  // also overwrite memory of secrets before freeing it.
 
  if (argc !=7) usage(argv[0]);

  if ( atoi(argv[1]) > 0 ) mode=atoi(argv[1]);// questionable numeric shortcut
  else if (!strcasecmp(argv[1],"GET"))         mode=1;
  else if (!strcasecmp(argv[1],"POST"))        mode=2;
  else if (!strcasecmp(argv[1],"POSTREQUEST")) mode=4;
  else usage(argv[0]);

  url      = argv[2];
  c_key    = argv[3];
  t_key    = argv[4];
  c_secret = argv[5];
  t_secret = argv[6];

  if (mode==1) { // GET
    char *geturl = NULL;
    geturl = oauth_sign_url2(url, NULL, OA_HMAC, NULL, c_key, c_secret, t_key, t_secret);
    if(geturl) {
      printf("%s\n", geturl);
      free(geturl);
    }
  } else { // POST
    char *postargs = NULL, *post = NULL;
    post = oauth_sign_url2(url, &postargs, OA_HMAC, NULL, c_key, c_secret, t_key, t_secret);
    if (!post || !postargs) {
    	return (1);
    }
    if (mode==2) { // print postargs only
      if (postargs) printf("%s\n", postargs);
    } else if (mode==3) { // print url and postargs
      if (post && postargs) printf("%s\n%s\n", post, postargs);
    } else if (post && postargs) {
      char *reply = oauth_http_post(post,postargs);
      if(reply){
      	//write(STDOUT, reply, strlen(reply))
        printf("%s\n", reply);
      	free(reply);
      }
    }
    if(post) free(post);
    if(postargs) free(postargs);
  }

  return (0);
}
