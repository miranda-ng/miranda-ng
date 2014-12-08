/*
 * $Id: file_transfer.cpp 13893 2011-10-23 19:29:28Z borkra $
 *
 * myYahoo Miranda Plugin 
 *
 * Authors: Gennady Feldman (aka Gena01) 
 *          Laurent Marechal (aka Peorth)
 *
 * This code is under GPL and is based on AIM, MSN and Miranda source code.
 * I want to thank Robert Rainwater and George Hazan for their code and support
 * and for answering some of my questions during development of this plugin.
 */
#include <time.h>
#include <sys/stat.h>

#include "yahoo.h"
#include <m_protosvc.h>
#include "file_transfer.h"

YList *file_transfers=NULL;

static y_filetransfer* new_ft(CYahooProto* ppro, int id, MCONTACT hContact, const char *who, const char *msg,  
					const char *url, const char *ft_token, int y7, YList *fs, int sending)
{
	yahoo_file_info * fi;
	int i=0;
	YList *l=fs;

	LOG(("[new_ft] id: %d, who: %s, msg: %s, ft_token: %s, y7: %d, sending: %d", id, who, msg, ft_token, y7, sending));

	y_filetransfer* ft = (y_filetransfer*) calloc(1, sizeof(y_filetransfer));
	ft->ppro = ppro;
	ft->id  = id;
	ft->who = strdup(who);
	ft->hWaitEvent = INVALID_HANDLE_VALUE;

	ft->hContact = hContact;
	ft->files = fs;

	ft->url = (url == NULL) ? NULL : strdup(url);
	ft->ftoken = (ft_token == NULL) ? NULL : strdup(ft_token);
	ft->msg = (msg != NULL) ? strdup(msg) : strdup("[no description given]");

	ft->cancel = 0;
	ft->y7 = y7;
	
	ft->hWaitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	ft->pfts.cbSize = sizeof(PROTOFILETRANSFERSTATUS);
	ft->pfts.hContact = hContact;
	ft->pfts.flags =  PFTS_TCHAR; 
	ft->pfts.flags |= (sending != 0) ? PFTS_SENDING : PFTS_RECEIVING;
	
	ft->pfts.tszWorkingDir = NULL;
	ft->pfts.currentFileTime = 0;

	ft->pfts.totalFiles = y_list_length(fs);

	ft->pfts.ptszFiles = (TCHAR**) mir_calloc(ft->pfts.totalFiles * sizeof(TCHAR *));
	ft->pfts.totalBytes = 0;

	while(l) {
		fi = ( yahoo_file_info* )l->data;

		ft->pfts.ptszFiles[i++] = mir_utf8decodeT(fi->filename);
		ft->pfts.totalBytes += fi->filesize;

		l=l->next;
	}

	ft->pfts.currentFileNumber = 0;

	fi = ( yahoo_file_info* )fs->data; 
	ft->pfts.tszCurrentFile = _tcsdup(ft->pfts.ptszFiles[ft->pfts.currentFileNumber]);
	ft->pfts.currentFileSize = fi->filesize; 

	file_transfers = y_list_prepend(file_transfers, ft);

	LOG(("[/new_ft]"));
	
	return ft;
}

y_filetransfer* find_ft(const char *ft_token, const char *who) 
{
	YList *l;
	y_filetransfer* f;
	LOG(("[find_ft] Searching for: %s", ft_token));
	
	for(l = file_transfers; l; l = y_list_next(l)) {
		f = (y_filetransfer* )l->data;
		if (mir_strcmp(f->ftoken, ft_token) == 0 && mir_strcmp(f->who, who) == 0) {
			LOG(("[find_ft] Got it!"));
			return f;
		}
	}

	LOG(("[find_ft] FT not found?"));
	return NULL;
}

