{$ifdef BIN_IMPORT_}
var j:integer;
{$else}
begin
   pt:=integer(pFileText);
   if FileLen>=10 then
   if true {(PWORD(pt)^=$4248) and (PDWORD(pt+2)^=$FFFFFFFF)} then
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
      fsz:=FileLen-2; //размер данных в файле
      DoMessage(ITXT_THREAD_MAXPROGRESS,0,fsz); 
      inc(pt,2);
      i:=0;
      while i<fsz do
       begin
        with dbei do
         begin
          fillchar(dbei,sizeof(dbei),0);
          cbSize:=sizeof(dbei);
          szModule:=PChar(proto);
          timestamp:=ImportTU.TimeStamp(
          RLWORD(pt+i+5),
          PByte(pt+i+4)^,
          PByte(pt+i+3)^,
          PByte(pt+i)^,
          PByte(pt+i+1)^,
          PByte(pt+i+2)^);
          if PByte(pt+i+$7)^=$00 then flags:=DBEF_READ
                                 else flags:=DBEF_SENT or DBEF_READ;
          eventType:= EVENTTYPE_MESSAGE;
          cbBlob:=(RLWORD(pt+i+$8)+1);
           s:='';
          for j:=0 to RLWORD(pt+i+$8) do
           s:=s+WideChar(RLWORD(pt+i+$0A+(j*2)));
          if IsMirandaUnicode then
            begin
             flags:=flags or DBEF_UTF;
             cbBlob:=cbBlob*SizeOf(WideChar);
             pBlob:=PByte(WideToUTF8(PWideChar(S),PChar(pBlob)));
            end
                              else
            pBlob:=PByte(WideToANSI(PWideChar(S),PChar(pBlob),cp));
         end;
        if not IsDuplicateEvent(DContact.hContact,dbei) then
         if pluginLink^.CallService(MS_DB_EVENT_ADD, wParam(DContact.hContact), lParam(@dbei))<>0 then Inc(AddedMessages)
                                                                                      else begin
                                                          s:= 'Error adding message to DB';
                                                          DoMessage(ITXT_THREAD_ERROR,integer(PWideChar(s)),0);  break; end

                                          else Inc(Duplicates);
        inc(i,dbei.cbBlob+$0C);
        FreeMem(dbei.pBlob);
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
     s:=WideFormat(TranslateWideString('Its not %s file'),['bayanICQ']);
     DoMessage(ITXT_THREAD_ERROR,integer(PWideChar(s)),0);
    end
                  else //file len
end;
{$endif}
