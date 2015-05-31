/*

Tlen Protocol Plugin for Miranda NG
Copyright (C) 2004-2007  Piotr Piastucki

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
#include "tlen_p2p_old.h"
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "tlen_list.h"


static void logInfo(const char *filename, const char *fmt, ...) {
	va_list vararg;
	FILE *flog=fopen(filename,"at");
	if (flog != NULL) {
		SYSTEMTIME time;
		GetLocalTime(&time);
		va_start(vararg, fmt);
		int strsize=2048;
		char *str = (char *) mir_alloc(strsize);
		while (mir_vsnprintf(str, strsize, fmt, vararg) == -1)
			str = (char *) realloc(str, strsize+=2048);
		va_end(vararg);
		fprintf(flog,"%04d-%02d-%02d %02d:%02d:%02d,%03d [%s]",time.wYear,time.wMonth,time.wDay,time.wHour,time.wMinute,time.wSecond,time.wMilliseconds, "INFO");
		fprintf(flog,"  %s\n",str);
		mir_free(str);
		fclose(flog);
	}
}

void __cdecl TlenNewFileReceiveThread(TLEN_FILE_TRANSFER *ft)
{
	ft->proto->debugLogA("P2P receive thread started");
	ProtoBroadcastAck(ft->proto->m_szModuleName, ft->hContact, ACKTYPE_FILE, ACKRESULT_CONNECTING, ft, 0);
//	ft->mode = FT_RECV;
//	ft->currentFile = 0;
//	ft->state = FT_CONNECTING;
	{
		FILE * fout = fopen("tlen_recv.dxx", "wt");
		SOCKADDR_IN toad;
		toad.sin_family = AF_INET;
		toad.sin_addr.s_addr = inet_addr(ft->hostName);
		toad.sin_port = htons(ft->wPort);
		if (fout != NULL) {
			fprintf(fout, "START:\n");
		}
		if (fout != NULL) {
			fclose(fout);
		}
		while (ft->udps != INVALID_SOCKET) {
			SOCKADDR_IN cad;
			int alen;
			int j, n;
			unsigned char buff[1024];
			alen = sizeof(struct sockaddr);
			n=recvfrom(ft->udps, (char*)buff,sizeof(buff),0, (struct sockaddr *) &cad, &alen);
			if (n<0) {
				break;
			}
			logInfo("tlen_recv.dxx", "UDP");
			fout = fopen("tlen_recv.dxx", "at");
			if (fout != NULL) {
				fprintf(fout, "\n|RECV %d bytes from %s:%d|",n, inet_ntoa(cad.sin_addr), cad.sin_port);
				for (j = 0; j < n; j++) {
					fprintf(fout, "%02X-", buff[j]);
				}
			}
			if (n == 1) {
				alen = sizeof(struct sockaddr);
				n = sendto(ft->udps, (char*)buff, n, 0,(struct sockaddr *) &toad, alen);
				if (fout != NULL) {
					fprintf(fout, "\n|SEND |");
					for (j = 0; j < n; j++) {
						fprintf(fout, "%02X-", buff[j]);
					}
				}
			}
			if (fout != NULL) {
				fprintf(fout, "\n");
				fclose(fout);
			}
		}
	}
	if (ft->udps != INVALID_SOCKET) {
		closesocket(ft->udps);
	}

	TlenListRemove(ft->proto, LIST_FILE, ft->iqId);
	if (ft->state == FT_DONE)
		ProtoBroadcastAck(ft->proto->m_szModuleName, ft->hContact, ACKTYPE_FILE, ACKRESULT_SUCCESS, ft, 0);
	else {
		ProtoBroadcastAck(ft->proto->m_szModuleName, ft->hContact, ACKTYPE_FILE, ACKRESULT_FAILED, ft, 0);
	}
	ft->proto->debugLogA("P2P receive thread ended");
	TlenP2PFreeFileTransfer(ft);
}

void __cdecl TlenNewFileSendThread(TLEN_FILE_TRANSFER *ft)
{
	ft->proto->debugLogA("P2P send thread started");
//	ft->mode = FT_RECV;
//	ProtoBroadcastAck(ft->proto->m_szModuleName, ft->hContact, ACKTYPE_FILE, ACKRESULT_CONNECTING, ft, 0);
//	ft->currentFile = 0;
//	ft->state = FT_CONNECTING;
	{
		FILE * fout = fopen("tlen_send.gxx", "wt");
		int step = 0;
		SOCKADDR_IN toad;
		toad.sin_family = AF_INET;
		toad.sin_addr.s_addr = inet_addr(ft->hostName);
		toad.sin_port = htons(ft->wPort);

		if (fout != NULL) {
			fprintf(fout, "START:");
		}
		for (step = 0; step < 10; step ++) {
//		while (ft->udps != INVALID_SOCKET) {
			int alen;
			int j, n;
			unsigned char buff[1024];
			alen = sizeof(struct sockaddr);
			if (step < 3 || step > 5) {
				buff[0] = 1;
				n = 1;
			} else {
				buff[0] = '$';
				buff[1] = '^';
				buff[2] = '&';
				buff[3] = '%';
				n = 4;
			}
			n=sendto(ft->udps, (char*)buff, n, 0, (struct sockaddr *)&toad, alen);
			logInfo("tlen_send.dxx", "UDP");
			if (fout != NULL) {
				fprintf(fout, "|send: %d %s %d|",n, inet_ntoa(toad.sin_addr), toad.sin_port);
				for (j = 0; j < n; j++) {
					fprintf(fout, "%02X-", buff[j]);
				}
			}
			if (fout != NULL) {
				fprintf(fout, "\n");
			}
			SleepEx(1000, TRUE);
		}
		if (fout != NULL) {
			fclose(fout);
		}
	}
	ft->proto->debugLogA("P2P send thread ended");
}

void TlenBindUDPSocket(TLEN_FILE_TRANSFER *ft)
{
	ft->proto->debugLogA("Binding UDP socket");
	ft->udps = socket(PF_INET, SOCK_DGRAM, 0);
	if (ft->udps != INVALID_SOCKET) {
		SOCKADDR_IN sin;
		int len = sizeof(struct sockaddr);
		memset((char *)&sin,0,sizeof(sin));
		sin.sin_family = AF_INET;
		sin.sin_addr.s_addr = htonl(INADDR_ANY);
		sin.sin_port = 0;
		if (bind(ft->udps, (struct sockaddr *)&sin, sizeof(sin)) >= 0) {
			if (!getsockname((SOCKET)ft->udps,(SOCKADDR *)&sin,&len)) {
				struct hostent *hp;
				char host_name[256];
				gethostname(host_name, sizeof(host_name));
				hp = gethostbyname(host_name);
				mir_snprintf(host_name, SIZEOF(host_name), "%u.%u.%u.%u", (unsigned char)hp->h_addr_list[0][0],
															(unsigned char)hp->h_addr_list[0][1],
															(unsigned char)hp->h_addr_list[0][2],
															(unsigned char)hp->h_addr_list[0][3]);

				ft->wLocalPort = ntohs(sin.sin_port);
				ft->localName= mir_strdup(host_name);
				ft->proto->debugLogA("UDP socket bound to %s:%d", ft->localName, ft->wLocalPort);
			}
		}
	}
}


/*
aaa sends a file to bbb:

1) aaa wants to SEND bbb a file
A SEND: <iq to='bbb@tlen.pl'><query xmlns='p2p'><fs t="bbb@tlen.pl" e="1" i="179610858" c="1" s="308278" v="2" /></query></iq>
B RECV: <iq from='aaa@tlen.pl'><query xmlns='p2p'><fs t="bbb@tlen.pl" e="1" i="179610858" c="1" s="308278" v="2" /></query></iq>

2) bbb ACCEPTs
B SEND: <iq to='aaa@tlen.pl'><query xmlns='p2p'><fs t="aaa@tlen.pl" e="5" i="179610858" v="2" /></query></iq>
A RECV: <iq from='bbb@tlen.pl'><query xmlns='p2p'><fs t="aaa@tlen.pl" e="5" i="179610858" v="2" /></query></iq>

3) aaa tries to establish p2p connection:
A SEND: <iq to='bbb@tlen.pl'><query xmlns='p2p'><dcng n='file_send' k='5' v='2' s='1' i='179610858' ck='rNbjnfRSQwJyaI+oRAXmfQ==' ks='16' iv='MAfaCCrqtuU2Ngw=rNbjnfRSQwJyaI+oRAXmfQ==' mi='24835584'/></query></iq>
B SEND: <iq from='aaa@tlen.pl'><query xmlns='p2p'><dcng n='file_send' k='5' v='2' s='1' i='179610858' ck='rNbjnfRSQwJyaI+oRAXmfQ==' ks='16' iv='MAfaCCrqtuU2Ngw=rNbjnfRSQwJyaI+oRAXmfQ==' mi='24835584'/></query></iq>

3) bbb sends IP/port details
B SEND: <iq to='bbb@tlen.pl'><query xmlns='p2p'><dcng la='xx.xx.xx.xx' lp='nnn' pa='xx.xx.xx.xx' pp='nnn' i='179610858' v='2' k='5' s='2'/></query></iq>

*/

