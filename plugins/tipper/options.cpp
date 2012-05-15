#include "common.h"
#include "options.h"
#include "resource.h"
#include "popwin.h"
#include "str_utils.h"

Options options;

#define WMU_ENABLE_LIST_BUTTONS			(WM_USER + 0x030)
#define WMU_ENABLE_MODULE_ENTRY			(WM_USER + 0x031)

void CreateDefaultItems() {
	DSListNode *ds_node;
	DIListNode *di_node;
	
	// last message item
	di_node = (DIListNode *)malloc(sizeof(DIListNode));
	_tcsncpy(di_node->di.label, _T("Last message: (%sys:last_msg_reltime% ago)"), LABEL_LEN);
	_tcsncpy(di_node->di.value, _T("%sys:last_msg%"), VALUE_LEN);
	di_node->di.line_above = di_node->di.value_newline = true;
	di_node->next = options.di_list;
	options.di_list = di_node;
	options.di_count++;

	// status message item
	di_node = (DIListNode *)malloc(sizeof(DIListNode));
	_tcsncpy(di_node->di.label, _T("Status message:"), LABEL_LEN);
	_tcsncpy(di_node->di.value, _T("%sys:status_msg%"), VALUE_LEN);
	di_node->di.line_above = di_node->di.value_newline = true;
	di_node->next = options.di_list;
	options.di_list = di_node;
	options.di_count++;

	// status substitution
	ds_node = (DSListNode *)malloc(sizeof(DSListNode));
	_tcsncpy(ds_node->ds.name, _T("status"), LABEL_LEN);
	ds_node->ds.type = DVT_PROTODB;
	strncpy(ds_node->ds.setting_name, "Status", SETTING_NAME_LEN);
	ds_node->ds.translate_func_id = 1;
	ds_node->next = options.ds_list;
	options.ds_list = ds_node;
	options.ds_count++;

	// status item
	di_node = (DIListNode *)malloc(sizeof(DIListNode));
	_tcsncpy(di_node->di.label, _T("Status:"), LABEL_LEN);
	_tcsncpy(di_node->di.value, _T("%status%"), VALUE_LEN);
	di_node->di.line_above = di_node->di.value_newline = false;
	di_node->next = options.di_list;
	options.di_list = di_node;
	options.di_count++;

	// Contact time
	di_node = (DIListNode *)malloc(sizeof(DIListNode));
	_tcsncpy(di_node->di.label, _T("Time:"), LABEL_LEN);
	_tcsncpy(di_node->di.value, _T("%sys:time%"), VALUE_LEN);
	di_node->di.line_above = di_node->di.value_newline = false;
	di_node->next = options.di_list;
	options.di_list = di_node;
	options.di_count++;

	// client substitution
	ds_node = (DSListNode *)malloc(sizeof(DSListNode));
	_tcsncpy(ds_node->ds.name, _T("client"), LABEL_LEN);
	ds_node->ds.type = DVT_PROTODB;
	strncpy(ds_node->ds.setting_name, "MirVer", SETTING_NAME_LEN);
	ds_node->ds.translate_func_id = 0;
	ds_node->next = options.ds_list;
	options.ds_list = ds_node;
	options.ds_count++;

	// client item
	di_node = (DIListNode *)malloc(sizeof(DIListNode));
	_tcsncpy(di_node->di.label, _T("Client:"), LABEL_LEN);
	_tcsncpy(di_node->di.value, _T("%client%"), VALUE_LEN);
	di_node->di.line_above = di_node->di.value_newline = false;
	di_node->next = options.di_list;
	options.di_list = di_node;
	options.di_count++;

	// idle time substitution (long date)
	ds_node = (DSListNode *)malloc(sizeof(DSListNode));
	_tcsncpy(ds_node->ds.name, _T("idle"), LABEL_LEN);
	ds_node->ds.type = DVT_PROTODB;
	strncpy(ds_node->ds.setting_name, "IdleTS", SETTING_NAME_LEN);
	ds_node->ds.translate_func_id = 15;
	ds_node->next = options.ds_list;
	options.ds_list = ds_node;
	options.ds_count++;

	// idle time substitution (time difference)
	ds_node = (DSListNode *)malloc(sizeof(DSListNode));
	_tcsncpy(ds_node->ds.name, _T("idle_diff"), LABEL_LEN);
	ds_node->ds.type = DVT_PROTODB;
	strncpy(ds_node->ds.setting_name, "IdleTS", SETTING_NAME_LEN);
	ds_node->ds.translate_func_id = 3;
	ds_node->next = options.ds_list;
	options.ds_list = ds_node;
	options.ds_count++;


	// idle item
	di_node = (DIListNode *)malloc(sizeof(DIListNode));
	_tcsncpy(di_node->di.label, _T("Idle:"), LABEL_LEN);
	_tcsncpy(di_node->di.value, _T("%idle% (%idle_diff% ago)"), VALUE_LEN);
	di_node->di.line_above = di_node->di.value_newline = false;
	di_node->next = options.di_list;
	options.di_list = di_node;
	options.di_count++;

	// first name substitution
	ds_node = (DSListNode *)malloc(sizeof(DSListNode));
	_tcsncpy(ds_node->ds.name, _T("first_name"), LABEL_LEN);
	ds_node->ds.type = DVT_PROTODB;
	strncpy(ds_node->ds.setting_name, "FirstName", SETTING_NAME_LEN);
	ds_node->ds.translate_func_id = 0;
	ds_node->next = options.ds_list;
	options.ds_list = ds_node;
	options.ds_count++;

	// last name substitution
	ds_node = (DSListNode *)malloc(sizeof(DSListNode));
	_tcsncpy(ds_node->ds.name, _T("last_name"), LABEL_LEN);
	ds_node->ds.type = DVT_PROTODB;
	strncpy(ds_node->ds.setting_name, "LastName", SETTING_NAME_LEN);
	ds_node->ds.translate_func_id = 0;
	ds_node->next = options.ds_list;
	options.ds_list = ds_node;
	options.ds_count++;

	// name item
	di_node = (DIListNode *)malloc(sizeof(DIListNode));
	_tcsncpy(di_node->di.label, _T("Name:"), LABEL_LEN);
	_tcsncpy(di_node->di.value, _T("%first_name% %last_name%"), VALUE_LEN);
	di_node->di.line_above = di_node->di.value_newline = false;
	di_node->next = options.di_list;
	options.di_list = di_node;
	options.di_count++;

}

bool LoadDS(DisplaySubst *ds, int index) {
	char setting[512];
	DBVARIANT dbv;
	mir_snprintf(setting, SIZEOF(setting), "Name%d", index);
	ds->name[0] = 0;
	if (!DBGetContactSettingTString(0, MODULE, setting, &dbv)) {
		_tcsncpy(ds->name, dbv.ptszVal, SIZEOF(ds->name));
		ds->name[SIZEOF(ds->name) - 1] = 0;
		DBFreeVariant(&dbv);
	} else
		return false;

	mir_snprintf(setting, SIZEOF(setting), "Type%d", index);
	ds->type = (DisplaySubstType)DBGetContactSettingByte(0, MODULE, setting, DVT_PROTODB);
	
	mir_snprintf(setting, SIZEOF(setting), "Module%d", index);
	ds->module_name[0] = 0;
	if(!DBGetContactSettingString(0, MODULE, setting, &dbv)) {
		strncpy(ds->module_name, dbv.pszVal, MODULE_NAME_LEN);
		ds->module_name[MODULE_NAME_LEN - 1] = 0;
		DBFreeVariant(&dbv);
	}

	mir_snprintf(setting, 512, "Setting%d", index);
	ds->setting_name[0] = 0;
	if(!DBGetContactSettingString(0, MODULE, setting, &dbv)) {
		strncpy(ds->setting_name, dbv.pszVal, SETTING_NAME_LEN);
		ds->setting_name[SETTING_NAME_LEN - 1] = 0;
		DBFreeVariant(&dbv);
	}

	mir_snprintf(setting, 512, "TransFuncId%d", index);
	ds->translate_func_id = DBGetContactSettingDword(0, MODULE, setting, (DWORD)-1);

	// a little backward compatibility
	if((DWORD)ds->translate_func_id == (DWORD)-1) {
		mir_snprintf(setting, 512, "TransFunc%d", index);
		ds->translate_func_id = (DWORD)DBGetContactSettingWord(0, MODULE, setting, 0);
	}
	return true;
}

