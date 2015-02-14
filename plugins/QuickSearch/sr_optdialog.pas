unit sr_optdialog;

interface
uses windows;

function DlgProcOptions(Dialog:HWND;hMessage:uint;wParam:WPARAM;lParam:LPARAM):LRESULT; stdcall;

implementation

uses
  messages,commctrl,
  m_api,common,mirutils,wrapper,dbsettings,
  sr_global,sr_window,
  sparam,editwrapper,srvblock;

var
  OldListProc:pointer;
var
  ServiceBlock:HWND; // single copy, can make it global
var
  editcolumns:array [0..MaxColumnAmount-1] of tcolumnitem;

const
  stByte       :PWideChar = 'Byte';
  stWord       :PWideChar = 'Word';
  stDWord      :PWideChar = 'DWord';
  stSigned     :PWideChar = 'Signed';
  stHexnum     :PWideChar = 'As hex';
  stString     :PWideChar = 'String';
  stLastSeen   :PWideChar = 'LastSeen';
  stIP         :PWideChar = 'IP';
  stContactInfo:PWideChar = 'ContactInfo';
  stLastEvent  :PWideChar = 'LastEvent';
  stTimeStamp  :PWideChar = 'TimeStamp';
  stService    :PWideChar = 'Service';
  stScript     :PWideChar = 'Script';
  stMetacontact:PWideChar = 'Metacontact';
  stEventCount :PWideChar = 'EventCount';

  stSetting    :PWideChar = 'DB setting';
  stOther      :PWideChar = 'Other';

const
  cnFirstName  = 'FIRSTNAME' ;
  cnLastName   = 'LASTNAME'  ;
  cnNick       = 'NICK'      ;
  cnCustomNick = 'CUSTOMNICK';
  cnEmail      = 'EMAIL'     ;
  cnCity       = 'CITY'      ;
  cnState      = 'STATE'     ;
  cnCountry    = 'COUNTRY'   ;
  cnPhone      = 'PHONE'     ;
  cnHomepage   = 'HOMEPAGE'  ;
  cnAbout      = 'ABOUT'     ;
  cnGender     = 'GENDER'    ;
  cnAge        = 'AGE'       ;
  cnFirstLast  = 'FIRSTLAST' ;
  cnUniqueID   = 'UNIQUEID'  ;
  cnFax        = 'FAX'       ;
  cnCellular   = 'CELLULAR'  ;
  cnTimezone   = 'TIMEZONE'  ;
  cnMyNotes    = 'MYNOTES'   ;
  cnBirthday   = 'BIRTHDAY'  ;
  cnBirthMonth = 'BIRTHMONTH';
  cnBirthYear  = 'BIRTHYEAR' ;
  cnStreet     = 'STREET'    ;
  cnZIP        = 'ZIP'       ;
  cnLanguage1  = 'LANGUAGE1' ;
  cnLanguage2  = 'LANGUAGE2' ;
  cnLanguage3  = 'LANGUAGE3' ;
  cnCoName     = 'CONAME'    ;
  cnCoDept     = 'CODEPT'    ;
  cnCoPosition = 'COPOSITION';
  cnCoStreet   = 'COSTREET'  ;
  cnCoCity     = 'COCITY'    ;
  cnCoState    = 'COSTATE'   ;
  cnCoZIP      = 'COZIP'     ;
  cnCoCountry  = 'COCOUNTRY' ;
  cnCoHomepage = 'COHOMEPAGE';
  cnDisplayUID = 'DISPLAYUID';

//----- Contact info -----

function setcnftype2str(settype:integer):PWideChar;
var
  res:pWideChar;
begin
  case settype of
//    CNF_FIRSTNAME:  result:=translate(cnFirstName);
    CNF_LASTNAME:   res:=cnLastName  ;
    CNF_NICK:       res:=cnNick      ;
    CNF_CUSTOMNICK: res:=cnCustomNick;
    CNF_EMAIL:      res:=cnEmail     ;
    CNF_CITY:       res:=cnCity      ;
    CNF_STATE:      res:=cnState     ;
    CNF_COUNTRY:    res:=cnCountry   ;
    CNF_PHONE:      res:=cnPhone     ;
    CNF_HOMEPAGE:   res:=cnHomepage  ;
    CNF_ABOUT:      res:=cnAbout     ;
    CNF_GENDER:     res:=cnGender    ;
    CNF_AGE:        res:=cnAge       ;
    CNF_FIRSTLAST:  res:=cnFirstLast ;
    CNF_UNIQUEID:   res:=cnUniqueID  ;

    CNF_FAX:        res:=cnFax       ;
    CNF_CELLULAR:   res:=cnCellular  ;
    CNF_TIMEZONE:   res:=cnTimezone  ;
    CNF_MYNOTES:    res:=cnMyNotes   ;
    CNF_BIRTHDAY:   res:=cnBirthday  ;
    CNF_BIRTHMONTH: res:=cnBirthMonth;
    CNF_BIRTHYEAR:  res:=cnBirthYear ;
    CNF_STREET:     res:=cnStreet    ;
    CNF_ZIP:        res:=cnZIP       ;
    CNF_LANGUAGE1:  res:=cnLanguage1 ;
    CNF_LANGUAGE2:  res:=cnLanguage2 ;
    CNF_LANGUAGE3:  res:=cnLanguage3 ;
    CNF_CONAME:     res:=cnCoName    ;
    CNF_CODEPT:     res:=cnCoDept    ;
    CNF_COPOSITION: res:=cnCoPosition;
    CNF_COSTREET:   res:=cnCoStreet  ;
    CNF_COCITY:     res:=cnCoCity    ;
    CNF_COSTATE:    res:=cnCoState   ;
    CNF_COZIP:      res:=cnCoZIP     ;
    CNF_COCOUNTRY:  res:=cnCoCountry ;
    CNF_COHOMEPAGE: res:=cnCoHomepage;
    CNF_DISPLAYUID: res:=cnDisplayUID;
  else
    res:=cnFirstName;
  end;
  result:=TranslateW(res);
