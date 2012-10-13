/*
 * OAuth http functions in POSIX-C.
 *
 * Copyright 2007, 2008, 2009, 2010 Robin Gareus <robin@gareus.org>
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
 *
 */
#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#ifdef WIN32
#  define snprintf _snprintf
#endif

#include "xmalloc.h"
#include "oauth.h"

#define OAUTH_USER_AGENT "liboauth-agent/" VERSION

#ifdef HAVE_CURL /* HTTP requests via libcurl */
#include <curl/curl.h>
#include <sys/stat.h>

# define GLOBAL_CURL_ENVIROMENT_OPTIONS \
  if (getenv("CURLOPT_PROXYAUTH")){ \
    curl_easy_setopt(curl, CURLOPT_PROXYAUTH, CURLAUTH_ANY); \
  } \
  if (getenv("CURLOPT_SSL_VERIFYPEER")){ \
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, (long) atol(getenv("CURLOPT_SSL_VERIFYPEER")) ); \
  } \
  if (getenv("CURLOPT_CAINFO")){ \
    curl_easy_setopt(curl, CURLOPT_CAINFO, getenv("CURLOPT_CAINFO") ); \
  } \
  if (getenv("CURLOPT_FOLLOWLOCATION")){ \
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, (long) atol(getenv("CURLOPT_FOLLOWLOCATION")) ); \
  } \
  if (getenv("CURLOPT_FAILONERROR")){ \
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, (long) atol(getenv("CURLOPT_FAILONERROR")) ); \
  }

struct MemoryStruct {
  char *data;
  size_t size; //< bytes remaining (r), bytes accumulated (w)

  size_t start_size; //< only used with ..AndCall()
  void (*callback)(void*,int,size_t,size_t); //< only used with ..AndCall()
  void *callback_data; //< only used with ..AndCall()
};

static size_t
WriteMemoryCallback(void *ptr, size_t size, size_t nmemb, void *data) {
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)data;

  mem->data = (char *)xrealloc(mem->data, mem->size + realsize + 1);
  if (mem->data) {
    memcpy(&(mem->data[mem->size]), ptr, realsize);
    mem->size += realsize;
    mem->data[mem->size] = 0;
  }
  return realsize;
}

static size_t
ReadMemoryCallback(void *ptr, size_t size, size_t nmemb, void *data) {
  struct MemoryStruct *mem = (struct MemoryStruct *)data;
  size_t realsize = size * nmemb;
  if (realsize > mem->size) realsize = mem->size;
  memcpy(ptr, mem->data, realsize);
  mem->size -= realsize;
  mem->data += realsize;
  return realsize;
}

static size_t
WriteMemoryCallbackAndCall(void *ptr, size_t size, size_t nmemb, void *data) {
  struct MemoryStruct *mem = (struct MemoryStruct *)data;
  size_t ret=WriteMemoryCallback(ptr,size,nmemb,data);
  mem->callback(mem->callback_data,0,mem->size,mem->size);
  return ret;
}

static size_t
ReadMemoryCallbackAndCall(void *ptr, size_t size, size_t nmemb, void *data) {
  struct MemoryStruct *mem = (struct MemoryStruct *)data;
  size_t ret=ReadMemoryCallback(ptr,size,nmemb,data);
  mem->callback(mem->callback_data,1,mem->start_size-mem->size,mem->start_size);
  return ret;
}

/**
 * cURL http post function.
 * the returned string (if not NULL) needs to be freed by the caller
 *
 * @param u url to retrieve
 * @param p post parameters 
 * @param customheader specify custom HTTP header (or NULL for none)
 * @return returned HTTP
 */
char *oauth_curl_post (const char *u, const char *p, const char *customheader) {
  CURL *curl;
  CURLcode res;
  struct curl_slist *slist=NULL;

  struct MemoryStruct chunk;
  chunk.data=NULL;
  chunk.size = 0;

  curl = curl_easy_init();
  if(!curl) return NULL;
  curl_easy_setopt(curl, CURLOPT_URL, u);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, p);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
  if (customheader) {
    slist = curl_slist_append(slist, customheader);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist); 
  }
  curl_easy_setopt(curl, CURLOPT_USERAGENT, OAUTH_USER_AGENT);
