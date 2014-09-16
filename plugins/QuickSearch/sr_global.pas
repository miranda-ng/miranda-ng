unit sr_global;

interface
uses windows,messages,commctrl,m_api,dbsettings,mirutils;

const
  QS_QS     :PAnsiChar = 'QS_QS';
  QS_NEW    :PAnsiChar = 'QS_New';
  QS_ITEM   :PAnsiChar = 'QS_Item';
  QS_UP     :PAnsiChar = 'QS_Up';
  QS_DOWN   :PAnsiChar = 'QS_Down';
  QS_DELETE :PAnsiChar = 'QS_Delete';
  QS_DEFAULT:PAnsiChar = 'QS_Default';
  QS_RELOAD :PAnsiChar = 'QS_Reload';
  QS_MALE   :PAnsiChar = 'QS_Male';
  QS_FEMALE :PAnsiChar = 'QS_Female';
const
  qs_module     :PAnsiChar = 'QuickSearch';
  qs_name       :PAnsiChar = 'Quick Search';
  QS_SHOWSERVICE:PAnsiChar = 'QuickSearch_PLUGIN/Show';

const
  StatusSort = 1000;

const //types
  QSTS_BYTE      = 0;
  QSTS_WORD      = 1;
  QSTS_DWORD     = 2;
  QSTS_STRING    = 3;
  QSTS_IP        = 4;
  QSTS_TIMESTAMP = 5;
  QSTS_SIGNED    = 6;
  QSTS_HEXNUM    = 7;

  // must be non-zero for empty-column checking
  QST_SETTING     = 100;
  QST_SCRIPT      = 1;
  QST_SERVICE     = 2;
  QST_CONTACTINFO = 3;
  QST_OTHER       = 200;

  QSTO_LASTSEEN    = 0;
  QSTO_LASTEVENT   = 1;
  QSTO_METACONTACT = 2;
  QSTO_EVENTCOUNT  = 3;

const
  COL_ON      = $0001; // Show column
  COL_FILTER  = $0004; // Filter column by pattern
  // QS window runtime flags
  COL_INIT    = $0002; // No need to update
  COL_XSTATUS = $0100;
  COL_GENDER  = $0200;
  COL_CLIENT  = $0400;
  COL_GROUP   = $0800;
  COL_CNTNR   = $1000;
  COL_REFRESH = $FF02; // mask

const
  QSO_SORTBYSTATUS = $00000001; // Sort by status
  QSO_DRAWGRID     = $00000002; // Draw listview grid
  QSO_TOOLSTYLE    = $00000004; // QS window tool style
  QSO_SAVEPATTERN  = $00000008; // Save filter pattern
  QSO_AUTOCLOSE    = $00000010; // Close QS window after action
  QSO_CLIENTICONS  = $00000020; // Show client icons (fingerprint)

  QSO_MAINOPTIONS  = $0000FFFF; // mask for common options

  // QS window options
  QSO_STAYONTOP    = $00010000; // Stay QS window on top
  QSO_SHOWOFFLINE  = $00020000; // Show offline contacts
  QSO_COLORIZE     = $00040000; // Colorize lines
  QSO_SORTASC      = $00080000; // Sort column ascending

{$include resource.inc}

type
  tserviceparam = record
    value:uint_ptr;
    _type:dword;
  end;

type
  pcolumnitem = ^tcolumnitem;
  tcolumnitem = record
    title          :PWideChar;
    setting_type   :word;      // QST_* constants
    flags          :word;      // COL_* constants
    width          :word;
    case integer of
      // db setting
      0: (
        datatype:word;         // QSTS_* constants
        module  :pAnsiChar;
        setting :pAnsiChar;
      );
      // script
      1: (script:pWideChar);
      // service
      2: (
        service:pAnsiChar;
        wparam :tserviceparam;
        lparam :tserviceparam;
        restype:dword;
      );
      // contact info
      3: (cnftype:word);      // CNF_* constants
      // other
      4: (other:word);        // QSTO_* constants
  end;
  tcolumnarray = array of tcolumnitem;

const
  MaxColumnAmount = 64;
type
  tqsopt = record
    grrect    :TRECT;        // QS window rect
    columns   :array [0..MaxColumnAmount-1] of tcolumnitem;
    numcolumns:integer;      // columns array size (really, needs just for db reading)
    columnsort:integer;      // sorted column number
    flags     :dword;        // QSO_* options
  end;


procedure reghotkeys;
procedure unreghotkeys;