end;

procedure AddCnf(list:HWND;param:integer);
begin
//  CB_AddStrDataW(list,setcnftype2str(param),param);
  SendMessage(list,CB_SETITEMDATA,
    SendMessageW(list,CB_ADDSTRING,0,lparam(setcnftype2str(param))),
    param);
end;

procedure addsetcnftypes(list:HWND);
begin
  AddCnf(list,CNF_FIRSTNAME);
  AddCnf(list,CNF_LASTNAME);
  AddCnf(list,CNF_NICK);
  AddCnf(list,CNF_CUSTOMNICK);
  AddCnf(list,CNF_EMAIL);
  AddCnf(list,CNF_CITY);
  AddCnf(list,CNF_STATE);
  AddCnf(list,CNF_COUNTRY);
  AddCnf(list,CNF_PHONE);
  AddCnf(list,CNF_HOMEPAGE);
  AddCnf(list,CNF_ABOUT);
  AddCnf(list,CNF_GENDER);
  AddCnf(list,CNF_AGE);
  AddCnf(list,CNF_FIRSTLAST);
  AddCnf(list,CNF_UNIQUEID);

  AddCnf(list,CNF_FAX);
  AddCnf(list,CNF_CELLULAR);
  AddCnf(list,CNF_TIMEZONE);
  AddCnf(list,CNF_MYNOTES);
  AddCnf(list,CNF_BIRTHDAY);
  AddCnf(list,CNF_BIRTHMONTH);
  AddCnf(list,CNF_BIRTHYEAR);
  AddCnf(list,CNF_STREET);
  AddCnf(list,CNF_ZIP);
  AddCnf(list,CNF_LANGUAGE1);
  AddCnf(list,CNF_LANGUAGE2);
  AddCnf(list,CNF_LANGUAGE3);
  AddCnf(list,CNF_CONAME);
  AddCnf(list,CNF_CODEPT);
  AddCnf(list,CNF_COPOSITION);
  AddCnf(list,CNF_COSTREET);
  AddCnf(list,CNF_COCITY);
  AddCnf(list,CNF_COSTATE);
  AddCnf(list,CNF_COZIP);
  AddCnf(list,CNF_COCOUNTRY);
  AddCnf(list,CNF_COHOMEPAGE);
  AddCnf(list,CNF_DISPLAYUID);

  SendMessage(list,CB_SETCURSEL,0,0);
end;


function GetSelectedItem(list:HWND):integer;
begin
  result:=SendMessage(list,LVM_GETNEXTITEM,-1,LVNI_FOCUSED);
end;

//----- Common interface -----

function IconChanged(wParam:WPARAM;lParam:LPARAM;lParam1:LPARAM):int;cdecl;
begin
  result:=0;
  SetButtonIcon(GetDlgItem(lParam1,IDC_NEW    ),QS_NEW);
  SetButtonIcon(GetDlgItem(lParam1,IDC_UP     ),QS_UP);
  SetButtonIcon(GetDlgItem(lParam1,IDC_DN     ),QS_DOWN);
  SetButtonIcon(GetDlgItem(lParam1,IDC_DELETE ),QS_DELETE);
  SetButtonIcon(GetDlgItem(lParam1,IDC_DEFAULT),QS_DEFAULT);
  SetButtonIcon(GetDlgItem(lParam1,IDC_RELOAD ),QS_RELOAD);
end;

procedure CheckDirection(Dialog:HWND;item:integer);
begin
  EnableWindow(GetDlgItem(Dialog,IDC_UP),item>0);

  EnableWindow(GetDlgItem(Dialog,IDC_DN),
      item<(SendMessage(GetDlgItem(Dialog,IDC_LIST),LVM_GETITEMCOUNT,0,0)-1));
end;

//----- Basic screen functions -----

procedure InitScreen(Dialog:HWND);
begin
  // setting
  SendMessage(GetDlgItem(Dialog,IDC_C_DATATYPE),CB_SETCURSEL,0,0);
  SetDlgItemTextW(Dialog,IDC_E_MODULE ,nil);
  SetDlgItemTextW(Dialog,IDC_E_SETTING,nil);
  // script
  SetDlgItemTextW(Dialog,IDC_E_SCRIPT,nil);
  // service
  ClearServiceBlock(ServiceBlock);
  // contact info
  SendMessage(GetDlgItem(Dialog,IDC_C_CNFTYPE),CB_SETCURSEL,0,0);
  // others
  SendMessage(GetDlgItem(Dialog,IDC_C_OTHER),CB_SETCURSEL,0,0);
end;

