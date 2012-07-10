/* 
Copyright (C) 2008 Ricardo Pescuma Domenecci

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt. If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA. 
*/

#include "commons.h"


// Prototypes ///////////////////////////////////////////////////////////////////////////


PLUGININFOEX pluginInfo={
	sizeof(PLUGININFOEX),
#ifdef UNICODE
	"Skins (Unicode)",
#else
	"Skins",
#endif
	PLUGIN_MAKE_VERSION(0,0,0,5),
	"Skins",
	"Ricardo Pescuma Domenecci",
	"",
	"© 2008-2010 Ricardo Pescuma Domenecci",
	"http://pescuma.org/miranda/skins",
	UNICODE_AWARE,
	0,		//doesn't replace anything built-in
#ifdef UNICODE
	{ 0xde546127, 0x2cdd, 0x48ca, { 0x8a, 0xe5, 0x36, 0x25, 0xe7, 0x24, 0xf2, 0xda } }
#else
	{ 0x2f630a2a, 0xd8f9, 0x4f81, { 0x8a, 0x4e, 0xce, 0x9a, 0x73, 0x5d, 0xf2, 0xe9 } }
#endif
};


HINSTANCE hInst;
PLUGINLINK *pluginLink;

std::vector<HANDLE> hHooks;
std::vector<HANDLE> hServices;
HANDLE hNetlibUser = 0;

HANDLE hSkinsFolder = NULL;
TCHAR skinsFolder[1024];

std::vector<MirandaSkinnedDialog *> dlgs;

LIST_INTERFACE li;
FI_INTERFACE *fei = NULL;
struct MM_INTERFACE mmi;
struct UTF8_INTERFACE utfi;


int ModulesLoaded(WPARAM wParam, LPARAM lParam);
int PreShutdown(WPARAM wParam, LPARAM lParam);

static int Service_GetInterface(WPARAM wParam, LPARAM lParam);


// Functions ////////////////////////////////////////////////////////////////////////////


extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) 
{
	hInst = hinstDLL;
	return TRUE;
}


extern "C" __declspec(dllexport) PLUGININFO* MirandaPluginInfo(DWORD mirandaVersion) 
{
	pluginInfo.cbSize = sizeof(PLUGININFO);
	return (PLUGININFO*) &pluginInfo;
}


extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	pluginInfo.cbSize = sizeof(PLUGININFOEX);
	return &pluginInfo;
}


static const MUUID interfaces[] = { MIID_SKINS, MIID_LAST };
extern "C" __declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}


extern "C" int __declspec(dllexport) Load(PLUGINLINK *link) 
{
	pluginLink = link;

	CHECK_VERSION("Skins");

	// TODO Assert results here
	mir_getMMI(&mmi);
	mir_getUTFI(&utfi);
	mir_getLI(&li);

	hHooks.push_back( HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded) );
	hHooks.push_back( HookEvent(ME_SYSTEM_PRESHUTDOWN, PreShutdown) );

	NETLIBUSER nl_user = {0};
	nl_user.cbSize = sizeof(nl_user);
	nl_user.szSettingsModule = MODULE_NAME;
	nl_user.flags = NUF_NOOPTIONS;
	nl_user.szDescriptiveName = Translate("Skins");
	hNetlibUser = (HANDLE)CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nl_user);

	TCHAR mirandaFolder[1024];
	GetModuleFileName(GetModuleHandle(NULL), mirandaFolder, MAX_REGS(mirandaFolder));
	TCHAR *p = _tcsrchr(mirandaFolder, _T('\\'));
	if (p != NULL)
		*p = _T('\0');

	// Folders plugin support
	if (ServiceExists(MS_FOLDERS_REGISTER_PATH))
	{
		hSkinsFolder = FoldersRegisterCustomPathT("Skins",  "Skins", _T(MIRANDA_PATH) _T("\\Skins"));

		FoldersGetCustomPathT(hSkinsFolder, skinsFolder, MAX_REGS(skinsFolder), _T("."));
	}
	else
	{
		mir_sntprintf(skinsFolder, MAX_REGS(skinsFolder), _T("%s\\Skins"), mirandaFolder);
	}

	InitOptions();

	hServices.push_back( CreateServiceFunction(MS_SKINS_GETINTERFACE, Service_GetInterface) );

	return 0;
}


