unit iac_dbrw;

interface

implementation

uses
  windows, messages, commctrl,
  global, iac_global,
  m_api,dbsettings,
  common,mirutils,wrapper,
  editwrapper,contact,dlgshare;

{$include i_cnst_database.inc}
{$resource iac_database.res}

const
  opt_module  = 'module';
  opt_setting = 'setting';
  opt_value   = 'value';
const
  ioOper         = 'oper';
  ioDelete       = 'delete';
  ioWrite        = 'write';
  ioCurrent      = 'current';
  ioParam        = 'param';
  ioResult       = 'result';
  ioModule       = 'module';
  ioSetting      = 'setting';
  ioContact      = 'contact';
  ioFileVariable = 'modvariables';
  ioArgVariable  = 'argvariables';
  ioVariables    = 'variables';
  ioType         = 'type';
  ioByte         = 'byte';
  ioWord         = 'word';
  ioDword        = 'dword';
  ioAnsi         = 'ansi';
  ioLast         = 'last';
  ioSaveValue    = 'savevalue';

const
  ACF_DBWRITE   = $00000001; // write to (not read from) DB 
  ACF_DBDELETE  = $00000002; // delete setting
  ACF_DBBYTE    = $00000004; // read/write byte (def. dword)
  ACF_DBWORD    = $00000008; // read/write word (def. dword)
  ACF_DBUTEXT   = $00000010; // read/write Unicode string
  ACF_DBANSI    = $00000020; // read/write ANSI string
  ACF_PARAM     = $00000040; // hContact from parameter
  ACF_CURRENT   = $00000080; // hContact is 0 (user settings)
  ACF_RESULT    = $00000100; // hContact is last result value
  ACF_LAST      = $00000200; // use last result for DB writing
  ACF_SAVE      = $00000400; // save writing value to Last Result
  // dummy
  ACF_DBDWORD   = 0;
  ACF_DBREAD    = 0;
  ACF_MANUAL    = 0;

  ACF_NOCONTACT = ACF_PARAM or ACF_CURRENT or ACF_RESULT;
  ACF_VALUETYPE = ACF_DBBYTE or ACF_DBWORD or ACF_DBUTEXT or ACF_DBANSI;
  ACF_TEXT      = ACF_DBUTEXT or ACF_DBANSI;
  ACF_OPERATION = ACF_DBWRITE or ACF_DBDELETE;

  ACF_RW_MODULE  = $00001000; // script for module name
  ACF_RW_SETTING = $00002000; // script for setting name
  ACF_RW_VALUE   = $00004000; // script for data value

type
  tDataBaseAction = class(tBaseAction)
  private
    dbcontact:TMCONTACT;
    dbmodule :PWideChar;
    dbsetting:PWideChar;
    dbvalue  :PWideChar; // keep all in unicode (str to int translation fast)
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

constructor tDataBaseACtion.Create(uid:dword);
begin
  inherited Create(uid);

  dbcontact:=0;
  dbmodule :=nil;
  dbsetting:=nil;
  dbvalue  :=nil;
end;

destructor tDataBaseAction.Destroy;
begin
  mFreeMem(dbmodule);
  mFreeMem(dbsetting);
  mFreeMem(dbvalue);

  inherited Destroy;
end;
{
function tDataBaseAction.Clone:tBaseAction;
var
  tmp:tDataBaseAction;
begin
  result:=tDataBaseAction.Create(0);
  Duplicate(result);

  tmp.dbcontact:=dbcontact;
  StrDupW(tmp.dbmodule ,dbmodule);
  StrDupW(tmp.dbsetting,dbsetting);

  if ((flags and ACF_DBDELETE)=0) and
     ((flags and ACF_LAST)=0)  then
    StrDupW(tmp.dbvalue,dbvalue);

  result:=tmp;
end;
}
function tDataBaseAction.DoAction(var WorkData:tWorkData):LRESULT;
var
  sbuf:array [0..31] of WideChar;
  bufw:array [0..255] of WideChar;
  ambuf,asbuf:array [0..127] of AnsiChar;
  ls,tmp:pWideChar;
  tmpa,tmpa1:pAnsiChar;
  hContact:TMCONTACT;
  proto:pAnsiChar;
  avalue:uint_ptr;