procedure ClearScreen(Dialog:HWND);
begin
  // setting
  ShowWindow(GetDlgItem(Dialog,IDC_S_DATATYPE),SW_HIDE);
  ShowWindow(GetDlgItem(Dialog,IDC_C_DATATYPE),SW_HIDE);
  ShowWindow(GetDlgItem(Dialog,IDC_S_MODULE  ),SW_HIDE);
  ShowWindow(GetDlgItem(Dialog,IDC_E_MODULE  ),SW_HIDE);
  ShowWindow(GetDlgItem(Dialog,IDC_S_SETTING ),SW_HIDE);
  ShowWindow(GetDlgItem(Dialog,IDC_E_SETTING ),SW_HIDE);

  // script
  ShowEditField(Dialog,IDC_E_SCRIPT,SW_HIDE);

  // service
  ShowWindow(ServiceBlock,SW_HIDE);

  // contact info
  ShowWindow(GetDlgItem(Dialog,IDC_S_CNFTYPE),SW_HIDE);
  ShowWindow(GetDlgItem(Dialog,IDC_C_CNFTYPE),SW_HIDE);

  // others
  ShowWindow(GetDlgItem(Dialog,IDC_C_OTHER),SW_HIDE);
end;

procedure SetupScreen(Dialog:HWND;code:integer);
begin
  if not IsWindowVisible(GetDlgItem(Dialog,IDC_E_TITLE)) then
    exit;

  case code of
    // setting
    QST_SETTING: begin
      ShowWindow(GetDlgItem(Dialog,IDC_S_DATATYPE),SW_SHOW);
      ShowWindow(GetDlgItem(Dialog,IDC_C_DATATYPE),SW_SHOW);
      ShowWindow(GetDlgItem(Dialog,IDC_S_MODULE  ),SW_SHOW);
      ShowWindow(GetDlgItem(Dialog,IDC_E_MODULE  ),SW_SHOW);
      ShowWindow(GetDlgItem(Dialog,IDC_S_SETTING ),SW_SHOW);
      ShowWindow(GetDlgItem(Dialog,IDC_E_SETTING ),SW_SHOW);
    end;

    // script
    QST_SCRIPT: begin
      ShowEditField(Dialog,IDC_E_SCRIPT,SW_SHOW);
    end;

    // service
    QST_SERVICE: begin
      ShowWindow(ServiceBlock,SW_SHOW);
    end;

    // contact info
    QST_CONTACTINFO: begin
      ShowWindow(GetDlgItem(Dialog,IDC_S_CNFTYPE),SW_SHOW);
      ShowWindow(GetDlgItem(Dialog,IDC_C_CNFTYPE),SW_SHOW);
    end;

    // others
    QST_OTHER: begin
      ShowWindow(GetDlgItem(Dialog,IDC_C_OTHER),SW_SHOW);
    end;
  end;
end;

//----- single column processing -----

procedure FillTableLine(list:HWND;item:integer;const column:tcolumnitem);
begin
  LV_SetItemW(list,column.title,item,1);
  case column.setting_type of
    QST_SETTING: begin
      LV_SetItem(list,column.module ,item,2);
      LV_SetItem(list,column.setting,item,3);
    end;
    QST_SCRIPT: begin
      LV_SetItemW(list,TranslateW('Script'),item,2);
//      LV_SetItemW(list,column.script,item,3);
    end;
    QST_SERVICE: begin
      LV_SetItemW(list,TranslateW('Service'),item,2);
      LV_SetItem (list,column.service       ,item,3);
    end;
    QST_CONTACTINFO: begin
      LV_SetItemW(list,TranslateW('Contact info')    ,item,2);
      LV_SetItemW(list,setcnftype2str(column.cnftype),item,3);
    end;
    QST_OTHER: begin
      case column.other of
        QSTO_METACONTACT: begin
          LV_SetItemW(list,TranslateW('Metacontact'),item,2);
        end;
      end;
    end;
  end;
end;

function savecuritem(Dialog:HWND):integer;
var
  list:HWND;
  srvalue:tServiceValue;
  i:integer;
  idx,lwidth:integer;
begin
  list:=GetDlgItem(Dialog,IDC_LIST);
  i:=GetSelectedItem(list);
  idx:=LV_GetLParam(list,i);
  result:=idx;

  lwidth:=editcolumns[idx].width;
  clear_column(editcolumns[idx]);
  with editcolumns[idx] do
  begin
    flags:=0;

    if ListView_GetCheckState(list,i)=BST_CHECKED then
      flags:=flags or COL_ON or COL_FILTER;

    setting_type:=CB_GetData(GetDlgItem(Dialog,IDC_C_VARTYPE));
    
    title:=GetDlgText(Dialog,IDC_E_TITLE);
    width:=lwidth;

    case setting_type of
      QST_SETTING: begin
        datatype:=CB_GetData(GetDlgItem(Dialog,IDC_C_DATATYPE));
        module  :=GetDlgText(Dialog,IDC_E_MODULE ,true);
        setting :=GetDlgText(Dialog,IDC_E_SETTING,true);
      end;

      QST_SCRIPT: begin
        script:=GetDlgText(Dialog,IDC_E_SCRIPT);
      end;

      QST_CONTACTINFO: begin
        cnftype:=CB_GetData(GetDlgItem(Dialog,IDC_C_CNFTYPE));
      end;

      QST_SERVICE: begin
        GetSrvBlockValue(ServiceBlock,srvalue);
        service     :=srvalue.service;
        wparam.value:=srvalue.wparam;
        wparam._type:=srvalue.w_flag;
        lparam.value:=srvalue.lparam;
        lparam._type:=srvalue.l_flag;
        restype     :=srvalue.flags;
      end;

      QST_OTHER: begin
        other:=CB_GetData(GetDlgItem(Dialog,IDC_C_OTHER));
      end;
    end;
  end;
  FillTableLine(list,i,editcolumns[idx]);
