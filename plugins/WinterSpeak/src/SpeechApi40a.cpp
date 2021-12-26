#include "stdafx.h"
#include "SpeechApi40a.h"

#include "SpeechApi40aLexicon.h"

#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <MMSystem.h>
#include <InitGuid.h>
#include <ObjBase.h>
#include <ObjError.h>
#include <Ole2Ver.h>

#include "SAPI 4.0/Include/speech.h"
//#include <spchwrap.h>

#include <sstream>
#include <memory>

//------------------------------------------------------------------------------
SpeechApi40a::SpeechApi40a() : m_tts_central(nullptr), m_tts_attribs(nullptr), m_state(TextToSpeech::State_Unloaded), m_voice(L""), m_volume(50), m_pitch(50), m_rate(50)
{
}

//------------------------------------------------------------------------------
SpeechApi40a::~SpeechApi40a()
{
	unload();
	CoUninitialize();
}

//------------------------------------------------------------------------------
bool SpeechApi40a::isAvailable()
{
	CoInitialize(nullptr);

	PITTSENUM pITTSEnum;
	bool      ret = true;

	// create the enumerator
	if (FAILED(CoCreateInstance(CLSID_TTSEnumerator, nullptr, CLSCTX_ALL, IID_ITTSEnum, (void**)&pITTSEnum))) {
		ret = false;
	}
	else {
		pITTSEnum->Release();
	}
	return ret;
}

//------------------------------------------------------------------------------
bool SpeechApi40a::load()
{
	if (isLoaded()) {
		return true;
	}
	return loadWithVoice(std::wstring(m_voice));
}

//------------------------------------------------------------------------------
bool SpeechApi40a::unload()
{
	if (m_tts_attribs) {
		m_tts_attribs->Release();
		m_tts_attribs = nullptr;
	}

	if (m_tts_central) {
		m_tts_central->Release();
		m_tts_central = nullptr;
	}

	m_state = TextToSpeech::State_Unloaded;
	return true;
}

//------------------------------------------------------------------------------
bool SpeechApi40a::isLoaded() const
{
	return (TextToSpeech::State_Loaded == m_state);
}

//------------------------------------------------------------------------------
bool SpeechApi40a::say(const std::wstring &sentence)
{
	//std::string text = mir_u2a_cp(sentence.c_str(), CP_ACP);
	//MessageBoxA(NULL, text.c_str(), "TTS4", MB_OK);
	bool ret = true;

	if (!isLoaded()) {
		ret = false;
	}
	else {
		SDATA data;
		data.dwSize = (uint32_t)(sentence.size() * sizeof(wchar_t));
		data.pData = (wchar_t *)sentence.c_str();
		m_tts_central->TextData(CHARSET_TEXT, 0, data, nullptr, IID_ITTSBufNotifySinkA);
	}

	return ret;
}

//------------------------------------------------------------------------------
bool SpeechApi40a::setVolume(int volume)
{
	m_volume = volume;

	if (!isLoaded()) {
		return true;
	}

	uint32_t new_vol = volume / 100.0 * 0xffff;
	new_vol |= new_vol << 16;

	if (FAILED(m_tts_attribs->VolumeSet(new_vol))) {
		return false;
	}

	return true;
}

//------------------------------------------------------------------------------
bool SpeechApi40a::setPitch(int pitch)
{
	m_pitch = pitch;

	// valid range is 50 to 350
	if (isLoaded() && FAILED(m_tts_attribs->PitchSet(pitch * 3.0 + 50))) {
		return false;
	}

	return true;
}

//------------------------------------------------------------------------------
bool SpeechApi40a::setRate(int rate)
{
	m_rate = rate;

	// valid range is 50 to 350
	if (isLoaded() && FAILED(m_tts_attribs->SpeedSet(rate * 3.0 + 50))) {
		return false;
	}

	return true;
}

//------------------------------------------------------------------------------
bool  SpeechApi40a::setVoice(const std::wstring &voice)
{
	m_voice = voice;

	if (!isLoaded()) {
		return true;
	}

	unload();
	return load();
}

//------------------------------------------------------------------------------
std::vector<std::wstring> SpeechApi40a::getVoices() const
{
	std::vector<std::wstring> ret;

	PITTSENUM pITTSEnum = nullptr;
	TTSMODEINFO inf;

	CoInitialize(nullptr);

	if (FAILED(CoCreateInstance(CLSID_TTSEnumerator, nullptr, CLSCTX_ALL, IID_ITTSEnum, (void**)&pITTSEnum)))
		return ret;

	while (SUCCEEDED(pITTSEnum->Next(1, &inf, nullptr)))
		ret.push_back(inf.szModeName);

	pITTSEnum->Release();

	return ret;
}

//------------------------------------------------------------------------------
bool SpeechApi40a::lexiconDialog(HWND window)
{
	// open the dialog
	SpeechApi40aLexicon dialog(window, m_tts_central);
	if (!dialog.display())
		return false;

	return true;
}

//------------------------------------------------------------------------------
std::wstring SpeechApi40a::getDescription()
{
	return L"Microsoft SAPI v4.0";
}

//------------------------------------------------------------------------------
// private:
//------------------------------------------------------------------------------
bool SpeechApi40a::loadWithVoice(const std::wstring &voice)
{
	CoInitialize(nullptr);

	PITTSENUM    pITTSEnum;
	TTSMODEINFO  inf;
	LPUNKNOWN    pAudioDest;

	// create the enumerator
	if (FAILED(CoCreateInstance(CLSID_TTSEnumerator, nullptr, CLSCTX_ALL, IID_ITTSEnum, (void**)&pITTSEnum)))
		return false;

	// iterate through the voices until we find the right one
	while (SUCCEEDED(pITTSEnum->Next(1, &inf, nullptr)))
		if (inf.szModeName == voice)
			break;

	if (FAILED(CoCreateInstance(CLSID_MMAudioDest, nullptr, CLSCTX_ALL, IID_IAudioMultiMediaDevice, (void**)&pAudioDest))) {
		pITTSEnum->Release();
		return false;
	}

	// select that voice
	if (FAILED(pITTSEnum->Select(inf.gModeID, &m_tts_central, pAudioDest))) {
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
	return true;
}
