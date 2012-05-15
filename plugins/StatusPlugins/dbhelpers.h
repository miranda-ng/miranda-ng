#ifndef PREFIX_ITH
#define PREFIX_ITH		""
#endif

// database helpers
static int __inline DBWriteIthSettingByte(DWORD i, HANDLE hContact,const char *szModule,const char *szSetting,BYTE val) {

	char dbSetting[128];

	_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_ITH, i, szSetting);
	return DBWriteContactSettingByte(hContact, szModule, dbSetting, val);
}

static int __inline DBWriteIthSettingWord(DWORD i, HANDLE hContact,const char *szModule,const char *szSetting,WORD val) {

	char dbSetting[128];

	_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_ITH, i, szSetting);
	return DBWriteContactSettingWord(hContact, szModule, dbSetting, val);
}

static int __inline DBWriteIthSettingDword(DWORD i, HANDLE hContact,const char *szModule,const char *szSetting,DWORD val) {

	char dbSetting[128];

	_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_ITH, i, szSetting);
	return DBWriteContactSettingDword(hContact, szModule, dbSetting, val);
}

static int __inline DBWriteIthSettingString(DWORD i, HANDLE hContact,const char *szModule,const char *szSetting,const char *val) {

	char dbSetting[128];

	_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_ITH, i, szSetting);
	return DBWriteContactSettingString(hContact, szModule, dbSetting, val);
}

static int __inline DBGetIthSettingByte(DWORD i, HANDLE hContact, const char *szModule, const char *szSetting, int errorValue) {


	char dbSetting[128];

	_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_ITH, i, szSetting);
	return DBGetContactSettingByte(hContact, szModule, dbSetting, errorValue);
}

static WORD __inline DBGetIthSettingWord(DWORD i, HANDLE hContact, const char *szModule, const char *szSetting, int errorValue) {


	char dbSetting[128];

	_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_ITH, i, szSetting);
	return DBGetContactSettingWord(hContact, szModule, dbSetting, errorValue);
}

static DWORD __inline DBGetIthSettingDword(DWORD i, HANDLE hContact, const char *szModule, const char *szSetting, int errorValue) {


	char dbSetting[128];

	_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_ITH, i, szSetting);
	return DBGetContactSettingDword(hContact, szModule, dbSetting, errorValue);
}

static int __inline DBGetIthSetting(DWORD i, HANDLE hContact, const char *szModule, const char *szSetting, DBVARIANT *dbv) {


	char dbSetting[128];

	_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_ITH, i, szSetting);
	return DBGetContactSetting(hContact, szModule, dbSetting, dbv);
}

static int __inline DBDeleteIthSetting(DWORD i, HANDLE hContact,const char *szModule,const char *szSetting) {

	char dbSetting[128];

	_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_ITH, i, szSetting);
	return DBDeleteContactSetting(hContact, szModule, dbSetting);
}

#define db_getb(a,b)		DBGetContactSettingByte(NULL, MODULENAME, a, b)
#define db_getw(a,b)		DBGetContactSettingWord(NULL, MODULENAME, a, b)
#define db_getd(a,b)		DBGetContactSettingDword(NULL, MODULENAME, a, b)
#define db_gets(a,b)		DBGetContactSetting(NULL, MODULENAME, a, b)
#define db_setb(a,b)		DBWriteContactSettingByte(NULL, MODULENAME, a, (BYTE)b)
#define db_sets(a,b)		DBWriteContactSettingString(NULL, MODULENAME, a, b)
#define db_setw(a,b)		DBWriteContactSettingWord(NULL, MODULENAME, a, (WORD)b)
#define db_setd(a,b)		DBWriteContactSettingDword(NULL, MODULENAME, a, (DWORD)b)
#define db_del(a)			DBDeleteContactSetting(NULL, MODULENAME, a);

#define dbi_getb(a,b,c)		DBGetIthSettingByte(a, NULL, MODULENAME, b, c)
#define dbi_getw(a,b,c)		DBGetIthSettingWord(a, NULL, MODULENAME, b, c)
#define dbi_getd(a,b,c)		DBGetIthSettingDword(a, NULL, MODULENAME, b, c)
#define dbi_gets(a,b,c)		DBGetIthSetting(a, NULL, MODULENAME, b, c)
#define dbi_setb(a,b,c)		DBWriteIthSettingByte(a, NULL, MODULENAME, b, (BYTE)c)
#define dbi_sets(a,b,c)		DBWriteIthSettingString(a, NULL, MODULENAME, b, c)
#define dbi_setw(a,b,c)		DBWriteIthSettingWord(a, NULL, MODULENAME, b, (WORD)c)
#define dbi_setd(a,b,c)		DBWriteIthSettingDword(a, NULL, MODULENAME, b, (DWORD)c)
#define dbi_del(a,b)		DBDeleteIthSetting(a, NULL, MODULENAME, b);