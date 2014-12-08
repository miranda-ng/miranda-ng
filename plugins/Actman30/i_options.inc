{Save/load options}

const
  opt_group    = 'Group';
  opt_actions  = 'Action';
  opt_numacts  = 'numactions';
  opt_nummacro = 'numgroups';

  opt_descr    = 'descr';
  opt_id       = 'id';
  opt_uid      = 'uid';
  opt_flags    = 'flags';

//----- Save settings -----

procedure SaveActions(Macro:pMacroRecord;section:pAnsiChar);
var
  p,p1:PAnsiChar;
  i:integer;
begin
  p:=StrEnd(section);
  StrCopy(p,opt_numacts); DBWriteWord(0,DBBranch,section,Macro^.ActionCount);

  // in: section = "Group#/"
  p1:=StrCopyE(p,opt_actions); // "Group#/Action"
  DBDeleteGroup(0,DBBranch,section);

  for i:=0 to Macro^.ActionCount-1 do
  begin
    p:=StrEnd(IntToStr(p1,i));
    p^:='/'; inc(p); // "Group#/Action#/"

//??    StrCopy(p,opt_uid); DBWriteDWord(0,DBBranch,section,Macro^.ActionList[i].uid);
    p^:=#0;
    Macro^.ActionList[i].Save(section,0);
  end;

end;

procedure SaveMacros;
var
  Macro:pMacroRecord;
  NumMacro:integer;
  i:integer;
  section:array [0..127] of AnsiChar;
  p,p1:PAnsiChar;
begin
// even if crap in settings, skip on read
//  DBDeleteGroup(0,DBBranch,opt_group);
  Macro:=MacroList[0];
  i:=MacroList.Count;
  NumMacro:=0;

  p1:=StrCopyE(section,opt_group);
  while i>0 do
  begin
    with Macro^ do
    begin
      if (flags and (ACF_ASSIGNED or ACF_VOLATILE))=ACF_ASSIGNED then
      begin
        p:=StrEnd(IntToStr(p1,NumMacro));
        p^:='/'; inc(p);

        StrCopy(p,opt_id   ); DBWriteDWord(0,DBBranch,section,id);
        StrCopy(p,opt_flags); DBWriteDWord(0,DBBranch,section,flags);
        StrCopy(p,opt_descr); DBWriteUnicode (0,DBBranch,section,descr);

        p^:=#0;

        SaveActions(Macro,section);

        inc(NumMacro);
      end;
    end;
    inc(Macro);
    dec(i);
  end;
  DBWriteWord(0,DBBranch,opt_nummacro,NumMacro);
end;

//===== Load settings =====

//----- V2 settings processing -----

const
  ACT_CONTACT = 1;
  ACT_SERVICE = 2;
  ACT_PROGRAM = 3;
  ACT_TEXT    = 4;
  ACT_ADVANCE = 5;
  ACT_CHAIN   = 6;
  ACT_RW      = 7;
  ACT_MESSAGE = 8;

// action flags
const
  ACF_MASK         = $00FFFFFF;

  ACF_OLD_CLIPBRD  = $00000002; // Clipboard operations, not window
  ACF_OLD_FILE     = $00000010; // File operations
  ACF_OLD_FWRITE   = $00000020; // read/write file
  ACF_OLD_FAPPEND  = $00000040; // append file

procedure CheckActionList(Macro:pMacroRecord;num:integer);
var
  tmplist:pActionList;
begin
  if num=Macro^.ActionCount then
  begin
    GetMem(tmplist,SizeOf(tBaseAction)*(num+1));
    move(Macro^.ActionList^,tmplist^,Macro^.ActionCount*SizeOf(tBaseAction));
    FreeMem(Macro^.ActionList);
    Macro^.ActionList:=tmplist;
    Inc(Macro^.ActionCount);
  end;
end;

function LoadActionsV2(Macro:pMacroRecord;section:pAnsiChar):boolean; stdcall;
var
  buf:array [0..31] of WideChar;
  p,p1,pc:PAnsiChar;
  action:tBaseAction;
  actm:pActModule;
  flags2,actionType:dword;
  code,i,num,count:integer;
  cond,act:byte;
