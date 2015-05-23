unit iac_inout;

interface

implementation

uses
  windows, messages, commctrl,
  iac_global, global,
  mirutils, mircontacts, common, dbsettings,
  wrapper, editwrapper, io, syswin,
  m_api;

{$include i_cnst_inout.inc}
{$resource iac_inout.res}

const
  opt_file = 'file';
const
  ioObject       = 'object';
  ioClipboard    = 'clipboard';
  ioOper         = 'oper';
  ioCopy         = 'copy';
  ioFile         = 'file';
  ioFileVariable = 'modvariables';
  ioWrite        = 'write';
  ioAppend       = 'append';
  ioEnc          = 'enc';
const
  ACF_CLIPBRD   = $00000002; // Clipboard operations, not window
  ACF_ANSI      = $00000004; // File: ANSI or Unicode (UTF8/UTF16) text
  ACF_COPYTO    = $00000008; // Clipboard operations: 'copy to' or 'paste from'

  ACF_FILE      = $00000010; // File operations
  ACF_FWRITE    = $00000020; // read/write file
  ACF_FAPPEND   = $00000040; // append file
  ACF_UTF8      = $00000080; // File: UTF8 or UTF16
  ACF_SIGN      = $00000100; // File: with signature or not
  ACF_FILE_PATH = $00000200;

  ACF_TEXTSEND  = $00000400;
  // dummy
  ACF_MESSAGE = 0;

const // V2
  ACF2_TXT_FILE = $00000001;

type
  tInOutAction = class(tBaseAction)
  private
    tfile:pWideChar;
  public
    constructor Create(uid:dword);
    destructor Destroy; override;
//    function  Clone:tBaseAction; override;
    function  DoAction(var WorkData:tWorkData):LRESULT; override;
    procedure Save(node:pointer;fmt:integer); override;
    procedure Load(node:pointer;fmt:integer); override;
  end;

//----- Support functions -----

//----- Object realization -----

constructor tInOutAction.Create(uid:dword);
begin
  inherited Create(uid);

  tfile:=nil;
end;

destructor tInOutAction.Destroy;
begin
  mFreeMem(tfile);

  inherited Destroy;
end;
{
function tInOutAction.Clone:tBaseAction;
begin
  result:=.Create(0);
  Duplicate(result);

end;
}
function tInOutAction.DoAction(var WorkData:tWorkData):LRESULT;
var
  tmp:PWideChar;
  blob,p:PAnsiChar;
  w:PWideChar;
  hContact:TMCONTACT;
  wnd:HWND;
  fexist:bool;
  dbei:TDBEVENTINFO;
  i:cardinal;
  cp:integer;
  fh:THANDLE;
  lstr:pWideChar;
  llen:integer;
  buf:array [0..31] of WideChar;
  b,b1:array [0..MAX_PATH] of AnsiChar;

  last:pWideChar;
