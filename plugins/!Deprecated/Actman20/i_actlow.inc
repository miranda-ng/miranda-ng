{Lowlevel actions work: clone, create, delete, execute}

type
  tAdvExpr = (aeNot,aeAdd,aeSub,aeMul,aeDiv,aeMod,aeAnd,aeOr,aeXor,aeSet);

function GetActNameById(id:dword):PWideChar;
var
  i:integer;
begin
  for i:=0 to MaxGroups-1 do
  begin
    if ((GroupList^[i].flags and ACF_ASSIGNED)<>0) and
       (id=GroupList^[i].id) then
    begin
      result:=GroupList^[i].descr;
      exit;
    end;
  end;
  result:=nil;
end;

function GetActIdByName(name:PWideChar):integer;
var
  i:integer;
begin
  for i:=0 to MaxGroups-1 do
  begin
    if ((GroupList^[i].flags and ACF_ASSIGNED)<>0) and
       (StrCmpW(name,GroupList^[i].descr)=0) then
    begin
      result:=GroupList^[i].id;
      exit;
    end;
  end;
  result:=0;
end;

function GetActIdxByName(name:PWideChar):integer;
var
  i:integer;
begin
  for i:=0 to MaxGroups-1 do
  begin
    if ((GroupList^[i].flags and ACF_ASSIGNED)<>0) and
       (StrCmpW(name,GroupList^[i].descr)=0) then
    begin
      result:=i;
      exit;
    end;
  end;
  result:=-1;
end;

function GetActIdxById(id:dword):integer;
var
  i:integer;
begin
  for i:=0 to MaxGroups-1 do
  begin
    if ((GroupList^[i].flags and ACF_ASSIGNED)<>0) and
       (id=GroupList^[i].id) then
    begin
      result:=i;
      exit;
    end;
  end;
  result:=-1;
end;

function FreeAction(act:pHKAction):dword;
begin
  result:=act^.next;
  with act^ do
  begin
    if (flags and ACF_ASSIGNED)<>0 then
    begin
      mFreeMem(descr);
      case actionType of
        ACT_SERVICE: begin
          mFreeMem(service);
          if (flags and (ACF_WPARNUM or ACF_WRESULT or ACF_WPARAM))=0 then
            mFreeMem(pointer(wparam));
          if ((flags and ACF_WPARNUM)<>0) and ((flags2 and ACF2_SRV_WPAR)<>0) then
            mFreeMem(pointer(wparam));
          if (flags and (ACF_LPARNUM or ACF_LRESULT or ACF_LPARAM))=0 then
            mFreeMem(pointer(lparam));
          if ((flags and ACF_LPARNUM)<>0) and ((flags2 and ACF2_SRV_LPAR)<>0) then
            mFreeMem(pointer(lparam));
        end;
        ACT_PROGRAM: begin
          mFreeMem(prgname);
          mFreeMem(args);
        end;
        ACT_TEXT: begin
          if (flags and ACF_CLIPBRD)=0 then
          begin
            mFreeMem(text);
            if (flags and ACF_FILE)<>0 then
              mFreeMem(tfile);
          end;
        end;
        ACT_ADVANCE: begin
          mFreeMem(varval);
          if (action and ADV_ACT_POST)=ADV_ACT_JUMP then
            mFreeMem(operval);
        end;
        ACT_CHAIN: begin
          if (flags and ACF_BYNAME)<>0 then
            mFreeMem(actname);
        end;
        ACT_RW: begin
          mFreeMem(dbmodule);
          mFreeMem(dbsetting);
          if (flags and ACF_DBUTEXT)<>0 then
            mFreeMem(dbvalue)
          else if (flags2 and ACF2_RW_TVAR)<>0 then
            mFreeMem(dbvalue);
        end;
        ACT_MESSAGE: begin
          mFreeMem(msgtitle);
          mFreeMem(msgtext);
        end;
      end;
    end;
  end;
  FillChar(act^,SizeOf(act^),0);
end;

