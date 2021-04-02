/*
Copyright (C) 2010, 2011 tico-tico
*/

#pragma once

#include <windows.h>
#include <commctrl.h>
#include <Uxtheme.h>

#include <newpluginapi.h>
#include <m_langpack.h>
#include <m_skin.h>
#include <m_database.h>
#include <m_options.h>
#include <m_clistint.h>
#include <m_cluiframes.h>
#include <m_fontservice.h>
#include <m_button_int.h>
#include <m_icolib.h>

#include <m_folders.h>

#include "resource.h"
#include "version.h"

#define MODULENAME LPGEN("BASS Interface")

struct CMPlugin : public PLUGIN<CMPlugin>
{
	CMPlugin();

	int Load() override;
};

#define OPT_VOLUME		"Volume"
#define OPT_BASSPATH	"PathToDll"
#define OPT_MAXCHAN		"MaxChannels"
#define OPT_OUTDEVICE	"OutDevice"
#define OPT_QUIETTIME	"QuietTime"
#define OPT_TIME1		"Time1"
#define OPT_TIME2		"Time2"
#define OPT_STATUS		"StatMask"
#define OPT_PREVIEW		"Preview"
