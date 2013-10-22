#include "Common.h"
#include "SpeechApi40aLexicon.h"

#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <MMSystem.h>
#include <InitGuid.h>
#include <ObjBase.h>
#include <ObjError.h>
#include <Ole2Ver.h>

#include <speech.h>
//#include <spchwrap.h>


//------------------------------------------------------------------------------
SpeechApi40aLexicon::SpeechApi40aLexicon(HWND window, ITTSCentralW *tts_central) : m_window(window), m_tts_central(tts_central)
{
}

//------------------------------------------------------------------------------
SpeechApi40aLexicon::~SpeechApi40aLexicon()
{
}

//------------------------------------------------------------------------------
bool SpeechApi40aLexicon::display()
{
	if (!m_tts_central)
	{
		return false;
	}

	ITTSDialogs *tts_dialogs = 0;

	m_tts_central->QueryInterface(IID_ITTSDialogs, (void**)&tts_dialogs);

	if (!tts_dialogs)
	{
		return false;
	}

	if (NOERROR != tts_dialogs->LexiconDlg(m_window, NULL))
	{
		return false;
	}
			
	tts_dialogs->Release();
	return true;
}
