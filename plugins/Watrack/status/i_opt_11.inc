{Templates}
const
  DLGED_INIT = $1000; // dialog init, not activate Apply button
  DLGED_PROT = $0800; // proto changed
  DLGED_STAT = $0400; // status changed
  DLGED_PACK = $0200; // needed string packing
  DLGED_CHGD = $0100; // something changed
  DLGED_MSG  = $0001; // message template changed
  DLGED_STTT = $0004; // status template changed
  DLGED_CHNL = $0008; // channel template changed
  DLGED_XTTL = $0010; // xstatus title changed
  DLGED_XTTT = $0020; // xstatus template changed
  DLGED_TUNE = $0040; // tunes template changed
  DLGED_BASE = DLGED_MSG +DLGED_STTT+DLGED_CHNL+
               DLGED_TUNE+DLGED_XTTL+DLGED_XTTT;

const
  maxShowControls = 7;
  aListFields:array [0..maxShowControls-1] of integer= (
     IDC_STATUS_TEXT,
     IDC_XSTATUS_TITLE, IDC_XSTATUS_TEXT,
     IDC_LISTENING_TEXT,
     IDC_STAT_ENABLE,IDC_XSTAT_ENABLE,IDC_TUNES_ENABLE);

  aShowFields: array [0..2,0..maxShowControls-1] of integer = (
{status}  (SW_SHOW, SW_HIDE, SW_HIDE, SW_HIDE, SW_SHOW, SW_HIDE, SW_HIDE),
{xstatus} (SW_HIDE, SW_SHOW, SW_SHOW, SW_HIDE, SW_HIDE, SW_SHOW, SW_HIDE),
{tunes}   (SW_HIDE, SW_HIDE, SW_HIDE, SW_SHOW, SW_HIDE, SW_HIDE, SW_SHOW));

var
  CurProto,
  CurStatus:integer;

procedure RedrawFields(Dialog:HWND;proto:integer=-1);
var
  wnd1,wnd:HWND;
  p:pWideChar;
  i:integer;
begin
  Changed:=Changed or DLGED_INIT;
  if proto<0 then
    proto:=CurProto;

  SetDlgItemTextW(Dialog,IDC_EDIT_MSG,
      GetTemplateStr(tmpl_pm,proto,CurStatus));

  p:=GetTemplateStr(tmpl_stext,proto,CurStatus);

  wnd:=GetDlgItem(Dialog,IDC_STATUS_TEXT);

  SendMessageW(wnd,WM_SETTEXT,0,lparam(p));
  if IsTemplateActive(tmpl_stext,proto,CurStatus) then
  begin
    EnableWindow(wnd,true);
    i:=BST_CHECKED;
  end
  else
  begin
    EnableWindow(wnd,false);
    i:=BST_UNCHECKED;
  end;
  CheckDlgButton(Dialog,IDC_STAT_ENABLE,i);

  if IsXStatusSupported(CurProto) then
  begin
    wnd :=GetDlgItem(Dialog,IDC_XSTATUS_TITLE);
    wnd1:=GetDlgItem(Dialog,IDC_XSTATUS_TEXT);
    p:=GetTemplateStr(tmpl_xtitle,proto,CurStatus);
    SendMessageW(wnd ,WM_SETTEXT,0,lparam(p));
    SendMessageW(wnd1,WM_SETTEXT,0,
        lparam(GetTemplateStr(tmpl_xtext,proto,CurStatus)));

    if IsTemplateActive(tmpl_xtitle,proto,CurStatus) then
    begin
      EnableWindow(wnd ,true);
      EnableWindow(wnd1,true);
      i:=BST_CHECKED;
    end
    else
    begin
      EnableWindow(wnd ,false);
      EnableWindow(wnd1,false);
      i:=BST_UNCHECKED;
    end;
    CheckDlgButton(Dialog,IDC_XSTAT_ENABLE,i);
  end;

  if IsTunesSupported(CurProto) then
  begin
    p:=GetTemplateStr(tmpl_tunes,proto,CurStatus);
    wnd:=GetDlgItem(Dialog,IDC_LISTENING_TEXT);
    SendMessageW(wnd,WM_SETTEXT,0,lparam(p));
    if IsTemplateActive(tmpl_tunes,proto,CurStatus) then
    begin
      EnableWindow(wnd,true);
      i:=BST_CHECKED;
    end
    else
    begin
      EnableWindow(wnd,false);
      i:=BST_UNCHECKED;
    end;
    CheckDlgButton(Dialog,IDC_TUNES_ENABLE,i);
  end;
 
  if IsChatSupported(CurProto) then
    SetDlgItemTextW(Dialog,IDC_EDIT_CHANNEL,
       GetTemplateStr(tmpl_chat,proto,CurStatus));

  Changed:=Changed and not DLGED_INIT;
