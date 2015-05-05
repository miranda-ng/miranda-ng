#include "stdafx.h"

CSteamPasswordEditor::CSteamPasswordEditor(CSteamProto *proto) :
	CSuper(proto, IDD_PASSWORD_EDITOR, false), m_ok(this, IDOK),
	m_password(this, IDC_PASSWORD), m_savePermanently(this, IDC_SAVEPERMANENTLY)
{
	m_ok.OnClick = Callback(this, &CSteamPasswordEditor::OnOk);
}

void CSteamPasswordEditor::OnInitDialog()
{
	char iconName[100];
	mir_snprintf(iconName, SIZEOF(iconName), "%s_%s", MODULE, "main");
	SendMessage(m_hwnd, WM_SETICON, ICON_BIG, (LPARAM)Skin_GetIcon(iconName, 16));
	SendMessage(m_hwnd, WM_SETICON, ICON_SMALL, (LPARAM)Skin_GetIcon(iconName, 32));

	SendMessage(m_password.GetHwnd(), EM_LIMITTEXT, 64, 0);

	Utils_RestoreWindowPosition(m_hwnd, NULL, m_proto->m_szModuleName, "PasswordWindow");
}

void CSteamPasswordEditor::OnOk(CCtrlButton*)
{
	if (m_savePermanently.Enabled())
		m_proto->setTString("Password", m_password.GetText());
	if (m_proto->password != NULL)
		mir_free(m_proto->password);
	m_proto->password = m_password.GetText();

	EndDialog(m_hwnd, 1);
}

void CSteamPasswordEditor::OnClose()
{
	Utils_SaveWindowPosition(m_hwnd, NULL, m_proto->m_szModuleName, "PasswordWindow");
}

/////////////////////////////////////////////////////////////////////////////////

CSteamGuardDialog::CSteamGuardDialog(CSteamProto *proto, char *domain) :
	CSuper(proto, IDD_GUARD, false), m_ok(this, IDOK),
	m_text(this, IDC_TEXT), m_link(this, IDC_GETDOMAIN, domain)
{
	mir_strcpy(m_domain, domain);
	m_ok.OnClick = Callback(this, &CSteamGuardDialog::OnOk);
}

void CSteamGuardDialog::OnInitDialog()
{
	char iconName[100];
	mir_snprintf(iconName, SIZEOF(iconName), "%s_%s", MODULE, "main");
	SendMessage(m_hwnd, WM_SETICON, ICON_BIG, (LPARAM)Skin_GetIcon(iconName, 16));
	SendMessage(m_hwnd, WM_SETICON, ICON_SMALL, (LPARAM)Skin_GetIcon(iconName, 32));

	SendMessage(m_text.GetHwnd(), EM_LIMITTEXT, 5, 0);

	Utils_RestoreWindowPosition(m_hwnd, NULL, m_proto->m_szModuleName, "GuardWindow");
}