procedure saveopt_wnd;
procedure loadopt_wnd;
function savecolumn(num:integer; const column:tcolumnitem):boolean;
procedure saveopt_db;
function loadopt_db(var columns:array of tcolumnitem):integer;
function loaddefaultcolumns(var columns:array of tcolumnitem):integer;

function CloneColumns(var dst: array of tcolumnitem; const src:array of tcolumnitem):integer;
function CloneColumn (var dst:tcolumnitem; const src:tcolumnitem):boolean;

procedure clear_columns(var columns:array of tcolumnitem);
procedure clear_column (var column:tcolumnitem);

function  new_column(var columns:array of tcolumnitem):integer;

procedure AddRemoveMenuItemToMainMenu;
procedure addtotoolbar;
procedure removetoolbar;

var
  qsopt:tqsopt;

const
  MainMenuItem:integer=0;
  hTTBButton  :THANDLE=0;

implementation

uses common, sparam;

const
  HKN_GLOBAL:PAnsiChar = 'QS_Global';
const
  so_mbottom         :PAnsiChar = 'mbottom';
  so_mright          :PAnsiChar = 'mright';
  so_mtop            :PAnsiChar = 'mtop';
  so_mleft           :PAnsiChar = 'mleft';

  so_columnsort      :PAnsiChar = 'columnsort';
  so_flags           :PAnsiChar = 'flags';

  so_numcolumns      :PAnsiChar = 'numcolumns';
  so_item            :PAnsiChar = 'item';

  so__title          :PAnsiChar = '_title';
  so__width          :PAnsiChar = '_width';
  so__flags          :PAnsiChar = '_flags';
  so__setting_type   :PAnsiChar = '_setting_type';

  so__cnftype        :PAnsiChar = '_cnftype';

  so__datatype       :PAnsiChar = '_datatype';
  so__module         :PAnsiChar = '_module';
  so__setting        :PAnsiChar = '_setting';

  so__script         :PAnsiChar = '_script';

  so__service        :PAnsiChar = '_service';
  so__restype        :PAnsiChar = '_restype';
  so__wparam_type    :PAnsiChar = '_wparam_type';
  so__lparam_type    :PAnsiChar = '_lparam_type';
  so__wparam         :PAnsiChar = '_wparam';
  so__lparam         :PAnsiChar = '_lparam';

  so__other          :PAnsiChar = '_other';

procedure reghotkeys;
var
  hkrec:THOTKEYDESC;
begin
  FillChar(hkrec,SizeOf(hkrec),0);
  with hkrec do
  begin
    cbSize          :=SizeOf(hkrec);
    pszName         :=HKN_GLOBAL;
    pszDescription.a:='QuickSearch window hotkey';
    pszSection.a    :=qs_name;
    pszService      :=QS_SHOWSERVICE;
    DefHotKey       :=(HOTKEYF_ALT shl 8) or VK_F3;
  end;
  CallService(MS_HOTKEY_REGISTER,0,lparam(@hkrec));
end;

procedure unreghotkeys;
begin
  CallService(MS_HOTKEY_UNREGISTER,0,lparam(HKN_GLOBAL));
end;

procedure removetoolbar;
begin
  if hTTBButton<>0 then
  begin
    if ServiceExists(MS_TTB_REMOVEBUTTON)>0 then
    begin
      CallService(MS_TTB_REMOVEBUTTON,WPARAM(hTTBButton),0);
      hTTBButton:=0;
    end;
  end;
end;

procedure addtotoolbar;
var
  ttbopt:TTBButton;
begin
  removetoolbar;

  if ServiceExists(MS_TTB_ADDBUTTON)>0 then
  begin
    ZeroMemory(@ttbopt,sizeof(ttbopt));
    ttbopt.cbSize    :=sizeof(ttbopt);
    ttbopt.pszService:=QS_SHOWSERVICE;
    ttbopt.hIconUp   :=CallService(MS_SKIN2_GETICON,0,lparam(QS_QS));
    ttbopt.hIconDn   :=ttbopt.hIconUp;
    ttbopt.dwFlags   :=TTBBF_VISIBLE;
    ttbopt.name      :=qs_module;
    hTTBButton:=TopToolbar_AddButton(@ttbopt);
    if hTTBButton=THANDLE(-1) then
      hTTBButton:=0;
  end;
end;

procedure AddRemoveMenuItemToMainMenu;
var
  cmi:TCLISTMENUITEM;