begin
  result:=0;

  if WorkData.ResultType=rtInt then
    last:=pWideChar(IntToStr(buf,WorkData.LastResult))
  else
    last:=pWideChar(WorkData.LastResult);

  // Clipboard
  if (flags and ACF_CLIPBRD)<>0 then
  begin
    if (flags and ACF_COPYTO)<>0 then
      CopyToClipboard(last,false)
    else
    begin
      ClearResult(WorkData);
      WorkData.LastResult:=uint_ptr(PasteFromClipboard(false));
      WorkData.ResultType:=rtWide;
    end;
    exit;
  end;

  // File
  if (flags and ACF_FILE)<>0 then
  begin
    if (flags and ACF_FILE_PATH)<>0 then
    begin
      if CallService(MS_DB_CONTACT_IS,WorkData.Parameter,0)<>0 then
        hContact:=WorkData.Parameter
      else
        hContact:=0;
      tmp:=ParseVarString(tfile,hContact,last)
    end
    else
      tmp:=tfile;

    // File write
    if (flags and (ACF_FAPPEND or ACF_FWRITE))<>0 then
    begin
      if      (flags and ACF_ANSI)=ACF_ANSI then cp:=1 // Ansi
      else if (flags and (ACF_UTF8 or ACF_SIGN))=ACF_UTF8 then cp:=2 // UTF8
      else if (flags and (ACF_UTF8 or ACF_SIGN))=ACF_SIGN then cp:=4 // Wide+Sign
      else if (flags and (ACF_UTF8 or ACF_SIGN))=(ACF_UTF8 or ACF_SIGN) then cp:=3 // UTF8+Sign
      else cp:=0; // Wide

      case cp of
        1: begin
          llen:=StrLen(WideToAnsi(last,pAnsiChar(lstr),MirandaCP));
        end;
        2,3: begin
          llen:=StrLen(WideToUTF8(last,pAnsiChar(lstr)));
        end;
      else
        lstr:=last;
        llen:=StrLenW(lstr)*SizeOf(WideChar);
      end;

      fexist:=FileExists(tmp);
      // Append file
      if fexist and ((flags and ACF_FAPPEND)<>0) then
      begin
        fh:=Append(tmp);
        if fh<>THANDLE(INVALID_HANDLE_VALUE) then
        begin
          BlockWrite(fh,lstr^,llen);
        end;
        if (cp<>0) and (cp<>4) then
          mFreeMem(lstr);
      end
      // Write file
      else if ((flags and ACF_FWRITE)<>0) or
              (not fexist and ((flags and ACF_FAPPEND)<>0)) then
      begin
        fh:=ReWrite(tmp);
        if fh<>THANDLE(INVALID_HANDLE_VALUE) then
        begin
          if cp=3 then
          begin
            i:=SIGN_UTF8;
            BlockWrite(fh,i,3); // UTF8 sign
          end
          else if cp=4 then
          begin
            i:=SIGN_UNICODE;
            BlockWrite(fh,i,2); // UTF16 sign
          end;

          BlockWrite(fh,lstr^,llen);
          if (cp<>0) and (cp<>4) then
            mFreeMem(lstr);
        end;
      end
      else
        fh:=THANDLE(INVALID_HANDLE_VALUE);
    end
    // File read
    else
    begin
      // remove file - download
      if StrPosW(tmp,'://')<>nil then // remote
      begin
        GetTempPathA(MAX_PATH,b);
        GetTempFileNameA(b,'wat',GetCurrentTime,b1);
        GetFile(FastWideToAnsiBuf(tmp,b),b1);
        if tmp<>tfile then
          mFreeMem(tmp);
        FastAnsiToWide(b1,tmp);
      end
      else
        b1[0]:=#0;
      fh:=Reset(tmp);
      // process file
      if fh<>THANDLE(INVALID_HANDLE_VALUE) then
      begin
        i:=GetFSize(tmp);
        mGetMem (w ,i+SizeOf(WideChar));
        FillChar(w^,i+SizeOf(WideChar),0);
        BlockRead(fh,w^,i);
        if (flags and ACF_ANSI)<>0 then
        begin
          AnsiToWide(pAnsiChar(w),lstr,MirandaCP);
          mFreeMem(w);
          w:=lstr;
        end
        else if (flags and ACF_UTF8)<>0 then
        begin
          UTF8ToWide(pAnsiChar(w),lstr);
          mFreeMem(w);
          w:=lstr;
        end
        else
          ChangeUnicode(w);

        ClearResult(WorkData);
        WorkData.LastResult:=uint_ptr(w);
        WorkData.ResultType:=rtWide;
      end;
      if b1[0]<>#0 then
        DeleteFileA(b1);
    end;

    if fh<>THANDLE(INVALID_HANDLE_VALUE) then
      CloseHandle(fh);
    if tmp<>tfile then
      mFreeMem(tmp);
    exit;
  end;

  // Message
  wnd:=WaitFocusedWndChild(GetForegroundWindow){GetFocus};

  // with Autosend
  if (flags and ACF_TEXTSEND)<>0 then
  begin
    if wnd<>0 then
      hContact:=WndToContact(wnd)
    else
      hContact:=0;

    if hContact=0 then
    begin
      if CallService(MS_DB_CONTACT_IS,WorkData.Parameter,0)<>0 then
        hContact:=WorkData.Parameter;
    end;

    if hContact=0 then exit;

    p:=GetContactProtoAcc(hContact);
    if DBReadByte(hContact,p,'ChatRoom',0)<>1 then
    begin
      WidetoUTF8(last,blob);
      CallContactService(hContact,PSS_MESSAGE,0,tlparam(blob));
      dbei.cbSize   :=sizeof(dbei);
      dbei.cbBlob   :=StrLen(blob);
      dbei.pBlob    :=pByte(blob);
      dbei.eventType:=EVENTTYPE_MESSAGE;
      dbei.timestamp:=GetCurrentTime;
      dbei.szModule :=p;
      dbei.flags    :=DBEF_SENT or DBEF_UTF;
      db_event_add(hContact, @dbei);
      mFreeMem(blob);
    end
    else
      SendToChat(hContact,last);
  end
  else
  begin
    GetWindowThreadProcessId(GetForegroundWindow,@i);
    if (i=GetCurrentProcessId) and (wnd<>0) then
      SendMessageW(wnd,EM_REPLACESEL,1,tlparam(last))
    else
      SendString(0,last);
  end;

