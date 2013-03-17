#ifndef guard_speak_config_speech_interface_h
#define guard_speak_config_speech_interface_h
//==============================================================================
// 	Miranda Speak Plugin, © 2002 Ryan Winter
//==============================================================================

#include "defs/voice_desc.h"

#include <string>
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
    TextToSpeech * createTts(std::string &engine) const;

	//--------------------------------------------------------------------------
	// Description : configure the tts object
    // Parameters  : tts  - the tts object to configure
    //               desc - the description of the voice
	//--------------------------------------------------------------------------
    void configureTts(TextToSpeech *tts, const VoiceDesc &desc) const;

	//--------------------------------------------------------------------------
	// Description : create a vector of available engines
	//--------------------------------------------------------------------------
    std::vector<std::string> getAvailableEngines();
};

//==============================================================================
//
//  Summary     : Configure a text to speech object
//
//  Description : Encapsulate the different speech engines available
//
//==============================================================================

#endif