begin
  result:=0;
  // contact
  case (flags and ACF_NOCONTACT) of
    ACF_CURRENT: hContact:=0;
    ACF_PARAM  : hContact:=WorkData.Parameter;
    ACF_RESULT : hContact:=WorkData.LastResult;
  else
    hContact:=dbcontact;
  end;

  //---
  // last result for scripts
  if WorkData.ResultType=rtWide then
    ls:=pWideChar(WorkData.LastResult)
  else
  begin
    ls:=@sbuf;
    IntToStr(sbuf,WorkData.LastResult);
  end;

  proto:=GetContactProtoAcc(hContact);
  // now need to process module
  if (flags and ACF_RW_MODULE)<>0 then
  begin
    tmp:=ParseVarString(dbmodule,hContact,ls);
    StrCopyW(bufw,tmp);
    mFreeMem(tmp);
  end
  else
    StrCopyW(bufw,dbmodule);
  StrReplaceW(@bufw,'<last>',ls);
  FastWideToAnsiBuf(bufw,ambuf,SizeOf(ambuf)-1);
  StrReplace(ambuf,protostr,proto);

  // now process settings
  if (flags and ACF_RW_SETTING)<>0 then
  begin
    tmp:=ParseVarString(dbsetting,hContact,ls);
    StrCopyW(bufw,tmp);
    mFreeMem(tmp);
  end
  else
    StrCopyW(bufw,dbsetting);
  StrReplaceW(@bufw,'<last>',ls);
  FastWideToAnsiBuf(bufw,asbuf,SizeOf(asbuf)-1);
  StrReplace(asbuf,protostr,proto);

  // Delete data
  if (flags and ACF_DBDELETE)<>0 then
  begin
    if (asbuf[0]='*') or (asbuf[StrLen(asbuf)-1]='*') then
      DBDeleteGroup(hContact,ambuf,asbuf)
    else
      DBDeleteSetting(hContact,ambuf,asbuf);
  end
  else
  begin
    if (flags and ACF_LAST)<>0 then
    begin
      avalue:=WorkData.LastResult;
      if WorkData.ResultType=rtInt then // have number
      begin
        if (flags and ACF_DBUTEXT)=ACF_DBUTEXT then // need wide text
          avalue:=uint_ptr(IntToStr(sbuf,avalue))
        else if (flags and ACF_DBANSI)=ACF_DBANSI then // need ansi text
          avalue:=uint_ptr(IntToStr(pAnsiChar(@sbuf),avalue));
      end
      // got wide text
      else if (flags and ACF_TEXT)=0 then // need number
        avalue:=NumToInt(pWideChar(avalue));
{
    val=LR(wide) (wide,ansi)
    val=pointer to static buffer (wide, ansi)
    val=number(number)
}
    end
    else
    begin
      if (flags and ACF_RW_VALUE)<>0 then
      begin
        avalue:=uint_ptr(ParseVarString(dbvalue,hContact,ls));
      end
      else
        avalue:=uint_ptr(dbvalue);

      if (flags and ACF_TEXT)=0 then // need a number
      begin
        tmp:=pWideChar(avalue);
        avalue:=NumToInt(pWideChar(avalue));
        if (flags and ACF_RW_VALUE)<>0 then
          mFreeMem(tmp);
      end;
{
    val=uint_ptr if need number(number)
    val=script result wide(need to free) (wide,ansi)
    val=original dbvalue wide (wide,ansi)
}
    end;
    // Write value
    if (flags and ACF_DBWRITE)<>0 then
    begin
      case (flags and ACF_VALUETYPE) of
        ACF_DBBYTE: DBWriteByte(hContact,ambuf,asbuf,avalue);
        ACF_DBWORD: DBWriteWord(hContact,ambuf,asbuf,avalue);
        ACF_DBANSI: begin
          WideToAnsi(pWideChar(avalue),tmpa,MirandaCP);
          DBWriteString(hContact,ambuf,asbuf,tmpa);
          mFreeMem(tmpa);
        end;
        ACF_DBUTEXT: begin
          DBWriteUnicode(hContact,ambuf,asbuf,pWideChar(avalue));
        end;
      else
        DBWriteDWord(hContact,ambuf,asbuf,avalue);
      end;

      if (flags and ACF_SAVE)<>0 then
      begin
        ClearResult(WorkData);
        case (flags and ACF_VALUETYPE) of
          ACF_DBANSI,
          ACF_DBUTEXT: begin
            StrDupW(pWideChar(WorkData.LastResult),pWideChar(avalue));
            WorkData.ResultType:=rtWide;
          end;
        else
          WorkData.LastResult:=avalue;
          WorkData.ResultType:=rtInt;
        end;
      end;
    end
    // Read value
    else
    begin
      ClearResult(WorkData);
      WorkData.ResultType:=rtInt;
      case (flags and ACF_VALUETYPE) of
        ACF_DBBYTE: WorkData.LastResult:=DBReadByte(hContact,ambuf,asbuf,avalue);
        ACF_DBWORD: WorkData.LastResult:=DBReadWord(hContact,ambuf,asbuf,avalue);
        ACF_DBANSI: begin
          WideToAnsi(pWideChar(avalue),tmpa1,MirandaCP);
          tmpa:=DBReadString(hContact,ambuf,asbuf,tmpa1);
          AnsiToWide(tmpa,PWideChar(WorkData.LastResult),MirandaCP);
          WorkData.ResultType:=rtWide;
          mFreeMem(tmpa1);
          mFreeMem(tmpa);
        end;
        ACF_DBUTEXT: begin
          WorkData.LastResult:=uint_ptr(DBReadUnicode(hContact,ambuf,asbuf,pWideChar(avalue)));
          WorkData.ResultType:=rtWide;
        end
      else
        WorkData.LastResult:=DBReadDWord(hContact,ambuf,asbuf,avalue);
      end;
    end;

    if (flags and ACF_RW_VALUE)<>0 then
    begin
      if (flags and ACF_TEXT)<>0 then
      begin
        mFreeMem(avalue);
      end;
    end;
  end;
