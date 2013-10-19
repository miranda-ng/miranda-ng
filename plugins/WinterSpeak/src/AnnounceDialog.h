#pragma once
#include "mirandadialog.h"
#include "AnnounceDatabase.h"

class AnnounceDialog : public MirandaDialog
{
  public:
	AnnounceDialog(AnnounceDatabase &db);
	~AnnounceDialog();

	//--------------------------------------------------------------------------
	// Description : process a dialog message
	// Return      : true - update the systems configuration
	//               false - do nothing
	//--------------------------------------------------------------------------
    static INT_PTR CALLBACK process(HWND window, UINT message, WPARAM wparam, LPARAM lparam);

  private:
    void command(HWND window, int control);

	//--------------------------------------------------------------------------
	// Description : load/save settings to the miranda database
	//--------------------------------------------------------------------------
	void load(HWND window);
	void save(HWND window);

    static AnnounceDialog *m_instance;
	AnnounceDatabase      &m_db;
};

