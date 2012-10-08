unit sr_global;

interface
uses windows,messages,commctrl,m_api,dbsettings,mirutils;

const
  // for QS window only
  IDM_STAYONTOP = WM_USER+1;
  WM_MYADDCONTACT    = WM_USER+2;
  WM_MYDELETECONTACT = WM_USER+3;

  WM_MYSHOWHIDEITEM = WM_USER + 4;
  WM_MYMOVEITEM     = WM_USER + 5;
const
  wcUp      = 1;
  wcDown    = 2;
  wcHide    = 3;
  wcShow    = 4;
  wcDelete  = 5;
  wcInsert  = 6;
  wcChange  = 7;
  wcRefresh = 8;

const
  opened:boolean = false;
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

const
  ptNumber  = 0;
  ptInteger = 1;
  ptString  = 2;
  ptUnicode = 3;
  ptCurrent = 4;
  
const //types
  ST_BYTE        = 0;
  ST_WORD        = 1;
  ST_INT         = 2;
  ST_STRING      = 3;
  ST_IP          = 4;
  ST_LASTSEEN    = 5;
  ST_CONTACTINFO = 6;
  ST_LASTEVENT   = 7;
  ST_TIMESTAMP   = 8;
  ST_SERVICE     = 9;
  ST_SCRIPT      = 10;
  ST_METACONTACT = 11;

  ST_MAXTYPE     = 11;

const
  COL_ON      = $0001; // Show column
  COL_INIT    = $0002; // No need to update
  COL_XSTATUS = $0100;
  COL_GENDER  = $0200;
  COL_CLIENT  = $0400;

{$include resource.inc}

type
  tserviceparam = record
    case _type:word of
      0: (n:dword);
      1: (i:integer);
      2: (a:PAnsiChar);
      3: (w:PWideChar);
  end;
type
  tcolumnitem=record
    title          :PWideChar;
    module_name    :PAnsiChar;
    width          :dword;
    setting_type   :dword;     // ST_* constants
    setting_cnftype:dword;     // pt* constants
    wparam         :tserviceparam;
    lparam         :tserviceparam;
    flags          :dword;     // COL_* constants
  end;
  tcolumnarray = array of tcolumnitem;
  tqsopt=record
    grrect          :TRECT;
    columns         :tcolumnarray;//array of tcolumnitem;
    numcolumns      :integer;
    columnsort      :integer;
    ascendsort      :boolean;
    sortbystatus    :boolean;
    drawgrid        :boolean;
    showinmenu      :boolean;
    showonlyinlist  :boolean;
    showintoptoolbar:boolean;
    usetoolstyle    :boolean;
    closeafteraction:boolean;
    stayontop       :boolean;
    showoffline     :boolean;
    showclienticons :boolean;
    exportheaders   :boolean;
    singlecsv       :boolean;
    skipminimized   :boolean;
    savepattern     :boolean;
    colorize        :boolean;
  end;

procedure reghotkeys;
procedure unreghotkeys;

procedure saveopt_wnd;
procedure saveopt_db;
procedure loadopt_db(full:boolean);
procedure clear_columns;
function new_column(after:integer=-1):integer;
procedure delete_column(pos:integer);
procedure loaddefaultcolumns;

procedure AddRemoveMenuItemToMainMenu;
procedure addtotoolbar;
procedure removetoolbar;

var
  qsopt:tqsopt;

const
  MainMenuItem:integer=0;
  hTTBButton  :thandle=0;

implementation

uses common;

const
  HKN_GLOBAL:PAnsiChar = 'QS_Global';
