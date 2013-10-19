#pragma once

#include "Common.h"
#include <string>
#include <vector>

class TextToSpeech
{
  public:
	enum State
	{
		State_Loaded,
		State_Unloaded,
	};

	TextToSpeech();
	virtual ~TextToSpeech();

	//--------------------------------------------------------------------------
	// Description : is the api available for use
	// Return      : true - it is available
	//               false - it is not available
	//--------------------------------------------------------------------------
	virtual bool isAvailable() = 0;

	//--------------------------------------------------------------------------
	// Description : load/unload/reload the speech api
	// Return      : true - the action succeeded
	//               false - the action failed
	//--------------------------------------------------------------------------
	virtual bool load() = 0;
	virtual bool unload() = 0;

	//--------------------------------------------------------------------------
	// Description : check if the speech api is loaded
	// Return      : true - the speech_api is loaded
	//               false - its not loaded
	//--------------------------------------------------------------------------	
	virtual bool isLoaded() const = 0;
	
	//--------------------------------------------------------------------------
	// Description : speak a sentence
	// Parameters  : sentence - the sentence to speak
	// Returns     : true - speak successful
	//               false - speak failed
	//--------------------------------------------------------------------------
	virtual bool say(const std::wstring &sentence) = 0;

	//--------------------------------------------------------------------------
	// Description : set the voice settings
	// Parameters  : range from 0 to 100
	//--------------------------------------------------------------------------
	virtual bool setVolume(int volume) = 0;
	virtual bool setPitch(int pitch) = 0;
	virtual bool setRate(int rate) = 0;

	//--------------------------------------------------------------------------
	// Description : set the voice
	//--------------------------------------------------------------------------
	virtual bool setVoice(const std::wstring &voice) = 0;

	//--------------------------------------------------------------------------
	// Description : get the available voices
	//--------------------------------------------------------------------------
	virtual std::vector<std::wstring> getVoices() const = 0;

	//--------------------------------------------------------------------------
	// Description : open the lexicon dialog for this engine
	// Parameters  : window - handle to the parent window
	// Return      : true - dialog completely successfully
	//               false - dialog failed
	//--------------------------------------------------------------------------
	virtual bool lexiconDialog(HWND window) = 0;
};