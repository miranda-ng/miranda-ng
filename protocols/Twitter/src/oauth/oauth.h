/**
 *  @brief OAuth.net implementation in POSIX-C.
 *  @file oauth.h
 *  @author Robin Gareus <robin@gareus.org>
 *
 * Copyright 2007-2011 Robin Gareus <robin@gareus.org>
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
#ifndef _OAUTH_H
#define _OAUTH_H      1 

#ifndef DOXYGEN_IGNORE
// liboauth version
#define LIBOAUTH_VERSION "0.9.6"
#define LIBOAUTH_VERSION_MAJOR  0
#define LIBOAUTH_VERSION_MINOR  9
#define LIBOAUTH_VERSION_MICRO  6

//interface revision number
//http://www.gnu.org/software/libtool/manual/html_node/Updating-version-info.html
#define LIBOAUTH_CUR  8
#define LIBOAUTH_REV  3
#define LIBOAUTH_AGE  8
#endif

#ifdef __GNUC__
#    define OA_GCC_VERSION_AT_LEAST(x,y) (__GNUC__ > x || __GNUC__ == x && __GNUC_MINOR__ >= y)
#else
#    define OA_GCC_VERSION_AT_LEAST(x,y) 0
#endif

#ifndef attribute_deprecated
#if OA_GCC_VERSION_AT_LEAST(3,1)
#    define attribute_deprecated __attribute__((deprecated))
#else
#    define attribute_deprecated
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

/** \enum OAuthMethod
 * signature method to used for signing the request.
 */ 
typedef enum { 
    OA_HMAC=0, ///< use HMAC-SHA1 request signing method
    OA_RSA, ///< use RSA signature 
    OA_PLAINTEXT ///< use plain text signature (for testing only)
  } OAuthMethod;

/**
 * Base64 encode and return size data in 'src'. The caller must free the
 * returned string.
 *
 * @param size The size of the data in src
 * @param src The data to be base64 encode
 * @return encoded string otherwise NULL
 */
char *oauth_encode_base64(int size, const unsigned char *src);

/**
 * Decode the base64 encoded string 'src' into the memory pointed to by
 * 'dest'. 
 *
 * @param dest Pointer to memory for holding the decoded string.
 * Must be large enough to receive the decoded string.
 * @param src A base64 encoded string.
 * @return the length of the decoded string if decode
 * succeeded otherwise 0.
 */
int oauth_decode_base64(unsigned char *dest, const char *src);

/**
 * Escape 'string' according to RFC3986 and
 * http://oauth.net/core/1.0/#encoding_parameters.
 *
 * @param string The data to be encoded
 * @return encoded string otherwise NULL
 * The caller must free the returned string.
 */
char *oauth_url_escape(const char *string);

/**
 * Parse RFC3986 encoded 'string' back to  unescaped version.
 *
 * @param string The data to be unescaped
 * @param olen unless NULL the length of the returned string is stored there.
 * @return decoded string or NULL
 * The caller must free the returned string.
 */
char *oauth_url_unescape(const char *string, size_t *olen);
 

/**
 * returns base64 encoded HMAC-SHA1 signature for
 * given message and key.
 * both data and key need to be urlencoded.
 *
 * the returned string needs to be freed by the caller
 *
 * @param m message to be signed
 * @param k key used for signing
 * @return signature string.
 */
char *oauth_sign_hmac_sha1 (const char *m, const char *k);

/**
 * same as \ref oauth_sign_hmac_sha1 but allows
 * to specify length of message and key (in case they contain null chars).
 *
 * @param m message to be signed
 * @param ml length of message
 * @param k key used for signing
 * @param kl length of key
 * @return signature string.
 */
char *oauth_sign_hmac_sha1_raw (const char *m, const size_t ml, const char *k, const size_t kl);

/**
 * returns plaintext signature for the given key.
 *
 * the returned string needs to be freed by the caller
 *
 * @param m message to be signed
 * @param k key used for signing
 * @return signature string
 */
char *oauth_sign_plaintext (const char *m, const char *k);

/**
 * returns RSA-SHA1 signature for given data.
 * the returned signature needs to be freed by the caller.
 *
 * @param m message to be signed
 * @param k private-key PKCS and Base64-encoded 
 * @return base64 encoded signature string.
 */
char *oauth_sign_rsa_sha1 (const char *m, const char *k);