void SaveDS(DisplaySubst *ds, int index) {	
	char setting[512];
	mir_snprintf(setting, SIZEOF(setting), "Name%d", index);
	DBWriteContactSettingTString(0, MODULE, setting, ds->name);
	mir_snprintf(setting, SIZEOF(setting), "Type%d", index);
	DBWriteContactSettingByte(0, MODULE, setting, (BYTE)ds->type);
	mir_snprintf(setting, SIZEOF(setting), "Module%d", index);
	DBWriteContactSettingString(0, MODULE, setting, ds->module_name);
	mir_snprintf(setting, SIZEOF(setting), "Setting%d", index);
	DBWriteContactSettingString(0, MODULE, setting, ds->setting_name);
	mir_snprintf(setting, SIZEOF(setting), "TransFuncId%d", index);
	DBWriteContactSettingDword(0, MODULE, setting, (WORD)ds->translate_func_id);
}

bool LoadDI(DisplayItem *di, int index) 
{
	char setting[512];
	DBVARIANT dbv;
	mir_snprintf(setting, SIZEOF(setting), "DILabel%d", index);
	di->label[0] = 0;
	if (!DBGetContactSettingTString(0, MODULE, setting, &dbv)) {
		_tcsncpy( di->label, dbv.ptszVal, SIZEOF(di->label));
		di->label[SIZEOF(di->label) - 1] = 0;
		DBFreeVariant(&dbv);
	} else 
		return false;

	mir_snprintf(setting, SIZEOF(setting), "DIValue%d", index);
	di->value[0] = 0;
	if(!DBGetContactSettingTString(0, MODULE, setting, &dbv)) {
		_tcsncpy(di->value, dbv.ptszVal, SIZEOF(di->value));
		di->value[SIZEOF(di->value) - 1] = 0;
		DBFreeVariant(&dbv);
	}

	mir_snprintf(setting, SIZEOF(setting), "DILineAbove%d", index);
	di->line_above = (DBGetContactSettingByte(0, MODULE, setting, 0) == 1);
	mir_snprintf(setting, SIZEOF(setting), "DIValNewline%d", index);
	di->value_newline = (DBGetContactSettingByte(0, MODULE, setting, 0) == 1);

	return true;
}

void SaveDI(DisplayItem *di, int index) {
	char setting[512];
	mir_snprintf(setting, SIZEOF(setting), "DILabel%d", index);
	DBWriteContactSettingTString(0, MODULE, setting, di->label);
	mir_snprintf(setting, SIZEOF(setting), "DIValue%d", index);
	DBWriteContactSettingTString(0, MODULE, setting, di->value);
	mir_snprintf(setting, SIZEOF(setting), "DILineAbove%d", index);
	DBWriteContactSettingByte(0, MODULE, setting, di->line_above ? 1 : 0);
	mir_snprintf(setting, SIZEOF(setting), "DIValNewline%d", index);
	DBWriteContactSettingByte(0, MODULE, setting, di->value_newline ? 1 : 0);
}

void SaveOptions() {
	DBWriteContactSettingDword(0, MODULE, "MaxWidth", options.win_width);
	DBWriteContactSettingDword(0, MODULE, "MaxHeight", options.win_max_height);
	DBWriteContactSettingByte(0, MODULE, "Opacity", (BYTE)options.opacity);
	DBWriteContactSettingByte(0, MODULE, "Border", (options.border ? 1 : 0));
	DBWriteContactSettingByte(0, MODULE, "DropShadow", (options.drop_shadow ? 1 : 0));
	DBWriteContactSettingByte(0, MODULE, "RoundCorners", (options.round ? 1 : 0));
	DBWriteContactSettingByte(0, MODULE, "AvatarRoundCorners", (options.av_round ? 1 : 0));
	DBWriteContactSettingByte(0, MODULE, "Animate", (options.animate ? 1 : 0));
	DBWriteContactSettingByte(0, MODULE, "TransparentBg", (options.trans_bg ? 1 : 0));
	DBWriteContactSettingByte(0, MODULE, "TitleLayout", (BYTE)options.title_layout);
	if (ServiceExists(MS_AV_DRAWAVATAR))
		DBWriteContactSettingByte(0, MODULE, "AVLayout", (BYTE)options.av_layout);
	DBWriteContactSettingDword(0, MODULE, "AVSize", options.av_size);
	DBWriteContactSettingDword(0, MODULE, "TextIndent", options.text_indent);
	DBWriteContactSettingByte(0, MODULE, "ShowNoFocus", (options.show_no_focus ? 1 : 0));

	int index = 0;
	DSListNode *ds_node = options.ds_list;
	while(ds_node) {
		SaveDS(&ds_node->ds, index);
		ds_node = ds_node->next;
		index++;
	}
	DBWriteContactSettingWord(0, MODULE, "DSNumValues", index);

	index = 0;
	DIListNode *di_node = options.di_list;
	while(di_node) {
		SaveDI(&di_node->di, index);
		di_node = di_node->next;
		index++;
	}
	DBWriteContactSettingWord(0, MODULE, "DINumValues", index);
	
	DBWriteContactSettingWord(0, MODULE, "TimeIn", options.time_in);
	CallService(MS_CLC_SETINFOTIPHOVERTIME, options.time_in, 0);

	DBWriteContactSettingWord(0, MODULE, "Padding", options.padding);
	DBWriteContactSettingWord(0, MODULE, "AvatarPadding", options.av_padding);
	DBWriteContactSettingWord(0, MODULE, "TextPadding", options.text_padding);
	DBWriteContactSettingByte(0, MODULE, "Position", (BYTE)options.pos);
	DBWriteContactSettingDword(0, MODULE, "MinWidth", (DWORD)options.min_width);
	DBWriteContactSettingDword(0, MODULE, "MinHeight", (DWORD)options.min_height);
	DBWriteContactSettingDword(0, MODULE, "SidebarWidth", (DWORD)options.sidebar_width);
	DBWriteContactSettingByte(0, MODULE, "MouseTollerance", (BYTE)options.mouse_tollerance);
	DBWriteContactSettingByte(0, MODULE, "SBarTips", (options.status_bar_tips ? 1 : 0));

	DBWriteContactSettingWord(0, MODULE, "LabelVAlign", options.label_valign);
	DBWriteContactSettingWord(0, MODULE, "LabelHAlign", options.label_halign);
	DBWriteContactSettingWord(0, MODULE, "ValueVAlign", options.value_valign);
	DBWriteContactSettingWord(0, MODULE, "ValueHAlign", options.value_halign);

	DBWriteContactSettingByte(0, MODULE, "NoAvatarResize", options.no_resize_av ? 1 : 0);
	DBWriteContactSettingTString(0, MODULE, "BackgroundFilename", options.bg_fn);
	DBWriteContactSettingByte(0, MODULE, "StretchBgImg", options.stretch_bg_img ? 1 : 0);
}

