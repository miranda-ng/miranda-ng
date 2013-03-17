//==============================================================================
// General Code, © 2002 Ryan Winter
//==============================================================================

#pragma warning(disable:4786)

#include "speech_api_40a.h"
#include "speech_api_40a_lexicon.h"

#include <general/debug/debug.h>

#include <windows.h>
#include <initguid.h>
#include <speech.h>

#include <sstream>
#include <memory>

//------------------------------------------------------------------------------
SpeechApi40a::SpeechApi40a()
	:
	m_tts_central(0),
    m_tts_attribs(0),
	m_state(TextToSpeech::State_Unloaded),
	m_voice(""),
	m_volume(50),
	m_pitch(50),
	m_rate(50)
{
	CLASSCERR("SpeechApi40a::SpeechApi40a");
}

//------------------------------------------------------------------------------
SpeechApi40a::~SpeechApi40a()
{
	CLASSCERR("SpeechApi40a::~SpeechApi40a");

	unload();

    CoUninitialize();
}

//------------------------------------------------------------------------------
bool 
SpeechApi40a::isAvailable()
{
	CLASSCERR("SpeechApi40a::isAvailable");

    CoInitialize(NULL);

	PITTSENUM pITTSEnum;
    bool      ret = true;

	// create the enumerator
    if (FAILED(CoCreateInstance(CLSID_TTSEnumerator, NULL, CLSCTX_ALL, 
		IID_ITTSEnum, (void**)&pITTSEnum)))
	{
    	ret = false;
	}
    else
    {
	    pITTSEnum->Release();
    }

    CLASSCERR("SpeechApi40a::isAvailable return " << (ret?"T":"F"));
    return ret;
}

//------------------------------------------------------------------------------
bool
SpeechApi40a::load()
{
	CLASSCERR("SpeechApi40a::load");

	if (isLoaded())
	{
		return true;
	}

	return loadWithVoice(std::string(m_voice));
}

//------------------------------------------------------------------------------
bool
SpeechApi40a::unload()
{
	CLASSCERR("SpeechApi40a::unload");

	if (m_tts_attribs) 
	{
		m_tts_attribs->Release();
		m_tts_attribs = 0;
	}

	if (m_tts_central) 
	{
		m_tts_central->Release();
		m_tts_central = 0;
	}

	m_state = TextToSpeech::State_Unloaded;

	return true;
}

//------------------------------------------------------------------------------
bool 
SpeechApi40a::isLoaded() const
{
	return (TextToSpeech::State_Loaded == m_state);
}

//------------------------------------------------------------------------------
bool
SpeechApi40a::say(const std::string &sentence)
{
	CLASSCERR("SpeechApi40a::say(" << sentence << ")");

    bool ret = true;

	if (!isLoaded())
	{
		ret = false;
	}
    else
    {
    	SDATA data;
    	data.dwSize = sentence.size();
    	data.pData = (char *)sentence.c_str();
    	m_tts_central->TextData(CHARSET_TEXT, 0, data, NULL, IID_ITTSBufNotifySinkA);
    }

	CLASSCERR("SpeechApi40a::say() return " << (ret?"T":"F"));
	return ret;
}

//------------------------------------------------------------------------------
bool
SpeechApi40a::setVolume(int volume)
{
	m_volume = volume;

	if (!isLoaded())
	{
		return true;
	}

	DWORD new_vol = volume / 100.0 * 0xffff;
	new_vol |= new_vol << 16;

	if (FAILED(m_tts_attribs->VolumeSet(new_vol)))
	{
		CLASSCERR("SpeechApi40a::setVolume() failed");
		return false;
	}

	return true;
}

//------------------------------------------------------------------------------
bool
SpeechApi40a::setPitch(int pitch)
{
	m_pitch = pitch;

    // valid range is 50 to 350
    if (isLoaded() && FAILED(m_tts_attribs->PitchSet(pitch * 3.0 + 50)))
	{
		return false;
	}

	return true;
}	

