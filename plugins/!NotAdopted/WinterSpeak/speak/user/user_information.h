#ifndef guard_speak_user_user_information_h
#define guard_speak_user_user_information_h
//==============================================================================
// 	Miranda Speak Plugin, © 2002 Ryan Winter
//==============================================================================

#include <windows.h>

#include <map>
#include <string>

class UserInformation
{
  public:
	UserInformation();
	~UserInformation();

	//--------------------------------------------------------------------------
	// Description : update the users status
	// Parameters  : user - the current user
	//               status - the users status
	// Returns     : true - the status changed
	//               false - the status stayed the same
	//--------------------------------------------------------------------------
	bool updateStatus(HANDLE user, int status);

	//--------------------------------------------------------------------------
	// Description : get a string containing the users current status string
	// Parameters  : user - the current user
	// Returns     : the string containing the users status
	//--------------------------------------------------------------------------
	std::string statusString(HANDLE user);

	//--------------------------------------------------------------------------
	// Description : return the status mode of the user
	// Parameters  : user - the current user
	// Returns     : the string containing the users status mode
	//--------------------------------------------------------------------------
    std::string statusModeString(HANDLE user);

	//--------------------------------------------------------------------------
	// Description : insert the name into the string at the %u location
	// Parameters  : str  - the string to have the username inserted into
	//               user - the current user
	//--------------------------------------------------------------------------
	void insertName(std::string &str, HANDLE user) const;

	//--------------------------------------------------------------------------
	// Description : get the name string for the user
	// Parameters  : user - the current user
	// Returns     : a string containing the user's name
	//--------------------------------------------------------------------------
	std::string nameString(HANDLE user) const;

  private:
	std::map<HANDLE, int>      m_status_info;
	std::map<int, std::string> m_status_strings;
};

//==============================================================================
//
//  Summary     : Contain information about the current users
//
//  Description : Provides an interface to get various information about the 
//                user. Also holds the users current status.
//
//==============================================================================

#endif