begin
  result:=false;
  p:=StrEnd(section);
  StrCopy(p,opt_numacts); Macro^.ActionCount:=DBReadWord(0,DBBranch,section);

  if Macro^.ActionCount>0 then
  begin
    GetMem(Macro^.ActionList,SizeOf(tBaseAction)*Macro^.ActionCount);
    p1:=StrCopyE(p,opt_actions); // "Group#/Action"

    num:=0;
    count:=Macro^.ActionCount;
    for i:=1 to count do
    begin
      p:=StrEnd(IntToStr(p1,i));
      p^:='/'; inc(p); // "Group#/Action#/"

      StrCopy(p,'type'  ); actionType:=DBReadByte (0,DBBranch,section,ACT_CONTACT);
      case actionType of
        ACT_CONTACT: pc:='Contact';
        ACT_SERVICE: pc:='Service';
        ACT_PROGRAM: pc:='Program';
        ACT_TEXT:    pc:=nil;
        ACT_ADVANCE: pc:=nil;
        ACT_CHAIN:   pc:='Chain';
        ACT_RW:      pc:='Database';
        ACT_MESSAGE: pc:='MessageBox';
      else
        pc:=nil;
      end;
      if pc<>nil then
      begin
        actm  :=GetLinkByName(pc);
        action:=actm.Create;
      end
      else
      begin
        action:=nil;
      end;

      p^:=#0;
      case actionType of
        ACT_CONTACT: begin
          action.Load(section,0);
        end;

        ACT_SERVICE: begin
          action.Load(section,100);
        end;

        ACT_PROGRAM: begin
          action.Load(section,100);
        end;

        ACT_TEXT: begin
          StrCopy(p,opt_flags); flags2:=DBReadDWord(0,DBBranch,section,0);
          p^:=#0;
          if ((flags2 and ACF_OLD_CLIPBRD)=0) then
          begin
            if ((flags2 and ACF_OLD_FILE)=0) or ((flags2 and (ACF_OLD_FWRITE or ACF_OLD_FAPPEND)<>0)) then
            begin
              actm  :=GetLinkByName('Text');
              action:=actm.Create;

              action.Load(section,100);

              CheckActionList(Macro,num);
              Macro^.ActionList^[num]:=action;
              inc(num);
            end;
          end;

          actm  :=GetLinkByName('In/Out');
          action:=actm.Create;

          action.Load(section,100);
        end;

        ACT_ADVANCE: begin
          StrCopy(p,'condition'  ); cond:=DBReadByte(0,DBBranch,section);
          StrCopy(p,'action'); act :=DBReadByte(0,DBBranch,section);
          p^:=#0;
          // get code of my list
          code:=0;
          if (cond and $0F)=0 then
          begin
            if  act<>0         then code:=4; // action and jump
            if (act and $F0)=0 then code:=6; // no action
            if (act and $0F)=0 then code:=5; // no jump
          end
          else // with condition
          begin
            if  act<>0         then code:=1; // action and jump
            if (act and $F0)=0 then code:=2; // no action
            if (act and $0F)=0 then code:=3; // no jump
          end;
          case code of
            1: begin // conditional action and jump
              actm  :=GetLinkByName('Jump'); // reversed condition jump to tmplabel
              action:=actm.Create;
              action.Load(section,102+i);
              CheckActionList(Macro,num);
              Macro^.ActionList^[num]:=action;
              inc(num);

              actm  :=GetLinkByName('Text'); // action
              action:=actm.Create;
              action.Load(section,101);
              CheckActionList(Macro,num);
              Macro^.ActionList^[num]:=action;
              inc(num);

              actm  :=GetLinkByName('Jump'); // jump to label (no condition)
              action:=actm.Create;
              action.Load(section,101);
              CheckActionList(Macro,num);
              Macro^.ActionList^[num]:=action;
              inc(num);

              actm  :=GetLinkByName('Notes'); // tmplabel
              action:=actm.Create;
              mFreeMem(action.ActionDescr); // created by constructor
              buf[0]:='$'; buf[1]:='$';
              IntToStr(PWideChar(@buf[2]),i);
              StrDupW(action.ActionDescr,buf);
            end;
            2,6: begin // conditional and unconditional jump
              actm  :=GetLinkByName('Jump');
              action:=actm.Create;

              action.Load(section,100);
            end;
            3: begin // conditional action
              actm  :=GetLinkByName('Jump');
              action:=actm.Create;
              action.Load(section,101+num);
              CheckActionList(Macro,num);
              Macro^.ActionList^[num]:=action;
              inc(num);

              actm  :=GetLinkByName('Text');
              action:=actm.Create;
              action.Load(section,101);
              CheckActionList(Macro,num);
              Macro^.ActionList^[num]:=action;
              inc(num);

              actm  :=GetLinkByName('Notes');
              action:=actm.Create;
              mFreeMem(action.ActionDescr); // created by constructor
              buf[0]:='$'; buf[1]:='$';
              StrDupW(action.ActionDescr,IntToStr(PWideChar(@buf[2]),num));
            end;
            4: begin // action + unconditional jump
              actm  :=GetLinkByName('Text');
              action:=actm.Create;
              action.Load(section,101);
              CheckActionList(Macro,num);
              Macro^.ActionList^[num]:=action;
              inc(num);

              actm  :=GetLinkByName('Jump');
              action:=actm.Create;

              action.Load(section,100);
            end;
            5: begin // direct text action
              actm  :=GetLinkByName('Text');
              action:=actm.Create;

              action.Load(section,101);
            end;
          end;
        end;

        ACT_CHAIN: begin
          action.Load(section,100);
        end;

        ACT_RW: begin
          action.Load(section,100);
        end;

        ACT_MESSAGE: begin
          action.Load(section,0);

          StrCopy(p,'flags2'); flags2:=DBReadDWord(0,DBBranch,section,0);
          action.flags:=(action.flags and not ACF_MASK) or flags2;
        end;

      end;

      if action<>nil then
      begin
        CheckActionList(Macro,num);
        Macro^.ActionList^[num]:=action;
        inc(num);
      end;
    end;
  end;
