{$IFDEF BIN_IMPORT_}

var
  j: integer;
{$ELSE}

begin
  pt := uint_ptr(pFileText);
  if FileLen >= 10 then
    if true { (PWORD(pt)^=$4248) and (PDWORD(pt+2)^=$FFFFFFFF) } then
    begin
      // смотрим есть ли у нас контакт назначения
      if (DContact.hContact = 0) or (DContact.hContact = INVALID_HANDLE_VALUE) then
        if (fContact.hContact <> 0) and (fContact.hContact <> INVALID_HANDLE_VALUE) then
          DContact := fContact;
      if (DContact.hContact <> 0) and (DContact.hContact <> INVALID_HANDLE_VALUE) then
      begin
        proto := GetContactProto(DContact.hContact);
        DoMessage(ITXT_THREAD_DEST_CONTACT, DContact.hContact, 0);
        DoMessage(ITXT_THREAD_START, 0, 0);
        fsz := FileLen - 2; // размер данных в файле
        DoMessage(ITXT_THREAD_MAXPROGRESS, 0, fsz);
        inc(pt, 2);
        i := 0;
        while Cardinal(i) < fsz do
        begin
          with dbei do
          begin
            fillchar(dbei, sizeof(dbei), 0);
            dbei.cbSize := sizeof(dbei);
            dbei.szModule := PAnsiChar(proto);
            dbei.timestamp := ImportTU.timestamp(
              RLWORD(pt + i + 5),
              PByte(pt + i + 4)^,
              PByte(pt + i + 3)^,
              PByte(pt + i)^,
              PByte(pt + i + 1)^,
              PByte(pt + i + 2)^);

            if PByte(pt + i + $7)^ = $00 then
              dbei.flags := DBEF_READ or DBEF_UTF
            else
              dbei.flags := DBEF_READ or DBEF_UTF or DBEF_SENT;
            dbei.eventType := EVENTTYPE_MESSAGE;
            dbei.cbBlob := (RLWORD(pt + i + $8) + 1) * sizeof(WideChar);
            s := '';
            for j := 0 to RLWORD(pt + i + $8) do
              s := s + WideChar(RLWORD(pt + i + $0A + (j * 2)));

            dbei.pBlob := PByte(WideToUTF8(PWideChar(s), PAnsiChar(pBlob)));
          end;
          if not IsDuplicateEvent(DContact.hContact, dbei) then
            if db_event_add(wParam(DContact.hContact), @dbei) <> 0 then
              inc(AddedMessages)
            else
            begin
              s := 'Error adding message to database';
              DoMessage(ITXT_THREAD_ERROR, wparam(PWideChar(s)), 0);
              break;
            end

          else
            inc(Duplicates);
          inc(i, dbei.cbBlob + $0C);
          FreeMem(dbei.pBlob);
          DoMessage(ITXT_THREAD_PROGRESS, i, 0);
        end;
      end
      else
      begin
        s := TranslateWideString('Can''t determine destination contact');
        DoMessage(ITXT_THREAD_ERROR, wparam(PWideChar(s)), 0);
      end;
    end
    else
    begin
      s := WideFormat(TranslateWideString('It''s not %s file'), ['bayanICQ']);
      DoMessage(ITXT_THREAD_ERROR, wparam(PWideChar(s)), 0);
    end
  else // file len
end;
{$ENDIF}
