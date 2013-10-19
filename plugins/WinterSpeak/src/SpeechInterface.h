#pragma once

#include <vector>

class TextToSpeech;

class SpeechInterface
{
  public:
    SpeechInterface();
    ~SpeechInterface();

	//--------------------------------------------------------------------------
	// Description : create the text to speech object
    // Parameters  : engine - the name of the engine to create
    // Returns     : an instance of the text to speech engine
	//--------------------------------------------------------------------------
    TextToSpeech * createTts(std::wstring &engine) const;

	//--------------------------------------------------------------------------
	// Description : configure the tts object
    // Parameters  : tts  - the tts object to configure
    //               desc - the description of the voice
	//--------------------------------------------------------------------------
    void configureTts(TextToSpeech *tts, const VoiceDesc &desc) const;

	//--------------------------------------------------------------------------
	// Description : create a vector of available engines
	//--------------------------------------------------------------------------
    std::vector<std::wstring> getAvailableEngines();
};