void LoadOptions() {
	options.win_width = DBGetContactSettingDword(0, MODULE, "MaxWidth", 420);
	options.win_max_height = DBGetContactSettingDword(0, MODULE, "MaxHeight", 400);
	options.opacity = DBGetContactSettingByte(0, MODULE, "Opacity", 75);
	options.border = (DBGetContactSettingByte(0, MODULE, "Border", 1) == 1);
	options.drop_shadow = (DBGetContactSettingByte(0, MODULE, "DropShadow", 1) == 1);
	options.round = (DBGetContactSettingByte(0, MODULE, "RoundCorners", 1) == 1);
	options.av_round = (DBGetContactSettingByte(0, MODULE, "AvatarRoundCorners", options.round ? 1 : 0) == 1);
	options.animate = (DBGetContactSettingByte(0, MODULE, "Animate", 0) == 1);
	options.trans_bg = (DBGetContactSettingByte(0, MODULE, "TransparentBg", 0) == 1);
	options.title_layout = (PopupTitleLayout)DBGetContactSettingByte(0, MODULE, "TitleLayout", (BYTE)PTL_LEFTICON);
	if(ServiceExists(MS_AV_DRAWAVATAR))
		options.av_layout = (PopupAvLayout)DBGetContactSettingByte(0, MODULE, "AVLayout", PAV_RIGHT);
	else
		options.av_layout = PAV_NONE;
	options.av_size = DBGetContactSettingDword(0, MODULE, "AVSize", 60); //tweety
	options.text_indent = DBGetContactSettingDword(0, MODULE, "TextIndent", 22); 
	options.sidebar_width = DBGetContactSettingDword(0, MODULE, "SidebarWidth", 22);
	options.show_no_focus = (DBGetContactSettingByte(0, MODULE, "ShowNoFocus", 1) == 1);

	int i, real_count = 0;
	options.ds_list = 0;
	DSListNode *ds_node;
	options.ds_count = DBGetContactSettingWord(0, MODULE, "DSNumValues", 0);
	for(i = options.ds_count - 1; i >= 0; i--) {
		ds_node = (DSListNode *)malloc(sizeof(DSListNode));
		if(LoadDS(&ds_node->ds, i)) {
			ds_node->next = options.ds_list;
			options.ds_list = ds_node;
			real_count++;
		} else free(ds_node);
	}
	options.ds_count = real_count;

	real_count = 0;
	options.di_list = 0;
	DIListNode *di_node;
	options.di_count = DBGetContactSettingWord(0, MODULE, "DINumValues", 0);
	for(i = options.di_count - 1; i >= 0; i--) {
		di_node = (DIListNode *)malloc(sizeof(DIListNode));
		if(LoadDI(&di_node->di, i)) {
			di_node->next = options.di_list;
			options.di_list = di_node;
			real_count++;
		} else free(di_node);
	}
	options.di_count = real_count;

	options.time_in = DBGetContactSettingWord(0, MODULE, "TimeIn", 750);
	options.padding = DBGetContactSettingWord(0, MODULE, "Padding", 4);
	options.av_padding = DBGetContactSettingWord(0, MODULE, "AvatarPadding", 6);
	options.text_padding = DBGetContactSettingWord(0, MODULE, "TextPadding", 4);
	options.pos = (PopupPosition)DBGetContactSettingByte(0, MODULE, "Position", (BYTE)PP_BOTTOMRIGHT);
	options.min_width = DBGetContactSettingDword(0, MODULE, "MinWidth", 0);
	options.min_height = DBGetContactSettingDword(0, MODULE, "MinHeight", 0);

	options.mouse_tollerance = DBGetContactSettingByte(0, MODULE, "MouseTollerance", (BYTE)GetSystemMetrics(SM_CXSMICON));
	options.status_bar_tips = (DBGetContactSettingByte(0, MODULE, "SBarTips", 1) == 1);

	// convert defunct last message and status message options to new 'sys' items, and remove the old settings
	if(DBGetContactSettingByte(0, MODULE, "ShowLastMessage", 0)) {
		DBDeleteContactSetting(0, MODULE, "ShowLastMessage");

		// find end of list
		di_node = options.di_list;
		while(di_node && di_node->next) di_node = di_node->next;
		
		// last message item
		if(di_node) {
			di_node->next = (DIListNode *)malloc(sizeof(DIListNode));
			di_node = di_node->next;
		} else {
			options.di_list = (DIListNode *)malloc(sizeof(DIListNode));
			di_node = options.di_list;
		}

		_tcsncpy(di_node->di.label, _T("Last message: (%sys:last_msg_reltime% ago)"), LABEL_LEN);
		_tcsncpy(di_node->di.value, _T("%sys:last_msg%"), VALUE_LEN);
		di_node->di.line_above = di_node->di.value_newline = true;
		di_node->next = 0;
		options.di_count++;
	}

	if(DBGetContactSettingByte(0, MODULE, "ShowStatusMessage", 0)) {
		DBDeleteContactSetting(0, MODULE, "ShowStatusMessage");

		// find end of list
		di_node = options.di_list;
		while(di_node && di_node->next) di_node = di_node->next;
		
		// status message item
		if(di_node) {
			di_node->next = (DIListNode *)malloc(sizeof(DIListNode));
			di_node = di_node->next;
		} else {
			options.di_list = (DIListNode *)malloc(sizeof(DIListNode));
			di_node = options.di_list;
		}
			
		_tcsncpy(di_node->di.label, _T("Status message:"), LABEL_LEN);
		_tcsncpy(di_node->di.value, _T("%sys:status_msg%"), VALUE_LEN);
		di_node->di.line_above = di_node->di.value_newline = true;
		di_node->next = 0;
		options.di_count++;
	}

	options.label_valign = DBGetContactSettingWord(0, MODULE, "LabelVAlign", DT_TOP /*DT_VCENTER*/);
	options.label_halign = DBGetContactSettingWord(0, MODULE, "LabelHAlign", DT_LEFT);
	options.value_valign = DBGetContactSettingWord(0, MODULE, "ValueVAlign", DT_TOP /*DT_VCENTER*/);
	options.value_halign = DBGetContactSettingWord(0, MODULE, "ValueHAlign", DT_LEFT);

	if(options.ds_count == 0 && options.di_count == 0 && DBGetContactSettingByte(0, MODULE, "DefaultsCreated", 0) == 0) {
		// set up some reasonable defaults - but only 'once'
		CreateDefaultItems();
		DBWriteContactSettingByte(0, MODULE, "DefaultsCreated", 1);
		SaveOptions();
	}

	options.no_resize_av = (DBGetContactSettingByte(0, MODULE, "NoAvatarResize", 0) == 1);
	DBVARIANT dbv;
	if(!DBGetContactSettingTString(0, MODULE, "BackgroundFilename", &dbv)) {
		_tcsncpy(options.bg_fn, dbv.ptszVal, MAX_PATH);
		DBFreeVariant(&dbv);
	} else
		options.bg_fn[0] = 0;
	options.stretch_bg_img = (DBGetContactSettingByte(0, MODULE, "StretchBgImg", 0) == 1);
}

static INT_PTR CALLBACK DlgProcAddItem(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	DisplayItem *di = (DisplayItem *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch ( msg ) {
	case WM_INITDIALOG:
		TranslateDialogDefault( hwndDlg );
		di = (DisplayItem *)lParam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)di);

		SetDlgItemText(hwndDlg, IDC_ED_LABEL, di->label);
		SetDlgItemText(hwndDlg, IDC_ED_VALUE, di->value);

		CheckDlgButton(hwndDlg, IDC_CHK_LINEABOVE, di->line_above ? TRUE : FALSE);
		CheckDlgButton(hwndDlg, IDC_CHK_VALNEWLINE, di->value_newline ? TRUE : FALSE);
		SetFocus(GetDlgItem(hwndDlg, IDC_ED_LABEL));
		return TRUE;
	case WM_COMMAND:
		if(HIWORD(wParam) == BN_CLICKED) {
			switch(LOWORD(wParam)) {
				case IDOK:
					GetDlgItemText(hwndDlg, IDC_ED_LABEL, di->label, LABEL_LEN);
					GetDlgItemText(hwndDlg, IDC_ED_VALUE, di->value, VALUE_LEN);

					di->line_above = (IsDlgButtonChecked(hwndDlg, IDC_CHK_LINEABOVE) ? true : false);
					di->value_newline = (IsDlgButtonChecked(hwndDlg, IDC_CHK_VALNEWLINE) ? true : false);

					EndDialog(hwndDlg, IDOK);
					return TRUE;
				case IDCANCEL:
					EndDialog(hwndDlg, IDCANCEL);
					return TRUE;
			}
		}
		break;
	}

	return 0;
}

