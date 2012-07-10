#ifndef __FONT_STATE_H__
# define __FONT_STATE_H__

#include "Field.h"


class FontState
{
public:
	FontState(HFONT hFont, COLORREF aColor);
	~FontState();

	HFONT getHFONT() const;
	void setHFONT(HFONT hFont);
	HFONT createHFONT() const; /// Return a copy of the internal HFONT. The caller must free it

	const TCHAR * getFace() const;
	void setFace(const TCHAR * face);

	int getSize() const;
	void setSize(int size);

	COLORREF getColor() const;
	void setColor(COLORREF color);

	bool isItalic() const;
	void setItalic(bool italic);

	bool isBold() const;
	void setBold(bool bold);

	bool isUnderline() const;
	void setUnderline(bool underline);

	bool isStrikeOut() const;
	void setStrikeOut(bool strikeout);

private:
	COLORREF color;
	HFONT hFont;
	bool externalFont;
	std::tstring face;
	int size;
	bool italic;
	bool bold;
	bool underline;
	bool strikeout;

	void rebuildHFONT();
	void buildHFONT();
	void releaseHFONT();
	void buildAttribs();
};



#endif // __FONT_STATE_H__