void __cdecl TlenProcessP2P(XmlNode *node, ThreadData *info) {
	XmlNode *queryNode;
	TLEN_LIST_ITEM *item;
	char *from;

	if (info == NULL) return;

	queryNode = TlenXmlGetChild(node, "query");
	if ((from=TlenXmlGetAttrValue(node, "from")) != NULL) {
		XmlNode *fs , *vs, *dcng, *dc;
		/* file send */
		fs = TlenXmlGetChild(queryNode, "fs");
		/* voice send */
		vs  = TlenXmlGetChild(queryNode, "vs");
		dcng  = TlenXmlGetChild(queryNode, "dcng");
		dc  = TlenXmlGetChild(queryNode, "dc");
		if (fs  != NULL) {
			char *e, *id;
			/* e - step in the process (starting with 1)*/
			/* i - id of the file */
			/* s - size of the file */
			/* c - number of files */
			/* v - ??? */
			e = TlenXmlGetAttrValue(fs, "e");
			id = TlenXmlGetAttrValue(fs, "i");
			if (e != NULL) {
				if (!mir_strcmp(e, "1")) {
					TLEN_FILE_TRANSFER * ft = (TLEN_FILE_TRANSFER *) mir_alloc(sizeof(TLEN_FILE_TRANSFER));
					memset(ft, 0, sizeof(TLEN_FILE_TRANSFER));
					char *c = TlenXmlGetAttrValue(fs, "c");
					char *s = TlenXmlGetAttrValue(fs, "s");
					ft->jid = mir_strdup(from);
					ft->proto = info->proto;
					ft->hContact = TlenHContactFromJID(info->proto, from);
					ft->iqId = mir_strdup(id);
					ft->fileTotalSize = atoi(s);
					ft->newP2P = TRUE;
					if ((item=TlenListAdd(ft->proto, LIST_FILE, ft->iqId)) != NULL) {
						char fileInfo[128];
						item->ft = ft;
						mir_snprintf(fileInfo, SIZEOF(fileInfo), "%s file(s), %s bytes", c, s);
						TCHAR* filenameT = mir_utf8decodeT((char*)fileInfo);
						PROTORECVFILET pre = { 0 };
						pre.dwFlags = PRFF_TCHAR;
						pre.fileCount = 1;
						pre.timestamp = time(NULL);
						pre.descr.t = filenameT;
						pre.files.t = &filenameT;
						pre.lParam = (LPARAM)ft;
						ft->proto->debugLogA("sending chainrecv");
						ProtoChainRecvFile(ft->hContact, &pre);
						mir_free(filenameT);
					}
				} else if (!mir_strcmp(e, "3")) {
					/* transfer error */
				} else if (!mir_strcmp(e, "4")) {
					/* transfer denied */
				} else if (!mir_strcmp(e, "5")) {
					/* transfer accepted */
					if ((item=TlenListGetItemPtr(info->proto, LIST_FILE, id)) != NULL) {
						item->id2 = mir_strdup("84273372");
						item->ft->id2 = mir_strdup("84273372");
						TlenSend(info->proto, "<iq to='%s'><query xmlns='p2p'><dcng n='file_send' k='5' v='2' s='1' i='%s' ck='o7a32V9n2UZYCWpBUhSbFw==' ks='16' iv='MhjWEj9WTsovrQc=o7a32V9n2UZYCWpBUhSbFw==' mi='%s'/></query></iq>", from, item->id2, id);
					}
				}
			}
		} else if (vs != NULL) {

		} else if (dcng != NULL) {
			info->proto->debugLogA("DCNG");
			char *s = TlenXmlGetAttrValue(dcng, "s");
			char *id2 = TlenXmlGetAttrValue(dcng, "i");
			char *id = TlenXmlGetAttrValue(dcng, "mi");
			if (!mir_strcmp(s, "1")) {
				/* Keys */
				/* s - step */
				/* i - id of the file */
				/* ks - key size (in bytes) */
				/* mi - p2p connection id */
				char *n = TlenXmlGetAttrValue(dcng, "n"); // n - name (file_send)
				char *k = TlenXmlGetAttrValue(dcng, "k"); // k - ???
				char *v = TlenXmlGetAttrValue(dcng, "v"); // v - ???
				char *ck = TlenXmlGetAttrValue(dcng, "ck"); // ck - aes key
				char *iv = TlenXmlGetAttrValue(dcng, "iv"); // iv - aes initial vector
				if (!mir_strcmp(n, "file_send")) {
					if ((item=TlenListGetItemPtr(info->proto, LIST_FILE, id)) != NULL) {
						item->id2 = mir_strdup(id2);
						item->ft->id2 = mir_strdup(id2);
						TlenBindUDPSocket(item->ft);
						TlenSend(info->proto, "<iq to='%s'><query xmlns='p2p'><dcng  la='%s' lp='%d' pa='%s' pp='%d' i='%s' v='2' k='5' s='2'/></query></iq>",
							item->ft->jid, item->ft->localName, item->ft->wLocalPort, item->ft->localName, item->ft->wLocalPort, item->ft->id2);
					}
				}
			}  else if (!mir_strcmp(s, "2")) {
				info->proto->debugLogA("step = 2");
				info->proto->debugLogA("%s",from);
				info->proto->debugLogA("%s",id2);
				/* IP and port */
				if ((item=TlenListFindItemPtrById2(info->proto, LIST_FILE, id2)) != NULL) {
					item->ft->hostName = mir_strdup(TlenXmlGetAttrValue(dcng, "pa"));
					item->ft->wPort = atoi(TlenXmlGetAttrValue(dcng, "pp"));
					TlenBindUDPSocket(item->ft);
					TlenSend(info->proto, "<iq to='%s'><query xmlns='p2p'><dcng  la='%s' lp='%d' pa='%s' pp='%d' i='%s' k='5' s='4'/></query></iq>",
						item->ft->jid, item->ft->localName, item->ft->wLocalPort, item->ft->localName, item->ft->wLocalPort, item->ft->id2);
					forkthread((void (__cdecl *)(void*))TlenNewFileReceiveThread, 0, item->ft);
					forkthread((void (__cdecl *)(void*))TlenNewFileSendThread, 0, item->ft);
				}
			} else if (!mir_strcmp(s, "4")) {
				/* IP and port */
				if ((item=TlenListFindItemPtrById2(info->proto, LIST_FILE, id2)) != NULL) {
					info->proto->debugLogA("step = 4");
					item->ft->hostName = mir_strdup(TlenXmlGetAttrValue(dcng, "pa"));
					item->ft->wPort = atoi(TlenXmlGetAttrValue(dcng, "pp"));
					forkthread((void (__cdecl *)(void*))TlenNewFileReceiveThread, 0, item->ft);
				}
			}

		} else if (dc != NULL) {

		}
	}
}