#ifdef OAUTH_CURL_TIMEOUT  
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, OAUTH_CURL_TIMEOUT);
  curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
#endif
  GLOBAL_CURL_ENVIROMENT_OPTIONS;
  res = curl_easy_perform(curl);
  curl_slist_free_all(slist);
  if (res) {
    return NULL;
  }

  curl_easy_cleanup(curl);
  return (chunk.data);
}

/**
 * cURL http get function.
 * the returned string (if not NULL) needs to be freed by the caller
 *
 * @param u url to retrieve
 * @param q optional query parameters 
 * @param customheader specify custom HTTP header (or NULL for none)
 * @return returned HTTP
 */
char *oauth_curl_get (const char *u, const char *q, const char *customheader) {
  CURL *curl;
  CURLcode res;
  struct curl_slist *slist=NULL;
  char *t1=NULL;
  struct MemoryStruct chunk;

  if (q) {
    t1=(char*)xmalloc(sizeof(char)*(strlen(u)+strlen(q)+2));
    strcpy(t1,u); strcat(t1,"?"); strcat(t1,q);
  }

  chunk.data=NULL;
  chunk.size = 0;

  curl = curl_easy_init();
  if(!curl) return NULL;
  curl_easy_setopt(curl, CURLOPT_URL, q?t1:u);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
  if (customheader) {
    slist = curl_slist_append(slist, customheader);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist); 
  }
#if 0 // TODO - support request methods..
  if (0) 
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "HEAD");
  else if (0) 
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
#endif
  curl_easy_setopt(curl, CURLOPT_USERAGENT, OAUTH_USER_AGENT);
#ifdef OAUTH_CURL_TIMEOUT  
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, OAUTH_CURL_TIMEOUT);
  curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
#endif
  GLOBAL_CURL_ENVIROMENT_OPTIONS;
  res = curl_easy_perform(curl);
  curl_slist_free_all(slist);
  if (q) free(t1);
  curl_easy_cleanup(curl);

  if (res) {
    return NULL;
  }
  return (chunk.data);
}

/**
 * cURL http post raw data from file.
 * the returned string needs to be freed by the caller
 *
 * @param u url to retrieve
 * @param fn filename of the file to post along
 * @param len length of the file in bytes. set to '0' for autodetection
 * @param customheader specify custom HTTP header (or NULL for default)
 * @return returned HTTP or NULL on error
 */
char *oauth_curl_post_file (const char *u, const char *fn, size_t len, const char *customheader) {
  CURL *curl;
  CURLcode res;
  struct curl_slist *slist=NULL;
  struct MemoryStruct chunk;
  FILE *f;

  chunk.data=NULL;
  chunk.size=0;

  if (customheader)
    slist = curl_slist_append(slist, customheader);
  else
    slist = curl_slist_append(slist, "Content-Type: image/jpeg;");

  if (!len) {
    struct stat statbuf;
    if (stat(fn, &statbuf) == -1) return(NULL);
    len = statbuf.st_size;
  }

  f = fopen(fn,"r");
  if (!f) return NULL;

  curl = curl_easy_init();
  if(!curl) return NULL;
  curl_easy_setopt(curl, CURLOPT_URL, u);
  curl_easy_setopt(curl, CURLOPT_POST, 1);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, len);
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist); 
  curl_easy_setopt(curl, CURLOPT_READDATA, f);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
  curl_easy_setopt(curl, CURLOPT_USERAGENT, OAUTH_USER_AGENT);
#ifdef OAUTH_CURL_TIMEOUT  
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, OAUTH_CURL_TIMEOUT);
  curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
#endif
  GLOBAL_CURL_ENVIROMENT_OPTIONS;
  res = curl_easy_perform(curl);
  curl_slist_free_all(slist);
  if (res) {
    // error
    return NULL;
  }
  fclose(f);

  curl_easy_cleanup(curl);
  return (chunk.data);
}

/**
 * http send raw data, with callback.
 * the returned string needs to be freed by the caller
 *
 * more documentation in oauth.h
 *
 * @param u url to retrieve
 * @param data data to post along
 * @param len length of the file in bytes. set to '0' for autodetection
 * @param customheader specify custom HTTP header (or NULL for default)
 * @param callback specify the callback function
 * @param callback_data specify data to pass to the callback function
 * @return returned HTTP reply or NULL on error
 */
