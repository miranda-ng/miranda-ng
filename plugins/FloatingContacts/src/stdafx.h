
#ifndef __STDHDR_H__
#define __STDHDR_H__

#include <windows.h>
#include <assert.h>
#include <math.h>
#include <Shlwapi.h>
#include <CommCtrl.h>

#include <newpluginapi.h>
#include <m_contacts.h>
#include <m_skin.h>
#include <m_file.h>
#include <m_fontservice.h>
#include <m_langpack.h>
#include <m_protosvc.h>
#include <m_database.h>
#include <m_message.h>
#include <m_options.h>
#include <m_icolib.h>
#include <m_clistint.h>
#include <m_hotkeys.h>
#include <m_imgsrvc.h>

#include "bitmap_funcs.h"
#include "fltcont.h"
#include "thumbs.h"
#include "filedrop.h"
#include "resource.h"
#include "version.h"

/////////////////////////////////////////////////////////////////////////////

#define MODULENAME "FloatingContacts"

struct CMPlugin : public PLUGIN<CMPlugin>
{
	CMPlugin();

	int Load() override;
	int Unload() override;
};

#endif	// #ifndef __STDHDR_H__

/////////////////////////////////////////////////////////////////////////////
// End Of File stdhdr.h
