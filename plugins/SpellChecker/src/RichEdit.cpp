#include "stdafx.h"

#define DEFINE_GUIDXXX(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
        const GUID CDECL name \
                = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }

DEFINE_GUIDXXX(IID_ITextDocument,0x8CC497C0,0xA1DF,0x11CE,0x80,0x98,0x00,0xAA,0x00,0x47,0xBE,0x5D);

RichEdit::RichEdit(HWND hwnd) :
	m_hwnd(nullptr),
	m_ole(nullptr),
	m_textDocument(nullptr),
	m_stopped(0),
	m_undoEnabled(TRUE)
{
	SetHWND(hwnd);
}

RichEdit::~RichEdit()
{
	SetHWND(nullptr);
}

bool RichEdit::IsValid() const
{
	return m_ole != nullptr;
}

HWND RichEdit::GetHWND() const
{
	return m_hwnd;
}

void RichEdit::SetHWND(HWND hwnd)
{
	if (m_textDocument != nullptr) {
		m_textDocument->Release();
		m_textDocument = nullptr;
	}
	if (m_ole != nullptr) {
		m_ole->Release();
		m_ole = nullptr;
	}

	m_hwnd = hwnd;

	if (hwnd == nullptr)
		return;

	SendMessage(EM_GETOLEINTERFACE, 0, (LPARAM)&m_ole);
	if (m_ole == nullptr)
		return;

	if (m_ole->QueryInterface(IID_ITextDocument, (void**)&m_textDocument) != S_OK)
		m_textDocument = nullptr;
}

LRESULT RichEdit::SendMessage(UINT Msg, WPARAM wParam, LPARAM lParam) const
{
	return ::SendMessage(m_hwnd, Msg, wParam, lParam);
}

bool RichEdit::IsReadOnly() const
{
	return (GetWindowLongPtr(m_hwnd, GWL_STYLE) & ES_READONLY) == ES_READONLY;
}

void RichEdit::SuspendUndo()
{
	if (m_textDocument != nullptr) {
		m_textDocument->Undo(tomSuspend, nullptr);
		m_undoEnabled = FALSE;
	}
}

void RichEdit::ResumeUndo()
{
	if (m_textDocument != nullptr) {
		m_textDocument->Undo(tomResume, nullptr);
		m_undoEnabled = TRUE;
	}
}

void RichEdit::Stop()
{
	m_stopped++;
	if (m_stopped != 1)
		return;

	SuspendUndo();

	SendMessage(WM_SETREDRAW, FALSE, 0);

	SendMessage(EM_GETSCROLLPOS, 0, (LPARAM)&m_old_scroll_pos);
	SendMessage(EM_EXGETSEL, 0, (LPARAM)&m_old_sel);
	GetCaretPos(&m_caretPos);

	m_old_mask = SendMessage(EM_GETEVENTMASK, 0, 0);
	SendMessage(EM_SETEVENTMASK, 0, m_old_mask & ~ENM_CHANGE);

	m_inverse = (m_old_sel.cpMin >= LOWORD(SendMessage(EM_CHARFROMPOS, 0, (LPARAM)&m_caretPos)));
}

void RichEdit::Start()
{
	m_stopped--;

	if (m_stopped < 0) {
		m_stopped = 0;
		return;
	}
	if (m_stopped > 0)
		return;

	if (m_inverse) {
		LONG tmp = m_old_sel.cpMin;
		m_old_sel.cpMin = m_old_sel.cpMax;
		m_old_sel.cpMax = tmp;
	}

	SendMessage(EM_SETEVENTMASK, 0, m_old_mask);
	SendMessage(EM_EXSETSEL, 0, (LPARAM)&m_old_sel);
	SendMessage(EM_SETSCROLLPOS, 0, (LPARAM)&m_old_scroll_pos);

	SendMessage(WM_SETREDRAW, TRUE, 0);
	InvalidateRect(m_hwnd, nullptr, FALSE);

	ResumeUndo();
}

BOOL RichEdit::IsStopped()
{
	return m_stopped > 0;
}

int RichEdit::GetCharFromPos(const POINT &pt)
{
	return LOWORD(SendMessage(EM_CHARFROMPOS, 0, (LPARAM)&pt));
}

int RichEdit::GetLineCount() const
{
	return SendMessage(EM_GETLINECOUNT, 0, 0);
}

void RichEdit::GetLine(int line, wchar_t *text, size_t text_len) const
{
	*((uint16_t*)text) = uint16_t(text_len - 1);
	unsigned size = (unsigned)SendMessage(EM_GETLINE, (WPARAM)line, (LPARAM)text);

	// Sometimes it likes to return size = lineLen+1, adding an \n at the end, so we remove it here
	// to make both implementations return same size
	int lineLen = GetLineLength(line);
	size = (unsigned)max(0, min((int)text_len - 1, min((int)size, lineLen)));
	text[size] = '\0';
}

