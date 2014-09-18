/*
 * PhoneNumber.h
 *
 */

#ifndef PHONENUMBER_H_
#define PHONENUMBER_H_

#include <string>

struct PhoneNumber
{
	PhoneNumber(const std::string &number);

	std::string Country;
	std::string Number;
	
	const char *ISO3166, *ISO639;
	int countryCode;
	int mcc, mnc;
};

#endif /* PHONENUMBER_H_ */