/**
 * verify RSA-SHA1 signature.
 *
 * returns the output of EVP_VerifyFinal() for a given message,
 * cert/pubkey and signature.
 *
 * @param m message to be verified
 * @param c public-key or x509 certificate
 * @param s base64 encoded signature
 * @return 1 for a correct signature, 0 for failure and -1 if some other error occurred
 */
int oauth_verify_rsa_sha1 (const char *m, const char *c, const char *s);

/**
 * url-escape strings and concatenate with '&' separator.
 * The number of strings to be concatenated must be
 * given as first argument.
 * all arguments thereafter must be of type (char *) 
 *
 * @param len the number of arguments to follow this parameter
 *
 * @return pointer to memory holding the concatenated 
 * strings - needs to be free(d) by the caller. or NULL
 * in case we ran out of memory.
 */
char *oauth_catenc(int len, ...);

/**
 * splits the given url into a parameter array. 
 * (see \ref oauth_serialize_url and \ref oauth_serialize_url_parameters for the reverse)
 * (see \ref oauth_split_post_paramters for a more generic version)
 *
 * @param url the url or query-string to parse; may be NULL
 * @param argv pointer to a (char *) array where the results are stored.
 *  The array is re-allocated to match the number of parameters and each 
 *  parameter-string is allocated with strdup. - The memory needs to be freed
 *  by the caller.
 * 
 * @return number of parameter(s) in array.
 */
int oauth_split_url_parameters(const char *url, char ***argv);

/**
 * splits the given url into a parameter array. 
 * (see \ref oauth_serialize_url and \ref oauth_serialize_url_parameters for the reverse)
 *
 * @param url the url or query-string to parse. 
 * @param argv pointer to a (char *) array where the results are stored.
 *  The array is re-allocated to match the number of parameters and each 
 *  parameter-string is allocated with strdup. - The memory needs to be freed
 *  by the caller.
 * @param qesc use query parameter escape (vs post-param-escape) - if set
 *        to 1 all '+' are treated as spaces ' '
 * 
 * @return number of parameter(s) in array.
 */
int oauth_split_post_paramters(const char *url, char ***argv, short qesc);

/**
 * build a url query string from an array.
 *
 * @param argc the total number of elements in the array
 * @param start element in the array at which to start concatenating.
 * @param argv parameter-array to concatenate.
 * @return url string needs to be freed by the caller.
 *
 */
char *oauth_serialize_url (int argc, int start, char **argv);

/**
 * encode query parameters from an array.
 *
 * @param argc the total number of elements in the array
 * @param start element in the array at which to start concatenating.
 * @param argv parameter-array to concatenate.
 * @param sep separator for parameters (usually "&") 
 * @param mod - bitwise modifiers: 
 *   1: skip all values that start with "oauth_"
 *   2: skip all values that don't start with "oauth_"
 *   4: double quotation marks are added around values (use with sep ", " for HTTP Authorization header).
 * @return url string needs to be freed by the caller.
 */
char *oauth_serialize_url_sep (int argc, int start, char **argv, char *sep, int mod);

/**
 * build a query parameter string from an array.
 *
 * This function is a shortcut for \ref oauth_serialize_url (argc, 1, argv). 
 * It strips the leading host/path, which is usually the first 
 * element when using oauth_split_url_parameters on an URL.
 *
 * @param argc the total number of elements in the array
 * @param argv parameter-array to concatenate.
 * @return url string needs to be freed by the caller.
 */
char *oauth_serialize_url_parameters (int argc, char **argv);
 
/**
 * generate a random string between 15 and 32 chars length
 * and return a pointer to it. The value needs to be freed by the
 * caller
 *
 * @return zero terminated random string.
 */
char *oauth_gen_nonce();

/**
 * string compare function for oauth parameters.
 *
 * used with qsort. needed to normalize request parameters.
 * see http://oauth.net/core/1.0/#anchor14
 */
int oauth_cmpstringp(const void *p1, const void *p2);


/**
 * search array for parameter key.
 * @param argv length of array to search
 * @param argc parameter array to search
 * @param key key of parameter to check.
 *
 * @return FALSE (0) if array does not contain a parameter with given key, TRUE (1) otherwise.
 */
int oauth_param_exists(char **argv, int argc, char *key);

/**
 * add query parameter to array
 *
 * @param argcp pointer to array length int
 * @param argvp pointer to array values 
 * @param addparam parameter to add (eg. "foo=bar")
 */