end;

procedure displcurinfo(Dialog:HWND;const column:tcolumnitem);
var
  srvalue:tServiceValue;
begin
  ClearScreen(Dialog);
  SetupScreen(Dialog,column.setting_type);

  SetDlgItemTextW(Dialog,IDC_E_TITLE,column.title);
  CB_SelectData(Dialog,IDC_C_VARTYPE,column.setting_type);

  case column.setting_type of
    QST_SETTING: begin
      CB_SelectData  (Dialog,IDC_C_DATATYPE,column.datatype);
      SetDlgItemTextA(Dialog,IDC_E_MODULE  ,column.module);
      SetDlgItemTextA(Dialog,IDC_E_SETTING ,column.setting);
    end;

    QST_SCRIPT: begin
      SetDlgItemTextW(Dialog,IDC_E_SCRIPT,column.script);
    end;

    QST_SERVICE: begin
      srvalue.service:=column.service;
      srvalue.wparam :=pointer(column.wparam.value);
      srvalue.w_flag :=column.wparam._type;
      srvalue.lparam :=pointer(column.lparam.value);
      srvalue.l_flag :=column.lparam._type;
      srvalue.flags  :=column.restype;
      SetSrvBlockValue(ServiceBlock,srvalue);
    end;

    QST_CONTACTINFO: begin
      CB_SelectData(Dialog,IDC_C_CNFTYPE,column.cnftype);
    end;

    QST_OTHER: begin
      CB_SelectData(Dialog,IDC_C_OTHER,column.other);
    end;
  end;
end;

function add_column(list:HWND;i,idx:integer):integer;
var
  li:LV_ITEMA;
begin
  result:=i;
  zeromemory(@li,sizeof(li));
  li.mask  :=LVIF_PARAM;
  li.lParam:=idx;
  li.iItem :=i;
  SendMessage(list,LVM_INSERTITEM,0,lparam(@li));
end;

procedure update_list(list:HWND);
var
  i,cnt:integer;
begin
  ListView_DeleteAllItems(list);

  cnt:=0;
  for i:=0 to MaxColumnAmount-1 do
  begin
    if editcolumns[i].setting_type<>0 then
    begin
      add_column(list,cnt,i);
      FillTableLine(list,cnt,editcolumns[i]);
      ListView_SetCheckState(list,cnt,(editcolumns[i].flags and COL_ON)<>0);
      inc(cnt);
    end;
  end;

  ListView_SetItemState(list,0,
      LVIS_FOCUSED or LVIS_SELECTED,
      LVIS_FOCUSED or LVIS_SELECTED);
end;

// fill comboboxes lists

procedure adddbsettypes(list:HWND);
begin
  CB_AddStrDataW(list,TranslateW(stByte)     ,QSTS_BYTE);
  CB_AddStrDataW(list,TranslateW(stWord)     ,QSTS_WORD);
  CB_AddStrDataW(list,TranslateW(stDWord)    ,QSTS_DWORD);
  CB_AddStrDataW(list,TranslateW(stSigned)   ,QSTS_SIGNED);
  CB_AddStrDataW(list,TranslateW(stHexnum)   ,QSTS_HEXNUM);
  CB_AddStrDataW(list,TranslateW(stString)   ,QSTS_STRING);
  CB_AddStrDataW(list,TranslateW(stIP)       ,QSTS_IP);
  CB_AddStrDataW(list,TranslateW(stTimeStamp),QSTS_TIMESTAMP);

  SendMessage(list,CB_SETCURSEL,0,0);
end;

procedure addothertypes(list:HWND);
begin
  CB_AddStrDataW(list,TranslateW(stLastSeen)   ,QSTO_LASTSEEN);
  CB_AddStrDataW(list,TranslateW(stLastEvent)  ,QSTO_LASTEVENT);
  CB_AddStrDataW(list,TranslateW(stMetacontact),QSTO_METACONTACT);
  CB_AddStrDataW(list,TranslateW(stEventCount) ,QSTO_EVENTCOUNT);

  SendMessage(list,CB_SETCURSEL,0,0);
end;

procedure addsettypes(list:HWND);
begin
  CB_AddStrDataW(list,TranslateW(stSetting),QST_SETTING);

  CB_AddStrDataW(list,TranslateW(stScript)     ,QST_SCRIPT);
  CB_AddStrDataW(list,TranslateW(stService)    ,QST_SERVICE);
  CB_AddStrDataW(list,TranslateW(stContactInfo),QST_CONTACTINFO);

  CB_AddStrDataW(list,TranslateW(stOther),QST_OTHER);

  SendMessage(list,CB_SETCURSEL,0,0);
end;

procedure addcolumn(handle:HWND;width:word;title:PWideChar);
var
  lvcol:LV_COLUMNW;
begin
  lvcol.mask   :=LVCF_TEXT or LVCF_WIDTH;
  lvcol.cx     :=width;
  lvcol.pszText:=TranslateW(title);
  SendMessageW(handle,LVM_INSERTCOLUMNW,0,lparam(@lvcol));
end;

