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
#ifndef FILE_H
#define FILE_H

struct CAimProto;

struct file_transfer
{
	MCONTACT hContact;
	char* sn;

	char icbm_cookie[8];

	HANDLE hConn;
	HANDLE hResumeEvent;
	HANDLE hDirectBoundPort;

	char* file;
	char* message;

	PROTOFILETRANSFERSTATUS pfts;

	unsigned long cf;

	//below is for when receiving only
	unsigned long local_ip;
	unsigned long verified_ip;
	unsigned long proxy_ip;
	unsigned short port;
	unsigned short max_ver;

	unsigned short req_num;
	unsigned short local_port;

	bool peer_force_proxy;
	bool me_force_proxy;
	bool sending;
	bool accepted;
	bool requester;
	bool success;

	file_transfer(MCONTACT hCont, char* nick, char* cookie);
	~file_transfer();

	void listen(CAimProto* ppro);
	void stop_listen(void);

};

struct ft_list_type : OBJLIST <file_transfer> 
{
	ft_list_type();

	file_transfer* find_by_handle(MCONTACT hContact);
	file_transfer* find_by_cookie(char* cookie, MCONTACT hContact);
	file_transfer* find_by_port(unsigned short port);

	bool find_by_ft(file_transfer *ft);

	void remove_by_ft(file_transfer *ft);
};


#endif