void oauth_add_param_to_array(int *argcp, char ***argvp, const char *addparam);

/**
 * free array args
 *
 * @param argcp pointer to array length int
 * @param argvp pointer to array values to be free()d
 */
void oauth_free_array(int *argcp, char ***argvp);

/**
 * compare two strings in constant-time (as to not let an
 * attacker guess how many leading chars are correct:
 * http://rdist.root.org/2010/01/07/timing-independent-array-comparison/ )
 *
 * @param a string to compare 
 * @param b string to compare
 * @param len_a length of string a
 * @param len_b length of string b
 *
 * returns 0 (false) if strings are not equal, and 1 (true) if strings are equal.
 */
int oauth_time_independent_equals_n(const char* a, const char* b, size_t len_a, size_t len_b);

/**
 * @deprecated Use oauth_time_independent_equals_n() instead.
 */
int oauth_time_indepenent_equals_n(const char* a, const char* b, size_t len_a, size_t len_b) attribute_deprecated;

/**
 * compare two strings in constant-time.
 * wrapper to \ref oauth_time_independent_equals_n 
 * which calls strlen() for each argument.
 *
 * @param a string to compare 
 * @param b string to compare
 *
 * returns 0 (false) if strings are not equal, and 1 (true) if strings are equal.
 */
int oauth_time_independent_equals(const char* a, const char* b);

/**
 * @deprecated Use oauth_time_independent_equals() instead.
 */
int oauth_time_indepenent_equals(const char* a, const char* b) attribute_deprecated;

/**
 * calculate OAuth-signature for a given HTTP request URL, parameters and oauth-tokens.
 *
 * if 'postargs' is NULL a "GET" request is signed and the 
 * signed URL is returned. Else this fn will modify 'postargs' 
 * to point to memory that contains the signed POST-variables 
 * and returns the base URL.
 *
 * both, the return value and (if given) 'postargs' need to be freed
 * by the caller.
 *
 * @param url The request URL to be signed. append all GET or POST 
 * query-parameters separated by either '?' or '&' to this parameter.
 *
 * @param postargs This parameter points to an area where the return value
 * is stored. If 'postargs' is NULL, no value is stored.
 *
 * @param method specify the signature method to use. It is of type 
 * \ref OAuthMethod and most likely \ref OA_HMAC.
 *
 * @param http_method The HTTP request method to use (ie "GET", "PUT",..)
 * If NULL is given as 'http_method' this defaults to "GET" when 
 * 'postargs' is also NULL and when postargs is not NULL "POST" is used.
 *
 * @param c_key consumer key
 * @param c_secret consumer secret
 * @param t_key token key
 * @param t_secret token secret
 *
 * @return the signed url or NULL if an error occurred.
 *
 */
char *oauth_sign_url2 (const char *url, char **postargs, 
  OAuthMethod method, 
  const char *http_method, //< HTTP request method
  const char *c_key, //< consumer key - posted plain text
  const char *c_secret, //< consumer secret - used as 1st part of secret-key 
  const char *t_key, //< token key - posted plain text in URL
  const char *t_secret //< token secret - used as 2st part of secret-key
  );

/**
 * @deprecated Use oauth_sign_url2() instead.
 */
char *oauth_sign_url (const char *url, char **postargs, 
  OAuthMethod method, 
  const char *c_key, //< consumer key - posted plain text
  const char *c_secret, //< consumer secret - used as 1st part of secret-key 
  const char *t_key, //< token key - posted plain text in URL
  const char *t_secret //< token secret - used as 2st part of secret-key
  ) attribute_deprecated;


/**
 * the back-end behind by /ref oauth_sign_array2.
 * however it does not serialize the signed URL again.
 * The user needs to call /ref oauth_serialize_url (oA)
 * and /ref oauth_free_array to do so.
 *
 * This allows to split parts of the URL to be used for
 * OAuth HTTP Authorization header:
 * see http://oauth.net/core/1.0a/#consumer_req_param
 * the oauthtest2 example code does so.
 * 
 *
 * @param argcp pointer to array length int
 * @param argvp pointer to array values  
 * (argv[0]="http://example.org:80/" argv[1]="first=QueryParamater" ..
 *  the array is modified: fi. oauth_ parameters are added)
 *  These arrays can be generated with /ref oauth_split_url_parameters
 *  or /ref oauth_split_post_paramters.
 *
 * @param postargs This parameter points to an area where the return value
 * is stored. If 'postargs' is NULL, no value is stored.
 *
 * @param method specify the signature method to use. It is of type 
 * \ref OAuthMethod and most likely \ref OA_HMAC.
 *
 * @param http_method The HTTP request method to use (ie "GET", "PUT",..)
 * If NULL is given as 'http_method' this defaults to "GET" when 
 * 'postargs' is also NULL and when postargs is not NULL "POST" is used.
 *
 * @param c_key consumer key
 * @param c_secret consumer secret
 * @param t_key token key
 * @param t_secret token secret
 *
 * @return void
 *
 */