end;

procedure tInOutAction.Load(node:pointer;fmt:integer);
var
  section: array [0..127] of AnsiChar;
  pc:pAnsiChar;
  tmp:pWideChar;
  flags2:dword;
begin
  inherited Load(node,fmt);
  case fmt of
    0: begin
      pc:=StrCopyE(section,pAnsiChar(node));
      if (flags and ACF_FILE)<>0 then
      begin
        StrCopy(pc,opt_file); tfile:=DBReadUnicode(0,DBBranch,section,nil);
      end;
    end;

    100: begin
      pc:=StrCopyE(section,pAnsiChar(node));
      if (flags and ACF_FILE)<>0 then
      begin
        StrCopy(pc,opt_file); tfile:=DBReadUnicode(0,DBBranch,section,nil);
      end;

      StrCopy(pc,'flags2'); flags2:=DBReadDWord(0,DBBranch,section,0);
      if (flags2 and ACF2_TXT_FILE)<>0 then
        flags:=flags or ACF_FILE_PATH;
    end;

    1: begin
      with xmlparser do
      begin
        tmp:=getAttrValue(HXML(node),ioObject);
        if lstrcmpiw(tmp,ioClipboard)=0 then
        begin
          flags:=flags or ACF_CLIPBRD;
          tmp:=getAttrValue(HXML(node),ioOper);
          if lstrcmpiw(tmp,ioCopy)=0 then flags:=flags or ACF_COPYTO;
  //        else if lstrcmpiw(tmp,'paste')=0 then ;
        end
        else
        begin
          if lstrcmpiw(tmp,ioFile)=0 then
          begin

            if StrToInt(getAttrValue(HXML(node),ioFileVariable))=1 then
              flags:=flags or ACF_FILE_PATH;

            flags:=flags or ACF_FILE;
            StrDupW(tfile,getAttrValue(HXML(node),ioFile));
            tmp:=getAttrValue(HXML(node),ioOper);
            if      lstrcmpiw(tmp,ioWrite )=0 then flags:=flags or ACF_FWRITE
            else if lstrcmpiw(tmp,ioAppend)=0 then flags:=flags or ACF_FAPPEND;
            case StrToInt(getAttrValue(HXML(node),ioEnc)) of
              0: flags:=flags or ACF_ANSI;
              1: flags:=flags or ACF_UTF8;
              2: flags:=flags or ACF_UTF8 or ACF_SIGN;
              3: flags:=flags or 0;
              4: flags:=flags or ACF_SIGN;
            end;
          end;
        end;
      end;
    end;
{
    2: begin
      pc:=GetParamSectionStr(node,ioObject);
      if lstrcmpi(tmp,ioClipboard)=0 then
      begin
        flags:=flags or ACF_CLIPBRD;
        pc:=GetParamSectionStr(node,ioOper);
        if lstrcmpi(pc,ioCopy)=0 then flags:=flags or ACF_COPYTO;
//        else if lstrcmpi(pc,'paste')=0 then ;
      end
      else
      begin
        if lstrcmpi(pc,ioFile)=0 then
        begin
          flags:=flags or ACF_FILE;

          if GetParamSectionInt(node,ioFileVariable))=1 then
            flags:=flags or ACF_FILE_PATH;

          UTF8ToWide(GetParamSectionStr(node,ioFile),tfile);

          pc:=GetParamSectionStr(node,ioOper);
          if      lstrcmpi(pc,ioWrite )=0 then flags:=flags or ACF_FWRITE
          else if lstrcmpi(pc,ioAppend)=0 then flags:=flags or ACF_FAPPEND;

          case GetParamSectionInt(node,ioEnc)) of
            0: flags:=flags or ACF_ANSI;
            1: flags:=flags or ACF_UTF8;
            2: flags:=flags or ACF_UTF8 or ACF_SIGN;
            3: flags:=flags or 0;
            4: flags:=flags or ACF_SIGN;
          end;
        end;
      end;
    end;
}
  end;
