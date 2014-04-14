#ifndef _STEAM_H_
#define _STEAM_H_

namespace SteamWebApi
{
	#define STEAM_API_URL "https://api.steampowered.com"
	#define STEAM_COM_URL "https://steamcommunity.com"

	class FriendListApi;
	class InvitationApi;

	class BaseApi
	{
	public:
		class Result
		{
			friend FriendListApi;
			friend InvitationApi;

		protected:
			bool success;
			HTTP_STATUS status;

		public:
			Result() : success(false), status(HTTP_STATUS_NONE) { }

			bool IsSuccess() const { return success; }
			HTTP_STATUS GetStatus() const { return status; }
		};
	};
}

#include "Steam\rsa_key.h"
#include "Steam\authorization.h"
#include "Steam\login.h"
#include "Steam\friend_list.h"
#include "Steam\invitation.h"
#include "Steam\friend.h"
#include "Steam\poll.h"
#include "Steam\message.h"
#include "Steam\search.h"
#include "Steam\avatar.h"

#endif //_STEAM_H_