end;

procedure SetScreenFull(Dialog:HWND);
var
  show:integer;
  buf:array [0..127] of AnsiChar;
  endis:boolean;
  wnd:HWND;
begin
  Changed:=Changed or DLGED_INIT;
  CurStatus:=0;

  CheckDlgButton(Dialog,IDC_IRC_USER   ,BST_CHECKED);
  CheckDlgButton(Dialog,IDC_IRC_CHANNEL,BST_UNCHECKED);
  ShowWindow(GetDlgItem(Dialog,IDC_EDIT_MSG    ),SW_SHOW);
  ShowWindow(GetDlgItem(Dialog,IDC_EDIT_CHANNEL),SW_HIDE);

  if IsChatSupported(CurProto) then
    show:=SW_SHOW
  else
    show:=SW_HIDE;

  ShowWindow(GetDlgItem(Dialog,IDC_IRC_USER   ),show);
  ShowWindow(GetDlgItem(Dialog,IDC_IRC_CHANNEL),show);

//  wnd:=GetDlgItem(Dialog,IDC_PROTOLIST);

//!!!!  SendMessage(wnd,CB_SETCURSEL,0,0); //???

//  SendMessageA(wnd,LVM_GETITEMTEXTA,);  	
//  ListView_GetItemTextA(wnd,CurProto,0,@buf,SizeOf(buf));

  StrCopy(buf,GetProtoName(CurProto));
  StrCat(buf,PS_GETCUSTOMSTATUSICON);
  endis:=ServiceExists(buf)<>0;

  EnableWindow(GetDlgItem(Dialog,IDC_CBEX       ),endis);
  EnableWindow(GetDlgItem(Dialog,IDC_XSTAT_AUDIO),endis);
  EnableWindow(GetDlgItem(Dialog,IDC_XSTAT_VIDEO),endis);
  if endis then
  begin
    CheckDlgButton(Dialog,IDC_XSTAT_AUDIO,BST_CHECKED);
    CheckDlgButton(Dialog,IDC_XSTAT_VIDEO,BST_UNCHECKED);
  end;
  
  wnd:=GetDlgItem(Dialog,IDC_STATUSLIST);
  FillStatusList(CurProto,wnd,true);
  SendMessage(wnd,CB_SETCURSEL,0,0);
  RedrawFields(Dialog);
end;

procedure SetTemplate(Dialog:HWND;idc:integer;Tmpl:tTemplateType);
begin
  SetTemplateStr   (GetDlgText(Dialog,ABS(idc)),Tmpl,CurProto,CurStatus);
  SetTemplateActive(idc>0                      ,Tmpl,CurProto,CurStatus);
end;

procedure SaveChanges(Dialog:HWND);
var
  i,j:integer;