end;

procedure tInOutAction.Save(node:pointer;fmt:integer);
var
  section: array [0..127] of AnsiChar;
  pc:pAnsiChar;
begin
  inherited Save(node,fmt);
  case fmt of
    0: begin
      pc:=StrCopyE(section,pAnsiChar(node));
      if (flags and ACF_FILE)<>0 then
      begin
        StrCopy(pc,opt_file); DBWriteUnicode(0,DBBranch,section,tfile);
      end;
    end;
{
    1: begin
    end;
}
    13: begin
{
  ACF_CLIPBRD   = $00000002; // Clipboard operations, not window
  ACF_ANSI      = $00000004; // File: ANSI or Unicode (UTF8/UTF16) text
  ACF_COPYTO    = $00000008; // Clipboard operations: 'copy to' or 'paste from'

  ACF_FILE      = $00000010; // File operations
  ACF_FWRITE    = $00000020; // read/write file
  ACF_FAPPEND   = $00000040; // append file
  ACF_UTF8      = $00000080; // File: UTF8 or UTF16
  ACF_SIGN      = $00000100; // File: with signature or not
  ACF_FILE_PATH = $00000200;

  ACF_TEXTSEND  = $00000400;
}
    end;
  end;
end;

//----- Dialog realization -----

procedure FillFileName(Dialog:HWND;idc:integer);
var
  pw,ppw:pWideChar;
begin
  mGetMem(pw,1024*SizeOf(WideChar));
  ppw:=GetDlgText(Dialog,idc);
  if ShowDlgW(pw,ppw) then
  begin
    SetDlgItemTextW(Dialog,idc,pw);
    SetEditFlags(Dialog,idc,EF_SCRIPT,0);
  end;
  mFreeMem(ppw);
  mFreeMem(pw);
end;

procedure MakeTextTypeList(wnd:HWND);
begin
  SendMessage(wnd,CB_RESETCONTENT,0,0);
  InsertString(wnd,0,'Ansi');
  InsertString(wnd,1,'UTF8');
  InsertString(wnd,2,'UTF8+sign');
  InsertString(wnd,3,'UTF16');
  InsertString(wnd,4,'UTF16+sign');
  SendMessage(wnd,CB_SETCURSEL,0,0);
end;

procedure SetSet(Dialog:HWND;num:integer);
var
  lclip,lfile,lmess:boolean;
begin
  case num of
    0: begin lclip:=true ; lfile:=false; lmess:=false; end;
    1: begin lclip:=false; lfile:=true ; lmess:=false; end;
//    2: begin lclip:=false; lfile:=false; lmess:=true; end;
  else
    lclip:=false; lfile:=false; lmess:=true;
  end;
  EnableWindow(GetDlgItem(Dialog,IDC_CLIP_COPYTO),lclip);
  EnableWindow(GetDlgItem(Dialog,IDC_CLIP_PASTE ),lclip);

  EnableWindow(GetDlgItem(Dialog,IDC_FILE_READ   ),lfile);
  EnableWindow(GetDlgItem(Dialog,IDC_FILE_WRITE  ),lfile);
  EnableWindow(GetDlgItem(Dialog,IDC_FILE_APPEND ),lfile);
  EnableWindow(GetDlgItem(Dialog,IDC_FILE_ENC    ),lfile);
  EnableWindow(GetDlgItem(Dialog,IDC_FILE_FILEBTN),lfile);
  EnableEditField(Dialog,IDC_FILE_PATH,lfile);

  EnableWindow(GetDlgItem(Dialog,IDC_TEXT_SEND),lmess);
