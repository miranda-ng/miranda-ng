unit inoutini;

interface

uses windows, lowlevelc;

{
type
  pINIioStruct = ^tINIioStruct;
  tINIioStruct = record
    storage,
    section,
    namespace:pAnsiChar;
  end;
}
function Import(list:tMacroList;fname:PWideChar;aflags:dword):integer;

implementation

uses
  io, common, m_api, question,
  iac_global, global, memini;

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

function ImportAction(section:pointer):tBaseAction;
var
  pa:pActModule;
begin
  if section<>nil then
  begin
    pa:=GetLinkByName(GetParamSectionStr(section,ioClass,nil));
    if pa<>nil then
    begin
      result:=pa.Create;
      result.Load(section,2);
    end
    else
      result:=tBaseAction(1);
  end
  else
    result:=nil;
end;

function Import(list:tMacroList;fname:PWideChar;aflags:dword):integer;
var
  i:integer;
  impact:integer;
  buf:array [0..511] of WideChar;
  oldid:dword;
  arr:array [0..63] of tBaseAction;
  act:tBaseAction;
  p:pMacroRecord;
  storage:pointer;
  pc,pc1,ppc,ppc1:pAnsiChar;
  pcw:pWideChar;
begin
  result:=0;

  for i:=0 to list.Count-1 do
    with list[i]^ do
      if (flags and (ACF_IMPORT or ACF_ASSIGNED))=
                    (ACF_IMPORT or ACF_ASSIGNED) then
        flags:=flags and not (ACF_IMPORT or ACF_OVERLOAD);

  if (fname=nil) or (fname^=#0) then
    exit;
  storage:=OpenStorage(fname);
  if storage=nil then
    exit;

  impact:=imp_yes;

  pc:=GetSectionList(storage,''); // list with "empty" namespaces
  ppc:=pc;
  while pc^<>#0 do
  begin
    UTF8ToWide(pc,pcw);

    p:=list.GetMacro(pcw);
    oldid:=$FFFFFFFF;
    if p<>nil then
    begin
      if (impact<>imp_yesall) and (impact<>imp_noall) then
      begin
        StrCopyW(buf,TranslateW('Action "$" exists, do you want to rewrite it?'));
        impact:=ShowQuestion(StrReplaceW(buf,'$',pcw));
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

        if GetParamInt(storage,pc,ioVolatile)=1 then flags:=flags or ACF_VOLATILE;
        StrCopyW(descr,pcw,MacroNameLen-1);

        // reading actions
        ActionCount:=0;      // amount of loaded
        pc1:=GetSectionList(storage,pc);
        ppc1:=pc1;
        while pc1^<>#0 do
        begin
          act:=ImportAction(SearchSection(storage,pc1,pc));
          if act=nil then
            break;
          if uint_ptr(act)<>1 then
          begin
            arr[ActionCount]:=act;
            inc(ActionCount);
          end;
        end;
        FreeSectionList(ppc1);
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
  FreeSectionList(ppc);

  CloseStorage(storage);
end;

end.
