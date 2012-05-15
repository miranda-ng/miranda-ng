{$ifdef BIN_IMPORT_}

{$else}
begin
   pt:=integer(pFileText);
   if FileLen>=10 then
   if (PWORD(pt)^=$4248) and (PDWORD(pt+2)^=$FFFFFFFF) then
    begin
    //смотрим есть ли у нас контакт назначения
    if (DContact.hContact=0) or (DContact.hContact=INVALID_HANDLE_VALUE) then
     if (fContact.hContact<>0) and (fContact.hContact<>INVALID_HANDLE_VALUE) then
         DContact:=fContact;
    if (DContact.hContact<>0) and (DContact.hContact<>INVALID_HANDLE_VALUE) then
     begin
      proto:=GetContactProto(DContact.hContact);
      DoMessage(ITXT_THREAD_DEST_CONTACT,DContact.hContact,0);
      DoMessage(ITXT_THREAD_START,0,0);
      fsz:=PDWORD(pt+6)^; //размер данных в файле
      DoMessage(ITXT_THREAD_MAXPROGRESS,0,fsz); 
      inc(pt,10);
      i:=0;
      while i<fsz do
       begin
        with dbei do
         begin
          cbSize:=PInteger(pt+i)^;
          szModule:=PChar(proto);
          timestamp:=PDWORD(pt+i+$08)^;
          flags:=PDWORD(pt+i+$0C)^;
          eventtype:=PDWORD(pt+i+$10)^;
          cbBlob:=PDWORD(pt+i+$14)^;
          pBlob:=PBYTE(pt+i+cbSize);
         end;
        if not IsDuplicateEvent(DContact.hContact,dbei) then
         if pluginLink^.CallService(MS_DB_EVENT_ADD, wParam(DContact.hContact), lParam(@dbei))<>0 then Inc(AddedMessages)
                                                                                      else begin
                                                          s:= 'Error adding message to DB';
                                                          DoMessage(ITXT_THREAD_ERROR,integer(PWideChar(s)),0);  break; end

                                          else Inc(Duplicates);
        inc(i,dbei.cbSize+dbei.cbBlob);
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
     s:=WideFormat(TranslateWideString('Its not %s file'),['mContact']);
     DoMessage(ITXT_THREAD_ERROR,integer(PWideChar(s)),0);
    end
                  else //file len
end;
{$endif}
