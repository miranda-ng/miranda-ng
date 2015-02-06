/*
 * WAException.h
 *
 *  Created on: 27/06/2012
 *      Author: Antonio
 */



#ifndef WAEXCEPTION_H_
#define WAEXCEPTION_H_

#include <stdexcept>
#include <string>

class WAException: public std::runtime_error {
public:
	int type;
	int subtype;
	time_t expire_date; // in seconds

	static const int LOGIN_FAILURE_EX = 1;
	static const int LOGIN_FAILURE_EX_TYPE_PASSWORD = 0;
	static const int LOGIN_FAILURE_EX_TYPE_EXPIRED = 1;

	static const int CORRUPT_STREAM_EX = 2;

	static const int SOCKET_EX = 3;
	static const int SOCKET_EX_RESOLVE_HOST = 0;
	static const int SOCKET_EX_OPEN = 1;
	static const int SOCKET_EX_INIT = 2;
	static const int SOCKET_EX_SEND = 3;
	static const int SOCKET_EX_RECV = 4;

	WAException(const std::string &err): runtime_error(err) {this->type = 0; this->subtype = 0; this->expire_date = 0;};
	WAException(const std::string &err, int type, int subtype): runtime_error(err), type(type), subtype(subtype), expire_date(0) {};
	WAException(const std::string &err, int type, int subtype, time_t expireDate): runtime_error(err), type(type), subtype(subtype), expire_date(expireDate) {};
};

#endif /* WAEXCEPTION_H_ */