end;

//----- V3 settings processing -----

function LoadActions(Macro:pMacroRecord;section:pAnsiChar):integer; stdcall;
var
  p,p1:PAnsiChar;
  actm:pActModule;
  action:tBaseAction;
  tmp:pActionList;
  uid:dword;
  i,num:integer;
begin
  result:=0;
  p:=StrEnd(section);
  StrCopy(p,opt_numacts); Macro^.ActionCount:=DBReadWord(0,DBBranch,section);
  if Macro^.ActionCount>0 then
  begin
    GetMem(Macro^.ActionList,SizeOf(tBaseAction)*Macro^.ActionCount);
    p1:=StrCopyE(p,opt_actions); // "Group#/Action"

    num:=0;
    for i:=0 to Macro^.ActionCount-1 do
    begin
      p:=StrEnd(IntToStr(p1,i));
      p^:='/'; inc(p); // "Group#/Action#/"

      // get uid
      StrCopy(p,opt_uid); uid:=DBReadDWord(0,DBBranch,section,0);
      if uid<>0 then
      begin
        p^:=#0;
        // call proper constructor
        actm:=GetLink(uid);
        if actm=nil then
          continue;

        action:=actm.Create;
        // call proper loader
        action.Load(section,0);

        Macro^.ActionList^[num]:=action;
        inc(num);
      end;
    end;
    if Macro^.ActionCount<>num then
    begin
      GetMem(tmp,SizeOf(tBaseAction)*num);
      move(Macro^.ActionList^,tmp^,SizeOf(tBaseAction)*num);
      Macro^.ActionCount:=num;
      FreeMem(Macro^.ActionList);
      Macro^.ActionList:=tmp;
    end;
  end;
end;

procedure LoadMacros;
var
  Macro:pMacroRecord;
  p,p1:PAnsiChar;
  section:array [0..127] of AnsiChar;
  tmp:pWideChar;
  i:cardinal;
  NumMacros:cardinal;
  v2:bool;
begin
  //!! Check if old actman version used
//  v2:=DBReadWord(0,DBBranch,opt_numacts,$FFFF)<>$FFFF;
  // V2 counts actions from 1, not 0
  v2:=DBReadDWord(0,DBBranch,'Group0/Action0/flags',$FFFFFFFF)=$FFFFFFFF;

  // Allocate macro list
  NumMacros :=DBReadWord(0,DBBranch,opt_nummacro,0);
  MacroList:=tMacroList.Create(NumMacros);

  // read macro list settings
  if NumMacros>0 then //?? really, not so necessary
  begin
    Macro:=MacroList[0];
    i:=0;
    p1:=StrCopyE(section,opt_group);
    while i<NumMacros do
    begin
      p:=StrEnd(IntToStr(p1,i));
      p^:='/'; inc(p);

      StrCopy(p,opt_flags);
      with Macro^ do
      begin
        flags:=DBReadDWord(0,DBBranch,section,0) and ACF_TOSAVE;
        if (flags and ACF_ASSIGNED)<>0 then //?? not needed in normal cases
        begin
          StrCopy(p,opt_id   ); id :=DBReadDWord  (0,DBBranch,section);
          StrCopy(p,opt_descr); tmp:=DBReadUnicode(0,DBBranch,section,NoDescription);
          StrCopyW(descr,tmp,MacroNameLen-1);
          mFreeMem(tmp);
          p^:=#0;
          if v2 then
            LoadActionsV2(Macro,section)
          else
            LoadActions(Macro,section);
        end;
      end;
      inc(Macro);
      inc(i);
    end;
  end;
end;
