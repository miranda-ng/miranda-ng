unit iac_jump;

interface

implementation

uses
  windows, messages, commctrl,
  m_api, dbsettings,
  global,iac_global, editwrapper, dlgshare,
  common, mirutils, wrapper;

{$include i_cnst_jump.inc}
{$resource iac_jump.res}

const // condition code
  aeGT  = 1;
  aeLT  = 2;
  aeEQ  = 3;
  aeXR  = 4;
  aeND  = 5;

  aeEMP = 1;
  aeEQU = 2;
  aeCON = 3;
  aeSTR = 4;
  aeEND = 5;
const
  opt_value     = 'value';
  opt_condition = 'condition';
  opt_label     = 'label';
const
  ioIf     = 'IF';
  ioCond   = 'cond';
  ioNop    = 'nop';
  ioNot    = 'not';
  ioValue  = 'value';
  ioOper   = 'oper';
  ioAction = 'action';
  ioLabel  = 'label';
  ioBreak  = 'break';
  ioJump   = 'jump';
  ioPost   = 'POST';
  ioCase   = 'case';
  ioBack   = 'back';
const
  ACF_NOP   = $00000001;
  ACF_MATH  = $00000002;
  ACF_NOT   = $00000004;
  ACF_CASE  = $00000008;
  ACF_BREAK = $00000010;
  ACF_BACK  = $00000020;
  ACF_VALUE = $00000100;

const // V2
  ADV_ACT_BREAK = 1;
  ADV_COND_NOT  = $80;
  ADV_COND_GT   = 1;
  ADV_COND_LT   = 2;
  ADV_COND_EQ   = 3;

type
  tJumpAction = class(tBaseAction)
  private
    value    :pWideChar;
    actlabel :pWideChar;
    condition:integer;
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

constructor tJumpAction.Create(uid:dword);
begin
  inherited Create(uid);

  condition:=0;
  value    :=nil;
  actlabel :=nil;
end;

destructor tJumpAction.Destroy;
begin
  mFreeMem(value);
  mFreeMem(actlabel);

  inherited Destroy;
end;
{
function tJumpAction.Clone:tBaseAction;
begin
  result:=tJumpAction.Create(0);
  Duplicate(result);

  result.condition:=condition;
  StrDupW(result.value,value);
  StrDupW(result.actlabel,actlabel);
end;
}
function tJumpAction.DoAction(var WorkData:tWorkData):LRESULT;
var
  tmpint:int64;
  vnum:int_ptr;
  vstr,vlast:pWideChar;
  buf:array [0..31] of WideChar;
  res:boolean;
  vlr,vval:pWideChar;
  tmp:pWideChar;
  delta:integer;
  lptr:pBaseAction;