void oauth_sign_array2_process (int *argcp, char***argvp,
  char **postargs,
  OAuthMethod method, 
  const char *http_method, //< HTTP request method
  const char *c_key, //< consumer key - posted plain text
  const char *c_secret, //< consumer secret - used as 1st part of secret-key 
  const char *t_key, //< token key - posted plain text in URL
  const char *t_secret //< token secret - used as 2st part of secret-key
  );

/**
 * same as /ref oauth_sign_url
 * with the url already split into parameter array 
 *
 * @param argcp pointer to array length int
 * @param argvp pointer to array values  
 * (argv[0]="http://example.org:80/" argv[1]="first=QueryParamater" ..
 *  the array is modified: fi. oauth_ parameters are added)
 *  These arrays can be generated with /ref oauth_split_url_parameters
 *  or /ref oauth_split_post_paramters.
 *
 * @param postargs This parameter points to an area where the return value
 * is stored. If 'postargs' is NULL, no value is stored.
 *
 * @param method specify the signature method to use. It is of type 
 * \ref OAuthMethod and most likely \ref OA_HMAC.
 *
 * @param http_method The HTTP request method to use (ie "GET", "PUT",..)
 * If NULL is given as 'http_method' this defaults to "GET" when 
 * 'postargs' is also NULL and when postargs is not NULL "POST" is used.
 *
 * @param c_key consumer key
 * @param c_secret consumer secret
 * @param t_key token key
 * @param t_secret token secret
 *
 * @return the signed url or NULL if an error occurred.
 */
char *oauth_sign_array2 (int *argcp, char***argvp,
  char **postargs,
  OAuthMethod method, 
  const char *http_method, //< HTTP request method
  const char *c_key, //< consumer key - posted plain text
  const char *c_secret, //< consumer secret - used as 1st part of secret-key 
  const char *t_key, //< token key - posted plain text in URL
  const char *t_secret //< token secret - used as 2st part of secret-key
  );

/**
 * @deprecated Use oauth_sign_array2() instead.
 */
char *oauth_sign_array (int *argcp, char***argvp,
  char **postargs,
  OAuthMethod method, 
  const char *c_key, //< consumer key - posted plain text
  const char *c_secret, //< consumer secret - used as 1st part of secret-key 
  const char *t_key, //< token key - posted plain text in URL
  const char *t_secret //< token secret - used as 2st part of secret-key
  ) attribute_deprecated;


/** 
 * calculate body hash (sha1sum) of given file and return
 * a oauth_body_hash=xxxx parameter to be added to the request.
 * The returned string needs to be freed by the calling function.
 *
 * see
 * http://oauth.googlecode.com/svn/spec/ext/body_hash/1.0/oauth-bodyhash.html
 * 
 * @param filename the filename to calculate the hash for
 *
 * @return URL oauth_body_hash parameter string
 */
char *oauth_body_hash_file(char *filename);

/** 
 * calculate body hash (sha1sum) of given data and return
 * a oauth_body_hash=xxxx parameter to be added to the request.
 * The returned string needs to be freed by the calling function.
 * The returned string is not yet url-escaped and suitable to be 
 * passed as argument to \ref oauth_catenc.
 *
 * see
 * http://oauth.googlecode.com/svn/spec/ext/body_hash/1.0/oauth-bodyhash.html
 * 
 * @param length length of the data parameter in bytes
 * @param data to calculate the hash for
 *
 * @return URL oauth_body_hash parameter string
 */
char *oauth_body_hash_data(size_t length, const char *data);

/**
 * base64 encode digest, free it and return a URL parameter
 * with the oauth_body_hash. The returned hash needs to be freed by the
 * calling function. The returned string is not yet url-escaped and 
 * thus suitable to be passed to \ref oauth_catenc.
 *
 * @param len length of the digest to encode
 * @param digest hash value to encode
 *
 * @return URL oauth_body_hash parameter string
 */
