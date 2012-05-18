#include "hdr/modern_commonheaders.h"
#include "hdr/modern_statusbar.h"
#include "./m_api/m_skin_eng.h"
#include "hdr/modern_commonprototypes.h"
#include "hdr/modern_clcpaint.h"
#include "hdr/modern_sync.h"

BOOL tooltipshoing;
POINT lastpnt;

#define TM_STATUSBAR 23435234
#define TM_STATUSBARHIDE 23435235

HWND hModernStatusBar=NULL;
HANDLE hFramehModernStatusBar=NULL;
extern void ApplyViewMode(const char *Name, bool onlySelector = false );
extern void SaveViewMode(const char *name, const TCHAR *szGroupFilter, const char *szProtoFilter, DWORD statusMask, DWORD stickyStatusMask, unsigned int options,  unsigned int stickies, unsigned int operators, unsigned int lmdat);

//int FindFrameID(HWND FrameHwnd);
COLORREF sttGetColor(char * module, char * color, COLORREF defColor);

#define DBFONTF_BOLD       1
#define DBFONTF_ITALIC     2
#define DBFONTF_UNDERLINE  4

typedef struct _ProtoItemData
{
    HICON icon;
    HICON extraIcon;
    int iconIndex;
    char * ProtoName;
    char * AccountName;
    int ProtoStatus;
    TCHAR *ProtoHumanName;
	char *ProtoEMailCount;
    char * ProtoStatusText;
    TCHAR * ProtoXStatus;
    int ProtoPos;
    int fullWidth;
    RECT protoRect;
    BOOL DoubleIcons;

	BYTE showProtoIcon;
	BYTE showProtoName;
	BYTE showStatusName;
	BYTE xStatusMode;     // 0-only main, 1-xStatus, 2-main as overlay
	BYTE connectingIcon;
	BYTE showProtoEmails;
	BYTE SBarRightClk;
	int PaddingLeft;
	int PaddingRight;

	bool isDimmed;

} ProtoItemData;

ProtoItemData *ProtosData=NULL;
int allocedItemData=0;
STATUSBARDATA g_StatusBarData={0};


char * ApendSubSetting(char * buf, int size, char *first, char *second)
{
    _snprintf(buf,size,"%sFont%s",first,second);
    return buf;
}

int LoadStatusBarData()
{
	g_StatusBarData.perProtoConfig=ModernGetSettingByte(NULL,"CLUI","SBarPerProto",SETTING_SBARPERPROTO_DEFAULT);
	g_StatusBarData.showProtoIcon=ModernGetSettingByte(NULL,"CLUI","SBarShow",SETTING_SBARSHOW_DEFAULT)&1;
	g_StatusBarData.showProtoName=ModernGetSettingByte(NULL,"CLUI","SBarShow",SETTING_SBARSHOW_DEFAULT)&2;
	g_StatusBarData.showStatusName=ModernGetSettingByte(NULL,"CLUI","SBarShow",SETTING_SBARSHOW_DEFAULT)&4;
	g_StatusBarData.xStatusMode=(BYTE)(ModernGetSettingByte(NULL,"CLUI","ShowXStatus",SETTING_SHOWXSTATUS_DEFAULT));
	g_StatusBarData.connectingIcon=ModernGetSettingByte(NULL,"CLUI","UseConnectingIcon",SETTING_USECONNECTINGICON_DEFAULT);
	g_StatusBarData.showProtoEmails=ModernGetSettingByte(NULL,"CLUI","ShowUnreadEmails",SETTING_SHOWUNREADEMAILS_DEFAULT);
	g_StatusBarData.SBarRightClk=ModernGetSettingByte(NULL,"CLUI","SBarRightClk",SETTING_SBARRIGHTCLK_DEFAULT);

	g_StatusBarData.nProtosPerLine=ModernGetSettingByte(NULL,"CLUI","StatusBarProtosPerLine",SETTING_PROTOSPERLINE_DEFAULT);
	g_StatusBarData.Align=ModernGetSettingByte(NULL,"CLUI","Align",SETTING_ALIGN_DEFAULT);
	g_StatusBarData.VAlign=ModernGetSettingByte(NULL,"CLUI","VAlign",SETTING_VALIGN_DEFAULT);
    g_StatusBarData.sameWidth=ModernGetSettingByte(NULL,"CLUI","EqualSections",SETTING_EQUALSECTIONS_DEFAULT);
    g_StatusBarData.rectBorders.left=ModernGetSettingDword(NULL,"CLUI","LeftOffset",SETTING_LEFTOFFSET_DEFAULT);
    g_StatusBarData.rectBorders.right=ModernGetSettingDword(NULL,"CLUI","RightOffset",SETTING_RIGHTOFFSET_DEFAULT);
    g_StatusBarData.rectBorders.top=ModernGetSettingDword(NULL,"CLUI","TopOffset",SETTING_TOPOFFSET_DEFAULT);
    g_StatusBarData.rectBorders.bottom=ModernGetSettingDword(NULL,"CLUI","BottomOffset",SETTING_BOTTOMOFFSET_DEFAULT);
    g_StatusBarData.extraspace=(BYTE)ModernGetSettingDword(NULL,"CLUI","SpaceBetween",SETTING_SPACEBETWEEN_DEFAULT);

	if (g_StatusBarData.BarFont) DeleteObject(g_StatusBarData.BarFont);
    g_StatusBarData.BarFont=NULL;//LoadFontFromDB("ModernData","StatusBar",&g_StatusBarData.fontColor);
    {
        int vis=ModernGetSettingByte(NULL,"CLUI","ShowSBar",SETTING_SHOWSBAR_DEFAULT);
        int frameopt;
        int frameID=Sync( FindFrameID, hModernStatusBar );
        frameopt=CallService(MS_CLIST_FRAMES_GETFRAMEOPTIONS,MAKEWPARAM(FO_FLAGS,frameID),0);
        frameopt=frameopt & (~F_VISIBLE);
        if(vis)
        {
            ShowWindow(hModernStatusBar,SW_SHOW);
            frameopt|=F_VISIBLE;
        }
        else
        {
            ShowWindow(hModernStatusBar,SW_HIDE);
        };
        CallService(MS_CLIST_FRAMES_SETFRAMEOPTIONS,MAKEWPARAM(FO_FLAGS,frameID),frameopt);
    }
    g_StatusBarData.TextEffectID=ModernGetSettingByte(NULL,"StatusBar","TextEffectID",SETTING_TEXTEFFECTID_DEFAULT);
    g_StatusBarData.TextEffectColor1=ModernGetSettingDword(NULL,"StatusBar","TextEffectColor1",SETTING_TEXTEFFECTCOLOR1_DEFAULT);
    g_StatusBarData.TextEffectColor2=ModernGetSettingDword(NULL,"StatusBar","TextEffectColor2",SETTING_TEXTEFFECTCOLOR2_DEFAULT);

    if(g_StatusBarData.hBmpBackground) {DeleteObject(g_StatusBarData.hBmpBackground); g_StatusBarData.hBmpBackground=NULL;}

    if (g_CluiData.fDisableSkinEngine)
    {
        DBVARIANT dbv;
        g_StatusBarData.bkColour=sttGetColor("StatusBar","BkColour",CLCDEFAULT_BKCOLOUR);
        if(ModernGetSettingByte(NULL,"StatusBar","UseBitmap",CLCDEFAULT_USEBITMAP)) {
            if(!ModernGetSettingString(NULL,"StatusBar","BkBitmap",&dbv)) {
                g_StatusBarData.hBmpBackground=(HBITMAP)CallService(MS_UTILS_LOADBITMAP,0,(LPARAM)dbv.pszVal);
                ModernDBFreeVariant(&dbv);
            }
        }
        g_StatusBarData.bkUseWinColors=ModernGetSettingByte(NULL,"StatusBar", "UseWinColours", CLCDEFAULT_USEWINDOWSCOLOURS);
        g_StatusBarData.backgroundBmpUse=ModernGetSettingWord(NULL,"StatusBar","BkBmpUse",CLCDEFAULT_BKBMPUSE);
    }
    SendMessage(pcli->hwndContactList,WM_SIZE,0,0);
	
    return 1;
}