end;

procedure ClearFields(Dialog:HWND);
begin
  SetDlgItemTextW(Dialog,IDC_FILE_PATH,nil);
  SetEditFlags(Dialog,IDC_FILE_PATH,EF_ALL,0);

  CheckDlgButton(Dialog,IDC_FLAG_CLIP   ,BST_UNCHECKED);
  CheckDlgButton(Dialog,IDC_FLAG_FILE   ,BST_UNCHECKED);
  CheckDlgButton(Dialog,IDC_FLAG_MESSAGE,BST_UNCHECKED);

  CheckDlgButton(Dialog,IDC_CLIP_COPYTO,BST_UNCHECKED);
  CheckDlgButton(Dialog,IDC_CLIP_PASTE ,BST_UNCHECKED);

  CheckDlgButton(Dialog,IDC_FILE_READ  ,BST_UNCHECKED);
  CheckDlgButton(Dialog,IDC_FILE_WRITE ,BST_UNCHECKED);
  CheckDlgButton(Dialog,IDC_FILE_APPEND,BST_UNCHECKED);

  CheckDlgButton(Dialog,IDC_TEXT_SEND,BST_UNCHECKED);
end;

function DlgProc(Dialog:HWND;hMessage:uint;wParam:WPARAM;lParam:LPARAM):LRESULT; stdcall;
const
  NoProcess:boolean=true;
var
  i:integer;
begin
  result:=0;

  case hMessage of
    WM_INITDIALOG: begin
      TranslateDialogDefault(Dialog);

      MakeTextTypeList(GetDlgItem(Dialog,IDC_FILE_ENC));

      MakeEditField(Dialog,IDC_FILE_PATH);
    end;

    WM_ACT_SETVALUE: begin
      NoProcess:=true;
      ClearFields(Dialog);

      with tInOutAction(lParam) do
      begin
        if (flags and ACF_CLIPBRD)<>0 then
        begin
          CheckDlgButton(Dialog,IDC_FLAG_CLIP,BST_CHECKED);
          if (flags and ACF_COPYTO)<>0 then
            CheckDlgButton(Dialog,IDC_CLIP_COPYTO,BST_CHECKED)
          else
            CheckDlgButton(Dialog,IDC_CLIP_PASTE,BST_CHECKED);
          SetSet(Dialog,0);
        end

        else if (flags and ACF_FILE)<>0 then
        begin
          CheckDlgButton(Dialog,IDC_FLAG_FILE,BST_CHECKED);

          if (flags and ACF_ANSI)<>0 then
            i:=0
          else if (flags and ACF_UTF8)<>0 then
          begin
            if (flags and ACF_SIGN)<>0 then
              i:=2
            else
              i:=1
          end
          else if (flags and ACF_SIGN)<>0 then
            i:=4
          else
            i:=3;
          CB_SelectData(GetDlgItem(Dialog,IDC_FILE_ENC),i);

          if      (flags and ACF_FAPPEND)<>0 then CheckDlgButton(Dialog,IDC_FILE_APPEND,BST_CHECKED)
          else if (flags and ACF_FWRITE )<>0 then CheckDlgButton(Dialog,IDC_FILE_WRITE ,BST_CHECKED)
          else                                    CheckDlgButton(Dialog,IDC_FILE_READ  ,BST_CHECKED);
          SetDlgItemTextW(Dialog,IDC_FILE_PATH,tfile);
          SetEditFlags(Dialog,IDC_FILE_PATH,EF_SCRIPT,ord((flags and ACF_FILE_PATH)<>0));

          SetSet(Dialog,1);
        end
        else
        begin
          CheckDlgButton(Dialog,IDC_FLAG_MESSAGE,BST_CHECKED);

          if (flags and ACF_TEXTSEND)<>0 then CheckDlgButton(Dialog,IDC_TEXT_SEND,BST_CHECKED);

          SetSet(Dialog,2);
        end;
      end;
      NoProcess:=false;
    end;

    WM_ACT_RESET: begin
      NoProcess:=true;
      ClearFields(Dialog);

      CheckDlgButton(Dialog,IDC_FLAG_CLIP,BST_CHECKED);
      CheckDlgButton(Dialog,IDC_CLIP_COPYTO,BST_CHECKED);
      CheckDlgButton(Dialog,IDC_FILE_READ,BST_CHECKED);
      SetSet(Dialog,0); // clipboard
      NoProcess:=false;
    end;

    WM_ACT_SAVE: begin
      with tInOutAction(lParam) do
      begin
