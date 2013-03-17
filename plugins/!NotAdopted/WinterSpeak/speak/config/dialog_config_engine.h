#ifndef guard_speak_config_dialog_config_engine_h
#define guard_speak_config_dialog_config_engine_h
//==============================================================================
// 	Miranda Speak Plugin, © 2002 Ryan Winter
//==============================================================================

#include "dialog/miranda_dialog.h"
#include "config/config_database.h"

#include <wtypes.h>
#include <memory>

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
    static int CALLBACK process(HWND window, UINT message, WPARAM wparam, 
        LPARAM lparam);

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
    std::string getEngine(HWND window);
    bool createTts(HWND window);

    static DialogConfigEngine   *m_instance;
	ConfigDatabase              &m_db;
    std::auto_ptr<TextToSpeech>  m_test_tts;
};

//==============================================================================
//
//  Summary     : Configuration Engine Dialog box
//
//  Description : Set up the configuration dialog box and process its input 
//
//==============================================================================

#endif