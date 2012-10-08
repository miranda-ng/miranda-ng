unit sr_optdialog;

interface
uses windows;

{.$include resource.inc}

procedure OptChangeColumns(code:integer;column,data:integer);
function DlgProcOptions(Dialog:HWnd;hMessage:UINT;wParam:WPARAM;lParam:LPARAM):lresult; stdcall;

const
  maindlg:HWND = 0;

implementation

uses messages,commctrl,sr_global,m_api,common,mirutils,wrapper,sr_window;

var
  OldListProc:pointer;
const
  curscript:pointer=nil;

const
  stByte       :PAnsiChar = 'Byte';
  stWord       :PAnsiChar = 'Word';
  stInt        :PAnsiChar = 'Int';
  stString     :PAnsiChar = 'String';
  stLastSeen   :PAnsiChar = 'LastSeen';
  stIP         :PAnsiChar = 'IP';
  stContactInfo:PAnsiChar = 'ContactInfo';
  stLastEvent  :PAnsiChar = 'LastEvent';
  stTimeStamp  :PAnsiChar = 'TimeStamp';
  stService    :PAnsiChar = 'Service';
  stScript     :PAnsiChar = 'Script';
  stMetacontact:PAnsiChar = 'Metacontact';

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
  cnZIP        = 'ZIP'       ;
  cnStreet     = 'STREET'    ;
  cnLanguage1  = 'LANGUAGE1' ;
  cnLanguage2  = 'LANGUAGE2' ;
  cnLanguage3  = 'LANGUAGE3' ;
  cnCoName     = 'CONAME'    ;

const
  strNotSelected = 'Not Selected';

const
  MaxControls = 13;
  aIdElement:array [0..MaxControls-1] of integer = (
    IDC_SCRIPT, IDC_STAT_VARTYPE, IDC_C_CNFTYPE, IDC_C_RESULT,
    IDC_STAT_SERVICE, IDC_STAT_MODULE, IDC_E_MODULE,
    IDC_STAT_WPAR, IDC_STAT_SETTING, IDC_E_VAR,
    IDC_C_WPAR, IDC_E_LPAR, IDC_C_LPAR);

  aShowElement:array [0..ST_MAXTYPE,0..MaxControls-1] of byte = (
{ST_BYTE       } ($00, $80, $80, $00,  $00, $81, $81,  $00, $81, $81,  $80, $80, $80),
{ST_WORD       } ($00, $80, $80, $00,  $00, $81, $81,  $00, $81, $81,  $80, $80, $80),
{ST_INT        } ($00, $80, $80, $00,  $00, $81, $81,  $00, $81, $81,  $80, $80, $80),
{ST_STRING     } ($00, $80, $80, $00,  $00, $81, $81,  $00, $81, $81,  $80, $80, $80),
{ST_IP         } ($00, $80, $80, $00,  $00, $81, $81,  $00, $81, $81,  $80, $80, $80),
{ST_LASTSEEN   } ($00, $80, $80, $00,  $00, $81, $81,  $00, $81, $81,  $80, $80, $80),
{ST_CONTACTINFO} ($00, $81, $81, $00,  $00, $80, $80,  $00, $80, $80,  $80, $80, $80),
{ST_LASTEVENT  } ($00, $80, $80, $00,  $00, $80, $80,  $00, $80, $80,  $80, $80, $80),
{ST_TIMESTAMP  } ($00, $80, $80, $00,  $00, $81, $81,  $00, $81, $81,  $80, $80, $80),
{ST_SERVICE    } ($00, $80, $00, $81,  $81, $00, $81,  $81, $00, $81,  $81, $81, $81),
{ST_SCRIPT     } ($81, $00, $00, $00,  $00, $80, $80,  $00, $80, $80,  $80, $80, $80),
{ST_METACONTACT} ($00, $80, $80, $00,  $00, $80, $80,  $00, $80, $80,  $80, $80, $80));

procedure SetupControls(Dialog:HWND; aType:integer);
var
  i,j: cardinal;
  wnd:HWND;
begin
  for i:=0 to MaxControls-1 do
  begin
    j :=aShowElement[aType][i];
    wnd:=GetDlgItem(Dialog,aIdElement[i]);
    EnableWindow(wnd,odd(j));
    if shortint(j)<0 then
      j:=SW_SHOW
    else
      j:=SW_HIDE;
    ShowWindow(wnd,j);
  end;
end;

function settype2str(settype:integer):pointer;
var
  tmp:array [0..127] of WideChar;
begin
  case settype of
