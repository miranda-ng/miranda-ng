#ifndef __MIRANDA_FONT_H__
# define __MIRANDA_FONT_H__

#include "commons.h"


class MirandaFont
{
public:
	MirandaFont(Field *field, const char *description);
	~MirandaFont();

	void registerFont(FontState *font);
	void reloadFont();

private:
	FontIDT fid;
	HFONT hFont;
	Field *field;

	void releaseFont();

	static int staticReloadFont(void *obj, WPARAM wParam, LPARAM lParam);
};



#endif // __MIRANDA_FONT_H__