static INT_PTR CALLBACK DlgProcAddSubst(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	DisplaySubst *ds = (DisplaySubst *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch ( msg ) {
	case WM_INITDIALOG:
		TranslateDialogDefault( hwndDlg );
		ds = (DisplaySubst *)lParam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)ds);

		SetDlgItemText(hwndDlg, IDC_ED_LABEL, ds->name);

		switch(ds->type) {
			case DVT_PROTODB:
				CheckDlgButton(hwndDlg, IDC_CHK_PROTOMOD, TRUE);
				SetDlgItemTextA(hwndDlg, IDC_ED_SETTING, ds->setting_name);
				break;
			case DVT_DB:
				SetDlgItemTextA(hwndDlg, IDC_ED_MODULE, ds->module_name);
				SetDlgItemTextA(hwndDlg, IDC_ED_SETTING, ds->setting_name);
				break;
		}

		{
			int index, id, i;
			for(i = 0; i < num_tfuncs; i++) {
				index = SendDlgItemMessage(hwndDlg, IDC_CMB_TRANSLATE, CB_ADDSTRING, (WPARAM)-1, (LPARAM)TranslateTS(translations[i].name));
				SendDlgItemMessage(hwndDlg, IDC_CMB_TRANSLATE, CB_SETITEMDATA, index, (LPARAM)translations[i].id);
			}
			for(i = 0; i < num_tfuncs; i++) {
				id = SendDlgItemMessage(hwndDlg, IDC_CMB_TRANSLATE, CB_GETITEMDATA, i, 0);
				if(id == ds->translate_func_id)
					SendDlgItemMessage(hwndDlg, IDC_CMB_TRANSLATE, CB_SETCURSEL, i, 0);
			}
		}

		SendMessage(hwndDlg, WMU_ENABLE_MODULE_ENTRY, 0, 0);
		SetFocus(GetDlgItem(hwndDlg, IDC_ED_LABEL));
		return TRUE;
	case WMU_ENABLE_MODULE_ENTRY:
		{
			HWND hw = GetDlgItem(hwndDlg, IDC_CHK_PROTOMOD);
			EnableWindow(hw, TRUE);
			hw = GetDlgItem(hwndDlg, IDC_ED_MODULE);
			EnableWindow(hw, !IsDlgButtonChecked(hwndDlg, IDC_CHK_PROTOMOD));
			hw = GetDlgItem(hwndDlg, IDC_ED_SETTING);
			EnableWindow(hw, TRUE);
		}
		return TRUE;
	case WM_COMMAND:
		if ( HIWORD( wParam ) == CBN_SELCHANGE) {
			return TRUE;
		} else if(HIWORD(wParam) == BN_CLICKED) {
			switch(LOWORD(wParam)) {
				case IDC_CHK_PROTOMOD:
					SendMessage(hwndDlg, WMU_ENABLE_MODULE_ENTRY, 0, 0);
					break;
				case IDOK:
					GetDlgItemText(hwndDlg, IDC_ED_LABEL, ds->name, LABEL_LEN);
					if(ds->name[0] == 0) {
						MessageBox(hwndDlg, TranslateT("You must enter a label"), TranslateT("Invalid Substitution"), MB_OK | MB_ICONWARNING);
						return TRUE;
					}

					if(IsDlgButtonChecked(hwndDlg, IDC_CHK_PROTOMOD))
						ds->type = DVT_PROTODB;
					else {
						ds->type = DVT_DB;
						GetDlgItemTextA(hwndDlg, IDC_ED_MODULE, ds->module_name, MODULE_NAME_LEN);
					}
					GetDlgItemTextA(hwndDlg, IDC_ED_SETTING, ds->setting_name, SETTING_NAME_LEN);

					{
						int sel = SendDlgItemMessage(hwndDlg, IDC_CMB_TRANSLATE, CB_GETCURSEL, 0, 0);
						ds->translate_func_id = SendDlgItemMessage(hwndDlg, IDC_CMB_TRANSLATE, CB_GETITEMDATA, sel, 0);
					}

					EndDialog(hwndDlg, IDOK);
					return TRUE;
				case IDCANCEL:
					EndDialog(hwndDlg, IDCANCEL);
					return TRUE;
			}
		}
		break;
	}

	return 0;
}