//    ST_BYTE:        result:=stByte;
    ST_WORD:        result:=stWord;
    ST_INT:         result:=stInt;
    ST_STRING:      result:=stString;
    ST_LASTSEEN:    result:=stLastSeen;
    ST_IP:          result:=stIP;
    ST_CONTACTINFO: result:=stContactInfo;
    ST_LASTEVENT:   result:=stLastEvent;
    ST_TIMESTAMP:   result:=stTimeStamp;
    ST_SERVICE:     result:=stService;
    ST_SCRIPT:      result:=stScript;
    ST_METACONTACT: result:=stMetacontact;
  else
    result:=stByte;
  end;

  FastAnsiToWideBuf(result,tmp);
  StrDupW(pWideChar(result),TranslateW(tmp));
end;

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
  else
    res:=cnFirstName;
  end;
  result:=TranslateW(res);
end;

procedure addcolumn(handle:hwnd;width:word;title:PAnsiChar);
var
  lvcol:LV_COLUMNW;
  buf:array [0..127] of WideChar;
begin
  lvcol.mask:=LVCF_TEXT or LVCF_WIDTH;
  lvcol.cx  :=width;
  lvcol.pszText:=TranslateW(FastAnsiToWideBuf(title,buf));
  SendMessageW(handle,LVM_INSERTCOLUMNW,0,lparam(@lvcol));
end;

function getselecteditem(list:hwnd):integer;
begin
  result:=SendMessage(list,LVM_GETNEXTITEM,-1,LVNI_FOCUSED);
end;

function savecuritem(Dialog:hwnd):integer;
var
  listhwnd:hwnd;
  selitem:integer;

  procedure GetText(id,subpos:integer;var dst:PAnsiChar); overload;
  var
    tpc:array [0..1023] of AnsiChar;
  begin
    GetDlgItemTextA(Dialog,id,tpc,SizeOf(tpc));
    mFreeMem(dst);
    StrDup(dst,tpc);
    LV_SetItem(listhwnd,dst,selitem,subpos);
  end;

  // now for Title only
  procedure GetText(id,subpos:integer;var dst:PWideChar); overload;
  var
    tpc:array [0..1023] of WideChar;
  begin
    GetDlgItemTextW(Dialog,id,@tpc,SizeOf(tpc) div SizeOf(WideChar));
    mFreeMem(dst);
    StrDupW(dst,@tpc);
    LV_SetItemW(listhwnd,dst,selitem,subpos);
  end;

  procedure setparam(var param:tserviceparam;cb,id:integer);
  var
    z:bool;
    tpc:array [0..1023] of WideChar;
  begin
    param._type:=SendDlgItemMessage(Dialog,cb,CB_GETCURSEL,0,0);
    case param._type of
      ptNumber:  param.n:=GetDlgItemInt(Dialog,id,z,false);
      ptInteger: param.i:=GetDlgItemInt(Dialog,id,z,true);
      ptString:  begin
        GetDlgItemTextA(Dialog,id,@tpc,SizeOf(tpc));
        StrDup(param.a,@tpc);
      end;
      ptUnicode: begin
        GetDlgItemTextW(Dialog,id,@tpc,SizeOf(tpc) div SizeOf(WideChar));
        StrDupW(param.w,@tpc);
      end;
    end;
  end;

var
  tpc :array [0..127] of WideChar;
  tmpwnd:HWND;
  oldtype,i:integer;
  column:integer;
begin
  listhwnd:=GetDlgItem(Dialog,IDC_LIST);
  selitem:=getselecteditem(listhwnd);
//!!  column:=LV_GetLParam(listhwnd,selitem);
  column:=selitem;
  result:=selitem;
  if (selitem>=0) and (selitem<qsopt.numcolumns) then
  begin
    with qsopt.columns[column] do
    begin
      flags:=0;

      if ListView_GetCheckState(listhwnd,selitem)=BST_CHECKED then
        flags:=flags or COL_ON;

      oldtype:=setting_type;

      tmpwnd:=GetDlgItem(Dialog,IDC_C_VARTYPE);

      setting_type:=SendMessage(tmpwnd,CB_GETITEMDATA,
             SendMessage(tmpwnd,CB_GETCURSEL,0,0),0);

      GetText(IDC_E_TITLE,1,title);

      case oldtype of
        ST_SCRIPT: begin
          mFreeMem(wparam.a);
        end;
        ST_SERVICE: begin
          if (wparam._type=ptString) or (wparam._type=ptUnicode) then mFreeMem(wparam.a);
          if (lparam._type=ptString) or (lparam._type=ptUnicode) then mFreeMem(lparam.a);
        end;
      else
        mFreeMem(wparam.a);
      end;

      case setting_type of
        ST_METACONTACT: begin
          LV_SetItemW(listhwnd,TranslateW('Metacontact'),selitem,2);
        end;

        ST_SCRIPT: begin
          StrDupW(wparam.w,curscript);
          LV_SetItemW(listhwnd,TranslateW('Script'),selitem,2);
        end;

        ST_CONTACTINFO: begin
          FillChar(tpc,SizeOf(tpc),0);
          tmpwnd:=GetDlgItem(Dialog,IDC_C_CNFTYPE);

          i:=SendMessage(tmpwnd,CB_GETCURSEL,0,0);
          setting_cnftype:=SendMessage(tmpwnd,CB_GETITEMDATA,i,0);

          SendMessageW(tmpwnd,CB_GETLBTEXT,i,tlparam(@tpc));
          LV_SetItemW(listhwnd,tpc,selitem,2);
        end;

        ST_SERVICE: begin
          GetText(IDC_E_MODULE,3,module_name);
          LV_SetItemW(listhwnd,TranslateW('Service'),selitem,2);
