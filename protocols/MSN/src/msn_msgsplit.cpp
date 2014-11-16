/*
Plugin of Miranda IM for communicating with users of the MSN Messenger protocol.

Copyright (c) 2012-2014 Miranda NG Team
Copyright (c) 2007-2012 Boris Krasnovskiy.

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

#include "msn_global.h"
#include "msn_proto.h"

chunkedmsg::chunkedmsg(const char* tid, const size_t totsz, const bool tbychunk)
	: size(totsz), recvsz(0), bychunk(tbychunk)
{
	id = mir_strdup(tid);
	msg = tbychunk ? NULL : (char*)mir_alloc(totsz + 1);
}

chunkedmsg::~chunkedmsg()
{
	mir_free(id);
	mir_free(msg);
}

void chunkedmsg::add(const char* tmsg, size_t offset, size_t portion)
{
	if (bychunk) {
		size_t oldsz = recvsz;
		recvsz += portion;
		msg = (char*)mir_realloc(msg, recvsz + 1);
		memcpy(msg + oldsz, tmsg, portion);
		--size;
	}
	else {
		size_t newsz = offset + portion;
		if (newsz > size) {
			portion = size - offset;
			newsz = size;
		}
		memcpy(msg + offset, tmsg, portion);
		if (newsz > recvsz) recvsz = newsz;
	}
}

bool chunkedmsg::get(char*& tmsg, size_t& tsize)
{
	bool alldata = bychunk ? size == 0 : recvsz == size;
	if (alldata) {
		msg[recvsz] = 0;
		tmsg = msg;
		tsize = recvsz;
		msg = NULL;
	}

	return alldata;
}


int CMsnProto::addCachedMsg(const char* id, const char* msg, const size_t offset,
	const size_t portion, const size_t totsz, const bool bychunk)
{
	int idx = msgCache.getIndex((chunkedmsg*)&id);
	if (idx == -1) {
		msgCache.insert(new chunkedmsg(id, totsz, bychunk));
		idx = msgCache.getIndex((chunkedmsg*)&id);
	}

	msgCache[idx].add(msg, offset, portion);

	return idx;
}

size_t CMsnProto::getCachedMsgSize(const char* id)
{
	int idx = msgCache.getIndex((chunkedmsg*)&id);
	return idx != -1 ? msgCache[idx].size : 0;
}

bool CMsnProto::getCachedMsg(int idx, char*& msg, size_t& size)
{
	bool res = msgCache[idx].get(msg, size);
	if (res)
		msgCache.remove(idx);

	return res;
}

bool CMsnProto::getCachedMsg(const char* id, char*& msg, size_t& size)
{
	int idx = msgCache.getIndex((chunkedmsg*)&id);
	return idx != -1 && getCachedMsg(idx, msg, size);
}


void CMsnProto::clearCachedMsg(int idx)
{
	if (idx != -1)
		msgCache.remove(idx);
	else
		msgCache.destroy();
}

void CMsnProto::CachedMsg_Uninit(void)
{
	clearCachedMsg();
}