static INT_PTR CALLBACK DlgProcOptContent(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {

	switch ( msg ) {
	case WM_INITDIALOG:
		TranslateDialogDefault( hwndDlg );
		{
			int index;
			DIListNode *di_node = options.di_list, *di_value;
			while(di_node) {
				di_value = (DIListNode *)malloc(sizeof(DIListNode));
				*di_value = *di_node;
				if(di_value->di.label[0] == 0)
					index = SendDlgItemMessage(hwndDlg, IDC_LST_ITEMS, LB_ADDSTRING, 0, (LPARAM)TranslateT("<No Label>"));
				else
					index = SendDlgItemMessage(hwndDlg, IDC_LST_ITEMS, LB_ADDSTRING, 0, (LPARAM)di_value->di.label);
				SendDlgItemMessage(hwndDlg, IDC_LST_ITEMS, LB_SETITEMDATA, index, (LPARAM)di_value);

				di_node = di_node->next;
			}

			DSListNode *ds_node = options.ds_list, *ds_value;
			while(ds_node) {
				ds_value = (DSListNode *)malloc(sizeof(DSListNode));
				*ds_value = *ds_node;
				index = SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_ADDSTRING, 0, (LPARAM)ds_value->ds.name);
				SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_SETITEMDATA, index, (LPARAM)ds_value);

				ds_node = ds_node->next;
			}
		}

		SendMessage(hwndDlg, WMU_ENABLE_LIST_BUTTONS, 0, 0);
		return FALSE;
	case WMU_ENABLE_LIST_BUTTONS:
		{
			int sel = SendDlgItemMessage(hwndDlg, IDC_LST_ITEMS, LB_GETCURSEL, 0, 0);
			if(sel == -1) {
				HWND hw = GetDlgItem(hwndDlg, IDC_BTN_REMOVE);
				EnableWindow(hw, FALSE);
				hw = GetDlgItem(hwndDlg, IDC_BTN_UP);
				EnableWindow(hw, FALSE);
				hw = GetDlgItem(hwndDlg, IDC_BTN_DOWN);
				EnableWindow(hw, FALSE);
				hw = GetDlgItem(hwndDlg, IDC_BTN_EDIT);
				EnableWindow(hw, FALSE);
			} else {
				int count = SendDlgItemMessage(hwndDlg, IDC_LST_ITEMS, LB_GETCOUNT, 0, 0);
				HWND hw = GetDlgItem(hwndDlg, IDC_BTN_REMOVE);
				EnableWindow(hw, TRUE);
				hw = GetDlgItem(hwndDlg, IDC_BTN_UP);
				EnableWindow(hw, sel > 0);
				hw = GetDlgItem(hwndDlg, IDC_BTN_DOWN);
				EnableWindow(hw, sel < count - 1);
				hw = GetDlgItem(hwndDlg, IDC_BTN_EDIT);
				EnableWindow(hw, TRUE);
			}

			sel = SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_GETCURSEL, 0, 0);
			if(sel == -1) {
				HWND hw = GetDlgItem(hwndDlg, IDC_BTN_REMOVE2);
				EnableWindow(hw, FALSE);
				hw = GetDlgItem(hwndDlg, IDC_BTN_EDIT2);
				EnableWindow(hw, FALSE);
			} else {
				HWND hw = GetDlgItem(hwndDlg, IDC_BTN_REMOVE2);
				EnableWindow(hw, TRUE);
				hw = GetDlgItem(hwndDlg, IDC_BTN_EDIT2);
				EnableWindow(hw, TRUE);
			}
		}
		return TRUE;		
	case WM_COMMAND:
		if ( HIWORD( wParam ) == LBN_SELCHANGE && LOWORD(wParam) == IDC_LST_ITEMS) {
			SendMessage(hwndDlg, WMU_ENABLE_LIST_BUTTONS, 0, 0);
		} else if ( HIWORD( wParam ) == LBN_SELCHANGE && LOWORD(wParam) == IDC_LST_SUBST) {
			SendMessage(hwndDlg, WMU_ENABLE_LIST_BUTTONS, 0, 0);
		} else if ( HIWORD( wParam ) == CBN_SELCHANGE) {
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		} else if ( HIWORD( wParam ) == EN_CHANGE && ( HWND )lParam == GetFocus()) {
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		} else if ( HIWORD( wParam) == LBN_DBLCLK && LOWORD(wParam) == IDC_LST_ITEMS) {
			{
				DIListNode *value;
				int sel = SendDlgItemMessage(hwndDlg, IDC_LST_ITEMS, LB_GETCURSEL, 0, 0);
				if(sel != -1) {
					value = (DIListNode *)SendDlgItemMessage(hwndDlg, IDC_LST_ITEMS, LB_GETITEMDATA, sel, 0);
					if(DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_ITEM), hwndDlg, DlgProcAddItem, (LPARAM)&value->di) == IDOK) {
						SendDlgItemMessage(hwndDlg, IDC_LST_ITEMS, LB_DELETESTRING, (WPARAM)sel, 0);

						SendDlgItemMessage(hwndDlg, IDC_LST_ITEMS, LB_INSERTSTRING, sel, (LPARAM)value->di.label);
						SendDlgItemMessage(hwndDlg, IDC_LST_ITEMS, LB_SETITEMDATA, sel, (LPARAM)value);

						SendDlgItemMessage(hwndDlg, IDC_LST_ITEMS, LB_SETCURSEL, sel, 0);

						SendMessage(hwndDlg, WMU_ENABLE_LIST_BUTTONS, 0, 0);
						SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					}
				}
			}
		} else if ( HIWORD( wParam) == LBN_DBLCLK && LOWORD(wParam) == IDC_LST_SUBST) {
			{
				DSListNode *value;
				int sel = SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_GETCURSEL, 0, 0);
				if(sel != -1) {
					value = (DSListNode *)SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_GETITEMDATA, sel, 0);
					if(DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_SUBST), hwndDlg, DlgProcAddSubst, (LPARAM)&value->ds) == IDOK) {
						SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_DELETESTRING, (WPARAM)sel, 0);

						sel = SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_ADDSTRING, 0, (LPARAM)value->ds.name);
						SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_SETITEMDATA, sel, (LPARAM)value);

						SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_SETCURSEL, sel, 0);

						SendMessage(hwndDlg, WMU_ENABLE_LIST_BUTTONS, 0, 0);
						SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					}
				}
			}
		} else if ( HIWORD( wParam ) == BN_CLICKED ) {
			switch(LOWORD(wParam)) {
				case IDC_BTN_ADD:
					{
						DIListNode *value = (DIListNode *)malloc(sizeof(DIListNode));
						memset(value, 0, sizeof(DIListNode));
						if(DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_ITEM), hwndDlg, DlgProcAddItem, (LPARAM)&value->di) == IDOK) {
							int sel = SendDlgItemMessage(hwndDlg, IDC_LST_ITEMS, LB_GETCURSEL, 0, 0), index = sel + 1;							
							if(value->di.label[0] == 0)
								SendDlgItemMessage(hwndDlg, IDC_LST_ITEMS, LB_INSERTSTRING, index, (LPARAM)TranslateT("<No Label>"));	
							else
								SendDlgItemMessage(hwndDlg, IDC_LST_ITEMS, LB_INSERTSTRING, index, (LPARAM)value->di.label);	
							SendDlgItemMessage(hwndDlg, IDC_LST_ITEMS, LB_SETITEMDATA, index, (LPARAM)value);
							SendDlgItemMessage(hwndDlg, IDC_LST_ITEMS, LB_SETCURSEL, index, 0);
							SendMessage(hwndDlg, WMU_ENABLE_LIST_BUTTONS, 0, 0);
							SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
						}
						
					}
					return TRUE;
				case IDC_BTN_REMOVE:
					{
						int sel = SendDlgItemMessage(hwndDlg, IDC_LST_ITEMS, LB_GETCURSEL, 0, 0);
						if(sel != -1) {
							DIListNode *value = (DIListNode *)SendDlgItemMessage(hwndDlg, IDC_LST_ITEMS, LB_GETITEMDATA, sel, 0);
							SendDlgItemMessage(hwndDlg, IDC_LST_ITEMS, LB_DELETESTRING, (WPARAM)sel, 0);
							free(value);
							SendMessage(hwndDlg, WMU_ENABLE_LIST_BUTTONS, 0, 0);
							SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
						}
					}
					return TRUE;
				case IDC_BTN_UP:
					{
						DIListNode *value_up;
						int sel = SendDlgItemMessage(hwndDlg, IDC_LST_ITEMS, LB_GETCURSEL, 0, 0);
						if(sel > 0) {
							value_up = (DIListNode *)SendDlgItemMessage(hwndDlg, IDC_LST_ITEMS, LB_GETITEMDATA, sel - 1, 0);
							SendDlgItemMessage(hwndDlg, IDC_LST_ITEMS, LB_DELETESTRING, (WPARAM)sel - 1, 0);

							SendDlgItemMessage(hwndDlg, IDC_LST_ITEMS, LB_INSERTSTRING, sel, (LPARAM)value_up->di.label);
							SendDlgItemMessage(hwndDlg, IDC_LST_ITEMS, LB_SETITEMDATA, sel, (LPARAM)value_up);

							SendDlgItemMessage(hwndDlg, IDC_LST_ITEMS, LB_SETCURSEL, sel - 1, 0);
						}
					}
					SendMessage(hwndDlg, WMU_ENABLE_LIST_BUTTONS, 0, 0);
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					return TRUE;
				case IDC_BTN_DOWN:
					{
						DIListNode *value_down;
						int sel = SendDlgItemMessage(hwndDlg, IDC_LST_ITEMS, LB_GETCURSEL, 0, 0);
						int count = SendDlgItemMessage(hwndDlg, IDC_LST_ITEMS, LB_GETCOUNT, 0, 0);
						if(sel < count - 1) {
							value_down = (DIListNode *)SendDlgItemMessage(hwndDlg, IDC_LST_ITEMS, LB_GETITEMDATA, sel + 1, 0);
							SendDlgItemMessage(hwndDlg, IDC_LST_ITEMS, LB_DELETESTRING, (WPARAM)sel + 1, 0);

							SendDlgItemMessage(hwndDlg, IDC_LST_ITEMS, LB_INSERTSTRING, sel, (LPARAM)value_down->di.label);
							SendDlgItemMessage(hwndDlg, IDC_LST_ITEMS, LB_SETITEMDATA, sel, (LPARAM)value_down);

							SendDlgItemMessage(hwndDlg, IDC_LST_ITEMS, LB_SETCURSEL, sel + 1, 0);
						}
					}
					SendMessage(hwndDlg, WMU_ENABLE_LIST_BUTTONS, 0, 0);
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					return TRUE;
				case IDC_BTN_EDIT:
					{
						DIListNode *value;
						int sel = SendDlgItemMessage(hwndDlg, IDC_LST_ITEMS, LB_GETCURSEL, 0, 0);
						if(sel != -1) {
							value = (DIListNode *)SendDlgItemMessage(hwndDlg, IDC_LST_ITEMS, LB_GETITEMDATA, sel, 0);
							if(DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_ITEM), hwndDlg, DlgProcAddItem, (LPARAM)&value->di) == IDOK) {
								SendDlgItemMessage(hwndDlg, IDC_LST_ITEMS, LB_DELETESTRING, (WPARAM)sel, 0);

								if(value->di.label[0] == 0)
									SendDlgItemMessage(hwndDlg, IDC_LST_ITEMS, LB_INSERTSTRING, sel, (LPARAM)TranslateT("<No Label>"));	
								else
									SendDlgItemMessage(hwndDlg, IDC_LST_ITEMS, LB_INSERTSTRING, sel, (LPARAM)value->di.label);
								SendDlgItemMessage(hwndDlg, IDC_LST_ITEMS, LB_SETITEMDATA, sel, (LPARAM)value);

								SendDlgItemMessage(hwndDlg, IDC_LST_ITEMS, LB_SETCURSEL, sel, 0);

								SendMessage(hwndDlg, WMU_ENABLE_LIST_BUTTONS, 0, 0);
								SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
							}
						}
					}
					return TRUE;

				case IDC_BTN_ADD2:
					{
						DSListNode *value = (DSListNode *)malloc(sizeof(DSListNode));
						memset(value, 0, sizeof(DSListNode));
						if(DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_SUBST), hwndDlg, DlgProcAddSubst, (LPARAM)&value->ds) == IDOK) {
							int index = SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_ADDSTRING, 0, (LPARAM)value->ds.name);
							SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_SETITEMDATA, index, (LPARAM)value);
							SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_SETCURSEL, index, 0);
							SendMessage(hwndDlg, WMU_ENABLE_LIST_BUTTONS, 0, 0);
							SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
						}
						
					}
					return TRUE;
				case IDC_BTN_REMOVE2:
					{
						int sel = SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_GETCURSEL, 0, 0);
						if(sel != -1) {
							DSListNode *value = (DSListNode *)SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_GETITEMDATA, sel, 0);
							SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_DELETESTRING, (WPARAM)sel, 0);
							free(value);
							SendMessage(hwndDlg, WMU_ENABLE_LIST_BUTTONS, 0, 0);
							SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
						}
					}
					return TRUE;
				case IDC_BTN_EDIT2:
					{
						DSListNode *value;
						int sel = SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_GETCURSEL, 0, 0);
						if(sel != -1) {
							value = (DSListNode *)SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_GETITEMDATA, sel, 0);
							if(DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_SUBST), hwndDlg, DlgProcAddSubst, (LPARAM)&value->ds) == IDOK) {
								SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_DELETESTRING, (WPARAM)sel, 0);

								sel = SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_ADDSTRING, 0, (LPARAM)value->ds.name);
								SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_SETITEMDATA, sel, (LPARAM)value);

								SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_SETCURSEL, sel, 0);

								SendMessage(hwndDlg, WMU_ENABLE_LIST_BUTTONS, 0, 0);
								SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
							}
						}
					}
					return TRUE;
				default:
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					return TRUE;
			}
		}
		break;
	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == (unsigned)PSN_APPLY ) {
			DIListNode *di_node;
			int i;
			while(options.di_list) {
				di_node = options.di_list;
				options.di_list = options.di_list->next;
				free(di_node);
			}

			DIListNode *di_value;
			options.di_count = SendDlgItemMessage(hwndDlg, IDC_LST_ITEMS, LB_GETCOUNT, 0, 0);
			for(i = options.di_count - 1; i >= 0; i--) {
				di_node = (DIListNode *)SendDlgItemMessage(hwndDlg, IDC_LST_ITEMS, LB_GETITEMDATA, i, 0);
				di_value = (DIListNode *)malloc(sizeof(DIListNode));
				*di_value = *di_node;

				di_value->next = options.di_list;
				options.di_list = di_value;
			}

			DSListNode *ds_node;
			while(options.ds_list) {
				ds_node = options.ds_list;
				options.ds_list = options.ds_list->next;
				free(ds_node);
			}

			DSListNode *ds_value;
			options.ds_count = SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_GETCOUNT, 0, 0);
			for(i = options.ds_count - 1; i >= 0; i--) {
				ds_node = (DSListNode *)SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_GETITEMDATA, i, 0);
				ds_value = (DSListNode *)malloc(sizeof(DSListNode));
				*ds_value = *ds_node;

				ds_value->next = options.ds_list;
				options.ds_list = ds_value;
			}

			SaveOptions();
			return TRUE;
		}
		break;
	case WM_DESTROY:
		{
			DIListNode *di_value;
			int i;
			int count = SendDlgItemMessage(hwndDlg, IDC_LST_ITEMS, LB_GETCOUNT, 0, 0);
			for(i = 0; i < count; i++) {
				di_value = (DIListNode *)SendDlgItemMessage(hwndDlg, IDC_LST_ITEMS, LB_GETITEMDATA, i, 0);
				free(di_value);
			}
			DSListNode *ds_value;
			count = SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_GETCOUNT, 0, 0);
			for(i = 0; i < count; i++) {
				ds_value = (DSListNode *)SendDlgItemMessage(hwndDlg, IDC_LST_SUBST, LB_GETITEMDATA, i, 0);
				free(ds_value);
			}
		}
		break;
	}

	return 0;
}