void CSteamGuardDialog::OnOk(CCtrlButton*)
{
	mir_strncpy(m_guardCode, ptrA(m_text.GetTextA()), SIZEOF(m_guardCode) + 1);
	EndDialog(m_hwnd, 1);
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

CSteamCaptchaDialog::CSteamCaptchaDialog(CSteamProto *proto, BYTE *captchaImage, int captchaImageSize) :
	CSuper(proto, IDD_CAPTCHA, false),
	m_ok(this, IDOK), m_text(this, IDC_TEXT),
	m_captchaImage(NULL)
{
	m_captchaImageSize = captchaImageSize;
	m_captchaImage = (BYTE*)mir_alloc(captchaImageSize);
	memcpy(m_captchaImage, captchaImage, captchaImageSize);
	m_ok.OnClick = Callback(this, &CSteamCaptchaDialog::OnOk);
}

CSteamCaptchaDialog::~CSteamCaptchaDialog()
{
	if(m_captchaImage)
		mir_free(m_captchaImage);
}

void CSteamCaptchaDialog::OnInitDialog()
{
	char iconName[100];
	mir_snprintf(iconName, SIZEOF(iconName), "%s_%s", MODULE, "main");
	SendMessage(m_hwnd, WM_SETICON, ICON_BIG, (LPARAM)Skin_GetIcon(iconName, 16));
	SendMessage(m_hwnd, WM_SETICON, ICON_SMALL, (LPARAM)Skin_GetIcon(iconName, 32));

	SendMessage(m_text.GetHwnd(), EM_LIMITTEXT, 6, 0);

	Utils_RestoreWindowPosition(m_hwnd, NULL, m_proto->m_szModuleName, "CaptchaWindow");
}

void CSteamCaptchaDialog::OnOk(CCtrlButton*)
{
	mir_strncpy(m_captchaText, ptrA(m_text.GetTextA()), SIZEOF(m_captchaText) + 1);
	EndDialog(m_hwnd, 1);
}

void CSteamCaptchaDialog::OnClose()
{
	Utils_SaveWindowPosition(m_hwnd, NULL, m_proto->m_szModuleName, "CaptchaWindow");
}

INT_PTR CSteamCaptchaDialog::DlgProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_PAINT)
	{
		FI_INTERFACE *fei = 0;

		INT_PTR result = CALLSERVICE_NOTFOUND;
		if (ServiceExists(MS_IMG_GETINTERFACE))
			result = CallService(MS_IMG_GETINTERFACE, FI_IF_VERSION, (LPARAM)&fei);

		if (fei == NULL || result != S_OK) {
			MessageBox(0, TranslateT("Fatal error, image services not found. Avatar services will be disabled."), TranslateT("Avatar Service"), MB_OK);
			return 0;
		}

		FIMEMORY *stream = fei->FI_OpenMemory(m_captchaImage, m_captchaImageSize);
		FREE_IMAGE_FORMAT fif = fei->FI_GetFileTypeFromMemory(stream, 0);
		FIBITMAP *bitmap = fei->FI_LoadFromMemory(fif, stream, 0);
		fei->FI_CloseMemory(stream);

		PAINTSTRUCT ps;
		HDC hDC = BeginPaint(m_hwnd, &ps);

		//SetStretchBltMode(hDC, COLORONCOLOR);
		StretchDIBits(
			hDC,
			11, 11,
			fei->FI_GetWidth(bitmap) - 13,
			fei->FI_GetHeight(bitmap),
			0, 0,
			fei->FI_GetWidth(bitmap),
			fei->FI_GetHeight(bitmap),
			fei->FI_GetBits(bitmap),
			fei->FI_GetInfo(bitmap),
			DIB_RGB_COLORS, SRCCOPY);

		fei->FI_Unload(bitmap);
		//fei->FI_DeInitialise();

		EndPaint(m_hwnd, &ps);

		return FALSE;
	}
	CSuper::DlgProc(msg, wParam, lParam);
	return FALSE;
}

const char* CSteamCaptchaDialog::GetCaptchaText()
{
	return m_captchaText;
}

/////////////////////////////////////////////////////////////////////////////////

CSteamOptionsMain::CSteamOptionsMain(CSteamProto *proto, int idDialog, HWND hwndParent)
	: CSuper(proto, idDialog, false),
	m_username(this, IDC_USERNAME), m_password(this, IDC_PASSWORD),
	m_group(this, IDC_GROUP), m_biggerAvatars(this, IDC_BIGGER_AVATARS)
{
	SetParent(hwndParent);

	CreateLink(m_username, "Username", _T(""));
	CreateLink(m_password, "Password", _T(""));
	CreateLink(m_group, "DefaultGroup", _T("Steam"));
	CreateLink(m_biggerAvatars, "UseBigAvatars", FALSE);
}

void CSteamOptionsMain::OnInitDialog()
{
	CSuper::OnInitDialog();

	SendMessage(m_username.GetHwnd(), EM_LIMITTEXT, 64, 0);
	SendMessage(m_password.GetHwnd(), EM_LIMITTEXT, 64, 0);
	SendMessage(m_group.GetHwnd(), EM_LIMITTEXT, 64, 0);
}

void CSteamOptionsMain::OnApply()
{
	TCHAR *group = m_group.GetText();
	if (mir_tstrlen(group) > 0 && Clist_GroupExists(group))
		Clist_CreateGroup(0, group);

	if (m_proto->IsOnline())
	{
		// may be we should show message box with warning?
		m_proto->SetStatus(ID_STATUS_OFFLINE);
	}
	if (m_username.IsChanged())
	{
		m_proto->delSetting("SteamID");
		m_proto->delSetting("TokenSecret");
	}
	if (m_password.IsChanged())
	{
		m_proto->delSetting("TokenSecret");
	}
	mir_free(group);
}

/////////////////////////////////////////////////////////////////////////////////

CSteamOptionsBlockList::CSteamOptionsBlockList(CSteamProto *proto)
	: CSuper(proto, IDD_OPT_BLOCK_LIST, false),
	m_list(this, IDC_LIST),
	m_contacts(this, IDC_CONTACTS),
	m_add(this, IDC_BLOCK)
{
	m_add.OnClick = Callback(this, &CSteamOptionsBlockList::OnBlock);
}

void CSteamOptionsBlockList::OnInitDialog()
{
	m_list.SetExtendedListViewStyle(LVS_EX_SUBITEMIMAGES | LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP);

	m_list.AddColumn(0, TranslateT("Name"), 220);
	m_list.AddColumn(1, _T(""), 32 - GetSystemMetrics(SM_CXVSCROLL));
}

void CSteamOptionsBlockList::OnBlock(CCtrlButton*)
{
}