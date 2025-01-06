#ifndef _STEAM_ENUMS_H_
#define _STEAM_ENUMS_H_

namespace FriendRelationship
{
	enum {
		None = 0,
		Blocked = 1,
		RequestRecipient = 2,
		Friend = 3,
		RequestInitiator = 4,
		Ignored = 5,
		IgnoredFriend = 6,
	};
}

namespace PersonaState
{
	enum {
		Offline = 0,
		Online = 1,
		Busy = 2,
		Away = 3,
		Snooze = 4,
		LookingToTrade = 5,
		LookingToPlay = 6,
		Invisible = 7,
	};
}

namespace PersonaStateFlag
{
	enum {
		None = 0,
		HasRichPresence = 1,
		InJoinableGame = 2,
		ClientTypeWeb = 256,
		ClientTypeMobile = 512,
		ClientTypeBigPicture = 1024,
		ClientTypeVR = 2048,
	};
}

namespace PersonaStatusFlag
{
	enum {
		Status = 1,
		PlayerName = 2,
		QueryPort = 4,
		SourceID = 8,
		Presence = 16,
		Metadata = 32,
		LastSeen = 64,
		ClanInfo = 128,
		GameExtraInfo = 256,
		GameDataBlob = 512,
		ClanTag = 1024,
		Facebook = 2048,
		Unknown = 4096,
	};
}

#endif //_STEAM_ENUMS_H_