function NewListProc(Dialog:HWND;hMessage:uint;wParam:WPARAM;lParam:LPARAM):LRESULT; stdcall;
begin
  result:=0;
  if hMessage=WM_KEYDOWN then
  begin
    case wParam of
      VK_INSERT: begin
        PostMessage(GetParent(Dialog),WM_COMMAND,(BN_CLICKED shl 16)+IDC_NEW,0);
        exit;
      end;
      VK_DELETE: begin
        PostMessage(GetParent(Dialog),WM_COMMAND,(BN_CLICKED shl 16)+IDC_DELETE,0);
        exit;
      end;
      VK_UP: begin
        if (GetKeyState(VK_CONTROL) and $8000)<>0 then
        begin
          PostMessage(GetParent(Dialog),WM_COMMAND,(BN_CLICKED shl 16)+IDC_UP,0);
          exit;
        end;
      end;
      VK_DOWN: begin
        if (GetKeyState(VK_CONTROL) and $8000)<>0 then
        begin
          PostMessage(GetParent(Dialog),WM_COMMAND,(BN_CLICKED shl 16)+IDC_DN,0);
          exit;
        end;
      end;
    end;
  end;
  result:=CallWindowProc(OldListProc,Dialog,hMessage,wParam,lParam);
end;

procedure ResizeControl(Dialog:HWND;id:integer;width:integer);
var
  wnd:HWND;
  rc:TRECT;
begin
  wnd:=GetDlgItem(Dialog,id);
  GetWindowRect(wnd,rc);
  SetWindowPos(wnd,0,0,0,width,rc.bottom-rc.top,SWP_NOMOVE or SWP_NOZORDER or SWP_SHOWWINDOW);
end;

procedure DoResize(Dialog:HWND);
var
  wSeparator:HWND;
  pcw:pWideChar;
  rc,rc1:TRECT;
  pt:TPOINT;
  dx,rside:integer;
begin
  GetClientRect(Dialog,rc);
  wSeparator:=GetDlgItem(Dialog,IDC_B_RESIZE);
  GetWindowRect(wSeparator,rc1);
  pt.x:=rc1.left;
  pt.y:=0;
  ScreenToClient(Dialog,pt);
  if pt.x<(rc.right-50) then //!!
  begin
    rside:=SW_HIDE;
    dx:=rc.right-(rc1.right-rc1.left)-4;
    pcw:='<';
  end
  else
  begin
    rside:=SW_SHOW;

    GetWindowRect(GetDlgItem(Dialog,IDC_S_COLSETTING),rc);
    pt.x:=rc.left;
    pt.y:=0;
    ScreenToClient(Dialog,pt);
    dx:=pt.x-(rc1.right-rc1.left)-4;
    pcw:='>';
  end;
  SendMessageW(wSeparator,WM_SETTEXT,0,lparam(pcw));

  // move separator button
  SetWindowPos(wSeparator,0,dx+2,2,0,0,SWP_NOSIZE or SWP_NOZORDER or SWP_SHOWWINDOW);

  // resize left side controls
  ResizeControl(Dialog,IDC_LIST    ,dx);
  ResizeControl(Dialog,IDC_CH_GROUP,dx);

  ResizeControl(Dialog,IDC_CH_USETOOLSTYLE,dx-8);
  ResizeControl(Dialog,IDC_CH_DRAWGRID    ,dx-8);
  ResizeControl(Dialog,IDC_CH_SAVEPATTERN ,dx-8);
  ResizeControl(Dialog,IDC_CH_AUTOCLOSE   ,dx-8);
  ResizeControl(Dialog,IDC_CH_SORTSTATUS  ,dx-8);
  ResizeControl(Dialog,IDC_CH_CLIENTICONS ,dx-8);

  // show/hide setting block (ugly, i know!)
  ShowWindow(GetDlgItem(Dialog,IDC_S_COLSETTING),rside);
  ShowWindow(GetDlgItem(Dialog,IDC_S_LINE      ),rside);
  ShowWindow(GetDlgItem(Dialog,IDC_S_TITLE     ),rside);
  ShowWindow(GetDlgItem(Dialog,IDC_E_TITLE     ),rside);
  ShowWindow(GetDlgItem(Dialog,IDC_S_VARTYPE   ),rside);
  ShowWindow(GetDlgItem(Dialog,IDC_C_VARTYPE   ),rside);
  ShowWindow(GetDlgItem(Dialog,IDC_SETITEM     ),rside);

  ClearScreen(Dialog);

  if rside=SW_SHOW then
  begin
    SetupScreen(Dialog,CB_GetData(GetDlgItem(Dialog,IDC_C_VARTYPE)));
  end;
end;

procedure SetButtons(Dialog:HWND);
var
//  ti:TTOOLINFOW;
//  hwndTooltip:HWND;
  hNew    :HWND;
  hUp     :HWND;
  hDown   :HWND;
  hDelete :HWND;
  hDefault:HWND;
  hReload :HWND;