//------------------------------------------------------------------------------
bool
SpeechApi40a::setRate(int rate)
{
	m_rate = rate;

	// valid range is 50 to 350
	if (isLoaded() && FAILED(m_tts_attribs->SpeedSet(rate * 3.0 + 50)))
    {
        return false;
    }

	return true;
}

//------------------------------------------------------------------------------
bool 
SpeechApi40a::setVoice(const std::string &voice)
{
	CLASSCERR("SpeechApi40a::setVoice(" << voice << ")");

	m_voice = voice;

	if (!isLoaded())
	{
		return true;
	}

	unload();
    return load();
}

//------------------------------------------------------------------------------
std::vector<std::string> 
SpeechApi40a::getVoices() const
{
	CLASSCERR("SpeechApi40a::getVoice");
	
	std::vector<std::string> ret;

	PITTSENUM pITTSEnum = NULL;
	TTSMODEINFO inf;

    CoInitialize(NULL);

	if (FAILED(CoCreateInstance(CLSID_TTSEnumerator, NULL, CLSCTX_ALL, 
		IID_ITTSEnum, (void**)&pITTSEnum)))
	{
    	CLASSCERR("SpeechApi40a::getVoice failed to create enum");
		return ret;
	}

	while (!pITTSEnum->Next(1, &inf, NULL))
	{
		ret.push_back(inf.szModeName);
	}

	pITTSEnum->Release();

	CLASSCERR("SpeechApi40a::getVoice return");
	return ret;
}

//------------------------------------------------------------------------------
bool
SpeechApi40a::lexiconDialog(HWND window)
{
	CLASSCERR("SpeechApi40a::lexiconDialog");

    // open the dialog
    SpeechApi40aLexicon dialog(window, m_tts_central);

    if (!dialog.display())
    {
        return false;
    }

    return true;
}
    	
//------------------------------------------------------------------------------
std::string 
SpeechApi40a::getDescription()
{
	CERR("SpeechApi40a::getDescription");

	return "Microsoft SAPI v4.0";
}

//------------------------------------------------------------------------------
// private:
//------------------------------------------------------------------------------
bool 
SpeechApi40a::loadWithVoice(std::string &voice)
{
	CLASSCERR("SpeechApi40a::loadWithVoice(" << voice << ")");
	
    CoInitialize(NULL);

	PITTSENUM    pITTSEnum;
	TTSMODEINFO  inf;
	LPUNKNOWN    pAudioDest;

	// create the enumerator
    if (FAILED(CoCreateInstance(CLSID_TTSEnumerator, NULL, CLSCTX_ALL, 
		IID_ITTSEnum, (void**)&pITTSEnum)))
	{
	    CLASSCERR("SpeechApi40a::createTtsEngine() failed to create enum");
    	return false;
	}

	// iterate through the voices until we find the right one
	while (!pITTSEnum->Next(1, &inf, NULL))
	{
		if (inf.szModeName == voice)
		{
			break;
		}
	}

    if (FAILED(CoCreateInstance(CLSID_MMAudioDest, NULL, CLSCTX_ALL, 
        IID_IAudioMultiMediaDevice, (void**)&pAudioDest)))
	{
	    CLASSCERR("SpeechApi40a::createTtsEngine() failed to create audio dest");
        pITTSEnum->Release();
		return false;
	}

	// select that voice
	if (FAILED(pITTSEnum->Select(inf.gModeID, &m_tts_central, pAudioDest)))
	{
	    CLASSCERR("SpeechApi40a::createTtsEngine() failed to select voice");
        pITTSEnum->Release();
		return NULL;
	}

	m_tts_central->QueryInterface(IID_ITTSAttributes, (LPVOID *)&m_tts_attribs);

	pITTSEnum->Release();

	// we made it
	m_state = TextToSpeech::State_Loaded;

    // configure the new voice
    setVolume(m_volume);
    setRate(m_rate);
    setPitch(m_pitch);

	CLASSCERR("SpeechApi40a::loadWithVoice() return successful");
	return true;
}

//------------------------------------------------------------------------------
