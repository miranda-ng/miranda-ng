#include "stdafx.h"

CSteamPasswordEditor::CSteamPasswordEditor(CSteamProto *proto)
	: CSteamDlgBase(proto, IDD_PASSWORD_EDITOR, false), m_ok(this, IDOK),
	m_password(this, IDC_PASSWORD), m_savePermanently(this, IDC_SAVEPERMANENTLY)
{
	m_ok.OnClick = Callback(this, &CSteamPasswordEditor::OnOk);
}

void CSteamPasswordEditor::OnInitDialog()
{
	char iconName[100];
	mir_snprintf(iconName, "%s_%s", MODULE, "main");
	Window_SetIcon_IcoLib(m_hwnd, IcoLib_GetIconHandle(iconName));

	SendMessage(m_password.GetHwnd(), EM_LIMITTEXT, 64, 0);

	Utils_RestoreWindowPosition(m_hwnd, NULL, m_proto->m_szModuleName, "PasswordWindow");
}

void CSteamPasswordEditor::OnOk(CCtrlButton*)
{
	m_proto->m_password = m_password.GetText();
	if (m_savePermanently.Enabled())
		m_proto->setWString("Password", m_proto->m_password);

	EndDialog(m_hwnd, DIALOG_RESULT_OK);
}

void CSteamPasswordEditor::OnClose()
{
	Utils_SaveWindowPosition(m_hwnd, NULL, m_proto->m_szModuleName, "PasswordWindow");
}

/////////////////////////////////////////////////////////////////////////////////

CSteamGuardDialog::CSteamGuardDialog(CSteamProto *proto, const char *domain)
	: CSteamDlgBase(proto, IDD_GUARD, false),
	m_ok(this, IDOK),
	m_text(this, IDC_TEXT),
	m_link(this, IDC_GETDOMAIN, domain)
{
	memset(m_guardCode, 0, sizeof(m_guardCode));
	mir_strcpy(m_domain, domain);
	m_ok.OnClick = Callback(this, &CSteamGuardDialog::OnOk);
}

void CSteamGuardDialog::OnInitDialog()
{
	char iconName[100];
	mir_snprintf(iconName, "%s_%s", MODULE, "main");
	Window_SetIcon_IcoLib(m_hwnd, IcoLib_GetIconHandle(iconName));

	SendMessage(m_text.GetHwnd(), EM_LIMITTEXT, 5, 0);

	Utils_RestoreWindowPosition(m_hwnd, NULL, m_proto->m_szModuleName, "GuardWindow");
}

void CSteamGuardDialog::OnOk(CCtrlButton*)
{
	mir_strncpy(m_guardCode, ptrA(m_text.GetTextA()), _countof(m_guardCode));
	EndDialog(m_hwnd, DIALOG_RESULT_OK);
}

void CSteamGuardDialog::OnClose()
{
	Utils_SaveWindowPosition(m_hwnd, NULL, m_proto->m_szModuleName, "GuardWindow");
}

const char* CSteamGuardDialog::GetGuardCode()
{
	return m_guardCode;
}

/////////////////////////////////////////////////////////////////////////////////

CSteamTwoFactorDialog::CSteamTwoFactorDialog(CSteamProto *proto)
: CSteamDlgBase(proto, IDD_TWOFACTOR, false),
m_ok(this, IDOK),
m_text(this, IDC_TEXT)
{
	memset(m_twoFactorCode, 0, sizeof(m_twoFactorCode));
	m_ok.OnClick = Callback(this, &CSteamTwoFactorDialog::OnOk);
}

void CSteamTwoFactorDialog::OnInitDialog()
{
	char iconName[100];
	mir_snprintf(iconName, "%s_%s", MODULE, "main");
	Window_SetIcon_IcoLib(m_hwnd, IcoLib_GetIconHandle(iconName));

	SendMessage(m_text.GetHwnd(), EM_LIMITTEXT, 5, 0);

	Utils_RestoreWindowPosition(m_hwnd, NULL, m_proto->m_szModuleName, "TwoFactorWindow");
}

