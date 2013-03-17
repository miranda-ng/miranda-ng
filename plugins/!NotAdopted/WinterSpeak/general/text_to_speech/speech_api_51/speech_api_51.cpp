//==============================================================================
// General Code, © 2002 Ryan Winter
//==============================================================================

#pragma warning(disable:4786)

#include "speech_api_51.h"

#include <general/debug/debug.h>
#include <general/text_to_speech/speech_api_51/speech_api_51_lexicon.h>

#include <sapi.h>
#include <sphelper.h>

#include <sstream>
#include <memory>

namespace
{

//------------------------------------------------------------------------------
// Description : implement callback
//------------------------------------------------------------------------------
/*class Sapi51Callback : public ISpNotifyCallback
{
  public:
    Sapi51Callback();

    virtual STDMETHODIMP NotifyCallback(WPARAM wParam, LPARAM lParam) 
    {

        return S_OK;
    }
};*/

}

//------------------------------------------------------------------------------
SpeechApi51::SpeechApi51()
	:
	m_sapi(0),
	m_state(TextToSpeech::State_Unloaded),
	m_voice(""),
	m_volume(50),
	m_pitch(50),
	m_rate(50)
{
	CLASSCERR("SpeechApi51::SpeechApi51");
}

//------------------------------------------------------------------------------
SpeechApi51::~SpeechApi51()
{
	CLASSCERR("SpeechApi51::~SpeechApi51");

	unload();

	CoUninitialize();
}

//------------------------------------------------------------------------------
bool 
SpeechApi51::isAvailable()
{
	CLASSCERR("SpeechApi51::isAvailable");

	CoInitialize(NULL);

	ISpVoice *sapi;
    bool      ret = true;

    if (FAILED(CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, 
		reinterpret_cast<void **>(&sapi))))
	{
        ret = false;
	}
    else
    {
        sapi->Release();
    }

	CLASSCERR("SpeechApi51::isAvailable return " << (ret?"T":"F"));
    return ret;
}

//------------------------------------------------------------------------------
bool
SpeechApi51::load()
{
	CLASSCERR("SpeechApi51::load");

	if (isLoaded())
	{
		CLASSCERR("SpeechApi51::load engine already loaded");
		return true;
	}

	CoInitialize(NULL);

    if (FAILED(CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, 
		reinterpret_cast<void **>(&m_sapi))))
	{
		CLASSCERR("SpeechApi51::load could not initialise sapi");
		return false;
	}

	m_state = TextToSpeech::State_Loaded;

	// adjust the volume and rate settings
	setVoice(m_voice);
	setVolume(m_volume);
	setRate(m_rate);
	
	CLASSCERR("SpeechApi51::load return success");
	return true;
}

//------------------------------------------------------------------------------
bool
SpeechApi51::unload()
{
	CLASSCERR("SpeechApi51::unload");

	if (isLoaded())
	{
		m_sapi->Release();
		m_sapi = 0;
	}

	m_state = TextToSpeech::State_Unloaded;
	return true;
}

//------------------------------------------------------------------------------
bool 
SpeechApi51::isLoaded() const
{
	return (TextToSpeech::State_Loaded == m_state);
}

//------------------------------------------------------------------------------
bool
SpeechApi51::say(const std::string &sentence)
{
	CLASSCERR("SpeechApi51::say(" << sentence << ")");

	if (!isLoaded())
	{
    	CLASSCERR("SpeechApi51::say() engine not loaded");
		return false;
	}

	// prepend the pitch setting
	std::stringstream output;
	output << "<pitch middle='" << m_pitch << "'/> " << sentence;

	std::auto_ptr<wchar_t> sapi_sentence(new wchar_t[output.str().size() + 1]);
	mbstowcs(sapi_sentence.get(), output.str().c_str(), output.str().size() + 1);
    
	// speak the sentence
	if (FAILED(m_sapi->Speak(sapi_sentence.get(), SPF_IS_XML | SPF_ASYNC, NULL)))
	{
    	CLASSCERR("SpeechApi51::say() failed to speak sentence");
		return false;
	}

  	CLASSCERR("SpeechApi51::say() succeeded");
	return true;
}

//------------------------------------------------------------------------------
bool
SpeechApi51::setVolume(int volume)
{
	m_volume = volume;

	if (isLoaded() && FAILED(m_sapi->SetVolume(volume)))
	{
		return false;
	}

	return true;
}

//------------------------------------------------------------------------------
bool
SpeechApi51::setPitch(int pitch)
{
	m_pitch = (pitch / 5) - 10;

	return true;
}	

//------------------------------------------------------------------------------
bool
SpeechApi51::setRate(int rate)
{
	m_rate = rate;

	// convert it to the range -10 to 10
	if (isLoaded() && FAILED(m_sapi->SetRate((rate / 5.0) - 10)))
	{
		return false;
	}

	return true;
}

//------------------------------------------------------------------------------
bool 
SpeechApi51::setVoice(const std::string &voice)
{
	CLASSCERR("SpeechApi51::setVoice(" << voice << ")");

	m_voice = voice;

	if (!isLoaded())
	{
    	CLASSCERR("SpeechApi51::setVoice() engine not loaded");
		return true;
	}

    // get a voice enumerator
	CComPtr<IEnumSpObjectTokens> cpEnum;
	if (FAILED(SpEnumTokens(SPCAT_VOICES, NULL, NULL, &cpEnum)))
	{
    	CLASSCERR("SpeechApi51::setVoice failed to load enumerator");
		return false;
	}

	// iterate through the list till we find a matching voice
	ISpObjectToken *voice_token;
	while (S_OK == cpEnum->Next(1, &voice_token, NULL))
	{
	    CSpDynamicString voice_str;

        if (SUCCEEDED(SpGetDescription(voice_token, &voice_str))
            && (voice == voice_str.CopyToChar()))
        {
   			m_sapi->SetVoice(voice_token);
            CLASSCERR("SpeechApi51::setVoice() voice found");
		    return true;
        }
	}

   	CLASSCERR("SpeechApi51::setVoice() voice not found");
	return false;
}

//------------------------------------------------------------------------------
std::vector<std::string> 
SpeechApi51::getVoices() const
{
	CLASSCERR("SpeechApi51::getVoices");

	std::vector<std::string> ret;
	
	CoInitialize(NULL);

    // get a voice enumerator
    CComPtr<IEnumSpObjectTokens> cpEnum;
	if (S_OK != SpEnumTokens(SPCAT_VOICES, NULL, NULL, &cpEnum))
	{
    	CLASSCERR("SpeechApi51::getVoices failed to load enumerator");
		return ret;
	}

	// iterate through the voices and add them to the string vector
    ISpObjectToken *voice_token;
    while (S_OK == cpEnum->Next(1, &voice_token, NULL))
    {
	    CSpDynamicString voice_str;

        if (SUCCEEDED(SpGetDescription(voice_token, &voice_str)))
        {
		    ret.push_back(voice_str.CopyToChar());
        }
	}

	CLASSCERR("SpeechApi51::getVoices return");
	return ret;
}


//------------------------------------------------------------------------------
bool 
SpeechApi51::lexiconDialog(HWND window)
{
	CLASSCERR("SpeechApi51::lexiconDialog");

    // open the dialog
    SpeechApi51Lexicon dialog(window);

    if (!dialog.display())
    {
        return false;
    }

    return true;
}

//------------------------------------------------------------------------------
std::string 
SpeechApi51::getDescription()
{
	CERR("SpeechApi51::getDescription");

	return "Microsoft SAPI v5.1";
}

//------------------------------------------------------------------------------
