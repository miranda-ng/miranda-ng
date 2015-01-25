/*
 * BinTreeNodeReader.cpp
 *
 *  Created on: 26/06/2012
 *      Author: Antonio
 */

#include "../common.h" // #TODO Remove Miranda-dependency

#include "BinTreeNodeReader.h"
#include "WAException.h"
#include "ProtocolTreeNode.h"
#include "utilities.h"

static const char *secondary_dict[] = {
	"mpeg4", "wmv", "audio/3gpp", "audio/aac", "audio/amr", "audio/mp4", "audio/mpeg", "audio/ogg", "audio/qcelp", "audio/wav",
	"audio/webm", "audio/x-caf", "audio/x-ms-wma", "image/gif", "image/jpeg", "image/png", "video/3gpp", "video/avi", "video/mp4",
	"video/mpeg", "video/quicktime", "video/x-flv", "video/x-ms-asf", "302", "400", "401", "402", "403", "404", "405", "406", "407",
	"409", "410", "500", "501", "503", "504", "abitrate", "acodec", "app_uptime", "asampfmt", "asampfreq", "audio", "clear", "conflict",
	"conn_no_nna", "cost", "currency", "duration", "extend", "file", "fps", "g_notify", "g_sound", "gcm", "gone", "google_play", "hash",
	"height", "invalid", "jid-malformed", "latitude", "lc", "lg", "live", "location", "log", "longitude", "max_groups", "max_participants",
	"max_subject", "mimetype", "mode", "napi_version", "normalize", "orighash", "origin", "passive", "password", "played",
	"policy-violation", "pop_mean_time", "pop_plus_minus", "price", "pricing", "redeem", "Replaced by new connection", "resume",
	"signature", "size", "sound", "source", "system-shutdown", "username", "vbitrate", "vcard", "vcodec", "video", "width",
	"xml-not-well-formed", "checkmarks", "image_max_edge", "image_max_kbytes", "image_quality", "ka", "ka_grow", "ka_shrink", "newmedia",
	"library", "caption", "forward", "c0", "c1", "c2", "c3", "clock_skew", "cts", "k0", "k1", "login_rtt", "m_id", "nna_msg_rtt",
	"nna_no_off_count", "nna_offline_ratio", "nna_push_rtt", "no_nna_con_count", "off_msg_rtt", "on_msg_rtt", "stat_name", "sts",
	"suspect_conn", "lists", "self", "qr", "web", "w:b", "recipient", "w:stats", "forbidden", "aurora.m4r", "bamboo.m4r", "chord.m4r",
	"circles.m4r", "complete.m4r", "hello.m4r", "input.m4r", "keys.m4r", "note.m4r", "popcorn.m4r", "pulse.m4r", "synth.m4r", "filehash",
	"max_list_recipients", "en-AU", "en-GB", "es-MX", "pt-PT", "zh-Hans", "zh-Hant", "relayelection", "relaylatency", "interruption",
	"Apex.m4r", "Beacon.m4r", "Bulletin.m4r", "By The Seaside.m4r", "Chimes.m4r", "Circuit.m4r", "Constellation.m4r", "Cosmic.m4r",
	"Crystals.m4r", "Hillside.m4r", "Illuminate.m4r", "Night Owl.m4r", "Opening.m4r", "Playtime.m4r", "Presto.m4r", "Radar.m4r",
	"Radiate.m4r", "Ripples.m4r", "Sencha.m4r", "Signal.m4r", "Silk.m4r", "Slow Rise.m4r", "Stargaze.m4r", "Summit.m4r", "Twinkle.m4r",
	"Uplift.m4r", "Waves.m4r", "voip", "eligible", "upgrade", "planned", "current", "future", "disable", "expire", "start", "stop",
	"accuracy", "speed", "bearing", "recording", "encrypt", "key", "identity", "w:gp2", "admin", "locked", "unlocked", "new", "battery",
	"archive", "adm", "plaintext_size", "compressed_size", "delivered", "msg", "pkmsg", "everyone", "v", "transport", "call-id"
};

BinTreeNodeReader::BinTreeNodeReader(WAConnection* conn, ISocketConnection* connection, const char** dictionary, const int dictionarysize)
{
	this->conn = conn;
	this->rawIn = connection;
	this->tokenMap = dictionary;
	this->tokenmapsize = dictionarysize;
	this->readSize = 1;
	this->in = NULL;
	this->buf = new std::vector<unsigned char>(BUFFER_SIZE);
}

BinTreeNodeReader::~BinTreeNodeReader()
{
	if (this->buf != NULL)
		delete this->buf;
	if (this->in != NULL)
		delete this->in;
}