begin
  if MainMenuItem<>0 then exit;

  ZeroMemory(@cmi,sizeof(cmi));
  cmi.cbSize      :=sizeof(cmi) ;
  cmi.szName.a    :=qs_name;
  cmi.position    :=500050000;
//    cmi.pszPopupName:=nil;
//    cmi.flags       :=0;
  cmi.pszService  :=QS_SHOWSERVICE;
  cmi.hIcon       :=CallService(MS_SKIN2_GETICON,0,lparam(QS_QS));
  MainMenuItem    :=Menu_AddMainMenuItem(@cmi);

  begin
{
    if (MainMenuItem<>0) then
    begin
      CallService(MO_REMOVEMENUITEM,MainMenuItem,0);
      MainMenuItem:=0;
    end;
}
  end;
end;

// -------- column functions ---------

function CloneColumn(var dst:tcolumnitem; const src:tcolumnitem):boolean;
begin
  if src.setting_type=0 then
  begin
    result:=false;
    exit;
  end;

  move(src,dst,SizeOf(tcolumnitem));
  StrDupW(dst.title,dst.title);
  case dst.setting_type of
    QST_SETTING: begin
      StrDup(dst.module,dst.module);
      StrDup(dst.setting,dst.setting);
    end;
    QST_SCRIPT: begin
      StrDupW(dst.script,dst.script);
    end;
    QST_SERVICE: begin
      StrDup(dst.service,dst.service);
      if (dst.wparam._type=ACF_STRING) or (dst.wparam._type=ACF_UNICODE) then
        StrDupW(pWideChar(dst.wparam.value),pWideChar(dst.wparam.value));
      if (dst.wparam._type=ACF_STRING) or (dst.wparam._type=ACF_UNICODE) then
        StrDupW(pWideChar(dst.lparam.value),pWideChar(dst.lparam.value));
    end;
  end;
  result:=true;
end;

function CloneColumns(var dst: array of tcolumnitem; const src:array of tcolumnitem):integer;
var
  i,cnt:integer;
begin
  cnt:=0;
  for i:=0 to MaxColumnAmount-1 do
  begin
    if CloneColumn(dst[cnt],src[i]) then
      inc(cnt);
  end;
  result:=cnt;
end;

procedure clear_column(var column:tcolumnitem);
begin
  if column.setting_type=0 then // empty already
    exit;

  mFreeMem(column.title);
  case column.setting_type of
    QST_SETTING: begin
      mFreeMem(column.module);
      mFreeMem(column.setting);
    end;
    QST_SCRIPT: begin
      mFreeMem(column.script);
    end;
    QST_SERVICE: begin
      mFreeMem(column.service);
      if (column.wparam._type=ACF_STRING) or (column.wparam._type=ACF_UNICODE) then
        mFreeMem(pointer(column.wparam.value));
      if (column.wparam._type=ACF_STRING) or (column.wparam._type=ACF_UNICODE) then
        mFreeMem(pointer(column.lparam.value));
    end;
    QST_CONTACTINFO: begin
    end;
    QST_OTHER: begin
    end;
  end;
  column.setting_type:=0; // mark as unused
end;

procedure clear_columns(var columns:array of tcolumnitem);
var
  i:integer;
begin
  for i:=0 to MaxColumnAmount-1 do
    clear_column(columns[i]);
end;

function new_column(var columns:array of tcolumnitem):integer;
var
  i:integer;
begin
  result:=MaxColumnAmount-1;
  for i:=0 to MaxColumnAmount-1 do
  begin
    if columns[i].setting_type=0 then // "empty" condition
    begin
      with columns[i] do
      begin
        StrDupW(title,'New column');
        width:=64;
        flags:=COL_ON;
        setting_type:=QST_SETTING;
      end;

      result:=i;
      break;
    end;
  end;
end;

function loaddefaultcolumns(var columns:array of tcolumnitem):integer;
var
  i:integer;
