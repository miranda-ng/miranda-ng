#ifndef __RICHEDIT_H__
#define __RICHEDIT_H__

class RichEdit
{
	HWND hwnd;
	IRichEditOle *ole;
	ITextDocument *textDocument;

	int stopped;
	BOOL undoEnabled;
	POINT old_scroll_pos;
	CHARRANGE old_sel;
	POINT caretPos;
	DWORD old_mask;
	BOOL inverse;

public:
	RichEdit(HWND hwnd);
	~RichEdit();

	bool IsValid() const;
	HWND GetHWND() const;

	LRESULT SendMessage(UINT Msg, WPARAM wParam, LPARAM lParam) const;

	bool IsReadOnly() const;

	void SuspendUndo();
	void ResumeUndo();
	void Stop();
	void Start();
	BOOL IsStopped();

	int GetCharFromPos(const POINT &pt);

	int GetLineCount() const;
	void GetLine(int line, TCHAR *text, size_t text_len) const;
	int GetLineLength(int line) const;
	int GetFirstCharOfLine(int line) const;
	int GetLineFromChar(int charPos) const;

	CHARRANGE GetSel() const;
	void SetSel(int start, int end);
	void SetSel(const CHARRANGE &sel);

	TCHAR *GetText(int start, int end) const;
	int GetTextLength() const;

	void ReplaceSel(const TCHAR *new_text);
	int Replace(int start, int end, const TCHAR *new_text);
	int Insert(int pos, const TCHAR *text);
	int Delete(int start, int end);

private:
	void SetHWND(HWND hwnd);

	int FixSel(CHARRANGE *to_fix, CHARRANGE sel_changed, int new_len);
};





#endif // __RICHEDIT_H__
