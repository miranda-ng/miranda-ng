{}
var
  xmlparser:TXML_API_W;
const
  // Nodes
  ioRoot         :PWideChar = 'ActMan_Export';
  ioAction       :PWideChar = 'Action';
  ioSubAction    :PWideChar = 'SubAction';
  ioContactWindow:PWideChar = 'Contact';
  ioCallService  :PWideChar = 'Service';
  ioRunProgram   :PWideChar = 'Program';
  ioInsertText   :PWideChar = 'In/Out';
  ioAdvanced     :PWideChar = 'Jump';
  ioLinkAction   :PWideChar = 'Chain';
  ioProfile      :PWideChar = 'Database';
  ioMessageBox   :PWideChar = 'MessageBox';
  ioWParam       :PWideChar = 'WPARAM';
  ioLParam       :PWideChar = 'LPARAM';
  ioItem         :PWideChar = 'ITEM';
  ioPost         :PWideChar = 'POST';
  ioIf           :PWideChar = 'IF';
  ioAct          :PWideChar = 'ACT';
  ioOutput       :PWideChar = 'OUTPUT';
  ioClass        :PWideChar = 'class';
  // Attributes
  ioType         :PWideChar = 'type';
  ioInverse      :PWideChar = 'inverse';
  ioName         :PWideChar = 'name';
  ioDisabled     :PWideChar = 'disabled';
  ioVolatile     :PWideChar = 'volatile';
  ioLast         :PWideChar = 'last';
  ioWindow       :PWideChar = 'window';
  ioParallel     :PWideChar = 'parallel';
  ioArgs         :PWideChar = 'args';
  ioWait         :PWideChar = 'wait';
  ioObject       :PWideChar = 'object';
  ioOper         :PWideChar = 'oper';
  ioMessage      :PWideChar = 'message';
//  ioDest         :PWideChar = 'dest';
  ioEnc          :PWideChar = 'enc';
  ioValue        :PWideChar = 'value';
  ioService      :PWideChar = 'service';
  ioNot          :PWideChar = 'not';
  ioCond         :PWideChar = 'cond';
  ioVariables    :PWideChar = 'variables';
  ioFileVariable :PWideChar = 'modvariables';
  ioArgVariable  :PWideChar = 'argvariables';
  ioModule       :PWideChar = 'module';
  ioSetting      :PWideChar = 'setting';
  ioCProto       :PWideChar = 'cproto';
  ioCUIDType     :PWideChar = 'cuidtype';
  ioCUID         :PWideChar = 'cuid';
  ioIsChat       :pWideChar = 'ischat';
  ioTitle        :PWideChar = 'title';
  ioFile         :PWideChar = 'file';
  ioAsInt        :PWideChar = 'asint';
  ioKeepOnly     :PWideChar = 'keeponly';
  ioKeepLast     :PWideChar = 'keeplast';
  ioReturn       :PWideChar = 'return';
  ioLength       :PWideChar = 'length';
  ioFree         :PWideChar = 'free';
  ioPacked       :PWideChar = 'packed';
  
  // Values
  ioNumber       :PWideChar = 'number';
  ioCurrent      :PWideChar = 'current';
  ioContact      :PWideChar = 'contact';
  ioStruct       :PWideChar = 'struct';
  ioResult       :PWideChar = 'result';
  ioParam        :PWideChar = 'param';
  ioByte         :PWideChar = 'byte';
  ioWord         :PWideChar = 'word';
  ioDword        :PWideChar = 'dword';
  ioAnsi         :PWideChar = 'ansi';
  ioUnicode      :PWideChar = 'unicode';
//  ioWStruct      :PWideChar = 'wordstruct';
//  ioBStruct      :PWideChar = 'bytestruct';
  ioHex          :PWideChar = 'hex';
  ioInt          :PWideChar = 'int';
  ioSigned       :PWideChar = 'signed';
  ioPopup        :PWideChar = 'popup';
  ioMsgBox       :PWideChar = 'msgbox';
  ioHidden       :PWideChar = 'hidden';
  ioMinimized    :PWideChar = 'minimized';
  ioMaximized    :PWideChar = 'maximized';
  ioNormal       :PWideChar = 'normal';
  ioClipboard    :PWideChar = 'clipboard';
  ioCopy         :PWideChar = 'copy';
  ioPaste        :PWideChar = 'paste';
  ioBreak        :PWideChar = 'break';
  ioJump         :PWideChar = 'jump';
  ioNop          :PWideChar = 'nop';
//  ioArray        :PWideChar = 'array';
  ioScript       :PWideChar = 'script';
  ioWrite        :PWideChar = 'write';
  ioRead         :PWideChar = 'read';
  ioAppend       :PWideChar = 'append';
  ioDelete       :PWideChar = 'delete';

const
  imp_yes    = 1;
  imp_yesall = 2;
  imp_no     = 3;
  imp_noall  = 4;
  imp_append = 5;

function ReadStruct(act:HXML):int_ptr;
{
var
  child,i:integer;
  tmp:pWideChar;
  sub:HXML;
  typ:pWideChar;
}
begin
  result:=0;