begin
{
  hwndTooltip:=CreateWindowW(TOOLTIPS_CLASS,nil,TTS_ALWAYSTIP,
      integer(CW_USEDEFAULT),integer(CW_USEDEFAULT),
      integer(CW_USEDEFAULT),integer(CW_USEDEFAULT),
      Dialog,0,hInstance,nil);
}
  hNew    :=GetDlgItem(Dialog,IDC_NEW);
  SendMessage(hNew,BUTTONADDTOOLTIP,TWPARAM(TranslateW('New')),BATF_UNICODE);
  hUp     :=GetDlgItem(Dialog,IDC_UP);
  SendMessage(hUp,BUTTONADDTOOLTIP,TWPARAM(TranslateW('Up')),BATF_UNICODE);
  hDown   :=GetDlgItem(Dialog,IDC_DN);
  SendMessage(hDown,BUTTONADDTOOLTIP,TWPARAM(TranslateW('Down')),BATF_UNICODE);
  hDelete :=GetDlgItem(Dialog,IDC_DELETE);
  SendMessage(hDelete,BUTTONADDTOOLTIP,TWPARAM(TranslateW('Delete')),BATF_UNICODE);
  hDefault:=GetDlgItem(Dialog,IDC_DEFAULT);
  SendMessage(hDefault,BUTTONADDTOOLTIP,TWPARAM(TranslateW('Default')),BATF_UNICODE);
  hReload :=GetDlgItem(Dialog,IDC_RELOAD);
  SendMessage(hReload,BUTTONADDTOOLTIP,TWPARAM(TranslateW('Reload')),BATF_UNICODE);
{
  FillChar(ti,SizeOf(ti),0);
  ti.cbSize  :=sizeof(TOOLINFO);
  ti.uFlags  :=TTF_IDISHWND or TTF_SUBCLASS;
  ti.hwnd    :=Dialog;
  ti.hinst   :=hInstance;
  ti.uId     :=hNew;
  ti.lpszText:=TranslateW('New');
  SendMessageW(hwndTooltip,TTM_ADDTOOLW,0,tlparam(@ti));
  ti.uId     :=hItem;
  ti.lpszText:=TranslateW('Save Item');
  SendMessageW(hwndTooltip,TTM_ADDTOOLW,0,tlparam(@ti));
  ti.uId     :=hUp;
  ti.lpszText:=TranslateW('Up');
  SendMessageW(hwndTooltip,TTM_ADDTOOLW,0,tlparam(@ti));
  ti.uId     :=hDown;
  ti.lpszText:=TranslateW('Down');
  SendMessageW(hwndTooltip,TTM_ADDTOOLW,0,tlparam(@ti));
  ti.uId     :=hDelete;
  ti.lpszText:=TranslateW('Delete');
  SendMessageW(hwndTooltip,TTM_ADDTOOLW,0,tlparam(@ti));
  ti.uId     :=hDefault;
  ti.lpszText:=TranslateW('Default');
  SendMessageW(hwndTooltip,TTM_ADDTOOLW,0,tlparam(@ti));
  ti.uId     :=hReload;
  ti.lpszText:=TranslateW('Reload');
  SendMessageW(hwndTooltip,TTM_ADDTOOLW,0,tlparam(@ti));
}
  SetButtonIcon(hNew    ,QS_NEW);
  SetButtonIcon(hUp     ,QS_UP);
  SetButtonIcon(hDown   ,QS_DOWN);
  SetButtonIcon(hDelete ,QS_DELETE);
  SetButtonIcon(hDefault,QS_DEFAULT);
  SetButtonIcon(hReload ,QS_RELOAD);

end;

procedure AddControls(Dialog:HWND);
var
  rc:TRECT;
  pt,pt1:TPOINT;
  width,height:integer;
begin
  // Settings
  // Script
  MakeEditField(Dialog, IDC_E_SCRIPT);
  SetEditFlags (Dialog, IDC_E_SCRIPT,EF_FORCE,EF_FORCES);

  // Service
  GetWindowRect(GetDlgItem(Dialog,IDC_C_VARTYPE),rc);
  width:=rc.right-rc.left;
  pt.x:=rc.left;
  pt.y:=rc.bottom;
  ScreenToClient(Dialog,pt);

  GetWindowRect(GetDlgItem(Dialog,IDC_SETITEM),rc);
  pt1.x:=rc.left;
  pt1.y:=rc.top;
  ScreenToClient(Dialog,pt1);

  height:=pt1.y-pt.y-2;
  ServiceBlock:=CreateServiceBlock(Dialog,pt.x,pt.y,width,height,ACF_NOSTRUCT);

  // Contact info
  // Other
end;

function DlgProcOptions(Dialog:HWND;hMessage:uint;wParam:WPARAM;lParam:LPARAM):LRESULT; stdcall;
const
  InitDlg:bool = true;
  hook:THANDLE = 0;
var
  pc:pWideChar;
  i,idx:integer;
  itemsel:integer;
  listhwnd:HWND;
  tmpbool:bool;
