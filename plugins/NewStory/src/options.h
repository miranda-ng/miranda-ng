#ifndef __options_h__
#define __options_h__

struct Options
{
	int dummy;
};

extern Options options;
extern int OptShowPage;

void LoadOptions();
void InitOptions();

#endif // __options_h__