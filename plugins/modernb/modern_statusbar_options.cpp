/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2008 Miranda ICQ/IM project, 
all portions of this codebase are copyrighted to the people 
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#include "hdr/modern_commonheaders.h"
#include "m_clc.h"
#include "hdr/modern_clc.h"
#include "hdr/modern_commonprototypes.h"
#include "hdr/modern_defsettings.h"
//#include "hdr/modern_effectenum.h"

typedef struct _StatusBarProtocolOptions
{
	char *szName;
	BOOL AccountIsCustomized;
	BOOL HideAccount;
	BYTE SBarShow;
	BYTE SBarRightClk;
	BYTE UseConnectingIcon;
	BYTE ShowUnreadEmails;
	BYTE ShowXStatus;
	int PaddingLeft;
	int PaddingRight;
} StatusBarProtocolOptions;

static StatusBarProtocolOptions _GlobalOptions = {0};

static void UpdateXStatusIconOptions(HWND hwndDlg, BOOL perProto, StatusBarProtocolOptions* dat, int curSelProto)
{
	int en=IsDlgButtonChecked(hwndDlg,IDC_SHOWSBAR) && IsDlgButtonChecked(hwndDlg,IDC_SHOWICON );

	if (IsDlgButtonChecked(hwndDlg,IDC_SHOWBOTH)) CheckDlgButton(hwndDlg,IDC_SHOWNORMAL,FALSE);
	EnableWindow(GetDlgItem(hwndDlg,IDC_SHOWBOTH),en && IsDlgButtonChecked(hwndDlg,IDC_SHOWXSTATUS) && !IsDlgButtonChecked(hwndDlg,IDC_SHOWNORMAL));

	if (IsDlgButtonChecked(hwndDlg,IDC_SHOWNORMAL))	CheckDlgButton(hwndDlg,IDC_SHOWBOTH,FALSE);
	EnableWindow(GetDlgItem(hwndDlg,IDC_SHOWNORMAL),en && IsDlgButtonChecked(hwndDlg,IDC_SHOWXSTATUS)&& !IsDlgButtonChecked(hwndDlg,IDC_SHOWBOTH));

	EnableWindow(GetDlgItem(hwndDlg,IDC_TRANSPARENTOVERLAY),en && IsDlgButtonChecked(hwndDlg,IDC_SHOWXSTATUS) && IsDlgButtonChecked(hwndDlg,IDC_SHOWNORMAL)&& !IsDlgButtonChecked(hwndDlg,IDC_SHOWBOTH));

	{
		BYTE val = 0;
		if (IsDlgButtonChecked(hwndDlg,IDC_SHOWXSTATUS))
		{
			if (IsDlgButtonChecked(hwndDlg,IDC_SHOWBOTH)) val=3;
			else if (IsDlgButtonChecked(hwndDlg,IDC_SHOWNORMAL)) val=2;
			else val=1;
			val+=IsDlgButtonChecked(hwndDlg,IDC_TRANSPARENTOVERLAY)?4:0;
		}
		val+=IsDlgButtonChecked(hwndDlg,IDC_SHOWXSTATUSNAME)?8:0;
		if (perProto)
		{
			dat[curSelProto].ShowXStatus = val;
		}
		else
		{
			_GlobalOptions.ShowXStatus = val;
		}
	}
}

