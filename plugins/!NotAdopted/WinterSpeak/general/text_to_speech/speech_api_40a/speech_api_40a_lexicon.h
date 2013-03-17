#ifndef guard_general_text_to_speech_speech_api_40a_speech_api_40a_lexicon_h
#define guard_general_text_to_speech_speech_api_40a_speech_api_40a_lexicon_h
//==============================================================================
// General Code, © 2002 Ryan Winter
//==============================================================================

#include <wtypes.h>

struct ITTSCentralA;

class SpeechApi40aLexicon
{
  public:
	//--------------------------------------------------------------------------
	// Description : Constuctor
    // Parameters  : window - handle to the parent window
    //               tts_central - pointer to the tts_central engine to use
	//--------------------------------------------------------------------------
	SpeechApi40aLexicon(HWND window, ITTSCentralA *tts_central);

	~SpeechApi40aLexicon();

	//--------------------------------------------------------------------------
	// Description : display the lexicon dialog
	// Return      : true - display ok
	//               false - display failed
	//--------------------------------------------------------------------------
    bool display();

  private:
    HWND          m_window;
    ITTSCentralA *m_tts_central;
};

//==============================================================================
//
//  Summary     : Lexicon dialog
//
//  Description : Displays the lexicon dialog for the speech api 4.0a which
//                allows the user to add/remove/edit the user lexicon database
//
//==============================================================================

#endif