static void free_ft(y_filetransfer* ft)
{
	YList *l;
	int i;
	
	LOG(("[free_ft] token: %s", ft->ftoken));
	
	for(l = file_transfers; l; l = y_list_next(l)) {
		if (l->data == ft) {
			LOG(("[free_ft] Ft found and removed from the list"));
			file_transfers = y_list_remove_link(file_transfers, l);
			y_list_free_1(l);
			break;
		}
	}

	if ( ft->hWaitEvent != INVALID_HANDLE_VALUE )
		CloseHandle( ft->hWaitEvent );
	
	FREE(ft->who);
	FREE(ft->msg);
	FREE(ft->url);
	FREE(ft->ftoken);
	FREE(ft->relay);
	
	LOG(("[free_ft] About to free the File List."));
	
	while(ft->files) {
		YList *tmp = ft->files;
		yahoo_file_info * c = ( yahoo_file_info* )ft->files->data;
		FREE(c->filename);
		FREE(c);
		ft->files = y_list_remove_link(ft->files, ft->files);
		y_list_free_1(tmp);
	}
	
	LOG(("[free_ft] About to free PFTS."));
	
	for (i=0; i< ft->pfts.totalFiles; i++)
		mir_free(ft->pfts.ptszFiles[i]);
	
	mir_free(ft->pfts.ptszFiles);
	FREE(ft->pfts.tszCurrentFile);
	FREE(ft->pfts.tszWorkingDir);
	FREE(ft);
	
	LOG(("[/free_ft]"));
}

static void upload_file(int id, INT_PTR fd, int error, void *data) 
{
	y_filetransfer *sf = (y_filetransfer*) data;
	struct yahoo_file_info *fi = (struct yahoo_file_info *)sf->files->data;
	char buf[1024];
	unsigned long size = 0;
	DWORD dw = 0;
	int   rw = 0;

	if (fd < 0) {
		LOG(("[get_fd] Connect Failed!"));
		error = 1;
	}

	if (!error) {
		HANDLE myhFile = CreateFile(sf->pfts.tszCurrentFile,
			GENERIC_READ,
			FILE_SHARE_READ|FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
			0);

		if (myhFile !=INVALID_HANDLE_VALUE) {
			DWORD lNotify = GetTickCount();

			LOG(("proto: %s, hContact: %p", sf->ppro->m_szModuleName, sf->hContact));

			LOG(("Sending file: %s", fi->filename));
			ProtoBroadcastAck(sf->ppro->m_szModuleName, sf->hContact, ACKTYPE_FILE, ACKRESULT_CONNECTED, sf, 0);

			do {
				ReadFile(myhFile, buf, sizeof(buf), &dw, NULL);

				if (dw) {
					rw = Netlib_Send((HANDLE)fd, buf, dw, MSG_NODUMP);

					if (rw < 1) {
						LOG(("Upload Failed. Send error? Got: %d", rw));
						error = 1;
						break;
					} else 
						size += rw;

					if (GetTickCount() >= lNotify + 500 || rw < 1024 || size == fi->filesize) {
						LOG(("DOING UI Notify. Got %lu/%lu", size, fi->filesize));
						sf->pfts.totalProgress = size;
						sf->pfts.currentFileProgress = size;

						ProtoBroadcastAck(sf->ppro->m_szModuleName, sf->hContact, ACKTYPE_FILE, ACKRESULT_DATA, sf, (LPARAM) & sf->pfts);
						lNotify = GetTickCount();
					}

				}

				if (sf->cancel) {
					LOG(("Upload Cancelled! "));
					error = 1;
					break;
				}
			} while ( rw > 0 && dw > 0 && !error);

			CloseHandle(myhFile);

			sf->pfts.totalProgress = size;
			sf->pfts.currentFileProgress = size;

			ProtoBroadcastAck(sf->ppro->m_szModuleName, sf->hContact, ACKTYPE_FILE, ACKRESULT_DATA, sf, (LPARAM) & sf->pfts);

		}
	}	

	if (fd > 0) {
		int tr = 0;

		do {
			rw = Netlib_Recv((HANDLE)fd, buf, sizeof(buf), 0);
			LOG(("Got: %d bytes", rw));

			if (tr == 0) {
				//"HTTP/1.1 999" 12
				// 012345678901
				if (rw > 12) {
					if (buf[9] != '2' || buf[10] != '0' || buf[11] != '0') {
						LOG(("File Transfer Failed: %c%c%c", buf[9], buf[10], buf[11]));
						error=1;
					}
				}
			}
			tr +=rw;
		} while (rw > 0);

		Netlib_CloseHandle((HANDLE)fd);
	}

	LOG(("File send complete!"));

	if (! error) {
		sf->pfts.currentFileNumber++;

		if (sf->pfts.currentFileNumber >= sf->pfts.totalFiles) {
			ProtoBroadcastAck(sf->ppro->m_szModuleName, sf->hContact, ACKTYPE_FILE, ACKRESULT_SUCCESS, sf, 0);
		} else {
			YList *l;
			struct yahoo_file_info * fi;
			
			// Do Next file
			FREE(sf->pfts.tszCurrentFile);
			
			l = sf->files;
			
			fi = ( yahoo_file_info* )l->data;
			FREE(fi->filename);
			FREE(fi);
			
			sf->files = y_list_remove_link(sf->files, l);
			y_list_free_1(l);
			
			// need to move to the next file on the list and fill the file information
			fi = ( yahoo_file_info* )sf->files->data; 
			sf->pfts.tszCurrentFile = _tcsdup(sf->pfts.ptszFiles[sf->pfts.currentFileNumber]);
			sf->pfts.currentFileSize = fi->filesize; 
			sf->pfts.currentFileProgress = 0;
			
			ProtoBroadcastAck(sf->ppro->m_szModuleName, sf->hContact, ACKTYPE_FILE, ACKRESULT_NEXTFILE, sf, 0);
			LOG(("Waiting for next file request packet..."));
		}

	} else {
		ProtoBroadcastAck(sf->ppro->m_szModuleName, sf->hContact, ACKTYPE_FILE, ACKRESULT_FAILED, sf, 0);
	}
}

