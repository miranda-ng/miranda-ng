{$ifdef BIN_IMPORT_}
var rsz:integer;

{$else}
begin
 pt:=integer(pFileText);
 i:=0;
 if FileLen>=$30 then
 if (StrLComp(PChar(pt),'midp-rms',8)=0) then
  begin
   if FileLen>=$48 then
    if (RLInteger(pt+$40)=$48) then
     begin
      fsz:=RLInteger(pt+$44);
      i:=$48;
     end;
   if (RLInteger(pt+$28)=$30) then
     begin
      fsz:=RLInteger(pt+$2C);
      i:=$30;
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
      while i<fsz do
       begin
        with dbei do
         begin
          fillchar(dbei,sizeof(dbei),0);
          cbSize:=sizeof(dbei);
          szModule:=PChar(proto);
          rsz:= RLInteger(pt+i+$08);
          if PByte(pt+i+$10)^=$00 then flags:=0
                                  else flags:=DBEF_SENT;
          flags:=flags or DBEF_READ or DBEF_UTF; //в файлах лежат в utf8 и импортируются без изменений
          eventType:= EVENTTYPE_MESSAGE;
          cbBlob:=RLWord(pt+i+$13+RLWord(pt+i+$11));
          pBlob:=Pointer(pt+i+$13+RLWord(pt+i+$11)+$2);
          timestamp:=StrToTimeStamp(pointer(pt+i+$13+RLWord(pt+i+$11)+$4+cbBlob),RLWord(pt+i+$13+RLWord(pt+i+$11)+$2+cbBlob));
          inc(cbBlob);
         end;
        try
        if not IsDuplicateEvent(DContact.hContact,dbei) then
         if pluginLink^.CallService(MS_DB_EVENT_ADD, wParam(DContact.hContact), lParam(@dbei))<>0 then Inc(AddedMessages)
                                                                                      else begin
                                                          s:= 'Error adding message to DB';
                                                          DoMessage(ITXT_THREAD_ERROR,integer(PWideChar(s)),0);  break; end

                                          else Inc(Duplicates);
        finally
        inc(i,rsz);
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
     s:=WideFormat(TranslateWideString('Its not %s file'),['midp-rms']);
     DoMessage(ITXT_THREAD_ERROR,integer(PWideChar(s)),0);
    end
        else //file len

end;
{$endif}
