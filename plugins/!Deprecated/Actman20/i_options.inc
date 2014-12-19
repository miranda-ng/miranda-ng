{Save/load options}

const
  opt_group   = 'Group';
  opt_actions = 'Action';
  opt_numacts = 'numactions';
  opt_numhk   = 'numgroups';
  opt_firstAction = 'firstaction';

  opt_cproto   = 'cproto';
  opt_cuid     = 'cuid';
  opt_ischat   = 'ischat';

  opt_descr    = 'descr';
  opt_id       = 'id';
  opt_flags    = 'flags';
  opt_flags2   = 'flags2';
  opt_time     = 'time';
  opt_show     = 'show';
  opt_action   = 'action';
  opt_value    = 'value';
  opt_file     = 'file';

  opt_next     = 'next';
  opt_type     = 'type';
  opt_contact  = 'contact';
  opt_text     = 'text';
  opt_prg      = 'program';
  opt_args     = 'arguments';
  opt_service  = 'service';
  opt_wparam   = 'wparam';
  opt_wlparam  = 'wparamlen';
  opt_llparam  = 'lparamlen';
  opt_lparam   = 'lparam';
  opt_chain    = 'chain';
  opt_cond     = 'condition';
  opt_count    = 'count';
  opt_module   = 'module';
  opt_setting  = 'setting';
  opt_oper     = 'operation';
  opt_mathval  = 'mathval';
  opt_operval  = 'operval';
  opt_varval   = 'varval';
  opt_msgtitle = 'msgtitle';
  opt_msgtext  = 'msgtext';
  opt_boxopts  = 'boxopts';


//----- Save settings -----

procedure SaveNumValue(setting:pAnsiChar;value:uint_ptr;isvar:boolean);
begin
  if isvar then
    DBWriteUnicode(0,DBBranch,setting,pWideChar(value))
  else
    DBWriteDWord  (0,DBBranch,setting,value);
end;

function SaveActions(section:pAnsiChar;first:integer):integer;
var
  p,p1:PAnsiChar;
  act:pHKAction;
  i:integer;
begin
  result:=0;
  // in: section = "Group#/"
  p1:=StrCopyE(StrEnd(section),opt_actions); // "Group#/Action"
  DBDeleteGroup(0,DBBranch,section);
  i:=1;
  while first<>0 do
  begin
    act:=@ActionList[first];
    p:=StrEnd(IntToStr(p1,i)); //!!!
    p^:='/'; inc(p); // "Group#/Action#/"

    StrCopy(p,opt_flags ); DBWriteDWord(0,DBBranch,section,act^.flags);
    StrCopy(p,opt_flags2); DBWriteDWord(0,DBBranch,section,act^.flags2);
    StrCopy(p,opt_type  ); DBWriteByte (0,DBBranch,section,act^.actionType);
    if act^.descr<>nil then
    begin
      StrCopy(p,opt_descr); DBWriteUnicode(0,DBBranch,section,act^.descr);
    end;

    case act^.actionType of
      ACT_CONTACT: begin
        p^:=#0;
        SaveContact(act^.contact,DBBranch,section);
      end;

      ACT_SERVICE: begin
        StrCopy(p,opt_service); DBWriteString(0,DBBranch,section,act^.service);

        if (act^.flags and (ACF_WCURRENT or ACF_WRESULT or ACF_WPARAM))=0 then
        begin
          StrCopy(p,opt_wparam);
          if (act^.flags and ACF_WPARNUM)<>0 then
            SaveNumValue(section,act^.wparam,(act^.flags2 and ACF2_SRV_WPAR)<>0)
//            DBWriteDWord(0,DBBranch,section,act^.wparam)
          else if act^.wparam<>0 then
          begin
            if (act^.flags and ACF_WSTRUCT)<>0 then
              DBWriteUTF8(0,DBBranch,section,pAnsiChar(act^.wparam))
            else if (act^.flags and ACF_WUNICODE)<>0 then
              DBWriteUnicode(0,DBBranch,section,pWideChar(act^.wparam))
            else
              DBWriteString(0,DBBranch,section,PAnsiChar(act^.wparam));
          end;
        end;

        if (act^.flags and (ACF_LCURRENT or ACF_LRESULT or ACF_LPARAM))=0 then
        begin
          StrCopy(p,opt_lparam);
          if (act^.flags and ACF_LPARNUM)<>0 then
            SaveNumValue(section,act^.lparam,(act^.flags2 and ACF2_SRV_LPAR)<>0)
