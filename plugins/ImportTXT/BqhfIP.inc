{$ifdef BIN_IMPORT_}
var
    qhfver:byte;
    szMsgHd:word;
    UIDstr:UTF8String;
    UIDLen,NickLen:word;
    NickStr:UTF8String;

function DecodeQHFMsg(ver:byte; adr:integer; cSize:word):string;

function get1(c: char): byte;
begin
 if ShortInt(c)>=0 then Result:=Byte(c)
 else Result := Byte(c)+256;
end;

function get2(b:byte): char;
begin
 if b<128 then result:=Char(b)
 else result:=Char(b-256);
end;

var i:integer;
begin
 SetLength(Result,cSize);
 for i:=1 to cSize do
  begin
   Result[i]:=PChar(adr+i-1)^;
   if ver<>$01 then
    begin
     inc(Result[i],i);
     Result[i]:=get2(255-get1(Result[i]));
    end
  end;
end;

{$else}
begin
   pt:=integer(pFileText);
   if FileLen>=$2E then
   if (PChar(pt)^='Q') and (PChar(pt+1)^='H') and (PChar(pt+2)^='F') then
    begin
    qhfver:=PByte(pt+3)^;
    if qhfver>=$03 then szMsgHd:=$23
                   else szMsgHd:=$21;
    fsz:=RLInteger(pt+$04);
    UIDLen:=RLWord(pt+$2C);
    SetLength(UIDstr,UIDLen);
    if qhfver<=2 then  lstrcpynA(PChar(UIDstr),PChar(pt+$2E),UIDLen)
                 else  lstrcpynA(PChar(UIDstr),PChar(pt+$2E),UIDLen+1);
    NickLen:=RLWord(pt+$2E+UIDLen);
    SetLength(NickStr,NickLen);
    if qhfver<=2 then  lstrcpynA(PChar(NickStr),PChar(pt+$2E+UIDLen+2),UIDLen)
                 else  lstrcpynA(PChar(NickStr),PChar(pt+$2E+UIDLen+2),UIDLen+1);
    inc(pt,$2E+UIDLen+2+NickLen);
    if fsz<>FileLen-($2E+UIDLen+2+NickLen) then fsz:= FileLen-($2E+UIDLen+2+NickLen);
    if (DContact.hContact=0) or (DContact.hContact=INVALID_HANDLE_VALUE) then
      begin
      if IsMirandaUnicode then DContact.ContactUID:=UIDstr
                          else
       begin
        tempstr:=UTF8ToANSI(PChar(UIDstr),tempstr,cp);
        DContact.ContactUID:=tempstr;
        FreeMem(tempstr);
       end;
      if IsMirandaUnicode then DContact.ContactNick:=NickStr
                          else
       begin
        tempstr:=UTF8ToANSI(PChar(NickStr),tempstr,cp);
        DContact.ContactNick:=tempstr;
        FreeMem(tempstr);
       end;
      TryDetermContact(DContact);
      end;
    if (DContact.hContact=0) or (DContact.hContact=INVALID_HANDLE_VALUE) then
     if (fContact.hContact<>0) and (fContact.hContact<>INVALID_HANDLE_VALUE) then
         DContact:=fContact;
    if (DContact.hContact<>0) and (DContact.hContact<>INVALID_HANDLE_VALUE) then
     begin
      proto:=GetContactProto(DContact.hContact);
      DoMessage(ITXT_THREAD_DEST_CONTACT,DContact.hContact,0);
      DoMessage(ITXT_THREAD_START,0,0);
      DoMessage(ITXT_THREAD_MAXPROGRESS,0,fsz);
      i:=0;
      while i<fsz do
       begin
        with dbei do
         begin
          fillchar(dbei,sizeof(dbei),0);
          cbSize:=sizeof(dbei);
          szModule:=PChar(proto);
          timestamp:=RLInteger(pt+i+$12);
          timestamp:=timestamp-(PluginLink.CallService(MS_DB_TIME_TIMESTAMPTOLOCAL,timestamp,0)-timestamp);//приводим к GMT 
          if PByte(pt+i+$1A)^=$00 then flags:=0
                                  else flags:=DBEF_SENT;
          flags:=flags or DBEF_READ or DBEF_UTF; //в файлах лежат в utf8 и импортируются без изменений
          eventType:= EVENTTYPE_MESSAGE;
          cbBlob:=RLWord(pt+i+szMsgHd-2)+1; //размер текста сообщения с #0
          pBlob:=AllocMem(cbBlob);
         end;
        try
        Move(DecodeQHFMsg(qhfver,pt+i+szMsgHd,dbei.cbBlob-1)[1],dbei.pBlob^,dbei.cbBlob-1);
        if not IsDuplicateEvent(DContact.hContact,dbei) then
         if pluginLink^.CallService(MS_DB_EVENT_ADD, wParam(DContact.hContact), lParam(@dbei))<>0 then Inc(AddedMessages)
                                                                                      else begin
                                                          s:= 'Error adding message to DB';
                                                          DoMessage(ITXT_THREAD_ERROR,integer(PWideChar(s)),0);  break; end

                                          else Inc(Duplicates);

        finally
        inc(i,szMsgHd+dbei.cbBlob-1);
        FreeMem(dbei.pBlob);
        end;
        DoMessage(ITXT_THREAD_PROGRESS,i,0);
       end;
     end
                                                                            else
     begin
      s:=TranslateWideString('Can''t determine destination contact');
      DoMessage(ITXT_THREAD_ERROR,integer(PWideChar(s)),0);
     end;
    end
        else
    begin
     s:=WideFormat(TranslateWideString('Its not %s file'),['QHF']);
     DoMessage(ITXT_THREAD_ERROR,integer(PWideChar(s)),0);
    end
        else //file len
end;
{$endif}