begin
  if (Changed and DLGED_BASE)<>0 then
  begin
    if (Changed and DLGED_MSG )<>0 then SetTemplate(Dialog,IDC_EDIT_MSG    ,tmpl_pm);
    if (Changed and DLGED_CHNL)<>0 then SetTemplate(Dialog,IDC_EDIT_CHANNEL,tmpl_chat);


    if (Changed and DLGED_STTT)<>0 then
    begin
      if IsDlgButtonChecked(Dialog,IDC_STAT_ENABLE)<>BST_UNCHECKED then
        i:=IDC_STATUS_TEXT
      else
        i:=-IDC_STATUS_TEXT;
      SetTemplate(Dialog,i,tmpl_stext);
    end;

    if (Changed and DLGED_TUNE)<>0 then
    begin
      if IsDlgButtonChecked(Dialog,IDC_TUNES_ENABLE)<>BST_UNCHECKED then
        i:=IDC_LISTENING_TEXT
      else
        i:=-IDC_LISTENING_TEXT;
      SetTemplate(Dialog,i,tmpl_tunes);
    end;

    if (Changed and (DLGED_XTTL or DLGED_XTTT))<>0 then
    begin
      if IsDlgButtonChecked(Dialog,IDC_XSTAT_ENABLE)<>BST_UNCHECKED then
      begin
        i:=IDC_XSTATUS_TITLE;
        j:=IDC_XSTATUS_TEXT;
      end
      else
      begin
        i:=-IDC_XSTATUS_TITLE;
        j:=-IDC_XSTATUS_TEXT;
      end;
      if (Changed and DLGED_XTTL)<>0 then SetTemplate(Dialog,i,tmpl_xtitle);
      if (Changed and DLGED_XTTT)<>0 then SetTemplate(Dialog,j,tmpl_xtext);
    end;

    Changed:=Changed and (not DLGED_BASE);
  end;
end;

function SaveCBExValue(Dialog:HWND;direct:boolean):cardinal;
var
  wnd:HWND;
  i,j,shift:cardinal;
begin
  wnd:=GetDlgItem(Dialog,IDC_CBEX);
  i:=GetProtoSetting(CurProto,true);

  if (IsDlgButtonChecked(Dialog,IDC_XSTAT_VIDEO)<>BST_UNCHECKED) xor direct then
    shift:=0
  else
    shift:=8;

  j:=SendMessage(wnd,CB_GETCURSEL,0,0);
  i:=(i and ($FFFF0000 or ($FF shl (8-shift)))) or (j shl shift);

  SetProtoSetting(CurProto,i,true);

  result:=(i shr (8-shift)) and $FF;
end;

procedure FillCBType(Dialog:HWND;proto:pAnsiChar=nil);
var
  wnd:HWND;
  j:integer;
begin
  wnd:=GetDlgItem(Dialog,IDC_CBSTATYPE);
  SendMessage(wnd,CB_RESETCONTENT,0,0);

  CB_AddStrDataW(wnd,TranslateW('Status'),0);
  if IsXStatusSupported(uint_ptr(proto)) then CB_AddStrDataW(wnd,TranslateW('XStatus'),1);
  if IsTunesSupported  (uint_ptr(proto)) then CB_AddStrDataW(wnd,TranslateW('Tunes'  ),2);

  SendMessage(wnd,CB_SETCURSEL,0,0);

  for j:=0 to maxShowControls-1 do
    ShowWindow(GetDlgItem(Dialog,aListFields[j]),aShowFields[0][j]);
end;

function DlgProcOptions11(Dialog:HWND;hMessage:uint;wParam:WPARAM;lParam:LPARAM):LRESULT; stdcall;
var
  Item:LV_ITEMA;
  buf:array [0..127] of AnsiChar;
  i,j:integer;
  wnd:HWND;
  b:boolean;