static void dl_file(int id, INT_PTR fd, int error,	const char *filename, unsigned long size, void *data) 
{
    y_filetransfer *sf = (y_filetransfer*) data;
	struct yahoo_file_info *fi = (struct yahoo_file_info *)sf->files->data;
    char buf[1024];
    unsigned long rsize = 0;
	DWORD dw, c;

	if (fd < 0) {
		LOG(("[get_url] Connect Failed!"));
		
		if (sf->ftoken != NULL) {
			LOG(("[get_url] DC Detected: asking sender to upload to Yahoo FileServers!"));
			yahoo_ftdc_deny(id, sf->who, fi->filename, sf->ftoken, 3);	
		}
		
		error = 1;
	}
	
    if (!error) {
		HANDLE myhFile;
		TCHAR filefull[MAX_PATH];
		
		/*
		 * We need FULL Path for File Resume to work properly!!!
		 *
		 * Don't rely on workingDir to be right, since it's not used to check if file exists.
		 */
		mir_sntprintf(filefull, MAX_PATH, _T("%s\\%s"), sf->pfts.tszWorkingDir, sf->pfts.tszCurrentFile);
		FREE(sf->pfts.tszCurrentFile);
		sf->pfts.tszCurrentFile = _tcsdup(filefull);		
		
		ResetEvent(sf->hWaitEvent);
		
		if ( sf->ppro->ProtoBroadcastAck( sf->hContact, ACKTYPE_FILE, ACKRESULT_FILERESUME, sf, (LPARAM)&sf->pfts )) {
			WaitForSingleObject( sf->hWaitEvent, INFINITE );
			
			LOG(("[dl_file] Got action: %ld", sf->action));
			
			switch(sf->action) {
				case FILERESUME_RENAME:
				case FILERESUME_OVERWRITE:	
				case FILERESUME_RESUME:	
					// no action needed at this point, just break out of the switch statement
					break;

				case FILERESUME_CANCEL	:
					sf->cancel = 1;
					break;

				case FILERESUME_SKIP	:
				default:
					sf->cancel = 2;
					break;
				}
		}

		
		
		if (! sf->cancel) {
			
			myhFile    = CreateFile(sf->pfts.tszCurrentFile,
									GENERIC_WRITE,
									FILE_SHARE_WRITE,
									NULL, OPEN_ALWAYS,  FILE_ATTRIBUTE_NORMAL,  0);
	
			if (myhFile !=INVALID_HANDLE_VALUE) {
				DWORD lNotify = GetTickCount();
				
				SetEndOfFile(myhFile);
				
				LOG(("proto: %s, hContact: %p", sf->ppro->m_szModuleName, sf->hContact));
				
				ProtoBroadcastAck(sf->ppro->m_szModuleName, sf->hContact, ACKTYPE_FILE, ACKRESULT_CONNECTED, sf, 0);
				
				do {
					dw = Netlib_Recv((HANDLE)fd, buf, 1024, MSG_NODUMP);
				
					if (dw > 0) {
						WriteFile(myhFile, buf, dw, &c, NULL);
						rsize += dw;
						sf->pfts.totalProgress += dw;
						sf->pfts.currentFileProgress += dw;
						
						if (GetTickCount() >= lNotify + 500 || dw <= 0 || rsize == size) {
							
							LOG(("DOING UI Notify. Got %lu/%lu", rsize, size));
							
							ProtoBroadcastAck(sf->ppro->m_szModuleName, sf->hContact, ACKTYPE_FILE, ACKRESULT_DATA, sf, (LPARAM) & sf->pfts);
							lNotify = GetTickCount();
						}
						
					} else {
						LOG(("Recv Failed! Socket Error?"));
						error = 1;
						break;
					}
					
					if (sf->cancel) {
						LOG(("Recv Cancelled! "));
						error = 1;
						break;
					}
				} while ( dw > 0 && rsize < size);
				
				while (dw > 0 && ! sf->cancel && ! error) {
					dw = Netlib_Recv((HANDLE)fd, buf, 1024, MSG_NODUMP);
					LOG(("Ack."));
				}
				
				ProtoBroadcastAck(sf->ppro->m_szModuleName, sf->hContact, ACKTYPE_FILE, ACKRESULT_DATA, sf, (LPARAM) & sf->pfts);
				
				LOG(("[Finished DL] Got %lu/%lu", rsize, size));
				CloseHandle(myhFile);
				
			} else {
				LOG(("Can not open file for writing: %s", buf));
				error = 1;
			}
			
		} 
    }
	
	if (fd > 0) {
		LOG(("Closing connection: %d", fd));
		Netlib_CloseHandle((HANDLE)fd);
	
		if (sf->cancel || error) {
			/* abort FT transfer */
			yahoo_ft7dc_abort(id, sf->who, sf->ftoken);
		}
	}
	
    if (! error) {
		sf->pfts.currentFileNumber++;
		
		LOG(("File %d/%d download complete!", sf->pfts.currentFileNumber, sf->pfts.totalFiles));
		
		if (sf->pfts.currentFileNumber >= sf->pfts.totalFiles) {
			ProtoBroadcastAck(sf->ppro->m_szModuleName, sf->hContact, ACKTYPE_FILE, ACKRESULT_SUCCESS, sf, 0);
		} else {
			YList *l;
			struct yahoo_file_info * fi;
			
			// Do Next file
			yahoo_ft7dc_nextfile(id, sf->who, sf->ftoken);
			FREE(sf->pfts.tszCurrentFile);
			
			l = sf->files;
			
			fi = ( yahoo_file_info* )l->data;
			FREE(fi->filename);
			FREE(fi);
			
			sf->files = y_list_remove_link(sf->files, l);
			y_list_free_1(l);
			
			// need to move to the next file on the list and fill the file information
			fi = ( yahoo_file_info* )sf->files->data; 
			sf->pfts.tszCurrentFile = _tcsdup(sf->pfts.ptszFiles[sf->pfts.currentFileNumber]);
			sf->pfts.currentFileSize = fi->filesize; 
			sf->pfts.currentFileProgress = 0;
			
			ProtoBroadcastAck(sf->ppro->m_szModuleName, sf->hContact, ACKTYPE_FILE, ACKRESULT_NEXTFILE, sf, 0);
		}
	} else {
		LOG(("File download failed!"));
		
		ProtoBroadcastAck(sf->ppro->m_szModuleName, sf->hContact, ACKTYPE_FILE, ACKRESULT_FAILED, sf, 0);
	}
}

