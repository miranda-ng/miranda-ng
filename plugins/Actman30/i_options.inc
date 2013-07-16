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

//----- Load settings -----

function LoadActions(Macro:pMacroRecord;section:pAnsiChar):integer;
var
  p,p1:PAnsiChar;
  i,num:integer;
  actm:pActModule;
  action:tBaseAction;
  uid:dword;
  tmp:pActionList;
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
  i:cardinal;
  p,p1:PAnsiChar;
  section:array [0..127] of AnsiChar;
  NumMacros:cardinal;
  tmp:pWideChar;
begin
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
          LoadActions(Macro,section);
        end;
      end;
      inc(Macro);
      inc(i);
    end;
  end;
end;