extern "C" int __declspec(dllexport) Unload(void) 
{
	return 0;
}


// Called when all the modules are loaded
int ModulesLoaded(WPARAM wParam, LPARAM lParam) 
{
	// add our modules to the KnownModules list
	CallService("DBEditorpp/RegisterSingleModule", (WPARAM) MODULE_NAME, 0);

	// updater plugin support
	if(ServiceExists(MS_UPDATE_REGISTER))
	{
		Update upd = {0};
		char szCurrentVersion[30];

		upd.cbSize = sizeof(upd);
		upd.szComponentName = pluginInfo.shortName;

		upd.szUpdateURL = UPDATER_AUTOREGISTER;

		upd.szBetaVersionURL = "http://pescuma.googlecode.com/svn/trunk/Miranda/Plugins/skins/Docs/skins_version.txt";
		upd.szBetaChangelogURL = "http://pescuma.googlecode.com/svn/trunk/Miranda/Plugins/skins/Docs/skins_changelog.txt";
		upd.pbBetaVersionPrefix = (BYTE *)"Skins ";
		upd.cpbBetaVersionPrefix = strlen((char *)upd.pbBetaVersionPrefix);
#ifdef UNICODE
		upd.szBetaUpdateURL = "http://pescuma.googlecode.com/files/skinsW.%VERSION%.zip";
#else
		upd.szBetaUpdateURL = "http://pescuma.googlecode.com/files/skins.%VERSION%.zip";
#endif

		upd.pbVersion = (BYTE *)CreateVersionStringPlugin((PLUGININFO*) &pluginInfo, szCurrentVersion);
		upd.cpbVersion = strlen((char *)upd.pbVersion);

		CallService(MS_UPDATE_REGISTER, 0, (LPARAM)&upd);
	}

	return 0;
}


int PreShutdown(WPARAM wParam, LPARAM lParam)
{
	unsigned int i;

	for(i = 0; i < hServices.size(); i++)
		DestroyServiceFunction(hServices[i]);
	hServices.clear();

	for(i = 0; i < hHooks.size(); i++)
		UnhookEvent(hHooks[i]);
	hHooks.clear();

	DeInitOptions();

	return 0;
}


BOOL FileExists(const char *filename)
{
	DWORD attrib = GetFileAttributesA(filename);
	if (attrib == 0xFFFFFFFF || (attrib & FILE_ATTRIBUTE_DIRECTORY))
		return FALSE;
	return TRUE;
}

#ifdef UNICODE
BOOL FileExists(const WCHAR *filename)
{
	DWORD attrib = GetFileAttributesW(filename);
	if (attrib == 0xFFFFFFFF || (attrib & FILE_ATTRIBUTE_DIRECTORY))
		return FALSE;
	return TRUE;
}
#endif


BOOL DirExists(const char *filename)
{
	DWORD attrib = GetFileAttributesA(filename);
	if (attrib == 0xFFFFFFFF || !(attrib & FILE_ATTRIBUTE_DIRECTORY))
		return FALSE;
	return TRUE;
}

#ifdef UNICODE
BOOL DirExists(const WCHAR *filename)
{
	DWORD attrib = GetFileAttributesW(filename);
	if (attrib == 0xFFFFFFFF || !(attrib & FILE_ATTRIBUTE_DIRECTORY))
		return FALSE;
	return TRUE;
}
#endif


BOOL CreatePath(const char *path) 
{
	char folder[1024];
	strncpy(folder, path, MAX_REGS(folder));
	folder[MAX_REGS(folder)-1] = '\0';

	char *p = folder;
	if (p[0] && p[1] == ':' && p[2] == '\\') p += 3; // skip drive letter

	SetLastError(ERROR_SUCCESS);
	while(p = strchr(p, '\\')) 
	{
		*p = '\0';
		CreateDirectoryA(folder, 0);
		*p = '\\';
		p++;
	}
	CreateDirectoryA(folder, 0);

	DWORD lerr = GetLastError();
	return (lerr == ERROR_SUCCESS || lerr == ERROR_ALREADY_EXISTS);
}