procedure FreeActions(list:pActList;idx:cardinal);
begin
  while idx<>0 do
    idx:=FreeAction(@list^[idx]);
end;

procedure FreeActionsContinued(act:pHKAction);
var
  act_org:pHKAction;
begin
  act_org:=act;
  repeat
    FreeAction(act);
    if act^.next<>0 then
      inc(act)
    else
      break;
  until false;
  FreeMem(act_org);
end;

procedure DestroyActions(act:pActList;count:integer);
var
  pact:pHKAction;
begin
  pact:=@act^;
  while count>0 do
  begin
    FreeAction(pact);
    inc(pact);
    dec(count);
  end;
  FreeMem(act);
end;

procedure CloneAction(dst,src:pHKAction);
begin
  move(src^,dst^,SizeOf(tHKAction));
  with dst^ do
  begin
    if (flags and ACF_ASSIGNED)<>0 then
    begin
      StrDupW(descr,descr);
      case actionType of
        ACT_SERVICE: begin
          StrDup(service,service);
          if (flags and ACF_WPARNUM)=0 then
          begin
            if (flags and ACF_WSTRUCT)<>0 then
              StrDupW(pWideChar(wparam),pWideChar(wparam))
            else if (flags and ACF_WUNICODE)<>0 then
              StrDupW(pWideChar(wparam),pWideChar(wparam))
            else
              StrDup(PAnsiChar(wparam),PAnsiChar(wparam));
          end
          else if (flags2 and ACF2_SRV_WPAR)<>0 then
            StrDupW(pWideChar(wparam),pWideChar(wparam));

          if (flags and ACF_LPARNUM)=0 then
          begin
            if (flags and ACF_LSTRUCT)<>0 then
              StrDupW(pWideChar(lparam),pWideChar(lparam))
            else if (flags and ACF_LUNICODE)<>0 then
              StrDupW(pWideChar(lparam),pWideChar(lparam))
            else
              StrDup(PAnsiChar(lparam),PAnsiChar(lparam));
          end
          else if (flags2 and ACF2_SRV_LPAR)<>0 then
            StrDupW(pWideChar(lparam),pWideChar(lparam));
        end;

        ACT_PROGRAM: begin
          StrDupW(prgname,prgname);
          StrDupW(args,args);
        end;

        ACT_TEXT: begin
          if (flags and ACF_CLIPBRD)=0 then
          begin
            StrDupW(text,text);
            if (flags and ACF_FILE)<>0 then
              StrDupW(tfile,tfile);
          end;
        end;

        ACT_ADVANCE: begin
          StrDupW(varval,varval);
          if (action and ADV_ACT_POST)=ADV_ACT_JUMP then
            StrDupW(operval,operval);
        end;

        ACT_CHAIN: begin
          if (flags or ACF_BYNAME)<>0 then
            StrDupW(actname,actname);
        end;

        ACT_RW: begin
          StrDup(dbmodule,dbmodule);
          StrDup(dbsetting,dbsetting);
          if (flags and ACF_DBUTEXT)<>0 then
            StrDupW(pWideChar(dbvalue),pWideChar(dbvalue))
          else if (flags2 and ACF2_RW_TVAR)<>0 then
            StrDupW(pWideChar(dbvalue),pWideChar(dbvalue));
        end;

        ACT_MESSAGE: begin
          StrDupW(msgtitle,msgtitle);
          StrDupW(msgtext,msgtext);
        end;
      end;
    end;
  end;
end;

function CloneActions(idx:cardinal):pointer;
var
  i,count:integer;
  aList:pHKAction;
begin
  count:=0;
  i:=idx;
  while i<>0 do
  begin
    inc(count);
    i:=ActionList^[i].next;
  end;
  if count>0 then
  begin
    GetMem(result,count*SizeOf(tHKAction));
    aList:=result;
    i:=idx;
    while i<>0 do
    begin
      CloneAction(aList,@ActionList^[i]);
      i:=ActionList^[i].next;
      inc(aList);
    end;
  end
  else
    result:=nil;
end;