//=======================================================
//File Transfer
//=======================================================
void __cdecl CYahooProto::recv_filethread(void *psf) 
{
	y_filetransfer *sf = ( y_filetransfer* )psf;
	struct yahoo_file_info *fi = (struct yahoo_file_info *)sf->files->data;
	
	ProtoBroadcastAck(sf->hContact, ACKTYPE_FILE, ACKRESULT_CONNECTING, sf, 0);
	
	debugLogA("[yahoo_recv_filethread] who: %s, msg: %s, filename: %s ", sf->who, sf->msg, fi->filename);
	
	
	yahoo_get_url_handle(m_id, sf->url, &dl_file, sf);
	
	if (sf->pfts.currentFileNumber >= sf->pfts.totalFiles)
		free_ft(sf);
	else
		debugLogA("[yahoo_recv_filethread] More files coming?");
}

void CYahooProto::ext_got_file(const char *me, const char *who, const char *url, long expires, const char *msg, const char *fname, unsigned long fesize, const char *ft_token, int y7)
{
	LOG(("[ext_yahoo_got_file] ident:%s, who: %s, url: %s, expires: %lu, msg: %s, fname: %s, fsize: %lu ftoken: %s y7: %d", me, who, url, expires, msg, fname, fesize, ft_token == NULL ? "NULL" : ft_token, y7));

	MCONTACT hContact = getbuddyH(who);
	if (hContact == NULL) 
		hContact = add_buddy(who, who, 0 /* NO FT for other IMs */, PALF_TEMPORARY);

	char fn[1024];
	memset(fn, 0, sizeof(fn));

	if (fname != NULL)
		mir_strncpy(fn, fname, 1024);
	else {
		char *start, *end;

		/* based on how gaim does this */
		start = ( char* )strrchr(url, '/');
		if (start)
			start++;

		end = ( char* )strrchr(url, '?');

		if (start && *start && end) {
			mir_strncpy(fn, start, end-start+1);
		} else 
			mir_strcpy(fn, "filename.ext");
	}

	yahoo_file_info *fi = y_new(struct yahoo_file_info,1);
	fi->filename = strdup(fn);
	fi->filesize = fesize;

	YList *files = NULL;
	y_list_append(files, fi);

	y_filetransfer *ft = new_ft(this, m_id, hContact, who, msg,	url, ft_token, y7, files, 0 /* downloading */);
	if (ft == NULL) {
		debugLogA("SF IS NULL!!!");
		return;
	}

	TCHAR* ptszFileName = mir_a2t(fn);

	PROTORECVFILET pre = {0};
	pre.flags = PREF_TCHAR;
	pre.fileCount = 1;
	pre.timestamp = time(NULL);
	pre.tszDescription = mir_a2t(msg);
	pre.ptszFiles = &ptszFileName;
	pre.lParam = (LPARAM)ft;
	ProtoChainRecvFile(hContact, &pre);

	mir_free(pre.tszDescription);
	mir_free(ptszFileName);
}

