{$IFDEF BIN_IMPORT_}

var
  qhfver: byte;
  szMsgHd: word;
  UIDstr: UTF8String;
  UIDLen, NickLen: word;
  NickStr: UTF8String;

function DecodeQHFMsg(ver: byte; adr: integer; cSize: word): AnsiString;

  function get1(c: AnsiChar): byte;
  begin
    if ShortInt(c) >= 0 then
      Result := byte(c)
    else
      Result := byte(c) + 256;
  end;

  function get2(b: byte): AnsiChar;
  begin
    if b < 128 then
      Result := AnsiChar(b)
    else
      Result := AnsiChar(b - 256);
  end;

var
  i: integer;
begin
  SetLength(Result, cSize);
  for i := 1 to cSize do
  begin
    Result[i] := PAnsiChar(adr + i - 1)^;
    if ver <> $01 then
    begin
      inc(Result[i], i);
      Result[i] := get2(255 - get1(Result[i]));
    end
  end;
end;

{$ELSE}

begin
  pt := uint_ptr(pFileText);
  if FileLen >= $2E then
    if (PAnsiChar(pt)^ = 'Q') and (PAnsiChar(pt + 1)^ = 'H') and (PAnsiChar(pt + 2)^ = 'F') then
    begin
      qhfver := PByte(pt + 3)^;
      if qhfver >= $03 then
        szMsgHd := $23
      else
        szMsgHd := $21;
      fsz := RLInteger(pt + $04);

      UIDLen := RLWord(pt + $2C);
      SetLength(UIDstr, UIDLen);
      if qhfver <= 2 then
        lstrcpynA(PAnsiChar(UIDstr), PAnsiChar(pt + $2E), UIDLen)
      else
        lstrcpynA(PAnsiChar(UIDstr), PAnsiChar(pt + $2E), UIDLen + 1);

      NickLen := RLWord(pt + $2E + UIDLen);
      SetLength(NickStr, NickLen);
      if qhfver <= 2 then
        lstrcpynA(PAnsiChar(NickStr), PAnsiChar(pt + $2E + UIDLen + 2), UIDLen)
      else
        lstrcpynA(PAnsiChar(NickStr), PAnsiChar(pt + $2E + UIDLen + 2), UIDLen + 1);

      inc(pt, $2E + UIDLen + 2 + NickLen);
      if fsz <> FileLen - ($2E + UIDLen + 2 + NickLen) then
        fsz := FileLen - ($2E + UIDLen + 2 + NickLen);

      if (DContact.hContact = 0) or (DContact.hContact = INVALID_HANDLE_VALUE)
      then
      begin
        DContact.ContactUID := UIDstr;
        DContact.ContactNick := UTF8Decode{UTF8ToWideString}(NickStr);
        TryDetermContact(DContact);
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
        i := 0;
        while Cardinal(i) < fsz do
        begin
          fillchar(dbei, sizeof(dbei), 0);
          dbei.cbSize := sizeof(dbei);
          dbei.szModule := PAnsiChar(proto);
          dbei.timestamp := RLInteger(pt + i + $12);
          dbei.timestamp := dbei.timestamp -
            (Cardinal(CallService(MS_DB_TIME_TIMESTAMPTOLOCAL, dbei.timestamp, 0)) - dbei.timestamp); // приводим к GMT
          if PByte(pt + i + $1A)^ = $00 then
            dbei.flags := DBEF_READ or DBEF_UTF
          else
            dbei.flags := DBEF_READ or DBEF_UTF or DBEF_SENT;
          // в файлах лежат в utf8 и импортируются без изменений
          dbei.eventType := EVENTTYPE_MESSAGE;
          dbei.cbBlob := RLWord(pt + i + szMsgHd - 2) + 1;
          // размер текста сообщения с #0
          dbei.pBlob := AllocMem(dbei.cbBlob);

          try
            Move(DecodeQHFMsg(qhfver, pt + i + szMsgHd, dbei.cbBlob - 1)[1],
              dbei.pBlob^, dbei.cbBlob - 1);
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
            inc(i, szMsgHd + dbei.cbBlob - 1);
            FreeMem(dbei.pBlob);
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
      s := WideFormat(TranslateWideString('It''s not %s file'), ['QHF']);
      DoMessage(ITXT_THREAD_ERROR, wparam(PWideChar(s)), 0);
    end
  else // file len
end;
{$ENDIF}