function DoAction(action:dword;aparam:LPARAM;var last:uint_ptr;restype:dword):integer;
var
  tmpact,act,act_org:pHKAction;
  val,prelast:uint_ptr;
  b:boolean;
  i:integer;
  lContact:THANDLE;
  buf:array [0..31] of WideChar;
  tmpc,tmpc1,tmpc2,tmpcv1,tmpcv2:pWideChar;
  oldrestype:integer;
begin
  if action<>0 then
  begin
    act:=CloneActions(action);
    // if act=nil then exit;
    act_org   :=act;
    oldrestype:=restype;
    prelast   :=0;
    repeat
      if (act^.flags and ACF_DISABLED)=0 then
      begin

        if (oldrestype=rtWide) and (last<>prelast) then
          mFreeMem(prelast);
        oldrestype:=restype;
        prelast:=last;

        with act^ do
        begin
          case actionType of

            ACT_CONTACT: begin
              if (flags and ACF_KEEPONLY)=0 then
                last:=OpenContact(contact)
              else
                last:=contact;

              restype:=rtInt;
            end;

            ACT_SERVICE: begin
              last:=RunService(act,last,aparam,restype);
{
              if (flags and ACF_STRING)<>0 then
              begin
                if (flags and ACF_UNICODE)=0 then
                begin
                  val:=last;
                  AnsiToWide(pAnsiChar(val),pWideChar(last),MirandaCP);
                  mFreeMem(val);
                end
                else
                  StrDupW(pWideChar(last),pWideChar(last));
                restype:=rtWide;
              end
              else
                restype:=rtInt;
}
            end;

            ACT_PROGRAM: begin
              if restype=rtInt then
                last:=uint_ptr(IntToStr(buf,last));

              last:=RunProgram(act,aparam,pWideChar(last));

              restype:=rtInt;
            end;

            ACT_TEXT: begin
              if restype=rtInt then
                last:=uint_ptr(IntToStr(buf,last));

              last:=InsertText(act,aparam,pWideChar(last));

              restype:=rtWide;
            end;

            ACT_ADVANCE: begin
              if restype=rtWide then
                val:=StrToInt(pWideChar(last))
              else
                val:=last;

              case condition and not ADV_COND_NOT of
                ADV_COND_EQ: b:=val=value;
                ADV_COND_GT: b:=integer(val)>integer(value);
                ADV_COND_LT: b:=integer(val)<integer(value);
              else
                b:=true;
              end;
              if ((condition and ADV_COND_NOT)<>0) and (condition<>ADV_COND_NOP) then
                b:=not b;
              if b then
              begin
                case action and ADV_ACTION of

                  ADV_ACT_MATH: begin
                    case tAdvExpr(oper) of
                      aeNot: last:= not val;
                      aeAdd: last:= integer(val) +   integer(mathval);
                      aeSub: last:= integer(val) -   integer(mathval);
                      aeMul: last:= integer(val) *   integer(mathval);
                      aeDiv: last:= integer(val) div integer(mathval);
                      aeMod: last:= val mod mathval;
                      aeAnd: last:= val and mathval;
                      aeOr : last:= val or  mathval;
                      aeXor: last:= val xor mathval;
                      aeSet: last:= mathval;
                    end;
                    restype:=rtInt;
                  end;

                  ADV_ACT_VARS: begin
