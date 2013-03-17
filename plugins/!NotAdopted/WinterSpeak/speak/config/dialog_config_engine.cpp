//==============================================================================
// 	Miranda Speak Plugin, © 2002 Ryan Winter
//==============================================================================

#pragma warning(disable:4786)

#include "dialog_config_engine.h"

#include "speak.h"
#include "resource.h"
#include "config/speech_interface.h"

#include <general/debug/debug.h>
#include <general/text_to_speech/text_to_speech/text_to_speech.h>

#include <windows.h>
#include <commctrl.h>

//------------------------------------------------------------------------------
DialogConfigEngine *DialogConfigEngine::m_instance = 0;

//------------------------------------------------------------------------------
// public:
//------------------------------------------------------------------------------
DialogConfigEngine::DialogConfigEngine(ConfigDatabase &db)
	:
	m_db(db),
    m_test_tts(0)
{
	CLASSCERR("DialogConfigEngine::DialogConfigEngine");

    m_instance = this;
}

//------------------------------------------------------------------------------
DialogConfigEngine::~DialogConfigEngine()
{
	CLASSCERR("DialogConfigEngine::~DialogConfigEngine");

    m_instance = 0;
}

//------------------------------------------------------------------------------
int CALLBACK 
DialogConfigEngine::process(HWND window, UINT message, WPARAM wparam, 
    LPARAM lparam)
{
    if (!m_instance)
    {
        return 1;
    }

	switch (message)
	{
      case WM_INITDIALOG:
        m_instance->load(window);
        break;

	  case WM_NOTIFY:
    	if (PSN_APPLY == reinterpret_cast<LPNMHDR>(lparam)->code)
    	{	
		    m_instance->save(window);
            m_instance->m_db.save();
	    }
        break;

	  case WM_HSCROLL:
		m_instance->changed(window);
		break;

	  case WM_COMMAND:
		m_instance->command(window, wparam);
        break;
	}

    return 0;
}

//------------------------------------------------------------------------------
// private:
//------------------------------------------------------------------------------
void
DialogConfigEngine::command(HWND window, int control)
{
	switch (LOWORD(control))
	{
	  case IDC_WELCOME_MSG:
        if (EN_CHANGE == HIWORD(control))
        {
            changed(window);
        }
        break;

      case IDC_SELECT_VOICE:
        if (CBN_SELCHANGE == HIWORD(control))
        {
            changed(window);
        }
        break;

      case IDC_SELECT_ENGINE:
        if (CBN_SELCHANGE == HIWORD(control))
        {
            updateVoices(window);
            changed(window);
        }
		break;

      case IDC_CONFIG_LEXICON:
        if (createTts(window))
        {
            if (!m_test_tts->lexiconDialog(window))
            {
   				MessageBox(window, "Lexicon for this engine is not supported",
                    "Speak", MB_OK | MB_ICONEXCLAMATION);
            }
        }
        break;

	  case IDC_BUTTON_TEST:
        if (createTts(window))
        {
            m_test_tts->say(Translate("testing testing 1 2 3"));
        }
        break;
	}
}

//------------------------------------------------------------------------------
void
DialogConfigEngine::load(HWND window)
{
	CLASSCERR("DialogConfigEngine::load()");

	TranslateDialogDefault(window);

    // add the available engines to the combo box
    SpeechInterface si;
    std::vector<std::string> engines = si.getAvailableEngines();
    for (unsigned int i = 0; i < engines.size(); ++i)
    {
    	CLASSCERR("DialogConfigEngine::load adding " << engines[i].c_str());
        
    	SendDlgItemMessage(window, IDC_SELECT_ENGINE, CB_ADDSTRING, 0, 
	    	(long)engines[i].c_str());
    }

    VoiceDesc desc = m_db.getVoiceDesc();

	// initialise the sliders
	SendDlgItemMessage(window, IDC_SLIDER_VOLUME, TBM_SETPOS, TRUE, 
		desc.volume);
	SendDlgItemMessage(window, IDC_SLIDER_PITCH, TBM_SETPOS, TRUE, 
	    desc.pitch);
	SendDlgItemMessage(window, IDC_SLIDER_RATE, TBM_SETPOS, TRUE, 
		desc.rate);

    // select the speech engine
	SendDlgItemMessage(window, IDC_SELECT_ENGINE, CB_SELECTSTRING, 0, 
		reinterpret_cast<long>(desc.engine.c_str()));

	// initialise the welcome message box
	SetDlgItemText(window, IDC_WELCOME_MSG, m_db.getWelcomeMessage().c_str());

    updateVoices(window);
}

