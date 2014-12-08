{
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
}


{$IFNDEF M_FTP_FILE}
{$DEFINE M_FTP_FILE}

const
  FNUM_DEFAULT = 0; // user's default FTP server
  FNUM_FTP1    = 1; // first FTP server in setting
  FNUM_FTP2    = 2; // second...
  FNUM_FTP3    = 3;
  FNUM_FTP4    = 4;
  FNUM_FTP5    = 5;

  FMODE_RAWFILE   = 1; // Object list contains path(s) to file(s) which will be uploaded as they are
  FMODE_ZIPFILE   = 2; // ... path(s) to file(s) which will be zipped and uploaded as one ZIP file 
  FMODE_ZIPFOLDER = 4; // ... path to folder which will be zipped and uploaded as one ZIP file (objectCount == 1)

  FUPL_UNICODE = 1; // Object list contains WCHAR* paths 

type
  TFTPUPLOAD = record
    cbSize:int;         // size of the structure
    hContact:TMCONTACT; // contact handle, can be NULL
    ftpNum:byte;        // number of the FTP server which will be used for upload, can be one of FNUM_* values
    mode:byte;          // upload mode, can be one of FMODE_* values
    flags:dword;        // bitwise OR of the FUPL_* flags above
    pszObjects:^TChar;  // pointer to the array of the object(s) to upload, content is determined by MODE value	
    objectCount:int;    // number of items in Object list
  end;

const
//
//  Send file(s) or folder in selected mode to the FTP server
//	wParam = 0; not used
//	lParam = (LPARAM)(FTPUPLOAD*)&ftpu; pointer to FTPUPLOAD
//  returns 0 if upload started with no errors, nonzero otherwise
//
  MS_FTPFILE_UPLOAD:PAnsiChar = 'FTPFile/Upload';

(*
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
*)

//
//  Show a simple file manager
//  wParam = 0; not used
//  lParam = 0; not used
//  returns 0 always
//
  MS_FTPFILE_SHOWMANAGER:PAnsiChar = 'FTPFile/ShowManager';


//
// OBSOLOTE SERVICE (used by Send Screenshot plugin)
// Do NOT use it!
//
  MS_FTPFILE_SHAREFILE:PAnsiChar = 'FTPFile/ShareFiles';

{$ENDIF}