ProtocolTreeNode* BinTreeNodeReader::nextTreeInternal()
{
	int b = this->in->read();
	int size = readListSize(b);
	b = this->in->read();
	if (b == 2)
		return NULL;

	std::string* tag = this->readStringAsString(b);

	if ((size == 0) || (tag == NULL))
		throw WAException("nextTree sees 0 list or null tag", WAException::CORRUPT_STREAM_EX, -1);
	int attribCount = (size - 2 + size % 2) / 2;
	std::map<string, string>* attribs = readAttributes(attribCount);
	if (size % 2 == 1) {
		ProtocolTreeNode* ret = new ProtocolTreeNode(*tag); ret->attributes = attribs;
		delete tag;
		return ret;
	}
	b = this->in->read();
	if (isListTag(b)) {
		ProtocolTreeNode* ret = new ProtocolTreeNode(*tag, NULL, readList(b)); ret->attributes = attribs;
		delete tag;
		return ret;
	}

	ReadData* obj = this->readString(b);
	std::vector<unsigned char>* data;
	if (obj->type == STRING) {
		std::string* s = (std::string*) obj->data;
		data = new std::vector<unsigned char>(s->begin(), s->end());
		delete s;
	}
	else {
		data = (std::vector<unsigned char>*) obj->data;
	}

	ProtocolTreeNode* ret = new ProtocolTreeNode(*tag, data); ret->attributes = attribs;
	delete obj;
	delete tag;
	return ret;
}

bool BinTreeNodeReader::isListTag(int b)
{
	return (b == 248) || (b == 0) || (b == 249);
}

void BinTreeNodeReader::decodeStream(int flags, int offset, int length)
{
	if ((flags & 8) != 0) {
		if (length < 4)
			throw WAException("invalid length" + length, WAException::CORRUPT_STREAM_EX, 0);

		length -= 4;

		unsigned char *pData = (unsigned char*)&(*this->buf)[0];
		this->conn->inputKey->decodeMessage(pData, offset + length, 0, length);
		this->rawIn->dump(pData + offset, length);
	}

	if (this->in != NULL)
		delete this->in;
	this->in = new ByteArrayInputStream(this->buf, offset, length);
}

std::map<string, string>* BinTreeNodeReader::readAttributes(int attribCount)
{
	std::map<string, string>* attribs = new std::map<string, string>();
	for (int i = 0; i < attribCount; i++) {
		std::string* key = readStringAsString();
		std::string* value = readStringAsString();
		(*attribs)[*key] = *value;
		delete key;
		delete value;
	}
	return attribs;
}

std::vector<ProtocolTreeNode*>* BinTreeNodeReader::readList(int token)
{
	int size = readListSize(token);
	std::vector<ProtocolTreeNode*>* list = new std::vector<ProtocolTreeNode*>(size);
	for (int i = 0; i < size; i++)
		(*list)[i] = nextTreeInternal();

	return list;
}

int BinTreeNodeReader::readListSize(int token)
{
	switch (token) {
	case 0:   return 0;
	case 248: return readInt8(this->in);
	case 249: return readInt16(this->in);
	default:
		throw new WAException("invalid list size in readListSize: token " + token, WAException::CORRUPT_STREAM_EX, 0);
	}
	return 0;
}

std::vector<ProtocolTreeNode*>* BinTreeNodeReader::readList()
{
	return readList(this->in->read());
}

ReadData* BinTreeNodeReader::readString()
{
	return readString(this->in->read());
}

ReadData* BinTreeNodeReader::readString(int token)
{
	if (token == -1)
		throw WAException("-1 token in readString", WAException::CORRUPT_STREAM_EX, -1);

	int bSize;
	ReadData* ret = new ReadData();

	if (token > 2 && token <= this->tokenmapsize) {
		if (token != this->tokenmapsize)
			ret->data = new std::string(this->tokenMap[token]);
		else {
			token = readInt8(this->in);
			if (token >= 0 && token < _countof(secondary_dict))
				ret->data = new std::string(secondary_dict[token]);
			else
				throw WAException("invalid token/length in getToken", WAException::CORRUPT_STREAM_EX, 0);
		}

		ret->type = STRING;
		return ret;
	}

	switch (token) {
	case 0:
		return NULL;

	case 252:
		bSize = readInt8(this->in);
		{
			std::vector<unsigned char>* buf8 = new std::vector<unsigned char>(bSize);
			fillArray(*buf8, bSize, this->in);
			ret->type = ARRAY;
			ret->data = buf8;
		}
		return ret;

	case 253:
		bSize = readInt24(this->in);
		{
			std::vector<unsigned char>* buf24 = new std::vector<unsigned char>(bSize);
			fillArray(*buf24, bSize, this->in);
			ret->type = ARRAY;
			ret->data = buf24;
		}
		return ret;

	case 255:
		bSize = readInt8(this->in);
		{
			int size = bSize & 0x7f;
			int numnibbles = size * 2 - ((bSize & 0x80) ? 1 : 0);

			std::vector<unsigned char> tmp(size);
			fillArray(tmp, size, this->in);
			std::string s;
			for (int i = 0; i < numnibbles; i++) {
				char c = (tmp[i / 2] >> (4 - ((i & 1) << 2))) & 0xF;
				if (c < 10) s += (c + '0');
				else s += (c - 10 + '-');
			}

			ret->type = STRING;
			ret->data = new std::string(s);
		}
		return ret;

	case 250:
		std::string* user = readStringAsString();
		std::string* server = readStringAsString();
		if ((user != NULL) && (server != NULL)) {
			std::string* result = new std::string(*user + "@" + *server);
			delete user;
			delete server;
			ret->type = STRING;
			ret->data = result;
			return ret;
		}
		if (server != NULL) {
			ret->type = STRING;
			ret->data = server;
			return ret;
		}
		throw WAException("readString couldn't reconstruct jid", WAException::CORRUPT_STREAM_EX, -1);
	}
	throw WAException("readString couldn't match token" + (int)token, WAException::CORRUPT_STREAM_EX, -1);
}