void log(const char *fmt, ...)
{
 va_list va;
 char text[1024];

 va_start(va, fmt);
 mir_vsnprintf(text, sizeof(text), fmt, va);
 va_end(va);

	CallService(MS_NETLIB_LOG, (WPARAM) NULL, (LPARAM) text);
}


MirandaSkinnedDialog *GetDialog(const char *name)
{
	_ASSERT(name != NULL);

	for(unsigned int i = 0; i < dlgs.size(); i++)
	{
		MirandaSkinnedDialog *dlg = dlgs[i];
		if (strcmp(name, dlg->getName()) == 0)
			return dlg;
	}

	return NULL;
}

void getSkinnedDialogFilename(std::tstring &ret, const TCHAR *skin, const char *dialogName)
{
	ret = skinsFolder;
	ret += _T("\\");
	ret += skin;
	ret += _T("\\");
	ret += CharToTchar(dialogName);
	ret += _T(".");
	ret += _T(SKIN_EXTENSION);
}

void getAvaiableSkins(std::vector<std::tstring> &skins, MirandaSkinnedDialog *dlg)
{
	TCHAR file[1024];
	mir_sntprintf(file, MAX_REGS(file), _T("%s\\*"), skinsFolder);

	WIN32_FIND_DATA ffd = {0};
	HANDLE hFFD = FindFirstFile(file, &ffd);
	if (hFFD == INVALID_HANDLE_VALUE)
		return;

	do
	{
		if (lstrcmp(ffd.cFileName, _T(".")) == 0 || lstrcmp(ffd.cFileName, _T("..")) == 0)
			continue;

		mir_sntprintf(file, MAX_REGS(file), _T("%s\\%s"), skinsFolder, ffd.cFileName);
		if (!DirExists(file))
			continue;

		if (dlg != NULL)
		{
			std::tstring filename;
			getSkinnedDialogFilename(filename, ffd.cFileName, dlg->getName());
			if (!FileExists(filename.c_str()))
				continue;
		}

		skins.push_back(std::tstring(ffd.cFileName));
	}
	while(FindNextFile(hFFD, &ffd));

	FindClose(hFFD);
}

void OnError(void *param, const TCHAR *err)
{
	MirandaSkinnedDialog *dlg = (MirandaSkinnedDialog *) param;
	
	std::tstring title;
	title = CharToTchar(dlg->getName());
	title += _T("- Error executing skin");

	MessageBox(NULL, err, title.c_str(), MB_OK | MB_ICONERROR);
}

SKINNED_DIALOG Interface_RegisterDialog(const char *name, const char *description, const char *module)
{
	if (name == NULL || name[0] == 0 || module == NULL || module[0] == 0)
		return NULL;
	if (GetDialog(name) != NULL)
		return NULL;

	// Check if default skin exists
	std::tstring filename;
	filename = skinsFolder;
	filename += _T("\\");
	filename += _T(DEFAULT_SKIN_NAME);
	filename += _T("\\");
	filename += Utf8ToTchar(name);
	filename += _T(".");
	filename += _T(SKIN_EXTENSION);

	if (!FileExists(filename.c_str()))
		return NULL;

	MirandaSkinnedDialog *dlg = new MirandaSkinnedDialog(name, description, module);
	dlg->setErrorCallback(OnError, dlg);
	dlgs.push_back(dlg);
	return (SKINNED_DIALOG) dlg;
}

void Interface_DeleteDialog(SKINNED_DIALOG aDlg)
{
	if (aDlg == NULL)
		return;

	MirandaSkinnedDialog * dlg = (MirandaSkinnedDialog *) aDlg;
	for(std::vector<MirandaSkinnedDialog*>::iterator it = dlgs.begin(); it != dlgs.end(); it++)
	{
		if (*it == dlg)
		{
			dlgs.erase(it);
			break;
		}
	}

	delete dlg;
}