(*
  mGetMem (tmp ,32768);
  FillChar(tmp^,32768,0);
  result:=int_ptr(tmp);
  with xmlparser do
  begin
{
    typ:=getAttrValue(act,ioPacked);
    if (typ<>nil) and (typ^<>#0) and (typ^<>'0') then
    begin
      tmp^:=char_packed; inc(tmp);
    end;
}
    child:=0;
    repeat
      sub:=getNextChild(act,ioItem,@child);
      if sub=0 then break;

      typ:=getAttrValue(sub,ioType);
      for i:=0 to MaxStructTypes-1 do
      begin
        if lstrcmpiw(typ,StructElems[i].short)=0 then break;
      end;
      if StrToInt(getAttrValue(sub,ioReturn))=1 then
      begin
        tmp^:=char_return; inc(tmp);
      end;
      if StrToInt(getAttrValue(sub,ioScript))=1 then
      begin
        tmp^:=char_script; inc(tmp);
      end;
      tmp:=StrCopyEW(tmp,typ);
      tmp^:=' '; inc(tmp);
      case StructElems[i].typ of
        SST_LAST, SST_PARAM: ;
        SST_BYTE,
        SST_WORD,
        SST_DWORD,
        SST_QWORD,
        SST_NATIVE: begin
          tmp:=StrCopyEW(tmp,getAttrValue(sub,ioValue));
        end;
        SST_BARR, SST_WARR,
        SST_BPTR, SST_WPTR: begin
          tmp:=StrCopyEW(tmp,getAttrValue(sub,ioLength));
          tmp^:=' '; inc(tmp);
          tmp:=StrCopyEW(tmp,getAttrValue(sub,ioValue));
        end;
      end;
      tmp^:='|'; inc(tmp);
    until false;
    dec(tmp); tmp^:=#0;
  end;
*)
end;

function ReadParam(act:HXML; var param:int_ptr;isvar:boolean):dword;
var
  tmp:pWideChar;
begin
  result:=0;
  if act=0 then
    exit;
  with xmlparser do
  begin
    tmp:=getAttrValue(act,ioType);
    if      lstrcmpiw(tmp,ioCurrent)=0 then result:=result or ACF_WCURRENT
    else if lstrcmpiw(tmp,ioResult )=0 then result:=result or ACF_WRESULT
    else if lstrcmpiw(tmp,ioParam  )=0 then result:=result or ACF_WPARAM
    else if lstrcmpiw(tmp,ioNumber )=0 then
    begin
      result:=result or ACF_WPARNUM;
      tmp:=getAttrValue(act,ioValue);
      if isvar then
        StrDupW(pWideChar(param),tmp)
      else
        param:=StrToInt(tmp);
    end
    else if lstrcmpiw(tmp,ioStruct)=0 then
    begin
      result:=result or ACF_WSTRUCT;
      param:=ReadStruct(act);
    end
    else if lstrcmpiw(tmp,ioUnicode)=0 then
    begin
      result:=result or ACF_WUNICODE;
      StrDupW(pWideChar(param),getAttrValue(act,ioValue));
    end
    else if lstrcmpiw(tmp,ioAnsi)=0 then
    begin
      WideToAnsi(getAttrValue(act,ioValue),pAnsiChar(param),MirandaCP);
    end;
  end;
end;

function ImportContact(node:HXML):THANDLE;
var
  proto:pAnsiChar;
  tmpbuf:array [0..63] of AnsiChar;
  dbv:TDBVARIANT;
  is_chat:boolean;
  bufLen:int;
begin
  with xmlparser do
  begin
    proto:=FastWideToAnsiBuf(getAttrValue(node,ioCProto),tmpbuf);
    if (proto=nil) or (proto^=#0) then
    begin
      result:=0;
      exit;
    end;
    is_chat:=StrToInt(getAttrValue(node,ioIsChat))<>0;

    if is_chat then
    begin
      dbv.szVal.W:=getAttrValue(node,ioCUID);
    end
    else
    begin
      FillChar(dbv,SizeOf(TDBVARIANT),0);
      dbv._type:=StrToInt(getAttrValue(node,ioCUIDType));
      case dbv._type of
        DBVT_BYTE  : dbv.bVal:=StrToInt(getAttrValue(node,ioCUID));
        DBVT_WORD  : dbv.wVal:=StrToInt(getAttrValue(node,ioCUID));
        DBVT_DWORD : dbv.dVal:=StrToInt(getAttrValue(node,ioCUID));
        DBVT_ASCIIZ: FastWideToAnsi(getAttrValue(node,ioCUID),dbv.szVal.A);
        DBVT_UTF8  : WideToUTF8(getAttrValue(node,ioCUID),dbv.szVal.A);
        DBVT_WCHAR : StrDupW(dbv.szVal.W,getAttrValue(node,ioCUID));
        DBVT_BLOB  : begin
          dbv.pbVal := mir_base64_decode(FastWideToAnsi(getAttrValue(node,ioCUID),pAnsiChar(dbv.pbVal)), bufLen);
          dbv.cpbVal := bufLen;
        end;
      end;
    end;
  end;
  result:=FindContactHandle(proto,dbv,is_chat);
  if not is_chat then
    case dbv._type of
      DBVT_WCHAR,
      DBVT_ASCIIZ,
      DBVT_UTF8  : mFreeMem(dbv.szVal.A);
      DBVT_BLOB  : mFreeMem(dbv.pbVal);
    end;
end;

function ImportAction(actnode:HXML):integer;
var
  tmp:pWideChar;
  act:tHKAction;
  sub:HXML;
begin
  FillChar(act,SizeOf(act),0);
  with xmlparser,act do
  begin
    flags:=ACF_ASSIGNED;
    if StrToInt(getAttrValue(actnode,ioDisabled))=1 then
      flags:=flags or ACF_DISABLED;

    StrDupW(descr,getAttrValue(actnode,ioName));

    actnode:=getChild(actnode,0);
    tmp:=getName(actnode);
//MessageBoxW(0,tmp,'node',0);

    // CONTACT
    if StrCmpW(tmp,ioContactWindow)=0 then
    begin
      actionType:=ACT_CONTACT;
      contact:=ImportContact(actnode);
//      contact:=StrToInt(getAttrValue(actnode,ioNumber));
      if StrToInt(getAttrValue(actnode,ioKeepOnly))=1 then
        flags:=flags or ACF_KEEPONLY;
    end

    // SERVICE
    else if StrCmpW(tmp,ioCallService)=0 then
    begin
      actionType:=ACT_SERVICE;
      FastWideToAnsi(getAttrValue(actnode,ioService),service);
//MessageBoxA(0,service,'service',0);
      if StrToInt(getAttrValue(actnode,ioVariables))=1 then
        flags2:=flags2 or ACF2_SRV_SRVC;

      sub:=getNthChild(actnode,ioWParam,0);
      if StrToInt(getAttrValue(sub,ioVariables))=1 then
        flags2:=flags2 or ACF2_SRV_WPAR;
      if StrToInt(getAttrValue(sub,ioHex))=1 then
        flags2:=flags2 or ACF2_SRV_WHEX;
      flags:=flags or ReadParam(sub,int_ptr(wparam),(flags2 and ACF2_SRV_WPAR)<>0);

      sub:=getNthChild(actnode,ioLParam,0);
      if StrToInt(getAttrValue(sub,ioVariables))=1 then
        flags2:=flags2 or ACF2_SRV_LPAR;
      if StrToInt(getAttrValue(sub,ioHex))=1 then
        flags2:=flags2 or ACF2_SRV_LHEX;
      flags:=flags or (ReadParam(sub,lparam,(flags2 and ACF2_SRV_LPAR)<>0) shl 1);

      sub:=getNthChild(actnode,ioOutput,0);
      if StrToInt(getAttrValue(sub,ioMessage))=1 then flags:=flags or ACF_INSERT;
      if StrToInt(getAttrValue(sub,ioPopup  ))=1 then flags:=flags or ACF_POPUP;
      if StrToInt(getAttrValue(sub,ioMsgBox ))=1 then flags:=flags or ACF_MESSAGE;

      if StrToInt(getAttrValue(sub,ioFree))=1 then flags2:=flags2 or ACF2_FREEMEM;

      tmp:=getAttrValue(sub,ioType);
      if      lstrcmpiw(tmp,ioUnicode)=0 then flags:=flags or ACF_UNICODE+ACF_STRING
      else if lstrcmpiw(tmp,ioAnsi   )=0 then flags:=flags or ACF_STRING
      else if lstrcmpiw(tmp,ioSigned )=0 then flags:=flags or ACF_SIGNED
      else if lstrcmpiw(tmp,ioHex    )=0 then flags:=flags or ACF_HEX
      else if lstrcmpiw(tmp,ioStruct )=0 then flags:=flags or ACF_STRUCT
      else if lstrcmpiw(tmp,ioInt    )=0 then ;
//MessageBoxW(0,'','end',0);
    end

    // PROGRAM
    else if StrCmpW(tmp,ioRunProgram)=0 then
    begin
      actionType:=ACT_PROGRAM;
      StrDupW(prgname,getText(actnode));
      StrDupW(args,getAttrValue(actnode,ioArgs));
      if StrToInt(getAttrValue(actnode,ioCurrent))=1 then
        flags:=flags or ACF_CURPATH;

      if StrToInt(getAttrValue(actnode,ioParallel))=1 then
        flags:=flags or ACF_PRTHREAD
      else
        time:=StrToInt(getAttrValue(actnode,ioWait));

      if StrToInt(getAttrValue(actnode,ioFileVariable))=1 then
        flags2:=flags2 or ACF2_PRG_PRG;

      if StrToInt(getAttrValue(actnode,ioArgVariable))=1 then
        flags2:=flags2 or ACF2_PRG_ARG;

      tmp:=getAttrValue(actnode,ioWindow);
      if      lstrcmpiw(tmp,ioHidden   )=0 then show:=SW_HIDE
      else if lstrcmpiw(tmp,ioMinimized)=0 then show:=SW_SHOWMINIMIZED
      else if lstrcmpiw(tmp,ioMaximized)=0 then show:=SW_SHOWMAXIMIZED
      else                                      show:=SW_SHOWNORMAL;
    end

    // INSERT TEXT
    else if StrCmpW(tmp,ioInsertText)=0 then
    begin
      actionType:=ACT_TEXT;
      tmp:=getAttrValue(actnode,ioObject);
      if lstrcmpiw(tmp,ioClipboard)=0 then
      begin
        flags:=flags or ACF_CLIPBRD;
        tmp:=getAttrValue(actnode,ioOper);
        if lstrcmpiw(tmp,ioCopy)=0 then flags:=flags or ACF_COPYTO;
//        else if lstrcmpiw(tmp,'paste')=0 then ;
        tmp:=getAttrValue(actnode,ioEnc);
        if lstrcmpiw(tmp,ioAnsi)=0 then flags:=flags or ACF_ANSI;
//        else if lstrcmpiw(tmp,'unicode')=0 then ;
      end
      else
      begin
        StrDupW(text,getText(actnode));

        if StrToInt(getAttrValue(actnode,ioVariables))=1 then
          flags2:=flags2 or ACF2_TXT_TEXT;

        if lstrcmpiw(tmp,ioFile)=0 then
        begin

          if StrToInt(getAttrValue(actnode,ioFileVariable))=1 then
            flags2:=flags2 or ACF2_TXT_FILE;

          flags:=flags or ACF_FILE;
          StrDupW(tfile,getAttrValue(actnode,ioFile));
          tmp:=getAttrValue(actnode,ioOper);
          if      lstrcmpiw(tmp,ioWrite )=0 then flags:=flags or ACF_FWRITE
          else if lstrcmpiw(tmp,ioAppend)=0 then flags:=flags or ACF_FAPPEND;
          case StrToInt(getAttrValue(actnode,ioEnc)) of
            0: flags:=flags or ACF_ANSI;
            1: flags:=flags or ACF_UTF8;
            2: flags:=flags or ACF_UTF8 or ACF_SIGN;
            3: flags:=flags or 0;
            4: flags:=flags or ACF_SIGN;
          end;
        end;
      end;
    end

    // ADVANCED
    else if StrCmpW(tmp,ioAdvanced)=0 then
    begin
      actionType:=ACT_ADVANCE;
      sub:=getNthChild(actnode,ioIf,0);
      if sub<>0 then
      begin
        tmp:=getAttrValue(sub,ioCond);
        if      lstrcmpiw(tmp,'gt' )=0 then condition:=condition or ADV_COND_GT
        else if lstrcmpiw(tmp,'lt' )=0 then condition:=condition or ADV_COND_LT
        else if lstrcmpiw(tmp,'eq' )=0 then condition:=condition or ADV_COND_EQ
        else if lstrcmpiw(tmp,ioNop)=0 then ;

        if StrToInt(getAttrValue(sub,ioNot))=1 then
          condition:=condition or ADV_COND_NOT;

        value:=StrToInt(getAttrValue(sub,ioValue));
      end;

      sub:=getNthChild(actnode,ioAct,0);
      tmp:=getAttrValue(sub,ioType);
      if lstrcmpiw(tmp,ioValue)=0 then
      begin
        action:=action or ADV_ACT_MATH;
        tmp:=getAttrValue(sub,ioOper);
        if lstrcmpiw(tmp,ioInverse)=0 then
          oper:=Cardinal(aeNot)
        else
        begin
          case tmp^ of
            '+': oper:=Cardinal(aeAdd);
            '-': oper:=Cardinal(aeSub);
            '*': oper:=Cardinal(aeMul);
            '\': oper:=Cardinal(aeDiv);
            '%': oper:=Cardinal(aeMod);
            '&': oper:=Cardinal(aeAnd);
            '|': oper:=Cardinal(aeOr );
            '^': oper:=Cardinal(aeXor);
            '=': oper:=Cardinal(aeSet);
          end;
          mathval:=StrToInt(getAttrValue(sub,ioValue));
        end;
      end
      else if lstrcmpiw(tmp,ioScript)=0 then
      begin
        if StrToInt(getAttrValue(sub,ioAsInt))<>0 then
          flags:=flags or ACF_VARASINT;
        action:=action or ADV_ACT_VARS;
        StrDupW(varval,getText(sub));
      end;

      sub:=getNthChild(actnode,ioPost,0);
      if sub<>0 then
      begin
        tmp:=getAttrValue(sub,ioOper);
        if      lstrcmpiw(tmp,ioBreak)=0 then action:=action or ADV_ACT_BREAK
        else if lstrcmpiw(tmp,ioJump )=0 then action:=action or ADV_ACT_JUMP
        else if lstrcmpiw(tmp,ioNop  )=0 then ;

        tmp:=getAttrValue(sub,ioValue);
        case action and ADV_ACT_POST of
          ADV_ACT_JUMP: StrDupW(operval,tmp);
        end;
      end;
    end

    // CHAIN
    else if StrCmpW(tmp,ioLinkAction)=0 then
    begin
      actionType:=ACT_CHAIN;
      StrDupW(actname,getText(actnode));
      flags:=flags or ACF_BYNAME;
    end

    // DBRW
    else if StrCmpW(tmp,ioProfile)=0 then
    begin
      actionType:=ACT_RW;
      tmp:=getAttrValue(actnode,ioOper);
      if      lstrcmpiw(tmp,ioDelete)=0 then flags:=flags or ACF_DBDELETE
      else if lstrcmpiw(tmp,ioWrite )=0 then flags:=flags or ACF_DBWRITE;
//      else if lstrcmpiw(tmp,ioRead)=0 then ;
      tmp:=getAttrValue(actnode,ioContact);
      if      lstrcmpiw(tmp,ioCurrent)=0 then flags:=flags or ACF_CURRENT
      else if lstrcmpiw(tmp,ioResult )=0 then flags:=flags or ACF_RESULT 
      else if lstrcmpiw(tmp,ioParam  )=0 then flags:=flags or ACF_PARAM
      else if lstrcmpiw(tmp,ioContact)=0 then
      begin
        contact:=ImportContact(actnode);
      end;

      FastWideToAnsi(getAttrValue(actnode,ioModule ),dbmodule);
      FastWideToAnsi(getAttrValue(actnode,ioSetting),dbsetting);

      if StrToInt(getAttrValue(actnode,ioFileVariable))=1 then flags2:=flags2 or ACF2_RW_MVAR;
      if StrToInt(getAttrValue(actnode,ioArgVariable ))=1 then flags2:=flags2 or ACF2_RW_SVAR;
      if StrToInt(getAttrValue(actnode,ioVariables   ))=1 then flags2:=flags2 or ACF2_RW_TVAR;
      if StrToInt(getAttrValue(actnode,ioHex         ))=1 then flags2:=flags2 or ACF2_RW_HEX;

      tmp:=getAttrValue(actnode,ioType);
      if      lstrcmpiw(tmp,ioByte )=0 then flags:=flags or ACF_DBBYTE
      else if lstrcmpiw(tmp,ioWord )=0 then flags:=flags or ACF_DBWORD
      else if lstrcmpiw(tmp,ioDword)=0 then
      else // if lstrcmpiw(tmp,ioUnicode)=0 then
      begin
        if lstrcmpiw(tmp,ioAnsi)=0 then
          flags:=flags or ACF_DBANSI
        else
          flags:=flags or ACF_DBUTEXT;
        StrDupW(pWideChar(dbvalue),getText(actnode));
      end;

      if StrToInt(getAttrValue(actnode,ioLast))=1 then
        flags:=flags or ACF_LAST
      else if (flags and ACF_DBUTEXT)=0 then
      begin
        if (flags2 and ACF2_RW_TVAR)<>0 then
          StrDupW(pWideChar(dbvalue),getText(actnode))
        else
          dbvalue:=StrToInt(getAttrValue(actnode,ioValue));
      end;
    end

    // MessageBox
    else if StrCmpW(tmp,ioMessageBox)=0 then
    begin
      actionType:=ACT_MESSAGE;
      StrDupW(msgtitle,getAttrValue(actnode,ioTitle));
      StrDupW(msgtext,getText(actnode));
      boxopts:=StrToInt(getAttrValue(actnode,ioType));
      if StrToInt(getAttrValue(actnode,ioArgVariable))=1 then flags2:=flags2 or ACF2_MSG_TXT;
      if StrToInt(getAttrValue(actnode,ioVariables  ))=1 then flags2:=flags2 or ACF2_MSG_TTL;
      if StrToInt(getAttrValue(actnode,ioKeepLast   ))=1 then flags :=flags  or ACF_MSG_KEEP;
    end

    else
    begin
      actionType:=ACT_UNKNOWN;
      result:=0;
      Exit;
    end;
  end;
  result:=NewAction(ActionList,MaxActions);
  move(act,ActionList^[result],SizeOf(tHKAction));
end;

function Import(fname:PWideChar;aflags:dword):integer;
var
  f:THANDLE;
  i,j:integer;
  tmp,res:pWideChar;
  root,actnode:HXML;
  last,next:integer;
  impact:integer;
  buf:array [0..511] of WideChar;
  oldid:dword;
begin
  result:=0;
  for i:=0 to MaxGroups-1 do
    with GroupList[i] do
      if (flags and (ACF_IMPORT or ACF_ASSIGNED))=
                    (ACF_IMPORT or ACF_ASSIGNED) then
        flags:=flags and not (ACF_IMPORT or ACF_OVERLOAD);

  if (fname=nil) or (fname^=#0) then
    exit;
  i:=GetFSize(fname);
  if i=0 then
    exit;
  mGetMem (res ,i+SizeOf(WideChar));
  FillChar(res^,i+SizeOf(WideChar),0);
  f:=Reset(fname);
  BlockRead(f,res^,i);
  CloseHandle(f);

//MessageBoxW(0,res,'SRC',0);
  xmlparser.cbSize:={XML_API_SIZEOF_V1;//}SizeOf(TXML_API_W);
  CallService(MS_SYSTEM_GET_XI,0,lparam(@xmlparser));
  with xmlparser do
  begin
    root:=parseString(ChangeUnicode(res),@i,nil);
    j:=0;
    impact:=imp_yes;
    repeat
      actnode:=getNthChild(root,ioAction,j);
      if actnode=0 then break;
//MessageBoxW(0,'not zero','actnode',0);
//??      if StrCmpW(getName(actnode),ioAction)<>0 then break;
      tmp:=getAttrValue(actnode,ioName);
//MessageBoxW(0,tmp,'nodename',0);
      if tmp<>nil then //!!
      begin
        i:=GetActIdxByName(tmp);
        oldid:=$FFFFFFFF;
        if i>=0 then
        begin
          if (impact<>imp_yesall) and (impact<>imp_noall) then
          begin
            StrCopyW(buf,TranslateW('Action "$" exists, do you want to rewrite it?'));
            impact:=DialogBoxParam(hInstance,MAKEINTRESOURCE(IDD_ASK),0,
            @QuestionDlg,TLPARAM(StrReplaceW(buf,'$',tmp)));
          end;
          if (impact=imp_yesall) or (impact=imp_yes) then
          begin
            oldid:=GroupList^[i].id;
            FreeGroup(i);
          end;
        end;
        if (i<0) or (impact=imp_yesall) or (impact=imp_yes) or (impact=imp_append) then
        begin
          with GroupList^[NewGroup(GroupList,MaxGroups)] do
          begin
            if (i>=0) and (oldid<>$FFFFFFFF) then // set old id to keep UseAction setting
            begin
              flags:=flags or ACF_IMPORT or ACF_OVERLOAD;
              id:=oldid;
            end;
            flags:=flags or ACF_IMPORT;
            if StrToInt(getAttrValue(actnode,ioDisabled))=1 then flags:=flags or ACF_DISABLED;
            if StrToInt(getAttrValue(actnode,ioVolatile))=1 then flags:=flags or ACF_VOLATILE;
            mFreeMem(descr);
            StrDupW(descr,tmp);
            i:=0;
            last:=-1;
//MessageBoxW(0,descr,'descr',0);
            repeat
              next:=ImportAction(getChild(actnode,i));
              if next=0 then
                break;
              if last<0 then
                firstAction:=next
              else
                ActionList[last].next:=next;
              last:=next;
              inc(i);
            until false;
            inc(result);
          end;
        end;
      end;
      inc(j);
    until false;
    DestroyNode(root);
  end;
  mFreeMem(res);
end;

//--------------------------

procedure WriteStruct(node:HXML;struct:PWideChar);
{var
  sub:HXML;
  ppc,value,lsrc,p,pc:pWideChar;
  i,len:integer;
  typ:integer;
}
begin
(*
  if struct=nil then exit;

  mGetMem(pc,4096);
  lsrc:=pWideChar(struct);
  with xmlparser do
  begin
{
    if lsrc^=char_packed then
    begin
      AddAttrInt(node,ioPacked,1);
      inc(lsrc);
    end;
}
    while lsrc^<>#0 do
    begin
      sub:=AddChild(node,ioItem,nil);
      p:=StrScanW(lsrc,'|');
      StrCopyW(pc,lsrc,p-lsrc);

      ppc:=pc;
      if ppc^=char_return then
      begin
        AddAttrInt(sub,ioReturn,1);
        inc(ppc);
      end;

      if ppc^=char_script then
      begin
        AddAttrInt(sub,ioScript,1);
        inc(ppc);
      end;

      typ:=GetOneElement(ppc,len,value);

      i:=0;
      while i<MaxStructTypes do
      begin
        if StructElems[i].typ=typ then //!!
          break;
        inc(i);
      end;
      AddAttr(sub,ioType,StructElems[i].short);


      case typ of
        SST_LAST,SST_PARAM: ;
        SST_BYTE,SST_WORD,SST_DWORD,SST_QWORD,SST_NATIVE: begin
          AddAttr(sub,ioValue,value);
        end;
        SST_BARR,SST_WARR,SST_BPTR,SST_WPTR: begin
          AddAttrInt(sub,ioLength,len);
          AddAttr   (sub,ioValue ,value);
        end;
      end;

      if p=nil then break;
      lsrc:=p+1;
    end;
  end;
  mFreeMem(pc);
*)
end;

procedure WriteParam(node:HXML;flags,param:int_ptr;flags2:integer);
var
  s:pWideChar;
  tmp:pWideChar;
begin
  with xmlparser do
  begin
    if (flags and ACF_WPARNUM)<>0 then
    begin
      s:=ioNumber;
      if (flags2 and ACF2_SRV_WPAR)<>0 then
        AddAttr(node,ioValue,PWideChar(param))
      else
        AddAttrInt(node,ioValue,param);
    end
    else if (flags and ACF_WCURRENT)<>0 then
    begin
      s:=ioCurrent;
    end
    else if (flags and ACF_WRESULT)<>0 then
    begin
      s:=ioResult;
    end
    else if (flags and ACF_WPARAM)<>0 then
    begin
      s:=ioParam;
    end
    else if (flags and ACF_WSTRUCT)<>0 then
    begin
      s:=ioStruct;
      WriteStruct(node,pointer(param));
    end
    else if (flags and ACF_WUNICODE)<>0 then
    begin
      s:=ioUnicode;
      AddAttr(node,ioValue,PWideChar(param));
    end
    else
    begin
      s:=ioAnsi;
      AddAttr(node,ioValue,AnsiToWide(PAnsiChar(param),tmp));
      mFreeMem(tmp);
    end;
    AddAttr(node,ioType,s);
  end;
end;

function ExportContact(node:HXML;hContact:THANDLE):integer;
var
  proto,uid:pAnsiChar;
  cws:TDBVARIANT;
  p1:pAnsiChar;
  p:pWideChar;
  tmpbuf:array [0..63] of WideChar;
  is_chat:boolean;
begin
  result:=0;
  proto:=GetContactProtoAcc(hContact);
  if proto<>nil then
  begin
    is_chat:=IsChat(hContact);
    if is_chat then
    begin
      with xmlparser do
      begin
        p:=DBReadUnicode(hContact,proto,'ChatRoomID');
        addAttr(node,ioCUID,p);
        mFreeMem(p);
      end;
      result:=1;
    end
    else
    begin
      uid:=pAnsiChar(CallProtoService(proto,PS_GETCAPS,PFLAG_UNIQUEIDSETTING,0));
      if DBReadSetting(hContact,proto,uid,@cws)=0 then
      begin
        result:=1;
        with xmlparser do
        begin
          addAttrInt(node,ioCUIDType,cws._type);
          case cws._type of
            DBVT_BYTE  : AddAttrInt(node,ioCUID,cws.bVal);
            DBVT_WORD  : AddAttrInt(node,ioCUID,cws.wVal);
            DBVT_DWORD : AddAttrInt(node,ioCUID,cws.dVal);
            DBVT_ASCIIZ: begin
              AddAttr(node,ioCUID,FastAnsiToWide(cws.szVal.A,p));
              mFreeMem(p);
            end;
            DBVT_UTF8  : begin
              AddAttr(node,ioCUID,UTF8ToWide(cws.szVal.A,p));
      //        AddAttr(node,'cuid',FastAnsiToWide(cws.szVal.A,p));
              mFreeMem(p);
            end;
            DBVT_WCHAR : AddAttr(node,ioCUID,cws.szVal.W);
            DBVT_BLOB  : begin
              p1:=mir_base64_encode(cws.pbVal,cws.cpbVal);
              AddAttr(node,ioCUID,FastAnsiToWide(p1,p));
              mFreeMem(p1);
              mFreeMem(p);
            end;
          end;
        end;
      end;
      DBFreeVariant(@cws);
    end;
    if result<>0 then
    begin
      with xmlparser do
      begin
        addAttr   (node,ioCProto,FastAnsiToWideBuf(proto,tmpbuf));
        addAttrInt(node,ioIsChat,ord(is_chat));
      end;
    end;
  end;
end;

procedure WriteAction(actnode:HXML;idx:integer);
var
  sub, act: HXML;
  s:PWideChar;
  i:integer;
  tmp:pWideChar;
begin
  with xmlparser,ActionList[idx] do
  begin
    actnode:=AddChild(actnode,ioSubAction,nil);
    if descr<>nil then
      AddAttr(actnode,ioName,descr);
    if (flags and ACF_DISABLED)<>0 then
      AddAttrInt(actnode,ioDisabled,1);

    case actionType of
// ----- CONTACT -----
      ACT_CONTACT: begin
        AddAttr(actnode,ioClass,ioContactWindow);
        sub:=actnode;
//        sub:=AddChild(actnode,ioContactWindow,nil);
        ExportContact(sub,contact);
//        AddAttrInt(sub,ioNumber,0); // contact
        if (flags and ACF_KEEPONLY)<>0 then AddAttrInt(sub,ioKeepOnly,1);
      end;
// ----- SERVICE ----- 
      ACT_SERVICE: begin
        AddAttr(actnode,ioClass,ioCallService);
        sub:=actnode;
//        sub:=AddChild(actnode,ioCallService,nil);
        AddAttr(sub,ioService,FastAnsiToWide(service,tmp));
        if (flags2 and ACF2_SRV_SRVC)<>0 then AddAttrInt(sub,ioVariables,1);
        mFreeMem(tmp);
        act:=AddChild(sub,ioWParam,nil); WriteParam(act,flags      ,wparam,flags2);
        if (flags2 and ACF2_SRV_WPAR)<>0 then AddAttrInt(act,ioVariables,1);
        if (flags2 and ACF2_SRV_WHEX)<>0 then AddAttrInt(act,ioHex      ,1);
        act:=AddChild(sub,ioLParam,nil); WriteParam(act,flags shr 1,lparam,flags2 shr 1);
        if (flags2 and ACF2_SRV_LPAR)<>0 then AddAttrInt(act,ioVariables,1);
        if (flags2 and ACF2_SRV_LHEX)<>0 then AddAttrInt(act,ioHex      ,1);

        act:=AddChild(sub,ioOutput,nil);
        if (flags and (ACF_MESSAGE+ACF_POPUP+ACF_INSERT))<>0 then
        begin
          if (flags and ACF_INSERT )<>0 then AddAttrInt(act,ioMessage,1);
          if (flags and ACF_POPUP  )<>0 then AddAttrInt(act,ioPopup  ,1);
          if (flags and ACF_MESSAGE)<>0 then AddAttrInt(act,ioMsgBox ,1);
        end;

        if (flags2 and ACF2_FREEMEM)<>0 then AddAttrInt(sub,ioFree,1);

        if (flags and ACF_STRUCT)<>0 then
          s:=ioStruct
        else if (flags and ACF_STRING)<>0 then
        begin
          if (flags and ACF_UNICODE)<>0 then
            s:=ioUnicode
          else
            s:=ioAnsi;
        end
        else
        begin
          if      (flags and ACF_SIGNED)<>0 then s:=ioSigned
          else if (flags and ACF_HEX   )<>0 then s:=ioHex
          else                                   s:=ioInt;
        end;
        AddAttr(act,ioType,s);
      end;
// ----- PROGRAM -----
      ACT_PROGRAM: begin
        AddAttr(actnode,ioClass,ioRunProgram);
        setText(actnode,prgname);
        sub:=actnode;
//        sub:=AddChild(actnode,ioRunProgram,prgname);
        if args<>nil then
          AddAttr(sub,ioArgs,args);
        if (flags and ACF_CURPATH)<>0 then AddAttrInt(sub,ioCurrent,1);
        if (flags and ACF_PRTHREAD)=0 then AddAttrInt(sub,ioWait,time)
        else AddAttrInt(sub,ioParallel,1);

        if (flags2 and ACF2_PRG_PRG)<>0 then AddAttrInt(sub,ioFileVariable,1);
        if (flags2 and ACF2_PRG_ARG)<>0 then AddAttrInt(sub,ioArgVariable ,1);

        case show of
          SW_HIDE         : s:=ioHidden;
          SW_SHOWMINIMIZED: s:=ioMinimized;
          SW_SHOWMAXIMIZED: s:=ioMaximized;
        else
          s:=ioNormal;
        end;
        AddAttr(sub,ioWindow,s);
      end;
// ----- TEXT -----
      ACT_TEXT: begin
        if (flags and ACF_CLIPBRD)<>0 then
        begin
          AddAttr(actnode,ioClass,ioInsertText);
        end
        else
        begin
          if (flags and ACF_FILE)<>0 then
            AddAttr(actnode,ioClass,ioInsertText)
          else
          begin
            AddAttr(actnode,ioClass,'Text');
            setText(actnode,text);
          end;
        end;
        sub:=actnode;
//        sub:=AddChild(actnode,ioInsertText,tmp);
        if (flags and ACF_CLIPBRD)<>0 then
        begin
          AddAttr(sub,ioObject,ioClipboard);
          if (flags and ACF_COPYTO)<>0 then
            s:=ioCopy
          else
            s:=ioPaste;
          AddAttr(sub,ioOper,s);
          if (flags and ACF_ANSI)=0 then
            s:=ioUnicode
          else
            s:=ioAnsi;
          AddAttr(sub,ioEnc,s);
        end
        else
        begin
          if (flags and ACF_FILE)<>0 then
          begin
            if (flags2 and ACF2_TXT_FILE)<>0 then
              AddAttrInt(sub,ioFileVariable,1);
            AddAttr(sub,ioObject,ioFile);
            AddAttr(sub,ioFile,tfile);
            if      (flags and ACF_FWRITE )<>0 then AddAttr(sub,ioOper,ioWrite)
            else if (flags and ACF_FAPPEND)<>0 then AddAttr(sub,ioOper,ioAppend);

            if (flags and ACF_ANSI)<>0 then
              i:=0
            else if (flags and ACF_UTF8)<>0 then
            begin
              if (flags and ACF_SIGN)<>0 then
                i:=2
              else
                i:=1;
            end
            else if (flags and ACF_SIGN)<>0 then
              i:=4
            else
              i:=3;
            AddAttrInt(sub,ioEnc,i);
          end
          else
            AddAttr(sub,ioObject,ioWindow);

          if (flags2 and ACF2_TXT_TEXT)<>0 then
            AddAttrInt(sub,ioVariables,1);
        end;
      end;
// ----- ADVANCED -----
      ACT_ADVANCE: begin
        AddAttr(actnode,ioClass,ioAdvanced);
        sub:=actnode;
//        sub:=AddChild(actnode,ioAdvanced,nil);
        if lobyte(condition)<>0 then
        begin
          act:=AddChild(sub,ioIf,nil);
          if (lobyte(condition) and ADV_COND_NOT)<>0 then
            AddAttrInt(act,ioNot,1);
          case lobyte(condition) and not ADV_COND_NOT of
            ADV_COND_GT: s:='gt';
            ADV_COND_LT: s:='lt';
            ADV_COND_EQ: s:='eq';
          else
            s:=ioNop;
          end;
          AddAttr(act,ioOper,'math');
          AddAttr(act,ioCond,s);
          AddAttrInt(act,ioValue,value);
        end;

        if (action and not 3)<>ADV_ACT_VARS then
          tmp:=nil
        else
          tmp:=varval;
        act:=AddChild(sub,ioAct,tmp);
        case action and ADV_ACTION of
          ADV_ACT_MATH: begin
            AddAttr(act,ioType,ioValue);
            if tAdvExpr(oper)=aeNot then
              AddAttr(act,ioOper,ioInverse)
            else
            begin
              case tAdvExpr(oper) of
                aeAdd: s:='+';
                aeSub: s:='-';
                aeMul: s:='*';
                aeDiv: s:='\';
                aeMod: s:='%';
                aeAnd: s:='&';
                aeOr : s:='|';
                aeXor: s:='^';
                aeSet: s:='=';
              else
                s:=nil;
              end;
              AddAttr(act,ioOper,s);
              AddAttrInt(act,ioValue,mathval);
            end;
          end;
          ADV_ACT_VARS: begin
            if (flags and ACF_VARASINT)<>0 then
              AddAttrInt(act,ioAsInt,1);
            AddAttr(act,ioType,ioScript);
          end;
        end;

        if (action and ADV_ACT_POST)<>0 then
        begin
          act:=AddChild(sub,ioPost,nil);
          case action and ADV_ACT_POST of
            ADV_ACT_BREAK: s:=ioBreak;
            ADV_ACT_JUMP : begin
              s:=ioJump;
              AddAttr(act,ioValue,operval);
            end;
          else
            s:=ioNop
          end;
          AddAttr(act,ioOper,s);
        end;
        //!!
      end;
// ----- LINK -----
      ACT_CHAIN: begin
        if (flags and ACF_BYNAME)<>0 then
          s:=actname
        else
          s:=GetActNameById(id);
        AddAttr(actnode,ioClass,ioLinkAction);
        setText(actnode,s);
//        AddChild(actnode,ioLinkAction,s);
      end;
// ----- DATABASE -----
      ACT_RW: begin
        if ((flags and ACF_DBUTEXT)=0) and ((flags2 and ACF2_RW_TVAR)=0) then
          tmp:=nil
        else
          tmp:=pWideChar(dbvalue);
        AddAttr(actnode,ioClass,ioProfile);
        setText(actnode,tmp);
        sub:=actnode;
//        sub:=AddChild(actnode,ioProfile,tmp);
        if      (flags and ACF_DBDELETE)<>0 then s:=ioDelete
        else if (flags and ACF_DBWRITE )<>0 then s:=ioWrite
        else                                     s:=ioRead;
        AddAttr(sub,ioOper,s);
        if      (flags and ACF_CURRENT)<>0 then s:=ioCurrent
        else if (flags and ACF_RESULT )<>0 then s:=ioResult
        else if (flags and ACF_PARAM  )<>0 then s:=ioParam
        else
        begin
          s:=ioContact;
          ExportContact(sub,dbcontact);
        end;
        AddAttr(sub,ioContact,s);
        AddAttr(sub,ioModule ,FastAnsiToWide(dbmodule ,tmp)); mFreeMem(tmp);
        AddAttr(sub,ioSetting,FastAnsiToWide(dbsetting,tmp)); mFreeMem(tmp);

        if      (flags and ACF_DBANSI )=ACF_DBANSI then s:=ioAnsi
        else if (flags and ACF_DBBYTE )=ACF_DBBYTE then s:=ioByte
        else if (flags and ACF_DBWORD )=ACF_DBWORD then s:=ioWord
        else if (flags and ACF_DBUTEXT)= 0 then s:=ioDword
        else s:=ioUnicode;
        AddAttr(sub,ioType,s);

        if (flags2 and ACF2_RW_MVAR)<>0 then AddAttrInt(sub,ioFileVariable,1);
        if (flags2 and ACF2_RW_SVAR)<>0 then AddAttrInt(sub,ioArgVariable ,1);
        if (flags2 and ACF2_RW_TVAR)<>0 then AddAttrInt(sub,ioVariables   ,1);
        if (flags2 and ACF2_RW_HEX )<>0 then AddAttrInt(sub,ioHex         ,1);

        if      ( flags  and ACF_LAST    )<>0 then AddAttrInt(sub,ioLast ,1)
        else if ((flags  and ACF_DBUTEXT )=0) and
                ((flags2 and ACF2_RW_TVAR)=0) then AddAttrInt(sub,ioValue,dbvalue);
      end;
// ----- MESSAGEBOX -----
      ACT_MESSAGE: begin
        AddAttr(actnode,ioClass,ioMessageBox);
        setText(actnode,msgtext);
        sub:=actnode;
//        sub:=AddChild(actnode,ioMessageBox,msgtext);
        if (flags2 and ACF2_MSG_TTL)<>0 then AddAttrInt(sub,ioVariables  ,1);
        if (flags2 and ACF2_MSG_TXT)<>0 then AddAttrInt(sub,ioArgVariable,1);
        if (flags  and ACF_MSG_KEEP)<>0 then AddAttrInt(sub,ioKeepLast   ,1);
        AddAttr   (sub,ioTitle,msgtitle);
        AddAttrInt(sub,ioType ,boxopts);
      end;
    end;
  end;
end;

procedure Export({act:integer;}fname:pWideChar;aflags:dword);
var
  i:integer;
  f:THANDLE;
  root,actnode:HXML;
  res:pWideChar;
  act:integer;
begin
  xmlparser.cbSize:={XML_API_SIZEOF_V1;//}SizeOf(TXML_API_W);
  CallService(MS_SYSTEM_GET_XI,0,lparam(@xmlparser));
  root:=0;
  with xmlparser do
  begin
    i:=0;
    if (aflags and ACIO_APPEND)<>0 then
    begin
      i:=GetFSize(fname);
      if i<>0 then
      begin
        mGetMem (res ,i+SizeOf(WideChar));
        FillChar(res^,i+SizeOf(WideChar),0);
        f:=Reset(fname);
        BlockRead(f,res^,i);
        CloseHandle(f);
        root:=parseString(res,@i,nil);
        mFreeMem(res);
        i:=1;
      end;
    end;
    if i=0 then // new file
      root:=CreateNode(ioRoot,nil,false);

    for act:=0 to MaxGroups-1 do 
      if ((aflags and ACIO_SELECTED)=0) or
         ((GroupList[act].flags and (ACF_EXPORT or ACF_ASSIGNED))=
                                    (ACF_EXPORT or ACF_ASSIGNED)) then
      begin
//        GroupList[act].flags:=GroupList[act].flags and not ACF_EXPORT;
        actnode:=addChild(root,ioAction,nil);
        AddAttr(actnode,ioName,GroupList[act].descr);
        if (GroupList[act].flags and ACF_DISABLED)<>0 then
          AddAttrInt(actnode,ioDisabled,1);

        i:=GroupList[act].firstAction;
        if i<>0 then
          repeat
            WriteAction(actnode,i);
            i:=ActionList[i].next;
          until i=0;
      end;

    res:=toString(root,@i);
    if i>0 then
    begin
      f:=Rewrite(fname);
      BlockWrite(f,res^,i*SizeOf(WideChar));
      CloseHandle(f);
    end;
    xmlparser.FreeMem(res);
    DestroyNode(root);
  end;
end;

function ActInOut(wParam:WPARAM;lParam:LPARAM):int_ptr; cdecl;
begin
  if (wParam and ACIO_EXPORT)=0 then
  begin
    result:=Import(pWideChar(lParam),wParam);
  end
  else
  begin
    result:=1;
    Export(pWideChar(lParam),wParam);
  end;
  NotifyEventHooks(hevinout,wParam,lParam);
end;