//            DBWriteDWord(0,DBBranch,section,act^.lparam)
          else if act^.lparam<>0 then
          begin
            if (act^.flags and ACF_LSTRUCT)<>0 then
              DBWriteUTF8(0,DBBranch,section,pAnsiChar(act^.lparam))
            else if (act^.flags and ACF_LUNICODE)<>0 then
              DBWriteUnicode(0,DBBranch,section,pWideChar(act^.lparam))
            else
              DBWriteString(0,DBBranch,section,PAnsiChar(act^.lparam));
          end;
        end;

      end;

      ACT_PROGRAM: begin
        StrCopy(p,opt_prg ); DBWriteUnicode(0,DBBranch,section,act^.prgname);
        StrCopy(p,opt_args); DBWriteUnicode(0,DBBranch,section,act^.args);
        StrCopy(p,opt_time); DBWriteDWord  (0,DBBranch,section,act^.time);
        StrCopy(p,opt_show); DBWriteDWord  (0,DBBranch,section,act^.show);
      end;

      ACT_TEXT: begin
        if (act^.flags and ACF_CLIPBRD)=0 then
        begin
          StrCopy(p,opt_text); DBWriteUnicode(0,DBBranch,section,act^.text);
          if (act^.flags and ACF_FILE)<>0 then
          begin
            StrCopy(p,opt_file); DBWriteUnicode(0,DBBranch,section,act^.tfile);
          end;
        end;
      end;

      ACT_ADVANCE: begin
        StrCopy(p,opt_cond   ); DBWriteByte   (0,DBBranch,section,act^.condition);
        StrCopy(p,opt_value  ); DBWriteDWord  (0,DBBranch,section,act^.value);
        StrCopy(p,opt_action ); DBWriteByte   (0,DBBranch,section,act^.action);
        StrCopy(p,opt_operval); DBWriteUnicode(0,DBBranch,section,act^.operval);
        StrCopy(p,opt_oper   ); DBWriteByte   (0,DBBranch,section,act^.oper);
        StrCopy(p,opt_mathval); DBWriteDWord  (0,DBBranch,section,act^.mathval);
        StrCopy(p,opt_varval ); DBWriteUnicode(0,DBBranch,section,act^.varval);
      end;

      ACT_CHAIN: begin
        StrCopy(p,opt_text); DBWriteDWord(0,DBBranch,section,act^.id);
      end;

      ACT_RW: begin
        if (act^.flags and ACF_NOCNTCT)=0 then
        begin
          p^:=#0;
          SaveContact(act^.dbcontact,DBBranch,section);
        end;
        StrCopy(p,opt_module ); DBWriteString(0,DBBranch,section,act^.dbmodule);
        StrCopy(p,opt_setting); DBWriteString(0,DBBranch,section,act^.dbsetting);
        StrCopy(p,opt_value  );
        if (act^.flags and ACF_DBUTEXT)=0 then
        begin
          SaveNumValue(section,act^.dbvalue,(act^.flags2 and ACF2_RW_TVAR)<>0);
//          DBWriteDWord(0,DBBranch,section,act^.dbvalue)
        end
        else
          DBWriteUnicode(0,DBBranch,section,pWideChar(act^.dbvalue));
      end;

      ACT_MESSAGE: begin
        StrCopy(p,opt_msgtitle); DBWriteUnicode(0,DBBranch,section,act^.msgtitle);
        StrCopy(p,opt_msgtext ); DBWriteUnicode(0,DBBranch,section,act^.msgtext);
        StrCopy(p,opt_boxopts ); DBWriteByte   (0,DBBranch,section,act^.boxopts); //!!
      end;

    end;
    inc(result);
    inc(i);
    first:=ActionList^[first].next;
  end;
end;

procedure SaveGroups;
var
  HK:pHKRecord;
  NumHK:integer;
  i,num:integer;
  section:array [0..127] of AnsiChar;
  p,p1:PAnsiChar;
  Actions:integer;
begin
// even if crap in settings, skip on read
//  DBDeleteGroup(0,DBBranch,opt_group);
  HK:=@GroupList^;
  i:=MaxGroups;
  NumHK:=0;
  Actions:=1;
  DBWriteUnicode(0,DBBranch,'CLformat',fCLformat);
  DBWriteByte   (0,DBBranch,'CLfilter',ord(fCLfilter));

  p1:=StrCopyE(section,opt_group);
  while i>0 do
  begin
    with HK^ do
    begin
      if (flags and (ACF_ASSIGNED or ACF_VOLATILE))=ACF_ASSIGNED then
      begin
        p:=StrEnd(IntToStr(p1,NumHK));
        p^:='/'; inc(p);

        StrCopy(p,opt_id   ); DBWriteDWord(0,DBBranch,section,id);
        StrCopy(p,opt_flags); DBWriteDWord(0,DBBranch,section,flags);
        StrCopy(p,opt_descr);
        if descr<>nil then
          DBWriteUnicode (0,DBBranch,section,descr)
        else
          DBDeleteSetting(0,DBBranch,section);

        p^:=#0;
    //??
        num:=SaveActions(section,firstAction);
        StrCopy(p,opt_numacts); DBWriteWord(0,DBBranch,section,num);

        inc(Actions,num);
        inc(NumHK);
      end;
    end;
    inc(HK);
    dec(i);
  end;
  DBWriteWord(0,DBBranch,opt_numhk  ,NumHK);
  DBWriteWord(0,DBBranch,opt_numacts,Actions-1);