void Interface_SetSkinChangedCallback(SKINNED_DIALOG aDlg, SkinOptionsChangedCallback cb, void *param)
{
	if (aDlg == NULL)
		return;

	MirandaSkinnedDialog * dlg = (MirandaSkinnedDialog *) aDlg;
	dlg->setOnSkinChangedCallback((MirandaSkinnedCallback) cb, param);
}

void Interface_FinishedConfiguring(SKINNED_DIALOG aDlg)
{
	if (aDlg == NULL)
		return;

	MirandaSkinnedDialog *dlg = (MirandaSkinnedDialog *) aDlg;
	dlg->finishedConfiguring();
}

SKINNED_FIELD Interface_AddTextField(SKINNED_DIALOG aDlg, const char *name, const char *description)
{
	if (aDlg == NULL || name == NULL || name[0] == 0)
		return NULL;

	MirandaSkinnedDialog *dlg = (MirandaSkinnedDialog *) aDlg;

	MirandaTextField *field = new MirandaTextField(dlg, name, description);
	if (!dlg->addField(field))
	{
		delete field;
		return NULL;
	}

	return (SKINNED_FIELD) field;
}

SKINNED_FIELD Interface_AddIconField(SKINNED_DIALOG aDlg, const char *name, const char *description)
{
	if (aDlg == NULL || name == NULL || name[0] == 0)
		return NULL;

	MirandaSkinnedDialog *dlg = (MirandaSkinnedDialog *) aDlg;

	MirandaIconField *field = new MirandaIconField(dlg, name, description);
	if (!dlg->addField(field))
	{
		delete field;
		return NULL;
	}

	return (SKINNED_FIELD) field;
}

SKINNED_FIELD Interface_AddImageField(SKINNED_DIALOG aDlg, const char *name, const char *description)
{
	if (aDlg == NULL || name == NULL || name[0] == 0)
		return NULL;

	MirandaSkinnedDialog *dlg = (MirandaSkinnedDialog *) aDlg;

	MirandaImageField *field = new MirandaImageField(dlg, name, description);
	if (!dlg->addField(field))
	{
		delete field;
		return NULL;
	}

	return (SKINNED_FIELD) field;
}

SKINNED_FIELD Interface_GetField(SKINNED_DIALOG aDlg, const char *name)
{
	if (aDlg == NULL || name == NULL || name[0] == 0)
		return NULL;

	MirandaSkinnedDialog *dlg = (MirandaSkinnedDialog *) aDlg;

	return (SKINNED_FIELD) dlg->getField(name);
}

void Interface_SetDialogSize(SKINNED_DIALOG aDlg, int width, int height)
{
	if (aDlg == NULL)
		return;

	MirandaSkinnedDialog *dlg = (MirandaSkinnedDialog *) aDlg;
	dlg->setSize(Size(width, height));
}

void Interface_SetInfoInt(SKINNED_DIALOG aDlg, const char *name, int value)
{
	if (aDlg == NULL || name == NULL)
		return;

	MirandaSkinnedDialog *dlg = (MirandaSkinnedDialog *) aDlg;
	dlg->getInfo()->set(name, value);
}

void Interface_SetInfoDouble(SKINNED_DIALOG aDlg, const char *name, double value)
{
	if (aDlg == NULL || name == NULL)
		return;

	MirandaSkinnedDialog *dlg = (MirandaSkinnedDialog *) aDlg;
	dlg->getInfo()->set(name, value);
}

void Interface_SetInfoBool(SKINNED_DIALOG aDlg, const char *name, BOOL value)
{
	if (aDlg == NULL || name == NULL)
		return;

	MirandaSkinnedDialog *dlg = (MirandaSkinnedDialog *) aDlg;
	dlg->getInfo()->set(name, value);
}

void Interface_SetInfoString(SKINNED_DIALOG aDlg, const char *name, const TCHAR *value)
{
	if (aDlg == NULL || name == NULL)
		return;

	MirandaSkinnedDialog *dlg = (MirandaSkinnedDialog *) aDlg;
	dlg->getInfo()->set(name, value);
}

