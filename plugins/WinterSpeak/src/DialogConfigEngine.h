#pragma once

#include "MirandaDialog.h"
#include "ConfigDatabase.h"

class TextToSpeech;

class DialogConfigEngine : public MirandaDialog
{
  public:
	DialogConfigEngine(ConfigDatabase &db);
	virtual ~DialogConfigEngine();

    //--------------------------------------------------------------------------
	// Description : process a dialog message
	// Return      : 0 - process ok
    //               1 - error
	//--------------------------------------------------------------------------
    static INT_PTR CALLBACK process(HWND window, UINT message, WPARAM wparam, LPARAM lparam);

  private:
    void command(HWND window, int control);

	//--------------------------------------------------------------------------
	// Description : load/save setting to the miranda database
	//--------------------------------------------------------------------------
	void load(HWND window);
	void save(HWND window);

	//--------------------------------------------------------------------------
	// Description : update the voices combo box
	//--------------------------------------------------------------------------
    void updateVoices(HWND window);
    void getVoiceDesc(HWND window, VoiceDesc &desc);
    std::wstring getEngine(HWND window);
    bool createTts(HWND window);

    static DialogConfigEngine   *m_instance;
	ConfigDatabase              &m_db;
    std::auto_ptr<TextToSpeech>  m_test_tts;
};