//!!          setitem(listhwnd,selitem,3,module_name);

          setting_cnftype:=SendDlgItemMessage(Dialog,IDC_C_RESULT,CB_GETCURSEL,0,0);
          setparam(wparam,IDC_C_WPAR,IDC_E_VAR);
          setparam(lparam,IDC_C_LPAR,IDC_E_LPAR);
        end;
      else
        GetText(IDC_E_MODULE,2,module_name);
        GetText(IDC_E_VAR   ,3,wparam.a)
      end;
    end;
  end;
end;

procedure disable_elem(Dialog:hwnd;id:cardinal);
begin
  EnableWindow(GetDlgItem(Dialog,id),FALSE);
end;

procedure enable_elem(Dialog:hwnd;id:cardinal);
begin
  EnableWindow(GetDlgItem(Dialog,id),TRUE);
end;

procedure CheckDirection(Dialog:HWND;item:integer);
begin
  if item=0 then
    disable_elem(Dialog,IDC_UP)
  else
    enable_elem(Dialog,IDC_UP);

  if item=(qsopt.numcolumns-1) then
    disable_elem(Dialog,IDC_DN)
  else
    enable_elem(Dialog,IDC_DN);
end;

procedure displcurinfo(Dialog:hwnd;column:integer);

  procedure set_elem(const param:tserviceparam;cb,id:integer);
  begin
    SendDlgItemMessage(Dialog,cb,CB_SETCURSEL,param._type,0);
    case param._type of
      ptNumber:  SetDlgItemInt  (Dialog,id,param.n,false);
      ptInteger: SetDlgItemInt  (Dialog,id,param.i,true);
      ptString:  SetDlgItemTextA(Dialog,id,param.a);
      ptUnicode: SetDlgItemTextW(Dialog,id,param.w);
    else
      SetDlgItemTextA(Dialog,id,'');
    end;
    EnableWindow(GetDlgItem(Dialog,id),param._type<>ptCurrent);
  end;

var
  v:PWideChar;
  i:int_ptr;
  selpos:integer;
begin
  CheckDirection(Dialog,column);

  selpos:=column;
  if (selpos>=0) and (selpos<qsopt.numcolumns) then
  begin
    enable_elem(Dialog,IDC_E_TITLE);
    enable_elem(Dialog,IDC_C_VARTYPE);
    enable_elem(Dialog,IDC_DELETE);
//    enable_elem(Dialog,IDC_SETITEM);

    with qsopt.columns[column] do
    begin
      SetupControls(Dialog,setting_type);
      case setting_type of
        ST_SCRIPT: begin
          mFreeMem(curscript);
          StrDupW(pWideChar(curscript),wparam.w);
        end;
        ST_SERVICE: begin
          SendDlgItemMessage(Dialog,IDC_C_RESULT,CB_SETCURSEL,setting_cnftype,0);
          SetDlgItemTextA(Dialog,IDC_E_MODULE,module_name);
          set_elem(wparam,IDC_C_WPAR,IDC_E_VAR);
          set_elem(lparam,IDC_C_LPAR,IDC_E_LPAR);
        end;
        ST_CONTACTINFO: begin
          i:=int_ptr(setcnftype2str(setting_cnftype));
          SendDlgItemMessageW(Dialog,IDC_C_CNFTYPE,CB_SELECTSTRING,twparam(-1),i);
        end;
      else
        SetDlgItemTextA(Dialog,IDC_E_MODULE,module_name);
        SetDlgItemTextA(Dialog,IDC_E_VAR,wparam.a);
      end;

      v:=settype2str(setting_type);
      SetDlgItemTextW(Dialog,IDC_E_TITLE,title);
      SendDlgItemMessageW(Dialog,IDC_C_VARTYPE,CB_SELECTSTRING,twparam(-1),tlparam(v));
      mFreeMem(v);
    end;
  end
  else
  begin
    disable_elem(Dialog,IDC_E_TITLE);
    disable_elem(Dialog,IDC_C_VARTYPE);
    disable_elem(Dialog,IDC_DELETE);
    disable_elem(Dialog,IDC_SETITEM);

    v:=TranslateW(strNotSelected);
    SetDlgItemTextW(Dialog,IDC_E_TITLE ,v);
    SetDlgItemTextW(Dialog,IDC_E_MODULE,v);
    SetDlgItemTextW(Dialog,IDC_E_VAR   ,v);
    SetDlgItemTextW(Dialog,IDC_E_LPAR  ,v);
    SendDlgItemMessageW(Dialog,IDC_C_VARTYPE,CB_SELECTSTRING,twparam(-1),tlparam(v));
    SendDlgItemMessageW(Dialog,IDC_C_CNFTYPE,CB_SELECTSTRING,twparam(-1),tlparam(v));
  end;
