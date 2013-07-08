#include "Glob.h"

/* MIME type/ext map */
ContentTypeDB MIME = NULL;
/* Default Mime type when recognition fails */
char DefaultMime[] = "application/octet-stream";

int bInitMimeHandling() {
	FILE *mimeDB;
	char line[LINE_MAX_SIZE];
	char *tok = NULL;
	ContentType *pDBCell = NULL;
	ContentTypeDB pDB = NULL;
	ExtensionList extListCur = NULL;
	ExtensionListCell *pExtCell = NULL;
	char szBuf[10000];

	strcpy(szBuf, szPluginPath);
	strcat(szBuf, szMimeTypeConfigFile);
	mimeDB = fopen(szBuf, "r");

	if (mimeDB != NULL) {
		while (fgets(line, LINE_MAX_SIZE, mimeDB)) {
			/*filter junk lines assuming Mime type start with letter
			(convention ?) */
			if ((line[0] <= 'z' && line[0] >= 'a')
			    || (line[0] <= 'Z' && line[0] >= 'A')) {
				/*remove comments trailing comments*/
				tok = strrchr(line, '#');
				if (tok != NULL) {
					*tok = '\0';
				}
				/* remove trailing \n */
				int lenght = (int)strlen(line);
				if (lenght > 0 && line[lenght - 1] == '\n')
					line[lenght - 1] = '\0';

				/* first token = mime type */
				tok = (char*)strtok(line, " \t");
				/*create and fill a cell*/
				pDBCell = (ContentType*)malloc(sizeof(ContentType));
				pDBCell->mimeType = (char*)malloc(strlen(tok) + 1);
				strcpy(pDBCell->mimeType, tok);
				pDBCell->extList = NULL;
				pDBCell->next = NULL;
				/* looking for extensions */
				tok = (char*)strtok(NULL, " \t");
				while (tok != NULL) {
					/*create and fill a cell*/
					pExtCell = (ExtensionListCell*)malloc(sizeof(ExtensionListCell));
					pExtCell->ext = (char*)malloc(strlen(tok) + 1);
					strcpy(pExtCell->ext, tok);
					pExtCell->next = NULL;
					/*link*/
					if (pDBCell->extList == NULL) {
						pDBCell->extList = pExtCell;
					} else {
						extListCur->next = pExtCell;
					}
					extListCur = pExtCell;
					tok = (char*)strtok(NULL, " \t");
				}
				/* link */
				if (pDBCell->extList != NULL) {	/*extension(s) found*/
					if (MIME == NULL) {
						MIME = pDBCell;
					} else {
						pDB->next = pDBCell;
					}
					pDB = pDBCell;
				} else {	/*no extension found, freeing memory*/
					free(pDBCell->mimeType);
					free(pDBCell);
				}
			}
		}

		fclose(mimeDB);
	}
	if (MIME == NULL) {
		return 0;
	}
	return 1;
}

const char * pszGetMimeType(const char * pszFileName) {
	ContentTypeDB courMIME;
	ExtensionList courEXT;
	const char* ext;

	ext = strrchr(pszFileName, '.');

	if (ext != NULL) {
		if (ext[1] == '\0') {
			/*empty extension */
			return DefaultMime;
		} else {
			/*remove the "."*/
			ext = ext + 1;
		}

		courMIME = MIME;
		while (courMIME != NULL) {
			courEXT = courMIME->extList;
			while (courEXT != NULL) {
				if (!_stricmp(courEXT->ext, ext)) {
					return courMIME->mimeType;
				}
				courEXT = courEXT->next;
			}
			courMIME = courMIME->next;
		}
		/*extension unknown*/
		return DefaultMime;
	} else {
		/*no extension*/
		return DefaultMime;
	}
}


#ifdef TEST
void printDB() {
	ContentTypeDB courMIME;
	ExtensionList courEXT;

	courMIME = MIME;
	while (courMIME != NULL) {
		printf("%s", courMIME->mimeType);
		courEXT = courMIME->extList;
		while (courEXT != NULL) {
			printf(" %s", courEXT->ext);
			courEXT = courEXT->next;
		}
		courMIME = courMIME->next;
		printf("\n");
	}
}

int main(int argc, char* argv[]) {
	bInitMimeHandling();
	printDB();
	printf("%s\n", pszGetMimeType(argv[1]));
	return 0;
}
#endif
