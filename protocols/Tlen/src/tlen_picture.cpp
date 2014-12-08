/*

Tlen Protocol Plugin for Miranda NG
Copyright (C) 2002-2004  Santithorn Bunchua
Copyright (C) 2004-2009  Piotr Piastucki

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "tlen.h"
#include "tlen_list.h"
#include "tlen_file.h"
#include "tlen_p2p_old.h"

typedef struct {
	TlenProtocol *proto;
	TLEN_LIST_ITEM *item;
} TLENPSREQUESTTHREADDATA;

static void LogPictureMessage(TlenProtocol *proto, const char *jid, const char *filename, BOOL isSent)
{
	char message[1024];
	const char *msg = isSent ? LPGEN("Image sent file://%s") : LPGEN("Image received file://%s");
	mir_snprintf(message, SIZEOF(message), Translate(msg), filename);
	TlenLogMessage(proto, TlenHContactFromJID(proto, jid), isSent ? DBEF_SENT : 0, message);
}

static void TlenPsPostThread(void *ptr) {
	TLENPSREQUESTTHREADDATA *data = (TLENPSREQUESTTHREADDATA *)ptr;
	TlenProtocol *proto = data->proto;
	TLEN_LIST_ITEM *item = data->item;
	HANDLE socket = TlenWsConnect(proto, "ps.tlen.pl", 443);
	BOOL bSent = FALSE;
	if (socket != NULL) {
		char header[512];
		DWORD ret;
		item->ft->s = socket;
		item->ft->hFileEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		ret = mir_snprintf(header, SIZEOF(header), "<pic auth='%s' t='p' to='%s' size='%d' idt='%s'/>", proto->threadData->username, item->ft->jid, item->ft->fileTotalSize, item->jid);
		TlenWsSend(proto, socket, header, (int)ret);
		ret = WaitForSingleObject(item->ft->hFileEvent, 1000 * 60 * 5);
		if (ret == WAIT_OBJECT_0) {
			FILE *fp = fopen( item->ft->files[0], "rb" );
			if (fp) {
				int i;
				char header[512];
				char fileBuffer[2048];
				i = mir_snprintf(header, SIZEOF(header), "<pic st='%s' idt='%s'/>", item->ft->iqId, item->jid);
				TlenWsSend(proto, socket, header, i);
				proto->debugLogA("Sending picture data...");
				for (i = item->ft->filesSize[0]; i > 0; ) {
					int toread = min(2048, i);
					int readcount = (int)fread(fileBuffer, (size_t)1, (size_t)toread, fp);
					i -= readcount;
					if (readcount > 0) {
						TlenWsSend(proto, socket, fileBuffer, readcount);
					}
					if (toread != readcount) {
						break;
					}
				}
				fclose(fp);
				SleepEx(3000, TRUE);
				bSent = TRUE;
			} else {
			  /* picture not found */
			}
		} else {
			/* 5 minutes passed */
		}
		Netlib_CloseHandle(socket);
		if (bSent) {
			TlenSend(proto, "<message to='%s' idt='%s' rt='%s' pid='1001' type='pic' />", item->ft->jid, item->jid, item->ft->id2);
			LogPictureMessage(proto, item->ft->jid, item->ft->files[0], TRUE);
		}
		TlenP2PFreeFileTransfer(item->ft);
		TlenListRemove(proto, LIST_PICTURE, item->jid);
	} else {
		/* cannot connect to ps server */
	}
	mir_free(data);
}

static void TlenPsPost(TlenProtocol *proto, TLEN_LIST_ITEM *item) {
	TLENPSREQUESTTHREADDATA *threadData = (TLENPSREQUESTTHREADDATA *)mir_alloc(sizeof(TLENPSREQUESTTHREADDATA));
	threadData->proto = proto;
	threadData->item = item;
	forkthread(TlenPsPostThread, 0, threadData);
}

