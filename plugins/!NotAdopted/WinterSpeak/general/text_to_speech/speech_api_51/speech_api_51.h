#ifndef guard_general_text_to_speech_speech_api_51_speech_api_51_h
#define guard_general_text_to_speech_speech_api_51_speech_api_51_h
//==============================================================================
// General Code, © 2002 Ryan Winter
//==============================================================================

#include <general/text_to_speech/text_to_speech/text_to_speech.h>

struct ISpVoice;
class Sapi51Callback;

class SpeechApi51 : public TextToSpeech
{
  public:
	SpeechApi51();
	virtual ~SpeechApi51();

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
    virtual bool say(const std::string &sentence);

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
	virtual bool setVoice(const std::string &voice);

	//--------------------------------------------------------------------------
	// Description : get the available voices
	//--------------------------------------------------------------------------
   	virtual std::vector<std::string> getVoices() const;

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
	static std::string getDescription();

  private:
	ISpVoice                 *m_sapi;

  	TextToSpeech::State       m_state;
	std::string               m_voice;
  	int                       m_volume;
  	int                       m_pitch;
  	int                       m_rate;
};

//==============================================================================
//
//  Summary     : API encapsulation
//
//  Description : This encapsulates the SAPI 5.1 interface
//
//==============================================================================

#endif