static void UpdateStatusBarOptionsDisplay(HWND hwndDlg)
{
	StatusBarProtocolOptions *dat = (StatusBarProtocolOptions*)GetWindowLongPtr(GetDlgItem(hwndDlg,IDC_STATUSBAR_PROTO_LIST),GWLP_USERDATA);
	BOOL perProto = (BOOL)IsDlgButtonChecked(hwndDlg,IDC_STATUSBAR_PER_PROTO);
	HWND hwndComboBox = GetDlgItem( hwndDlg, IDC_STATUSBAR_PROTO_LIST );
	StatusBarProtocolOptions sbpo;
	int curSelProto = SendMessage(hwndComboBox, CB_GETCURSEL, 0, 0) - 1; //first entry is the combo box is a constant.

	if (curSelProto < 0)
		perProto = FALSE;

	if (perProto)
	{
		sbpo = dat[curSelProto];
	}
	else
	{
		sbpo = _GlobalOptions;
	}

	if (perProto)
	{
		EnableWindow(GetDlgItem(hwndDlg, IDC_SBAR_USE_ACCOUNT_SETTINGS), TRUE);
		CheckDlgButton(hwndDlg, IDC_SBAR_USE_ACCOUNT_SETTINGS, sbpo.AccountIsCustomized ? BST_CHECKED : BST_UNCHECKED);
	}
	else
	{
		EnableWindow(GetDlgItem(hwndDlg, IDC_SBAR_USE_ACCOUNT_SETTINGS), FALSE);
	}

	CheckDlgButton(hwndDlg, IDC_SBAR_HIDE_ACCOUNT_COMPLETELY, sbpo.HideAccount ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwndDlg, IDC_USECONNECTINGICON, sbpo.UseConnectingIcon ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwndDlg, IDC_SHOWXSTATUSNAME, ((sbpo.ShowXStatus&8)>0) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwndDlg, IDC_SHOWXSTATUS, ((sbpo.ShowXStatus&3)>0) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwndDlg, IDC_SHOWNORMAL, ((sbpo.ShowXStatus&3)==2) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwndDlg, IDC_SHOWBOTH, ((sbpo.ShowXStatus&3)==3) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwndDlg, IDC_SHOWUNREADEMAIL, (sbpo.ShowUnreadEmails==1) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwndDlg, IDC_TRANSPARENTOVERLAY, ((sbpo.ShowXStatus&4)) ? BST_CHECKED : BST_UNCHECKED);
	{
		BYTE showOpts=sbpo.SBarShow;
		CheckDlgButton(hwndDlg, IDC_SHOWICON, showOpts&1 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SHOWPROTO, showOpts&2 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SHOWSTATUS, showOpts&4 ? BST_CHECKED : BST_UNCHECKED);
	}
	CheckDlgButton(hwndDlg, IDC_RIGHTSTATUS, sbpo.SBarRightClk ? BST_UNCHECKED : BST_CHECKED);
	CheckDlgButton(hwndDlg, IDC_RIGHTMIRANDA, !IsDlgButtonChecked(hwndDlg,IDC_RIGHTSTATUS) ? BST_CHECKED : BST_UNCHECKED);

	SendDlgItemMessage(hwndDlg,IDC_OFFSETSPIN_LEFT,UDM_SETRANGE,0,MAKELONG(50,0));
	SendDlgItemMessage(hwndDlg,IDC_OFFSETSPIN_LEFT,UDM_SETPOS,0,MAKELONG(sbpo.PaddingLeft,2));

	SendDlgItemMessage(hwndDlg,IDC_OFFSETSPIN_RIGHT,UDM_SETRANGE,0,MAKELONG(50,0));
	SendDlgItemMessage(hwndDlg,IDC_OFFSETSPIN_RIGHT,UDM_SETPOS,0,MAKELONG(sbpo.PaddingRight,2));

	if (!sbpo.AccountIsCustomized)
		UpdateXStatusIconOptions(hwndDlg, perProto, dat, curSelProto);

	{
		BOOL enableIcons = IsDlgButtonChecked(hwndDlg,IDC_SHOWICON );
		BOOL enableOptions = !perProto || sbpo.AccountIsCustomized;
		EnableWindow(GetDlgItem(hwndDlg, IDC_SBAR_HIDE_ACCOUNT_COMPLETELY), enableOptions && perProto);
		EnableWindow(GetDlgItem(hwndDlg, IDC_USECONNECTINGICON), enableOptions && enableIcons);
		EnableWindow(GetDlgItem(hwndDlg, IDC_SHOWXSTATUSNAME), enableOptions);
		EnableWindow(GetDlgItem(hwndDlg, IDC_SHOWXSTATUS), enableOptions && enableIcons);

		if (!enableOptions)
		{
			EnableWindow(GetDlgItem(hwndDlg, IDC_SHOWNORMAL), enableOptions && enableIcons);
			EnableWindow(GetDlgItem(hwndDlg, IDC_SHOWBOTH), enableOptions && enableIcons );
			EnableWindow(GetDlgItem(hwndDlg, IDC_TRANSPARENTOVERLAY), enableOptions && enableIcons);
		}

		EnableWindow(GetDlgItem(hwndDlg, IDC_SHOWUNREADEMAIL), enableOptions);
		EnableWindow(GetDlgItem(hwndDlg, IDC_SHOWICON), enableOptions);
		EnableWindow(GetDlgItem(hwndDlg, IDC_SHOWPROTO), enableOptions);
		EnableWindow(GetDlgItem(hwndDlg, IDC_SHOWSTATUS), enableOptions);
		EnableWindow(GetDlgItem(hwndDlg, IDC_RIGHTSTATUS), enableOptions);
		EnableWindow(GetDlgItem(hwndDlg, IDC_RIGHTMIRANDA), enableOptions);
		EnableWindow(GetDlgItem(hwndDlg, IDC_OFFSETICON_LEFT), enableOptions);
		EnableWindow(GetDlgItem(hwndDlg, IDC_OFFSETSPIN_LEFT), enableOptions);
		EnableWindow(GetDlgItem(hwndDlg, IDC_OFFSETICON_RIGHT), enableOptions);
		EnableWindow(GetDlgItem(hwndDlg, IDC_OFFSETSPIN_RIGHT), enableOptions);
	}

	if (!perProto || sbpo.AccountIsCustomized)
		UpdateXStatusIconOptions(hwndDlg, perProto, dat, curSelProto);
}

 INT_PTR CALLBACK DlgProcSBarOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	StatusBarProtocolOptions *dat = (StatusBarProtocolOptions*)GetWindowLongPtr(GetDlgItem(hwndDlg,IDC_STATUSBAR_PROTO_LIST),GWLP_USERDATA);
	LOGFONTA lf;
	BOOL perProto = IsDlgButtonChecked(hwndDlg, IDC_STATUSBAR_PER_PROTO);
	HWND hwndComboBox = GetDlgItem( hwndDlg, IDC_STATUSBAR_PROTO_LIST );
	int curSelProto = SendMessage(hwndComboBox, CB_GETCURSEL, 0, 0) - 1; //first entry in the combo box is a constant

	if (curSelProto < 0)
		perProto = FALSE;

	switch (msg)
	{
	case WM_DESTROY:
		mir_free(dat);
		break;
	case WM_INITDIALOG:
		{
			perProto = (BOOL)ModernGetSettingByte(NULL,"CLUI","SBarPerProto",SETTING_SBARPERPROTO_DEFAULT);

			TranslateDialogDefault(hwndDlg);

			CheckDlgButton(hwndDlg, IDC_SHOWSBAR, ModernGetSettingByte(NULL,"CLUI","ShowSBar",SETTING_SHOWSBAR_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_STATUSBAR_PER_PROTO, perProto ? BST_CHECKED : BST_UNCHECKED);


			{ // populate per-proto list box.
				char *szName;
				char buf[256];
				int i,count;

				SendMessage(hwndComboBox, CB_RESETCONTENT, 0, 0);

				PROTOACCOUNT **accs;
				ProtoEnumAccounts( &count, &accs );

				dat = (StatusBarProtocolOptions*)mir_alloc(sizeof(StatusBarProtocolOptions)*count);
				SetWindowLongPtr(GetDlgItem(hwndDlg,IDC_STATUSBAR_PROTO_LIST),GWLP_USERDATA,(LONG_PTR)dat);

				SendMessage(hwndComboBox, CB_ADDSTRING, 0, (LPARAM)TranslateT( "<<Global>>" ));

				for ( i = 0; i < count; i++ )
				{
					szName = accs[i]->szProtoName;
					dat[i].szName = szName;

#ifdef UNICODE
					SendMessage(hwndComboBox, CB_ADDSTRING, 0, (LPARAM)accs[i]->tszAccountName);
#else
					SendMessage(hwndComboBox, CB_ADDSTRING, 0, (LPARAM)accs[i]->tszAccountName);
#endif

					mir_snprintf(buf, SIZEOF(buf), "SBarAccountIsCustom_%s", szName);
					dat[i].AccountIsCustomized = ModernGetSettingByte(NULL,"CLUI", buf, SETTING_SBARACCOUNTISCUSTOM_DEFAULT);

					mir_snprintf(buf, SIZEOF(buf), "HideAccount_%s", szName);
					dat[i].HideAccount = ModernGetSettingByte(NULL,"CLUI", buf, SETTING_SBARHIDEACCOUNT_DEFAULT);

					mir_snprintf(buf, SIZEOF(buf), "SBarShow_%s", szName);
					dat[i].SBarShow = ModernGetSettingByte(NULL,"CLUI", buf, SETTING_SBARSHOW_DEFAULT);

					mir_snprintf(buf, SIZEOF(buf), "SBarRightClk_%s", szName);
					dat[i].SBarRightClk = ModernGetSettingByte(NULL,"CLUI", buf, SETTING_SBARRIGHTCLK_DEFAULT);

					mir_snprintf(buf, SIZEOF(buf), "ShowUnreadEmails_%s", szName);
					dat[i].ShowUnreadEmails = ModernGetSettingByte(NULL,"CLUI", buf, SETTING_SHOWUNREADEMAILS_DEFAULT);

					mir_snprintf(buf, SIZEOF(buf), "ShowXStatus_%s", szName);
					dat[i].ShowXStatus = ModernGetSettingByte(NULL,"CLUI", buf, SETTING_SHOWXSTATUS_DEFAULT);

					mir_snprintf(buf, SIZEOF(buf), "UseConnectingIcon_%s", szName);
					dat[i].UseConnectingIcon = ModernGetSettingByte(NULL,"CLUI", buf, SETTING_USECONNECTINGICON_DEFAULT);

					mir_snprintf(buf, SIZEOF(buf), "PaddingLeft_%s", szName);
					dat[i].PaddingLeft = ModernGetSettingDword(NULL,"CLUI", buf, SETTING_PADDINGLEFT_DEFAULT);

					mir_snprintf(buf, SIZEOF(buf), "PaddingRight_%s", szName);
					dat[i].PaddingRight = ModernGetSettingDword(NULL,"CLUI", buf, SETTING_PADDINGRIGHT_DEFAULT);
				}

				if (count)
				{
					SendMessage(hwndComboBox, CB_SETCURSEL, 0, 0);
				}
			}

			_GlobalOptions.AccountIsCustomized = TRUE;
			_GlobalOptions.SBarRightClk = ModernGetSettingByte(NULL,"CLUI", "SBarRightClk", SETTING_SBARRIGHTCLK_DEFAULT);
			_GlobalOptions.ShowUnreadEmails = ModernGetSettingByte(NULL,"CLUI", "ShowUnreadEmails", SETTING_SHOWUNREADEMAILS_DEFAULT);
			_GlobalOptions.ShowXStatus = ModernGetSettingByte(NULL,"CLUI", "ShowXStatus", SETTING_SHOWXSTATUS_DEFAULT);
			_GlobalOptions.UseConnectingIcon = ModernGetSettingByte(NULL,"CLUI", "UseConnectingIcon", SETTING_USECONNECTINGICON_DEFAULT);
			_GlobalOptions.SBarShow = ModernGetSettingByte(NULL,"CLUI","SBarShow",SETTING_SBARSHOW_DEFAULT);

			CheckDlgButton(hwndDlg, IDC_EQUALSECTIONS, ModernGetSettingByte(NULL,"CLUI","EqualSections",SETTING_EQUALSECTIONS_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);

			SendDlgItemMessage(hwndDlg,IDC_MULTI_SPIN,UDM_SETRANGE,0,MAKELONG(50,0));
			SendDlgItemMessage(hwndDlg,IDC_MULTI_SPIN,UDM_SETPOS,0,MAKELONG(ModernGetSettingByte(NULL,"CLUI","StatusBarProtosPerLine",SETTING_PROTOSPERLINE_DEFAULT),0));

			SendDlgItemMessage(hwndDlg,IDC_OFFSETSPIN,UDM_SETRANGE,0,MAKELONG(50,0));
			SendDlgItemMessage(hwndDlg,IDC_OFFSETSPIN,UDM_SETPOS,0,MAKELONG(ModernGetSettingDword(NULL,"CLUI","LeftOffset",SETTING_LEFTOFFSET_DEFAULT),0));

			SendDlgItemMessage(hwndDlg,IDC_OFFSETSPIN2,UDM_SETRANGE,0,MAKELONG(50,0));
			SendDlgItemMessage(hwndDlg,IDC_OFFSETSPIN2,UDM_SETPOS,0,MAKELONG(ModernGetSettingDword(NULL,"CLUI","RightOffset",SETTING_RIGHTOFFSET_DEFAULT),0));

			SendDlgItemMessage(hwndDlg,IDC_SBAR_BORDER_TOP_SPIN,UDM_SETRANGE,0,MAKELONG(50,0));
			SendDlgItemMessage(hwndDlg,IDC_SBAR_BORDER_TOP_SPIN,UDM_SETPOS,0,MAKELONG(ModernGetSettingDword(NULL,"CLUI","TopOffset",SETTING_TOPOFFSET_DEFAULT),0));

			SendDlgItemMessage(hwndDlg,IDC_SBAR_BORDER_BOTTOM_SPIN,UDM_SETRANGE,0,MAKELONG(50,0));
			SendDlgItemMessage(hwndDlg,IDC_SBAR_BORDER_BOTTOM_SPIN,UDM_SETPOS,0,MAKELONG(ModernGetSettingDword(NULL,"CLUI","BottomOffset",SETTING_BOTTOMOFFSET_DEFAULT),0));

			SendDlgItemMessage(hwndDlg,IDC_OFFSETSPIN3,UDM_SETRANGE,0,MAKELONG(50,0));
			SendDlgItemMessage(hwndDlg,IDC_OFFSETSPIN3,UDM_SETPOS,0,MAKELONG(ModernGetSettingDword(NULL,"CLUI","SpaceBetween",SETTING_SPACEBETWEEN_DEFAULT),2));

			{
				int i, item;
				TCHAR *align[]={_T("Left"), _T("Center"), _T("Right")};
				for (i=0; i<SIZEOF(align); i++) {
					item=SendDlgItemMessage(hwndDlg,IDC_SBAR_HORIZ_ALIGN,CB_ADDSTRING,0,(LPARAM)TranslateTS(align[i]));
				}

				SendDlgItemMessage(hwndDlg, IDC_SBAR_HORIZ_ALIGN, CB_SETCURSEL, ModernGetSettingByte(NULL, "CLUI", "Align", SETTING_ALIGN_DEFAULT), 0);
			}

			{
				int i, item;
				TCHAR *align[]={_T("Top"), _T("Center"), _T("Bottom")};
				for (i=0; i<SIZEOF(align); i++) {
					item=SendDlgItemMessage(hwndDlg,IDC_SBAR_VERT_ALIGN,CB_ADDSTRING,0,(LPARAM)TranslateTS(align[i]));
				}

				SendDlgItemMessage(hwndDlg, IDC_SBAR_VERT_ALIGN, CB_SETCURSEL, ModernGetSettingByte(NULL, "CLUI", "VAlign", SETTING_VALIGN_DEFAULT), 0);
			}

			{
				int en=IsDlgButtonChecked(hwndDlg,IDC_SHOWSBAR);
				int en_icons = IsDlgButtonChecked(hwndDlg,IDC_SHOWICON );

				EnableWindow(GetDlgItem(hwndDlg,IDC_SHOWICON),en);
				EnableWindow(GetDlgItem(hwndDlg,IDC_SHOWPROTO),en);
				EnableWindow(GetDlgItem(hwndDlg,IDC_SHOWSTATUS),en);
				EnableWindow(GetDlgItem(hwndDlg,IDC_RIGHTSTATUS),en);
				EnableWindow(GetDlgItem(hwndDlg,IDC_RIGHTMIRANDA),en);
				EnableWindow(GetDlgItem(hwndDlg,IDC_EQUALSECTIONS),en);
				EnableWindow(GetDlgItem(hwndDlg,IDC_USECONNECTINGICON),en && en_icons);
				EnableWindow(GetDlgItem(hwndDlg,IDC_USEOWNERDRAW),en);
				EnableWindow(GetDlgItem(hwndDlg,IDC_OFFSETSPIN),en);
				EnableWindow(GetDlgItem(hwndDlg,IDC_OFFSETICON),en);
				EnableWindow(GetDlgItem(hwndDlg,IDC_OFFSETSPIN2),en);
				EnableWindow(GetDlgItem(hwndDlg,IDC_OFFSETICON2),en);
				EnableWindow(GetDlgItem(hwndDlg,IDC_OFFSETSPIN3),en);
				EnableWindow(GetDlgItem(hwndDlg,IDC_OFFSETICON3),en);
				EnableWindow(GetDlgItem(hwndDlg,IDC_BUTTON_BROWSE),en);
				EnableWindow(GetDlgItem(hwndDlg,IDC_SHOW_ONLY_IF_DIFFERENT),en);
				EnableWindow(GetDlgItem(hwndDlg,IDC_COLOUR),en);
				EnableWindow(GetDlgItem(hwndDlg,IDC_SHOWXSTATUSNAME),en);
				EnableWindow(GetDlgItem(hwndDlg,IDC_SHOWXSTATUS),en && en_icons);
				EnableWindow(GetDlgItem(hwndDlg,IDC_SHOWBOTH),en && en_icons && IsDlgButtonChecked(hwndDlg,IDC_SHOWXSTATUS) && !IsDlgButtonChecked(hwndDlg,IDC_SHOWNORMAL));
				EnableWindow(GetDlgItem(hwndDlg,IDC_SHOWNORMAL),en && en_icons && IsDlgButtonChecked(hwndDlg,IDC_SHOWXSTATUS)&& !IsDlgButtonChecked(hwndDlg,IDC_SHOWBOTH));
				EnableWindow(GetDlgItem(hwndDlg,IDC_TRANSPARENTOVERLAY),en && en_icons && IsDlgButtonChecked(hwndDlg,IDC_SHOWXSTATUS) && IsDlgButtonChecked(hwndDlg,IDC_SHOWNORMAL)&& !IsDlgButtonChecked(hwndDlg,IDC_SHOWBOTH));
				EnableWindow(GetDlgItem(hwndDlg,IDC_SHOWUNREADEMAIL),en);

				EnableWindow(GetDlgItem(hwndDlg,IDC_OFFSETICON_LEFT),en);
				EnableWindow(GetDlgItem(hwndDlg,IDC_OFFSETSPIN_LEFT),en);
				EnableWindow(GetDlgItem(hwndDlg,IDC_OFFSETICON_RIGHT),en);
				EnableWindow(GetDlgItem(hwndDlg,IDC_OFFSETSPIN_RIGHT),en);

				EnableWindow(GetDlgItem(hwndDlg,IDC_MULTI),en);
				EnableWindow(GetDlgItem(hwndDlg,IDC_MULTI_2),en);
				EnableWindow(GetDlgItem(hwndDlg,IDC_MULTI_COUNT),en);
				EnableWindow(GetDlgItem(hwndDlg,IDC_MULTI_SPIN),en);

				EnableWindow(GetDlgItem(hwndDlg,IDC_STATUSBAR_PER_PROTO),en);
				EnableWindow(GetDlgItem(hwndDlg, IDC_STATUSBAR_PROTO_LIST), en && IsDlgButtonChecked(hwndDlg, IDC_STATUSBAR_PER_PROTO));
				EnableWindow(GetDlgItem(hwndDlg, IDC_SBAR_USE_ACCOUNT_SETTINGS), FALSE);
			}

			UpdateStatusBarOptionsDisplay(hwndDlg);

			return TRUE;
		}
	case WM_COMMAND:
		if(LOWORD(wParam)==IDC_BUTTON1)  
		{ 
			if (HIWORD(wParam)==BN_CLICKED)
			{
				CHOOSEFONTA fnt;
				memset(&fnt,0,sizeof(CHOOSEFONTA));
				fnt.lStructSize=sizeof(CHOOSEFONTA);
				fnt.hwndOwner=hwndDlg;
				fnt.Flags=CF_SCREENFONTS|CF_INITTOLOGFONTSTRUCT;
				fnt.lpLogFont=&lf;
				ChooseFontA(&fnt);
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, (WPARAM)hwndDlg, 0);
				return 0;
			} 
		}
		else if (LOWORD(wParam)==IDC_COLOUR ||(LOWORD(wParam)==IDC_SBAR_HORIZ_ALIGN && HIWORD(wParam)==CBN_SELCHANGE)) SendMessage(GetParent(hwndDlg), PSM_CHANGED, (WPARAM)hwndDlg, 0);
		else if (LOWORD(wParam)==IDC_SHOWSBAR) {
			int en=IsDlgButtonChecked(hwndDlg,IDC_SHOWSBAR);
			int en_icons = IsDlgButtonChecked(hwndDlg,IDC_SHOWICON );
			EnableWindow(GetDlgItem(hwndDlg,IDC_SHOWICON),en);
			EnableWindow(GetDlgItem(hwndDlg,IDC_SHOWPROTO),en);
			EnableWindow(GetDlgItem(hwndDlg,IDC_SHOWSTATUS),en);
			EnableWindow(GetDlgItem(hwndDlg,IDC_RIGHTSTATUS),en);
			EnableWindow(GetDlgItem(hwndDlg,IDC_RIGHTMIRANDA),en);
			EnableWindow(GetDlgItem(hwndDlg,IDC_EQUALSECTIONS),en);
			EnableWindow(GetDlgItem(hwndDlg,IDC_USECONNECTINGICON),en && en_icons);
			EnableWindow(GetDlgItem(hwndDlg,IDC_USEOWNERDRAW),en);
			EnableWindow(GetDlgItem(hwndDlg,IDC_OFFSETSPIN),en);
			EnableWindow(GetDlgItem(hwndDlg,IDC_OFFSETICON),en);
			EnableWindow(GetDlgItem(hwndDlg,IDC_OFFSETSPIN2),en);
			EnableWindow(GetDlgItem(hwndDlg,IDC_OFFSETICON2),en);
			EnableWindow(GetDlgItem(hwndDlg,IDC_OFFSETSPIN3),en);
			EnableWindow(GetDlgItem(hwndDlg,IDC_OFFSETICON3),en);
			EnableWindow(GetDlgItem(hwndDlg,IDC_SBAR_BORDER_TOP),en);
			EnableWindow(GetDlgItem(hwndDlg,IDC_SBAR_BORDER_TOP_SPIN),en);
			EnableWindow(GetDlgItem(hwndDlg,IDC_SBAR_BORDER_BOTTOM),en);
			EnableWindow(GetDlgItem(hwndDlg,IDC_SBAR_BORDER_BOTTOM_SPIN),en);
			EnableWindow(GetDlgItem(hwndDlg,IDC_SBAR_HORIZ_ALIGN),en);
			EnableWindow(GetDlgItem(hwndDlg,IDC_COLOUR),en);
			EnableWindow(GetDlgItem(hwndDlg,IDC_BUTTON1),en);
			EnableWindow(GetDlgItem(hwndDlg,IDC_SHOWXSTATUSNAME),en);
			EnableWindow(GetDlgItem(hwndDlg,IDC_SHOWXSTATUS),en && en_icons);
			EnableWindow(GetDlgItem(hwndDlg,IDC_SHOWUNREADEMAIL),en);

//			EnableWindow(GetDlgItem(hwndDlg,IDC_SHOWBOTH),en && IsDlgButtonChecked(hwndDlg,IDC_SHOWXSTATUS) && !IsDlgButtonChecked(hwndDlg,IDC_SHOWNORMAL));
//			EnableWindow(GetDlgItem(hwndDlg,IDC_SHOWNORMAL),en && IsDlgButtonChecked(hwndDlg,IDC_SHOWXSTATUS)&& !IsDlgButtonChecked(hwndDlg,IDC_SHOWBOTH));
//			EnableWindow(GetDlgItem(hwndDlg,IDC_TRANSPARENTOVERLAY),en && IsDlgButtonChecked(hwndDlg,IDC_SHOWXSTATUS) && IsDlgButtonChecked(hwndDlg,IDC_SHOWNORMAL)&& !IsDlgButtonChecked(hwndDlg,IDC_SHOWBOTH));

//			EnableWindow(GetDlgItem(hwndDlg,IDC_OFFSETICON_LEFT),en);
//			EnableWindow(GetDlgItem(hwndDlg,IDC_OFFSETSPIN_LEFT),en);
//			EnableWindow(GetDlgItem(hwndDlg,IDC_OFFSETICON_RIGHT),en);
//			EnableWindow(GetDlgItem(hwndDlg,IDC_OFFSETSPIN_RIGHT),en);

			EnableWindow(GetDlgItem(hwndDlg,IDC_MULTI),en);
			EnableWindow(GetDlgItem(hwndDlg,IDC_MULTI_2),en);
			EnableWindow(GetDlgItem(hwndDlg,IDC_MULTI_COUNT),en);
			EnableWindow(GetDlgItem(hwndDlg,IDC_MULTI_SPIN),en);

			EnableWindow(GetDlgItem(hwndDlg,IDC_STATUSBAR_PER_PROTO),en);
			EnableWindow(GetDlgItem(hwndDlg,IDC_STATUSBAR_PROTO_LIST),en && IsDlgButtonChecked(hwndDlg,IDC_STATUSBAR_PER_PROTO));
			EnableWindow(GetDlgItem(hwndDlg,IDC_SBAR_USE_ACCOUNT_SETTINGS),en && IsDlgButtonChecked(hwndDlg,IDC_STATUSBAR_PER_PROTO));

			UpdateStatusBarOptionsDisplay(hwndDlg);

			SendMessage(GetParent(hwndDlg), PSM_CHANGED, (WPARAM)hwndDlg, 0);	  
		}
		else if (LOWORD(wParam)==IDC_STATUSBAR_PER_PROTO)
		{
			int en=IsDlgButtonChecked(hwndDlg,IDC_STATUSBAR_PER_PROTO);
			EnableWindow(GetDlgItem(hwndDlg,IDC_STATUSBAR_PROTO_LIST),en);
			EnableWindow(GetDlgItem(hwndDlg,IDC_SBAR_HIDE_ACCOUNT_COMPLETELY), en && perProto);
			EnableWindow(GetDlgItem(hwndDlg,IDC_SBAR_USE_ACCOUNT_SETTINGS), en);

			UpdateStatusBarOptionsDisplay(hwndDlg);

			SendMessage(GetParent(hwndDlg), PSM_CHANGED, (WPARAM)hwndDlg, 0);
		}
		else if (
			LOWORD(wParam) == IDC_SHOWXSTATUS ||
			LOWORD(wParam) == IDC_SHOWBOTH ||
			LOWORD(wParam) == IDC_SHOWNORMAL ||
			LOWORD(wParam) == IDC_TRANSPARENTOVERLAY ||
			LOWORD(wParam) == IDC_SHOWXSTATUSNAME
			)
		{
			UpdateXStatusIconOptions(hwndDlg, perProto, dat, curSelProto);
		}
		else if (LOWORD(wParam) == IDC_SBAR_USE_ACCOUNT_SETTINGS)
		{
			if (perProto)
			{
				dat[curSelProto].AccountIsCustomized = IsDlgButtonChecked(hwndDlg, IDC_SBAR_USE_ACCOUNT_SETTINGS);

				UpdateStatusBarOptionsDisplay(hwndDlg);
			}
		}
		else if (LOWORD(wParam) == IDC_SBAR_HIDE_ACCOUNT_COMPLETELY)
		{
			if (perProto)
			{
				dat[curSelProto].HideAccount = IsDlgButtonChecked(hwndDlg, IDC_SBAR_HIDE_ACCOUNT_COMPLETELY);
			}
		}
		else if (LOWORD(wParam) == IDC_USECONNECTINGICON)
		{
			if (perProto)
			{
				dat[curSelProto].UseConnectingIcon = IsDlgButtonChecked(hwndDlg, IDC_USECONNECTINGICON);
			}
			else
			{
				_GlobalOptions.UseConnectingIcon = IsDlgButtonChecked(hwndDlg, IDC_USECONNECTINGICON);
			}
		}
		else if (LOWORD(wParam) == IDC_SHOWUNREADEMAIL)
		{
			if (perProto)
			{
				dat[curSelProto].ShowUnreadEmails = IsDlgButtonChecked(hwndDlg, IDC_SHOWUNREADEMAIL);
			}
			else
			{
				_GlobalOptions.ShowUnreadEmails = IsDlgButtonChecked(hwndDlg, IDC_SHOWUNREADEMAIL);
			}
		}
		else if (
			LOWORD(wParam) == IDC_SHOWICON ||
			LOWORD(wParam) == IDC_SHOWPROTO ||
			LOWORD(wParam) == IDC_SHOWSTATUS
			)
		{
			BYTE val = (IsDlgButtonChecked(hwndDlg, IDC_SHOWICON)?1:0)|(IsDlgButtonChecked(hwndDlg, IDC_SHOWPROTO)?2:0)|(IsDlgButtonChecked(hwndDlg, IDC_SHOWSTATUS)?4:0);
			if (perProto)
			{
				dat[curSelProto].SBarShow = val;
			}
			else
			{
				_GlobalOptions.SBarShow = val;
			}
			UpdateStatusBarOptionsDisplay(hwndDlg);
		}
		else if (LOWORD(wParam) == IDC_RIGHTSTATUS || LOWORD(wParam) == IDC_RIGHTMIRANDA)
		{
			if (perProto)
			{
				dat[curSelProto].SBarRightClk = IsDlgButtonChecked(hwndDlg,IDC_RIGHTMIRANDA);
			}
			else
			{
				_GlobalOptions.SBarRightClk = IsDlgButtonChecked(hwndDlg,IDC_RIGHTMIRANDA);
			}
		}
		else if (LOWORD(wParam) == IDC_OFFSETICON_LEFT)
		{
			if (perProto)
			{
				dat[curSelProto].PaddingLeft = (DWORD)SendDlgItemMessage(hwndDlg,IDC_OFFSETSPIN_LEFT,UDM_GETPOS,0,0);
			}
			else
			{
				_GlobalOptions.PaddingLeft = (DWORD)SendDlgItemMessage(hwndDlg,IDC_OFFSETSPIN_LEFT,UDM_GETPOS,0,0);
			}
		}
		else if (LOWORD(wParam) == IDC_OFFSETICON_RIGHT)
		{
			if (perProto)
			{
				dat[curSelProto].PaddingRight = (DWORD)SendDlgItemMessage(hwndDlg,IDC_OFFSETSPIN_RIGHT,UDM_GETPOS,0,0);
			}
			else
			{
				_GlobalOptions.PaddingRight = (DWORD)SendDlgItemMessage(hwndDlg,IDC_OFFSETSPIN_RIGHT,UDM_GETPOS,0,0);
			}
		}
		else if (
			(
				LOWORD(wParam) == IDC_MULTI_COUNT ||
				LOWORD(wParam) == IDC_OFFSETICON ||
				LOWORD(wParam) == IDC_OFFSETICON2 ||
				LOWORD(wParam) == IDC_OFFSETICON3 ||
				LOWORD(wParam) == IDC_SBAR_BORDER_BOTTOM ||
				LOWORD(wParam) == IDC_SBAR_BORDER_TOP
			) && (
				HIWORD(wParam) != EN_CHANGE ||
				(HWND)lParam != GetFocus()
			))
			return 0; // dont make apply enabled during buddy set crap 
		else if ( LOWORD(wParam) == IDC_STATUSBAR_PROTO_LIST )
		{
			UpdateStatusBarOptionsDisplay(hwndDlg);
			return 0;
		}
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, (WPARAM)hwndDlg, 0);
		break;
	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code)
		{
		case PSN_APPLY:
			{
				int count = ModernGetSettingDword(0,"Protocols","ProtoCount",-1);
				ModernWriteSettingByte(NULL, "CLUI", "SBarPerProto", IsDlgButtonChecked(hwndDlg, IDC_STATUSBAR_PER_PROTO));

				for (int i = 0; i < count; i++)
				{
					HWND hwndComboBox = GetDlgItem( hwndDlg, IDC_STATUSBAR_PROTO_LIST );
					StatusBarProtocolOptions sbpo = dat[i];
					char *defProto = sbpo.szName;

					char settingBuf[256];
					mir_snprintf(settingBuf, SIZEOF(settingBuf), "SBarAccountIsCustom_%s", defProto);
					ModernWriteSettingByte(NULL,"CLUI",settingBuf,(BYTE)sbpo.AccountIsCustomized);

					mir_snprintf(settingBuf, SIZEOF(settingBuf), "HideAccount_%s", defProto);
					ModernWriteSettingByte(NULL,"CLUI",settingBuf,(BYTE)sbpo.HideAccount);

					mir_snprintf(settingBuf, SIZEOF(settingBuf), "SBarShow_%s", defProto);
					ModernWriteSettingByte(NULL,"CLUI",settingBuf,(BYTE)sbpo.SBarShow);
					mir_snprintf(settingBuf, SIZEOF(settingBuf), "SBarRightClk_%s", defProto);
					ModernWriteSettingByte(NULL,"CLUI",settingBuf,(BYTE)sbpo.SBarRightClk);
					mir_snprintf(settingBuf, SIZEOF(settingBuf), "UseConnectingIcon_%s", defProto);
					ModernWriteSettingByte(NULL,"CLUI",settingBuf,(BYTE)sbpo.UseConnectingIcon);
					mir_snprintf(settingBuf, SIZEOF(settingBuf), "ShowUnreadEmails_%s", defProto);
					ModernWriteSettingByte(NULL,"CLUI",settingBuf,(BYTE)sbpo.ShowUnreadEmails);
					mir_snprintf(settingBuf, SIZEOF(settingBuf), "ShowXStatus_%s", defProto);
					ModernWriteSettingByte(NULL,"CLUI",settingBuf,sbpo.ShowXStatus);
					mir_snprintf(settingBuf, SIZEOF(settingBuf), "PaddingLeft_%s", defProto);
					ModernWriteSettingDword(NULL,"CLUI",settingBuf,sbpo.PaddingLeft);
					mir_snprintf(settingBuf, SIZEOF(settingBuf), "PaddingRight_%s", defProto);
					ModernWriteSettingDword(NULL,"CLUI",settingBuf,sbpo.PaddingRight);
				}

				ModernWriteSettingByte(NULL,"CLUI","SBarShow",(BYTE)_GlobalOptions.SBarShow);
				ModernWriteSettingByte(NULL,"CLUI","SBarRightClk",(BYTE)_GlobalOptions.SBarRightClk);
				ModernWriteSettingByte(NULL,"CLUI","UseConnectingIcon",(BYTE)_GlobalOptions.UseConnectingIcon);
				ModernWriteSettingByte(NULL,"CLUI","ShowUnreadEmails",(BYTE)_GlobalOptions.ShowUnreadEmails);
				ModernWriteSettingByte(NULL,"CLUI","ShowXStatus",_GlobalOptions.ShowXStatus);
				ModernWriteSettingDword(NULL,"CLUI","PaddingLeft",_GlobalOptions.PaddingLeft);
				ModernWriteSettingDword(NULL,"CLUI","PaddingRight",_GlobalOptions.PaddingRight);


				ModernWriteSettingByte(NULL,"CLUI","StatusBarProtosPerLine",(BYTE)SendDlgItemMessage(hwndDlg,IDC_MULTI_SPIN,UDM_GETPOS,0,0));
				ModernWriteSettingByte(NULL,"CLUI","EqualSections",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_EQUALSECTIONS));
				ModernWriteSettingByte(NULL,"CLUI","Align",(BYTE)SendDlgItemMessage(hwndDlg,IDC_SBAR_HORIZ_ALIGN,CB_GETCURSEL,0,0));
				ModernWriteSettingByte(NULL,"CLUI","VAlign",(BYTE)SendDlgItemMessage(hwndDlg,IDC_SBAR_VERT_ALIGN,CB_GETCURSEL,0,0));
				ModernWriteSettingDword(NULL,"CLUI","LeftOffset",(DWORD)SendDlgItemMessage(hwndDlg,IDC_OFFSETSPIN,UDM_GETPOS,0,0));
				ModernWriteSettingDword(NULL,"CLUI","RightOffset",(DWORD)SendDlgItemMessage(hwndDlg,IDC_OFFSETSPIN2,UDM_GETPOS,0,0));
				ModernWriteSettingDword(NULL,"CLUI","TopOffset",(DWORD)SendDlgItemMessage(hwndDlg,IDC_SBAR_BORDER_TOP_SPIN,UDM_GETPOS,0,0));
				ModernWriteSettingDword(NULL,"CLUI","BottomOffset",(DWORD)SendDlgItemMessage(hwndDlg,IDC_SBAR_BORDER_BOTTOM_SPIN,UDM_GETPOS,0,0));
				ModernWriteSettingDword(NULL,"CLUI","SpaceBetween",(DWORD)SendDlgItemMessage(hwndDlg,IDC_OFFSETSPIN3,UDM_GETPOS,0,0));
				ModernWriteSettingDword(NULL,"ModernData","StatusBarFontCol",SendDlgItemMessage(hwndDlg,IDC_COLOUR,CPM_GETCOLOUR,0,0));
				ModernWriteSettingByte(NULL,"CLUI","ShowSBar",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_SHOWSBAR));

				LoadStatusBarData();
				CLUIServices_ProtocolStatusChanged(0,0);	
				return TRUE;
			}
		}
		break;
	}
	return FALSE;
}
