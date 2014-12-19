unit inoutxml;

interface

uses windows, lowlevelc;

function Import(list:tMacroList;fname:PWideChar;aflags:dword):integer;

implementation

uses
  io, common, m_api, question,
  iac_global, global;

const
  ioAction = 'Action';
  ioClass  = 'class';
  ioName   = 'name';
  ioVolatile = 'volatile';
const
  imp_yes    = 1;
  imp_yesall = 2;
  imp_no     = 3;
  imp_noall  = 4;
  imp_append = 5;

function ImportAction(actnode:HXML):tBaseAction;
var
  pa:pActModule;
  buf:array [0..127] of AnsiChar;
begin
  result:=nil;
  if actnode=0 then exit;
  with xmlparser do
  begin
    pa:=GetLinkByName(FastWideToAnsiBuf(getAttrValue(actnode,ioClass),buf));
    if pa<>nil then
    begin
      result:=pa.Create;
      result.Load(pointer(actnode),1);
    end
    else
      result:=tBaseAction(1);
  end;
end;

function Import(list:tMacroList;fname:PWideChar;aflags:dword):integer;
var
  f:THANDLE;
  i,nodenum,actcnt:integer;
  tmp,res:pWideChar;
  root,actnode:HXML;
  impact:integer;
  buf:array [0..511] of WideChar;
  oldid:dword;
  arr:array [0..63] of tBaseAction;
  act:tBaseAction;
  p:pMacroRecord;
begin
  result:=0;

  for i:=0 to list.Count-1 do
    with list[i]^ do
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
  xmlparser.cbSize:=SizeOf(TXML_API_W);
  CallService(MS_SYSTEM_GET_XI,0,lparam(@xmlparser));
  with xmlparser do
  begin
    root:=parseString(ChangeUnicode(res),@i,nil);
    nodenum:=0;
    impact:=imp_yes;
    repeat
      actnode:=getNthChild(root,ioAction,nodenum);
      if actnode=0 then break;
//??      if StrCmpW(getName(actnode),ioAction)<>0 then break;
      tmp:=getAttrValue(actnode,ioName);
      if tmp<>nil then //!!
      begin
        p:=list.GetMacro(tmp);
        oldid:=$FFFFFFFF;
        if p<>nil then
        begin
          if (impact<>imp_yesall) and (impact<>imp_noall) then
          begin
            StrCopyW(buf,TranslateW('Action "$" exists, do you want to rewrite it?'));
            impact:=ShowQuestion(StrReplaceW(buf,'$',tmp));
          end;
          if (impact=imp_yesall) or (impact=imp_yes) then
          begin
            oldid:=p^.id;
            FreeMacro(p);
          end;
        end;
        // if new or overwriting then read macro details/actions
        if (p=nil) or (impact=imp_yesall) or (impact=imp_yes) or (impact=imp_append) then
        begin
          with List[list.NewMacro()]^ do
          begin
            if (p<>nil) and (oldid<>$FFFFFFFF) then // set old id to keep UseAction setting
            begin
              flags:=flags or ACF_IMPORT or ACF_OVERLOAD;
              id:=oldid;
            end
            else
              flags:=flags or ACF_IMPORT;
            if StrToInt(getAttrValue(actnode,ioVolatile))=1 then flags:=flags or ACF_VOLATILE;
            StrCopyW(descr,tmp,MacroNameLen-1);

            // reading actions
            actcnt:=0; // count in file 
            ActionCount:=0;      // amount of loaded
            repeat
              act:=ImportAction(getChild(actnode,actcnt));
              if act=nil then
                break;
              if uint_ptr(act)<>1 then
              begin
                arr[ActionCount]:=act;
                inc(ActionCount);
              end;
              inc(actcnt);
            until false;
            // moving actions to their place
            if Actioncount>0 then
            begin
              GetMem(ActionList,SizeOf(tBaseAction)*ActionCount);
              move(arr,ActionList^,SizeOf(tBaseAction)*ActionCount);
            end;
            inc(result);
          end;
        end;
      end;
      inc(nodenum);
    until false;
    destroyNode(root);
  end;
  mFreeMem(res);
end;

end.