void Interface_RemoveInfo(SKINNED_DIALOG aDlg, const char *name)
{
	if (aDlg == NULL || name == NULL)
		return;

	MirandaSkinnedDialog *dlg = (MirandaSkinnedDialog *) aDlg;
	dlg->getInfo()->remove(name);
}


void Interface_SetEnabled(SKINNED_FIELD aField, BOOL enabled)
{
	if (aField == NULL)
		return;

	Field *field = (Field *) aField;
	field->setEnabled(enabled != 0);
}

void Interface_SetToolTipA(SKINNED_FIELD aField, const char *tooltip)
{
	if (aField == NULL)
		return;

	Field *field = (Field *) aField;
	field->setToolTip(CharToTchar(tooltip));
}

void Interface_SetToolTipW(SKINNED_FIELD aField, const WCHAR *tooltip)
{
	if (aField == NULL)
		return;

	Field *field = (Field *) aField;
	field->setToolTip(WcharToTchar(tooltip));
}

void Interface_SetTextA(SKINNED_FIELD aField, const char *text)
{
	if (aField == NULL)
		return;

	Field *field = (Field *) aField;
	switch(field->getType())
	{
	case SIMPLE_TEXT:
		((TextField *) field)->setText(CharToTchar(text));
		break;
	case CONTROL_LABEL:
	case CONTROL_BUTTON:
	case CONTROL_EDIT:
		((ControlField *) field)->setText(CharToTchar(text));
		break;
	}
}

void Interface_SetTextW(SKINNED_FIELD aField, const WCHAR *text)
{
	if (aField == NULL)
		return;

	Field *field = (Field *) aField;
	switch(field->getType())
	{
	case SIMPLE_TEXT:
		((TextField *) field)->setText(WcharToTchar(text));
		break;
	case CONTROL_LABEL:
	case CONTROL_BUTTON:
	case CONTROL_EDIT:
		((ControlField *) field)->setText(WcharToTchar(text));
		break;
	}
}

void Interface_SetIcon(SKINNED_FIELD aField, HICON hIcon)
{
	if (aField == NULL)
		return;

	Field *field = (Field *) aField;
	switch(field->getType())
	{
	case SIMPLE_ICON:
		((IconField *) field)->setIcon(hIcon);
		break;
	}
}

void Interface_SetImage(SKINNED_FIELD aField, HBITMAP hBmp)
{
	if (aField == NULL)
		return;

	Field *field = (Field *) aField;
	switch(field->getType())
	{
	case SIMPLE_IMAGE:
		((ImageField *) field)->setImage(hBmp);
		break;
	}
}

SKINNED_DIALOG_STATE Interface_Run(SKINNED_DIALOG aDlg)
{
	if (aDlg == NULL)
		return NULL;

	MirandaSkinnedDialog *dlg = (MirandaSkinnedDialog *) aDlg;
	return (SKINNED_DIALOG_STATE) dlg->getState();
}

/*
void Interface_DeleteDialogState(SKINNED_DIALOG_STATE aDlg)
{
	if (aDlg == NULL)
		return;

	DialogState * dlg = (DialogState *) aDlg;

	delete dlg;
}
*/

SKINNED_FIELD_STATE Interface_GetFieldState(SKINNED_DIALOG_STATE aDlg, const char *name)
{
	if (aDlg == NULL || name == NULL || name[0] == 0)
		return NULL;

	DialogState *dlg = (DialogState *) aDlg;

	return (SKINNED_FIELD_STATE) dlg->getField(name);
}

RECT Interface_GetDialogBorders(SKINNED_DIALOG_STATE aDlg)
{
	RECT ret = {0};

	if (aDlg == NULL)
		return ret;

	DialogState *state = (DialogState *) aDlg;
	BorderState *borders = state->getBorders();

	ret.left = borders->getLeft();
	ret.top = borders->getTop();
	ret.right = borders->getRight();
	ret.bottom = borders->getBottom();

	return ret;
}

RECT Interface_GetRect(SKINNED_FIELD_STATE field)
{
	if (field == NULL)
	{
		RECT ret = {0};
		return ret;
	}

	FieldState *fieldState = (FieldState *) field;

	return fieldState->getRect();
}

