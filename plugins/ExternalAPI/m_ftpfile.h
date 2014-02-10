/*
FTP File YM plugin
Copyright (C) 2007-2010 Jan Holub

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  
*/


#ifndef __M_FTP_FILE_H__
#define __M_FTP_FILE_H__

#define FNUM_DEFAULT	0		// user's default FTP server
#define FNUM_FTP1		1		// first FTP server in setting
#define FNUM_FTP2		2		// second...
#define FNUM_FTP3		3
#define FNUM_FTP4		4
#define FNUM_FTP5		5

#define FMODE_RAWFILE	1		// Object list contains path(s) to file(s) which will be uploaded as they are
#define FMODE_ZIPFILE	2		// ... path(s) to file(s) which will be zipped and uploaded as one ZIP file 
#define FMODE_ZIPFOLDER	4		// ... path to folder which will be zipped and uploaded as one ZIP file (objectCount == 1)

#define FUPL_UNICODE	1		// Object list contains WCHAR* paths 

#if defined _UNICODE || defined UNICODE
#define FUPL_TCHAR		FUPL_UNICODE
#else
#define FUPL_TCHAR		0
#endif

typedef struct 
{
	int cbSize;				// size of the structure
	MCONTACT hContact;			// contact handle, can be NULL
	BYTE ftpNum;				// number of the FTP server which will be used for upload, can be one of FNUM_* values
	BYTE mode;				// upload mode, can be one of FMODE_* values
	DWORD flags;				// bitwise OR of the FUPL_* flags above

	union {
		TCHAR	**pstzObjects;	// pointer to the array of the object(s) to upload, content is determined by MODE value	
		char	**pszObjects;	
		wchar_t	**pswzObjects;	
	};	

	int objectCount;			// number of items in Object list
} FTPUPLOAD;

//
//  Send file(s) or folder in selected mode to the FTP server
//	wParam = 0; not used
//	lParam = (LPARAM)(FTPUPLOAD*)&ftpu; pointer to FTPUPLOAD
//  returns 0 if upload started with no errors, nonzero otherwise
//
#define MS_FTPFILE_UPLOAD		"FTPFile/Upload"

__inline static INT_PTR FTPFileUploadA(MCONTACT hContact, BYTE ftpNum, BYTE mode, char **pszObjects, int objCount)
{	
	FTPUPLOAD ftpu = {0};
	ftpu.cbSize = sizeof(ftpu);
	ftpu.hContact = hContact;
	ftpu.ftpNum = ftpNum;
	ftpu.mode = mode;
	ftpu.pszObjects = pszObjects;
	ftpu.objectCount = objCount;
	return CallService(MS_FTPFILE_UPLOAD, 0, (LPARAM)&ftpu);
}

__inline static INT_PTR FTPFileUploadW(MCONTACT hContact, BYTE ftpNum, BYTE mode, wchar_t **pswzObjects, int objCount)
{	
	FTPUPLOAD ftpu = {0};
	ftpu.cbSize = sizeof(ftpu);
	ftpu.hContact = hContact;
	ftpu.ftpNum = ftpNum;
	ftpu.mode = mode;
	ftpu.flags = FUPL_UNICODE;
	ftpu.pswzObjects = pswzObjects;
	ftpu.objectCount = objCount;
	return CallService(MS_FTPFILE_UPLOAD, 0, (LPARAM)&ftpu);
}

#if defined _UNICODE || defined UNICODE
#define FTPFileUpload	FTPFileUploadW
#else
#define FTPFileUpload	FTPFileUploadA
#endif

//
//  Show a simple file manager
//  wParam = 0; not used
//  lParam = 0; not used
//  returns 0 always
//
#define MS_FTPFILE_SHOWMANAGER	"FTPFile/ShowManager"

__inline static INT_PTR FTPFileShowManager()
{	
	return CallService(MS_FTPFILE_SHOWMANAGER, 0, 0);
}

//
// OBSOLOTE SERVICE (used by Send Screenshot plugin)
// Do NOT use it!
//
#define MS_FTPFILE_SHAREFILE	"FTPFile/ShareFiles"

#endif