char *oauth_body_hash_encode(size_t len, unsigned char *digest);

/**
 * xep-0235 - TODO
 */
char *oauth_sign_xmpp (const char *xml,
  OAuthMethod method, 
  const char *c_secret, //< consumer secret - used as 1st part of secret-key 
  const char *t_secret //< token secret - used as 2st part of secret-key
  );

/**
 * do a HTTP GET request, wait for it to finish 
 * and return the content of the reply.
 * (requires libcurl or a command-line HTTP client)
 *
 * If compiled <b>without</b> libcurl this function calls
 * a command-line executable defined in the environment variable
 * OAUTH_HTTP_GET_CMD - it defaults to 
 * <tt>curl -sA 'liboauth-agent/0.1' '%%u'</tt>
 * where %%u is replaced with the URL and query parameters.
 *
 * bash & wget example:
 * <tt>export OAUTH_HTTP_CMD="wget -q -U 'liboauth-agent/0.1' '%u' "</tt>
 *
 * WARNING: this is a tentative function. it's convenient and handy for testing
 * or developing OAuth code. But don't rely on this function
 * to become a stable part of this API. It does not do 
 * much error checking or handling for one thing..
 *
 * NOTE: \a u and \a q are just concatenated with a '?' in between,
 * unless \a q is NULL. in which case only \a u will be used.
 *
 * @param u base url to get
 * @param q query string to send along with the HTTP request or NULL.
 * @return  In case of an error NULL is returned; otherwise a pointer to the
 * replied content from HTTP server. latter needs to be freed by caller.
 */
char *oauth_http_get (const char *u, const char *q);

/**
 * do a HTTP GET request, wait for it to finish 
 * and return the content of the reply.
 *
 * (requires libcurl)
 *
 * This is equivalent to /ref oauth_http_get but allows to
 * specifiy a custom HTTP header and has
 * has no support for commandline-curl.
 *
 * If liboauth is compiled <b>without</b> libcurl this function
 * always returns NULL.
 *
 * @param u base url to get
 * @param q query string to send along with the HTTP request or NULL.
 * @param customheader specify custom HTTP header (or NULL for none)
 * Multiple header elements can be passed separating them with "\r\n"
 * @return  In case of an error NULL is returned; otherwise a pointer to the
 * replied content from HTTP server. latter needs to be freed by caller.
 */
char *oauth_http_get2 (const char *u, const char *q, const char *customheader);


/**
 * do a HTTP POST request, wait for it to finish 
 * and return the content of the reply.
 * (requires libcurl or a command-line HTTP client)
 *
 * If compiled <b>without</b> libcurl this function calls
 * a command-line executable defined in the environment variable
 * OAUTH_HTTP_CMD - it defaults to 
 * <tt>curl -sA 'liboauth-agent/0.1' -d '%%p' '%%u'</tt>
 * where %%p is replaced with the postargs and %%u is replaced with 
 * the URL. 
 *
 * bash & wget example:
 * <tt>export OAUTH_HTTP_CMD="wget -q -U 'liboauth-agent/0.1' --post-data='%p' '%u' "</tt>
 *
 * NOTE: This function uses the curl's default HTTP-POST Content-Type:
 * application/x-www-form-urlencoded which is the only option allowed
 * by oauth core 1.0 spec. Experimental code can use the Environment variable
 * to transmit custom HTTP headers or parameters.
 *
 * WARNING: this is a tentative function. it's convenient and handy for testing
 * or developing OAuth code. But don't rely on this function
 * to become a stable part of this API. It does not do 
 * much error checking for one thing..
 *
 * @param u url to query
 * @param p postargs to send along with the HTTP request.
 * @return replied content from HTTP server. needs to be freed by caller.
 */
char *oauth_http_post (const char *u, const char *p);

/**
 * do a HTTP POST request, wait for it to finish 
 * and return the content of the reply.
 * (requires libcurl)
 *
 * It's equivalent to /ref oauth_http_post, but offers
 * the possibility to specify a custom HTTP header and
 * has no support for commandline-curl.
 *
 * If liboauth is compiled <b>without</b> libcurl this function
 * always returns NULL.
 *
 * @param u url to query
 * @param p postargs to send along with the HTTP request.
 * @param customheader specify custom HTTP header (or NULL for none)
 * Multiple header elements can be passed separating them with "\r\n"
 * @return replied content from HTTP server. needs to be freed by caller.
 */