RECT Interface_GetInsideRect(SKINNED_FIELD_STATE field)
{
	if (field == NULL)
	{
		RECT ret = {0};
		return ret;
	}

	FieldState *fieldState = (FieldState *) field;

	return fieldState->getInsideRect();
}

RECT Interface_GetRawRect(SKINNED_FIELD_STATE field)
{
	if (field == NULL)
	{
		RECT ret = {0};
		return ret;
	}

	FieldState *fieldState = (FieldState *) field;

	return fieldState->getRect(true);
}

RECT Interface_GetRawInsideRect(SKINNED_FIELD_STATE field)
{
	if (field == NULL)
	{
		RECT ret = {0};
		return ret;
	}

	FieldState *fieldState = (FieldState *) field;

	return fieldState->getInsideRect(true);
}

RECT Interface_GetBorders(SKINNED_FIELD_STATE field)
{
	RECT ret = {0};

	if (field == NULL)
		return ret;

	FieldState *fieldState = (FieldState *) field;
	BorderState *borders = fieldState->getBorders();

	ret.left = borders->getLeft();
	ret.top = borders->getTop();
	ret.right = borders->getRight();
	ret.bottom = borders->getBottom();

	return ret;
}

BOOL Interface_IsVisible(SKINNED_FIELD_STATE field)
{
	if (field == NULL)
		return FALSE;

	FieldState *fieldState = (FieldState *) field;

	return fieldState->isVisible();
}

char * Interface_GetToolTipA(SKINNED_FIELD_STATE field)
{
	if (field == NULL)
		return NULL;

	FieldState *fieldState = (FieldState *) field;
	return TcharToChar(fieldState->getToolTip()).detach();
}

WCHAR * Interface_GetToolTipW(SKINNED_FIELD_STATE field) 
{
	if (field == NULL)
		return NULL;

	FieldState *fieldState = (FieldState *) field;
	return TcharToWchar(fieldState->getToolTip()).detach();
}

char * Interface_GetTextA(SKINNED_FIELD_STATE field)
{
	if (field == NULL)
		return NULL;

	FieldState *fieldState = (FieldState *) field;

	switch(fieldState->getField()->getType())
	{
		case SIMPLE_TEXT:
			return TcharToChar(((TextFieldState *) fieldState)->getText()).detach();
		case CONTROL_LABEL:
		case CONTROL_BUTTON:
		case CONTROL_EDIT:
			return TcharToChar(((ControlFieldState *) field)->getText()).detach();
	}

	return NULL;
}

WCHAR * Interface_GetTextW(SKINNED_FIELD_STATE field) 
{
	if (field == NULL)
		return NULL;

	FieldState *fieldState = (FieldState *) field;

	switch(fieldState->getField()->getType())
	{
		case SIMPLE_TEXT:
			return TcharToWchar(((TextFieldState *) fieldState)->getText()).detach();
		case CONTROL_LABEL:
		case CONTROL_BUTTON:
		case CONTROL_EDIT:
			return TcharToWchar(((ControlFieldState *) field)->getText()).detach();
	}

	return NULL;
}

HFONT Interface_GetFont(SKINNED_FIELD_STATE field)
{
	if (field == NULL)
		return NULL;

	FieldState *fieldState = (FieldState *) field;

	switch(fieldState->getField()->getType())
	{
		case SIMPLE_TEXT:
			return ((TextFieldState *) fieldState)->getFont()->getHFONT();
		case CONTROL_LABEL:
		case CONTROL_BUTTON:
		case CONTROL_EDIT:
			return ((ControlFieldState *) fieldState)->getFont()->getHFONT();
	}

	return NULL;
}

COLORREF Interface_GetFontColor(SKINNED_FIELD_STATE field)
{
	if (field == NULL)
		return RGB(0,0,0);

	FieldState *fieldState = (FieldState *) field;

	switch(fieldState->getField()->getType())
	{
		case SIMPLE_TEXT:
			return ((TextFieldState *) fieldState)->getFont()->getColor();
		case CONTROL_LABEL:
		case CONTROL_BUTTON:
		case CONTROL_EDIT:
			return ((ControlFieldState *) fieldState)->getFont()->getColor();
	}

	return RGB(0,0,0);
}

