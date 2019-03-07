#pragma once

enum
{
	ICON_OTR,
	ICON_PRIVATE,
	ICON_UNVERIFIED,
	ICON_FINISHED,
	ICON_NOT_PRIVATE,
	ICON_REFRESH,
	ICON_MAXID
};

extern IconItem iconList[ICON_MAXID];

void InitIcons();
