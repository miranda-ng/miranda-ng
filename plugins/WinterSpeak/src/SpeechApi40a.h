#pragma once
#include "texttospeech.h"

struct ITTSCentralW;
struct ITTSAttributesA;

class SpeechApi40a : public TextToSpeech
{
  public:
	SpeechApi40a();
	virtual ~SpeechApi40a();

	//--------------------------------------------------------------------------
	// Description : is the api available for use
	// Return      : true - it is available
	//               false - it is not available
	//--------------------------------------------------------------------------
	virtual bool isAvailable();

	//--------------------------------------------------------------------------
	// Description : load/unload/reload the speech api
	// Return      : true - the action succeeded
	//               false - the action failed
	//--------------------------------------------------------------------------
	virtual bool load();
	virtual bool unload();

	//--------------------------------------------------------------------------
	// Description : check if the speech api is loaded
	// Return      : true - the speech_api is loaded
	//               false - its not loaded
	//--------------------------------------------------------------------------
	virtual bool isLoaded() const;

	//--------------------------------------------------------------------------
	// Description : speak a sentence
	// Parameters  : sentence - the sentence to speak
	// Returns     : true - speak successful
	//               false - speak failed
	//--------------------------------------------------------------------------
	virtual bool say(const std::wstring &sentence);

	//--------------------------------------------------------------------------
	// Description : set the voice settings
	// Parameters  : range from 0 to 100
	//--------------------------------------------------------------------------
	virtual bool setVolume(int volume);
	virtual bool setPitch(int pitch);
	virtual bool setRate(int rate);

	//--------------------------------------------------------------------------
	// Description : set the voice
	//--------------------------------------------------------------------------
	virtual bool setVoice(const std::wstring &voice);

	//--------------------------------------------------------------------------
	// Description : get the available voices
	//--------------------------------------------------------------------------
	virtual std::vector<std::wstring> getVoices() const;

	//--------------------------------------------------------------------------
	// Description : open the lexicon dialog for this engine
	// Parameters  : window - handle to the parent window
	// Return      : true - dialog completely successfully
	//               false - dialog failed
	//--------------------------------------------------------------------------
	virtual bool lexiconDialog(HWND window);

	//--------------------------------------------------------------------------
	// Description : get the description of the tts engine
	//--------------------------------------------------------------------------
	static std::wstring getDescription();

  private:
	//--------------------------------------------------------------------------
	// Description : load the speech api with the specified voice
	//--------------------------------------------------------------------------
	bool loadWithVoice(std::wstring &voice);

	ITTSCentralW        *m_tts_central;
	ITTSAttributesA     *m_tts_attribs;

	TextToSpeech::State  m_state;
	std::wstring          m_voice;
	int                  m_volume;
	int                  m_pitch;
	int                  m_rate;
};