int Interface_GetHorizontalAlign(SKINNED_FIELD_STATE field) 
{
	if (field == NULL)
		return SKN_HALIGN_LEFT;

	FieldState *fieldState = (FieldState *) field;
	return fieldState->getHAlign();
}

int Interface_GetVerticalAlign(SKINNED_FIELD_STATE field) 
{
	if (field == NULL)
		return SKN_VALIGN_TOP;

	FieldState *fieldState = (FieldState *) field;
	return fieldState->getVAlign();
}

HICON Interface_GetIcon(SKINNED_FIELD_STATE field)
{
	if (field == NULL)
		return NULL;

	FieldState *fieldState = (FieldState *) field;

	switch(fieldState->getField()->getType())
	{
		case SIMPLE_ICON:
			return ((IconFieldState *) fieldState)->getIcon();
	}
	return NULL;
}

HBITMAP Interface_GetImage(SKINNED_FIELD_STATE field)
{
	if (field == NULL)
		return NULL;

	FieldState *fieldState = (FieldState *) field;

	switch(fieldState->getField()->getType())
	{
		case SIMPLE_IMAGE:
			return ((ImageFieldState *) fieldState)->getImage();
	}
	return NULL;
}


static int Service_GetInterface(WPARAM wParam, LPARAM lParam)
{
	SKIN_INTERFACE *mski = (SKIN_INTERFACE *) lParam;
	if (mski == NULL)
		return -1;

	if (mski->cbSize < sizeof(SKIN_INTERFACE))
		return -2;

	mski->RegisterDialog = &Interface_RegisterDialog;
	mski->DeleteDialog = &Interface_DeleteDialog;
	mski->SetSkinChangedCallback = &Interface_SetSkinChangedCallback;
	mski->FinishedConfiguring = &Interface_FinishedConfiguring;

	mski->AddTextField = &Interface_AddTextField;
	mski->AddIconField = &Interface_AddIconField;
	mski->AddImageField = &Interface_AddImageField;
	mski->GetField = &Interface_GetField;
	mski->SetDialogSize = &Interface_SetDialogSize;

	mski->SetInfoInt = &Interface_SetInfoInt;
	mski->SetInfoDouble = &Interface_SetInfoDouble;
	mski->SetInfoBool = &Interface_SetInfoBool;
	mski->SetInfoString = &Interface_SetInfoString;
	mski->RemoveInfo = &Interface_RemoveInfo;

	mski->SetEnabled = &Interface_SetEnabled;
	mski->SetToolTipA = &Interface_SetToolTipA;
	mski->SetToolTipW = &Interface_SetToolTipW;

	mski->SetTextA = &Interface_SetTextA;
	mski->SetTextW = &Interface_SetTextW;

	mski->SetIcon = &Interface_SetIcon;

	mski->SetImage = &Interface_SetImage;

	mski->Run = &Interface_Run;

	mski->GetFieldState = &Interface_GetFieldState;
	mski->GetDialogBorders = &Interface_GetDialogBorders;

	mski->GetRect = &Interface_GetRect;
	mski->GetInsideRect = &Interface_GetInsideRect;
	mski->GetRawRect = &Interface_GetRawRect;
	mski->GetRawInsideRect = &Interface_GetRawInsideRect;

	mski->GetBorders = &Interface_GetBorders;
	mski->IsVisible = &Interface_IsVisible;
	mski->GetToolTipA = &Interface_GetToolTipA;
	mski->GetToolTipW = &Interface_GetToolTipW;
	mski->GetHorizontalAlign = &Interface_GetHorizontalAlign;
	mski->GetVerticalAlign = &Interface_GetVerticalAlign;

	mski->GetTextA = &Interface_GetTextA;
	mski->GetTextW = &Interface_GetTextW;
	mski->GetFont = &Interface_GetFont;
	mski->GetFontColor = &Interface_GetFontColor;

	mski->GetIcon = &Interface_GetIcon;

	mski->GetImage = &Interface_GetImage;

	return 0;
}