int BgStatusBarChange(WPARAM wParam,LPARAM lParam)
{
    if (MirandaExiting()) return 0;
    LoadStatusBarData();
    return 0;
}

//ProtocolData;
int NewStatusPaintCallbackProc(HWND hWnd, HDC hDC, RECT * rcPaint, HRGN rgn, DWORD dFlags, void * CallBackData)
{
    return ModernDrawStatusBar(hWnd,hDC);
}

int ModernDrawStatusBar(HWND hwnd, HDC hDC)
{
    if (hwnd==(HWND)-1) return 0;
    if (GetParent(hwnd)==pcli->hwndContactList)
        return ModernDrawStatusBarWorker(hwnd,hDC);
    else
        CLUI__cliInvalidateRect(hwnd,NULL,FALSE);
    return 0;
}

int ModernDrawStatusBarWorker(HWND hWnd, HDC hDC)
{
	int line;
	int iconHeight=GetSystemMetrics(SM_CYSMICON)+2;
	int protosperline=0;
	int visProtoCount=0;
	int protoCount;
	int SumWidth=0;
	int rectwidth=0;
	int aligndx=0;
	int * ProtoWidth=NULL;
	int i,j,po=0;

	char servName[40];
	char protoNameExt[40];
	// Count visible protos
	PROTOACCOUNT **accs;
	RECT rc;
	HFONT hOldFont;
	int iconY, textY;
	int spaceWidth;
	int maxwidth=0;
	int xstatus=0;
	SIZE textSize={0};
	GetClientRect(hWnd,&rc);
	if (g_CluiData.fDisableSkinEngine)
	{
		if (g_StatusBarData.bkUseWinColors && xpt_IsThemed(g_StatusBarData.hTheme))
		{
			xpt_DrawTheme(g_StatusBarData.hTheme, hWnd, hDC, 0, 0, &rc, &rc);           
		}
		else
			DrawBackGround(hWnd, hDC,  g_StatusBarData.hBmpBackground, g_StatusBarData.bkColour, g_StatusBarData.backgroundBmpUse );
	}
	else
	{
		SkinDrawGlyph(hDC,&rc,&rc,"Main,ID=StatusBar"); //TBD
	}
	g_StatusBarData.nProtosPerLine=ModernGetSettingByte(NULL,"CLUI","StatusBarProtosPerLine",SETTING_PROTOSPERLINE_DEFAULT);
	hOldFont=g_clcPainter.ChangeToFont(hDC,NULL,FONTID_STATUSBAR_PROTONAME,NULL);

	//  hOldFont=SelectObject(hDC,g_StatusBarData.BarFont);
	//  SetTextColor(hDC,g_StatusBarData.fontColor);
	{

		GetTextExtentPoint32A(hDC," ",1,&textSize);
		spaceWidth=textSize.cx;
		textY=rc.top+((rc.bottom-rc.top-textSize.cy)>>1);
	}
	iconY=rc.top+((rc.bottom-rc.top-GetSystemMetrics(SM_CXSMICON))>>1);

	{
		ProtoEnumAccounts( &protoCount, &accs );
		if (allocedItemData && ProtosData)
		{
			int k;

			for (k=0; k<allocedItemData; k++)
			{
				if(ProtosData[k].ProtoXStatus) mir_free_and_nill (ProtosData[k].ProtoXStatus);
				if(ProtosData[k].ProtoName) mir_free_and_nill (ProtosData[k].ProtoName);
				if(ProtosData[k].AccountName) mir_free_and_nill (ProtosData[k].AccountName);
				if(ProtosData[k].ProtoHumanName) mir_free_and_nill (ProtosData[k].ProtoHumanName);
				if(ProtosData[k].ProtoEMailCount) mir_free_and_nill (ProtosData[k].ProtoEMailCount);
				if(ProtosData[k].ProtoStatusText) mir_free_and_nill (ProtosData[k].ProtoStatusText);
			}
			mir_free_and_nill(ProtosData);
			ProtosData=NULL;
			allocedItemData=0;
		}
		if ( protoCount == 0 ) 
			return 0;

		ProtosData=(ProtoItemData*)mir_alloc(sizeof(ProtoItemData)*protoCount);
		memset(ProtosData,0,sizeof(ProtoItemData)*protoCount);
		for (j=0; j<protoCount; j++)
		{
			int vis;
			char buf[256];

			i=pcli->pfnGetAccountIndexByPos(j);
			if (i==-1) 
				vis=FALSE;
			else
				vis=pcli->pfnGetProtocolVisibility(accs[i]->szModuleName);
			if (!vis) continue;
				
			mir_snprintf(buf, SIZEOF(buf), "SBarAccountIsCustom_%s", accs[i]->szModuleName);

			if (g_StatusBarData.perProtoConfig && ModernGetSettingByte(NULL, "CLUI", buf, SETTING_SBARACCOUNTISCUSTOM_DEFAULT))
			{
				mir_snprintf(buf, SIZEOF(buf), "HideAccount_%s", accs[i]->szModuleName);
				if (ModernGetSettingByte(NULL, "CLUI", buf, SETTING_SBARHIDEACCOUNT_DEFAULT))
					continue;

				mir_snprintf(buf, SIZEOF(buf), "SBarShow_%s", accs[i]->szModuleName);
				{
					BYTE showOps = ModernGetSettingByte(NULL,"CLUI", buf, SETTING_SBARSHOW_DEFAULT);
					ProtosData[visProtoCount].showProtoIcon = showOps&1;
					ProtosData[visProtoCount].showProtoName = showOps&2;
					ProtosData[visProtoCount].showStatusName = showOps&4;
				}

				mir_snprintf(buf, SIZEOF(buf), "ShowXStatus_%s", accs[i]->szModuleName);
				ProtosData[visProtoCount].xStatusMode = ModernGetSettingByte(NULL,"CLUI", buf, SETTING_SBARSHOW_DEFAULT);

				mir_snprintf(buf, SIZEOF(buf), "UseConnectingIcon_%s", accs[i]->szModuleName);
				ProtosData[visProtoCount].connectingIcon = ModernGetSettingByte(NULL,"CLUI", buf, SETTING_USECONNECTINGICON_DEFAULT);

				mir_snprintf(buf, SIZEOF(buf), "ShowUnreadEmails_%s", accs[i]->szModuleName);
				ProtosData[visProtoCount].showProtoEmails = ModernGetSettingByte(NULL,"CLUI", buf, SETTING_SHOWUNREADEMAILS_DEFAULT);

				mir_snprintf(buf, SIZEOF(buf), "SBarRightClk_%s", accs[i]->szModuleName);
				ProtosData[visProtoCount].SBarRightClk = ModernGetSettingByte(NULL,"CLUI", buf, SETTING_SBARRIGHTCLK_DEFAULT);

				mir_snprintf(buf, SIZEOF(buf), "PaddingLeft_%s", accs[i]->szModuleName);
				ProtosData[visProtoCount].PaddingLeft = ModernGetSettingDword(NULL,"CLUI", buf, SETTING_PADDINGLEFT_DEFAULT);

				mir_snprintf(buf, SIZEOF(buf), "PaddingRight_%s", accs[i]->szModuleName);
				ProtosData[visProtoCount].PaddingRight = ModernGetSettingDword(NULL,"CLUI", buf, SETTING_PADDINGRIGHT_DEFAULT);
			}
			else
			{
				ProtosData[visProtoCount].showProtoIcon = g_StatusBarData.showProtoIcon;
				ProtosData[visProtoCount].showProtoName = g_StatusBarData.showProtoName;
				ProtosData[visProtoCount].showStatusName = g_StatusBarData.showStatusName;
				ProtosData[visProtoCount].xStatusMode = g_StatusBarData.xStatusMode;
				ProtosData[visProtoCount].connectingIcon = g_StatusBarData.connectingIcon;
				ProtosData[visProtoCount].showProtoEmails = g_StatusBarData.showProtoEmails;
				ProtosData[visProtoCount].SBarRightClk = 0;
				ProtosData[visProtoCount].PaddingLeft = 0;
				ProtosData[visProtoCount].PaddingRight = 0;
			}

			ProtosData[visProtoCount].ProtoStatus = CallProtoService(accs[i]->szModuleName,PS_GETSTATUS,0,0);

			ProtosData[visProtoCount].ProtoEMailCount = NULL;
			if (ProtosData[visProtoCount].ProtoStatus > ID_STATUS_OFFLINE)
			{
				// create service name
				mir_snprintf(servName, SIZEOF(servName), "%s/GetUnreadEmailCount", accs[i]->szModuleName);
				if (ProtosData[visProtoCount].showProtoEmails == 1 && ServiceExists(servName))
				{
					mir_snprintf(protoNameExt, SIZEOF(protoNameExt),"[%d]", (int) CallService(servName, 0, 0));
					ProtosData[visProtoCount].ProtoEMailCount = mir_strdup(protoNameExt);
				}
			}

			ProtosData[visProtoCount].ProtoHumanName = mir_tstrdup(accs[i]->tszAccountName);
			ProtosData[visProtoCount].AccountName = mir_strdup(accs[i]->szModuleName);
            ProtosData[visProtoCount].ProtoName = mir_strdup(accs[i]->szProtoName);
			ProtosData[visProtoCount].ProtoStatusText = mir_strdup((char*)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION,(WPARAM)ProtosData[visProtoCount].ProtoStatus,0));
			ProtosData[visProtoCount].ProtoPos = visProtoCount;

			ProtosData[visProtoCount].isDimmed = 0;
			if(g_CluiData.bFilterEffective & CLVM_FILTER_PROTOS) 
			{
				char szTemp[2048];
				mir_snprintf(szTemp, SIZEOF(szTemp), "%s|", ProtosData[visProtoCount].AccountName );
				ProtosData[visProtoCount].isDimmed = strstr(g_CluiData.protoFilter, szTemp) ? 0 : 1;
			}
			
			visProtoCount++;
			allocedItemData++;
		}

		// Calculate rects

		//START MULTILINE HERE 
		{
			int orig_protoCount=protoCount;
			int orig_visProtoCount=visProtoCount;
			ProtoItemData * orig_ProtosData=ProtosData;
			int linecount;
			if (g_StatusBarData.nProtosPerLine)
				protosperline=g_StatusBarData.nProtosPerLine;
			else if (orig_visProtoCount)
				protosperline=orig_visProtoCount;
			else if (protoCount)
			{
				protosperline=protoCount;
				orig_visProtoCount=protoCount;
			}
			else 
			{
				protosperline=1;
				orig_visProtoCount=1;
			}
			protosperline=min(protosperline,orig_visProtoCount);
            linecount=protosperline ? (orig_visProtoCount+(protosperline-1))/protosperline : 1; //divide with rounding to up
			for (line=0; line<linecount; line++)
			{    
				int height;
				int rowsdy;
				int rowheight=max(textSize.cy+2,iconHeight);
				protoCount=min(protosperline,(orig_protoCount-line*protosperline));
				visProtoCount=min(protosperline,(orig_visProtoCount-line*protosperline));
				ProtosData+=line*protosperline; 
				GetClientRect(hWnd,&rc);

				rc.top += g_StatusBarData.rectBorders.top;
				rc.bottom -= g_StatusBarData.rectBorders.bottom;

				aligndx=0;
				maxwidth=0;
				xstatus=0;
				SumWidth=0;
				height=(rowheight*linecount);

				if (height > (rc.bottom - rc.top))
				{
					rowheight = (rc.bottom - rc.top) / linecount;
					height=(rowheight*linecount);
				}

				rowsdy=((rc.bottom-rc.top)-height)/2;
				if (rowheight*(line)+rowsdy<rc.top-rowheight) continue;
				if (rowheight*(line+1)+rowsdy>rc.bottom+rowheight)
				{
					ProtosData=orig_ProtosData;
					break;
				}
				if (g_StatusBarData.VAlign == 0) //top
				{
					rc.bottom=rc.top+rowheight*(line+1);
					rc.top=rc.top+rowheight*line+1;
				}
				else if (g_StatusBarData.VAlign == 1) //center
				{
					rc.bottom=rc.top+rowsdy+rowheight*(line+1);
					rc.top=rc.top+rowsdy+rowheight*line+1;
				}
				else if (g_StatusBarData.VAlign == 2) //bottom
				{
					rc.top=rc.bottom - (rowheight*(linecount - line));
					rc.bottom=rc.bottom - (rowheight*(linecount - line - 1)+1);
				}

				textY=rc.top+(((rc.bottom-rc.top)-textSize.cy)/2);
				iconY=rc.top+(((rc.bottom-rc.top)-iconHeight)/2);

				{ //Code for each line
					{
						DWORD sw;
						rectwidth=rc.right-rc.left-g_StatusBarData.rectBorders.left-g_StatusBarData.rectBorders.right;
						if (visProtoCount>1) sw=(rectwidth-(g_StatusBarData.extraspace*(visProtoCount-1)))/visProtoCount;
						else sw=rectwidth;
						if (ProtoWidth) mir_free_and_nill(ProtoWidth);
						ProtoWidth=(int*)mir_alloc(sizeof(int)*visProtoCount);
						for (i=0; i<visProtoCount; i++)
						{
							SIZE textSize;
							DWORD w = 0;
							// FIXME strange error occurs here due to Dimon log
							w = ProtosData[i].PaddingLeft;
							w += ProtosData[i].PaddingRight;

							if ( ProtosData[i].showProtoIcon )
							{

								w += GetSystemMetrics(SM_CXSMICON)+1;

								ProtosData[i].extraIcon=NULL;
								if ((ProtosData[i].xStatusMode&8) && ProtosData[i].ProtoStatus>ID_STATUS_OFFLINE) 
								{
									char str[MAXMODULELABELLENGTH];
									mir_snprintf(str, SIZEOF(str), "%s/GetXStatus", ProtosData[i].AccountName);
									if (ServiceExists(str))
									{
										char * dbTitle="XStatusName";
										char * dbTitle2=NULL;
										xstatus=CallProtoService(ProtosData[i].AccountName,"/GetXStatus",(WPARAM)&dbTitle,(LPARAM)&dbTitle2);
										if (dbTitle && xstatus)
										{
											DBVARIANT dbv={0};
											if (!ModernGetSettingTString(NULL,ProtosData[i].AccountName,dbTitle,&dbv))
											{
												ProtosData[i].ProtoXStatus=mir_tstrdup(dbv.ptszVal);
												//mir_free_and_nill(dbv.ptszVal);
												ModernDBFreeVariant(&dbv);
											}
										}
									}
								}
								if ((ProtosData[i].xStatusMode&3))
								{
									if (ProtosData[i].ProtoStatus>ID_STATUS_OFFLINE)
									{
										char str[MAXMODULELABELLENGTH];
										mir_snprintf(str, SIZEOF(str), "%s/GetXStatusIcon", ProtosData[i].AccountName);
										if (ServiceExists(str))
											ProtosData[i].extraIcon=(HICON)CallService(str,0,0);
										if (ProtosData[i].extraIcon && (ProtosData[i].xStatusMode&3)==3)
											w+=GetSystemMetrics(SM_CXSMICON)+1;

									}
								}
							}

							if (ProtosData[i].showProtoName)
							{
								GetTextExtentPoint32(hDC,ProtosData[i].ProtoHumanName,lstrlen(ProtosData[i].ProtoHumanName),&textSize);
								w += textSize.cx + 3 + spaceWidth;
							}

							if (ProtosData[i].showProtoEmails && ProtosData[i].ProtoEMailCount )
							{
								GetTextExtentPoint32A(hDC,ProtosData[i].ProtoEMailCount,lstrlenA(ProtosData[i].ProtoEMailCount),&textSize);
								w+=textSize.cx+3+spaceWidth;
							}

							if (ProtosData[i].showStatusName)
							{
								GetTextExtentPoint32A(hDC,ProtosData[i].ProtoStatusText,lstrlenA(ProtosData[i].ProtoStatusText),&textSize);
								w += textSize.cx + 3 + spaceWidth;
							}

							if ((ProtosData[i].xStatusMode&8) && ProtosData[i].ProtoXStatus)
							{
								GetTextExtentPoint32(hDC,ProtosData[i].ProtoXStatus,lstrlen(ProtosData[i].ProtoXStatus),&textSize);
								w += textSize.cx + 3 + spaceWidth;
							}

							if ( ( ProtosData[i].showProtoName) || 
								(ProtosData[i].showProtoEmails && ProtosData[i].ProtoEMailCount ) || 
								(ProtosData[i].showStatusName) ||
								((ProtosData[i].xStatusMode&8) && ProtosData[i].ProtoXStatus) )
								w -= spaceWidth;

							ProtosData[i].fullWidth=w;
							if (g_StatusBarData.sameWidth)
							{
								ProtoWidth[i]=sw;
								SumWidth+=w;
							}
							else
							{
								ProtoWidth[i]=w;
								SumWidth+=w;
							}
						}

						// Reposition rects
						for(i=0; i<visProtoCount; i++)
							if (ProtoWidth[i]>maxwidth) maxwidth=ProtoWidth[i];

						if (g_StatusBarData.sameWidth)
						{
							for (i=0; i<visProtoCount; i++)
								ProtoWidth[i]=maxwidth;
							SumWidth=maxwidth*visProtoCount;
						}
						SumWidth+=(visProtoCount-1)*(g_StatusBarData.extraspace+1);

						if (SumWidth>rectwidth)
						{
							float f=(float)rectwidth/SumWidth;
							//dx=(int)(0.5+(float)dx/visProtoCount);
							//SumWidth-=dx*visProtoCount;
							SumWidth=0;
							for (i=0; i<visProtoCount; i++)
							{
								ProtoWidth[i]=(int)((float)ProtoWidth[i]*f);
								SumWidth+=ProtoWidth[i];
							}
							SumWidth+=(visProtoCount-1)*(g_StatusBarData.extraspace+1);
						}
					}
					if (g_StatusBarData.Align==1) //center
						aligndx=(rectwidth-SumWidth)>>1;
					else if (g_StatusBarData.Align==2) //right
						aligndx=(rectwidth-SumWidth);
					// Draw in rects
					//ske_SelectTextEffect(hDC,g_StatusBarData.TextEffectID,g_StatusBarData.TextEffectColor1,g_StatusBarData.TextEffectColor2);
					{
						RECT r=rc;
//						r.top+=g_StatusBarData.rectBorders.top;
//						r.bottom-=g_StatusBarData.rectBorders.bottom;
						r.left+=g_StatusBarData.rectBorders.left+aligndx;
						for (i=0; i< visProtoCount; i++)
						{
							HRGN rgn;
							int x = r.left;
							HICON hIcon=NULL;
							HICON hxIcon=NULL;
							BOOL NeedDestroy=FALSE;
							x = r.left; 
							x += ProtosData[i].PaddingLeft;
							r.right=r.left+ProtoWidth[i];

							if ( ProtosData[i].showProtoIcon )
							{
								if (ProtosData[i].ProtoStatus>ID_STATUS_OFFLINE && ((ProtosData[i].xStatusMode)&3)>0)
								{
									char str[MAXMODULELABELLENGTH];
									mir_snprintf(str, SIZEOF(str), "%s/GetXStatusIcon", ProtosData[i].AccountName);
									if (ServiceExists(str))
									{
										hxIcon=ProtosData[i].extraIcon;
										if (hxIcon)
										{
											if (((ProtosData[i].xStatusMode)&3)==2)
											{
												hIcon=GetMainStatusOverlay(ProtosData[i].ProtoStatus);
												NeedDestroy=TRUE;
											}
											else if (((ProtosData[i].xStatusMode)&3)==1)
											{
												hIcon=hxIcon;
												NeedDestroy=TRUE;
												hxIcon=NULL;
											}

										}
									}
								}
								if (hIcon==NULL && (hxIcon==NULL || (((ProtosData[i].xStatusMode)&3)==3)))
								{
									if (hIcon==NULL && (ProtosData[i].connectingIcon==1) && ProtosData[i].ProtoStatus>=ID_STATUS_CONNECTING&&ProtosData[i].ProtoStatus<=ID_STATUS_CONNECTING+MAX_CONNECT_RETRIES)
									{
										hIcon=(HICON)CLUI_GetConnectingIconService((WPARAM)ProtosData[i].AccountName,0);
										if (hIcon) NeedDestroy=TRUE;
										else hIcon=LoadSkinnedProtoIcon(ProtosData[i].AccountName,ProtosData[i].ProtoStatus);
									}
									else hIcon=LoadSkinnedProtoIcon(ProtosData[i].AccountName,ProtosData[i].ProtoStatus);
								}


								rgn=CreateRectRgn(r.left,r.top,r.right,r.bottom);
								//
								{
									if (g_StatusBarData.sameWidth)
									{
										int fw=ProtosData[i].fullWidth;
										int rw=r.right-r.left;
										if (g_StatusBarData.Align==1)
										{
											x=r.left+((rw-fw)/2);
										}
										else if (g_StatusBarData.Align==2)
										{
											x=r.left+((rw-fw));
										}
										else x=r.left;
									}
								}

								
								SelectClipRgn(hDC,rgn);
								ProtosData[i].DoubleIcons=FALSE;

								DWORD dim = ProtosData[i].isDimmed ? ( ( 64<<24 ) | 0x80 ) : 0;

								if ((ProtosData[i].xStatusMode&3)==3)
								{
									if (hIcon) mod_DrawIconEx_helper(hDC,x,iconY,hIcon,GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),0,NULL,  DI_NORMAL|dim );
									if (hxIcon) 
									{
										mod_DrawIconEx_helper(hDC,x+GetSystemMetrics(SM_CXSMICON)+1,iconY,hxIcon,GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),0,NULL,DI_NORMAL|dim);
										x+=GetSystemMetrics(SM_CXSMICON)+1;
									}
									ProtosData[i].DoubleIcons=hIcon&&hxIcon;
								}
								else
								{
									if (hxIcon) mod_DrawIconEx_helper(hDC,x,iconY,hxIcon,GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),0,NULL,DI_NORMAL|dim);
									if (hIcon) mod_DrawIconEx_helper(hDC,x,iconY,hIcon,GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),0,NULL,DI_NORMAL| ((hxIcon&&(ProtosData[i].xStatusMode&4))?(192<<24):0 ) | dim );
								}

								if ( ( hxIcon || hIcon) && TRUE /* TODO g_StatusBarData.bDrawLockOverlay  options to draw locked proto*/  )
								{
									if ( ModernGetSettingByte( NULL,ProtosData[i].AccountName,"LockMainStatus",0 ) )
									{
										HICON hLockOverlay = LoadSkinnedIcon(SKINICON_OTHER_STATUS_LOCKED);
										if (hLockOverlay != NULL)
										{
											mod_DrawIconEx_helper(hDC, x, iconY, hLockOverlay, GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),0,NULL,DI_NORMAL | dim);
											CallService(MS_SKIN2_RELEASEICON, (WPARAM)hLockOverlay, 0);									}
									}
								}
								if (hxIcon) DestroyIcon_protect(hxIcon);
								if (NeedDestroy) DestroyIcon_protect(hIcon);
								else CallService(MS_SKIN2_RELEASEICON, (WPARAM)hIcon, 0);
								x+=GetSystemMetrics(SM_CXSMICON)+1;
							}
							if (ProtosData[i].showProtoName)
							{
								SIZE textSize;
								RECT rt=r;
								rt.left=x+(spaceWidth>>1);
								rt.top=textY;
								ske_DrawText(hDC,ProtosData[i].ProtoHumanName,lstrlen(ProtosData[i].ProtoHumanName),&rt,0);
								
								if ((ProtosData[i].showProtoEmails && ProtosData[i].ProtoEMailCount!=NULL) || ProtosData[i].showStatusName || ((ProtosData[i].xStatusMode&8) && ProtosData[i].ProtoXStatus))
								{
									GetTextExtentPoint32(hDC, ProtosData[i].ProtoHumanName, lstrlen(ProtosData[i].ProtoHumanName), &textSize);
									x += textSize.cx + 3;
								}
							}

							if (ProtosData[i].showProtoEmails && ProtosData[i].ProtoEMailCount!=NULL)
							{
								SIZE textSize;
								RECT rt=r;
								rt.left=x+(spaceWidth>>1);
								rt.top=textY;
								ske_DrawTextA(hDC,ProtosData[i].ProtoEMailCount,lstrlenA(ProtosData[i].ProtoEMailCount),&rt,0);
								//TextOutS(hDC,x,textY,ProtosData[i].ProtoName,lstrlenA(ProtosData[i].ProtoName));
								if (ProtosData[i].showStatusName || ((ProtosData[i].xStatusMode&8) && ProtosData[i].ProtoXStatus))
								{
									GetTextExtentPoint32A(hDC,ProtosData[i].ProtoEMailCount,lstrlenA(ProtosData[i].ProtoEMailCount),&textSize);
									x+=textSize.cx+3;
								}
							}

							if (ProtosData[i].showStatusName)
							{
								SIZE textSize;
								RECT rt=r;
								rt.left=x+(spaceWidth>>1);
								rt.top=textY;
								ske_DrawTextA(hDC, ProtosData[i].ProtoStatusText, lstrlenA(ProtosData[i].ProtoStatusText),&rt,0);
								if (((ProtosData[i].xStatusMode&8) && ProtosData[i].ProtoXStatus))
								{
									GetTextExtentPoint32A(hDC,ProtosData[i].ProtoStatusText,lstrlenA(ProtosData[i].ProtoStatusText),&textSize);
									x+=textSize.cx+3;
								}
								//TextOutS(hDC,x,textY,ProtosData[i].ProtoStatusText,lstrlenA(ProtosData[i].ProtoStatusText));
							}

							if ((ProtosData[i].xStatusMode&8) && ProtosData[i].ProtoXStatus)
							{
								RECT rt=r;
								rt.left=x+(spaceWidth>>1);
								rt.top=textY;
								ske_DrawText(hDC,ProtosData[i].ProtoXStatus,lstrlen(ProtosData[i].ProtoXStatus),&rt,0);
								//TextOutS(hDC,x,textY,ProtosData[i].ProtoStatusText,lstrlenA(ProtosData[i].ProtoStatusText));
							}

							ProtosData[i].protoRect=r;

							r.left=r.right+g_StatusBarData.extraspace;
							//SelectClipRgn(hDC,NULL);
							DeleteObject(rgn);

						}
					}

					if (ProtoWidth) mir_free_and_nill(ProtoWidth);
				} //code for each line
				ProtosData=orig_ProtosData;
			}
		}
	}

	SelectObject(hDC,hOldFont);
	ske_ResetTextEffect(hDC);
	return 0;
}