//        flags:=0;
        // Clipboard
        if IsDlgButtonChecked(Dialog,IDC_FLAG_CLIP)<>BST_UNCHECKED then
        begin
          flags:=flags or ACF_CLIPBRD;
          if IsDlgButtonChecked(Dialog,IDC_CLIP_COPYTO)<>BST_UNCHECKED then
            flags:=flags or ACF_COPYTO;
        end
        // text file
        else if IsDlgButtonChecked(Dialog,IDC_FLAG_FILE)<>BST_UNCHECKED then
        begin
          flags:=flags or ACF_FILE;
          case CB_GetData(GetDlgItem(Dialog,IDC_FILE_ENC)) of
            0: flags:=flags or ACF_ANSI;
            1: flags:=flags or ACF_UTF8;
            2: flags:=flags or ACF_UTF8 or ACF_SIGN;
            3: flags:=flags or 0;
            4: flags:=flags or ACF_SIGN;
          end;

          tfile:=GetDlgText(Dialog,IDC_FILE_PATH);
          if (GetEditFlags(Dialog,IDC_FILE_PATH) and EF_SCRIPT)<>0 then flags:=flags or ACF_FILE_PATH;

          if IsDlgButtonChecked(Dialog,IDC_FILE_APPEND)<>BST_UNCHECKED then
            flags:=flags or ACF_FAPPEND
          else if IsDlgButtonChecked(Dialog,IDC_FILE_WRITE)<>BST_UNCHECKED then
            flags:=flags or ACF_FWRITE;
        end
        // Message
        else
        begin
          if IsDlgButtonChecked(Dialog,IDC_TEXT_SEND)<>BST_UNCHECKED then
            flags:=flags or ACF_TEXTSEND;
        end;
      end;
    end;

    WM_COMMAND: begin
      case wParam shr 16 of
        CBN_SELCHANGE,
        EN_CHANGE: if not NoProcess then
            SendMessage(GetParent(GetParent(Dialog)),PSM_CHANGED,0,0);

        BN_CLICKED: begin
          case loword(wParam) of
            IDC_FLAG_CLIP: begin
              SetSet(Dialog,0);
            end;
            IDC_FLAG_FILE: begin
              SetSet(Dialog,1);
            end;
            IDC_FLAG_MESSAGE: begin
              SetSet(Dialog,2);
            end;
            IDC_FILE_FILEBTN: begin
              FillFileName(Dialog,IDC_FILE_PATH);
            end;
          end;
          SendMessage(GetParent(GetParent(Dialog)),PSM_CHANGED,0,0);
        end;
      end;
    end;

    WM_HELP: begin
      result:=1;
    end;

  end;
end;

//----- Export/interface functions -----

var
  vc:tActModule;

function CreateAction:tBaseAction;
begin
  result:=tInOutAction.Create(vc.Hash);
end;

function CreateDialog(parent:HWND):HWND;
begin
  result:=CreateDialogW(hInstance,'IDD_INOUT',parent,@DlgProc);
end;

procedure Init;
begin
  vc.Next    :=ModuleLink;

  vc.Name    :='In/Out';
  vc.Dialog  :=@CreateDialog;
  vc.Create  :=@CreateAction;
  vc.Icon    :='IDI_INOUT';

  ModuleLink :=@vc;
end;

begin
  Init;
end.