const
  so_mbottom         :PAnsiChar = 'mbottom';
  so_mright          :PAnsiChar = 'mright';
  so_mtop            :PAnsiChar = 'mtop';
  so_mleft           :PAnsiChar = 'mleft';
  so_columnsort      :PAnsiChar = 'columnsort';
  so_sortbystatus    :PAnsiChar = 'sortbystatus';
  so_ascendsort      :PAnsiChar = 'ascendsort';
  so_showonlyinlist  :PAnsiChar = 'showonlyinlist';
  so_dontusetoolstyle:PAnsiChar = 'dontusetoolstyle';
  so_showinmenu      :PAnsiChar = 'showinmenu';
  so_showintoptoolbar:PAnsiChar = 'showintoptoolbar';
  so_closeafteraction:PAnsiChar = 'closeafteraction';
  so_exportheaders   :PAnsiChar = 'exportheaders';
  so_singlecsv       :PAnsiChar = 'singlecsv';
  so_savepattern     :PAnsiChar = 'savepattern';
  so_numcolumns      :PAnsiChar = 'numcolumns';
  so_item            :PAnsiChar = 'item';
  so_drawgrid        :PAnsiChar = 'drawgrid';
  so_stayontop       :PAnsiChar = 'stayontop';
  so_showclienticons :PAnsiChar = 'showclienticons';
  so_skipminimized   :PAnsiChar = 'skipminimized';
  so_showoffline     :PAnsiChar = 'showoffline';
  so_colorize        :PAnsiChar = 'colorize';
  
  so__title          :PAnsiChar = '_title';
  so__setting_type   :PAnsiChar = '_setting_type';
  so__setting_cnftype:PAnsiChar = '_setting_cnftype';
  so__module_name    :PAnsiChar = '_module_name';
  so__wparam_type    :PAnsiChar = '_wparam_type';
  so__lparam_type    :PAnsiChar = '_lparam_type';
  so__wparam         :PAnsiChar = '_wparam';
  so__lparam         :PAnsiChar = '_lparam';
  so__width          :PAnsiChar = '_width';
  so__flags          :PAnsiChar = '_flags';

procedure reghotkeys;
var
  hkrec:HOTKEYDESC;
begin
  FillChar(hkrec,SizeOf(hkrec),0);
  with hkrec do
  begin
    cbSize          :=HOTKEYDESC_SIZE_V1;
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
  
  if qsopt.showintoptoolbar then
  begin
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
end;

procedure AddRemoveMenuItemToMainMenu;
var
  cmi:TCLISTMENUITEM;
begin
  if qsopt.showinmenu then
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
  end
  else
  begin
    if (MainMenuItem<>0) and
       (ServiceExists(MS_CLIST_REMOVEMAINMENUITEM)<>0) then
    begin
      CallService(MS_CLIST_REMOVEMAINMENUITEM,MainMenuItem,0);
      MainMenuItem:=0;
    end;
  end;
end;

// -------- column functions ---------

procedure clear_column(num:integer);
begin
  with qsopt.columns[num] do
  begin
    mFreeMem(title);
    if setting_type<>ST_CONTACTINFO then
    begin
      mFreeMem(module_name);
      if setting_type<>ST_SERVICE then
        mFreeMem(wparam.a)
      else
      begin
        if (wparam._type=ptString) or (wparam._type=ptUnicode) then mFreeMem(wparam.a);
        if (lparam._type=ptString) or (lparam._type=ptUnicode) then mFreeMem(lparam.a);
      end;
    end;
  end;
end;

procedure clear_columns;
var
  i:integer;
begin
  for i:=0 to qsopt.numcolumns-1 do
    clear_column(i);
  FillChar(qsopt.columns[0],Length(qsopt.columns),0);
  qsopt.numcolumns:=0;
end;

procedure delete_column(pos:integer);
begin
  if (pos>=0) and (qsopt.numcolumns>0) then
  begin
    dec(qsopt.numcolumns);
    clear_column(pos);
    move(qsopt.columns[pos+1],qsopt.columns[pos],(qsopt.numcolumns-pos)*sizeof(tcolumnitem));
    SetLength(qsopt.columns,qsopt.numcolumns);
  end;
end;

function new_column(after:integer=-1):integer;
begin
  SetLength(qsopt.columns,qsopt.numcolumns+1);
  FillChar(qsopt.columns[qsopt.numcolumns],SizeOf(tcolumnitem),0);
  with qsopt.columns[qsopt.numcolumns] do
  begin
    StrDupW(title,'New column');
    width:=64;
    flags:=COL_ON;
  end;
  result:=qsopt.numcolumns;
  inc(qsopt.numcolumns);
end;

procedure MakeTitle(var title; name:pAnsiChar);
begin
  FastAnsiToWide(name,pWideChar(title));
end;

