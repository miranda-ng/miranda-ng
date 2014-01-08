{$IFDEF BIN_IMPORT_}
var
  rsz: integer;

{$ELSE}

begin
  pt := uint_ptr(pFileText);
  i := 0;
  if FileLen >= $30 then
    if (StrLComp(PAnsiChar(pt), 'midp-rms', 8) = 0) then
    begin
      if FileLen >= $48 then
        if (RLInteger(pt + $40) = $48) then
        begin
          fsz := RLInteger(pt + $44);
          i := $48;
        end;
      if (RLInteger(pt + $28) = $30) then
      begin
        fsz := RLInteger(pt + $2C);
        i := $30;
      end;

      if (DContact.hContact = 0) or (DContact.hContact = INVALID_HANDLE_VALUE) then
        if (fContact.hContact <> 0) and (fContact.hContact <> INVALID_HANDLE_VALUE) then
          DContact := fContact;
      if (DContact.hContact <> 0) and (DContact.hContact <> INVALID_HANDLE_VALUE) then
      begin
        proto := GetContactProto(DContact.hContact);
        DoMessage(ITXT_THREAD_DEST_CONTACT, DContact.hContact, 0);
        DoMessage(ITXT_THREAD_START, 0, 0);
        DoMessage(ITXT_THREAD_MAXPROGRESS, 0, fsz);
        while Cardinal(i) < fsz do
        begin
          fillchar(dbei, sizeof(dbei), 0);
          dbei.cbSize := sizeof(dbei);
          dbei.szModule := PAnsiChar(proto);
          rsz := RLInteger(pt + i + $08);
          if PByte(pt + i + $10)^ = $00 then
            dbei.flags := DBEF_READ or DBEF_UTF
          else
            dbei.flags := DBEF_READ or DBEF_UTF or DBEF_SENT;
          // в файлах лежат в utf8 и импортируются без изменений
          dbei.eventType := EVENTTYPE_MESSAGE;
          dbei.cbBlob := RLWord(pt + i + $13 + RLWord(pt + i + $11));
          dbei.pBlob := Pointer(pt + i + $13 + RLWord(pt + i + $11) + $2);
          dbei.timestamp := StrToTimeStamp
            (Pointer(uint_ptr(pt + i) + $13 + RLWord(pt + i + $11) + $4 + dbei.cbBlob),
              RLWord(uint_ptr(pt + i) + $13 + RLWord(pt + i + $11) + $2 + dbei.cbBlob));
          inc(dbei.cbBlob);

          try
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
          finally
            inc(i, rsz);
          end;
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
      s := WideFormat(TranslateWideString('It''s not %s file'), ['midp-rms']);
      DoMessage(ITXT_THREAD_ERROR, wparam(PWideChar(s)), 0);
    end
  else // file len

end;
{$ENDIF}