char *oauth_curl_send_data_with_callback (const char *u, const char *data, size_t len, const char *customheader, void (*callback)(void*,int,size_t,size_t), void *callback_data, const char *httpMethod) {
  CURL *curl;
  CURLcode res;
  struct curl_slist *slist=NULL;
  struct MemoryStruct chunk;
  struct MemoryStruct rdnfo;
  
  chunk.data=NULL;
  chunk.size=0;
  chunk.start_size=0;
  chunk.callback=callback;
  chunk.callback_data=callback_data;
  rdnfo.data=(char *)data;
  rdnfo.size=len;
  rdnfo.start_size=len;
  rdnfo.callback=callback;
  rdnfo.callback_data=callback_data;

  if (customheader)
    slist = curl_slist_append(slist, customheader);
  else
    slist = curl_slist_append(slist, "Content-Type: image/jpeg;");
  
  curl = curl_easy_init();
  if(!curl) return NULL;
  curl_easy_setopt(curl, CURLOPT_URL, u);
  curl_easy_setopt(curl, CURLOPT_POST, 1);
  if (httpMethod) curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, httpMethod);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, len);
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist); 
  curl_easy_setopt(curl, CURLOPT_READDATA, (void *)&rdnfo);
  if (callback)
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, ReadMemoryCallbackAndCall);
  else
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, ReadMemoryCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
  if (callback)
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallbackAndCall);
  else 
     curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
  curl_easy_setopt(curl, CURLOPT_USERAGENT, OAUTH_USER_AGENT);
#ifdef OAUTH_CURL_TIMEOUT  
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, OAUTH_CURL_TIMEOUT);
  curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
#endif
  GLOBAL_CURL_ENVIROMENT_OPTIONS;
  res = curl_easy_perform(curl);
  curl_slist_free_all(slist);
  if (res) {
    // error
    return NULL;
  }
  
  curl_easy_cleanup(curl);
  return (chunk.data);
}

/**
 * http post raw data.
 * the returned string needs to be freed by the caller
 *
 * more documentation in oauth.h
 *
 * @param u url to retrieve
 * @param data data to post along
 * @param len length of the file in bytes. set to '0' for autodetection
 * @param customheader specify custom HTTP header (or NULL for default)
 * @return returned HTTP reply or NULL on error
 */
char *oauth_curl_post_data(const char *u, const char *data, size_t len, const char *customheader) {
  return oauth_curl_send_data_with_callback(u, data, len, customheader, NULL, NULL, NULL);
}

char *oauth_curl_send_data (const char *u, const char *data, size_t len, const char *customheader, const char *httpMethod) {
  return oauth_curl_send_data_with_callback(u, data, len, customheader, NULL, NULL, httpMethod);
}

char *oauth_curl_post_data_with_callback (const char *u, const char *data, size_t len, const char *customheader, void (*callback)(void*,int,size_t,size_t), void *callback_data) {
  return oauth_curl_send_data_with_callback(u, data, len, customheader, callback, callback_data, NULL);
}

#endif // libcURL.


#ifdef HAVE_SHELL_CURL /* HTTP requests via command-line curl  */

// command line presets and ENV variable name
#define _OAUTH_ENV_HTTPCMD "OAUTH_HTTP_CMD"
#define _OAUTH_ENV_HTTPGET "OAUTH_HTTP_GET_CMD"

#ifdef OAUTH_CURL_TIMEOUT  

#define cpxstr(s) cpstr(s)
#define cpstr(s) #s

#ifndef _OAUTH_DEF_HTTPCMD
# define _OAUTH_DEF_HTTPCMD "curl -sA '"OAUTH_USER_AGENT"' -m "cpxstr(OAUTH_CURL_TIMEOUT)" -d '%p' '%u' "
//alternative: "wget -q -U 'liboauth-agent/0.1' --post-data='%p' '%u' "
#endif

