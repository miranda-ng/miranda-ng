#pragma once

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