end;

procedure tDataBaseAction.Load(node:pointer;fmt:integer);
var
  section: array [0..127] of AnsiChar;
  pc:pAnsiChar;
  tmp:pWideChar;
begin
  inherited Load(node,fmt);
  case fmt of
    0: begin
      if (flags and ACF_NOCONTACT)=0 then
        dbcontact:=LoadContact(DBBranch,node);
      pc:=StrCopyE(section,pAnsiChar(node));
      StrCopy(pc,opt_module ); dbmodule :=DBReadUnicode(0,DBBranch,section);
      StrCopy(pc,opt_setting); dbsetting:=DBReadUnicode(0,DBBranch,section);
      if ((flags and ACF_DBDELETE)=0) and
         ((flags and ACF_LAST)=0)  then
      begin
        StrCopy(pc,opt_value); dbvalue:=DBReadUnicode(0,DBBranch,section);
      end;
    end;

    1: begin
      with xmlparser do
      begin
        tmp:=getAttrValue(HXML(node),ioOper);
        if      lstrcmpiw(tmp,ioDelete)=0 then flags:=flags or ACF_DBDELETE
        else if lstrcmpiw(tmp,ioWrite )=0 then flags:=flags or ACF_DBWRITE;
  //      else if lstrcmpiw(tmp,ioRead)=0 then ;
        tmp:=getAttrValue(HXML(node),ioContact);
        if      lstrcmpiw(tmp,ioCurrent)=0 then flags:=flags or ACF_CURRENT
        else if lstrcmpiw(tmp,ioResult )=0 then flags:=flags or ACF_RESULT
        else if lstrcmpiw(tmp,ioParam  )=0 then flags:=flags or ACF_PARAM
        else if lstrcmpiw(tmp,ioContact)=0 then
        begin
          dbcontact:=ImportContact(HXML(node));
        end;

        StrDupW(dbmodule ,getAttrValue(HXML(node),ioModule));
        StrDupW(dbsetting,getAttrValue(HXML(node),ioSetting));

        if StrToInt(getAttrValue(HXML(node),ioFileVariable))=1 then flags:=flags or ACF_RW_MODULE;
        if StrToInt(getAttrValue(HXML(node),ioArgVariable ))=1 then flags:=flags or ACF_RW_SETTING;
        if StrToInt(getAttrValue(HXML(node),ioVariables   ))=1 then flags:=flags or ACF_RW_VALUE;
	
        tmp:=getAttrValue(HXML(node),ioType);
        if      lstrcmpiw(tmp,ioByte )=0 then flags:=flags or ACF_DBBYTE
        else if lstrcmpiw(tmp,ioWord )=0 then flags:=flags or ACF_DBWORD
        else if lstrcmpiw(tmp,ioDword)=0 then
        else if lstrcmpiw(tmp,ioAnsi )=0 then flags:=flags or ACF_DBANSI
        else                                  flags:=flags or ACF_DBUTEXT;

        if StrToInt(getAttrValue(HXML(node),ioSaveValue))=1 then
          flags:=flags or ACF_SAVE;

        if StrToInt(getAttrValue(HXML(node),ioLast))=1 then
          flags:=flags or ACF_LAST
        else
          StrDupW(dbvalue,getText(HXML(node)));
      end;
    end;
{
    2: begin
      pc:=GetParamSectionStr(node,ioOper);
      if      lstrcmpi(pc,ioDelete)=0 then flags:=flags or ACF_DBDELETE
      else if lstrcmpi(pc,ioWrite )=0 then flags:=flags or ACF_DBWRITE;
//      else if lstrcmpiw(tmp,ioRead)=0 then ;
      pc:=GetParamSectionStr(node,ioContact);
      if      lstrcmpi(pc,ioCurrent)=0 then flags:=flags or ACF_CURRENT
      else if lstrcmpi(pc,ioResult )=0 then flags:=flags or ACF_RESULT
      else if lstrcmpi(pc,ioParam  )=0 then flags:=flags or ACF_PARAM
      else if lstrcmpi(pc,ioContact)=0 then
      begin
        dbcontact:=ImportContactINI(node);
      end;

      UF8ToWide(GetParamSectionStr(node,ioModule ),dbmodule);
      UF8ToWide(GetParamSectionStr(node,ioSetting),dbsetting);

      if GetParamSectionInt(node,ioFileVariable)=1 then flags:=flags or ACF_RW_MODULE;
      if GetParamSectionInt(node,ioArgVariable )=1 then flags:=flags or ACF_RW_SETTING;
      if GetParamSectionInt(node,ioVariables   )=1 then flags:=flags or ACF_RW_VALUE;

      pc:=GetParamSectionStr(node,ioType);
      if      lstrcmpi(pc,ioByte )=0 then flags:=flags or ACF_DBBYTE
      else if lstrcmpi(pc,ioWord )=0 then flags:=flags or ACF_DBWORD
      else if lstrcmpi(pc,ioDword)=0 then
      else if lstrcmpi(pc,ioAnsi )=0 then flags:=flags or ACF_DBANSI
      else                                flags:=flags or ACF_DBUTEXT;

      if GetParamSectionInt(node,ioLast))=1 then
        flags:=flags or ACF_LAST
      else
        UF8ToWide(GetParamSectionStr(node,'value'),dbvalue); //!!
    end;
}
  end;