void CSteamTwoFactorDialog::OnOk(CCtrlButton*)
{
	mir_strncpy(m_twoFactorCode, ptrA(m_text.GetTextA()), _countof(m_twoFactorCode));
	EndDialog(m_hwnd, DIALOG_RESULT_OK);
}

void CSteamTwoFactorDialog::OnClose()
{
	Utils_SaveWindowPosition(m_hwnd, NULL, m_proto->m_szModuleName, "TwoFactorWindow");
}

const char* CSteamTwoFactorDialog::GetTwoFactorCode()
{
	return m_twoFactorCode;
}

/////////////////////////////////////////////////////////////////////////////////

CSteamCaptchaDialog::CSteamCaptchaDialog(CSteamProto *proto, const uint8_t *captchaImage, int captchaImageSize)
	: CSteamDlgBase(proto, IDD_CAPTCHA, false),
	m_ok(this, IDOK), m_text(this, IDC_TEXT),
	m_captchaImage(nullptr)
{
	memset(m_captchaText, 0, sizeof(m_captchaText));
	m_captchaImageSize = captchaImageSize;
	m_captchaImage = (uint8_t*)mir_alloc(captchaImageSize);
	memcpy(m_captchaImage, captchaImage, captchaImageSize);
	m_ok.OnClick = Callback(this, &CSteamCaptchaDialog::OnOk);
}

CSteamCaptchaDialog::~CSteamCaptchaDialog()
{
	if (m_captchaImage)
		mir_free(m_captchaImage);
}

void CSteamCaptchaDialog::OnInitDialog()
{
	char iconName[100];
	mir_snprintf(iconName, "%s_%s", MODULE, "main");
	Window_SetIcon_IcoLib(m_hwnd, IcoLib_GetIconHandle(iconName));

	SendMessage(m_text.GetHwnd(), EM_LIMITTEXT, 6, 0);

	Utils_RestoreWindowPosition(m_hwnd, NULL, m_proto->m_szModuleName, "CaptchaWindow");
}

void CSteamCaptchaDialog::OnOk(CCtrlButton*)
{
	mir_strncpy(m_captchaText, ptrA(m_text.GetTextA()), _countof(m_captchaText));
	EndDialog(m_hwnd, DIALOG_RESULT_OK);
}

void CSteamCaptchaDialog::OnClose()
{
	Utils_SaveWindowPosition(m_hwnd, NULL, m_proto->m_szModuleName, "CaptchaWindow");
}

INT_PTR CSteamCaptchaDialog::DlgProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_PAINT) {
		FIMEMORY *stream = FreeImage_OpenMemory(m_captchaImage, m_captchaImageSize);
		FREE_IMAGE_FORMAT fif = FreeImage_GetFileTypeFromMemory(stream, 0);
		FIBITMAP *bitmap = FreeImage_LoadFromMemory(fif, stream, 0);
		FreeImage_CloseMemory(stream);

		PAINTSTRUCT ps;
		HDC hDC = BeginPaint(m_hwnd, &ps);

		StretchDIBits(
			hDC,
			11, 11,
			FreeImage_GetWidth(bitmap) - 13,
			FreeImage_GetHeight(bitmap),
			0, 0,
			FreeImage_GetWidth(bitmap),
			FreeImage_GetHeight(bitmap),
			FreeImage_GetBits(bitmap),
			FreeImage_GetInfo(bitmap),
			DIB_RGB_COLORS, SRCCOPY);

		FreeImage_Unload(bitmap);

		EndPaint(m_hwnd, &ps);

		return FALSE;
	}
	CSteamDlgBase::DlgProc(msg, wParam, lParam);
	return FALSE;
}

const char* CSteamCaptchaDialog::GetCaptchaText()
{
	return m_captchaText;
}