end;

function add_column(list:HWND;i:integer):integer;
var
  li:LV_ITEMA;
begin
  result:=i;
  zeromemory(@li,sizeof(li));
  li.mask  :=LVIF_PARAM;
  li.lParam:=i;
  li.iItem :=i;
  SendMessage(list,LVM_INSERTITEM,0,lparam(@li));

  with qsopt.columns[i] do
  begin
    ListView_SetCheckState(list,i,(flags and COL_ON)<>0);
    LV_SetItemW(list,title,i,1);
    case setting_type of
      ST_METACONTACT: begin
        LV_SetItemW(list,TranslateW('Metacontact'),i,2);
      end;

      ST_CONTACTINFO: begin
        LV_SetItemW(list,setcnftype2str(setting_cnftype),i,2)
      end;

      ST_SCRIPT: begin
        LV_SetItemW(list,TranslateW('Script'),i,2);
      end;

      ST_SERVICE: begin
        LV_SetItemW(list,TranslateW('Service'),i,2);
        LV_SetItem(list,module_name,i,3);
      end;
    else
      LV_SetItem(list,module_name,i,2);
      LV_SetItem(list,wparam.a,i,3);
    end;
  end;
end;

procedure update_list(list:hwnd);
var
  i:integer;
begin
  ListView_DeleteAllItems(list);
  ListView_SetItemCount(list,qsopt.numcolumns);

  for i:=0 to qsopt.numcolumns-1 do
  begin
    add_column(list,i);
  end;
//!!  SortColumns(list);
  ListView_SetItemState(list,0,LVIS_FOCUSED or LVIS_SELECTED,
    LVIS_FOCUSED or LVIS_SELECTED);
end;

procedure addresulttypes(list:integer);
begin
  SendMessageW(list,CB_INSERTSTRING,ptNumber ,lparam(TranslateW('number value'  )));
  SendMessageW(list,CB_INSERTSTRING,ptInteger,lparam(TranslateW('integer value' )));
  SendMessageW(list,CB_INSERTSTRING,ptString ,lparam(TranslateW('ANSI string'   )));
  SendMessageW(list,CB_INSERTSTRING,ptUnicode,lparam(TranslateW('Unicode string')));
  SendMessage(list,CB_SETCURSEL,0,0);
end;

procedure addparamtypes(list:integer);
begin
  addresulttypes(list);
  SendMessageW(list,CB_INSERTSTRING,ptCurrent,lparam(TranslateW('current contact')));
end;

procedure AddVal(list:HWND;param:integer);
var
  i:integer;
  v:pointer;
begin
  v:=settype2str(param);
  i:=SendMessageW(list,CB_ADDSTRING,0,lparam(v));
  mFreeMem(v);
  SendMessage(list,CB_SETITEMDATA,i,param);
end;

procedure addsettypes(list:hwnd);
begin
  AddVal(list,ST_BYTE);
  AddVal(list,ST_WORD);
  AddVal(list,ST_INT);
  AddVal(list,ST_STRING);
  AddVal(list,ST_LASTSEEN);
  AddVal(list,ST_IP);
  AddVal(list,ST_CONTACTINFO);
  AddVal(list,ST_LASTEVENT);
  AddVal(list,ST_TIMESTAMP);
  AddVal(list,ST_SERVICE);
  AddVal(list,ST_SCRIPT);
  AddVal(list,ST_METACONTACT);
  SendMessage(list,CB_SETCURSEL,0,0);
end;

procedure AddCnf(list:HWND;param:integer);
begin
  SendMessage(list,CB_SETITEMDATA,
    SendMessageW(list,CB_ADDSTRING,0,lparam(setcnftype2str(param))),
    param);
//    mFreeMem(str);
end;

procedure addsetcnftypes(list:hwnd);
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

  SendMessage(list,CB_SETCURSEL,0,0);