std::string* BinTreeNodeReader::objectAsString(ReadData* o)
{
	if (o->type == STRING)
		return (std::string*) o->data;

	if (o->type == ARRAY) {
		std::vector<unsigned char>* v = (std::vector<unsigned char>*) o->data;
		std::string* ret = new std::string(v->begin(), v->end());
		delete v;
		return ret;
	}

	return NULL;
}

std::string* BinTreeNodeReader::readStringAsString()
{
	ReadData* o = this->readString();
	std::string* ret = this->objectAsString(o);
	delete o;
	return ret;
}

std::string* BinTreeNodeReader::readStringAsString(int token)
{
	ReadData* o = this->readString(token);
	std::string* ret = this->objectAsString(o);
	delete o;
	return ret;
}

void BinTreeNodeReader::fillArray(std::vector<unsigned char>& buff, int len, ByteArrayInputStream* in)
{
	int count = 0;
	while (count < len)
		count += in->read(buff, count, len - count);
}

void BinTreeNodeReader::fillArray(std::vector<unsigned char>& buff, int len, ISocketConnection* in)
{
	int count = 0;
	while (count < len)
		count += in->read(buff, count, len - count);
}

void BinTreeNodeReader::getTopLevelStream()
{
	int stanzaSize = readInt24(this->rawIn);
	int flags = (stanzaSize >> 20);
	stanzaSize &= 0x0FFFFF;	

	if (this->buf->size() < (size_t)stanzaSize) {
		int newsize = max((int)(this->buf->size() * 3 / 2), stanzaSize);
		delete this->buf;
		this->buf = new std::vector<unsigned char>(newsize);
	}
	fillArray(*this->buf, stanzaSize, this->rawIn);

	this->decodeStream(flags, 0, stanzaSize);
}

int BinTreeNodeReader::readInt8(ByteArrayInputStream* in)
{
	return in->read();
}

int BinTreeNodeReader::readInt16(ByteArrayInputStream* in)
{
	int intTop = in->read();
	int intBot = in->read();
	int value = (intTop << 8) + intBot;
	return value;
}

int BinTreeNodeReader::readInt24(ByteArrayInputStream* in)
{
	int int1 = in->read();
	int int2 = in->read();
	int int3 = in->read();
	int value = (int1 << 16) + (int2 << 8) + int3;

	return value;
}

ProtocolTreeNode* BinTreeNodeReader::nextTree()
{
	this->getTopLevelStream();
	return nextTreeInternal();
}

void BinTreeNodeReader::streamStart()
{
	this->getTopLevelStream();

	int tag = this->in->read();
	int size = readListSize(tag);
	tag = this->in->read();
	if (tag != 1) {
		throw WAException("expecting STREAM_START in streamStart", WAException::CORRUPT_STREAM_EX, 0);
	}
	int attribCount = (size - 2 + size % 2) / 2;

	std::map<string, string>* attributes = readAttributes(attribCount);
	delete attributes;
}

int BinTreeNodeReader::readInt8(ISocketConnection* in)
{
	return in->read();
}

int BinTreeNodeReader::readInt16(ISocketConnection* in)
{
	unsigned char data[2];
	in->read(data, 2);
	return (int(data[0]) << 8) + int(data[1]);
}

int BinTreeNodeReader::readInt24(ISocketConnection* in)
{
	unsigned char data[3];
	in->read(data, 3);
	return (int(data[0]) << 16) + (int(data[1]) << 8) + int(data[2]);
}
