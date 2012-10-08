{service insertion code}
unit mApiCardC;

interface

uses windows,messages;

type
  tmApiCard = class
  private
    function  GetDescription:pAnsiChar; 
    function  GetResultType :pAnsiChar; 
    procedure SetCurrentService(item:pAnsiChar);
  public
    constructor Create(fname:pAnsiChar; lparent:HWND=0);
//    procedure Free;
    procedure FillList(combo:HWND; mode:integer=0);

    function FillParams(wnd:HWND{;item:pAnsiChar};wparam:boolean):pAnsiChar;
    procedure Show;//(item:pAnsiChar);

    property Description:pAnsiChar read GetDescription;
    property ResultType :pAnsiChar read GetResultType;
    property Service    :pAnsiChar write SetCurrentService;
    property Event      :pAnsiChar write SetCurrentService;
  private
    current: array [0..127] of AnsiChar;
    IniFile: array [0..511] of AnsiChar;
    parent,
    HelpWindow:HWND;
    isServiceHelp:boolean;

    procedure Update(item:pAnsiChar=nil);
  end;

function CreateServiceCard(parent:HWND=0):tmApiCard;
function CreateEventCard  (parent:HWND=0):tmApiCard;

implementation

uses common,io,m_api,mirutils;

{$r mApiCard.res}

{$include i_card_const.inc}

const
  WM_UPDATEHELP = WM_USER+100;

const
  BufSize = 2048;

const
  ServiceHlpFile = 'plugins\services.ini';
  EventsHlpFile  = 'plugins\events.ini';
{
procedure tmApiCard.Free;
begin
end;
}
function tmApiCard.GetResultType:pAnsiChar;
var
  buf:array [0..2047] of AnsiChar;
  p:pAnsiChar;
begin
  if INIFile[0]<>#0 then
  begin
    GetPrivateProfileStringA(@current,'return','',buf,SizeOf(buf),@INIFile);
    p:=@buf;
    while p^ in sWordOnly do inc(p);
    p^:=#0;
    StrDup(result,@buf);
  end
  else
    result:=nil;
end;

function tmApiCard.GetDescription:pAnsiChar;
var
  buf:array [0..2047] of AnsiChar;
begin
  if INIFile[0]<>#0 then
  begin
    GetPrivateProfileStringA(@current,'descr','',buf,SizeOf(buf),@INIFile);
    StrDup(result,@buf);
  end
  else
    result:=nil;
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
  if INIFile[0]=#0 then
  begin
    result:=nil;
    exit;
  end;
  if wparam then
    paramname:='wparam'
  else
    paramname:='lparam';
  GetPrivateProfileStringA(@current,paramname,'',buf,SizeOf(buf),@INIFile);
  StrDup(result,@buf);
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
        if (p=@buf) and (lstrcmpia(p,'structure')=0) then
          break;
      end;
      p:=pc+1;
    until pc=nil;
    FreeMem(tmp);
  end;
  SendMessage(wnd,CB_SETCURSEL,0,0);
end;

procedure tmApiCard.FillList(combo:hwnd; mode:integer=0);
var
  buf:array [0..8191] of AnsiChar;
  tmpbuf:array [0..127] of AnsiChar;
  p,pc:PAnsiChar;
begin
  if INIFile[0]<>#0 then
  begin
    SendMessage(combo,CB_RESETCONTENT,0,0);
    buf[0]:=#0;
    GetPrivateProfileSectionNamesA(@buf,SizeOf(buf),@INIFile); // sections
    p:=@buf;
    while p^<>#0 do
    begin
      case mode of
        1: begin // just constant name
          GetPrivateProfileStringA(p,'alias','',tmpbuf,127,@INIFile);
          pc:=@tmpbuf;
        end;
        2: begin // value (name)
          pc:=StrCopyE(tmpbuf,p);
          pc^:=' '; inc(pc);
          pc^:='('; inc(pc);
          GetPrivateProfileStringA(p,'alias','',pc,63,@INIFile);
          pc:=StrEnd(tmpbuf);
          pc^:=')'; inc(pc);
          pc^:=#0;
          pc:=@tmpbuf;
        end;
        3: begin // name 'value'
          GetPrivateProfileStringA(p,'alias','',tmpbuf,127,@INIFile);
          pc:=StrEnd(tmpbuf);
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
      SendMessageA(combo,CB_ADDSTRING,0,lparam(pc));
      while p^<>#0 do inc(p); inc(p);
    end;
    SendMessage(combo,CB_SETCURSEL,-1,0);
  end;
end;

function ServiceHelpDlg(Dialog:HWnd;hMessage:uint;wParam:WPARAM;lParam:LPARAM):LRESULT; stdcall;
var
  buf,p:PAnsiChar;
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
        if (INIFile[0]<>#0) and (lParam<>0) then
        begin
          GetMem(buf,BufSize);
          GetMem(tmp,BufSize*SizeOf(WideChar));
          SetDlgItemTextA(Dialog,IDC_HLP_SERVICE,@current);

          GetPrivateProfileStringA(@current,'alias','',buf,BufSize,@INIFile);
          SetDlgItemTextA(Dialog,IDC_HLP_ALIAS,buf);

          GetPrivateProfileStringA(@current,'return','Undefined',buf,BufSize,@INIFile);
          p:=buf;
          // skip result type
  //        while p^ in sWordOnly do inc(p); if (p<>@buf) and (p^<>#0) then inc(p);
          FastAnsiToWideBuf(p,tmp);
          SetDlgItemTextW(Dialog,IDC_HLP_RETURN,TranslateW(tmp));

          GetPrivateProfileStringA(@current,'descr','Undefined',buf,BufSize,@INIFile);
          FastAnsiToWideBuf(buf,tmp);
          SetDlgItemTextW(Dialog,IDC_HLP_EFFECT,TranslateW(tmp));

          GetPrivateProfileStringA(@current,'plugin','',buf,BufSize,@INIFile);
          FastAnsiToWideBuf(buf,tmp);
          SetDlgItemTextW(Dialog,IDC_HLP_PLUGIN,TranslateW(tmp));
          // Parameters
          GetPrivateProfileStringA(@current,'wparam','0',buf,BufSize,@INIFile);
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

          GetPrivateProfileStringA(@current,'lparam','0',buf,BufSize,@INIFile);
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
  StrCopy(@current,item);
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
      SendMessageW(HelpWindow,WM_SETTEXT,0,LPARAM(title));

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

  Update(@current);
end;

constructor tmApiCard.Create(fname:pAnsiChar; lparent:HWND=0);
begin
  inherited Create;

  StrCopy(@IniFile,fname);
  current[0]:=#0;
  HelpWindow:=0;

  if fname<>nil then
  begin
    ConvertFileName(fname,@INIFile);
  //  PluginLink^.CallService(MS_UTILS_PATHTOABSOLUTE,
  //    dword(PAnsiChar(ServiceHlpFile)),dword(INIFile));
    if GetFSize(pAnsiChar(@INIFile))=0 then
    begin
      INIFile[0]:=#0;
    end;
    parent:=lparent;
  end;
end;

function CreateServiceCard(parent:HWND=0):tmApiCard;
begin
  result:=tmApiCard.Create(ServiceHlpFile,parent);
  result.isServiceHelp:=true;
end;

function CreateEventCard(parent:HWND=0):tmApiCard;
begin
  result:=tmApiCard.Create(EventsHlpFile,parent);
  result.isServiceHelp:=false;
end;


//initialization
//finalization
end.
