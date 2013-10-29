#ifndef _CLCDTextObject_H_
#define _CLCDTextObject_H_

#include "CLCDObject.h"

class CLCDTextObject : public CLCDObject
{
public:
	// constructor
	CLCDTextObject();
	// destructor
	~CLCDTextObject();
	
	// initializes the textobject
	bool Initialize();
	// deinitializes the textobject
	bool Shutdown();

	// sets the textobject's font
	bool SetFont(LOGFONT& lf);
	// sets the textobject's font's facename
	void SetFontFaceName(tstring strFontName);
	// sets the textobject's font's pointsize
    void SetFontPointSize(int nPointSize);
	// sets the textobject's font's weight
    void SetFontWeight(int nWeight);
	// sets the textobject's font's italic flag
	void SetFontItalic(bool flag);

	// sets the textobject's alignment
	void SetAlignment(int iAlignment);
	// sets the textobject's wordwrap mode
	void SetWordWrap(bool bWrap);

	// draws the textobject
	bool Draw(CLCDGfx *pGfx);
	// updates the textobject
	bool Update();

protected:
	virtual void OnFontChanged();

protected:
	bool			m_bWordWrap;

	HFONT			m_hFont;
	int				m_iFontHeight;
	DRAWTEXTPARAMS	m_dtp;
    UINT			m_iTextFormat;
	UINT			m_iTextAlignment;
};

#endif