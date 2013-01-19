/*
Copyright (C) 2010, 2011 tico-tico
*/

#ifndef __BASS_INTERFACE_H__
#define __BASS_INTERFACE_H__

#include <windows.h>
#include <commctrl.h>

#include <newpluginapi.h>
#include <m_langpack.h>
#include <m_skin.h>
#include <m_database.h>
#include <m_options.h>
#include <win2k.h>
#include <m_cluiframes.h>
#include <m_clui.h>
#include <m_fontservice.h>

#include "m_toptoolbar.h"

#include "resource.h"
#include "version.h"

#define BASSDEF(f) (WINAPI *f)
#include "bass.h"

#define BASS_TCHAR   BASS_UNICODE

#define LOADBASSFUNCTION(f) (*((void**)&f)=(void*)GetProcAddress(hBass,#f))

#define ModuleName "Bass Interface"

#define OPT_VOLUME		"Volume"
#define OPT_BASSPATH	"PathToDll"
#define OPT_MAXCHAN		"MaxChannels"
#define OPT_OUTDEVICE	"OutDevice"
#define OPT_QUIETTIME	"QuietTime"
#define OPT_TIME1		"Time1"
#define OPT_TIME2		"Time2"
#define OPT_STATUS		"StatMask"
#define OPT_PREVIEW		"Preview"
#define OPT_DEVOPEN		"DevOpen"

#endif