begin
  result:=0;
  case hMessage of
    WM_DESTROY: begin
      ListView_SetImageList(GetDlgItem(Dialog,IDC_STATUSLIST),0,LVSIL_SMALL);
      ListView_SetImageList(GetDlgItem(Dialog,IDC_PROTOLIST ),0,LVSIL_SMALL);
    end;

    WM_INITDIALOG: begin
      TranslateDialogDefault(Dialog);
      Changed:=DLGED_INIT;
      FillProtoList(GetDlgItem(Dialog,IDC_PROTOLIST),true);
      CurProto  :=0;
      SetScreenFull(Dialog);
      FillCBType(Dialog,nil);
      SendMessage(Dialog,WM_COMMAND,(CBN_SELCHANGE shl 16)+IDC_CBSTATYPE,
          GetDlgItem(Dialog,IDC_CBSTATYPE));
      result:=0;
      Changed:=0;
    end;

    WM_COMMAND: begin
      case wParam shr 16 of
        EN_CHANGE: begin
          if (Changed and DLGED_INIT)=0 then
          begin
            Changed:=Changed or DLGED_CHGD or DLGED_PACK;
            case loword(wParam) of
              IDC_EDIT_MSG      : Changed:=Changed or DLGED_MSG;
              IDC_EDIT_CHANNEL  : Changed:=Changed or DLGED_CHNL;
              IDC_STATUS_TEXT   : Changed:=Changed or DLGED_STTT;
              IDC_XSTATUS_TITLE : Changed:=Changed or DLGED_XTTL;
              IDC_XSTATUS_TEXT  : Changed:=Changed or DLGED_XTTT;
              IDC_LISTENING_TEXT: Changed:=Changed or DLGED_TUNE;
            end;
            SendMessage(GetParent(Dialog),PSM_CHANGED,0,0);
          end;
        end;

        CBN_SELCHANGE: begin
          case loword(wParam) of
            IDC_CBSTATYPE: begin
              i:=CB_GetData(lParam);
              for j:=0 to maxShowControls-1 do
                ShowWindow(GetDlgItem(Dialog,aListFields[j]),aShowFields[i][j]);
            end;

            IDC_CBEX: begin
              SendMessage(GetParent(Dialog),PSM_CHANGED,0,0);
            end;
          end;
        end;

        BN_CLICKED: begin
          case LoWord(wParam) of
            IDC_CMD_DEFAULT: begin
              RedrawFields(Dialog,0);
              Changed:=Changed or DLGED_CHGD or DLGED_BASE;
              SendMessage(GetParent(Dialog),PSM_CHANGED,0,0);
            end;
            IDC_CMD_RESET: begin
              RedrawFields(Dialog);
            end;

            IDC_STAT_ENABLE, IDC_XSTAT_ENABLE, IDC_TUNES_ENABLE: begin
              case LoWord(wParam) of
                IDC_STAT_ENABLE: begin
                  Changed:=Changed or DLGED_STTT;
                  EnableWindow(GetDlgItem(Dialog,IDC_STATUS_TEXT),
                      IsDlgButtonChecked(Dialog,IDC_STAT_ENABLE)<>BST_UNCHECKED);
                end;
                IDC_XSTAT_ENABLE: begin
                  b:=IsDlgButtonChecked(Dialog,IDC_XSTAT_ENABLE)<>BST_UNCHECKED;
                  EnableWindow(GetDlgItem(Dialog,IDC_XSTATUS_TITLE),b);
                  EnableWindow(GetDlgItem(Dialog,IDC_XSTATUS_TEXT ),b);
                  Changed:=Changed or DLGED_XTTL;
                  Changed:=Changed or DLGED_XTTT;
                end;
                IDC_TUNES_ENABLE: begin
                  Changed:=Changed or DLGED_TUNE;
                  EnableWindow(GetDlgItem(Dialog,IDC_LISTENING_TEXT),
                      IsDlgButtonChecked(Dialog,IDC_TUNES_ENABLE)<>BST_UNCHECKED);
                end;
              end;
              SendMessage(GetParent(Dialog),PSM_CHANGED,0,0);
            end;

            IDC_XSTAT_VIDEO, IDC_XSTAT_AUDIO: begin
              SendDlgItemMessage(Dialog,IDC_CBEX,CB_SETCURSEL,SaveCBExValue(Dialog,false),0);
            end;
            IDC_HELP_COLOR: begin
              ShowColorHelpDlg(Dialog);
              exit;
            end;
            IDC_HELP_FORMAT: begin
              MessageBoxW(0,TranslateW(sFormatHelp),TranslateW('Format text Info'),0);
              exit;
            end;
            IDC_HELP_VARIABLES: begin
              CallService(MS_WAT_MACROHELP,Dialog,0);
              exit;
            end;
            IDC_IRC_USER: begin
//              CheckDlgButton(Dialog,IDC_IRC_USER   ,BST_CHECKED);
//              CheckDlgButton(Dialog,IDC_IRC_CHANNEL,BST_UNCHECKED);
              ShowWindow(GetDlgItem(Dialog,IDC_EDIT_MSG    ),SW_SHOW);
              ShowWindow(GetDlgItem(Dialog,IDC_EDIT_CHANNEL),SW_HIDE);
            end;
            IDC_IRC_CHANNEL: begin
