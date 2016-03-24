typedef void* SkyLogin;

/* Size of the buffer you should supply as pszOutUIC on SkyLogin_CreateUICString */
#define UICSTR_SIZE	1024

#ifndef EXPORT
#define EXPORT
#endif

/* Initialize SkyLogin Instance */
EXPORT SkyLogin SkyLogin_Init();

/* Uninitialize Skylogin Instance */
EXPORT void SkyLogin_Exit(SkyLogin pInst);

/* Optionally sets a function that receives debug logging for diagnosis. i.e. 
 * you can use fprintf and stderr here
 */
EXPORT void SkyLogin_SetLogFunction(SkyLogin pPInst, int (__cdecl *pfLog)(void *stream, const char *format, ...), void *pLogStream);

/* Load Credentials from cache, if they are available and valid
 * Returns:
 * 1 if credentials are successfully loaded, 
 * 0 if they are invalid or don't exist */
EXPORT int SkyLogin_LoadCredentials(SkyLogin pInst, char *pszUser);

/* Perform login with Username and Password 
 * Returns:
 * 1 on success, 0 on failure, -1 on socket error, -2 on bad response */
EXPORT int SkyLogin_PerformLogin(SkyLogin pInst, char *pszUser, char *pszPass);

/* Perform login with OAuth token
 * Returns:
 * 1 on success, 0 on failure, -1 on socket error, -2 on bad response */
EXPORT int SkyLogin_PerformLoginOAuth(SkyLogin pInst, const char *OAuth);

/* Creates UIC string from nonce pszNonce and places it in pszOutUIC
 * pszOutUIC buffer should be at least UICSTR_SIZE in size.
 *
 * Returns:
 * Size of UIC string in Bytes on success, 0 on failure
 */
EXPORT int SkyLogin_CreateUICString(SkyLogin pInst, const char *pszNonce, char *pszOutUIC);

/* Gets the base64 encoded signed Credentials after login. They are required if
 * you do OAUTH-Login instead of UICString from above.
 * pszOutUIC buffer should be at least UICSTR_SIZE in size.
 *
 * Returns:
 * Size of UIC string in Bytes on success, 0 on failure
 */
EXPORT int SkyLogin_GetCredentialsUIC(SkyLogin pInst, char *pszOutUIC);

/* Gets the Skype username of the currently loaded login credentials.
 * This can be useful if you do OAuth login and want to get the assigned
 * Skype username without the need to query it seperately so that you can
 * i.e. store it in your DB for later calls of SkyLogin_LoadCredentials()
 *
 * Returns:
 * Pointer to the username, NULL if it doesn't exist. Ne need to free()!
 */
EXPORT char *SkyLogin_GetUser(SkyLogin pInst);
