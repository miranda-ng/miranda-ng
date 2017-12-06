#ifndef _CLCDLABEL_H_
#define _CLCDLABEL_H_

#include "CLCDTextObject.h"

class CLCDLabel : public CLCDTextObject
{
public:
	// constructor
	CLCDLabel();
	// destructor
	~CLCDLabel();

	// initializes the label
	bool Initialize();
	// deinitializes the label
	bool Shutdown();

	// update the label
	bool Update();
	// draw the label
	bool Draw(CLCDGfx *pGfx);

	// sets the label's text
	void SetText(tstring strText);

	// sets the cutoff mode
	void SetCutOff(bool bEnable);

	// sets the wordwrap mode
	void SetWordWrap(bool bEnable);

private:
	// updates the cutoff index
	void UpdateCutOffIndex();
	// called when the labels size has changed
	void OnSizeChanged(SIZE OldSize);
	// called when the labels font has changed
	void OnFontChanged();

	bool m_bWordWrap;

	bool m_bCutOff;
	int m_iCutOffIndex;
	tstring m_strText;
	tstring m_strCutOff;
	int	m_iLineCount;

	vector<tstring> m_vLines;
};

#endif