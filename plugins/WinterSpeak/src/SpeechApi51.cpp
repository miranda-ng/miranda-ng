#include "Common.h"
#include "SpeechApi51.h"
#include "SpeechApi51Lexicon.h"

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
SpeechApi51::SpeechApi51() : m_sapi(0), m_state(TextToSpeech::State_Unloaded), 	m_voice(L""), m_volume(50), m_pitch(50), m_rate(50)
{
}

//------------------------------------------------------------------------------
SpeechApi51::~SpeechApi51()
{
	unload();

	CoUninitialize();
}

//------------------------------------------------------------------------------
bool SpeechApi51::isAvailable()
{
	CoInitialize(NULL);

	ISpVoice *sapi;
	bool      ret = true;

	if (FAILED(CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, reinterpret_cast<void **>(&sapi))))
	{
		ret = false;
	}
	else
	{
		sapi->Release();
	}

	return ret;
}

//------------------------------------------------------------------------------
bool SpeechApi51::load()
{
	if (isLoaded())
	{
		return true;
	}

	CoInitialize(NULL);

	if (FAILED(CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, reinterpret_cast<void **>(&m_sapi))))
	{
		return false;
	}

	m_state = TextToSpeech::State_Loaded;

	// adjust the volume and rate settings
	setVoice(m_voice);
	setVolume(m_volume);
	setRate(m_rate);
	
	return true;
}

//------------------------------------------------------------------------------
bool SpeechApi51::unload()
{
	if (isLoaded())
	{
		m_sapi->Release();
		m_sapi = 0;
	}

	m_state = TextToSpeech::State_Unloaded;
	return true;
}

//------------------------------------------------------------------------------
bool  SpeechApi51::isLoaded() const
{
	return (TextToSpeech::State_Loaded == m_state);
}

//------------------------------------------------------------------------------
bool SpeechApi51::say(const std::wstring &sentence)
{
	if (!isLoaded())
	{
		return false;
	}

	// prepend the pitch setting
	std::wstringstream output;
	output << "<pitch middle='" << m_pitch << "'/> " << sentence;
	
	const std::wstring& wstr = output.str(); // extends lifetime of temporary 
	const LPCWSTR p = wstr.c_str();
	//std::auto_ptr<wchar_t> sapi_sentence(new wchar_t[output.str().size() + 1]);
	//mbstowcs(sapi_sentence.get(), output.str().c_str(), output.str().size() + 1);
	
	// speak the sentence
	if (FAILED(m_sapi->Speak(p, SPF_IS_XML | SPF_ASYNC, NULL)))
	{
		return false;
	}

	return true;
}

//------------------------------------------------------------------------------
bool SpeechApi51::setVolume(int volume)
{
	m_volume = volume;

	if (isLoaded() && FAILED(m_sapi->SetVolume(volume)))
	{
		return false;
	}

	return true;
}

//------------------------------------------------------------------------------
bool SpeechApi51::setPitch(int pitch)
{
	m_pitch = (pitch / 5) - 10;

	return true;
}	

//------------------------------------------------------------------------------
bool SpeechApi51::setRate(int rate)
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
bool SpeechApi51::setVoice(const std::wstring &voice)
{
	m_voice = voice;

	if (!isLoaded())
	{
		return true;
	}

	// get a voice enumerator
	CComPtr<IEnumSpObjectTokens> cpEnum;
	if (FAILED(SpEnumTokens(SPCAT_VOICES, NULL, NULL, &cpEnum)))
	{
		return false;
	}

	// iterate through the list till we find a matching voice
	ISpObjectToken *voice_token;
	while (S_OK == cpEnum->Next(1, &voice_token, NULL))
	{
		CSpDynamicString voice_str;

		if (SUCCEEDED(SpGetDescription(voice_token, &voice_str))
			&& (voice == voice_str.Copy()))
		{
			m_sapi->SetVoice(voice_token);
			return true;
		}
	}

	return false;
}

//------------------------------------------------------------------------------
std::vector<std::wstring> SpeechApi51::getVoices() const
{
	std::vector<std::wstring> ret;
	
	CoInitialize(NULL);

	// get a voice enumerator
	CComPtr<IEnumSpObjectTokens> cpEnum;
	if (S_OK != SpEnumTokens(SPCAT_VOICES, NULL, NULL, &cpEnum))
	{
		return ret;
	}

	// iterate through the voices and add them to the string vector
	ISpObjectToken *voice_token;
	while (S_OK == cpEnum->Next(1, &voice_token, NULL))
	{
		CSpDynamicString voice_str;

		if (SUCCEEDED(SpGetDescription(voice_token, &voice_str)))
		{
			ret.push_back(voice_str.Copy());
		}
	}

	return ret;
}


//------------------------------------------------------------------------------
bool SpeechApi51::lexiconDialog(HWND window)
{
	// open the dialog
	SpeechApi51Lexicon dialog(window);

	if (!dialog.display())
	{
		return false;
	}

	return true;
}

//------------------------------------------------------------------------------
std::wstring SpeechApi51::getDescription()
{
	return L"Microsoft SAPI v5.1";
}

//------------------------------------------------------------------------------