end;

//----- Load settings -----

function LoadNumValue(setting:pAnsiChar;isvar:boolean):uint_ptr;
begin
  if isvar then
    result:=uint_ptr(DBReadUnicode(0,DBBranch,setting,nil))
  else
    result:=DBReadDWord(0,DBBranch,setting);
end;

function LoadActions(section:pAnsiChar;count:integer):integer;
var
  p,p1:PAnsiChar;
  act:tHKAction;
  i,num,oldnum:integer;
begin
  result:=0;
  p1:=StrCopyE(StrEnd(section),opt_actions); // "Group#/Action"

  oldnum:=0;
  for i:=1 to count do
  begin
    p:=StrEnd(IntToStr(p1,i));
    p^:='/'; inc(p); // "Group#/Action#/"
    FillChar(act,SizeOf(act),0);

    StrCopy(p,opt_flags ); act.flags :=DBReadDWord  (0,DBBranch,section,0);
    if (act.flags and ACF_ASSIGNED)<>0 then
    begin
      StrCopy(p,opt_flags2); act.flags2    :=DBReadDWord  (0,DBBranch,section,0);
      StrCopy(p,opt_descr ); act.descr     :=DBReadUnicode(0,DBBranch,section,nil);
      StrCopy(p,opt_type  ); act.actionType:=DBReadByte   (0,DBBranch,section,ACT_CONTACT);

      case act.actionType of
        ACT_CONTACT: begin
          p^:=#0;
          act.contact:=LoadContact(DBBranch,section);
        end;

        ACT_SERVICE: begin
          StrCopy(p,opt_service);
          act.service:=DBReadString(0,DBBranch,section,nil);

          if (act.flags and (ACF_WCURRENT or ACF_WRESULT or ACF_WPARAM))=0 then
          begin
            StrCopy(p,opt_wparam);
            if (act.flags and ACF_WPARNUM)<>0 then
              act.wparam:=LoadNumValue(section,(act.flags2 and ACF2_SRV_WPAR)<>0)
            else if (act.flags and ACF_WSTRUCT)<>0 then
              act.wparam:=wparam(DBReadUTF8(0,DBBranch,section,nil))
            else if (act.flags and ACF_WUNICODE)<>0 then
              act.wparam:=wparam(DBReadUnicode(0,DBBranch,section,nil))
            else
              act.wparam:=wparam(DBReadString (0,DBBranch,section,nil));
          end;

          if (act.flags and (ACF_LCURRENT or ACF_LRESULT or ACF_LPARAM))=0 then
          begin
            StrCopy(p,opt_lparam);
            if (act.flags and ACF_LPARNUM)<>0 then
               act.lparam:=LoadNumValue(section,(act.flags2 and ACF2_SRV_LPAR)<>0)
  //              act.lparam:=DBReadDWord(0,DBBranch,section,0)
            else if (act.flags and ACF_LSTRUCT)<>0 then
              act.lparam:=lparam(DBReadUTF8(0,DBBranch,section,nil))
            else if (act.flags and ACF_LUNICODE)<>0 then
              act.lparam:=lparam(DBReadUnicode(0,DBBranch,section,nil))
            else
              act.lparam:=lparam(DBReadString(0,DBBranch,section,nil));
          end;

        end;

        ACT_PROGRAM: begin
          StrCopy(p,opt_prg ); act.prgname:=DBReadUnicode(0,DBBranch,section,nil);
          StrCopy(p,opt_args); act.args   :=DBReadUnicode(0,DBBranch,section,nil);
          StrCopy(p,opt_time); act.time   :=DBReadDWord  (0,DBBranch,section,0);
          StrCopy(p,opt_show); act.show   :=DBReadDWord  (0,DBBranch,section,SW_SHOW);
        end;

        ACT_TEXT: begin
          if (act.flags and ACF_CLIPBRD)=0 then
          begin
            StrCopy(p,opt_text); act.text:=DBReadUnicode(0,DBBranch,section,nil);
            if (act.flags and ACF_FILE)<>0 then
            begin
              StrCopy(p,opt_file); act.tfile:=DBReadUnicode(0,DBBranch,section,nil);
            end;
          end;
        end;

        ACT_ADVANCE: begin
          StrCopy(p,opt_cond   ); act.condition:=DBReadByte   (0,DBBranch,section);
          StrCopy(p,opt_value  ); act.value    :=DBReadDWord  (0,DBBranch,section);
          StrCopy(p,opt_action ); act.action   :=DBReadByte   (0,DBBranch,section);
          StrCopy(p,opt_oper   ); act.oper     :=DBReadByte   (0,DBBranch,section);
          StrCopy(p,opt_mathval); act.mathval  :=DBReadDWord  (0,DBBranch,section);
          StrCopy(p,opt_operval); act.operval  :=DBReadUnicode(0,DBBranch,section);
          StrCopy(p,opt_varval ); act.varval   :=DBReadUnicode(0,DBBranch,section);
        end;

        ACT_CHAIN: begin
          StrCopy(p,opt_text); act.id:=DBReadDWord(0,DBBranch,section);
        end;

        ACT_RW: begin
          if (act.flags and ACF_NOCNTCT)=0 then
          begin
            p^:=#0;
            act.dbcontact:=LoadContact(DBBranch,section);
          end;
          StrCopy(p,opt_module ); act.dbmodule :=DBReadString(0,DBBranch,section);
          StrCopy(p,opt_setting); act.dbsetting:=DBReadString(0,DBBranch,section);
          StrCopy(p,opt_value  );

          if (act.flags and ACF_DBUTEXT)=0 then
            act.dbvalue:=LoadNumValue(section,(act.flags2 and ACF2_RW_TVAR)<>0)
          else
            act.dbvalue:=uint_ptr(DBReadUnicode(0,DBBranch,section));
        end;

        ACT_MESSAGE: begin
          StrCopy(p,opt_msgtitle); act.msgtitle:=DBReadUnicode(0,DBBranch,section);
          StrCopy(p,opt_msgtext ); act.msgtext :=DBReadUnicode(0,DBBranch,section);
          StrCopy(p,opt_boxopts ); act.boxopts :=DBReadByte   (0,DBBranch,section);
        end;

      end;
      num:=NewAction(ActionList,MaxActions);
      move(act,ActionList^[num],SizeOf(tHKAction));
      if i=1 then
        result:=num
      else
        ActionList^[oldnum].next:=num;
      oldnum:=num;
    end;
  end;