void CYahooProto::ext_got_files(const char *me, const char *who, const char *ft_token, int y7, YList* files)
{
	MCONTACT hContact;
	y_filetransfer *ft;
	YList *f;
	char fn[4096];
	int fc = 0;

	LOG(("[ext_yahoo_got_files] ident:%s, who: %s, ftoken: %s ", me, who, ft_token == NULL ? "NULL" : ft_token));

	hContact = getbuddyH(who);
	if (hContact == NULL) 
		hContact = add_buddy(who, who, 0 /* NO FT for other IMs */, PALF_TEMPORARY);

	ft = new_ft(this, m_id, hContact, who, NULL, NULL, ft_token, y7, files, 0 /* downloading */);
	if (ft == NULL) {
		debugLogA("SF IS NULL!!!");
		return;
	}

	fn[0] = '\0';
	for (f=files; f; f = y_list_next(f)) {
		char z[1024];
		struct yahoo_file_info *fi = (struct yahoo_file_info *) f->data;

		mir_snprintf(z, SIZEOF(z), "%s (%lu)\r\n", fi->filename, fi->filesize);
		mir_strcat(fn, z);
		fc++;
	}

	if (fc > 1) {
		/* multiple files */

	}

	TCHAR* ptszFileName = mir_a2t(fn);

	PROTORECVFILET pre = {0};
	pre.flags = PREF_TCHAR;
	pre.fileCount = 1;
	pre.timestamp = time(NULL);
	pre.tszDescription = _T("");
	pre.ptszFiles = &ptszFileName;
	pre.lParam = (LPARAM)ft;
	ProtoChainRecvFile(ft->hContact, &pre);

	mir_free(ptszFileName);
}

void CYahooProto::ext_got_file7info(const char *me, const char *who, const char *url, const char *fname, const char *ft_token)
{
	y_filetransfer *ft;

	LOG(("[ext_yahoo_got_file7info] ident:%s, who: %s, url: %s, fname: %s, ft_token: %s", me, who, url, fname, ft_token));
	
	ft = find_ft(ft_token, who);
	
	if (ft == NULL) {
		LOG(("ERROR: Can't find the token: %s in my file transfers list...", ft_token));
		return;
	}
	
	ProtoBroadcastAck(ft->hContact, ACKTYPE_GETINFO, ACKRESULT_SUCCESS, (HANDLE) 1, 0);
	
	FREE(ft->url);
	
	ft->url = strdup(url);
	
	ForkThread(&CYahooProto::recv_filethread, ft);
}