#ifndef _OAUTH_DEF_HTTPGET 
# define _OAUTH_DEF_HTTPGET "curl -sA '"OAUTH_USER_AGENT"' -m "cpxstr(OAUTH_CURL_TIMEOUT)" '%u' "
//alternative: "wget -q -U 'liboauth-agent/0.1' '%u' "
#endif

#else // no timeout

#ifndef _OAUTH_DEF_HTTPCMD
# define _OAUTH_DEF_HTTPCMD "curl -sA '"OAUTH_USER_AGENT"' -d '%p' '%u' "
//alternative: "wget -q -U 'liboauth-agent/0.1' --post-data='%p' '%u' "
#endif

#ifndef _OAUTH_DEF_HTTPGET 
# define _OAUTH_DEF_HTTPGET "curl -sA '"OAUTH_USER_AGENT"' '%u' "
//alternative: "wget -q -U 'liboauth-agent/0.1' '%u' "
#endif

#endif

#include <stdio.h>

/**
 *  escape URL for use in String Quotes (aka shell single quotes).
 *  the returned string needs to be free()d by the calling function
 *
 * WARNING: this function only escapes single-quotes (')
 *
 *
 * RFC2396 defines the following 
 *  reserved    = ";" | "/" | "?" | ":" | "@" | "&" | "=" | "+" | "$" | ","
 *  besides alphanum the following are allowed as unreserved:
 *  mark        = "-" | "_" | "." | "!" | "~" | "*" | "'" | "(" | ")"
 *
 *  checking  `echo '-_.!~*()'` it seems we
 *  just need to escape the tick (') itself from "'" to "'\''"
 *
 *  In C shell, the "!" character may need a backslash before it. 
 *  It depends on the characters next to it. If it is surrounded by spaces,
 *  you don't need to use a backslash. 
 *  (here: we'd always need to escape it for c shell)
 * @todo: escape  '!' for c-shell curl/wget commandlines
 *
 * @param cmd URI string or parameter to be escaped
 * @return escaped parameter
 */
char *oauth_escape_shell (const char *cmd) {
  char *esc = xstrdup(cmd);
  char *tmp = esc;
  int idx;
  while ((tmp=strchr(tmp,'\''))) { 
    idx = tmp-esc;
    esc=(char*)xrealloc(esc,(strlen(esc)+5)*sizeof(char));
    memmove(esc+idx+4,esc+idx+1, strlen(esc+idx));
    esc[idx+1]='\\'; esc[idx+2]='\''; esc[idx+3]='\'';
    tmp=esc+(idx+4);
  }

// TODO escape '!' if CSHELL ?!

  return esc;
}

/**
 * execute command via shell and return it's output.
 * This is used to call 'curl' or 'wget'.
 * the command is uses <em>as is</em> and needs to be propery escaped.
 *
 * @param cmd the commandline to execute
 * @return stdout string that needs to be freed or NULL if there's no output
 */
char *oauth_exec_shell (const char *cmd) {
#ifdef DEBUG_OAUTH
  printf("DEBUG: executing: %s\n",cmd);
#endif
  FILE *in = popen (cmd, "r");
  size_t len = 0;
  size_t alloc = 0;
  char *data = NULL;
  int rcv = 1;
  while (in && rcv > 0 && !feof(in)) {
    alloc +=1024;
    data = (char*)xrealloc(data, alloc * sizeof(char));
    rcv = fread(data + (alloc-1024), sizeof(char), 1024, in);
    len += rcv;
  }
  pclose(in);
#ifdef DEBUG_OAUTH
  printf("DEBUG: read %i bytes\n",len);
#endif
  data[len]=0;
#ifdef DEBUG_OAUTH
  if (data) printf("DEBUG: return: %s\n",data);
  else printf("DEBUG: NULL data\n");
#endif
  return (data);
}

/**
 * send POST via a command line HTTP client,  wait for it to finish
 * and return the content of the reply. requires a command-line HTTP client
 *
 * see \ref  oauth_http_post
 *
 * @param u url to query
 * @param p postargs to send along with the HTTP request.
 * @return  In case of an error NULL is returned; otherwise a pointer to the
 * replied content from HTTP server. latter needs to be freed by caller.
 */