end;

procedure _GetIcon(idc:integer;ico:PAnsiChar);
begin
  SetButtonIcon(GetDlgItem(maindlg,idc),ico);
end;

function IconChanged(wParam:WPARAM;lParam:LPARAM):int;cdecl;
begin
  result:=0;
  _GetIcon(IDC_NEW    ,QS_NEW);
  _GetIcon(IDC_SETITEM,QS_ITEM);
  _GetIcon(IDC_UP     ,QS_UP);
  _GetIcon(IDC_DN     ,QS_DOWN);
  _GetIcon(IDC_DELETE ,QS_DELETE);
  _GetIcon(IDC_DEFAULT,QS_DEFAULT);
  _GetIcon(IDC_RELOAD ,QS_RELOAD);
end;

function ScriptEdit(Dialog:HWnd;hMessage:UINT;wParam:WPARAM;lParam:LPARAM):lresult; stdcall;
var
  tmp:pointer;
  vhi:TVARHELPINFO;
begin
  result:=0;
  case hMessage of

    WM_INITDIALOG: begin
      TranslateDialogDefault(Dialog);
      SetDlgItemTextW(Dialog,IDC_EDIT_SCRIPT,pWideChar(lParam));
    end;

    WM_COMMAND: begin
      case wParam shr 16 of
        BN_CLICKED: begin
          case loword(wParam) of
            IDHELP: SendMessage(Dialog,WM_HELP,0,0);
            IDOK: begin
              tmp:=GetDlgText(Dialog,IDC_EDIT_SCRIPT);
              EndDialog(Dialog,tlparam(tmp));
            end;
            IDCANCEL: begin // clear result / restore old value
              EndDialog(Dialog,0);
            end;
          end;
        end;
      end;
    end;

    WM_HELP: begin
      FillChar(vhi,SizeOf(vhi),0);
      with vhi do
      begin
        cbSize:=SizeOf(vhi);
        flags:=VHF_NOINPUTDLG;
      end;
      CallService(MS_VARS_SHOWHELPEX,Dialog,tlparam(@vhi));
    end;
  end;
end;

function NewListProc(Dialog:HWnd;hMessage:UINT;wParam:WPARAM;lParam:LPARAM):integer; stdcall;
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

function DlgProcOptions(Dialog:HWnd;hMessage:UINT;wParam:WPARAM;lParam:LPARAM):lresult; stdcall;
const
  InitDlg:bool = true;
  hook:THANDLE = 0;
var
//  tpc:array [0..255] of AnsiChar;
  itemsel:integer;
  tmpwnd:HWND;
  tmpbool:bool;
  i:integer;
//  ti:TTOOLINFOW;
//  hwndTooltip:HWND;
  hNew    :hwnd;
  hItem   :hwnd;
  hUp     :hwnd;
  hDown   :hwnd;
  hDelete :hwnd;
  hDefault:hwnd;
  hReload :hwnd;
  listhwnd:hwnd;
  tmpcol:tcolumnitem;