begin
  result:=0;

  // Condition
  if (flags and ACF_NOP)=0 then
  begin
    // preparing value
    if WorkData.ResultType=rtInt then
      vlast:=IntToStr(buf,WorkData.LastResult)
    else
      vlast:=pWideChar(WorkData.LastResult);

    if (flags and ACF_VALUE)<>0 then
    begin
      vstr:=ParseVarString(value,WorkData.Parameter,vlast);
    end
    else
      vstr:=value;

    res:=false;
    // now comparing
    if (flags and ACF_MATH)<>0 then
    begin
      vnum:=int_ptr(GetResultNumber(WorkData));
      tmpint:=NumToInt(vstr);
      case condition of
        aeGT: res:=vnum>tmpint;
        aeLT: res:=vnum<tmpint;
        aeEQ: res:=vnum=tmpint;
        aeXR: res:=(vnum xor tmpint)<>0;
        aeND: res:=(vnum and tmpint)<>0;
      end;

    end
    else
    begin
      if (condition=aeEMP) and ((vlast=nil) or (vlast[0]=#0)) then
        res:=true
      else
      begin
        if (flags and ACF_CASE)=0 then
        begin
          StrDupW(vlr,vlast);
          StrDupW(vval,vstr);
          CharUpperW(vlr);
          CharUpperW(vval);
        end
        else
        begin
          vlr :=vlast;
          vval:=vstr;
        end;

        if (flags and ACF_BACK)<>0 then
        begin
          tmp:=vlr;
          vlr:=vval;
          vval:=tmp;
        end;

        case condition of
          aeEQU: res:=StrCmpW(vlr,vval)=0;
          aeCON: res:=StrPosW(vlr,vval)<>nil;
          aeSTR: res:=StrPosW(vlr,vval)=vlr;
          aeEND: begin
            delta:=StrLenW(vval)-StrLenW(vlr);
            if delta>=0 then
              res:=StrCmpW(vlr,vval+delta)=0;
          end;
        end;

        if (vlr<>vlast) and (vlr<>vstr) then
        begin
          mFreeMem(vlr);
          mFreeMem(vval);
        end;
      end;
    end;

    if (flags and ACF_NOT)<>0 then
      res:=not res;

    if (flags and ACF_VALUE)<>0 then
      mFreeMem(vstr);
  end
  else
    res:=true;

  // Operation
  if res then
    if (flags and ACF_BREAK)<>0 then
      result:=-1
    else
    begin
      lptr:=pBaseAction(WorkData.ActionList);
      for delta:=0 to WorkData.ActionCount-1 do
      begin
        if StrCmpW(actlabel,lptr^.ActionDescr)=0 then
        begin
          result:=delta+1;
          break;
        end;
        inc(lptr);
      end;
    end;
end;

procedure tJumpAction.Load(node:pointer;fmt:integer);
var
  section: array [0..127] of AnsiChar;
  buf:array [0..31] of WideChar;
  pc:pAnsiChar;
  tmp:pWideChar;
  sub:HXML;
  oper,cond:byte;
begin
  inherited Load(node,fmt);
  case fmt of
    0: begin
      pc:=StrCopyE(section,pAnsiChar(node));
      if (flags and ACF_NOP)=0 then
      begin
        StrCopy(pc,opt_value    ); value    :=DBReadUnicode(0,DBBranch,section,nil);
        StrCopy(pc,opt_condition); condition:=DBReadByte   (0,DBBranch,section,0);
      end;
      if (flags and ACF_BREAK)=0 then
      begin
        StrCopy(pc,opt_label); actlabel:=DBReadUnicode(0,DBBranch,section,nil);
      end;
    end;

    100..199: begin
      flags:=flags and not ACF_MASK;
      pc:=StrCopyE(section,pAnsiChar(node));
      if fmt<>101 then // condition
      begin
        StrCopy(pc,opt_condition); cond:=DBReadByte(0,DBBranch,section);
        if (cond and $0F)=0 then
          flags:=flags or ACF_NOP
        else
        begin
          if (cond and ADV_COND_NOT)<>0 then flags:=flags or ACF_NOT;
          flags:=flags or ACF_MATH;
          StrCopy(pc,opt_value); StrDupW(value,IntToStr(buf,DBReadDWord(0,DBBranch,section)));

          case cond and $0F of
            ADV_COND_GT: condition:=aeGT;
            ADV_COND_LT: condition:=aeLT;
            ADV_COND_EQ: condition:=aeEQ;
          end;
        end;
      end
      else // skip condition, jump
      begin
        flags:=flags or ACF_NOP
      end;

      if fmt>101 then // inverse condition, jump to label
      begin
        flags:=flags xor ACF_NOT;
        buf[0]:='$'; buf[1]:='$';
        IntToStr(PWideChar(@buf[2]),fmt-102);
        StrDupW(actlabel,buf);
      end
      else
      begin
        StrCopy(pc,'action'); oper:=DBReadByte(0,DBBranch,section) and $0F;
        if (oper and ADV_ACT_BREAK)<>0 then
          flags:= flags or ACF_BREAK
        else
        begin
          StrCopy(pc,'operval'); actlabel:=DBReadUnicode(0,DBBranch,section);
        end;
      end;
    end;

    1: begin
      with xmlparser do
      begin
        sub:=getNthChild(HXML(node),ioIf,0);
        if sub<>0 then
        begin
          tmp:=getAttrValue(sub,ioOper);
          if      lstrcmpiw(tmp,'math')=0 then flags:=flags or ACF_MATH
          else if lstrcmpiw(tmp,ioNop )=0 then flags:=flags or ACF_NOP;

          tmp:=getAttrValue(sub,ioCond);
          if lstrcmpiw(tmp,ioNop)=0 then flags:=flags or ACF_NOP // compatibility
          else if (flags and ACF_NOP)=0 then
          begin
            if flags and ACF_MATH<>0 then
            begin
              if      lstrcmpiw(tmp,'gt' )=0 then condition:=aeGT
              else if lstrcmpiw(tmp,'lt' )=0 then condition:=aeLT
              else if lstrcmpiw(tmp,'eq' )=0 then condition:=aeEQ
              else if lstrcmpiw(tmp,'xor')=0 then condition:=aeXR
              else if lstrcmpiw(tmp,'and')=0 then condition:=aeND;
            end
            else
            begin
              if      lstrcmpiw(tmp,'empty')=0 then condition:=aeEMP
              else if lstrcmpiw(tmp,'eq'   )=0 then condition:=aeEQU
              else if lstrcmpiw(tmp,'cont' )=0 then condition:=aeCON
              else if lstrcmpiw(tmp,'start')=0 then condition:=aeSTR
              else if lstrcmpiw(tmp,'ends' )=0 then condition:=aeEND;

              if StrToInt(getAttrValue(sub,ioCase))=1 then
                flags:=flags or ACF_CASE;
              if StrToInt(getAttrValue(sub,ioBack))=1 then
                flags:=flags or ACF_BACK;
            end;
            if StrToInt(getAttrValue(sub,ioNot))=1 then
              flags:=flags or ACF_NOT;

            if ((flags and ACF_MATH)<>0) or (condition<>aeEMP) then
              StrDupW(value,getAttrValue(sub,ioValue));
          end;
        end;

        sub:=getNthChild(HXML(node),ioPost,0);
        if sub<>0 then
        begin
          tmp:=getAttrValue(sub,ioOper);
          if      lstrcmpiw(tmp,ioBreak)=0 then flags:=flags or ACF_BREAK
          else if lstrcmpiw(tmp,ioJump )=0 then StrDupW(actlabel,getAttrValue(sub,ioValue));
        end;
      end;
    end;
{
    2: begin
      pc:=GetParamSectionStr(node,ioOper);
      if      lstrcmpi(pc,'math')=0 then flags:=flags or ACF_MATH
      else if lstrcmpi(pc,ioNop )=0 then flags:=flags or ACF_NOP;

      pc:=GetParamSectionStr(node,ioCond);
      if lstrcmpi(pc,ioNop)=0 then flags:=flags or ACF_NOP // compatibility
      else if (flags and ACF_NOP)=0 then
      begin
        if flags and ACF_MATH<>0 then
        begin
          if      lstrcmpi(pc,'gt' )=0 then condition:=aeGT
          else if lstrcmpi(pc,'lt' )=0 then condition:=aeLT
          else if lstrcmpi(pc,'eq' )=0 then condition:=aeEQ
          else if lstrcmpi(pc,'xor')=0 then condition:=aeXR
          else if lstrcmpi(pc,'and')=0 then condition:=aeND;
        end
        else
        begin
          if      lstrcmpi(pc,'empty')=0 then condition:=aeEMP
          else if lstrcmpi(pc,'eq'   )=0 then condition:=aeEQU
          else if lstrcmpi(pc,'cont' )=0 then condition:=aeCON
          else if lstrcmpi(pc,'start')=0 then condition:=aeSTR
          else if lstrcmpi(pc,'ends' )=0 then condition:=aeEND;

          if GetParamSectionInt(node,ioCase)=1 then
            flags:=flags or ACF_CASE;
          if GetParamSectionInt(node,ioBack)=1 then
            flags:=flags or ACF_BACK;
        end;
        if GetParamSectionInt(node,ioNot)=1 then
          flags:=flags or ACF_NOT;

        if ((flags and ACF_MATH)<>0) or (condition<>aeEMP) then
          UTF8ToWide(GetParamSectionStr(node,ioValue),value);
      end;

      pc:=GetParamSectionStr(node,ioAction);
      if      lstrcmpi(pc,ioBreak)=0 then flags:=flags or ACF_BREAK
      else if lstrcmpi(pc,ioJump )=0 then UTF8ToWide(GetParamSectionStr(node,ioLabel),actlabel);
    end;
}
  end;
end;

procedure tJumpAction.Save(node:pointer;fmt:integer);
var
  section: array [0..127] of AnsiChar;
  pc:pAnsiChar;
begin
  inherited Save(node,fmt);
  case fmt of
    0: begin
      pc:=StrCopyE(section,pAnsiChar(node));
      if (flags and ACF_NOP)=0 then
      begin
        StrCopy(pc,opt_value    ); DBWriteUnicode(0,DBBranch,section,value);
        StrCopy(pc,opt_condition); DBWriteByte   (0,DBBranch,section,condition);
      end;
      if (flags and ACF_BREAK)=0 then
      begin
        StrCopy(pc,opt_label); DBWriteUnicode(0,DBBranch,section,actlabel);
      end;
    end;
{
    1: begin
    end;
}
    13: begin
{
  ACF_NOP   = $00000001;
  ACF_MATH  = $00000002;
  ACF_NOT   = $00000004;
  ACF_CASE  = $00000008;
  ACF_BREAK = $00000010;
  ACF_BACK  = $00000020;
  ACF_VALUE = $00000100;
}
    end;
  end;
end;

//----- Dialog realization -----

procedure FillMathList(Dialog:HWND);
var
  wnd:HWND;
begin
  wnd:=GetDlgItem(Dialog,IDC_JMP_MATH);

  SendMessage(wnd,CB_RESETCONTENT,0,0);

  InsertString(wnd,cardinal(aeGT),'> greater');
  InsertString(wnd,cardinal(aeLT),'< lesser');
  InsertString(wnd,cardinal(aeEQ),'= equ');
  InsertString(wnd,cardinal(aeXR),'^ xor');
  InsertString(wnd,cardinal(aeND),'& and');

  SendMessage(wnd,CB_SETCURSEL,0,0);
end;

procedure FillTextList(Dialog:HWND);
var
  wnd:HWND;
begin
  wnd:=GetDlgItem(Dialog,IDC_JMP_TEXT);

  SendMessage(wnd,CB_RESETCONTENT,0,0);

  InsertString(wnd,cardinal(aeEMP),'empty');
  InsertString(wnd,cardinal(aeEQU),'= equ');
  InsertString(wnd,cardinal(aeCON),'contains');
  InsertString(wnd,cardinal(aeSTR),'starts with');
  InsertString(wnd,cardinal(aeEND),'ends with');

  SendMessage(wnd,CB_SETCURSEL,0,0);
end;

procedure FillActionList(Dialog:HWND);
var
  list,wnd:HWND;
  i,act:integer;
  arr:array [0..127] of WideChar;
  li:LV_ITEMW;
begin
  wnd:=GetDlgItem(Dialog,IDC_JMP_ACTLIST);

  SendMessage(wnd,CB_RESETCONTENT,0,0);

  list:=ActionListWindow;
  act:=SendMessageW(list,LVM_GETITEMCOUNT,0,0);
  i:=0;
  li.mask      :=LVIF_TEXT;
  li.iSubItem  :=0;
  li.pszText   :=@arr;
  li.cchTextMax:=SizeOf(arr) div SizeOf(WideChar);
  while i<act do
  begin
    li.iItem:=i;
    SendMessageW(list,LVM_GETITEMW,0,lparam(@li));
    SendMessageW(wnd,CB_ADDSTRING,0,lparam(PWideChar(@arr)));
    inc(i);
  end;
  SendMessage(wnd,CB_SETCURSEL,0,0);
end;

procedure SetFields(Dialog:HWND);
var
  bmath,btext:boolean;
begin
  if IsDlgButtonChecked(Dialog,IDC_FLAG_NOP)<>BST_UNCHECKED then
  begin
    bmath:=false;
    btext:=false;
  end
  else if IsDlgButtonChecked(Dialog,IDC_FLAG_MATH)<>BST_UNCHECKED then
  begin
    bmath:=true;
    btext:=false;
  end
  else
  begin
    bmath:=false;
    btext:=true;
  end;
  EnableWindow(GetDlgItem(Dialog,IDC_JMP_MATH ),bmath);
  EnableWindow(GetDlgItem(Dialog,IDC_JMP_TEXT ),btext);
  EnableWindow(GetDlgItem(Dialog,IDC_FLAG_CASE),btext);
  EnableWindow(GetDlgItem(Dialog,IDC_FLAG_BACK),btext);
  EnableWindow(GetDlgItem(Dialog,IDC_FLAG_NOT ),bmath or btext);
  EnableEditField(GetDlgItem(Dialog,IDC_JMP_VALUE),bmath or btext);
  if btext then
  begin
    btext:=CB_GetData(GetDlgItem(Dialog,IDC_JMP_TEXT))<>aeEMP;
    EnableWindow   (GetDlgItem(Dialog,IDC_FLAG_CASE),btext);
    EnableWindow   (GetDlgItem(Dialog,IDC_FLAG_BACK),btext);
    EnableEditField(GetDlgItem(Dialog,IDC_JMP_VALUE),btext);
  end;
end;

procedure ClearFields(Dialog:HWND);
begin
  EnableWindow(GetDlgItem(Dialog,IDC_JMP_MATH ),true);
  EnableWindow(GetDlgItem(Dialog,IDC_JMP_TEXT ),true);
  EnableWindow(GetDlgItem(Dialog,IDC_FLAG_NOT ),true);
  EnableWindow(GetDlgItem(Dialog,IDC_FLAG_CASE),true);
  EnableEditField(GetDlgItem(Dialog,IDC_JMP_VALUE),true);
  SetDlgItemTextW(Dialog,IDC_JMP_VALUE,nil);
  SetEditFlags(Dialog,IDC_JMP_VALUE,EF_ALL,0);

  CheckDlgButton(Dialog,IDC_FLAG_NOP  ,BST_UNCHECKED);
  CheckDlgButton(Dialog,IDC_FLAG_MATH ,BST_UNCHECKED);
  CheckDlgButton(Dialog,IDC_FLAG_TEXT ,BST_UNCHECKED);
  CheckDlgButton(Dialog,IDC_FLAG_NOT  ,BST_UNCHECKED);
  CheckDlgButton(Dialog,IDC_FLAG_CASE ,BST_UNCHECKED);
  CheckDlgButton(Dialog,IDC_FLAG_BACK ,BST_UNCHECKED);
  CheckDlgButton(Dialog,IDC_FLAG_BREAK,BST_UNCHECKED);
  CheckDlgButton(Dialog,IDC_FLAG_JUMP ,BST_UNCHECKED);
end;

function DlgProc(Dialog:HWND;hMessage:uint;wParam:WPARAM;lParam:LPARAM):LRESULT; stdcall;
const
  NoProcess:boolean=true;
var
  bb:boolean;
  wnd:HWND;
  tmp:dword;
begin
  result:=0;

  case hMessage of
    WM_INITDIALOG: begin
      FillMathList(Dialog);
      FillTextList(Dialog);

      TranslateDialogDefault(Dialog);

      MakeEditField(Dialog,IDC_JMP_VALUE);
    end;

    WM_ACT_SETVALUE: begin
      NoProcess:=true;
      ClearFields(Dialog);
      with tJumpAction(lParam) do
      begin
        FillActionList(Dialog);
//        SendDlgItemMessage(Dialog,IDC_JMP_ACTLIST,CB_SETCURSEL,0,0);
        // Condition
        if (flags and ACF_NOP)<>0 then
        begin
          CheckDlgButton(Dialog,IDC_FLAG_NOP,BST_CHECKED);
        end
        else
        begin
          if (flags and ACF_NOT)<>0 then
            CheckDlgButton(Dialog,IDC_FLAG_NOT,BST_CHECKED);
          SetDlgItemTextW(Dialog,IDC_JMP_VALUE,value);
          SetEditFlags(Dialog,IDC_JMP_VALUE,EF_SCRIPT,ord((flags and ACF_VALUE)<>0));

          // Math
          if (flags and ACF_MATH)<>0 then
          begin
            CheckDlgButton(Dialog,IDC_FLAG_MATH,BST_CHECKED);
            CB_SelectData(Dialog,IDC_JMP_MATH,condition);
          end
          // Text
          else
          begin
            if (flags and ACF_CASE)<>0 then
              CheckDlgButton(Dialog,IDC_FLAG_CASE,BST_CHECKED);
            if (flags and ACF_BACK)<>0 then
              CheckDlgButton(Dialog,IDC_FLAG_BACK,BST_CHECKED);
            CheckDlgButton(Dialog,IDC_FLAG_TEXT,BST_CHECKED);
            CB_SelectData(Dialog,IDC_JMP_TEXT,condition);
          end;
        end;
        SetFields(Dialog);

        //Operation
        if (flags and ACF_BREAK)<>0 then
        begin
          CheckDlgButton(Dialog,IDC_FLAG_BREAK,BST_CHECKED);
          EnableWindow(GetDlgItem(Dialog,IDC_JMP_ACTLIST),false);
        end
        else
        begin
          CheckDlgButton(Dialog,IDC_FLAG_JUMP,BST_CHECKED);
          EnableWindow(GetDlgItem(Dialog,IDC_JMP_ACTLIST),true);
          SendDlgItemMessageW(Dialog,IDC_JMP_ACTLIST,CB_SELECTSTRING,
              twparam(-1),tlparam(actlabel));
        end;

      end;
      NoProcess:=false;
    end;

    WM_ACT_RESET: begin
      NoProcess:=true;
      ClearFields(Dialog);

      CheckDlgButton(Dialog,IDC_FLAG_BREAK,BST_CHECKED);
      CheckDlgButton(Dialog,IDC_FLAG_NOP  ,BST_CHECKED);
      SetFields(Dialog);
      CB_SelectData(GetDlgItem(Dialog,IDC_JMP_MATH),aeEQ);
      CB_SelectData(GetDlgItem(Dialog,IDC_JMP_TEXT),aeEQU);
      EnableWindow(GetDlgItem(Dialog,IDC_JMP_ACTLIST),false);
      NoProcess:=false;
    end;

    WM_ACT_SAVE: begin
      with tJumpAction(lParam) do
      begin
        // Condition
        if IsDlgButtonChecked(Dialog,IDC_FLAG_NOP)<>BST_UNCHECKED then
          flags:=flags or ACF_NOP
        else
        begin
          if IsDlgButtonChecked(Dialog,IDC_FLAG_NOT)<>BST_UNCHECKED then
            flags:=flags or ACF_NOT;

          value:=GetDlgText(Dialog,IDC_JMP_VALUE);
          if (GetEditFlags(Dialog,IDC_JMP_VALUE) and EF_SCRIPT)<>0 then
            flags:=flags or ACF_VALUE;

          // math
          if IsDlgButtonChecked(Dialog,IDC_FLAG_MATH)<>BST_UNCHECKED then
          begin
            flags:=flags or ACF_MATH;
            condition:=CB_GetData(GetDlgItem(Dialog,IDC_JMP_MATH));
          end
          // text
          else
          begin
            condition:=CB_GetData(GetDlgItem(Dialog,IDC_JMP_TEXT));
            if condition<>aeEMP then
            begin
              if IsDlgButtonChecked(Dialog,IDC_FLAG_CASE)<>BST_UNCHECKED then
                flags:=flags or ACF_CASE;

              if IsDlgButtonChecked(Dialog,IDC_FLAG_BACK)<>BST_UNCHECKED then
                flags:=flags or ACF_BACK;
            end;
          end;
        end;

        // Operation
        if IsDlgButtonChecked(Dialog,IDC_FLAG_BREAK)<>BST_UNCHECKED then
          flags:=flags or ACF_BREAK
        else
        begin
          actlabel:=GetDlgText(Dialog,IDC_JMP_ACTLIST);
        end;

      end;
    end;

    WM_ACT_LISTCHANGE: begin
      if wParam=2 then
      begin
        wnd:=GetDlgItem(Dialog,IDC_JMP_ACTLIST);
        tmp:=CB_GetData(wnd);
        FillActionList(Dialog);
        CB_SelectData(wnd,tmp);
      end;
    end;

    WM_COMMAND: begin
      case wParam shr 16 of
        EN_CHANGE: if not NoProcess then
            SendMessage(GetParent(GetParent(Dialog)),PSM_CHANGED,0,0);

        CBN_SELCHANGE:  begin
          case loword(wParam) of
            IDC_JMP_TEXT: begin
              bb:=CB_GetData(lParam)<>aeEMP;
              EnableWindow   (GetDlgItem(Dialog,IDC_FLAG_CASE),bb);
              EnableWindow   (GetDlgItem(Dialog,IDC_FLAG_BACK),bb);
              EnableEditField(GetDlgItem(Dialog,IDC_JMP_VALUE),bb);
            end;
          end;
          SendMessage(GetParent(GetParent(Dialog)),PSM_CHANGED,0,0);
        end;

        BN_CLICKED: begin
          case loword(wParam) of
            IDC_FLAG_NOP,
            IDC_FLAG_MATH,
            IDC_FLAG_TEXT: SetFields(Dialog);
            IDC_FLAG_BREAK: begin
              EnableWindow(GetDlgItem(Dialog,IDC_JMP_ACTLIST),false);
            end;
            IDC_FLAG_JUMP: begin
              EnableWindow(GetDlgItem(Dialog,IDC_JMP_ACTLIST),true);
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
  result:=tJumpAction.Create(vc.Hash);
end;

function CreateDialog(parent:HWND):HWND;
begin
  result:=CreateDialogW(hInstance,'IDD_ACTJUMP',parent,@DlgProc);
end;

procedure Init;
begin
  vc.Next    :=ModuleLink;

  vc.Name    :='Jump';
  vc.Dialog  :=@CreateDialog;
  vc.Create  :=@CreateAction;
  vc.Icon    :='IDI_JUMP';

  ModuleLink :=@vc;
end;

begin
  Init;
end.
