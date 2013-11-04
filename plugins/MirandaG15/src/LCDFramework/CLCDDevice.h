#ifndef _CLCDDEVICE_H_
#define _CLCDDEVICE_H_

#include "g15sdk/lglcd.h"

class CLCDDevice {
private:
	int m_iIndex;

protected:
	CLCDDevice(int iIndex) {
		m_iIndex = iIndex;
	}

public:
	int GetIndex() {
		return m_iIndex;
	}

	// Returns the display name
	virtual tstring GetDisplayName() = NULL;
	// Returns the display size
	virtual SIZE GetDisplaySize() = NULL;
	// Returns the number of buttons for the display
	virtual int GetButtonCount() = NULL;
	// Returns the number of available colors
	virtual int GetColorCount() = NULL;
};

class CLgLCDDevice : public CLCDDevice {
private:
	SIZE m_size;
	int m_iButtons;
	int m_iBPP;

public:
	CLgLCDDevice(DWORD type, SIZE size, int buttons, int BPP) : CLCDDevice(type) {
		m_size = size;
		m_iButtons = buttons;
		m_iBPP = BPP;
	}
	
	// Returns the display name
	tstring GetDisplayName() {
		return m_iBPP == 1? _T("G15") : _T("G19");
	}

	// Returns the display size
	SIZE GetDisplaySize() {
		return m_size;
	}
	// Returns the number of buttons for the display
	int GetButtonCount() {
		return m_iButtons;
	}

	// Returns the number of available colors
	int GetColorCount() {
		return m_iBPP;
	}
};

#endif