static BOOL _ModernStatus_OnExtraIconClick( int protoIndex )
{
    /** FIXME
        Create Protocol services and
        move This portion of code to related Protocols
    */
    
    if ( !mir_strcmpi( ProtosData[protoIndex].ProtoName, "ICQ" ) )
    {
        if ( ProtosData[protoIndex].ProtoStatus < ID_STATUS_ONLINE ) return FALSE;

        HMENU hMainStatusMenu = (HMENU)CallService(MS_CLIST_MENUGETSTATUS,0,0);
        if ( !hMainStatusMenu ) return FALSE;
        
        HMENU hProtoStatusMenu = GetSubMenu( hMainStatusMenu, protoIndex );
        if ( !hProtoStatusMenu ) return FALSE;

        int extraStatusMenuIndex = 1;
        HMENU hExtraStatusMenu = GetSubMenu( hProtoStatusMenu, extraStatusMenuIndex );
        if ( !hExtraStatusMenu ) return FALSE;

        POINT pt; GetCursorPos( &pt );
        {
            HWND hWnd = (HWND) CallService( MS_CLUI_GETHWND, 0 ,0 );
            TrackPopupMenu( hExtraStatusMenu, TPM_TOPALIGN|TPM_LEFTALIGN|TPM_LEFTBUTTON, pt.x, pt.y, 0, hWnd, NULL );
        }
        return TRUE;
    } 
    else if ( !mir_strcmpi( ProtosData[protoIndex].ProtoName, "JABBER" ) )
    {
        if ( ProtosData[protoIndex].ProtoStatus < ID_STATUS_ONLINE ) return FALSE;
        // Show Moods
        char szService[128];
        mir_snprintf(szService, SIZEOF(szService), "%s/AdvStatusSet/Mood", ProtosData[protoIndex].AccountName );
        if ( ServiceExists( szService ) )
        {
            CallService( szService, 0 ,0 );
            return TRUE;
        }
    }
    return FALSE;
}