char *oauth_http_post2 (const char *u, const char *p, const char *customheader);


/**
 * http post raw data from file.
 * the returned string needs to be freed by the caller
 * (requires libcurl)
 *
 * see dislaimer: /ref oauth_http_post
 *
 * @param u url to retrieve
 * @param fn filename of the file to post along
 * @param len length of the file in bytes. set to '0' for autodetection
 * @param customheader specify custom HTTP header (or NULL for default).
 * Multiple header elements can be passed separating them with "\r\n"
 * @return returned HTTP reply or NULL on error
 */
char *oauth_post_file (const char *u, const char *fn, const size_t len, const char *customheader);

/**
 * http post raw data
 * the returned string needs to be freed by the caller
 * (requires libcurl)
 *
 * see dislaimer: /ref oauth_http_post
 *
 * @param u url to retrieve
 * @param data data to post
 * @param len length of the data in bytes. 
 * @param customheader specify custom HTTP header (or NULL for default)
 * Multiple header elements can be passed separating them with "\r\n"
 * @return returned HTTP reply or NULL on error
 */
char *oauth_post_data (const char *u, const char *data, size_t len, const char *customheader);

/**
 * http post raw data, with callback.
 * the returned string needs to be freed by the caller
 * (requires libcurl)
 *
 * Invokes the callback - in no particular order - when HTTP-request status updates occur.
 * The callback is called with:
 *   void * callback_data: supplied on function call.
 *   int type: 0=data received, 1=data sent.
 *   size_t size: amount of data received or amount of data sent so far
 *   size_t totalsize: original amount of data to send, or amount of data received
 *
 * @param u url to retrieve
 * @param data data to post along
 * @param len length of the file in bytes. set to '0' for autodetection
 * @param customheader specify custom HTTP header (or NULL for default)
 * Multiple header elements can be passed separating them with "\r\n"
 * @param callback specify the callback function
 * @param callback_data specify data to pass to the callback function
 * @return returned HTTP reply or NULL on error
 */
char *oauth_post_data_with_callback      (const char *u, 
                                          const char *data, 
                                          size_t len, 
                                          const char *customheader,
                                          void (*callback)(void*,int,size_t,size_t),
                                          void *callback_data);

/**
 * http send raw data. similar to /ref oauth_http_post but provides
 * for specifying the HTTP request method.
 *
 * the returned string needs to be freed by the caller
 * (requires libcurl)
 *
 * see dislaimer: /ref oauth_http_post
 *
 * @param u url to retrieve
 * @param data data to post
 * @param len length of the data in bytes. 
 * @param customheader specify custom HTTP header (or NULL for default)
 * Multiple header elements can be passed separating them with "\r\n"
 * @param httpMethod specify http verb ("GET"/"POST"/"PUT"/"DELETE") to be used. if httpMethod is NULL, a POST is executed.
 * @return returned HTTP reply or NULL on error
 */
char *oauth_send_data (const char *u,
                       const char *data,
                       size_t len,
                       const char *customheader,
                       const char *httpMethod);

/**
 * http post raw data, with callback.
 * the returned string needs to be freed by the caller
 * (requires libcurl)
 *
 * Invokes the callback - in no particular order - when HTTP-request status updates occur.
 * The callback is called with:
 *   void * callback_data: supplied on function call.
 *   int type: 0=data received, 1=data sent.
 *   size_t size: amount of data received or amount of data sent so far
 *   size_t totalsize: original amount of data to send, or amount of data received
 *
 * @param u url to retrieve
 * @param data data to post along
 * @param len length of the file in bytes. set to '0' for autodetection
 * @param customheader specify custom HTTP header (or NULL for default)
 * Multiple header elements can be passed separating them with "\r\n"
 * @param callback specify the callback function
 * @param callback_data specify data to pass to the callback function
 * @param httpMethod specify http verb ("GET"/"POST"/"PUT"/"DELETE") to be used.
 * @return returned HTTP reply or NULL on error
 */
char *oauth_send_data_with_callback      (const char *u, 
                                          const char *data, 
                                          size_t len, 
                                          const char *customheader,
                                          void (*callback)(void*,int,size_t,size_t),
                                          void *callback_data,
                                          const char *httpMethod);

#ifdef __cplusplus
}       /* extern "C" */
#endif  /* __cplusplus */

#endif
/* vi:set ts=8 sts=2 sw=2: */
