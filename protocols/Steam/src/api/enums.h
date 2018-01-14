#ifndef _STEAM_ENUMS_H_
#define _STEAM_ENUMS_H_

enum VisibilityState
{
	Private = 1,
	FriendsOnly = 2,
	FriendsOfFriends = 3,
	UsersOnly = 4,
	Public = 5,
};

enum PersonaState
{
	Offline = 0,
	Online = 1,
	Busy = 2,
	Away = 3,
	Snooze = 4,
	LookingToTrade = 5,
	LookingToPlay = 6,
};

enum PersonaStateFlag
{
	None = 0,
	HasRichPresence = 1,
	InJoinableGame = 2,
	OnlineUsingWeb = 256,
	OnlineUsingMobile = 512,
	OnlineUsingBigPicture = 1024,
};

enum PersonaStatusFlag
{
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
};

enum PersonaRelationshipAction
{
	// friend removed from contact list
	Remove = 0,
	// friend added you to ignore list
	Ignore = 1,
	// friend requested auth
	AuthRequest = 2,
	// friend added you to contact list
	AddToList = 3,
	// friend got (or approved?) your auth request
	AuthRequested = 4,
};

#endif //_STEAM_ENUMS_H_