void ext_yahoo_send_file7info(int id, const char *me, const char *who, const char *ft_token)
{
	y_filetransfer *ft;
	yahoo_file_info *fi;
	
	char *c;
	LOG(("[ext_yahoo_send_file7info] id: %i, ident:%s, who: %s, ft_token: %s", id, me, who, ft_token));
	
	ft = find_ft(ft_token, who);
	
	if (ft == NULL) {
		LOG(("ERROR: Can't find the token: %s in my file transfers list...", ft_token));
		return;
	}
	
	fi = (yahoo_file_info *) ft->files->data;
	
	c = strrchr(fi->filename, '\\');
	if (c != NULL) {
		c++;
	} else {
		c = fi->filename;
	}
	
	LOG(("Resolving relay.msg.yahoo.com..."));
	PHOSTENT he = gethostbyname("relay.msg.yahoo.com");
	
	if (he) {
		ft->relay = strdup( inet_ntoa(*( PIN_ADDR )he->h_addr_list[0]));
		LOG(("Got Relay IP: %s", ft->relay));
	} else {
		ft->relay = strdup("98.136.112.33");
		LOG(("DNS Lookup failed. Using Relay IP: %s", ft->relay));
	}
	
	yahoo_send_file7info(id, me, who, ft_token, c, ft->relay );
}

struct _sfs{
	char *me;
	char *token;
	y_filetransfer *sf;
};

void CYahooProto::ext_ft7_send_file(const char *me, const char *who, const char *filename, const char *token, const char *ft_token)
{
	y_filetransfer *sf;
	struct _sfs *s;
	
	LOG(("[ext_yahoo_send_file7info] ident:%s, who: %s, ft_token: %s", me, who, ft_token));
	
	sf = find_ft(ft_token, who);
	
	if (sf == NULL) {
		LOG(("ERROR: Can't find the token: %s in my file transfers list...", ft_token));
		return;
	}

	s = (struct _sfs *) malloc( sizeof( struct _sfs ));
	
	s->me = strdup(me);
	s->token = strdup(token);
	s->sf = sf;
	
	ForkThread(&CYahooProto::send_filethread, s);
}

/**************** Send File ********************/

void __cdecl CYahooProto::send_filethread(void *psf) 
{
	struct _sfs *s = ( struct _sfs * )psf;
	y_filetransfer *sf = s->sf;
	struct yahoo_file_info *fi = (struct yahoo_file_info *)sf->files->data;
	
	ProtoBroadcastAck(sf->hContact, ACKTYPE_FILE, ACKRESULT_CONNECTING, sf, 0);
	
	LOG(("who %s, msg: %s, filename: %s filesize: %ld", sf->who, sf->msg, fi->filename, fi->filesize));
	
	yahoo_send_file_y7(sf->id, s->me, sf->who, sf->relay, fi->filesize, s->token,  &upload_file, sf);
	
	FREE(s->me);
	FREE(s->token);
	FREE(s);
	
	if (sf->pfts.currentFileNumber >= sf->pfts.totalFiles) {
		free_ft(sf);
	} else {
		debugLogA("[yahoo_send_filethread] More files coming?");
	}

}


////////////////////////////////////////////////////////////////////////////////////////
// SendFile - sends a file

