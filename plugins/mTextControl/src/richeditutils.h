#ifndef __richeditutils_h__
#define __richeditutils_h__

extern IRichEditOleCallback *reOleCallback;

void LoadRichEdit();
void UnloadRichEdit();

void InitRichEdit(ITextServices *ts);
HWND CreateProxyWindow(ITextServices *ts);

#endif // __richeditutils_h__