//!! need to clear 'Last' if was string?
                    if (varval<>NIL) and (varval^<>#0) then
                    begin
                      if CallService(MS_DB_CONTACT_IS,aparam,0)<>0 then
                        lContact:=aparam
                      else
                        lContact:=0;
                      if restype=rtInt then
                       last:=uint_ptr(IntToStr(buf,last));

                      pWideChar(last):=ParseVarString(varval,lContact,pWideChar(last));

                      if (flags and ACF_VARASINT)<>0 then
                      begin
                        tmpc:=pWideChar(last);
                        last:=StrToInt(tmpc);
                        mFreeMem(tmpc);
                        restype:=rtInt;
                      end
                      else
                        restype:=rtWide;
                    end;
                  end;

                end;
                case action and ADV_ACT_POST of

                  ADV_ACT_JUMP : begin
                    tmpact:=act_org;
                    repeat
                      if StrCmpW(tmpact^.descr,operval)=0 then
                      begin
                        act:=tmpact;
                        tmpact:=nil;
                        break;
                      end;
                      if tmpact^.next=0 then
                        break;
                      inc(tmpact);
                    until false;
                   if tmpact=nil then continue;
                  end;

                  ADV_ACT_BREAK: break;
                end;
              end;
            end;//last:=MakeAdvanced(act,last);

            ACT_CHAIN: begin
              if (flags and ACF_BYNAME)<>0 then
                i:=GetActIdxByName(actname)
              else
                i:=GetActIdxById(id);

              if i>=0 then
              begin
                restype:=DoAction(GroupList^[i].firstAction,aparam,last,restype);
                // cleared in called Action
                oldrestype:=rtInt;
                prelast:=0;
              end
              else
              begin
                restype:=rtInt;
                last:=0;
              end;
            end;

            ACT_RW: begin
              if      (flags and ACF_CURRENT)<>0 then i:=0
              else if (flags and ACF_PARAM  )<>0 then i:=aparam
              else if (flags and ACF_RESULT )<>0 then i:=last
              else
                i:=dbcontact;
              if (flags and ACF_LAST)=0 then
                val:=dbvalue
              else
              begin
                val:=last;
                if (flags and ACF_DBUTEXT)<>0 then
                begin
                  if restype=rtInt then
                    val:=uint_ptr(IntToStr(buf,val));
                end
                else
                begin
                  if restype=rtWide then
                    val:=StrToInt(pWideChar(val));
                end;
              end;

              last:=DBRW(act,i,val,last,restype);

              if (flags and ACF_DBUTEXT)<>0 then
                restype:=rtWide
              else
                restype:=rtInt;
            end;

            ACT_MESSAGE: begin
              if restype=rtWide then
                tmpc:=PWideChar(last)
              else
              begin
                IntToStr(buf,last);
                tmpc:=@buf;
              end;

              if StrPosW(msgtitle,'<last>')<>nil then
              begin
                mGetMem(tmpc1,8192);
                StrCopyW(tmpc1,msgtitle);
                StrReplaceW(tmpc1,'<last>',tmpc);
              end
              else
                tmpc1:=msgtitle;
              if StrPosW(msgtext,'<last>')<>nil then
              begin
                mGetMem(tmpc2,8192);
                StrCopyW(tmpc2,msgtext);
                StrReplaceW(tmpc2,'<last>',tmpc);
              end
              else
                tmpc2:=msgtext;

              if (flags2 and ACF2_MSG_TTL)<>0 then
                tmpcv1:=ParseVarString(tmpc1,aparam,tmpc)
              else
                tmpcv1:=tmpc1;
              if (flags2 and ACF2_MSG_TXT)<>0 then
                tmpcv2:=ParseVarString(tmpc2,aparam,tmpc)
              else
                tmpcv2:=tmpc2;

              i:=MessageBoxW(0,tmpcv2,tmpcv1,boxopts);

              if (flags and ACF_MSG_KEEP)=0 then
              begin
                restype:=rtInt;
                last:=i
              end;

              if tmpcv1<>tmpc1    then mFreeMem(tmpcv1);
              if tmpcv2<>tmpc2    then mFreeMem(tmpcv2);
              if tmpc1 <>msgtitle then mFreeMem(tmpc1);
              if tmpc2 <>msgtext  then mFreeMem(tmpc2);
            end;

          else
            last:=0;
          end;
        end;
      end;
      if (act_org^.flags and ACF_DOBREAK)<>0 then
        break;
      if act^.next=0 then
        break;
      inc(act);
    until false;
    FreeActionsContinued(act_org);
    if (oldrestype=rtWide) and (last<>prelast) then
      mFreeMem(prelast);
  end;
  result:=restype;
end;

type
  pActStartData = ^tActStartData;
  tActStartData = record
    event :THANDLE;
    action:dword;
    param :LPARAM;
    group :pHKRecord;
    last  :LPARAM;
  end;

procedure ThDoAction(arg:pActStartData); cdecl;
var
  ltmp:uint_ptr;
  res:integer;
begin
  ltmp:=arg^.last;

  if arg^.group<>nil then
  begin
    NotifyEventHooks(hevaction,arg^.group.id,0); // started
    arg^.group.flags:=arg^.group.flags or ACF_USEDNOW;
  end;

  res:=DoAction(arg^.action,arg^.param,ltmp,rtInt);

  if arg^.group<>nil then
  begin
    arg^.group.flags:=arg^.group.flags and not ACF_USEDNOW;
    NotifyEventHooks(hevaction,arg^.group.id,1); // finished
  end;

  if arg^.event<>0 then
  begin
    arg^.last:=ltmp;
    SetEvent(arg^.event);
  end
  else if res=rtWide then
  begin
    mFreeMem(ltmp);
  end;
end;

function ActionStarterWait(action:dword;aparam:LPARAM=0;group:pHKRecord=nil;alast:LPARAM=0):LPARAM;
var
  tmp:pActStartData;
begin
  mGetMem(tmp,SizeOf(tActStartData));
  tmp^.action:=action;
  tmp^.param :=aparam;
  tmp^.group :=group;
  tmp^.last  :=alast;
  tmp^.event :=CreateEvent(nil,FALSE,FALSE,nil);
  mir_forkthread(@ThDoAction,tmp);
  WaitForSingleObjectEx(tmp.event,INFINITE,true);
  CloseHandle(tmp^.event);
  result:=tmp^.last;
end;

function ActionStarter(action:dword;aparam:dword=0;group:pHKRecord=nil;alast:dword=0):integer;
var
  tmp:pActStartData;
begin
  result:=0;
  mGetMem(tmp,SizeOf(tActStartData));
  tmp^.action:=action;
  tmp^.param :=aparam;
  tmp^.group :=group;
  tmp^.last  :=alast;
  tmp^.event :=0;
  mir_forkthread(@ThDoAction,tmp);
end;

procedure ReallocActionList(var ActList:pActList;var MaxAct:cardinal);
var
  i:cardinal;
  tmp:pActList;
begin
  i:=(MaxAct+ActListPage)*SizeOf(tHKAction);
  GetMem(tmp,i);
  FillChar(tmp^,i,0);
  if MaxAct>0 then
  begin
    move(ActList^,tmp^,MaxAct*SizeOf(tHKAction));
    FreeMem(ActList);
  end;
  ActList:=tmp;
  inc(MaxAct,ActListPage);
end;

function NewAction(var ActList:pActList;var MaxAct:cardinal):cardinal;
var
  i:cardinal;
  pAct:pHKAction;
begin
  i:=1;
  pAct:=@ActList^;
  inc(pAct); // skip zero
  while i<MaxAct do
  begin
    if (pAct^.flags and ACF_ASSIGNED)=0 then
    begin
      result:=i;
      FillChar(pAct^,SizeOf(tHKAction),0);
      pAct^.actionType:=ACT_CONTACT;
      pAct^.flags     :=ACF_ASSIGNED;
      exit;
    end;
    inc(i);
    inc(pAct);
  end;

  if MaxAct=0 then
    result:=1
  else
    result:=MaxAct;

  ReallocActionList(ActList,MaxAct);

  ActList^[result].actionType:=ACT_CONTACT;
  ActList^[result].flags     :=ACF_ASSIGNED;
end;

procedure ReallocHKList(var HKList:pHKList;var MaxHK:cardinal);
var
  i:cardinal;
  tmp:pHKList;
begin
  i:=(MaxHK+HKListPage)*SizeOf(tHKRecord);
  GetMem(tmp,i);
  FillChar(tmp^,i,0);
  if MaxHK>0 then
  begin
    move(HKList^,tmp^,MaxHK*SizeOf(tHKRecord));
    FreeMem(HKList);
  end;
  HKList:=tmp;
  inc(MaxHK,HKListPage);
end;

procedure InitGroupValue(pHK:pHKRecord);
var
//  time:TSYSTEMTIME;
  tmp:int64;
begin
  with pHK^ do
  begin
    StrDupW(descr,NoDescription);
{
    GetSystemTime(time);
    id         :=time.wSecond+time.wMinute*60+time.wHour*3600+time.wMilliseconds*86400;
}
    QueryPerformanceCounter(tmp);
    id         :=tmp and $FFFFFFFF;
    firstAction:=0;
    active     :=nil;
    flags      :=ACF_ASSIGNED;
  end;
end;

// Root,Size,MaxCount(Page,flag)
function NewGroup(var HKList:pHKList;var MaxHK:cardinal):cardinal;
var
  i:cardinal;
  pHK:pHKRecord;
begin
  i:=0;
  pHK:=@HKList^;
  while i<MaxHK do
  begin
    if (pHK^.flags and ACF_ASSIGNED)=0 then
    begin
      result:=i;
      InitGroupValue(pHK);
      exit;
    end;
    inc(i);
    inc(pHK);
  end;
  // realloc
  result:=MaxHK;
  ReallocHKList(HKList,MaxHK);
  InitGroupValue(@HKList^[result]);
end;

procedure FreeGroup(num:cardinal);
begin
  with GroupList^[num] do
  begin
    if (flags and ACF_ASSIGNED)<>0 then
    begin
      flags:=0;
      mFreeMem(descr);
      FreeActions(ActionList,firstAction);
    end;
  end;
end;

procedure FreeGroups;
var
  i:integer;
begin
  for i:=0 to MaxGroups-1 do
  begin
    FreeGroup(i);
  end;
  MaxGroups:=0;
  FreeMem(GroupList);
  FreeMem(ActionList);
  GroupList:=nil;
  ActionList:=nil;
end;

procedure DestroyGroups(HKList:pHKList;count:integer);
var
  pHK:pHKRecord;
begin
  pHK:=@HKList^;
  while count>0 do
  begin
    if (pHK^.flags and ACF_ASSIGNED)<>0 then
      mFreeMem(pHK^.descr);
    inc(pHK);
    dec(count);
  end;
  FreeMem(HKList);
end;

function CloneActionList:pActList;
var
  src,dst:pHKAction;
  i:integer;
begin
  i:=MaxActions;
  GetMem(result,i*SizeOf(tHKAction));
  src:=@ActionList^;
  dst:=@result^;
  while i>0 do
  begin
    CloneAction(dst,src);
    inc(src);
    inc(dst);
    dec(i);
  end;
end;

procedure CloneGroup(dst,src:pHKRecord);
begin
  move(src^,dst^,SizeOf(tHKRecord));
  if (src^.flags and ACF_ASSIGNED)<>0 then
    StrDupW(dst^.descr,src^.descr);
end;

function CloneGroupList:pHKList;
var
  src,dst:pHKRecord;
  i:integer;
begin
  i:=MaxGroups;
  GetMem(result,i*SizeOf(tHKRecord));
  src:=@GroupList^;
  dst:=@result^;
  while i>0 do
  begin
    CloneGroup(dst,src);
    inc(src);
    inc(dst);
    dec(i);
  end;
end;

function ActSelect(wParam:WPARAM;lParam:LPARAM):int;cdecl;
begin
  if (wParam and ACCF_ID)<>0 then
    result:=GetActIdxById(lParam)
  else
    result:=GetActIdxByName(pWideChar(lParam));
  if result=-1 then
    exit;
  with GroupList^[result] do
  begin
    if (wParam and ACCF_CLEAR)<>0 then
      flags:=flags and not (uint_ptr(wParam) and ACCF_FLAGS)
    else
      flags:=flags or      (uint_ptr(wParam) and ACCF_FLAGS);
  end;
end;