static void TlenPsGetThread(void *ptr) {
	TLENPSREQUESTTHREADDATA *data = (TLENPSREQUESTTHREADDATA *)ptr;
	TlenProtocol *proto = data->proto;
	TLEN_LIST_ITEM *item = data->item;
	FILE *fp;
	fp = fopen( item->ft->files[0], "wb" );
	if (fp) {
		HANDLE socket = TlenWsConnect(proto, "ps.tlen.pl", 443);
		if (socket != NULL) {
			XmlState xmlState;
			char header[512];
			char fileBuffer[2048];
			TlenXmlInitState(&xmlState);
			int header_len = mir_snprintf(header, SIZEOF(header), "<pic auth='%s' t='g' to='%s' pid='1001' idt='%s' rt='%s'/>", proto->threadData->username, item->ft->jid, item->jid, item->ft->id2);
			TlenWsSend(proto, socket, header, header_len);
			proto->debugLogA("Reveiving picture data...");
			{
				int totalcount = 0;
				int size = item->ft->filesSize[0];
				BOOL bHeader = TRUE;
				while (TRUE) {
					int readcount = TlenWsRecv(proto, socket, fileBuffer, 2048 - totalcount);
					if (readcount == 0) {
						break;
					}
					totalcount += readcount;
					if (bHeader) {
						char * tagend = (char*)memchr(fileBuffer, '/', totalcount);
						tagend = (char*)memchr(tagend + 1, '>', totalcount - (tagend - fileBuffer) - 1);
						if (tagend != NULL) {
							int parsed = TlenXmlParse(&xmlState, fileBuffer, tagend - fileBuffer + 1);
							if (parsed == 0) {
								continue;
							}
							bHeader = FALSE;
							totalcount -= parsed;
							memmove(fileBuffer, fileBuffer+parsed, totalcount);
						}
					}
					if (!bHeader) {
						if (totalcount > 0) {
							fwrite(fileBuffer, 1, totalcount, fp);
							size -= totalcount;
							totalcount = 0;
						}
						if (size == 0) {
							break;
						}
					}
				}
			}
			Netlib_CloseHandle(socket);
			proto->debugLogA("Picture received...");
			LogPictureMessage(proto, item->ft->jid, item->ft->files[0], FALSE);
		} else {
		  /* cannot connect to ps server */
		}
		fclose(fp);
	} else {
		/* cannot create file */
	}
	TlenP2PFreeFileTransfer(item->ft);
	TlenListRemove(proto, LIST_PICTURE, item->jid);
	mir_free(data);
}

static void TlenPsGet(TlenProtocol *proto, TLEN_LIST_ITEM *item) {
	TLENPSREQUESTTHREADDATA *threadData = (TLENPSREQUESTTHREADDATA *)mir_alloc(sizeof(TLENPSREQUESTTHREADDATA));
	threadData->proto = proto;
	threadData->item = item;
	forkthread(TlenPsGetThread, 0, threadData);
}

void TlenProcessPic(XmlNode *node, TlenProtocol *proto) {
	TLEN_LIST_ITEM *item = NULL;
	char *crc, *crc_c, *idt, *size, *from, *fromRaw, *rt;
	from = TlenXmlGetAttrValue(node, "from");
	fromRaw = TlenLoginFromJID(from);
	idt = TlenXmlGetAttrValue(node, "idt");
	size = TlenXmlGetAttrValue(node, "size");
	crc_c = TlenXmlGetAttrValue(node, "crc_c");
	crc = TlenXmlGetAttrValue(node, "crc");
	rt = TlenXmlGetAttrValue(node, "rt");
	if (idt != NULL) {
		item = TlenListGetItemPtr(proto, LIST_PICTURE, idt);
	}
	if (item != NULL) {
		if (!strcmp(from, "ps")) {
			char *st = TlenXmlGetAttrValue(node, "st");
			if (st != NULL) {
				item->ft->iqId = mir_strdup(st);
				item->ft->id2 = mir_strdup(rt);
				if (item->ft->hFileEvent != NULL) {
					SetEvent(item->ft->hFileEvent);
					item->ft->hFileEvent = NULL;
				}
			}
		} else if (!strcmp(item->ft->jid, fromRaw)) {
			if (crc_c != NULL) {
				if (!strcmp(crc_c, "n")) {
					/* crc_c = n, picture transfer accepted */
					TlenPsPost(proto, item);
				} else if (!strcmp(crc_c, "f")) {
					/* crc_c = f, picture cached, no need to transfer again */
					LogPictureMessage(proto, item->ft->jid, item->ft->files[0], TRUE);
					TlenP2PFreeFileTransfer(item->ft);
					TlenListRemove(proto, LIST_PICTURE, idt);
				}
			} else if (rt != NULL) {
				item->ft->id2 = mir_strdup(rt);
				TlenPsGet(proto, item);
			}
		}
	} else if (crc != NULL) {
		BOOL bAccept = proto->tlenOptions.imagePolicy == TLEN_IMAGES_ACCEPT_ALL || (proto->tlenOptions.imagePolicy == TLEN_IMAGES_IGNORE_NIR && IsAuthorized(proto, from));
		if (bAccept) {
			FILE* fp;
			char fileName[MAX_PATH];
			char *ext = TlenXmlGetAttrValue(node, "ext");
			char *tmpPath = Utils_ReplaceVars( "%miranda_userdata%" );
			int tPathLen = mir_snprintf(fileName, SIZEOF(fileName), "%s\\Images\\Tlen", tmpPath);
			long oldSize = 0, lSize = atol(size);
			DWORD dwAttributes = GetFileAttributesA( fileName );
			if ( dwAttributes == 0xffffffff || ( dwAttributes & FILE_ATTRIBUTE_DIRECTORY ) == 0 )
				CreateDirectoryTree(fileName);

			mir_free(tmpPath);
			fileName[ tPathLen++ ] = '\\';
			mir_snprintf( fileName + tPathLen, SIZEOF(fileName) - tPathLen, "%s.%s", crc, ext );
			fp = fopen( fileName, "rb" );
			if (fp) {
				fseek(fp, 0, SEEK_END);
				oldSize = ftell(fp);
				fclose(fp);
			}
			if (oldSize != lSize) {
				item = TlenListAdd(proto, LIST_PICTURE, idt);
				item->ft = TlenFileCreateFT(proto, from);
				item->ft->files = (char **) mir_alloc(sizeof(char *));
				item->ft->filesSize = (long *) mir_alloc(sizeof(long));
				item->ft->files[0] = mir_strdup(fileName);
				item->ft->filesSize[0] = lSize;
				item->ft->fileTotalSize = item->ft->filesSize[0];
				TlenSend(proto, "<message type='pic' to='%s' crc_c='n' idt='%s'/>", from, idt);
			} else {
				TlenSend(proto, "<message type='pic' to='%s' crc_c='f' idt='%s'/>", from, idt);
				LogPictureMessage(proto, from, fileName, FALSE);
			}
		}
	}
	mir_free(fromRaw);
}

