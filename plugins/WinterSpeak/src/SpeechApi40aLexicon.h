#pragma once

#include <wtypes.h>

struct ITTSCentralW;
class SpeechApi40aLexicon
{
  public:
	//--------------------------------------------------------------------------
	// Description : Constuctor
	// Parameters  : window - handle to the parent window
	//               tts_central - pointer to the tts_central engine to use
	//--------------------------------------------------------------------------
	SpeechApi40aLexicon(HWND window, ITTSCentralW *tts_central);

	~SpeechApi40aLexicon();

	//--------------------------------------------------------------------------
	// Description : display the lexicon dialog
	// Return      : true - display ok
	//               false - display failed
	//--------------------------------------------------------------------------
	bool display();

  private:
	HWND          m_window;
	ITTSCentralW *m_tts_central;
};