HANDLE __cdecl CYahooProto::SendFile(MCONTACT hContact, const PROTOCHAR* szDescription, PROTOCHAR** ppszFiles )
{
	DBVARIANT dbv;
	y_filetransfer *sf;
	
	LOG(("[YahooSendFile]"));
	
	if ( !m_bLoggedIn )
		return 0;

	if (!getString(hContact, YAHOO_LOGINID, &dbv)) {
		long tFileSize = 0;
		struct _stat statbuf;
		struct yahoo_file_info *fi;
		YList *fs=NULL;
		int i=0;
		char *s;
	
		while (ppszFiles[i] != NULL) {
			if ( _tstat( ppszFiles[i], &statbuf ) == 0)
				tFileSize = statbuf.st_size;
	
			fi = y_new(struct yahoo_file_info,1);
			
			/**
			 * Need to use regular memory allocator/deallocator, since this is how things are build w/ libyahoo2
			 */
			s = mir_utf8encodeT(ppszFiles[i]);
			fi->filename = strdup(s);
			mir_free(s);
			
			fi->filesize = tFileSize;
		
			fs = y_list_append(fs, fi);
			i++;
		}
	
		sf = new_ft(this, m_id, hContact, dbv.pszVal, ( char* )szDescription,
					NULL, NULL, 0, fs, 1 /* sending */);
					
		db_free(&dbv);
		
		if (sf == NULL) {
			debugLogA("SF IS NULL!!!");
			return 0;
		}

		LOG(("who: %s, msg: %s, # files: %d", sf->who, sf->msg, i));
		
		sf->ftoken=yahoo_ft7dc_send(m_id, sf->who, fs);
		
		LOG(("Exiting SendRequest..."));
		
		return sf;
	}
	
	LOG(("[/YahooSendFile]"));
	
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// FileAllow - starts a file transfer

HANDLE __cdecl CYahooProto::FileAllow(MCONTACT hContact, HANDLE hTransfer, const PROTOCHAR* szPath)
{
	y_filetransfer *ft = (y_filetransfer *)hTransfer;
	size_t len;

	debugLogA("[YahooFileAllow]");

	//LOG(LOG_INFO, "[%s] Requesting file from %s", ft->cookie, ft->user);
	ft->pfts.tszWorkingDir = _tcsdup( szPath );

	len = _tcslen(ft->pfts.tszWorkingDir) - 1;
	if (ft->pfts.tszWorkingDir[len] == '\\')
		ft->pfts.tszWorkingDir[len] = 0;
		
	if (ft->y7) {
		debugLogA("[YahooFileAllow] Trying to relay Y7 transfer.");
		//void yahoo_ft7dc_accept(int id, const char *buddy, const char *ft_token);
		yahoo_ft7dc_accept(ft->id, ft->who, ft->ftoken);

		return hTransfer;
	}

	ForkThread(&CYahooProto::recv_filethread, ft);
	return hTransfer;
}

////////////////////////////////////////////////////////////////////////////////////////
// FileCancel - cancels a file transfer

int __cdecl CYahooProto::FileCancel(MCONTACT hContact, HANDLE hTransfer)
{
	debugLogA("[YahooFileCancel]");

	y_filetransfer* ft = (y_filetransfer*)hTransfer;
	
	if (! (ft->pfts.flags & PFTS_SENDING)  && ! ft->cancel) {
		/* abort FT transfer */
		yahoo_ft7dc_abort(ft->id, ft->who, ft->ftoken);
	}
	
	if ( ft->hWaitEvent != INVALID_HANDLE_VALUE )
		SetEvent( ft->hWaitEvent );

	ft->action = FILERESUME_CANCEL;
	ft->cancel = 1;
	
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// FileDeny - denies a file transfer

int __cdecl CYahooProto::FileDeny(MCONTACT hContact, HANDLE hTransfer, const PROTOCHAR*)
{
	/* deny file receive request.. just ignore it! */
	y_filetransfer *ft = (y_filetransfer *)hTransfer;

	debugLogA("[YahooFileDeny]");

	if (!m_bLoggedIn || ft == NULL) {
		debugLogA("[YahooFileDeny] Not logged-in or some other error!");
		return 1;
	}

	if (ft->y7) {
		debugLogA("[YahooFileDeny] Y7 Transfer detected.");
		//void yahoo_ft7dc_accept(int id, const char *buddy, const char *ft_token);
		yahoo_ft7dc_deny(ft->id, ft->who, ft->ftoken);
		return 0;
	}

	if (ft->ftoken != NULL) {
		struct yahoo_file_info *fi = (struct yahoo_file_info *)ft->files->data;

		debugLogA("[YahooFileDeny] DC Detected: Denying File Transfer!");
		yahoo_ftdc_deny(m_id, ft->who, fi->filename, ft->ftoken, 2);
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// FileResume - processes file renaming etc

int __cdecl CYahooProto::FileResume(HANDLE hTransfer, int* action, const PROTOCHAR** szFilename)
{
	y_filetransfer *ft = (y_filetransfer *)hTransfer;

	debugLogA("[YahooFileResume] Action: %d", *action);

	if (!m_bLoggedIn || ft == NULL) {
		debugLogA("[YahooFileResume] Not loggedin or some other error!");
		return 1;
	}

	ft->action = *action;

	debugLogA("[YahooFileResume] Action: %d", *action);

	if (*action == FILERESUME_RENAME) {
		debugLogA("[YahooFileResume] Renamed file!");

		FREE(ft->pfts.tszCurrentFile);
		ft->pfts.tszCurrentFile = _tcsdup(*szFilename);
	}

	SetEvent(ft->hWaitEvent);
	return 0;
}
