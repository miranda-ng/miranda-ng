//==============================================================================
// 	Miranda Speak Plugin, © 2002 Ryan Winter
//==============================================================================

#pragma warning(disable:4786)

#include "speech_interface.h"

#include "config/config_database.h"

#include <general/debug/debug.h>
#include <general/text_to_speech/speech_api_40a/speech_api_40a.h>
#include <general/text_to_speech/speech_api_51/speech_api_51.h>

//------------------------------------------------------------------------------
// public:
//------------------------------------------------------------------------------
SpeechInterface::SpeechInterface()
{
	CLASSCERR("SpeechInterface::SpeechInterface");
}

//------------------------------------------------------------------------------
SpeechInterface::~SpeechInterface()
{
	CLASSCERR("SpeechInterface::~SpeechInterface");
}

//------------------------------------------------------------------------------
TextToSpeech *
SpeechInterface::createTts(std::string &engine) const
{
	CLASSCERR("SpeechInterface::createTts()");

	TextToSpeech *tts = 0;

	if (SpeechApi40a::getDescription() == engine)
	{
		tts = new SpeechApi40a();
	}
	else if (SpeechApi51::getDescription() == engine)
	{
		tts = new SpeechApi51();
	}

	CLASSCERR("SpeechInterface::createTts() return " << tts);
	return tts;
}

//------------------------------------------------------------------------------
void
SpeechInterface::configureTts(TextToSpeech *tts, const VoiceDesc &desc) const
{
	CLASSCERR("SpeechInterface::configureTts(" << tts << ",)");

    if (!tts)
    {
        return;
    }
    
    tts->setVoice(desc.voice);
	tts->setVolume(desc.volume);
	tts->setRate(desc.rate);
	tts->setPitch(desc.pitch);
    tts->load();
}

//------------------------------------------------------------------------------
std::vector<std::string>
SpeechInterface::getAvailableEngines()
{
	CLASSCERR("SpeechInterface::getAvailableEngines");

    std::vector<std::string> engines;
    
    SpeechApi40a sapi40a;
    if (sapi40a.isAvailable())
    {
        engines.push_back(SpeechApi40a::getDescription());
    }

    SpeechApi51 sapi51;
    if (sapi51.isAvailable())
    {
        engines.push_back(SpeechApi51::getDescription());
    }

    return engines;
}

//==============================================================================
