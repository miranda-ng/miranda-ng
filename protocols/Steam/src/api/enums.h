#ifndef _STEAM_ENUMS_H_
#define _STEAM_ENUMS_H_

enum class FriendRelationship : int
{
	None = 0,
	Blocked = 1,
	RequestRecipient = 2,
	Friend = 3,
	RequestInitiator = 4,
	Ignored = 5,
	IgnoredFriend = 6,
};

enum class PersonaState : int
{
	Offline = 0,
	Online = 1,
	Busy = 2,
	Away = 3,
	Snooze = 4,
	LookingToTrade = 5,
	LookingToPlay = 6,
	Invisible = 7,
};

enum class PersonaStateFlag : int
{
	None = 0,
	HasRichPresence = 1,
	InJoinableGame = 2,
	ClientTypeWeb = 256,
	ClientTypeMobile = 512,
	ClientTypeBigPicture = 1024,
	ClientTypeVR = 2048,
};

inline PersonaStateFlag operator &(PersonaStateFlag lhs, PersonaStateFlag rhs)
{
	return static_cast<PersonaStateFlag> (
		static_cast<std::underlying_type<PersonaStateFlag>::type>(lhs) &
		static_cast<std::underlying_type<PersonaStateFlag>::type>(rhs));
}

enum class PersonaStatusFlag : int
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
	Unknown = 4096,
};

inline PersonaStatusFlag operator &(PersonaStatusFlag lhs, PersonaStatusFlag rhs)
{
	return static_cast<PersonaStatusFlag> (
		static_cast<std::underlying_type<PersonaStatusFlag>::type>(lhs) &
		static_cast<std::underlying_type<PersonaStatusFlag>::type>(rhs));
}

template<typename T>
bool contains_flag(T x, T y) {
	return (static_cast<typename std::underlying_type<T>::type>(x)
		& static_cast<typename std::underlying_type<T>::type>(y))
		== static_cast<typename std::underlying_type<T>::type>(y);
}

#endif //_STEAM_ENUMS_H_
