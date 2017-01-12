#include "stdafx.h"
#include "WASocketConnection.h"

HNETLIBUSER g_hNetlibUser = NULL;

void WASocketConnection::initNetwork(HNETLIBUSER hNetlibUser) throw (WAException)
{
	g_hNetlibUser = hNetlibUser;
}

void WASocketConnection::quitNetwork()
{
}

WASocketConnection::WASocketConnection(const std::string &dir, int port) throw (WAException)
{
	NETLIBOPENCONNECTION	noc = { sizeof(noc) };
	noc.szHost = dir.c_str();
	noc.wPort = port;
	noc.flags = NLOCF_V2; // | NLOCF_SSL;
	this->hConn = Netlib_OpenConnection(g_hNetlibUser, &noc);
	if (this->hConn == NULL)
		throw WAException(getLastErrorMsg(), WAException::SOCKET_EX, WAException::SOCKET_EX_OPEN);

	this->connected = true;
}

void WASocketConnection::write(int i)
{
	char buffer;
	buffer = (char)i;

	int result = Netlib_Send(this->hConn, &buffer, 1, MSG_NOHTTPGATEWAYWRAP | MSG_NODUMP);
	if (result < 1)
		throw WAException(getLastErrorMsg(), WAException::SOCKET_EX, WAException::SOCKET_EX_SEND);
}

void WASocketConnection::makeNonBlock()
{
	throw WAException("Error setting socket nonblocking!", WAException::SOCKET_EX, WAException::SOCKET_EX_OPEN);
}

void WASocketConnection::flush() {}

void WASocketConnection::write(const std::vector<unsigned char> &bytes, int length)
{
	std::string tmpBuf = std::string(bytes.begin(), bytes.end());
	int result = Netlib_Send(hConn, tmpBuf.c_str(), length, MSG_NODUMP);
	if (result < length)
		throw WAException(getLastErrorMsg(), WAException::SOCKET_EX, WAException::SOCKET_EX_SEND);
}

unsigned char WASocketConnection::read()
{
	SetLastError(0);

	char c;
	int result = Netlib_Recv(this->hConn, &c, 1, 0);
	if (result <= 0)
		throw WAException(getLastErrorMsg(), WAException::SOCKET_EX, WAException::SOCKET_EX_RECV);

	return c;
}

int WASocketConnection::read(unsigned char *buf, int length)
{
	int result = Netlib_Recv(this->hConn, (char*)buf, length, MSG_NODUMP);
	if (result <= 0)
		throw WAException(getLastErrorMsg(), WAException::SOCKET_EX, WAException::SOCKET_EX_RECV);

	return result;
}

int WASocketConnection::read(std::vector<unsigned char>& b, int off, int length)
{
	if (off < 0 || length < 0)
		throw new WAException("Out of bounds", WAException::SOCKET_EX, WAException::SOCKET_EX_RECV);

	char* buffer = (char*)_alloca(length);
	int result = Netlib_Recv(this->hConn, buffer, length, MSG_NOHTTPGATEWAYWRAP | MSG_NODUMP);
	if (result <= 0)
		throw WAException(getLastErrorMsg(), WAException::SOCKET_EX, WAException::SOCKET_EX_RECV);

	for (int i = 0; i < result; i++)
		b[off + i] = buffer[i];

	return result;
}

void WASocketConnection::forceShutdown()
{
	Netlib_Shutdown(this->hConn);
}

void WASocketConnection::log(const char *prefix, const char *str)
{
	Netlib_Logf(g_hNetlibUser, "%s%s", prefix, str);
}

WASocketConnection::~WASocketConnection()
{
	this->forceShutdown();
	Netlib_CloseHandle(this->hConn);
}