end;

procedure tDataBaseAction.Save(node:pointer;fmt:integer);
var
  section: array [0..127] of AnsiChar;
  pc:pAnsiChar;
begin
  inherited Save(node,fmt);
  case fmt of
    0: begin
      if (flags and ACF_NOCONTACT)=0 then
        SaveContact(dbcontact,DBBranch,node);
      pc:=StrCopyE(section,pAnsiChar(node));
      StrCopy(pc,opt_module ); DBWriteUnicode(0,DBBranch,section,dbmodule);
      StrCopy(pc,opt_setting); DBWriteUnicode(0,DBBranch,section,dbsetting);
      if ((flags and ACF_DBDELETE)=0) and
         ((flags and ACF_LAST)=0)  then
      begin
        StrCopy(pc,opt_value); DBWriteUnicode(0,DBBranch,section,dbvalue);
      end;
    end;
{
    1: begin
    end;
}
  end;
end;

//----- Dialog realization -----

procedure MakeDataTypeList(wnd:HWND);
begin
  SendMessage(wnd,CB_RESETCONTENT,0,0);
  InsertString(wnd,0,'Byte');
  InsertString(wnd,1,'Word');
  InsertString(wnd,2,'DWord');
  InsertString(wnd,3,'Ansi');
  InsertString(wnd,4,'Unicode');
  SendMessage(wnd,CB_SETCURSEL,0,0);
