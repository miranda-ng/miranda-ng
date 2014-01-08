{$IFDEF BIN_IMPORT_}
{$ELSE}

begin
  pt := uint_ptr(pFileText);
  if FileLen >= 10 then
    if (PWORD(pt)^ = $4248) and (PDWORD(pt + 2)^ = $FFFFFFFF) then
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
        fsz := PDWORD(pt + 6)^; // размер данных в файле
        DoMessage(ITXT_THREAD_MAXPROGRESS, 0, fsz);
        inc(pt, 10);
        i := 0;
        while cardinal(i) < fsz do
        begin
          dbei.cbSize    := PInteger(pt + i)^;
          dbei.szModule  := PAnsiChar(proto);
          dbei.timestamp := PDWORD(pt + i + $08)^;
          dbei.flags     := PDWORD(pt + i + $0C)^;
          dbei.eventtype := PDWORD(pt + i + $10)^;
          dbei.cbBlob    := PDWORD(pt + i + $14)^;
          dbei.pBlob     := PBYTE(pt + i + dbei.cbSize);

          if not IsDuplicateEvent(DContact.hContact, dbei) then
            if db_event_add(DContact.hContact, @dbei) <> 0 then
              inc(AddedMessages)
            else
            begin
              s := 'Error adding message to database';
              DoMessage(ITXT_THREAD_ERROR, wparam(PWideChar(s)), 0);
              break;
            end
          else
            inc(Duplicates);
          inc(i, dbei.cbSize + integer(dbei.cbBlob));
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
      s := WideFormat(TranslateWideString('It''s not %s file'), ['mContact']);
      DoMessage(ITXT_THREAD_ERROR, wparam(PWideChar(s)), 0);
    end
  else // file len
end;
{$ENDIF}