static INT_PTR CALLBACK DlgProcOptWindow(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {

	switch ( msg ) {
	case WM_INITDIALOG:
		TranslateDialogDefault( hwndDlg );
		SendDlgItemMessage(hwndDlg, IDC_SPIN_WIDTH, UDM_SETRANGE, 0, (LPARAM)MAKELONG(2048, 16));
		SendDlgItemMessage(hwndDlg, IDC_SPIN_MINWIDTH, UDM_SETRANGE, 0, (LPARAM)MAKELONG(2048, 16));
		SendDlgItemMessage(hwndDlg, IDC_SPIN_MAXHEIGHT, UDM_SETRANGE, 0, (LPARAM)MAKELONG(2048, 16));
		SendDlgItemMessage(hwndDlg, IDC_SPIN_MINHEIGHT, UDM_SETRANGE, 0, (LPARAM)MAKELONG(2048, 16));
		SetDlgItemInt(hwndDlg, IDC_ED_WIDTH, options.win_width, FALSE);
		SetDlgItemInt(hwndDlg, IDC_ED_MAXHEIGHT, options.win_max_height, FALSE);
		SetDlgItemInt(hwndDlg, IDC_ED_MINWIDTH, options.min_width, FALSE);
		SetDlgItemInt(hwndDlg, IDC_ED_MINHEIGHT, options.min_height, FALSE);

		SetDlgItemInt(hwndDlg, IDC_ED_TRANS, options.opacity, FALSE);
		CheckDlgButton(hwndDlg, IDC_CHK_TRANSBG, options.trans_bg);

		CheckDlgButton(hwndDlg, IDC_CHK_NOFOCUS, options.show_no_focus ? TRUE : FALSE);
		CheckDlgButton(hwndDlg, IDC_CHK_SBAR, options.status_bar_tips ? TRUE : FALSE);
		CheckDlgButton(hwndDlg, IDC_CHK_BORDER, options.border);
		CheckDlgButton(hwndDlg, IDC_CHK_ROUNDCORNERS, options.round);
		CheckDlgButton(hwndDlg, IDC_CHK_ROUNDCORNERSAV, options.av_round);

		CheckDlgButton(hwndDlg, IDC_CHK_ANIMATE, options.animate);
		CheckDlgButton(hwndDlg, IDC_CHK_SHADOW, options.drop_shadow);

		SendDlgItemMessage(hwndDlg, IDC_SPIN_HOVER, UDM_SETRANGE, 0, (LPARAM)MAKELONG(5000, 5));
		SetDlgItemInt(hwndDlg, IDC_ED_HOVER, options.time_in, FALSE);
		break;
	case WM_COMMAND:
		if ( HIWORD( wParam ) == CBN_SELCHANGE) {
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		} else if ( HIWORD( wParam ) == EN_CHANGE && ( HWND )lParam == GetFocus()) {
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		} else if ( HIWORD( wParam ) == BN_CLICKED ) {
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		}
		break;
	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == (unsigned)PSN_APPLY ) {
			BOOL trans;
			int new_val;
			new_val = GetDlgItemInt(hwndDlg, IDC_ED_WIDTH, &trans, FALSE);
			if(trans) options.win_width = new_val;
			new_val = GetDlgItemInt(hwndDlg, IDC_ED_MINWIDTH, &trans, FALSE);
			if(trans) options.min_width = new_val;
			new_val = GetDlgItemInt(hwndDlg, IDC_ED_MAXHEIGHT, &trans, FALSE);
			if(trans) options.win_max_height = new_val;
			new_val = GetDlgItemInt(hwndDlg, IDC_ED_MINHEIGHT, &trans, FALSE);
			if(trans) options.min_height = new_val;

			new_val = GetDlgItemInt(hwndDlg, IDC_ED_TRANS, &trans, FALSE);
			if(trans) options.opacity = new_val;			
			options.trans_bg = IsDlgButtonChecked(hwndDlg, IDC_CHK_TRANSBG) ? true : false;

			new_val = GetDlgItemInt(hwndDlg, IDC_ED_HOVER, &trans, FALSE);
			if(trans) options.time_in = new_val;

			options.border = IsDlgButtonChecked(hwndDlg, IDC_CHK_BORDER) && IsWindowEnabled(GetDlgItem(hwndDlg, IDC_CHK_BORDER)) ? true : false;
			options.round = IsDlgButtonChecked(hwndDlg, IDC_CHK_ROUNDCORNERS) && IsWindowEnabled(GetDlgItem(hwndDlg, IDC_CHK_ROUNDCORNERS))  ? true : false;
			options.av_round = IsDlgButtonChecked(hwndDlg, IDC_CHK_ROUNDCORNERSAV) && IsWindowEnabled(GetDlgItem(hwndDlg, IDC_CHK_ROUNDCORNERSAV))  ? true : false;
			options.animate = IsDlgButtonChecked(hwndDlg, IDC_CHK_ANIMATE) ? true : false;
			options.drop_shadow = IsDlgButtonChecked(hwndDlg, IDC_CHK_SHADOW) ? true : false;

			options.show_no_focus = IsDlgButtonChecked(hwndDlg, IDC_CHK_NOFOCUS) ? true : false;
			options.status_bar_tips = IsDlgButtonChecked(hwndDlg, IDC_CHK_SBAR) ? true : false;

			SaveOptions();
			return TRUE;
		}
		break;
	}
	return 0;
}