end;

procedure LoadGroups;
var
  HK:pHKRecord;
  i,num:cardinal;
  p,p1:PAnsiChar;
  section:array [0..127] of AnsiChar;
  NumGroups,NumActions:cardinal;
begin
{ remove doubling - no need? (called just once)
  if MaxGroups>0 then
  begin
    while MaxGroups>0 do
    begin
      FreeGroup(MaxGroups);
      dec(MaxGroups);
    end;
    FreeMem(GroupList);
  end;
}  
  NumGroups:=DBReadWord(0,DBBranch,opt_numhk,HKListPage);
  if NumGroups<HKListPage then
    MaxGroups:=HKListPage
  else
    MaxGroups:=NumGroups;
  GetMem  (GroupList ,MaxGroups*SizeOf(tHKRecord));
  FillChar(GroupList^,MaxGroups*SizeOf(tHKRecord),0);

{ remove doubling - no need? (called just once)
  if MaxActions<>0 then
  begin
    act:=@ActionList[1];
    while MaxActions>0 do
    begin
      FreeAction(act);
      inc(act);
      dec(MaxActions);
    end;
    FreeMem(ActionList);
  end;
}
  NumActions:=DBReadWord(0,DBBranch,opt_numacts,ActListPage);
  if NumActions<ActListPage then
    MaxActions:=ActListPage
  else
    MaxActions:=NumActions+1;
  GetMem  (ActionList ,MaxActions*SizeOf(tHKAction));
  FillChar(ActionList^,MaxActions*SizeOf(tHKAction),0);

  HK:=@GroupList^; //??
  i:=0;
  p1:=StrCopyE(section,opt_group);
  while i<NumGroups do
  begin
    p:=StrEnd(IntToStr(p1,i));
    p^:='/'; inc(p);

    StrCopy(p,opt_flags);
    with HK^ do
    begin
      flags:=DBReadDWord(0,DBBranch,section,0{integer(ACF_ASSIGNED or ACF_DISABLED)});
      if (flags and ACF_ASSIGNED)<>0 then // not needed in normal cases
      begin
        StrCopy(p,opt_id   ); id   :=DBReadDWord  (0,DBBranch,section);
        StrCopy(p,opt_descr); descr:=DBReadUnicode(0,DBBranch,section,nil);
        if descr=nil then
          StrDupW(descr,TranslateW('No description'));

        StrCopy(p,opt_numacts); num:=DBReadWord(0,DBBranch,section);
        p^:=#0;
        firstAction:=LoadActions(section,num);
      end;
    end;
    inc(HK);
    inc(i);
  end;
  fCLfilter:=DBReadByte   (0,DBBranch,'CLfilter',0)<>0;
  fCLformat:=DBReadUnicode(0,DBBranch,'CLformat');
end;
