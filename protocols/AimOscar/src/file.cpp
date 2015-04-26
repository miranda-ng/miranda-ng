/*
Plugin of Miranda IM for communicating with users of the AIM protocol.
Copyright (c) 2008-2012 Boris Krasnovskiy
Copyright (C) 2005-2006 Aaron Myles Landwehr

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "stdafx.h"

#pragma pack(push, 1)
struct oft2//oscar file transfer 2 class- See On_Sending_Files_via_OSCAR.pdf
{
	char protocol_version[4];//4
	unsigned short length;//6
	unsigned short type;//8
	unsigned char icbm_cookie[8];//16
	unsigned short encryption;//18
	unsigned short compression;//20
	unsigned short total_files;//22
	unsigned short num_files_left;//24
	unsigned short total_parts;//26
	unsigned short parts_left;//28
	unsigned long total_size;//32
	unsigned long size;//36
	unsigned long mod_time;//40
	unsigned long checksum;//44
	unsigned long recv_RFchecksum;//48
	unsigned long RFsize;//52
	unsigned long creation_time;//56
	unsigned long RFchecksum;//60
	unsigned long recv_bytes;//64
	unsigned long recv_checksum;//68
	unsigned char idstring[32];//100
	unsigned char flags;//101
	unsigned char list_name_offset;//102
	unsigned char list_size_offset;//103
	unsigned char dummy[69];//172
	unsigned char mac_info[16];//188
	unsigned short encoding;//190
	unsigned short sub_encoding;//192
	unsigned char filename[64];//256
 };

#pragma pack(pop)

bool send_init_oft2(file_transfer *ft, char* file)
{
	aimString astr(file);

	unsigned short len = max(0x100, 0xc0 + astr.getTermSize());

	oft2 *oft = (oft2*)alloca(len);
	memset(oft, 0, len);

	memcpy(oft->protocol_version, "OFT2", 4);
	oft->length           = _htons(len);
	oft->type             = 0x0101;
	oft->total_files      = _htons(ft->pfts.totalFiles);
	oft->num_files_left   = _htons(ft->pfts.totalFiles - ft->pfts.currentFileNumber);
	oft->total_parts      = _htons(1);
	oft->parts_left       = _htons(1);
	oft->total_size       = _htonl(ft->pfts.totalBytes);
	oft->size             = _htonl(ft->pfts.currentFileSize);
	oft->mod_time         = _htonl(ft->pfts.currentFileTime);
	oft->checksum         = _htonl(aim_oft_checksum_file(ft->pfts.tszCurrentFile));
	oft->recv_RFchecksum  = 0x0000FFFF;
	oft->RFchecksum       = 0x0000FFFF;
	oft->recv_checksum    = 0x0000FFFF;
	memcpy(oft->idstring, "Cool FileXfer", 13);
	oft->flags            = 0x20;
	oft->list_name_offset = 0x1c;
	oft->list_size_offset = 0x11;
	oft->encoding         = _htons(astr.isUnicode() ? 2 : 0);
	memcpy(oft->filename, astr.getBuf(), astr.getTermSize());

	if (!ft->requester || ft->pfts.currentFileNumber)
		memcpy(oft->icbm_cookie, ft->icbm_cookie, 8);

	return Netlib_Send(ft->hConn, (char*)oft, len, 0) > 0;
}

void CAimProto::report_file_error(TCHAR *fname)
{
	TCHAR errmsg[512];
	TCHAR* error = mir_a2t(_strerror(NULL));
	mir_sntprintf(errmsg, SIZEOF(errmsg), TranslateT("Failed to open file: %s : %s"), fname, error);
	mir_free(error);
	ShowPopup((char*)errmsg, ERROR_POPUP | TCHAR_POPUP);
}

bool setup_next_file_send(file_transfer *ft)
{
	TCHAR *file;
	struct _stati64 statbuf;
	for (;;)
	{
		file = ft->pfts.ptszFiles[ft->cf];
		if (file == NULL) return false;

		if (_tstati64(file, &statbuf) == 0 && (statbuf.st_mode & _S_IFDIR) == 0) 
			break;

		++ft->cf;
	}

	ft->pfts.tszCurrentFile = file;
	ft->pfts.currentFileSize = statbuf.st_size;
	ft->pfts.currentFileTime = statbuf.st_mtime;
	ft->pfts.currentFileProgress = 0;

	char* fnamea;
	char* fname = mir_utf8encodeT(file);
	if (ft->pfts.totalFiles > 1 && ft->file[0])
	{
		size_t dlen = strlen(ft->file);
		if (strncmp(fname, ft->file, dlen) == 0 && fname[dlen] == '\\')
		{
			fnamea = &fname[dlen+1];
			for (char *p = fnamea; *p; ++p) { if (*p == '\\') *p = 1; }
		}
		else
			fnamea = get_fname(fname);
	}
	else
		fnamea = get_fname(fname);

	send_init_oft2(ft, fnamea);

	mir_free(fname);
	return true;
}

int CAimProto::sending_file(file_transfer *ft, HANDLE hServerPacketRecver, NETLIBPACKETRECVER &packetRecv)
{
	debugLogA("P2P: Entered file sending thread.");

	bool failed = true;
	bool failed_conn = false;

	if (!setup_next_file_send(ft)) return 2;

	debugLogA("Sent file information to buddy.");
	//start listen for packets stuff

	for (;;)
	{
		int recvResult = packetRecv.bytesAvailable - packetRecv.bytesUsed;
		if (recvResult <= 0)
			recvResult = CallService(MS_NETLIB_GETMOREPACKETS, (WPARAM)hServerPacketRecver, (LPARAM)&packetRecv);
		if (recvResult == 0)
		{
			debugLogA("P2P: File transfer connection Error: 0");
			break;
		}
		if (recvResult == SOCKET_ERROR)
		{
			failed_conn = true;
			debugLogA("P2P: File transfer connection Error: -1");
			break;
		}
		if (recvResult > 0)
		{	
			if (recvResult < 0x100) continue;
		   
			oft2* recv_ft = (oft2*)&packetRecv.buffer[packetRecv.bytesUsed];

			unsigned short pkt_len = _htons(recv_ft->length);
			if (recvResult < pkt_len) continue;

			packetRecv.bytesUsed += pkt_len;
			unsigned short type = _htons(recv_ft->type);
			if (type == 0x0202 || type == 0x0207)
			{
				debugLogA("P2P: Buddy Accepts our file transfer.");

				int fid = _topen(ft->pfts.tszCurrentFile, _O_RDONLY | _O_BINARY, _S_IREAD);
				if (fid < 0)
				{
					report_file_error(ft->pfts.tszCurrentFile);
					return 2;
				}

				if (ft->pfts.currentFileProgress) _lseeki64(fid, ft->pfts.currentFileProgress, SEEK_SET);

				NETLIBSELECT tSelect = {0};
				tSelect.cbSize = sizeof(tSelect);
				tSelect.hReadConns[0] = ft->hConn;

				ProtoBroadcastAck(ft->hContact, ACKTYPE_FILE, ACKRESULT_DATA, ft, (LPARAM)&ft->pfts);

				clock_t lNotify = clock();
				for (;;)
				{
					char buffer[4096];
					int bytes = _read(fid, buffer, sizeof(buffer));
					if (bytes <= 0) break;

					if (Netlib_Send(ft->hConn, buffer, bytes, MSG_NODUMP) <= 0) break;
					ft->pfts.currentFileProgress += bytes;
					ft->pfts.totalProgress += bytes;
					
					if (clock() >= lNotify)
					{
						ProtoBroadcastAck(ft->hContact, ACKTYPE_FILE, ACKRESULT_DATA, ft, (LPARAM)&ft->pfts);
						if (CallService(MS_NETLIB_SELECT, 0, (LPARAM)&tSelect)) break;

						lNotify = clock() + 500;
					}
				}
				ProtoBroadcastAck(ft->hContact, ACKTYPE_FILE, ACKRESULT_DATA, ft, (LPARAM)&ft->pfts);
				debugLogA("P2P: Finished sending file bytes.");
				_close(fid);
			}
			else if (type == 0x0204)
			{
				// Handle file skip case
				if (ft->pfts.currentFileProgress == 0)
				{
					ft->pfts.totalProgress += ft->pfts.currentFileSize;
				}

				debugLogA("P2P: Buddy says they got the file successfully");
				if ((ft->pfts.currentFileNumber + 1) < ft->pfts.totalFiles)
				{
					ProtoBroadcastAck(ft->hContact, ACKTYPE_FILE, ACKRESULT_NEXTFILE, ft, 0);
					++ft->pfts.currentFileNumber; ++ft->cf;

					if (!setup_next_file_send(ft))
					{
						report_file_error(ft->pfts.tszCurrentFile);
						return 2;
					}
				}
				else
				{
					failed = _htonl(recv_ft->recv_bytes) != ft->pfts.currentFileSize;
					break;
				}
			}
			else if (type == 0x0205)
			{
				oft2* recv_ft = (oft2*)packetRecv.buffer;
				recv_ft->type = _htons(0x0106);
				
				ft->pfts.currentFileProgress = _htonl(recv_ft->recv_bytes);
				if (aim_oft_checksum_file(ft->pfts.tszCurrentFile, ft->pfts.currentFileProgress) != _htonl(recv_ft->recv_checksum))
				{
					ft->pfts.currentFileProgress = 0;
					recv_ft->recv_bytes = 0;
				}

				debugLogA("P2P: Buddy wants us to start sending at a specified file point. (%I64u)", ft->pfts.currentFileProgress);
				if (Netlib_Send(ft->hConn, (char*)recv_ft, _htons(recv_ft->length), 0) <= 0) break;

				ft->pfts.totalProgress += ft->pfts.currentFileProgress;
				ProtoBroadcastAck(ft->hContact, ACKTYPE_FILE, ACKRESULT_DATA, ft, (LPARAM)&ft->pfts);
			}
		}
	}

	ft->success = !failed;
	return failed ? (failed_conn ? 1 : 2) : 0;
}

int CAimProto::receiving_file(file_transfer *ft, HANDLE hServerPacketRecver, NETLIBPACKETRECVER &packetRecv)
{
	debugLogA("P2P: Entered file receiving thread.");
	bool failed = true;
	bool failed_conn = false;
	bool accepted_file = false;
	int fid = -1;

	oft2 *oft = NULL;

	ft->pfts.tszWorkingDir = mir_utf8decodeT(ft->file);

	//start listen for packets stuff
	for (;;)
	{
		int recvResult = packetRecv.bytesAvailable - packetRecv.bytesUsed;
		if (recvResult <= 0)
			recvResult = CallService(MS_NETLIB_GETMOREPACKETS, (WPARAM)hServerPacketRecver, (LPARAM)&packetRecv);
		if (recvResult == 0)
		{
			debugLogA("P2P: File transfer connection Error: 0");
			break;
		}
		if (recvResult == SOCKET_ERROR)
		{
			failed_conn = true;
			debugLogA("P2P: File transfer connection Error: -1");
			break;
		}
		if (recvResult > 0)
		{	
			if (!accepted_file)
			{
				if (recvResult < 0x100) continue;
			   
				oft2* recv_ft = (oft2*)&packetRecv.buffer[packetRecv.bytesUsed];
				unsigned short pkt_len = _htons(recv_ft->length);

				if (recvResult < pkt_len) continue;
				packetRecv.bytesUsed += pkt_len;

				unsigned short type = _htons(recv_ft->type);
				if (type == 0x0101)
				{
					debugLogA("P2P: Buddy Ready to begin transfer.");
					oft = (oft2*)mir_realloc(oft, pkt_len);
					memcpy(oft, recv_ft, pkt_len);
					memcpy(oft->icbm_cookie, ft->icbm_cookie, 8);

					int buflen = pkt_len - 0x100 + 64;
					char *buf = (char*)mir_calloc(buflen + 2);
					unsigned short enc;

					ft->pfts.currentFileSize = _htonl(recv_ft->size);
					ft->pfts.totalBytes = _htonl(recv_ft->total_size);
					ft->pfts.currentFileTime = _htonl(recv_ft->mod_time);
					memcpy(buf, recv_ft->filename, buflen);
					enc = _htons(recv_ft->encoding);

					TCHAR *name;
					if (enc == 2)
					{
						wchar_t* wbuf = (wchar_t*)buf;
						wcs_htons(wbuf);
						for (wchar_t *p = wbuf; *p; ++p) { if (*p == 1) *p = '\\'; }
						name = mir_u2t(wbuf);
					}
					else
					{
						for (char *p = buf; *p; ++p) { if (*p == 1) *p = '\\'; }
						name = mir_a2t(buf);
					}

					mir_free(buf);

					TCHAR fname[256];
					mir_sntprintf(fname, SIZEOF(fname), _T("%s%s"), ft->pfts.tszWorkingDir, name);
					mir_free(name);
					mir_free(ft->pfts.tszCurrentFile);
					ft->pfts.tszCurrentFile = mir_tstrdup(fname);

					ResetEvent(ft->hResumeEvent);
					if (ProtoBroadcastAck(ft->hContact, ACKTYPE_FILE, ACKRESULT_FILERESUME, ft, (LPARAM)&ft->pfts))
						WaitForSingleObject(ft->hResumeEvent, INFINITE);

					if (ft->pfts.tszCurrentFile)
					{
						TCHAR* dir = get_dir(ft->pfts.tszCurrentFile);
						CreateDirectoryTreeT(dir);
						mir_free(dir);

						oft->type = _htons(ft->pfts.currentFileProgress ? 0x0205 : 0x0202);

						const int flag = ft->pfts.currentFileProgress ? 0 : _O_TRUNC;
						fid = _topen(ft->pfts.tszCurrentFile, _O_CREAT | _O_WRONLY | _O_BINARY | flag, _S_IREAD | _S_IWRITE);

						if (fid < 0)	
						{
							report_file_error(fname);
							break;
						}

						accepted_file = ft->pfts.currentFileProgress == 0;

						if (ft->pfts.currentFileProgress)
						{
							bool the_same;
							oft->recv_bytes = _htonl(ft->pfts.currentFileProgress);
							oft->recv_checksum = _htonl(aim_oft_checksum_file(ft->pfts.tszCurrentFile));
							the_same = oft->size == oft->recv_bytes && oft->checksum == oft->recv_checksum;
							if (the_same)
							{
								ft->pfts.totalProgress += ft->pfts.currentFileProgress;
								oft->type = _htons(0x0204);
								_close(fid);

								ProtoBroadcastAck(ft->hContact, ACKTYPE_FILE, ACKRESULT_NEXTFILE, ft, 0);
								++ft->pfts.currentFileNumber;
								ft->pfts.currentFileProgress = 0;
							}
						}
					}
					else
					{
						oft->type = _htons(0x0204);

						ProtoBroadcastAck(ft->hContact, ACKTYPE_FILE, ACKRESULT_NEXTFILE, ft, 0);
						++ft->pfts.currentFileNumber;
						ft->pfts.currentFileProgress = 0;
					}

					if (Netlib_Send(ft->hConn, (char*)oft, pkt_len, 0) == SOCKET_ERROR)
						break;

					if (ft->pfts.currentFileNumber >= ft->pfts.totalFiles && _htons(oft->type) == 0x0204)
					{
						failed = false;
						break;
					}
				}
				else if (type == 0x0106)
				{
					oft = (oft2*)mir_realloc(oft, pkt_len);
					memcpy(oft, recv_ft, pkt_len);

					ft->pfts.currentFileProgress = _htonl(oft->recv_bytes);
					ft->pfts.totalProgress += ft->pfts.currentFileProgress;

					_lseeki64(fid, ft->pfts.currentFileProgress, SEEK_SET);
					accepted_file = true;

					oft->type = _htons(0x0207);
					if (Netlib_Send(ft->hConn, (char*)oft, pkt_len, 0) == SOCKET_ERROR)
						break;
				}
				else
					break;
			}
			else
			{
				packetRecv.bytesUsed = packetRecv.bytesAvailable;
				_write(fid, packetRecv.buffer, packetRecv.bytesAvailable);
				ft->pfts.currentFileProgress += packetRecv.bytesAvailable;
				ft->pfts.totalProgress       += packetRecv.bytesAvailable;
				ProtoBroadcastAck(ft->hContact, ACKTYPE_FILE, ACKRESULT_DATA, ft, (LPARAM)&ft->pfts);
				if (ft->pfts.currentFileSize == ft->pfts.currentFileProgress)
				{
					oft->type = _htons(0x0204);
					oft->recv_bytes = _htonl(ft->pfts.currentFileProgress);
					oft->recv_checksum = _htonl(aim_oft_checksum_file(ft->pfts.tszCurrentFile));

					debugLogA("P2P: We got the file successfully");
					Netlib_Send(ft->hConn, (char*)oft, _htons(oft->length), 0);
					if (_htons(oft->num_files_left) == 1)
					{
						failed = false;
						break;
					}
					else
					{
						accepted_file = false;
						_close(fid);

						ProtoBroadcastAck(ft->hContact, ACKTYPE_FILE, ACKRESULT_NEXTFILE, ft, 0);
						++ft->pfts.currentFileNumber;
						ft->pfts.currentFileProgress = 0;
					}
				}
			}
		}
	}

	if (accepted_file) _close(fid);
	mir_free(oft);

	ft->success = !failed;
	return failed ? (failed_conn ? 1 : 2) : 0;
}

void CAimProto::shutdown_file_transfers(void)
{
	for(int i=0; i<ft_list.getCount(); ++i)
	{
		file_transfer& ft = ft_list[i];
		if (ft.hConn)
			Netlib_Shutdown(ft.hConn);
	}
}

ft_list_type::ft_list_type() :  OBJLIST <file_transfer>(10) {};

file_transfer* ft_list_type::find_by_cookie(char* cookie, MCONTACT hContact)
{
	for (int i = 0; i < getCount(); ++i)
	{
		file_transfer *ft = items[i];
		if (ft->hContact == hContact && memcmp(ft->icbm_cookie, cookie, 8) == 0)
			return ft;
	}
	return NULL;
}

file_transfer* ft_list_type::find_by_port(unsigned short port)
{
	for (int i = getCount(); i--; )
	{
		file_transfer *ft = items[i];
		if (ft->requester && ft->local_port  == port)
			return ft;
	}
	return NULL;
}


bool ft_list_type::find_by_ft(file_transfer *ft)
{
	for (int i = 0; i < getCount(); ++i)
	{
		if (items[i] == ft) return true;
	}
	return false;
}

void ft_list_type::remove_by_ft(file_transfer *ft)
{
	for (int i = 0; i < getCount(); ++i)
	{
		if (items[i] == ft)
		{
			remove(i);
			break;
		}
	}
}

file_transfer::file_transfer(MCONTACT hCont, char* nick, char* cookie)
{ 
	memset(this, 0, sizeof(*this)); 

	pfts.cbSize = sizeof(pfts);
	pfts.flags = PFTS_TCHAR;
	pfts.hContact = hCont;

	hContact = hCont;
	sn = mir_strdup(nick);

	if (cookie)
		memcpy(icbm_cookie, cookie, 8);
	else
		CallService(MS_UTILS_GETRANDOM, 8, (LPARAM)icbm_cookie);
	
	hResumeEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
}

file_transfer::~file_transfer() 
{ 
	stop_listen();

	mir_free(file); 
	mir_free(message); 
	mir_free(sn); 

	mir_free(pfts.tszWorkingDir);
	if (!sending) mir_free(pfts.tszCurrentFile);

	if (success && pfts.ptszFiles)
	{
		for (int i = 0; pfts.ptszFiles[i]; i++)
			mir_free(pfts.ptszFiles[i]);

		mir_free(pfts.ptszFiles);
	}
	CloseHandle(hResumeEvent);
}

void file_transfer::listen(CAimProto* ppro) 
{ 
	if (hDirectBoundPort) return;

	NETLIBBIND nlb = {0};
	nlb.cbSize = sizeof(nlb);
	nlb.pfnNewConnectionV2 = aim_direct_connection_initiated;
	nlb.pExtra = ppro;
	hDirectBoundPort = (HANDLE)CallService(MS_NETLIB_BINDPORT, (WPARAM)ppro->hNetlibPeer, (LPARAM)&nlb);
	local_port = hDirectBoundPort ? nlb.wPort : 0;
}

void file_transfer::stop_listen(void) 
{ 
	if (hDirectBoundPort)
	{
		Netlib_CloseHandle(hDirectBoundPort);
		hDirectBoundPort = NULL;
		local_port = 0;
	}
}