procedure loaddefaultcolumns;
begin
  clear_columns;
  qsopt.numcolumns:=15;
  SetLength(qsopt.columns    ,qsopt.numcolumns);
  FillChar(qsopt.columns[0],qsopt.numcolumns*SizeOf(tcolumnitem),0);

  // protocol
  with qsopt.columns[0] do
  begin
    MakeTitle(title,'Protocol');
    StrDup (module_name,MS_PROTO_GETCONTACTBASEPROTO);
    width          :=82;
    setting_type   :=ST_SERVICE;
    setting_cnftype:=ptString;
    wparam._type   :=ptCurrent;
    lparam._type   :=ptNumber;
    lparam.n       :=0;
    flags          :=COL_ON;
  end;

  with qsopt.columns[1] do
  begin
    MakeTitle(title,'Real Protocol');
    StrDup(module_name,'Protocol');
    StrDup(wparam.a   ,'p');
    width          :=82;
    setting_type   :=ST_STRING;
    flags          :=0;
  end;

  //gender
  with qsopt.columns[2] do
  begin
    MakeTitle(title,'Gender');
    width          :=20;
    setting_type   :=ST_CONTACTINFO;
    setting_cnftype:=CNF_GENDER;
    flags          :=COL_ON;
  end;

  //uin
  with qsopt.columns[3] do
  begin
    MakeTitle(title,'UserID');
    width          :=80;
    setting_type   :=ST_CONTACTINFO;
    setting_cnftype:=CNF_UNIQUEID;
    flags          :=COL_ON;
  end;

  //username(displayname)
  with qsopt.columns[4] do
  begin
    MakeTitle(title,'Nickname');
    StrDup(module_name,MS_CLIST_GETCONTACTDISPLAYNAME);
    width          :=76;
    setting_type   :=ST_SERVICE;
    setting_cnftype:=ptUnicode;
    wparam._type   :=ptCurrent;
    lparam._type   :=ptNumber;
    lparam.n       :=2; // 0 for ANSI
    flags          :=COL_ON;
  end;

  //firstname
  with qsopt.columns[5] do
  begin
    MakeTitle(title,'First name');
    width          :=68;
    setting_type   :=ST_CONTACTINFO;
    setting_cnftype:=CNF_FIRSTNAME;
    flags          :=COL_ON;
  end;

  //lastname
  with qsopt.columns[6] do
  begin
    MakeTitle(title,'Last name');
    width          :=66;
    setting_type   :=ST_CONTACTINFO;
    setting_cnftype:=CNF_LASTNAME;
    flags          :=COL_ON;
  end;

  //group
  with qsopt.columns[7] do
  begin
    MakeTitle(title,'Group');
    width          :=80;
    StrDup(module_name,'CList');
    StrDup(wparam.a   ,'Group');
    setting_type   :=ST_STRING;
    flags          :=COL_ON;
  end;

  //email
  with qsopt.columns[8] do
  begin
    MakeTitle(title,'E-mail');
    width          :=116;
    setting_type   :=ST_CONTACTINFO;
    setting_cnftype:=CNF_EMAIL;
    flags          :=COL_ON;
  end;

 //miranda version
  with qsopt.columns[9] do
  begin
    MakeTitle(title,'ClientID');
    StrDup(wparam.a,'MirVer');
    width       :=60;
    setting_type:=ST_STRING;
    flags       :=COL_ON;
  end;

 //IP version
  with qsopt.columns[10] do
  begin
    MakeTitle(title,'Ext IP');
    StrDup(module_name,'ICQ');
    StrDup(wparam.a   ,'IP');
    width       :=100;
    setting_type:=ST_IP;
    flags       :=0;
  end;

 //LastSeen
  with qsopt.columns[11] do
  begin
    MakeTitle(title,'LastSeen');
    StrDup(module_name,'SeenModule');
    width       :=116;
    setting_type:=ST_LASTSEEN;
    flags       :=0;
  end;

 //last event
  with qsopt.columns[12] do
  begin
    MakeTitle(title,'Last Event');
    width       :=100;
    setting_type:=ST_LASTEVENT;
    flags       :=0;
  end;

 //online since
  with qsopt.columns[13] do
  begin
    MakeTitle(title,'Online since');
    StrDup(module_name,'ICQ');
    StrDup(wparam.a   ,'LogonTS');
    width       :=100;
    setting_type:=ST_TIMESTAMP;
    flags       :=0;
  end;

 //metacontacts
  with qsopt.columns[14] do
  begin
    MakeTitle(title,'Metacontact');
    width       :=50;
    setting_type:=ST_METACONTACT;
    flags       :=0;
  end;
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
procedure WriteBool(setting:PAnsiChar;value:bool);
begin
  DBWriteByte(0,qs_module,setting,ord(value))
end;

procedure saveopt_wnd;
var
  i:integer;
  buf:array [0..127] of AnsiChar;
  p,pp:PAnsiChar;
