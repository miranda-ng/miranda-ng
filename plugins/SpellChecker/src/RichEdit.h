#ifndef __RICHEDIT_H__
#define __RICHEDIT_H__

class RichEdit
{
	HWND m_hwnd;
	IRichEditOle *m_ole;
	ITextDocument *m_textDocument;

	int m_stopped;
	BOOL m_undoEnabled;
	POINT m_old_scroll_pos;
	CHARRANGE m_old_sel;
	POINT m_caretPos;
	uint32_t m_old_mask;
	BOOL m_inverse;

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
	void GetLine(int line, wchar_t *text, size_t text_len) const;
	int GetLineLength(int line) const;
	int GetFirstCharOfLine(int line) const;
	int GetLineFromChar(int charPos) const;

	CHARRANGE GetSel() const;
	void SetSel(int start, int end);
	void SetSel(const CHARRANGE &sel);

	wchar_t* GetText(int start, int end) const;
	int GetTextLength() const;

	void ReplaceSel(const wchar_t *new_text);
	int Replace(int start, int end, const wchar_t *new_text);
	int Insert(int pos, const wchar_t *text);
	int Delete(int start, int end);

private:
	void SetHWND(HWND hwnd);

	int FixSel(CHARRANGE *to_fix, CHARRANGE sel_changed, int new_len);
};

#endif // __RICHEDIT_H__
