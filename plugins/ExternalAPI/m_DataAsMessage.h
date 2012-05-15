/*
	DataAsMessage plugin for Miranda IM
	Copyright (c) 2006-2007 Chervov Dmitry

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef __M_DATAASMESSAGE_H
#define __M_DATAASMESSAGE_H

// DAM_SENDRESULTINFO::iResult values
#define DAM_SR_SUCCESS 0
#define DAM_SR_TIMEOUT 1 // timeout period expired; this value is returned also if the contact went offline for a time longer than a timeout period
#define DAM_SR_NOTSUPPORTED 2 // means this szDataType is not supported by the remote side
#define DAM_SR_NODAM 3 // means there is no DataAsMessage plugin on the remote side; keep in mind that this error can also appear accidentally because of a bad connectivity during the handshake (if there was a timeout when waiting for a response)
#define DAM_SR_CANCELLEDLOCAL 4 // cancelled from the local(sending) side
#define DAM_SR_CANCELLEDREMOTE 5 // cancelled from the remote(receiving) side
#define DAM_SR_BADCRC 6 // bad CRC; we can't do anything with this error. the most probable cause is that protocol filters some of characters in our messages
#define DAM_SR_UNKNOWN 7 // unknown error

// Return values for DAM_SENDRESULTPROC
#define DAM_SRA_RETRY 1

typedef struct
{
	int cbSize; // sizeof(DAM_SENDRESULTINFO)
	HANDLE hContact;
	char *szDataType;
	DWORD SessionID;
	int iResult; // transmission result code
} DAM_SENDRESULTINFO; // hContact, szDataType and SessionID fields correspond to the fields of the DAM_SENDDATAINFO structure

typedef int (*DAM_SENDRESULTPROC)(DAM_SENDRESULTINFO *sri); // this procedure receives the result of the transmission. it's called when the session closes (either the data was sent successfully or there was an error)
// you can return DAM_SRA_RETRY when iResult is DAM_SR_TIMEOUT if you want to retry sending


// DAM_SENDDATAINFO::Flags constants
#define DAM_SDF_DONTPACK 1 // don't compress the data (by default all the data is compressed)
#define DAM_SDF_NOTIMEOUT 2 // don't generate a timeout error ever, keep trying to send the data. If the contact is offline, the data is saved in the memory until the contact goes online. Loss of the data occurs only if the sender's miranda closes (this may change in future to allow fully functional offline sending that will guarantee the data to be sent in any case, but of course the sending starts only when the both contacts are online). other errors than the timeout error can be still generated though.

typedef struct
{
	int cbSize; // sizeof(DAM_SENDDATAINFO)
	HANDLE hContact;
	char *szDataType; // zero-terminated string, containing data type, preferably in format "YourPluginName" or "YourPluginName/Something" (make sure this string won't coincide by an accident with someone else's string!). you can identify your data by this ID later
	int nDataLen; // keep in mind that if the length is too big (more than about 8 KB), it's more preferable to split your data into several chunks, as you won't be able to "pick up" your data at the other end until all the data is transferred
	char *cData;
	int Flags; // combination of the DAM_SDF_ constants
	DWORD SendAfterSessionID; // may be NULL; otherwise it's guaranteed that the sending starts only after successful completion of SendAfterSessionID session
	DAM_SENDRESULTPROC SendResultProc; // pointer to a procedure that receives the result; can be NULL
	DWORD SessionID; // OUT; receives the session ID
} DAM_SENDDATAINFO;

// MS_DAM_SENDDATA return values
#define DAM_SDA_NOERROR 0
#define DAM_SDA_NOTSUPPORTED (-1) // contact's protocol doesn't support sending/receiving messages
#define DAM_SDA_TOOMANYSESSIONS (-2) // too many sessions

// MS_DAM_SENDDATA
// sends the data
// wParam = (WPARAM)(DAM_SENDDATAINFO*)sdi;
// lParam = 0
// Returns 0 (DAM_SDA_NOERROR) and fills SessionID if the session was queued for sending successfully; returns one of the DAM_SDA_ values on failure
#define MS_DAM_SENDDATA "DataAsMessage/SendData"

static int __inline DAMSendData(HANDLE hContact, char *szDataType, int nDataLen, char *cData, int Flags, DWORD SendAfterSessionID, DAM_SENDRESULTPROC SendResultProc, DWORD *pSessionID)
{
	int Result;
	DAM_SENDDATAINFO sdi;
	ZeroMemory(&sdi, sizeof(sdi));
	sdi.cbSize = sizeof(sdi);
	sdi.hContact = hContact;
	sdi.szDataType = szDataType;
	sdi.nDataLen = nDataLen;
	sdi.cData = cData;
	sdi.Flags = Flags;
	sdi.SendAfterSessionID = SendAfterSessionID;
	sdi.SendResultProc = SendResultProc;
	Result = CallService(MS_DAM_SENDDATA, (WPARAM)&sdi, 0);
	if (pSessionID)
	{
		*pSessionID = sdi.SessionID;
	}
	return Result;
}


typedef struct
{
	int cbSize; // sizeof(DAM_RECVDATAINFO)
	HANDLE hContact;
	char *szDataType;
	int nDataLen;
	char *cData;
} DAM_RECVDATAINFO;

// ME_DAM_RECVDATA
// hook up to this event to check for incoming data
// make sure rdi->szDataType is yours before doing anything!
// The important thing here is that your plugin will receive TWO ME_DAM_RECVDATA notifications on every single MS_DAM_SENDDATA call from a remote side:
//   The first notification arrives when the remote side starts to transmit the data. In this case DAM_RECVDATAINFO::cData = NULL (and DAM_RECVDATAINFO::nDataLen = -1) as we didn't receive any data yet. Return 1 to indicate that your plugin recognized the DAM_RECVDATAINFO::szDataType, otherwise return 0. If there are no any plugin that recognized the data, DAM cancels the transfer and there won't be any second notification for it.
//   The second notification is when the data is transmitted successfully. nDataLen contains the usual data size and cData points to the data buffer. cData is guaranteed to be valid only during the ME_DAM_RECVDATA call. You must copy the data to your own plugin's memory if you need to use it later. And again, return 1 to indicate that your plugin recognized the data, otherwise return 0
// wParam = (WPARAM)(DAM_RECVDATAINFO*)rdi;
// lParam = 0
#define ME_DAM_RECVDATA "DataAsMessage/RecvData"


typedef struct
{
	int cbSize; // sizeof(DAM_COMPRESSION_DATA)
	void *(*malloc)(size_t); // pointer to the malloc() function of the calling module
	int nInputDataLen; // IN; length of the input data in bytes
	char *cInputData; // IN; pointer to the input data
	int nOutputDataLen; // OUT; length of the output data in bytes
	char *cOutputData; // OUT; pointer to the output data
} DAM_COMPRESSION_DATA;

// Compression/decompression services. You DON'T have to use them if you want to send compressed data using DAM services, as DAM compresses the data automatically. These services are here just in case you need to compress/decompress data for your own needs

// MS_DAM_COMPRESS
// compresses the data using BZip2
// wParam = (WPARAM)(DAM_COMPRESSION_DATA*)bd;
// lParam = 0
// cbSize, malloc, nInputDataLen and cInputData fields must be valid when calling the service.
// Returns 0 and fills nOutputDataLen and cOutputData on success; returns non-zero on failure
// This service allocates the memory for cOutputData using the specified malloc function; you must call free(cOutputData) when you've finished working with cOutputData
#define MS_DAM_COMPRESS "DataAsMessage/Compress"

// MS_DAM_DECOMPRESS
// decompresses the data
// wParam = (WPARAM)(DAM_COMPRESSION_DATA*)bd;
// lParam = 0
// cbSize, malloc, nInputDataLen and cInputData fields must be valid when calling the service.
// Returns 0 and fills nOutputDataLen and cOutputData on success; returns non-zero on failure
// This service allocates the memory for cOutputData using the specified malloc function; you must call free(cOutputData) when you've finished working with cOutputData
#define MS_DAM_DECOMPRESS "DataAsMessage/Decompress"


__inline int DAM_Compress(char *cInputData, int nInputDataLen, char **cOutputData, int *nOutputDataLen)
{
	int Result;
	DAM_COMPRESSION_DATA bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.cbSize = sizeof(bd);
	bd.malloc = malloc;
	bd.nInputDataLen = nInputDataLen;
	bd.cInputData = cInputData;
	Result = CallService(MS_DAM_COMPRESS, (WPARAM)&bd, 0);
	*nOutputDataLen = bd.nOutputDataLen;
	*cOutputData = bd.cOutputData;
	return Result;
}

__inline int DAM_Decompress(char *cInputData, int nInputDataLen, char **cOutputData, int *nOutputDataLen)
{
	int Result;
	DAM_COMPRESSION_DATA bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.cbSize = sizeof(bd);
	bd.malloc = malloc;
	bd.nInputDataLen = nInputDataLen;
	bd.cInputData = cInputData;
	Result = CallService(MS_DAM_DECOMPRESS, (WPARAM)&bd, 0);
	*nOutputDataLen = bd.nOutputDataLen;
	*cOutputData = bd.cOutputData;
	return Result;
}


#endif // __M_DATAASMESSAGE_H