static INT_PTR CALLBACK DlgProcOptLayout(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {

	switch ( msg ) {
	case WM_INITDIALOG:
		TranslateDialogDefault( hwndDlg );

		SendDlgItemMessage(hwndDlg, IDC_CMB_ICON, CB_ADDSTRING, 0, (LPARAM)TranslateT("Icon on left"));
		SendDlgItemMessage(hwndDlg, IDC_CMB_ICON, CB_ADDSTRING, 0, (LPARAM)TranslateT("Icon on right"));
		SendDlgItemMessage(hwndDlg, IDC_CMB_ICON, CB_ADDSTRING, 0, (LPARAM)TranslateT("No icon"));
		SendDlgItemMessage(hwndDlg, IDC_CMB_ICON, CB_ADDSTRING, 0, (LPARAM)TranslateT("No title"));
		SendDlgItemMessage(hwndDlg, IDC_CMB_ICON, CB_SETCURSEL, (int)options.title_layout, 0);

		SendDlgItemMessage(hwndDlg, IDC_CMB_POS, CB_ADDSTRING, 0, (LPARAM)TranslateT("Bottom right"));
		SendDlgItemMessage(hwndDlg, IDC_CMB_POS, CB_ADDSTRING, 0, (LPARAM)TranslateT("Bottom left"));
		SendDlgItemMessage(hwndDlg, IDC_CMB_POS, CB_ADDSTRING, 0, (LPARAM)TranslateT("Top right"));
		SendDlgItemMessage(hwndDlg, IDC_CMB_POS, CB_ADDSTRING, 0, (LPARAM)TranslateT("Top left"));
		SendDlgItemMessage(hwndDlg, IDC_CMB_POS, CB_SETCURSEL, (int)options.pos, 0);

		SendDlgItemMessage(hwndDlg, IDC_CMB_LV, CB_ADDSTRING, 0, (LPARAM)TranslateT("Top"));
		SendDlgItemMessage(hwndDlg, IDC_CMB_LV, CB_ADDSTRING, 0, (LPARAM)TranslateT("Centre"));
		SendDlgItemMessage(hwndDlg, IDC_CMB_LV, CB_ADDSTRING, 0, (LPARAM)TranslateT("Bottom"));
		switch(options.label_valign) {
			case DT_TOP: SendDlgItemMessage(hwndDlg, IDC_CMB_LV, CB_SETCURSEL, 0, 0); break;
			case DT_VCENTER: SendDlgItemMessage(hwndDlg, IDC_CMB_LV, CB_SETCURSEL, 1, 0); break;
			case DT_BOTTOM: SendDlgItemMessage(hwndDlg, IDC_CMB_LV, CB_SETCURSEL, 2, 0); break;
		}
		
		SendDlgItemMessage(hwndDlg, IDC_CMB_VV, CB_ADDSTRING, 0, (LPARAM)TranslateT("Top"));
		SendDlgItemMessage(hwndDlg, IDC_CMB_VV, CB_ADDSTRING, 0, (LPARAM)TranslateT("Centre"));
		SendDlgItemMessage(hwndDlg, IDC_CMB_VV, CB_ADDSTRING, 0, (LPARAM)TranslateT("Bottom"));
		switch(options.value_valign) {
			case DT_TOP: SendDlgItemMessage(hwndDlg, IDC_CMB_VV, CB_SETCURSEL, 0, 0); break;
			case DT_VCENTER: SendDlgItemMessage(hwndDlg, IDC_CMB_VV, CB_SETCURSEL, 1, 0); break;
			case DT_BOTTOM: SendDlgItemMessage(hwndDlg, IDC_CMB_VV, CB_SETCURSEL, 2, 0); break;
		}
		
		SendDlgItemMessage(hwndDlg, IDC_CMB_LH, CB_ADDSTRING, 0, (LPARAM)TranslateT("Left"));
		SendDlgItemMessage(hwndDlg, IDC_CMB_LH, CB_ADDSTRING, 0, (LPARAM)TranslateT("Right"));
		switch(options.label_halign) {
			case DT_LEFT: SendDlgItemMessage(hwndDlg, IDC_CMB_LH, CB_SETCURSEL, 0, 0); break;
			case DT_RIGHT: SendDlgItemMessage(hwndDlg, IDC_CMB_LH, CB_SETCURSEL, 1, 0); break;
		}
		
		SendDlgItemMessage(hwndDlg, IDC_CMB_VH, CB_ADDSTRING, 0, (LPARAM)TranslateT("Left"));
		SendDlgItemMessage(hwndDlg, IDC_CMB_VH, CB_ADDSTRING, 0, (LPARAM)TranslateT("Right"));
		switch(options.value_halign) {
			case DT_LEFT: SendDlgItemMessage(hwndDlg, IDC_CMB_VH, CB_SETCURSEL, 0, 0); break;
			case DT_RIGHT: SendDlgItemMessage(hwndDlg, IDC_CMB_VH, CB_SETCURSEL, 1, 0); break;
		}

		SendDlgItemMessage(hwndDlg, IDC_CMB_AV, CB_ADDSTRING, 0, (LPARAM)TranslateT("No avatar"));
		if(ServiceExists(MS_AV_DRAWAVATAR)) {
			SendDlgItemMessage(hwndDlg, IDC_CMB_AV, CB_ADDSTRING, 0, (LPARAM)TranslateT("Left avatar"));
			SendDlgItemMessage(hwndDlg, IDC_CMB_AV, CB_ADDSTRING, 0, (LPARAM)TranslateT("Right avatar"));
		} else {
			HWND hw = GetDlgItem(hwndDlg, IDC_CMB_AV);
			EnableWindow(hw, FALSE);
			hw = GetDlgItem(hwndDlg, IDC_SPIN_AVSIZE);
			EnableWindow(hw, FALSE);
			hw = GetDlgItem(hwndDlg, IDC_ED_AVSIZE);
			EnableWindow(hw, FALSE);
		}
		SendDlgItemMessage(hwndDlg, IDC_CMB_AV, CB_SETCURSEL, (int)options.av_layout, 0);

		SendDlgItemMessage(hwndDlg, IDC_SPIN_TRANS, UDM_SETRANGE, 0, (LPARAM)MAKELONG(99, 0));
		SendDlgItemMessage(hwndDlg, IDC_SPIN_AVSIZE, UDM_SETRANGE, 0, (LPARAM)MAKELONG(100, 16));
		SendDlgItemMessage(hwndDlg, IDC_SPIN_INDENT, UDM_SETRANGE, 0, (LPARAM)MAKELONG(400, 0));
		SendDlgItemMessage(hwndDlg, IDC_SPIN_PADDING, UDM_SETRANGE, 0, (LPARAM)MAKELONG(128, 0));
		SendDlgItemMessage(hwndDlg, IDC_SPIN_TEXTPADDING, UDM_SETRANGE, 0, (LPARAM)MAKELONG(128, 0));
		SendDlgItemMessage(hwndDlg, IDC_SPIN_AVPADDING, UDM_SETRANGE, 0, (LPARAM)MAKELONG(128, 0));
		SendDlgItemMessage(hwndDlg, IDC_SPIN_SBWIDTH, UDM_SETRANGE, 0, (LPARAM)MAKELONG(2048, 0));

		CheckDlgButton(hwndDlg, IDC_CHK_NORESIZEAV, options.no_resize_av);
		if(options.no_resize_av) {
			HWND hw = GetDlgItem(hwndDlg, IDC_ED_AVSIZE);
			EnableWindow(hw, FALSE);
			hw = GetDlgItem(hwndDlg, IDC_SPIN_AVSIZE);
			EnableWindow(hw, FALSE);
		}
		SetDlgItemInt(hwndDlg, IDC_ED_AVSIZE, options.av_size, FALSE);
		SetDlgItemInt(hwndDlg, IDC_ED_INDENT, options.text_indent, FALSE);
		SetDlgItemInt(hwndDlg, IDC_ED_PADDING, options.padding, FALSE);
		SetDlgItemInt(hwndDlg, IDC_ED_TEXTPADDING, options.text_padding, FALSE);
		SetDlgItemInt(hwndDlg, IDC_ED_AVPADDING, options.av_padding, FALSE);
		SetDlgItemInt(hwndDlg, IDC_ED_SBWIDTH, options.sidebar_width, FALSE);

		SetDlgItemText(hwndDlg, IDC_ED_BGFN, options.bg_fn);
		CheckDlgButton(hwndDlg, IDC_CHK_STRETCHBG, options.stretch_bg_img);
		if(!ServiceExists(MS_IMG_LOAD)) {
			HWND hw = GetDlgItem(hwndDlg, IDC_ED_BGFN);
			EnableWindow(hw, FALSE);
			hw = GetDlgItem(hwndDlg, IDC_CHK_STRETCHBG);
			EnableWindow(hw, FALSE);
		}

		return FALSE;
	case WM_COMMAND:
		if ( HIWORD( wParam ) == CBN_SELCHANGE) {
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		} else if ( HIWORD( wParam ) == EN_CHANGE && ( HWND )lParam == GetFocus()) {
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		} else if ( HIWORD( wParam ) == BN_CLICKED ) {
			if(LOWORD(wParam) == IDC_BTN_BROWSE) {
				TCHAR tempBgFn[MAX_PATH];
				tempBgFn[0] = 0;
				OPENFILENAME ofn={0};
				ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
				ofn.hwndOwner = hwndDlg;
				ofn.lpstrFile = tempBgFn;
				ofn.lpstrFilter=_T("All Files (*.*)\0*.*\0\0");
				ofn.nFilterIndex = 1;
				ofn.nMaxFile = MAX_PATH;
				ofn.Flags=OFN_HIDEREADONLY;
				ofn.lpstrInitialDir = _T(".");
				ofn.lpstrDefExt=_T("");
				if (GetOpenFileName(&ofn)) {
					SetDlgItemText(hwndDlg, IDC_ED_BGFN, tempBgFn);
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				}
			} else {
				if(LOWORD(wParam) == IDC_CHK_NORESIZEAV) {
					bool checked = IsDlgButtonChecked(hwndDlg, IDC_CHK_NORESIZEAV) != BST_UNCHECKED;
					HWND hw = GetDlgItem(hwndDlg, IDC_ED_AVSIZE);
					EnableWindow(hw, !checked);
					hw = GetDlgItem(hwndDlg, IDC_SPIN_AVSIZE);
					EnableWindow(hw, !checked);
				}
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			}
		}
		break;
	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == (unsigned)PSN_APPLY ) {
			BOOL trans;
			int new_val;
			new_val = GetDlgItemInt(hwndDlg, IDC_ED_AVSIZE, &trans, FALSE);
			if(trans) options.av_size = new_val;
			new_val = GetDlgItemInt(hwndDlg, IDC_ED_INDENT, &trans, FALSE);
			if(trans) options.text_indent = new_val;
			new_val = GetDlgItemInt(hwndDlg, IDC_ED_PADDING, &trans, FALSE);
			if(trans) options.padding = new_val;
			new_val = GetDlgItemInt(hwndDlg, IDC_ED_TEXTPADDING, &trans, FALSE);
			if(trans) options.text_padding = new_val;
			new_val = GetDlgItemInt(hwndDlg, IDC_ED_AVPADDING, &trans, FALSE);
			if(trans) options.av_padding = new_val;
			new_val = GetDlgItemInt(hwndDlg, IDC_ED_SBWIDTH, &trans, FALSE);
			if(trans) options.sidebar_width = new_val;

			options.title_layout = (PopupTitleLayout)SendDlgItemMessage(hwndDlg, IDC_CMB_ICON, CB_GETCURSEL, 0, 0);
			options.av_layout = (PopupAvLayout)SendDlgItemMessage(hwndDlg, IDC_CMB_AV, CB_GETCURSEL, 0, 0);
			options.pos = (PopupPosition)SendDlgItemMessage(hwndDlg, IDC_CMB_POS, CB_GETCURSEL, 0, 0);
			
			switch(SendDlgItemMessage(hwndDlg, IDC_CMB_LV, CB_GETCURSEL, 0, 0)) {
				case 0: options.label_valign = DT_TOP; break;
				case 1: options.label_valign = DT_VCENTER; break;
				case 2: options.label_valign = DT_BOTTOM; break;
			}
			switch(SendDlgItemMessage(hwndDlg, IDC_CMB_VV, CB_GETCURSEL, 0, 0)) {
				case 0: options.value_valign = DT_TOP; break;
				case 1: options.value_valign = DT_VCENTER; break;
				case 2: options.value_valign = DT_BOTTOM; break;
			}

			switch(SendDlgItemMessage(hwndDlg, IDC_CMB_LH, CB_GETCURSEL, 0, 0)) {
				case 0: options.label_halign = DT_LEFT; break;
				case 1: options.label_halign = DT_RIGHT; break;
			}
			switch(SendDlgItemMessage(hwndDlg, IDC_CMB_VH, CB_GETCURSEL, 0, 0)) {
				case 0: options.value_halign = DT_LEFT; break;
				case 1: options.value_halign = DT_RIGHT; break;
			}

			options.no_resize_av = IsDlgButtonChecked(hwndDlg, IDC_CHK_NORESIZEAV) ? true : false;
			GetDlgItemText(hwndDlg, IDC_ED_BGFN, options.bg_fn, MAX_PATH);
			options.stretch_bg_img = IsDlgButtonChecked(hwndDlg, IDC_CHK_STRETCHBG) ? true : false;

			SaveOptions();
			return TRUE;
		}
		break;
	}

	return 0;
}

