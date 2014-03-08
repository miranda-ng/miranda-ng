{service insertion code}
unit mApiCardM;

interface

uses windows,messages;

type
  tmApiCard = class
  private
    function  GetDescription:pAnsiChar; 
    function  GetResultType :pAnsiChar; 
    procedure SetCurrentService(item:pAnsiChar);
    function  GetWindowStatus:boolean;
  public
    constructor Create(fname:pAnsiChar; lparent:HWND=0);
    destructor Destroy; override;
    procedure FillList(combo:HWND; mode:integer=0);

    function NameFromList(cb:HWND):pAnsiChar;
    function HashToName(ahash:longword):pAnsiChar;
    function FillParams(wnd:HWND{;item:pAnsiChar};wparam:boolean):pAnsiChar;
    function GetParam(wparam:boolean):pAnsiChar;
    procedure Show;//(item:pAnsiChar);

    property Description:pAnsiChar read GetDescription;
    property ResultType :pAnsiChar read GetResultType;
    property Service    :pAnsiChar write SetCurrentService;
    property Event      :pAnsiChar write SetCurrentService;
    property IsShown    :boolean   read GetWindowStatus;
  private
    storage:pointer;
    current:pointer;
    namespace: array [0.. 63] of AnsiChar;
    parent,
    HelpWindow:HWND;
    isServiceHelp:boolean;

    procedure Update(item:pAnsiChar=nil);
  end;

function CreateServiceCard(parent:HWND=0):tmApiCard;
function CreateEventCard  (parent:HWND=0):tmApiCard;

implementation

{
  mirutils unit is for ConvertFileName function only
  m_api is for TranslateW and TrandlateDialogDefault
}
uses common,io,m_api,mirutils,memini,wrapper;

{$r mApiCard.res}

{$include i_card_const.inc}

const
  WM_UPDATEHELP = WM_USER+100;

const
  BufSize = 2048;

const
  ApiHlpFile = 'plugins\services.ini';
{
  ServiceHlpFile = 'plugins\services.ini';
  EventsHlpFile  = 'plugins\events.ini';
}
function tmApiCard.GetResultType:pAnsiChar;
var
  buf:array [0..2047] of AnsiChar;
  p:pAnsiChar;
begin
  if storage<>nil then
  begin
    StrCopy(buf,GetParamSectionStr(current,'return',''));
    p:=@buf;
    while p^ in sWordOnly do inc(p);
    p^:=#0;
    StrDup(result,@buf);
  end
  else
    result:=nil;
end;

function tmApiCard.GetDescription:pAnsiChar;
begin
  if storage<>nil then
  begin
    StrDup(result,GetParamSectionStr(current,'descr',''));
  end
  else
    result:=nil;
end;

function tmApiCard.GetWindowStatus:boolean;
begin
  result:=HelpWindow<>0;
end;

function tmApiCard.GetParam(wparam:boolean):pAnsiChar;
var
  paramname:pAnsiChar;
begin
  if storage=nil then
  begin
    result:=nil;
    exit;
  end;
  if wparam then
    paramname:='wparam'
  else
    paramname:='lparam';

  StrDup(result,GetParamSectionStr(current,paramname,''));
end;

function tmApiCard.FillParams(wnd:HWND{;item:pAnsiChar};wparam:boolean):pAnsiChar;
var
  buf :array [0..2047] of AnsiChar;
  bufw:array [0..2047] of WideChar;
  j:integer;
  p,pp,pc:PAnsiChar;
  tmp:pWideChar;
  paramname:pAnsiChar;