char *oauth_exec_post (const char *u, const char *p) {
  char cmd[BUFSIZ];
  char *t1,*t2;
  char *cmdtpl = getenv(_OAUTH_ENV_HTTPCMD);
  if (!cmdtpl) cmdtpl = xstrdup (_OAUTH_DEF_HTTPCMD);
  else cmdtpl = xstrdup (cmdtpl); // clone getenv() string.

  // add URL and post param - error if no '%p' or '%u' present in definition
  t1=strstr(cmdtpl, "%p");
  t2=strstr(cmdtpl, "%u");
  if (!t1 || !t2) {
    fprintf(stderr, "\nliboauth: invalid HTTP command. set the '%s' environment variable.\n\n",_OAUTH_ENV_HTTPCMD);
    return(NULL);
  }
  // TODO: check if there are exactly two '%' in cmdtpl
  *(++t1)= 's'; *(++t2)= 's';
  if (t1>t2) {
    t1=oauth_escape_shell(u);
    t2=oauth_escape_shell(p);
  } else {
    t1=oauth_escape_shell(p);
    t2=oauth_escape_shell(u);
  }
  snprintf(cmd, BUFSIZ, cmdtpl, t1, t2);
  free(cmdtpl);
  free(t1); free(t2);
  return oauth_exec_shell(cmd);
}

/**
 * send GET via a command line HTTP client
 * and return the content of the reply..
 * requires a command-line HTTP client.
 * 
 * Note: u and q are just concatenated with a '?' in between unless q is NULL. in which case only u will be used.
 *
 * see \ref  oauth_http_get
 *
 * @param u base url to get
 * @param q query string to send along with the HTTP request.
 * @return  In case of an error NULL is returned; otherwise a pointer to the
 * replied content from HTTP server. latter needs to be freed by caller.
 */
char *oauth_exec_get (const char *u, const char *q) {
  char cmd[BUFSIZ];
  char *cmdtpl, *t1, *e1;

  if (!u) return (NULL);

  cmdtpl = getenv(_OAUTH_ENV_HTTPGET);
  if (!cmdtpl) cmdtpl = xstrdup (_OAUTH_DEF_HTTPGET);
  else cmdtpl = xstrdup (cmdtpl); // clone getenv() string.

  // add URL and post param - error if no '%p' or '%u' present in definition
  t1=strstr(cmdtpl, "%u");
  if (!t1) {
    fprintf(stderr, "\nliboauth: invalid HTTP command. set the '%s' environment variable.\n\n",_OAUTH_ENV_HTTPGET);
    return(NULL);
  }
  *(++t1)= 's';

  e1 = oauth_escape_shell(u);
  if (q) {
    char *e2;
    e2 = oauth_escape_shell(q);
    t1=(char*)xmalloc(sizeof(char)*(strlen(e1)+strlen(e2)+2));
    strcpy(t1,e1); strcat(t1,"?"); strcat(t1,e2);
    free(e2);
  }
  snprintf(cmd, BUFSIZ, cmdtpl, q?t1:e1);
  free(cmdtpl);
  free(e1);
  if (q) free(t1);
  return oauth_exec_shell(cmd);
}
#endif // command-line curl.

/* wrapper functions */

/**
 * do a HTTP GET request, wait for it to finish 
 * and return the content of the reply.
 * (requires libcurl or a command-line HTTP client)
 * 
 * more documentation in oauth.h
 *
 * @param u base url to get
 * @param q query string to send along with the HTTP request or NULL.
 * @return  In case of an error NULL is returned; otherwise a pointer to the
 * replied content from HTTP server. latter needs to be freed by caller.
 */
char *oauth_http_get (const char *u, const char *q) {
#ifdef HAVE_CURL
  return oauth_curl_get(u,q,NULL);
#elif defined(HAVE_SHELL_CURL)
  return oauth_exec_get(u,q);
#else 
  return NULL;
#endif
}

/**
 * do a HTTP GET request, wait for it to finish 
 * and return the content of the reply.
 * (requires libcurl)
 * 
 * @param u base url to get
 * @param q query string to send along with the HTTP request or NULL.
 * @param customheader specify custom HTTP header (or NULL for none)
 * @return  In case of an error NULL is returned; otherwise a pointer to the
 * replied content from HTTP server. latter needs to be freed by caller.
 */