begin
  result:=0;

  case hMessage of
    WM_DESTROY: begin
      clear_columns(editcolumns);

      if hook<>0 then
        UnhookEvent(hook);

      listhwnd:=GetDlgItem(Dialog,IDC_LIST);
      DBWriteWord(0,qs_module,'col1',ListView_GetColumnWidth(listhwnd,1));
      DBWriteWord(0,qs_module,'col2',ListView_GetColumnWidth(listhwnd,2));
      DBWriteWord(0,qs_module,'col3',ListView_GetColumnWidth(listhwnd,3));
    end;

    WM_INITDIALOG: begin
      InitDlg:=true;
      listhwnd:=GetDlgItem(Dialog,IDC_LIST);

      SendMessageW(listhwnd,LVM_SETEXTENDEDLISTVIEWSTYLE,0,
        SendMessageW(listhwnd,LVM_GETEXTENDEDLISTVIEWSTYLE,0,0) or
          LVS_EX_FULLROWSELECT or LVS_EX_CHECKBOXES);

      addcolumn(listhwnd,DBReadWord(0,qs_module,'col3',95) ,'Setting');
      addcolumn(listhwnd,DBReadWord(0,qs_module,'col2',105),'Module/InfoType');
      addcolumn(listhwnd,DBReadWord(0,qs_module,'col1',85) ,'Title');
      addcolumn(listhwnd,20 ,'#');

      addsettypes   (GetDlgItem(Dialog,IDC_C_VARTYPE));
      adddbsettypes (GetDlgItem(Dialog,IDC_C_DATATYPE));
      addothertypes (GetDlgItem(Dialog,IDC_C_OTHER));
      addsetcnftypes(GetDlgItem(Dialog,IDC_C_CNFTYPE));

      CheckDlgButton(Dialog,IDC_CH_SORTSTATUS  ,ORD((qsopt.flags and QSO_SORTBYSTATUS)<>0));
      CheckDlgButton(Dialog,IDC_CH_AUTOCLOSE   ,ORD((qsopt.flags and QSO_AUTOCLOSE)<>0));
      CheckDlgButton(Dialog,IDC_CH_USETOOLSTYLE,ORD((qsopt.flags and QSO_TOOLSTYLE)<>0));
      CheckDlgButton(Dialog,IDC_CH_DRAWGRID    ,ORD((qsopt.flags and QSO_DRAWGRID)<>0));
      CheckDlgButton(Dialog,IDC_CH_SAVEPATTERN ,ORD((qsopt.flags and QSO_SAVEPATTERN)<>0));
      CheckDlgButton(Dialog,IDC_CH_CLIENTICONS ,ORD((qsopt.flags and QSO_CLIENTICONS)<>0));

      SetButtons(Dialog);

      AddControls(Dialog);

      TranslateDialogDefault(Dialog);

      CloneColumns(editcolumns,qsopt.columns);

      update_list(listhwnd);

      hook:=HookEventParam(ME_SKIN2_ICONSCHANGED,@IconChanged,Dialog);

      result:=1;

      OldListProc:=pointer(SetWindowLongPtrW(listhwnd,GWL_WNDPROC,LONG_PTR(@NewListProc)));

      DoResize(Dialog);

      InitDlg:=false;
    end;

    WM_NOTIFY: begin
      case integer(PNMHdr(lParam)^.code) of

        PSN_APPLY: begin
          // checkboxes
          qsopt.flags:=qsopt.flags and not QSO_MAINOPTIONS;

          if IsDlgButtonChecked(Dialog,IDC_CH_SORTSTATUS)<>BST_UNCHECKED then
            qsopt.flags:=qsopt.flags or QSO_SORTBYSTATUS;

          if IsDlgButtonChecked(Dialog,IDC_CH_AUTOCLOSE)<>BST_UNCHECKED then
            qsopt.flags:=qsopt.flags or QSO_AUTOCLOSE;

          if IsDlgButtonChecked(Dialog,IDC_CH_USETOOLSTYLE)<>BST_UNCHECKED then
            qsopt.flags:=qsopt.flags or QSO_TOOLSTYLE;

          if IsDlgButtonChecked(Dialog,IDC_CH_DRAWGRID)<>BST_UNCHECKED then
            qsopt.flags:=qsopt.flags or QSO_DRAWGRID;

          if IsDlgButtonChecked(Dialog,IDC_CH_CLIENTICONS)<>BST_UNCHECKED then
            qsopt.flags:=qsopt.flags or QSO_CLIENTICONS;

          if IsDlgButtonChecked(Dialog,IDC_CH_SAVEPATTERN)<>BST_UNCHECKED then
            qsopt.flags:=qsopt.flags or QSO_SAVEPATTERN;

          tmpbool:=CloseSrWindow(false);

          listhwnd:=GetDlgItem(Dialog,IDC_LIST);
          clear_columns(qsopt.columns);
          qsopt.numcolumns:=SendMessage(listhwnd,LVM_GETITEMCOUNT,0,0);
          for i:=0 to qsopt.numcolumns-1 do
          begin
            idx:=LV_GetLParam(listhwnd,i);
            with editcolumns[idx] do
            begin
              if ListView_GetCheckSTate(listhwnd,i)=0 then
                flags:=flags and not COL_ON
              else
                flags:=flags or COL_ON or COL_FILTER;
            end;
            CloneColumn(qsopt.columns[i],editcolumns[idx]);
          end;

          saveopt_db;

          if tmpbool then
            OpenSrWindow(nil,qsopt.flags);
          result:=1;
        end;

        LVN_ITEMCHANGED: begin
          if wParam=IDC_LIST then
          begin
            i:=(PNMLISTVIEW(lParam)^.uOldState and LVNI_FOCUSED)-
               (PNMLISTVIEW(lParam)^.uNewState and LVNI_FOCUSED);
            if i<0 then // new focus
            begin
              CheckDirection(Dialog,PNMLISTVIEW(lParam)^.iItem);
              InitScreen(Dialog);
              displcurinfo(Dialog,editcolumns[PNMLISTVIEW(lParam)^.lParam]);

              result:=1;
            end else if (i=0) and not InitDlg then
            begin
              if (PNMLISTVIEW(lParam)^.uOldState or PNMLISTVIEW(lParam)^.uNewState)=$3000 then
              begin
                i:=PNMLISTVIEW(lParam)^.uOldState-PNMLISTVIEW(lParam)^.uNewState;
                if abs(i)=$1000 then
                begin
                  SendMessage(GetParent(Dialog),PSM_CHANGED,0,0);
                  result:=1;
                end;
              end;
            end;
          end;
        end;

      end;
    end;

    WM_COMMAND: begin
      case wParam shr 16 of
        CBN_SELCHANGE: begin
          if loword(wParam)=IDC_C_VARTYPE then
          begin
            ClearScreen(Dialog);
            SetupScreen(Dialog,CB_GetData(lParam));
          end;
        end;

        BN_CLICKED: begin
          if loword(wParam)<>IDC_B_RESIZE then
            SendMessage(GetParent(Dialog),PSM_CHANGED,0,0);

          listhwnd:=GetDlgItem(Dialog,IDC_LIST);
          result:=1;
          case loword(wParam) of
            IDC_NEW: begin
              i:=GetSelectedItem(listhwnd)+1;
              add_column(listhwnd,i,new_column(editcolumns));

              SendMessage(listhwnd,LVM_ENSUREVISIBLE,i,0);
              ListView_SetItemState(listhwnd,i,
                  LVIS_FOCUSED+LVIS_SELECTED,LVIS_FOCUSED+LVIS_SELECTED);
              InitScreen(Dialog);
              CheckDirection(Dialog,i);
              EnableWindow(GetDlgItem(Dialog,IDC_DELETE),true);
            end;

            IDC_DELETE: begin
              i:=GetSelectedItem(listhwnd);
              clear_column(editcolumns[LV_GetLParam(listhwnd,i)]);
              SendMessage(listhwnd,LVM_DELETEITEM,i,0);

              idx:=SendMessage(listhwnd,LVM_GETITEMCOUNT,0,0);
              if idx=0 then
              begin
                EnableWindow(lParam,false);
                InitScreen(Dialog);
              end
              else
              begin
                if i=idx then dec(i);
                ListView_SetItemState(listhwnd,i,
                    LVIS_FOCUSED+LVIS_SELECTED,LVIS_FOCUSED+LVIS_SELECTED);
              end;
              CheckDirection(Dialog,i);
            end;

            IDC_UP: begin
              itemsel:=GetSelectedItem(listhwnd);
              if itemsel>0 then
              begin
                LV_MoveItem(listhwnd,-1,itemsel);
                CheckDirection(Dialog,itemsel-1);
              end;
            end;

            IDC_DN: begin
              itemsel:=GetSelectedItem(listhwnd);
              if itemsel>=0 then
              begin
                LV_MoveItem(listhwnd,1,itemsel);
                CheckDirection(Dialog,itemsel+1);
              end;
            end;

            IDC_SETITEM: begin
              if SendMessage(listhwnd,LVM_GETITEMCOUNT,0,0)=0 then
              begin
                add_column(listhwnd,0,0);
                ListView_SetItemState(listhwnd,0,
                    LVIS_FOCUSED+LVIS_SELECTED,LVIS_FOCUSED+LVIS_SELECTED);
                EnableWindow(GetDlgItem(Dialog,IDC_DELETE),true);
              end;
              savecuritem(Dialog);
            end;

            IDC_DEFAULT: begin
              loaddefaultcolumns(editcolumns);
              update_list(listhwnd);
            end;

            IDC_RELOAD: begin
              loadopt_db(editcolumns);
              update_list(listhwnd);
            end;

            IDC_B_RESIZE: begin
              DoResize(Dialog);
            end;
          else
            result:=0;
          end;
        end;
      end;
    end;

    WM_HELP: begin
      case CB_GetData(GetDlgItem(Dialog,IDC_C_VARTYPE)) of
        QST_SETTING: begin
          MessageBoxW(0,
              TranslateW('Column content is simple database setting.'),
              TranslateW('DB setting'),0);
        end;
        QST_SCRIPT: begin
          MessageBoxW(0,
              TranslateW('Column content is script result.'#13#10+
                         'More help from "Help" button in script dialog.'),
              TranslateW('Script'),0);
        end;
        QST_SERVICE: begin
          SendMessage(ServiceBlock,WM_HELP,0,0);
        end;
        QST_CONTACTINFO: begin
          MessageBoxW(0,
              TranslateW('Column content is contact property (see list). Can be empty.'),
              TranslateW('ContactInfo'),0);
        end;
        QST_OTHER: begin
          case CB_GetData(GetDlgItem(Dialog,IDC_C_OTHER)) of
            QSTO_LASTSEEN: begin
              pc:='Content is last online time.';
            end;
            QSTO_LASTEVENT: begin
              pc:='Content is time of last contact event.';
            end;
            QSTO_METACONTACT: begin
              pc:='Content is metacontact info.';
            end;
            QSTO_EVENTCOUNT: begin
              pc:='Content is count of ALL contact events (not messages only)';
            end;
          else
            pc:='Unknown';
          end;
          MessageBoxW(0,TranslateW(pc),TranslateW('Other info'),0);
        end;
      end;
      result:=1;
    end;

//  else
//    result:=DefWindowProc(Dialog,hMessage,wParam,lParam);
  end;
  DefWindowProc(Dialog,hMessage,wParam,lParam);
end;

end.
