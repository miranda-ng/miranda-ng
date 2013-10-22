#include "Common.h"
#include "SpeechInterface.h"
#include "SpeechApi51.h"
#include "SpeechApi40a.h"


SpeechInterface::SpeechInterface()
{
}

//------------------------------------------------------------------------------
SpeechInterface::~SpeechInterface()
{
}

//------------------------------------------------------------------------------
TextToSpeech * SpeechInterface::createTts(std::wstring &engine) const
{
	TextToSpeech *tts = 0;

	if (SpeechApi40a::getDescription() == engine)
	{
		tts = new SpeechApi40a();
	}
	else
	if (SpeechApi51::getDescription() == engine)
	{
		tts = new SpeechApi51();
	}

	return tts;
}

//------------------------------------------------------------------------------
void SpeechInterface::configureTts(TextToSpeech *tts, const VoiceDesc &desc) const
{
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
std::vector<std::wstring> SpeechInterface::getAvailableEngines()
{
	std::vector<std::wstring> engines;
    
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