end;

procedure ClearFields(Dialog:HWND);
begin
  CheckDlgButton(Dialog,IDC_RW_LAST,BST_UNCHECKED);
  CheckDlgButton(Dialog,IDC_RW_SAVE,BST_UNCHECKED);

  CheckDlgButton(Dialog,IDC_RW_CURRENT,BST_UNCHECKED);
  CheckDlgButton(Dialog,IDC_RW_MANUAL ,BST_UNCHECKED);
  CheckDlgButton(Dialog,IDC_RW_PARAM  ,BST_UNCHECKED);
  CheckDlgButton(Dialog,IDC_RW_RESULT ,BST_UNCHECKED);

  CheckDlgButton(Dialog,IDC_RW_READ  ,BST_UNCHECKED);
  CheckDlgButton(Dialog,IDC_RW_WRITE ,BST_UNCHECKED);
  CheckDlgButton(Dialog,IDC_RW_DELETE,BST_UNCHECKED);
end;

function DlgProc(Dialog:HWND;hMessage:uint;wParam:WPARAM;lParam:LPARAM):LRESULT; stdcall;
const
  NoProcess:boolean=true;
var
  wnd:HWND;
  i:integer;
  bb:boolean;
begin
  result:=0;

  case hMessage of
    
    WM_INITDIALOG: begin
      TranslateDialogDefault(Dialog);

      MakeDataTypeList(GetDlgItem(Dialog,IDC_RW_DATATYPE));

      wnd:=GetDlgItem(Dialog,IDC_CNT_REFRESH);
      OptSetButtonIcon(wnd,ACI_REFRESH);
      SendMessage(wnd,BUTTONADDTOOLTIP,TWPARAM(TranslateW('Refresh')),BATF_UNICODE);
      OptFillContactList(GetDlgItem(Dialog,IDC_CONTACTLIST));

      MakeEditField(Dialog,IDC_RW_MODULE);
      MakeEditField(Dialog,IDC_RW_SETTING);
      MakeEditField(Dialog,IDC_RW_VALUE);
    end;

    WM_ACT_SETVALUE: begin
      NoProcess:=true;
      ClearFields(Dialog);

      with tDataBaseAction(lParam) do
      begin
        // operation
        if      (flags and ACF_DBDELETE)<>0 then i:=IDC_RW_DELETE
        else if (flags and ACF_DBWRITE )= 0 then i:=IDC_RW_READ
        else                                     i:=IDC_RW_WRITE;
        CheckDlgButton(Dialog,i,BST_CHECKED);

        // contact
        bb:=false;
        case (flags and ACF_NOCONTACT) of
          ACF_CURRENT: i:=IDC_RW_CURRENT;
          ACF_PARAM  : i:=IDC_RW_PARAM;
          ACF_RESULT : i:=IDC_RW_RESULT;
        else
          i:=IDC_RW_MANUAL;
          bb:=true;
          SendDlgItemMessage(Dialog,IDC_CONTACTLIST,CB_SETCURSEL,
            FindContact(GetDlgItem(Dialog,IDC_CONTACTLIST),dbcontact),0);
        end;
        CheckDlgButton(Dialog,i,BST_CHECKED);
        EnableWindow(GetDlgItem(Dialog,IDC_CONTACTLIST),bb);

        SetDlgItemTextW(Dialog,IDC_RW_MODULE ,dbmodule);
        SetDlgItemTextW(Dialog,IDC_RW_SETTING,dbsetting);
        SetEditFlags(Dialog,IDC_RW_MODULE ,EF_SCRIPT,ord((flags and ACF_RW_MODULE )<>0));
        SetEditFlags(Dialog,IDC_RW_SETTING,EF_SCRIPT,ord((flags and ACF_RW_SETTING)<>0));

        // values
        bb:=true;
        if (flags and ACF_LAST)<>0 then
        begin
          CheckDlgButton(Dialog,IDC_RW_LAST,BST_CHECKED);
          bb:=false;
        end;
        if (flags and ACF_DBDELETE)<>0 then
          bb:=false;
        EnableWindow(GetDlgItem(Dialog,IDC_RW_VALUE),bb);

        if (flags and ACF_SAVE)<>0 then
          CheckDlgButton(Dialog,IDC_RW_SAVE,BST_CHECKED);
        EnableWindow(GetDlgItem(Dialog,IDC_RW_SAVE),(flags and ACF_DBWRITE)<>0);

        case (flags and ACF_VALUETYPE) of
          ACF_DBBYTE : i:=0;
          ACF_DBWORD : i:=1;
          ACF_DBANSI : i:=3;
          ACF_DBUTEXT: i:=4;
        else
          i:=2;
        end;
        CB_SelectData(GetDlgItem(Dialog,IDC_RW_DATATYPE),i);

        SetDlgItemTextW(Dialog,IDC_RW_VALUE,dbvalue);
        SetEditFlags(Dialog,IDC_RW_VALUE,EF_SCRIPT,ord((flags and ACF_RW_VALUE)<>0));
      end;
      NoProcess:=false;
    end;

    WM_ACT_RESET: begin
      NoProcess:=true;
      ClearFields(Dialog);

      SetDlgItemTextW(Dialog,IDC_RW_MODULE ,nil);
      SetDlgItemTextW(Dialog,IDC_RW_SETTING,nil);
      SetDlgItemTextW(Dialog,IDC_RW_VALUE  ,nil);
      SetEditFlags(Dialog,IDC_RW_MODULE ,EF_ALL,0);
      SetEditFlags(Dialog,IDC_RW_SETTING,EF_ALL,0);
      SetEditFlags(Dialog,IDC_RW_VALUE  ,EF_ALL,0);

      CB_SelectData(GetDlgItem(Dialog,IDC_RW_DATATYPE),0);
      CheckDlgButton(Dialog,IDC_RW_READ  ,BST_CHECKED);
      CheckDlgButton(Dialog,IDC_RW_MANUAL,BST_CHECKED);

      EnableWindow(GetDlgItem(Dialog,IDC_CONTACTLIST),true);
      EnableWindow(GetDlgItem(Dialog,IDC_RW_VALUE),true);
      NoProcess:=false;
    end;

    WM_ACT_SAVE: begin
      with tDataBaseAction(lParam) do
      begin
        // contact
        if      IsDlgButtonChecked(Dialog,IDC_RW_CURRENT)=BST_CHECKED then flags:=flags or ACF_CURRENT
        else if IsDlgButtonChecked(Dialog,IDC_RW_RESULT )=BST_CHECKED then flags:=flags or ACF_RESULT
        else if IsDlgButtonChecked(Dialog,IDC_RW_PARAM  )=BST_CHECKED then flags:=flags or ACF_PARAM
        else
          dbcontact:=SendDlgItemMessage(Dialog,IDC_CONTACTLIST,CB_GETITEMDATA,
              SendDlgItemMessage(Dialog,IDC_CONTACTLIST,CB_GETCURSEL,0,0),0);

        {mFreeMem(dbmodule ); }dbmodule :=GetDlgText(Dialog,IDC_RW_MODULE);
        {mFreeMem(dbsetting); }dbsetting:=GetDlgText(Dialog,IDC_RW_SETTING);
        if (GetEditFlags(Dialog,IDC_RW_MODULE ) and EF_SCRIPT)<>0 then flags:=flags or ACF_RW_MODULE;
        if (GetEditFlags(Dialog,IDC_RW_SETTING) and EF_SCRIPT)<>0 then flags:=flags or ACF_RW_SETTING;

        // operation
        if      IsDlgButtonChecked(Dialog,IDC_RW_WRITE )=BST_CHECKED then flags:=flags or ACF_DBWRITE
        else if IsDlgButtonChecked(Dialog,IDC_RW_DELETE)=BST_CHECKED then flags:=flags or ACF_DBDELETE;

        // value
        if IsDlgButtonChecked(Dialog,IDC_RW_LAST)<>BST_UNCHECKED then
          flags:=flags or ACF_LAST
        else if (flags and ACF_DBDELETE)=0 then
        begin
          {mFreeMem(dbvalue); }dbvalue:=GetDlgText(Dialog,IDC_RW_VALUE);
          if (GetEditFlags(Dialog,IDC_RW_VALUE) and EF_SCRIPT)<>0 then flags:=flags or ACF_RW_VALUE;
        end;

        if IsDlgButtonChecked(Dialog,IDC_RW_SAVE)<>BST_UNCHECKED then
          flags:=flags or ACF_SAVE;

        i:=CB_GetData(GetDlgItem(Dialog,IDC_RW_DATATYPE));
        case i of
          0: flags:=flags or ACF_DBBYTE;
          1: flags:=flags or ACF_DBWORD;
          2: flags:=flags or 0;
          3: flags:=flags or ACF_DBANSI;
          4: flags:=flags or ACF_DBUTEXT;
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
            IDC_RW_READ,
            IDC_RW_WRITE,
            IDC_RW_DELETE: begin
              bb:=IsDlgButtonChecked(Dialog,IDC_RW_DELETE)=BST_UNCHECKED;
              EnableWindow(GetDlgItem(Dialog,IDC_RW_DATATYPE),bb);
              EnableWindow(GetDlgItem(Dialog,IDC_RW_LAST),bb);
              if bb then
                bb:=IsDlgButtonChecked(Dialog,IDC_RW_LAST)=BST_UNCHECKED;
              EnableEditField(GetDlgItem(Dialog,IDC_RW_VALUE),bb);

              bb:=loword(wParam)=IDC_RW_WRITE;
              EnableWindow(GetDlgItem(Dialog,IDC_RW_SAVE),bb);
            end;
            IDC_RW_CURRENT,
            IDC_RW_PARAM,
            IDC_RW_RESULT: EnableWindow(GetDlgItem(Dialog,IDC_CONTACTLIST),false);
            IDC_RW_MANUAL: EnableWindow(GetDlgItem(Dialog,IDC_CONTACTLIST),true);

            IDC_RW_LAST: begin
              EnableEditField(GetDlgItem(Dialog,IDC_RW_VALUE),
                  IsDlgButtonChecked(Dialog,IDC_RW_LAST)=BST_UNCHECKED);
            end;

            IDC_CNT_REFRESH: begin
              OptFillContactList(GetDlgItem(Dialog,IDC_CONTACTLIST));
              exit;
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
  result:=tDataBaseAction.Create(vc.Hash);
end;

function CreateDialog(parent:HWND):HWND;
begin
  result:=CreateDialogW(hInstance,'IDD_ACTDATABASE',parent,@DlgProc);
end;

procedure Init;
begin
  vc.Next    :=ModuleLink;

  vc.Name    :='Database';
  vc.Dialog  :=@CreateDialog;
  vc.Create  :=@CreateAction;
  vc.Icon    :='IDI_DATABASE';
  vc.Hash    :=0;

  ModuleLink :=@vc;
end;

begin
  Init;
end.
