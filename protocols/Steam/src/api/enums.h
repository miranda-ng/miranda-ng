#ifndef _STEAM_ENUMS_H_
#define _STEAM_ENUMS_H_

enum PersonaState
{
	Offline = 0,
	Online = 1,
	Busy = 2,
	Away = 3,
	Snooze = 4,
	LookingToTrade = 5,
	LookingToPlay = 6,
	Max = 7,
};

enum StatusFlags
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
}

#endif //_STEAM_ENUMS_H_