begin
  WriteInt (so_mbottom    ,qsopt.grrect.bottom);
  WriteInt (so_mright     ,qsopt.grrect.right);
  WriteInt (so_mtop       ,qsopt.grrect.top);
  WriteInt (so_mleft      ,qsopt.grrect.left);

  WriteBool(so_showoffline,qsopt.showoffline);
  WriteBool(so_colorize   ,qsopt.colorize);

  WriteInt (so_columnsort ,qsopt.columnsort);

  pp:=StrCopyE(buf,so_item);
  for i:=0 to qsopt.numcolumns-1 do
  begin
    p:=StrEnd(IntToStr(pp,i));
    with qsopt.columns[i] do
    begin
      StrCopy(p,so__flags); WriteInt (buf,flags);
      StrCopy(p,so__width); WriteWord(buf,width);
    end;
  end;
end;

{
  "fast" writing"
  order array - if column order only changed
  column flags - if checkboxes changed only
}
procedure saveopt_db;
var
  i:integer;
  buf:array [0..127] of AnsiChar;
  p,pp:PAnsiChar;
begin
  WriteWord(so_numcolumns      ,qsopt.numcolumns);

  WriteBool(so_sortbystatus    ,qsopt.sortbystatus);
  WriteBool(so_showinmenu      ,qsopt.showinmenu);
//  WriteInt (so_columnsort      ,qsopt.columnsort);
  WriteBool(so_ascendsort      ,qsopt.ascendsort);
  WriteBool(so_showonlyinlist  ,qsopt.showonlyinlist);

  WriteBool(so_showintoptoolbar,qsopt.showintoptoolbar);
  WriteBool(so_dontusetoolstyle,not qsopt.usetoolstyle);
  WriteBool(so_closeafteraction,qsopt.closeafteraction);
  WriteBool(so_drawgrid        ,qsopt.drawgrid);
  WriteBool(so_stayontop       ,qsopt.stayontop);
  WriteBool(so_showclienticons ,qsopt.showclienticons);
  WriteBool(so_exportheaders   ,qsopt.exportheaders);
  WriteBool(so_singlecsv       ,qsopt.singlecsv);
  WriteBool(so_skipminimized   ,qsopt.skipminimized);
  WriteBool(so_savepattern     ,qsopt.savepattern);

  pp:=StrCopyE(buf,so_item);
  for i:=0 to qsopt.numcolumns-1 do
  begin
    p:=StrEnd(IntToStr(pp,i));
    with qsopt.columns[i] do
    begin
      StrCopy(p,so__title);
      WriteUnicode(buf,title);
      case setting_type of
        ST_SCRIPT: begin
          StrCopy(p,so__wparam);
          WriteUnicode(buf,wparam.w);
        end;
        ST_CONTACTINFO: begin
          StrCopy(p,so__setting_cnftype); WriteWord(buf,setting_cnftype);
        end;
        ST_SERVICE: begin
          StrCopy(p,so__module_name    ); WriteStr (buf,module_name);
          StrCopy(p,so__setting_cnftype); WriteWord(buf,setting_cnftype);
          StrCopy(p,so__wparam_type    ); WriteWord(buf,wparam._type);
          StrCopy(p,so__lparam_type    ); WriteWord(buf,lparam._type);
          StrCopy(p,so__wparam);
          case wparam._type of
            ptNumber,
            ptInteger: WriteInt    (buf,wparam.n);
            ptString : WriteStr    (buf,wparam.a);
            ptUnicode: WriteUnicode(buf,wparam.w);
          end;
          StrCopy(p,so__lparam);
          case lparam._type of
            ptNumber,
            ptInteger: WriteInt    (buf,lparam.n);
            ptString : WriteStr    (buf,lparam.a);
            ptUnicode: WriteUnicode(buf,lparam.w);
          end;
        end;
      else
        StrCopy(p,so__module_name); WriteStr(buf,module_name);
        StrCopy(p,so__wparam     ); WriteStr(buf,wparam.a);
      end;
      StrCopy(p,so__setting_type); WriteWord(buf,setting_type);
      StrCopy(p,so__flags       ); WriteInt (buf,flags);
      StrCopy(p,so__width       ); WriteWord(buf,width);
    end;
  end;
end;

function GetInt(setting:PAnsiChar;default:integer):integer;
begin
  result:=DBReadDWord(0,qs_module,setting,default);
end;
function GetWord(setting:PAnsiChar;default:word):word;
begin
  result:=DBReadWord(0,qs_module,setting,default);