char *oauth_http_get2 (const char *u, const char *q, const char *customheader) {
#ifdef HAVE_CURL
  return oauth_curl_get(u,q,customheader);
#else 
  return NULL;
#endif
}

/**
 * do a HTTP POST request, wait for it to finish 
 * and return the content of the reply.
 * (requires libcurl or a command-line HTTP client)
 *
 * more documentation in oauth.h
 *
 * @param u url to query
 * @param p postargs to send along with the HTTP request.
 * @return  In case of an error NULL is returned; otherwise a pointer to the
 * replied content from HTTP server. latter needs to be freed by caller.
 */
char *oauth_http_post (const char *u, const char *p) {
#ifdef HAVE_CURL
  return oauth_curl_post(u,p,NULL);
#elif defined(HAVE_SHELL_CURL)
  return oauth_exec_post(u,p);
#else 
  return NULL;
#endif
}


/**
 * do a HTTP POST request, wait for it to finish 
 * and return the content of the reply.
 * (requires libcurl)
 *
 * more documentation in oauth.h
 *
 * @param u url to query
 * @param p postargs to send along with the HTTP request.
 * @param customheader specify custom HTTP header (or NULL for none)
 * @return  In case of an error NULL is returned; otherwise a pointer to the
 * replied content from HTTP server. latter needs to be freed by caller.
 */
char *oauth_http_post2 (const char *u, const char *p, const char *customheader) {
#ifdef HAVE_CURL
  return oauth_curl_post(u,p,customheader);
#else
  return NULL;
#endif
}

/**
 * http post raw data from file.
 * the returned string needs to be freed by the caller
 *
 * more documentation in oauth.h
 *
 * @param u url to retrieve
 * @param fn filename of the file to post along
 * @param len length of the file in bytes. set to '0' for autodetection
 * @param customheader specify custom HTTP header (or NULL for default)
 * @return returned HTTP reply or NULL on error
 */
char *oauth_post_file (const char *u, const char *fn, const size_t len, const char *customheader){
#ifdef HAVE_CURL
  return oauth_curl_post_file (u, fn, len, customheader);
#elif defined(HAVE_SHELL_CURL)
  fprintf(stderr, "\nliboauth: oauth_post_file requires libcurl. libcurl is not available.\n\n");
  return NULL;
#else 
  return NULL;
#endif
}

/**
 * http post raw data.
 * the returned string needs to be freed by the caller
 *
 * more documentation in oauth.h
 *
 * @param u url to retrieve
 * @param data data to post along
 * @param len length of the file in bytes. set to '0' for autodetection
 * @param customheader specify custom HTTP header (or NULL for default)
 * @return returned HTTP reply or NULL on error
 */
char *oauth_post_data (const char *u, const char *data, size_t len, const char *customheader) {
#ifdef HAVE_CURL
  return oauth_curl_post_data (u, data, len, customheader);
#elif defined(HAVE_SHELL_CURL)
  fprintf(stderr, "\nliboauth: oauth_post_file requires libcurl. libcurl is not available.\n\n");
  return NULL;
#else
  return (NULL);
#endif
}

char *oauth_send_data (const char *u, const char *data, size_t len, const char *customheader, const char *httpMethod) {
#ifdef HAVE_CURL
  return oauth_curl_send_data (u, data, len, customheader, httpMethod);
#elif defined(HAVE_SHELL_CURL)
  fprintf(stderr, "\nliboauth: oauth_send_file requires libcurl. libcurl is not available.\n\n");
  return NULL;
#else
  return (NULL);
#endif
}

char *oauth_post_data_with_callback (const char *u, const char *data, size_t len, const char *customheader, void (*callback)(void*,int,size_t,size_t), void *callback_data) {
#ifdef HAVE_CURL
  return oauth_curl_post_data_with_callback(u, data, len, customheader, callback, callback_data);
#elif defined(HAVE_SHELL_CURL)
  fprintf(stderr, "\nliboauth: oauth_post_data_with_callback requires libcurl.\n\n");
  return NULL;
#else
  return (NULL);
#endif
}
/* vi:set ts=8 sts=2 sw=2: */