begin
  clear_columns(columns);
  // lazy to renumber if changes
  i:=0;

  // account
  with columns[i] do
  begin
    StrDupW(title,TranslateW('Account'));
    width          :=82;
    flags          :=COL_ON;
    setting_type   :=QST_SERVICE;
    StrDup (service,MS_PROTO_GETCONTACTBASEACCOUNT);
    restype        :=ACF_RSTRING;
    wparam._type   :=ACF_CURRENT;
    lparam._type   :=ACF_NUMBER;
    lparam.value   :=0;
  end;
  inc(i);

  // gender
  with columns[i] do
  begin
    StrDupW(title,TranslateW('Gender'));
    width          :=20;
    flags          :=COL_ON;
    setting_type   :=QST_CONTACTINFO;
    cnftype        :=CNF_GENDER;
  end;
  inc(i);

  // uin
  with columns[i] do
  begin
    StrDupW(title,TranslateW('UserID'));
    width          :=80;
    flags          :=COL_ON+COL_FILTER;
    setting_type   :=QST_CONTACTINFO;
    cnftype        :=CNF_UNIQUEID;
  end;
  inc(i);

  // username(displayname)
  with columns[i] do
  begin
    StrDupW(title,TranslateW('Nickname'));
    width          :=76;
    flags          :=COL_ON+COL_FILTER;
    setting_type   :=QST_SERVICE;
    StrDup(service,MS_CLIST_GETCONTACTDISPLAYNAME);
    restype        :=ACF_RUNICODE;
    wparam._type   :=ACF_CURRENT;
    lparam._type   :=ACF_NUMBER;
    lparam.value   :=2; // 0 for ANSI
  end;
  inc(i);

  // firstname
  with columns[i] do
  begin
    StrDupW(title,TranslateW('First name'));
    width          :=68;
    flags          :=COL_ON+COL_FILTER;
    setting_type   :=QST_CONTACTINFO;
    cnftype        :=CNF_FIRSTNAME;
  end;
  inc(i);

  // lastname
  with columns[i] do
  begin
    StrDupW(title,TranslateW('Last name'));
    width          :=66;
    flags          :=COL_ON+COL_FILTER;
    setting_type   :=QST_CONTACTINFO;
    cnftype        :=CNF_LASTNAME;
  end;
  inc(i);

  // group
  with columns[i] do
  begin
    StrDupW(title,TranslateW('Group'));
    width          :=80;
    flags          :=COL_ON+COL_FILTER;
    setting_type   :=QST_SETTING;
    datatype       :=QSTS_STRING;
    StrDup(module ,'CList');
    StrDup(setting,'Group');
  end;
  inc(i);

  // TabSRMM container
  with columns[i] do
  begin
    StrDupW(title,TranslateW('Container'));
    width          :=80;
    flags          :=COL_ON+COL_FILTER;
    setting_type   :=QST_SETTING;
    datatype       :=QSTS_STRING;
    StrDup(module ,'Tab_SRMsg');
    StrDup(setting,'containerW');
  end;
  inc(i);

  // email
  with columns[i] do
  begin
    StrDupW(title,TranslateW('E-mail'));
    width          :=116;
    flags          :=COL_ON+COL_FILTER;
    setting_type   :=QST_CONTACTINFO;
    cnftype        :=CNF_EMAIL;
  end;
  inc(i);

 // miranda version
  with columns[i] do
  begin
    StrDupW(title,TranslateW('Client ID'));
    width       :=60;
    flags       :=COL_ON;
    setting_type:=QST_SETTING;
    datatype    :=QSTS_STRING;
    StrDup(setting,'MirVer');
  end;
  inc(i);

 // IP version
  with columns[i] do
  begin
    StrDupW(title,TranslateW('Ext IP'));
    width       :=100;
    flags       :=0;
    setting_type:=QST_SETTING;
    datatype    :=QSTS_IP;
    StrDup(module ,'ICQ');
    StrDup(setting,'IP');
  end;
  inc(i);

 // LastSeen
  with columns[i] do
  begin
    StrDupW(title,TranslateW('LastSeen'));
    width       :=116;
    flags       :=0;
    setting_type:=QST_OTHER;
    other       :=QSTO_LASTSEEN;
  end;
  inc(i);

 // last event
  with columns[i] do
  begin
    StrDupW(title,TranslateW('Last Event'));
    width       :=100;
    flags       :=0;
    setting_type:=QST_OTHER;
    other       :=QSTO_LASTEVENT;
  end;
  inc(i);

 // online since
  with columns[i] do
  begin
    StrDupW(title,TranslateW('Online since'));
    width       :=100;
    flags       :=0;
    setting_type:=QST_SETTING;
    datatype    :=QSTS_TIMESTAMP;
    StrDup(module ,'ICQ');
    StrDup(setting,'LogonTS');
  end;
  inc(i);

 // metacontacts
  with columns[i] do
  begin
    StrDupW(title,TranslateW('Metacontact'));
    width       :=50;
    flags       :=0;
    setting_type:=QST_OTHER;
    other       :=QSTO_METACONTACT;
  end;
  inc(i);

  // events
  with columns[i] do
  begin
    StrDupW(title,TranslateW('Event count'));
    width       :=50;
    flags       :=0;
    setting_type:=QST_OTHER;
    other       :=QSTO_EVENTCOUNT;
  end;
  inc(i);

  // Contact add time
  with columns[i] do
  begin
    StrDupW(title,TranslateW('Contact add time'));
    width          :=80;
    flags          :=COL_ON;
    setting_type   :=QST_SETTING;
    datatype       :=QSTS_TIMESTAMP;
    StrDup(module ,'UserInfo');
    StrDup(setting,'ContactAddTime');
  end;
  inc(i);

  result:=i;