int OptInit(WPARAM wParam, LPARAM lParam) {
	OPTIONSDIALOGPAGE odp = { 0 };

	odp.cbSize						= sizeof(odp);

	odp.flags						= ODPF_BOLDGROUPS;
	odp.position					= -790000000;
	odp.hInstance					= hInst;
	odp.pszTitle					= LPGEN("Tooltips");
	odp.pszGroup					= LPGEN("Customize");

	odp.pszTemplate					= MAKEINTRESOURCEA(IDD_OPT_WINDOW);
	odp.pszTab						= LPGEN("Window");
	odp.pfnDlgProc					= DlgProcOptWindow;
	CallService( MS_OPT_ADDPAGE, wParam,( LPARAM )&odp );

	odp.pszTemplate					= MAKEINTRESOURCEA(IDD_OPT_LAYOUT);
	odp.pszTab						= LPGEN("Layout");
	odp.pfnDlgProc					= DlgProcOptLayout;
	CallService( MS_OPT_ADDPAGE, wParam,( LPARAM )&odp );

	odp.pszTemplate					= MAKEINTRESOURCEA(IDD_OPT_CONTENT);
	odp.pszTab						= LPGEN("Content");
	odp.pfnDlgProc					= DlgProcOptContent;
	CallService( MS_OPT_ADDPAGE, wParam,( LPARAM )&odp );

	return 0;
}

HANDLE hEventOptInit;

void InitOptions() {
	hEventOptInit = HookEvent(ME_OPT_INITIALISE, OptInit);
}

void DeinitOptions() {
	UnhookEvent(hEventOptInit);

	DIListNode *di_node = options.di_list;
	while(options.di_list) {
		di_node = options.di_list;
		options.di_list = options.di_list->next;
		free(di_node);
	}

	DSListNode *ds_node = options.ds_list;
	while(options.ds_list) {
		ds_node = options.ds_list;
		options.ds_list = options.ds_list->next;
		free(ds_node);
	}
}