begin
  result:=0;

  case hMessage of
    WM_DESTROY: begin
      // if  closed by Cancel, with changes but without apply
      loadopt_db(false); //!!!!
      WndChangeColumns(wcRefresh);

      mFreeMem(curscript);
      if hook<>0 then
        UnhookEvent(hook);
      maindlg:=0;
    end;

    WM_INITDIALOG: begin
      InitDlg:=true;
      listhwnd:=GetDlgItem(Dialog,IDC_LIST);

      SendMessageW(listhwnd,LVM_SETEXTENDEDLISTVIEWSTYLE,0,
        SendMessageW(listhwnd,LVM_GETEXTENDEDLISTVIEWSTYLE,0,0) or
          LVS_EX_FULLROWSELECT or LVS_EX_CHECKBOXES);

      TranslateDialogDefault(Dialog);

      addcolumn(listhwnd,95 ,'Setting');
      addcolumn(listhwnd,105,'Module/InfoType');
      addcolumn(listhwnd,85 ,'Title');
      addcolumn(listhwnd,20 ,'#');
      addsettypes   (GetDlgItem(Dialog,IDC_C_VARTYPE));
      addsetcnftypes(GetDlgItem(Dialog,IDC_C_CNFTYPE));
      addparamtypes (GetDlgItem(Dialog,IDC_C_WPAR));
      addparamtypes (GetDlgItem(Dialog,IDC_C_LPAR));
      addresulttypes(GetDlgItem(Dialog,IDC_C_RESULT));

      CheckDlgButton(Dialog,IDC_CH_SORTSTATUS     ,ORD(qsopt.sortbystatus));
      CheckDlgButton(Dialog,IDC_CH_SHOWINMENU     ,ORD(qsopt.showinmenu));
      CheckDlgButton(Dialog,IDC_CH_SHOWONLYUSERS  ,ORD(qsopt.showonlyinlist));
      CheckDlgButton(Dialog,IDC_CH_AUTOCLOSE      ,ORD(qsopt.closeafteraction));
      CheckDlgButton(Dialog,IDC_CH_ADDTOTOPTOOLBAR,ORD(qsopt.showintoptoolbar));
      CheckDlgButton(Dialog,IDC_CH_USETOOLSTYLE   ,ORD(qsopt.usetoolstyle));
      CheckDlgButton(Dialog,IDC_CH_DRAWGRID       ,ORD(qsopt.drawgrid));
      CheckDlgButton(Dialog,IDC_CH_SINGLECSV      ,ORD(qsopt.singlecsv));
      CheckDlgButton(Dialog,IDC_CH_EXPORTHEADERS  ,ORD(qsopt.exportheaders));
      CheckDlgButton(Dialog,IDC_CH_SKIPMINIMIZED  ,ORD(qsopt.skipminimized));
      CheckDlgButton(Dialog,IDC_CH_SAVEPATTERN    ,ORD(qsopt.savepattern));
      if ServiceExists(MS_FP_GETCLIENTICON)<>0 then
        CheckDlgButton(Dialog,IDC_CH_SHOWCLIENTICONS,ORD(qsopt.showclienticons))
      else
        EnableWindow(GetDlgItem(Dialog,IDC_CH_SHOWCLIENTICONS),false);

      if ServiceExists(MS_TTB_ADDBUTTON)=0 then
        EnableWindow(GetDlgItem(Dialog,IDC_CH_ADDTOTOPTOOLBAR),FALSE);
{
      hwndTooltip:=CreateWindowW(TOOLTIPS_CLASS,nil,TTS_ALWAYSTIP,
          integer(CW_USEDEFAULT),integer(CW_USEDEFAULT),
          integer(CW_USEDEFAULT),integer(CW_USEDEFAULT),
          Dialog,0,hInstance,nil);
}
      hNew    :=GetDlgItem(Dialog,IDC_NEW);
      SendMessage(hNew,BUTTONADDTOOLTIP,TWPARAM(TranslateW('New')),BATF_UNICODE);
      hItem   :=GetDlgItem(Dialog,IDC_SETITEM);
      SendMessage(hItem,BUTTONADDTOOLTIP,TWPARAM(TranslateW('Save Item')),BATF_UNICODE);
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
      SetButtonIcon(hItem,   QS_ITEM);
      SetButtonIcon(hUp     ,QS_UP);
      SetButtonIcon(hDown   ,QS_DOWN);
      SetButtonIcon(hDelete ,QS_DELETE);
      SetButtonIcon(hDefault,QS_DEFAULT);
      SetButtonIcon(hReload ,QS_RELOAD);

      update_list(listhwnd);

      maindlg:=Dialog;
      hook:=HookEvent(ME_SKIN2_ICONSCHANGED,@IconChanged);

      result:=1;

      OldListProc:=pointer(SetWindowLongPtrW(listhwnd,GWL_WNDPROC,LONG_PTR(@NewListProc)));

      InitDlg:=false;
    end;

    WM_NOTIFY: begin
      case integer(PNMHdr(lParam)^.code) of

        PSN_APPLY: begin
          // checkboxes
          listhwnd:=GetDlgItem(Dialog,IDC_LIST);

          for i:=0 to SendMessage(listhwnd,LVM_GETITEMCOUNT,0,0)-1 do
          begin
            with qsopt.columns[i] do
            begin
              if ListView_GetCheckSTate(listhwnd,i)=0 then
                flags:=flags and not COL_ON
              else
                flags:=flags or COL_ON;
            end;
          end;

          disable_elem(Dialog,IDC_SETITEM);
          savecuritem(Dialog);

          qsopt.sortbystatus    :=IsDlgButtonChecked(Dialog,IDC_CH_SORTSTATUS     )<>BST_UNCHECKED;
          qsopt.showonlyinlist  :=IsDlgButtonChecked(Dialog,IDC_CH_SHOWONLYUSERS  )<>BST_UNCHECKED;
          qsopt.closeafteraction:=IsDlgButtonChecked(Dialog,IDC_CH_AUTOCLOSE      )<>BST_UNCHECKED;
          qsopt.usetoolstyle    :=IsDlgButtonChecked(Dialog,IDC_CH_USETOOLSTYLE   )<>BST_UNCHECKED;
          qsopt.drawgrid        :=IsDlgButtonChecked(Dialog,IDC_CH_DRAWGRID       )<>BST_UNCHECKED;
          qsopt.showclienticons :=IsDlgButtonChecked(Dialog,IDC_CH_SHOWCLIENTICONS)<>BST_UNCHECKED;
          qsopt.singlecsv       :=IsDlgButtonChecked(Dialog,IDC_CH_SINGLECSV      )<>BST_UNCHECKED;
          qsopt.exportheaders   :=IsDlgButtonChecked(Dialog,IDC_CH_EXPORTHEADERS  )<>BST_UNCHECKED;
          qsopt.skipminimized   :=IsDlgButtonChecked(Dialog,IDC_CH_SKIPMINIMIZED  )<>BST_UNCHECKED;
          qsopt.savepattern     :=IsDlgButtonChecked(Dialog,IDC_CH_SAVEPATTERN    )<>BST_UNCHECKED;

          tmpbool:=IsDlgButtonChecked(Dialog,IDC_CH_SHOWINMENU)<>BST_UNCHECKED;
          if qsopt.showinmenu<>tmpbool then
          begin
            qsopt.showinmenu:=tmpbool;
            AddRemoveMenuItemToMainMenu;
          end;
          tmpbool:=IsDlgButtonChecked(Dialog,IDC_CH_ADDTOTOPTOOLBAR )<>BST_UNCHECKED;
          if qsopt.showintoptoolbar<>tmpbool then
          begin
            qsopt.showintoptoolbar:=tmpbool;
            addtotoolbar;
          end;

          saveopt_db;
          result:=1;
        end;

        LVN_ITEMCHANGED: begin
          if wParam=IDC_LIST then
          begin
            i:=(PNMLISTVIEW(lParam)^.uOldState and LVNI_FOCUSED)-
               (PNMLISTVIEW(lParam)^.uNewState and LVNI_FOCUSED);
            if i<0 then // new focus
            begin
              InitDlg:=true;
              displcurinfo(Dialog,PNMLISTVIEW(lParam)^.iItem);
{!!
              displcurinfo(Dialog,
                 LV_GetLParam(PNMLISTVIEW(lParam)^.hdr.hwndFrom,
                 PNMLISTVIEW(lParam)^.iItem));
}
              InitDlg:=false;
              result:=1;
            end else if (i=0) and not InitDlg then
            begin
              if (PNMLISTVIEW(lParam)^.uOldState or PNMLISTVIEW(lParam)^.uNewState)=$3000 then
              begin
                i:=PNMLISTVIEW(lParam)^.uOldState-PNMLISTVIEW(lParam)^.uNewState;
                if abs(i)=$1000 then
                begin
                  SendMessage(GetParent(Dialog),PSM_CHANGED,0,0);
                  if i<0 then
                    i:=wcShow
                  else
                    i:=wcHide;
                  WndChangeColumns(i,PNMLISTVIEW(lParam)^.iItem);
                  result:=1;
                end;
              end;
            end;
          end;
        end;

      end;
    end;

    WM_MYSHOWHIDEITEM: begin
      InitDlg:=true;
      ListView_SetCheckState(GetDlgItem(Dialog,IDC_LIST),wParam,lParam<>0);
      InitDlg:=false;
    end;

    WM_MYMOVEITEM: begin
      listhwnd:=GetDlgItem(Dialog,IDC_LIST);
      LV_MoveItem(listhwnd,lParam,wParam);
      itemsel:=wParam+lParam;
      i:=SizeOf(tcolumnitem)*abs(integer(lParam));
      move(qsopt.columns[wParam],tmpcol,SizeOf(tcolumnitem));

      if integer(lParam)>0 then
        move(qsopt.columns[wParam+1],qsopt.columns[wParam],i)
      else
        move(qsopt.columns[itemsel],qsopt.columns[itemsel+1],i);

      move(tmpcol,qsopt.columns[itemsel],SizeOf(tcolumnitem));

      CheckDirection(Dialog,getselecteditem(listhwnd));
    end;

    WM_COMMAND: begin
      if ((wParam shr 16)=CBN_SELCHANGE) then
      begin
        case loword(wParam) of
          IDC_C_VARTYPE: begin
            i:=SendMessage(lParam,CB_GETITEMDATA,
                 SendMessage(lParam,CB_GETCURSEL,0,0),0);

            SetupControls(Dialog,i);

            EnableWindow(GetDlgItem(Dialog,IDC_E_VAR),
              SendDlgItemMessage(Dialog,IDC_C_WPAR,CB_GETCURSEL,0,0)<>ptCurrent);
            EnableWindow(GetDlgItem(Dialog,IDC_E_LPAR),
              SendDlgItemMessage(Dialog,IDC_C_LPAR,CB_GETCURSEL,0,0)<>ptCurrent);
          end;
          IDC_C_WPAR: begin
            EnableWindow(GetDlgItem(Dialog,IDC_E_VAR),
              SendMessage(lParam,CB_GETCURSEL,0,0)<>ptCurrent);
          end;
          IDC_C_LPAR: begin
            EnableWindow(GetDlgItem(Dialog,IDC_E_LPAR),
              SendMessage(lParam,CB_GETCURSEL,0,0)<>ptCurrent);
          end;
        end;
      end;

      if not InitDlg then
        case wParam shr 16 of
          CBN_SELCHANGE,
          BN_CLICKED,
          EN_CHANGE: begin
            SendMessage(GetParent(Dialog),PSM_CHANGED,0,0);
            enable_elem(Dialog,IDC_SETITEM);
          end;
        end;

      listhwnd:=GetDlgItem(Dialog,IDC_LIST);
      result:=1;
      case loword(wParam) of
        IDC_SCRIPT: begin
          tmpwnd:=DialogBoxParamW(hInstance,MAKEINTRESOURCEW(IDD_SCRIPT),
             Dialog,@ScriptEdit,tlparam(curscript));
          if tmpwnd<>0 then
          begin
            curscript:=pointer(tmpwnd);
          end;
        end;

        IDC_NEW: begin
          InitDlg:=true;
          WndChangeColumns(wcInsert,
            add_column(listhwnd,new_column(getselecteditem(listhwnd))));

          SendMessage(listhwnd,LVM_ENSUREVISIBLE,qsopt.numcolumns-1,0);
          ListView_SetItemState(listhwnd,qsopt.numcolumns-1,
              LVIS_FOCUSED+LVIS_SELECTED,LVIS_FOCUSED+LVIS_SELECTED);
          InitDlg:=false;
        end;

        IDC_DELETE: begin
          i:=getselecteditem(listhwnd);
          SendMessage(listhwnd,LVM_DELETEITEM,i,0);
          delete_column(i);
          WndChangeColumns(wcDelete,i);