#define TOOLTIP_TOLERANCE 5
LRESULT CALLBACK ModernStatusProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
    static POINT ptToolTipShow={0};
    switch (msg) 
    {
    case WM_CREATE:
		g_StatusBarData.hTheme=xpt_AddThemeHandle(hwnd,L"STATUS");
		break;
    case WM_DESTROY:
		xpt_FreeThemeForWindow(hwnd);
		if (allocedItemData && ProtosData)
		{
			int k;

			for (k=0; k<allocedItemData; k++)
			{
				if(ProtosData[k].AccountName) mir_free_and_nill (ProtosData[k].AccountName);
				if(ProtosData[k].ProtoName) mir_free_and_nill (ProtosData[k].ProtoName);
				if(ProtosData[k].ProtoEMailCount) mir_free_and_nill (ProtosData[k].ProtoEMailCount);
				if(ProtosData[k].ProtoHumanName) mir_free_and_nill (ProtosData[k].ProtoHumanName);
				if(ProtosData[k].ProtoStatusText) mir_free_and_nill (ProtosData[k].ProtoStatusText);
				if(ProtosData[k].ProtoEMailCount) mir_free_and_nill (ProtosData[k].ProtoEMailCount);
				if(ProtosData[k].ProtoXStatus) mir_free_and_nill (ProtosData[k].ProtoXStatus);
			}
			mir_free_and_nill(ProtosData);
			ProtosData=NULL;
			allocedItemData=0;
		}
		break;
    case WM_SIZE:
		if (!g_CluiData.fLayered || GetParent(hwnd)!=pcli->hwndContactList)
            InvalidateRect(hwnd,NULL,FALSE);
		return DefWindowProc(hwnd, msg, wParam, lParam);
    case WM_ERASEBKGND:
		return 1;
    case WM_PAINT:
		if (GetParent(hwnd)==pcli->hwndContactList && g_CluiData.fLayered)
            CallService(MS_SKINENG_INVALIDATEFRAMEIMAGE,(WPARAM)hwnd,0);
		else if (GetParent(hwnd)==pcli->hwndContactList && !g_CluiData.fLayered)
		{
            HDC hdc, hdc2;
            HBITMAP hbmp,hbmpo;
            RECT rc={0};
            GetClientRect(hwnd,&rc);
            rc.right++;
            rc.bottom++;
            hdc = GetDC(hwnd);
            hdc2=CreateCompatibleDC(hdc);
            hbmp=ske_CreateDIB32(rc.right,rc.bottom);
            hbmpo=(HBITMAP)SelectObject(hdc2,hbmp);  
            SetBkMode(hdc2,TRANSPARENT);
            ske_BltBackImage(hwnd,hdc2,&rc);
            ModernDrawStatusBarWorker(hwnd,hdc2);
            BitBlt(hdc,rc.left,rc.top,rc.right-rc.left,rc.bottom-rc.top,
                hdc2,rc.left,rc.top,SRCCOPY);
            SelectObject(hdc2,hbmpo);
            DeleteObject(hbmp);
            mod_DeleteDC(hdc2);

            SelectObject(hdc,GetStockObject(DEFAULT_GUI_FONT));
            ReleaseDC(hwnd,hdc);
            ValidateRect(hwnd,NULL);
        }
        else
        {
            HDC hdc, hdc2;
            HBITMAP hbmp, hbmpo;
            RECT rc;
            PAINTSTRUCT ps;
            HBRUSH br=GetSysColorBrush(COLOR_3DFACE);
            GetClientRect(hwnd,&rc);
            hdc=BeginPaint(hwnd,&ps);
            hdc2=CreateCompatibleDC(hdc);
            hbmp=ske_CreateDIB32(rc.right,rc.bottom);
            hbmpo=(HBITMAP) SelectObject(hdc2,hbmp);
            FillRect(hdc2,&ps.rcPaint,br);
            ModernDrawStatusBarWorker(hwnd,hdc2);
            //BitBlt(hdc,ps.rcPaint.left,ps.rcPaint.top,ps.rcPaint.right-ps.rcPaint.left,ps.rcPaint.bottom-ps.rcPaint.top,
            //  hdc2,ps.rcPaint.left,ps.rcPaint.top,SRCCOPY);
            BitBlt(hdc,ps.rcPaint.left,ps.rcPaint.top,ps.rcPaint.right-ps.rcPaint.left,ps.rcPaint.bottom-ps.rcPaint.top,
                hdc2,ps.rcPaint.left,ps.rcPaint.top,SRCCOPY);
            SelectObject(hdc2,hbmpo);
            DeleteObject(hbmp);
            mod_DeleteDC(hdc2);
            ps.fErase=FALSE;
            EndPaint(hwnd,&ps);
        }
        return DefWindowProc(hwnd, msg, wParam, lParam);

    case WM_GETMINMAXINFO:{
        RECT rct;
        GetWindowRect(hwnd,&rct);
        memset((LPMINMAXINFO)lParam,0,sizeof(MINMAXINFO));
        ((LPMINMAXINFO)lParam)->ptMinTrackSize.x=16;
        ((LPMINMAXINFO)lParam)->ptMinTrackSize.y=16;
        ((LPMINMAXINFO)lParam)->ptMaxTrackSize.x=1600;
        ((LPMINMAXINFO)lParam)->ptMaxTrackSize.y=1600;
        return(0);
                          }

    case WM_SHOWWINDOW:
        {
            int res;
            int ID;
            if (tooltipshoing){
                NotifyEventHooks(g_CluiData.hEventStatusBarHideToolTip,0,0);
                tooltipshoing=FALSE;
            };
            ID=Sync( FindFrameID, hwnd );
            if (ID)
            {
                res=CallService(MS_CLIST_FRAMES_GETFRAMEOPTIONS, MAKEWPARAM(FO_FLAGS,ID),0);
                if (res>=0) ModernWriteSettingByte(0,"CLUI","ShowSBar",(BYTE)(wParam/*(res&F_VISIBLE)*/?1:0));
            }
        }
        break;
    case WM_TIMER:
        {
            if (wParam==TM_STATUSBARHIDE)
            {
                KillTimer(hwnd,TM_STATUSBARHIDE);
                if (tooltipshoing)
                {
                    NotifyEventHooks(g_CluiData.hEventStatusBarHideToolTip,0,0);
                    tooltipshoing=FALSE;
                    ReleaseCapture();
                };
            }
            else if (wParam==TM_STATUSBAR)
            {
                POINT pt;
                KillTimer(hwnd,TM_STATUSBAR);
                GetCursorPos(&pt);
                if (pt.x==lastpnt.x&&pt.y==lastpnt.y)
                {
                    int i;
                    RECT rc;
                    ScreenToClient(hwnd,&pt);
                    for (i=0; i<allocedItemData; i++)
                    {
                        rc=ProtosData[i].protoRect;
                        if(PtInRect(&rc,pt))
                        {
                            NotifyEventHooks(g_CluiData.hEventStatusBarShowToolTip,(WPARAM)ProtosData[i].AccountName,0);
                            CLUI_SafeSetTimer(hwnd,TM_STATUSBARHIDE,ModernGetSettingWord(NULL,"CLUIFrames","HideToolTipTime",SETTING_HIDETOOLTIPTIME_DEFAULT),0);
                            tooltipshoing=TRUE;
                            ClientToScreen(hwnd,&pt);
                            ptToolTipShow=pt;
                            SetCapture(hwnd);
                            return 0;
                        }
                    }
                    return 0;
                }
            }
            return 0;
        }
    case WM_MOUSEMOVE:
        if (tooltipshoing)
        {
            POINT pt;
            GetCursorPos(&pt);
            if (abs(pt.x-ptToolTipShow.x)>TOOLTIP_TOLERANCE || abs(pt.y-ptToolTipShow.y)>TOOLTIP_TOLERANCE)
            {
                KillTimer(hwnd,TM_STATUSBARHIDE);
                NotifyEventHooks(g_CluiData.hEventStatusBarHideToolTip,0,0);
                tooltipshoing=FALSE;
                ReleaseCapture();
            }
        }
        break;
    case WM_SETCURSOR:
        {
            if (g_CluiData.bBehindEdgeSettings) CLUI_UpdateTimer(0);
            {
                POINT pt;
                GetCursorPos(&pt);
                SendMessage(GetParent(hwnd),msg,wParam,lParam);
                if (pt.x==lastpnt.x&&pt.y==lastpnt.y)
                {
                    return(CLUI_TestCursorOnBorders());
                };
                lastpnt=pt;
                if (tooltipshoing)
                    if  (abs(pt.x-ptToolTipShow.x)>TOOLTIP_TOLERANCE || abs(pt.y-ptToolTipShow.y)>TOOLTIP_TOLERANCE)
                    {
                        KillTimer(hwnd,TM_STATUSBARHIDE);
                        NotifyEventHooks(g_CluiData.hEventStatusBarHideToolTip,0,0);
                        tooltipshoing=FALSE;
                        ReleaseCapture();
                    };
                KillTimer(hwnd,TM_STATUSBAR);
                CLUI_SafeSetTimer(hwnd,TM_STATUSBAR,ModernGetSettingWord(NULL,"CLC","InfoTipHoverTime",CLCDEFAULT_INFOTIPTIME),0);

                return(CLUI_TestCursorOnBorders());
            }
        }
    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
        {
            RECT rc;
            POINT pt;
            int i;
            pt.x=(short)LOWORD(lParam);
            pt.y=(short)HIWORD(lParam);
            KillTimer(hwnd,TM_STATUSBARHIDE);
            KillTimer(hwnd,TM_STATUSBAR);

            if (tooltipshoing){
                NotifyEventHooks(g_CluiData.hEventStatusBarHideToolTip,0,0);
            };
            tooltipshoing=FALSE;
            for (i=0; i<allocedItemData; i++)
            {
                RECT rc1;
                BOOL isOnExtra=FALSE;

                rc=ProtosData[i].protoRect;
                rc1=rc;
                rc1.left=rc.left+16;
                rc1.right=rc1.left+16;
                if (PtInRect(&rc,pt) && PtInRect(&rc1,pt)&&ProtosData[i].DoubleIcons)
                    isOnExtra=TRUE;
                if(PtInRect(&rc,pt))
                {
                    HMENU hMenu=NULL;

                    BOOL bShift =( GetKeyState( VK_SHIFT )&0x8000 );
                    BOOL bCtrl  =( GetKeyState( VK_CONTROL )&0x8000 );

                    if ( ( msg==WM_MBUTTONDOWN || ( msg==WM_RBUTTONDOWN && bCtrl ) || isOnExtra) && _ModernStatus_OnExtraIconClick( i ) )
                    {
                        return TRUE;
                    }
                    if ( msg == WM_LBUTTONDOWN && bCtrl )
                    {
                        if ( g_CluiData.bFilterEffective != CLVM_FILTER_PROTOS || !bShift )
                        {
                            ApplyViewMode( "" );
                            mir_snprintf( g_CluiData.protoFilter, SIZEOF(g_CluiData.protoFilter), "%s|", ProtosData[i].AccountName );
                            g_CluiData.bFilterEffective = CLVM_FILTER_PROTOS;
                        }
                        else
                        {
                            char protoF[ sizeof(g_CluiData.protoFilter) ];
                            mir_snprintf( protoF, SIZEOF(protoF), "%s|", ProtosData[i].AccountName );
                            char * pos = strstri( g_CluiData.protoFilter, ProtosData[i].AccountName );
                            if ( pos ) 
                            {
                                // remove filter
                                int len = strlen( protoF );
                                memmove( pos, pos + len, strlen( pos + len ) + 1 );

                                if ( strlen( g_CluiData.protoFilter ) == 0 )
                                    ApplyViewMode( "" );
                                else
                                    g_CluiData.bFilterEffective = CLVM_FILTER_PROTOS;
                            }
                            else
                            {
                                //add filter
                                mir_snprintf( g_CluiData.protoFilter, SIZEOF(g_CluiData.protoFilter), "%s%s", g_CluiData.protoFilter, protoF );
                                g_CluiData.bFilterEffective = CLVM_FILTER_PROTOS;
                            }
                        }
                        if ( g_CluiData.bFilterEffective == CLVM_FILTER_PROTOS)
                        {
                            char filterName[ sizeof(g_CluiData.protoFilter) ] = { 0 };
                            filterName[0] = (char)13;

                            int protoCount;
                            PROTOACCOUNT ** accs;
                            ProtoEnumAccounts( &protoCount, &accs );

                            bool first = true;
                            for ( int pos = 0; pos<protoCount; pos++ )
                            {
                                int i = pcli->pfnGetAccountIndexByPos( pos );

                                if ( i < 0 && i >= protoCount )
                                    continue;

                                char protoF[ sizeof(g_CluiData.protoFilter) ];
                                mir_snprintf( protoF, SIZEOF(protoF), "%s|", accs[i]->szModuleName );
                                if ( strstri( g_CluiData.protoFilter, protoF ) )
                                {
                                    char * temp = mir_utf8encodeT( accs[i]->tszAccountName );
                                    if ( !first )
                                        strncat( filterName, "; ", SIZEOF(filterName) - strlen(filterName) );
                                    strncat( filterName, temp, SIZEOF(filterName) - strlen(filterName) );
                                    first = false;
                                    mir_free( temp );
                                }
                            }
                           
                            SaveViewMode( filterName, _T(""), g_CluiData.protoFilter, 0, -1, 0, 0, 0, 0 );

                            ApplyViewMode( filterName );
                        }
                        pcli->pfnClcBroadcast(CLM_AUTOREBUILD, 0, 0);
                        CLUI__cliInvalidateRect( hwnd, NULL, FALSE );
                        SetCapture( NULL );
                        return 0;
                    }
                    if (!hMenu)
                    {
                        if (msg==WM_RBUTTONDOWN)
                        {
                            BOOL a = ( (g_StatusBarData.perProtoConfig && ProtosData[i].SBarRightClk) || g_StatusBarData.SBarRightClk );
                            if ( a ^ bShift )
                                hMenu=(HMENU)CallService(MS_CLIST_MENUGETMAIN,0,0);
                            else
                                hMenu=(HMENU)CallService(MS_CLIST_MENUGETSTATUS,0,0);
                        }
                        else
                        {
                            hMenu=(HMENU)CallService(MS_CLIST_MENUGETSTATUS,0,0);  
                            unsigned int cpnl = 0;
                            int mcnt = GetMenuItemCount(hMenu);
                            for (int j=0; j<mcnt; ++j) {
                                HMENU hMenus = GetSubMenu(hMenu, j);
                                if (hMenus && cpnl++ == i) { 
                                    hMenu = hMenus; 
                                    break; 
                                }
                            }
                        }
                    }
                    ClientToScreen(hwnd,&pt);
                    {
                        HWND parent=GetParent(hwnd);
                        if (parent!=pcli->hwndContactList) parent=GetParent(parent);
                        TrackPopupMenu(hMenu,TPM_TOPALIGN|TPM_LEFTALIGN|TPM_LEFTBUTTON,pt.x,pt.y,0,parent,NULL);
                    }
                    return 0;
                }
            }
            GetClientRect( hwnd, &rc );
            if ( PtInRect( &rc, pt ) && msg == WM_LBUTTONDOWN && g_CluiData.bFilterEffective == CLVM_FILTER_PROTOS )
            {
                ApplyViewMode( "" );
                CLUI__cliInvalidateRect( hwnd, NULL, FALSE );
                SetCapture( NULL );
                return 0;
            }
            return SendMessage(GetParent(hwnd), msg, wParam, lParam );
        }
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

HWND StatusBar_Create(HWND parent)
{
    WNDCLASS wndclass={0};
    TCHAR pluginname[]=TEXT("ModernStatusBar");
    int h=GetSystemMetrics(SM_CYSMICON)+2;
    if (GetClassInfo(g_hInst,pluginname,&wndclass) ==0)
    {
        wndclass.style         = 0;
        wndclass.lpfnWndProc   = ModernStatusProc;
        wndclass.cbClsExtra    = 0;
        wndclass.cbWndExtra    = 0;
        wndclass.hInstance     = g_hInst;
        wndclass.hIcon         = NULL;
        wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW);
        wndclass.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
        wndclass.lpszMenuName  = NULL;
        wndclass.lpszClassName = pluginname;
        RegisterClass(&wndclass);
    }
    hModernStatusBar=CreateWindow(pluginname,pluginname,WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN,
        0,0,0,h,parent,NULL,g_hInst,NULL);
    // register frame
    {
        CLISTFrame Frame;
        memset(&Frame,0,sizeof(Frame));
        Frame.cbSize=sizeof(CLISTFrame);
        Frame.hWnd=hModernStatusBar;
        Frame.align=alBottom;
        Frame.hIcon=LoadSkinnedIcon (SKINICON_OTHER_MIRANDA);
        Frame.Flags=(ModernGetSettingByte(NULL,"CLUI","ShowSBar",SETTING_SHOWSBAR_DEFAULT)?F_VISIBLE:0)|F_LOCKED|F_NOBORDER|F_NO_SUBCONTAINER|F_TCHAR;
        Frame.height=h;
        Frame.tname=_T("Status Bar");
        Frame.TBtname=TranslateT("Status Bar");
        hFramehModernStatusBar=(HANDLE)CallService(MS_CLIST_FRAMES_ADDFRAME,(WPARAM)&Frame,(LPARAM)0);
        CallService(MS_SKINENG_REGISTERPAINTSUB,(WPARAM)Frame.hWnd,(LPARAM)NewStatusPaintCallbackProc); //$$$$$ register sub for frame
    }

    LoadStatusBarData();
    CLUIServices_ProtocolStatusChanged(0,0);
    CallService(MS_CLIST_FRAMES_UPDATEFRAME,-1,0);
    return hModernStatusBar;
}