end;

// -------- save/load settings ---------

procedure WriteInt(setting:PAnsiChar;value:int);
begin
  DBWriteDword(0,qs_module,setting,value)
end;
procedure WriteWord(setting:PAnsiChar;value:word);
begin
  DBWriteWord(0,qs_module,setting,value)
end;
procedure WriteStr(setting:PAnsiChar;value:PAnsiChar);
begin
  DBWriteString(0,qs_module,setting,value)
end;
procedure WriteUnicode(setting:PAnsiChar;value:PWideChar);
begin
  DBWriteUnicode(0,qs_module,setting,value)
end;

procedure saveopt_wnd;
begin
  WriteInt(so_mbottom    ,qsopt.grrect.bottom);
  WriteInt(so_mright     ,qsopt.grrect.right);
  WriteInt(so_mtop       ,qsopt.grrect.top);
  WriteInt(so_mleft      ,qsopt.grrect.left);

  WriteInt(so_flags      ,qsopt.flags);
  WriteInt(so_columnsort ,qsopt.columnsort);
end;

function savecolumn(num:integer; const column:tcolumnitem):boolean;
var
  buf:array [0..127] of AnsiChar;
  p,pp:PAnsiChar;
begin
  if column.setting_type=0 then
  begin
    result:=false;
    exit;
  end;
  result:=true;

  pp:=StrCopyE(buf,so_item);
  p:=StrEnd(IntToStr(pp,num));
  with column do
  begin
    StrCopy(p,so__title); WriteUnicode(buf,title);
    StrCopy(p,so__setting_type); WriteWord(buf,setting_type);
    StrCopy(p,so__flags); WriteWord(buf,flags and not COL_REFRESH);
    StrCopy(p,so__width); WriteWord(buf,width);
    case setting_type of
      QST_SETTING: begin
        StrCopy(p,so__datatype); WriteWord(buf,datatype);
        StrCopy(p,so__module  ); WriteStr (buf,module);
        StrCopy(p,so__setting ); WriteStr (buf,setting);
      end;

      QST_SCRIPT: begin
        StrCopy(p,so__script); WriteUnicode(buf,script);
      end;

      QST_CONTACTINFO: begin
        StrCopy(p,so__cnftype); WriteWord(buf,cnftype);
      end;

      QST_SERVICE: begin
        StrCopy(p,so__service    ); WriteStr(buf,service);
        StrCopy(p,so__restype    ); WriteInt(buf,restype);
        StrCopy(p,so__wparam_type); WriteInt(buf,wparam._type);
        StrCopy(p,so__lparam_type); WriteInt(buf,lparam._type);
        StrCopy(p,so__wparam);
        case wparam._type of
          ACF_NUMBER : WriteInt    (buf,wparam.value);
          ACF_STRING : WriteStr    (buf,pointer(wparam.value));
          ACF_UNICODE: WriteUnicode(buf,pointer(wparam.value));
        end;
        StrCopy(p,so__lparam);
        case lparam._type of
          ACF_NUMBER : WriteInt    (buf,lparam.value);
          ACF_STRING : WriteStr    (buf,pointer(lparam.value));
          ACF_UNICODE: WriteUnicode(buf,pointer(lparam.value));
        end;
      end;

      QST_OTHER: begin
        StrCopy(p,so__other); WriteWord(buf,other);
      end;
    end;
  end;
end;

procedure saveopt_db;
var
  i,cnt:integer;
begin
  DBDeleteGroup(0,qs_module,'item*');
  WriteInt(so_flags,qsopt.flags);

  cnt:=0;
  for i:=0 to MaxColumnAmount-1 do
  begin
    if savecolumn(cnt,qsopt.columns[i]) then
      inc(cnt);
  end;
  WriteWord(so_numcolumns,cnt);
end;

function GetInt(setting:PAnsiChar;default:integer):integer;
begin
  result:=DBReadDWord(0,qs_module,setting,default);
