#include "stdafx.h"

IconItemT CToxProto::Icons[] =
{
	{ LPGENT("Protocol icon"),			"main",				IDI_TOX			},
	{ LPGENT("Audio call"),				"audio_call",		IDI_AUDIO_CALL	},
	{ LPGENT("Audio ring"),				"audio_ring",		IDI_AUDIO_RING	},
	{ LPGENT("Audio start"),			"audio_start",		IDI_AUDIO_START	},
	{ LPGENT("Audio end"),				"audio_end",		IDI_AUDIO_END	},
};

void CToxProto::InitIcons()
{
	Icon_RegisterT(g_hInstance, LPGENT("Protocols") L"/" LPGENT(MODULE), Icons, _countof(Icons), MODULE);
}

HANDLE CToxProto::GetIconHandle(int iconId)
{
	for (size_t i = 0; i < _countof(Icons); i++)
		if (Icons[i].defIconID == iconId)
			return Icons[i].hIcolib;

	return NULL;
}