#ifndef guard_general_text_to_speech_speech_api_51_speech_api_51_lexicon_h
#define guard_general_text_to_speech_speech_api_51_speech_api_51_lexicon_h
//==============================================================================
// General Code, © 2002 Ryan Winter
//==============================================================================

#include <wtypes.h>

class SpeechApi51;

class SpeechApi51Lexicon
{
  public:
	//--------------------------------------------------------------------------
	// Description : Constuctor
    // Parameters  : window - handle to the parent window
	//--------------------------------------------------------------------------
	SpeechApi51Lexicon(HWND window);

	~SpeechApi51Lexicon();

	//--------------------------------------------------------------------------
	// Description : display the lexicon dialog
	// Return      : true - display ok
	//               false - display failed
	//--------------------------------------------------------------------------
    bool display();

  private:
    static INT_PTR CALLBACK dialogEvent(HWND hwndDlg, UINT uMsg, 
        WPARAM wParam, LPARAM lParam);

    void addLexicon();
    void deleteLexicon();
    void displayLexicon();

    HWND m_parent_window;
    HWND m_window;
};

//==============================================================================
//
//  Summary     : Lexicon dialog
//
//  Description : Displays the lexicon dialog for the speech api 5.1 which
//                allows the user to add/remove/edit the user lexicon database
//
//==============================================================================

#endif