char * __cdecl strtok_r (
        char * string,
        const char * control,
		char **nextoken
        );

void TranslateMirandaRelativePathToAbsolute(LPCSTR cszPath, LPSTR szAbsolutePath, BOOL fQuoteSpaces);