//              CheckDlgButton(Dialog,IDC_IRC_USER   ,BST_UNCHECKED);
//              CheckDlgButton(Dialog,IDC_IRC_CHANNEL,BST_CHECKED);
              ShowWindow(GetDlgItem(Dialog,IDC_EDIT_CHANNEL),SW_SHOW);
              ShowWindow(GetDlgItem(Dialog,IDC_EDIT_MSG    ),SW_HIDE);
            end;
          else
            SaveChanges(Dialog); //??
          end;
        end;
      end;

    end;

    WM_HELP: begin
      case PHELPINFO(lParam).iCtrlId of
        IDC_EDIT_MSG,IDC_EDIT_CHANNEL,IDC_XSTATUS_TITLE,
        IDC_STATUS_TEXT,IDC_XSTATUS_TEXT,IDC_LISTENING_TEXT:
          CallService(MS_WAT_MACROHELP,Dialog,0);
      end;
    end;

    WM_NOTIFY: begin
      if (Changed and DLGED_INIT)<>0 then
        exit;
      if integer(PNMLISTVIEW(lParam)^.hdr.code)=LVN_ITEMCHANGED then
      begin
        SendMessage(GetParent(Dialog),PSM_CHANGED,0,0);
        if ((PNMLISTVIEW(lParam)^.uNewState and LVIS_FOCUSED)<>0) then
        begin
          SaveChanges(Dialog);

          if PNMLISTVIEW(lParam)^.hdr.idFrom=IDC_PROTOLIST then
          begin
            CheckStatusList(GetDlgItem(Dialog,IDC_STATUSLIST),CurProto);
            SaveCBExValue(Dialog,true);
            CurProto:=PNMLISTVIEW(lParam)^.iItem;
            SetScreenFull(Dialog);
//            ListView_GetItemTextA(PNMLISTVIEW(lParam)^.hdr.hwndFrom,CurProto,0,@buf,SizeOf(buf));
            StrCopy(buf,GetProtoName(CurProto));
            wnd:=GetDlgItem(Dialog,IDC_CBEX);
            FillCBType(Dialog,buf);
            if AddCBEx(wnd,buf)<>0 then
            begin
              i:=GetProtoSetting(CurProto,true);
              SendMessage(wnd,CB_SETCURSEL,i and $FF,0);
            end;
          end

          else //IDC_STATUSLIST
          begin
            Item.iItem:=PNMLISTVIEW(lParam)^.iItem;
            Item.mask:=LVIF_PARAM;
            SendMessageA(PNMLISTVIEW(lParam)^.hdr.hwndFrom,LVM_GETITEMA,0,tlparam(@Item));
//            ListView_GetItemA(PNMLISTVIEW(lParam)^.hdr.hwndFrom,Item);
            CurStatus:=GetStatusNum(Item.lParam);
            RedrawFields(Dialog);
          end;
        end

        else if PNMLISTVIEW(lParam)^.uNewState<>0 then
        begin
          if PNMLISTVIEW(lParam)^.hdr.idFrom=IDC_PROTOLIST then
            Changed:=Changed or DLGED_PROT or DLGED_CHGD
          else
            Changed:=Changed or DLGED_STAT or DLGED_CHGD;
        end;
      end

      else if integer(PNMHdr(lParam)^.code)=PSN_APPLY then
      begin
        if (Changed and DLGED_PROT)<>0 then
        begin
          CheckProtoList(GetDlgItem(Dialog,IDC_PROTOLIST));
          Changed:=Changed and not DLGED_PROT;
        end;
        if (Changed and DLGED_STAT)<>0 then
        begin
          CheckStatusList(GetDlgItem(Dialog,IDC_STATUSLIST),CurProto);
          Changed:=Changed and not DLGED_STAT;
        end;
        SaveCBExValue(Dialog,true);
        SaveChanges(Dialog);
        SaveTemplates;
      end;
    end;
  else
    {result:=}DefWindowProc(Dialog,hMessage,wParam,lParam);
  end;
end;
