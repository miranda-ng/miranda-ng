/*
 * WARegister.h
 */

#ifndef WAREGISTER_H_
#define WAREGISTER_H_

#include <string>

struct WAToken
{
	static std::string GenerateToken(const std::string &number);
};

class WARegister
{
	static std::string GenerateIdentity(const std::string &phone);

public:
	static CMStringA RequestCodeUrl(const std::string &phone, const std::string &code);
};

#endif /* WAREGISTER_H_ */