//------------------------------------------------------------------------------
void
DialogConfigEngine::save(HWND window)
{
	CLASSCERR("DialogConfigEngine::save()");

    VoiceDesc desc;
    getVoiceDesc(window, desc);
    m_db.setVoiceDesc(desc);

	// store the welcome message
	char text[512];
	GetDlgItemText(window, IDC_WELCOME_MSG, text, sizeof(text));
	m_db.setWelcomeMessage(std::string(text));
}

//------------------------------------------------------------------------------
void 
DialogConfigEngine::updateVoices(HWND window)
{
	CLASSCERR("DialogConfigEngine::updateVoices()");

    SpeechInterface si;
    m_test_tts = std::auto_ptr<TextToSpeech>(si.createTts(getEngine(window)));

    if (!m_test_tts.get())
    {
        // we couldnt open the text to speech engine
    	CLASSCERR("DialogConfigEngine::updateVoices invalid tts");
        return;
    }

    // add the voices onto the list
    std::vector<std::string> voices = m_test_tts->getVoices();

	SendDlgItemMessage(window, IDC_SELECT_VOICE, CB_RESETCONTENT, 0, 0);
	for (unsigned int i = 0; i < voices.size(); ++i)
	{
		SendDlgItemMessage(window, IDC_SELECT_VOICE, CB_ADDSTRING, 0, 
			(long)voices[i].c_str());
	}

    // get the voice saved in the database
    std::string voice = m_db.getVoiceDesc().voice;
    
    if (FAILED(SendDlgItemMessage(window, IDC_SELECT_VOICE, 
        CB_FINDSTRINGEXACT, 0, (long)voice.c_str())))
    {
        // select the first one
	    SendDlgItemMessage(window, IDC_SELECT_VOICE, CB_SETCURSEL , 0, 0);
    }
    else
    {
        // select the saved voice
    	SendDlgItemMessage(window, IDC_SELECT_VOICE, CB_SELECTSTRING, 0, 
	    	(long)voice.c_str());
    }
}

//------------------------------------------------------------------------------
void
DialogConfigEngine::getVoiceDesc(HWND window, VoiceDesc &desc)
{
	CLASSCERR("DialogConfigEngine::getVoiceDesc(,)");

	// get the engine
	char text[100];
	GetDlgItemText(window, IDC_SELECT_ENGINE, text, sizeof(text));
	desc.engine = std::string(text);

	// get the voice
	std::auto_ptr<char> voice(new char[50]);
	SendDlgItemMessage(window, IDC_SELECT_VOICE, CB_GETLBTEXT, 
		SendDlgItemMessage(window, IDC_SELECT_VOICE, CB_GETCURSEL, 0, 0), 
		reinterpret_cast<long>(voice.get()));
    desc.voice = voice.get();

    // get the voice setting sliders
	desc.volume 
        = SendDlgItemMessage(window, IDC_SLIDER_VOLUME, TBM_GETPOS, 0, 0);
	desc.pitch 
        = SendDlgItemMessage(window, IDC_SLIDER_PITCH, TBM_GETPOS, 0, 0);
	desc.rate 
        = SendDlgItemMessage(window, IDC_SLIDER_RATE, TBM_GETPOS, 0, 0);
}

//------------------------------------------------------------------------------
std::string
DialogConfigEngine::getEngine(HWND window)
{
	CLASSCERR("DialogConfigEngine::getEngine()");

	// store the engine
	char text[100];
	GetDlgItemText(window, IDC_SELECT_ENGINE, text, sizeof(text));

	CLASSCERR("DialogConfigEngine::getEngine() return " << text);
    return text;
}

//------------------------------------------------------------------------------
bool
DialogConfigEngine::createTts(HWND window)
{
    CLASSCERR("DialogConfigEngine::createTts()");

    VoiceDesc desc;
    getVoiceDesc(window, desc);

    SpeechInterface si;
    m_test_tts = std::auto_ptr<TextToSpeech>(si.createTts(desc.engine));

    if (!m_test_tts.get())
    {
        return false;
    }
    
    si.configureTts(m_test_tts.get(), desc);
    return true;
}

//==============================================================================