BOOL SendPicture(TlenProtocol *proto, MCONTACT hContact) {
	DBVARIANT dbv;
	if (!db_get(hContact, proto->m_szModuleName, "jid", &dbv)) {
		char *jid = dbv.pszVal;
		char szFilter[512];
		char *szFileName = (char*) mir_alloc(_MAX_PATH);
		OPENFILENAMEA ofn = {0};
		CallService(MS_UTILS_GETBITMAPFILTERSTRINGS, ( WPARAM ) sizeof( szFilter ), ( LPARAM )szFilter );
		ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
		ofn.hwndOwner = NULL;
		ofn.lpstrFilter = szFilter;
		ofn.lpstrCustomFilter = NULL;
		ofn.lpstrFile = szFileName;
		ofn.nMaxFile = _MAX_PATH;
		ofn.Flags = OFN_FILEMUSTEXIST;
		szFileName[0] = '\0';
		if ( GetOpenFileNameA( &ofn )) {
			long size;
			FILE* fp = fopen( szFileName, "rb" );
			if (fp) {
				fseek(fp, 0, SEEK_END);
				size = ftell(fp);
				if (size > 0 && size < 256*1024) {
					TLEN_LIST_ITEM *item;
					mir_sha1_ctx sha;
					DWORD digest[5];
					int i;
					char idStr[10];
					char fileBuffer[2048];
					int id = TlenSerialNext(proto);
					mir_snprintf(idStr, SIZEOF(idStr), "%d", id);
					item = TlenListAdd(proto, LIST_PICTURE, idStr);
					item->ft = TlenFileCreateFT(proto, jid);
					item->ft->files = (char **) mir_alloc(sizeof(char *));
					item->ft->filesSize = (long *) mir_alloc(sizeof(long));
					item->ft->files[0] = szFileName;
					item->ft->filesSize[0] = size;
					item->ft->fileTotalSize = size;
					fseek(fp, 0, SEEK_SET);
					mir_sha1_init( &sha );
					for (i = item->ft->filesSize[0]; i > 0; ) {
						int toread = min(2048, i);
						int readcount = (int)fread(fileBuffer, (size_t)1, (size_t)toread, fp);
						i -= readcount;
						if (readcount > 0) {
							mir_sha1_append( &sha, (BYTE* )fileBuffer, readcount);
						}
						if (toread != readcount) {
							break;
						}
					}
					mir_sha1_finish( &sha, (BYTE* )digest );
					TlenSend(proto, "<message type='pic' to='%s' crc='%08x%08x%08x%08x%08x' idt='%s' size='%d' ext='%s'/>", jid,
						(int)htonl(digest[0]), (int)htonl(digest[1]), (int)htonl(digest[2]), (int)htonl(digest[3]), (int)htonl(digest[4]), idStr, item->ft->filesSize[0], "jpg");
				} else {
					/* file too big */
				}
				fclose(fp);
			}
		}
		db_free(&dbv);
	}
	return FALSE;
}