//          update_list(listhwnd);

          if i=qsopt.numcolumns then dec(i);
          ListView_SetItemState(listhwnd,i,
              LVIS_FOCUSED+LVIS_SELECTED,LVIS_FOCUSED+LVIS_SELECTED);
        end;

        IDC_UP: begin
          itemsel:=getselecteditem(listhwnd);
          if itemsel>0 then
          begin
            LV_MoveItem(listhwnd,-1,itemsel);
            move(qsopt.columns[itemsel]  ,tmpcol                  ,SizeOf(tcolumnitem));
            move(qsopt.columns[itemsel-1],qsopt.columns[itemsel]  ,SizeOf(tcolumnitem));
            move(tmpcol                  ,qsopt.columns[itemsel-1],SizeOf(tcolumnitem));
            WndChangeColumns(wcUp,itemsel);
            CheckDirection(Dialog,itemsel-1);
          end;
        end;

        IDC_DN: begin
          itemsel:=getselecteditem(listhwnd);
          if (itemsel>=0) and (itemsel<(qsopt.numcolumns-1)) then
          begin
            LV_MoveItem(listhwnd,1,itemsel);
            move(qsopt.columns[itemsel]  ,tmpcol                  ,SizeOf(tcolumnitem));
            move(qsopt.columns[itemsel+1],qsopt.columns[itemsel]  ,SizeOf(tcolumnitem));
            move(tmpcol                  ,qsopt.columns[itemsel+1],SizeOf(tcolumnitem));
            WndChangeColumns(wcDown,itemsel);
            CheckDirection(Dialog,itemsel+1);
          end;
        end;

        IDC_SETITEM: begin
          WndChangeColumns(wcChange,savecuritem(Dialog));
        end;

        IDC_DEFAULT: begin
          InitDlg:=true;
          loaddefaultcolumns;
          update_list(listhwnd);
          WndChangeColumns(wcRefresh);
          InitDlg:=false;
        end;

        IDC_RELOAD: begin
          InitDlg:=true;
          loadopt_db(false);
          update_list(listhwnd);
          WndChangeColumns(wcRefresh);
          InitDlg:=false;
        end;
      else
        result:=0;
      end;
    end;
//  else
//    result:=DefWindowProc(Dialog,hMessage,wParam,lParam);
  end;
    DefWindowProc(Dialog,hMessage,wParam,lParam);
end;

procedure OptChangeColumns(code:integer;column,data:integer);
begin
  case code of
    wcUp: begin
      SendMessage(maindlg,WM_MYSHOWHIDEITEM,column,data);
    end;

    wcShow: begin
      SendMessage(maindlg,WM_MYSHOWHIDEITEM,column,data);
    end;
  end;
end;

end.
