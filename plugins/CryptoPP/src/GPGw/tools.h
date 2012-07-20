#ifndef __TOOLS_H__
#define __TOOLS_H__

#ifdef __cplusplus
extern "C" {
#endif

void replace(char *atext, const char *apattern, const char *areplacement);
char *getNextPart(char *aresult, char *atext, const char *aseparator);
void getLastPart(char *aresult, char *atext);
void appendText(char **abuffer, const char *atext, int atextsize);
void quoteEscapeSequences(char *atext);
void unquoteEscapeSequences(char *atext);

BOOL existsFile(const char *afilename);
BOOL existsPath(const char *apathname);
BOOL writeToFile(char *afilename, const char *atext);
BOOL readFromFile(char *aresult, const char *afilename);
void getTemporaryFileName(char *aresult);

#ifdef __cplusplus
}
#endif

#endif // __TOOLS_H__