begin
  if storage=nil then
  begin
    result:=nil;
    exit;
  end;
  if wparam then
    paramname:='wparam'
  else
    paramname:='lparam';

  StrCopy(buf,GetParamSectionStr(current,paramname,''));
  StrDup(result,@buf);

  if wnd=0 then
    exit;

  SendMessage(wnd,CB_RESETCONTENT,0,0);
  if buf[0]<>#0 then
  begin
    p:=@buf;
    GetMem(tmp,BufSize*SizeOf(WideChar));
    repeat
      pc:=StrScan(p,'|');
      if pc<>nil then
        pc^:=#0;

      if (p^ in ['0'..'9']) or ((p^='-') and (p[1] in ['0'..'9'])) then
      begin
        j:=0;
        pp:=p;
        repeat
          bufw[j]:=WideChar(pp^);
          inc(j); inc(pp);
        until (pp^=#0) or (pp^=' ');
        if pp^<>#0 then
        begin
          bufw[j]:=' '; bufw[j+1]:='-'; bufw[j+2]:=' '; inc(j,3);
          FastAnsitoWideBuf(pp+1,tmp);
          StrCopyW(bufw+j,TranslateW(tmp));
          SendMessageW(wnd,CB_ADDSTRING,0,lparam(@bufw));
        end
        else
          SendMessageA(wnd,CB_ADDSTRING,0,lparam(p));
      end
      else
      begin
        FastAnsitoWideBuf(p,tmp);
        SendMessageW(wnd,CB_ADDSTRING,0,lparam(TranslateW(tmp)));
        if (p=@buf) and (StrCmp(p,'structure')=0) then
          break;
      end;
      p:=pc+1;
    until pc=nil;
    FreeMem(tmp);
  end;
  SendMessage(wnd,CB_SETCURSEL,0,0);
end;

function tmApiCard.HashToName(ahash:longword):pAnsiChar;
var
  p,pp:PAnsiChar;
begin
  result:=nil;
  if storage<>nil then
  begin
    p:=GetSectionList(storage,namespace);
    pp:=p;
    while p^<>#0 do
    begin
      if ahash=Hash(p,StrLen(p)) then
      begin
        StrDup(result,p);
      end;
      while p^<>#0 do inc(p); inc(p);
    end;
    FreeSectionList(pp);
  end;
end;

function tmApiCard.NameFromList(cb:HWND):pAnsiChar;
var
  buf:array [0..255] of AnsiChar;
  pc:pAnsiChar;
  idx:integer;
begin
  pc:=GetDlgText(cb,true);
  idx:=SendMessage(cb,CB_GETCURSEL,0,0);
  if idx<>CB_ERR then
  begin
    SendMessageA(cb,CB_GETLBTEXT,idx,lparam(@buf));
    // edit field is text from list
    if StrCmp(pc,@buf)=0 then
    begin
      mFreeMem(pc);
      result:=HashToName(CB_GetData(cb,idx));
      exit;
    end;
  end;
  // no select or changed text
  result:=pc;
end;

procedure tmApiCard.FillList(combo:HWND; mode:integer=0);
var
  tmpbuf:array [0..127] of AnsiChar;
  p,pp,pc:PAnsiChar;
  idx:integer;
begin
  if storage<>nil then
  begin
    SendMessage(combo,CB_RESETCONTENT,0,0);
    p:=GetSectionList(storage,namespace);
    pp:=p;
    while p^<>#0 do
    begin
      case mode of
        1: begin // just constant name
          pc:=GetParamStr(storage,p,'alias',nil,namespace);
          if pc=nil then
            pc:=p;
        end;
        2: begin // value (name)
          pc:=StrCopyE(tmpbuf,p);
          pc^:=' '; inc(pc);
          pc^:='('; inc(pc);
          pc:=StrCopyE(pc,GetParamStr(storage,p,'alias',nil,namespace));
          pc^:=')'; inc(pc);
          pc^:=#0;
          pc:=@tmpbuf;
        end;
        3: begin // name 'value'
          pc:=@tmpbuf;
          pc:=StrCopyE(pc,GetParamStr(storage,p,'alias',nil,namespace));
          pc^:=' '; inc(pc);
          pc^:=''''; inc(pc);
          pc:=StrCopyE(pc,p);
          pc^:=''''; inc(pc);
          pc^:=#0;
          pc:=@tmpbuf;
        end;
      else // just constant value
        pc:=p;
      end;
      idx:=SendMessageA(combo,CB_ADDSTRING,0,lparam(pc));
      SendMessageA(combo,CB_SETITEMDATA,idx,Hash(p,StrLen(p)));
      while p^<>#0 do inc(p); inc(p);
    end;
    FreeSectionList(pp);
    SendMessage(combo,CB_SETCURSEL,-1,0);
  end;
end;

function ServiceHelpDlg(Dialog:HWND;hMessage:uint;wParam:WPARAM;lParam:LPARAM):LRESULT; stdcall;
var
  buf:PAnsiChar;
  tmp:PWideChar;
  card:tmApiCard;
begin
  result:=0;
  case hMessage of
    WM_CLOSE: begin
      card:=tmApiCard(GetWindowLongPtr(Dialog,GWLP_USERDATA{DWLP_USER}));
      card.HelpWindow:=0;
      DestroyWindow(Dialog); //??
    end;

    WM_INITDIALOG: begin
      TranslateDialogDefault(Dialog);
      result:=1;
    end;

    WM_COMMAND: begin
      if (wParam shr 16)=BN_CLICKED then
      begin
        case loword(wParam) of
          IDOK,IDCANCEL: begin
            card:=tmApiCard(GetWindowLongPtr(Dialog,GWLP_USERDATA{DWLP_USER}));
            card.HelpWindow:=0;
            DestroyWindow(Dialog);
          end;
        end;
      end;
    end;

    WM_UPDATEHELP: begin
      with tmApiCard(lParam) do
      begin
        if (storage<>nil) and (lParam<>0) and (current<>nil) then
        begin
          GetMem(buf,BufSize);
          GetMem(tmp,BufSize*SizeOf(WideChar));

          SetDlgItemTextA(Dialog,IDC_HLP_SERVICE,GetSectionName(current));

          SetDlgItemTextA(Dialog,IDC_HLP_ALIAS,
              GetParamSectionStr(current,'alias',''));
          
          FastAnsiToWideBuf(GetParamSectionStr(current,'return','Undefined'),tmp);
          SetDlgItemTextW(Dialog,IDC_HLP_RETURN,TranslateW(tmp));

          FastAnsiToWideBuf(GetParamSectionStr(current,'descr','Undefined'),tmp);
          SetDlgItemTextW(Dialog,IDC_HLP_EFFECT,TranslateW(tmp));

          FastAnsiToWideBuf(GetParamSectionStr(current,'plugin',''),tmp);
          SetDlgItemTextW(Dialog,IDC_HLP_PLUGIN,TranslateW(tmp));

          // Parameters
          StrCopy(buf,GetParamSectionStr(current,'wparam','0'));
          if StrScan(buf,'|')<>nil then
          begin
            ShowWindow(GetDlgItem(Dialog,IDC_HLP_WPARAML),SW_SHOW);
            ShowWindow(GetDlgItem(Dialog,IDC_HLP_WPARAM ),SW_HIDE);
            FillParams(GetDlgItem(Dialog,IDC_HLP_WPARAML),true);
          end
          else
          begin
            ShowWindow(GetDlgItem(Dialog,IDC_HLP_WPARAML),SW_HIDE);
            ShowWindow(GetDlgItem(Dialog,IDC_HLP_WPARAM ),SW_SHOW);
            FastAnsiToWideBuf(buf,tmp);
            SetDlgItemTextW(Dialog,IDC_HLP_WPARAM,TranslateW(tmp));
          end;

          StrCopy(buf,GetParamSectionStr(current,'lparam','0'));
          if StrScan(buf,'|')<>nil then
          begin
            ShowWindow(GetDlgItem(Dialog,IDC_HLP_LPARAML),SW_SHOW);
            ShowWindow(GetDlgItem(Dialog,IDC_HLP_LPARAM ),SW_HIDE);
            FillParams(GetDlgItem(Dialog,IDC_HLP_LPARAML),false);
          end
          else
          begin
            ShowWindow(GetDlgItem(Dialog,IDC_HLP_LPARAML),SW_HIDE);
            ShowWindow(GetDlgItem(Dialog,IDC_HLP_LPARAM ),SW_SHOW);
            FastAnsiToWideBuf(buf,tmp);
            SetDlgItemTextW(Dialog,IDC_HLP_LPARAM,TranslateW(tmp));
          end;

          FreeMem(tmp);
          FreeMem(buf);
        end
        else
        begin
          SetDlgItemTextW(Dialog,IDC_HLP_SERVICE,nil);
          SetDlgItemTextW(Dialog,IDC_HLP_ALIAS  ,nil);
          SetDlgItemTextW(Dialog,IDC_HLP_PLUGIN ,nil);
          SetDlgItemTextW(Dialog,IDC_HLP_RETURN ,nil);
          SetDlgItemTextW(Dialog,IDC_HLP_EFFECT ,nil);
          SetDlgItemTextW(Dialog,IDC_HLP_WPARAM ,nil);
          SetDlgItemTextW(Dialog,IDC_HLP_LPARAM ,nil);
          SendDlgItemMessage(Dialog,IDC_HLP_WPARAML,CB_RESETCONTENT,0,0);
          SendDlgItemMessage(Dialog,IDC_HLP_LPARAML,CB_RESETCONTENT,0,0);
          ShowWindow(GetDlgItem(Dialog,IDC_HLP_WPARAML),SW_HIDE);
          ShowWindow(GetDlgItem(Dialog,IDC_HLP_LPARAML),SW_HIDE);
        end;
      end;
    end;
  end;
end;

procedure tmApiCard.SetCurrentService(item:pAnsiChar);
begin
  if (item=nil) or (item^=#0) then
    current:=nil
  else
    current:=SearchSection(storage,item,namespace);
end;

procedure tmApiCard.Update(item:pAnsiChar=nil);
begin
  SendMessage(HelpWindow,WM_UPDATEHELP,0,LPARAM(self));
end;

procedure tmApiCard.Show;
var
  note,
  title:pWideChar;
begin
  if HelpWindow=0 then
  begin
    HelpWindow:=CreateDialogW(hInstance,'IDD_MAPIHELP',//MAKEINTRESOURCEW(IDD_HELP),
                parent,@ServiceHelpDlg);
    if HelpWindow<>0 then
    begin
      SetWindowLongPtr(HelpWindow,GWLP_USERDATA{DWLP_USER},LONG_PTR(Self));
      if isServiceHelp then
      begin
        title:='Miranda service help';
        note :='''<proto>'' in service name will be replaced by protocol name for contact handle in parameter';
      end
      else
      begin
        title:='Miranda event help';
        note :='';
      end;
      SendMessageW(HelpWindow,WM_SETTEXT,0,LPARAM(TranslateW(title)));

      SendMessageW(GetDlgItem(HelpWindow,IDC_HLP_NOTE),WM_SETTEXT,0,LPARAM(TranslateW(Note)));
    end;
  end
  else
  begin
{
    if parent<>GetParent(HelpWindow) then
      SetParent(HelpWindow,parent);
}
  end;
//  if title<>nil then
//    SendMessageW(HelpWindow,WM_SETTEXT,0,TranslateW(title));

  Update(current);
end;

constructor tmApiCard.Create(fname:pAnsiChar; lparent:HWND=0);
var
  IniFile: array [0..511] of AnsiChar;
begin
  inherited Create;

  StrCopy(@IniFile,fname);
  HelpWindow:=0;
  current:=nil;
  if fname<>nil then
  begin
    ConvertFileName(fname,@INIFile);
  //  CallService(MS_UTILS_PATHTOABSOLUTE,
  //    WPARAM(PAnsiChar(ServiceHlpFile)),LPARAM(INIFile));
    if GetFSize(pAnsiChar(@INIFile))=0 then
    begin
      INIFile[0]:=#0;
    end;
    parent:=lparent;
  end;
  storage:=OpenStorage(pAnsiChar(@IniFile));
end;

destructor tmApiCard.Destroy;
begin
  CloseStorage(storage);

  inherited;
end;

function CreateServiceCard(parent:HWND=0):tmApiCard;
begin
  result:=tmApiCard.Create(ApiHlpFile,parent);
  result.isServiceHelp:=true;
  StrCopy(result.namespace,'Service');
end;

function CreateEventCard(parent:HWND=0):tmApiCard;
begin
  result:=tmApiCard.Create(ApiHlpFile,parent);
  result.isServiceHelp:=false;
  StrCopy(result.namespace,'Event');
end;


//initialization
//finalization
end.
