//==============================================================================
// General Code, © 2002 Ryan Winter
//==============================================================================

#pragma warning(disable:4786)

#include "speech_api_40a_lexicon.h"

#include <general/debug/debug.h>

#include <windows.h>
#include <speech.h>

//------------------------------------------------------------------------------
SpeechApi40aLexicon::SpeechApi40aLexicon(HWND window, 
    ITTSCentralA *tts_central)
    :
    m_window(window),
    m_tts_central(tts_central)
{
	CLASSCERR("SpeechApi40aLexicon::SpeechApi40aLexicon");
}

//------------------------------------------------------------------------------
SpeechApi40aLexicon::~SpeechApi40aLexicon()
{
    CLASSCERR("SpeechApi40aLexicon::~SpeechApi40aLexicon");
}

//------------------------------------------------------------------------------
bool
SpeechApi40aLexicon::display()
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

//------------------------------------------------------------------------------