int RichEdit::GetLineLength(int line) const
{
	return SendMessage(EM_LINELENGTH, GetFirstCharOfLine(line), 0);
}

int RichEdit::GetFirstCharOfLine(int line) const
{
	return SendMessage(EM_LINEINDEX, (WPARAM)line, 0);
}

int RichEdit::GetLineFromChar(int charPos) const
{
	return SendMessage(EM_LINEFROMCHAR, charPos, 0);
}

CHARRANGE RichEdit::GetSel() const
{
	CHARRANGE sel;
	SendMessage(EM_EXGETSEL, 0, (LPARAM)&sel);
	return sel;
}

void RichEdit::SetSel(int start, int end)
{
	CHARRANGE sel = { start, end };
	SetSel(sel);
}

void RichEdit::SetSel(const CHARRANGE &sel)
{
	SendMessage(EM_EXSETSEL, 0, (LPARAM)&sel);
}

int RichEdit::GetTextLength() const
{
	return GetWindowTextLength(m_hwnd);
}

wchar_t* RichEdit::GetText(int start, int end) const
{
	if (end <= start)
		end = GetTextLength();

	if (m_textDocument != nullptr) {
		ITextRange *range;
		if (m_textDocument->Range(start, end, &range) != S_OK)
			return mir_wstrdup(L"");

		BSTR text = nullptr;
		if (FAILED(range->GetText(&text))) {
			if (text)
				::SysFreeString(text);
			range->Release();
			return mir_wstrdup(L"");
		}

		wchar_t *res = mir_wstrdup(text);
		range->Release();
		::SysFreeString(text);
		return res;
	}

	int len = (GetTextLength() + 1);
	wchar_t *tmp = (wchar_t *)mir_alloc(len * sizeof(wchar_t));
	GetWindowText(m_hwnd, tmp, len);
	tmp[len] = 0;

	wchar_t *ret = (wchar_t *)mir_alloc((end - start + 1) * sizeof(wchar_t));
	memmove(ret, &tmp[start], (end - start) * sizeof(wchar_t));
	ret[end - start] = 0;

	mir_free(tmp);
	return ret;
}

void RichEdit::ReplaceSel(const wchar_t *new_text)
{
	if (m_stopped) {
		CHARRANGE sel = GetSel();

		ResumeUndo();

		SendMessage(EM_REPLACESEL, m_undoEnabled, (LPARAM)new_text);

		SuspendUndo();

		FixSel(&m_old_sel, sel, (int)mir_wstrlen(new_text));

		SendMessage(WM_SETREDRAW, FALSE, 0);
		SendMessage(EM_SETEVENTMASK, 0, m_old_mask & ~ENM_CHANGE);
	}
	else SendMessage(EM_REPLACESEL, m_undoEnabled, (LPARAM)new_text);
}

int RichEdit::Replace(int start, int end, const wchar_t *new_text)
{
	CHARRANGE sel = GetSel();
	CHARRANGE replace_sel = { start, end };
	SetSel(replace_sel);

	ReplaceSel(new_text);

	int dif = FixSel(&sel, replace_sel, (int)mir_wstrlen(new_text));
	SetSel(sel);
	return dif;
}

int RichEdit::Insert(int pos, const wchar_t *text)
{
	CHARRANGE sel = GetSel();
	CHARRANGE replace_sel = { pos, pos };
	SetSel(replace_sel);

	ReplaceSel(text);

	int dif = FixSel(&sel, replace_sel, (int)mir_wstrlen(text));
	SetSel(sel);
	return dif;
}

int RichEdit::Delete(int start, int end)
{
	CHARRANGE sel = GetSel();
	CHARRANGE replace_sel = { start, end };
	SetSel(replace_sel);

	ReplaceSel(L"");

	int dif = FixSel(&sel, replace_sel, 0);
	SetSel(sel);
	return dif;
}

int RichEdit::FixSel(CHARRANGE *to_fix, CHARRANGE sel_changed, int new_len)
{
	int dif = new_len - (sel_changed.cpMax - sel_changed.cpMin);

	if (to_fix->cpMax <= sel_changed.cpMin)
		return dif;

	int newMax = sel_changed.cpMax + dif;

	if      (to_fix->cpMin >= sel_changed.cpMax) to_fix->cpMin += dif;
	else if (to_fix->cpMin >= newMax)            to_fix->cpMin = newMax;

	if      (to_fix->cpMax >= sel_changed.cpMax) to_fix->cpMax += dif;
	else if (to_fix->cpMax >= newMax)            to_fix->cpMax = newMax;

	return dif;
}