end;
function GetBool(setting:PAnsiChar;default:bool):bool;
begin
  result:=bool(DBReadByte(0,qs_module,setting,integer(default)));
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
procedure loadopt_db(full:boolean);
var
  i:integer;
  buf:array [0..127] of AnsiChar;
  p,pp:PAnsiChar;
begin
  if full then
  begin
    zeromemory(@qsopt,sizeof(qsopt));

    qsopt.grrect.bottom:=GetInt(so_mbottom,240);
    qsopt.grrect.right :=GetInt(so_mright,550);
    qsopt.grrect.top   :=GetInt(so_mtop,0);
    qsopt.grrect.left  :=GetInt(so_mleft,0);

    qsopt.columnsort      :=GetInt (so_columnsort,StatusSort);
    qsopt.sortbystatus    :=GetBool(so_sortbystatus,true);
    qsopt.ascendsort      :=GetBool(so_ascendsort  ,true);

    qsopt.showonlyinlist  :=GetBool(so_showonlyinlist  ,false);
    qsopt.usetoolstyle    :=not GetBool(so_dontusetoolstyle,false);
    qsopt.showinmenu      :=GetBool(so_showinmenu      ,true);
    qsopt.showintoptoolbar:=GetBool(so_showintoptoolbar,true);
    qsopt.closeafteraction:=GetBool(so_closeafteraction,false);
    qsopt.drawgrid        :=GetBool(so_drawgrid        ,true);
    qsopt.stayontop       :=GetBool(so_stayontop       ,false);
    qsopt.singlecsv       :=GetBool(so_singlecsv       ,false);
    qsopt.exportheaders   :=GetBool(so_exportheaders   ,false);
    qsopt.showoffline     :=GetBool(so_showoffline     ,true);
    qsopt.skipminimized   :=GetBool(so_skipminimized   ,true);
    qsopt.savepattern     :=GetBool(so_savepattern     ,true);
    qsopt.colorize        :=GetBool(so_colorize        ,true);

    if ServiceExists(MS_FP_GETCLIENTICON)<>0 then
      qsopt.showclienticons:=GetBool(so_showclienticons,true)
    else
      qsopt.showclienticons:=false;
  end
  else
    clear_columns;

  qsopt.numcolumns:=GetWord(so_numcolumns,0);
  if qsopt.numcolumns=0 then
  begin
    loaddefaultcolumns;
    saveopt_db;
  end
  else
  begin
    pp:=StrCopyE(buf,so_item);
    SetLength(qsopt.columns,qsopt.numcolumns);
    FillChar(qsopt.columns[0],SizeOf(tcolumnitem)*qsopt.numcolumns,0);
    for i:=0 to qsopt.numcolumns-1 do
    begin
      p:=StrEnd(IntToStr(pp,i));
      with qsopt.columns[i] do
      begin
        StrCopy(p,so__title);
        title:=GetUnicode(buf);
        StrCopy(p,so__setting_type); setting_type:=GetWord(buf,0);
        case setting_type of
          ST_SCRIPT: begin
            StrCopy(p,so__wparam);
            wparam.w:=GetUnicode(buf);
          end;
          ST_CONTACTINFO: begin
            StrCopy(p,so__setting_cnftype); setting_cnftype:=GetWord(buf,0);
          end;
          ST_SERVICE: begin
            StrCopy(p,so__setting_cnftype); setting_cnftype:=GetWord(buf,0);
            StrCopy(p,so__module_name); module_name :=GetStr(buf);
            StrCopy(p,so__wparam_type); wparam._type:=GetWord(buf,0);
            StrCopy(p,so__lparam_type); lparam._type:=GetWord(buf,0);
            StrCopy(p,so__wparam);
            case wparam._type of
              ptNumber,
              ptInteger: wparam.n:=GetInt(buf,0);
              ptString : wparam.a:=GetStr(buf);
              ptUnicode: wparam.w:=GetUnicode(buf);
            end;
            StrCopy(p,so__lparam);
            case lparam._type of
              ptNumber,
              ptInteger: lparam.n:=GetInt(buf,0);
              ptString : lparam.a:=GetStr(buf);
              ptUnicode: lparam.w:=GetUnicode(buf);
            end;
          end;
        else
          StrCopy(p,so__module_name); module_name:=GetStr(buf);
          StrCopy(p,so__wparam     ); wparam.a   :=GetStr(buf);
        end;
        StrCopy(p,so__width); width:=GetWord(buf,20);
        StrCopy(p,so__flags); flags:=GetInt (buf,COL_ON);
      end;
    end;
  end;
end;

end.
