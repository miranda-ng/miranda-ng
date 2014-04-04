#ifndef _STEAM_H_
#define _STEAM_H_

namespace SteamWebApi
{
	#define STEAM_API_URL "https://api.steampowered.com:443"
	#define STEAM_COMMUNITY_URL "https://steamcommunity.com"

	struct Result
	{
		bool success;
	};

	class BaseApi
	{
	public:
		class Result
		{
		protected:
			bool success;

		public:
			Result() : success(false) { }
			bool IsSuccess() const { return success; }
		};
	};
}

#include <vector>
#include <string>

#include "Steam\crypto.h"
#include "Steam\authorization.h"
#include "Steam\login.h"
#include "Steam\friend_list.h"
#include "Steam\friend.h"
#include "Steam\poll.h"

#endif //_STEAM_H_