end;
function GetWord(setting:PAnsiChar;default:word):word;
begin
  result:=DBReadWord(0,qs_module,setting,default);
end;
function GetStr(setting:PAnsiChar):PAnsiChar;
begin
  result:=DBReadString(0,qs_module,setting,nil);
end;
function GetUnicode(setting:PAnsiChar):PWideChar;
begin
  result:=DBReadUnicode(0,qs_module,setting,nil);
end;

{
  if "fast"writing enabled
  fill columns accordingly comumn order array
}

procedure loadopt_wnd;
begin
  qsopt.grrect.bottom:=GetInt(so_mbottom,240);
  qsopt.grrect.right :=GetInt(so_mright ,550);
  qsopt.grrect.top   :=GetInt(so_mtop   ,0);
  qsopt.grrect.left  :=GetInt(so_mleft  ,0);

  qsopt.columnsort   :=GetInt(so_columnsort,StatusSort);
  qsopt.flags        :=GetInt(so_flags,QSO_SORTBYSTATUS+QSO_DRAWGRID+QSO_CLIENTICONS+
                                       QSO_COLORIZE+QSO_SORTASC);
end;

function loadopt_db(var columns:array of tcolumnitem):integer;
var
  buf:array [0..127] of AnsiChar;
  p,pp:PAnsiChar;
  i:integer;
begin
  if DBGetSettingType(0,qs_module,so_flags)=DBVT_DELETED then
  begin
    DBDeleteModule(qs_module);
    qsopt.flags:=
        QSO_SORTBYSTATUS or QSO_DRAWGRID or
        QSO_CLIENTICONS  or QSO_COLORIZE or
        QSO_SORTASC;
    result:=0;
  end
  else
  begin
    qsopt.flags:=GetInt(so_flags,
        QSO_SORTBYSTATUS or QSO_DRAWGRID or
        QSO_CLIENTICONS  or QSO_COLORIZE or
        QSO_SORTASC);
    result:=GetWord(so_numcolumns,0);
  end;

  if result=0 then
  begin
    result:=loaddefaultcolumns(columns);
    saveopt_db;
  end
  else
  begin
    clear_columns(columns);

    pp:=StrCopyE(buf,so_item);
//??    FillChar(qsopt.columns[0],SizeOf(qsopt.columns),0);
    for i:=0 to result-1 do
    begin
      p:=StrEnd(IntToStr(pp,i));
      with columns[i] do
      begin
        StrCopy(p,so__setting_type); setting_type:=GetWord(buf,0);
        StrCopy(p,so__title); title:=GetUnicode(buf);
        StrCopy(p,so__width); width:=GetWord(buf,20);
        StrCopy(p,so__flags); flags:=GetWord(buf,COL_ON) and not COL_REFRESH;
        case setting_type of
          QST_SETTING: begin
            StrCopy(p,so__datatype); datatype:=GetWord(buf,0);
            StrCopy(p,so__module  ); module  :=GetStr (buf);
            StrCopy(p,so__setting ); setting :=GetStr (buf);
          end;

          QST_SCRIPT: begin
            StrCopy(p,so__script); script:=GetUnicode(buf);
          end;

          QST_CONTACTINFO: begin
            StrCopy(p,so__cnftype); cnftype:=GetWord(buf,0);
          end;

          QST_SERVICE: begin
            StrCopy(p,so__service); service:=GetStr(buf);
            StrCopy(p,so__restype); restype:=GetInt(buf,0);
            StrCopy(p,so__wparam_type); wparam._type:=GetInt(buf,0);
            StrCopy(p,so__lparam_type); lparam._type:=GetInt(buf,0);
            StrCopy(p,so__wparam);
            case wparam._type of
              ACF_NUMBER : wparam.value:=GetInt(buf,0);
              ACF_STRING : wparam.value:=uint_ptr(GetStr(buf));
              ACF_UNICODE: wparam.value:=uint_ptr(GetUnicode(buf));
            end;
            StrCopy(p,so__lparam);
            case lparam._type of
              ACF_NUMBER : lparam.value:=GetInt(buf,0);
              ACF_STRING : lparam.value:=uint_ptr(GetStr(buf));
              ACF_UNICODE: lparam.value:=uint_ptr(GetUnicode(buf));
            end;
          end;

          QST_OTHER: begin
            StrCopy(p,so__other); other:=GetWord(buf,0);
          end;
        end;
      end;
    end;
  end;
end;

end.
