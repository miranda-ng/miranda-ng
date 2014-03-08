{$IFDEF BIN_IMPORT_}

var
  XI: TXML_API_W;
  rhxml: HXML;
  curxml: HXML;
  chldxml: HXML;
  ptxt: PWideChar;
  txtlen: int;
  ic, tm: integer;
  res: LongBool;

{$ELSE}

begin
  If ServiceExists(MS_SYSTEM_GET_XI) > 0 then
  begin
    XI.cbSize := SizeOf(TXML_API_W);
    res := LongBool(CallService(MS_SYSTEM_GET_XI, 0, int(@XI)));
    if not res then
    begin
      XI.cbSize := SizeOf(TXML_API_W) - 4;
      res := LongBool(CallService(MS_SYSTEM_GET_XI, 0, int(@XI)));
      if not res then
      begin
        s := TranslateWideString('Error at initialization XML parser');
        DoMessage(ITXT_THREAD_ERROR, wparam(PWideChar(s)), 0);
        exit;
      end;
    end;
  end
  else
  begin
    s := TranslateWideString('Your version of Miranda don''t support XML parsing');
    DoMessage(ITXT_THREAD_ERROR, wparam(PWideChar(s)), 0);
    exit;
  end;
  if (DContact.hContact = 0) or (DContact.hContact = INVALID_HANDLE_VALUE) then
    if (fContact.hContact <> 0) and (fContact.hContact <> INVALID_HANDLE_VALUE) then
      DContact := fContact;
  if (DContact.hContact <> 0) and (DContact.hContact <> INVALID_HANDLE_VALUE) then
  begin
    proto := GetContactProto(DContact.hContact);
    DoMessage(ITXT_THREAD_DEST_CONTACT, DContact.hContact, 0);
    DoMessage(ITXT_THREAD_START, 0, 0);
    GetMem(ptxt, SizeOf(WideChar) * FileLen + 2);
    try
      ptxt := UTF8toWide(pFileText, ptxt, SizeOf(WideChar) * FileLen + 2);
      ptxt[FileLen + 1] := #0000;
      rhxml := XI.parseString(ptxt, @txtlen, 'root');
    finally
      freemem(ptxt);
    end;
    if (rhxml = 0) then
      exit;
    DoMessage(ITXT_THREAD_MAXPROGRESS, 0, XI.getChildCount(rhxml));
    chldxml := XI.getFirstChild(rhxml);
    if XI.getName(chldxml) = 'version' then
    begin
      ic := 0;
      curxml := XI.getNextChild(rhxml, 'event', @ic);
      while (curxml <> 0) do
      begin
        fillchar(dbei, SizeOf(dbei), 0);
        dbei.cbSize := SizeOf(dbei);
        dbei.szModule := PAnsiChar(proto);
        if TryStrToInt(XI.getText(XI.getChildByPath(curxml, 'time', false)), tm)
        then
          dbei.timestamp := tm;
        if XI.getText(XI.getChildByPath(curxml, 'incoming', false)) = 'Yes' then
          dbei.flags := DBEF_READ or DBEF_UTF
        else
          dbei.flags := DBEF_READ or DBEF_UTF or DBEF_SENT;

        if XI.getText(XI.getChildByPath(curxml, 'type', false))^ = '2' then
        begin
          s := XI.getText(XI.getChildByPath(curxml, 'text', false)) + ': ' +
            XI.getText(XI.getChildByPath(curxml, 'data', false));
          dbei.eventType := EVENTTYPE_URL;
        end
        else
        begin
          s := XI.getText(XI.getChildByPath(curxml, 'text', false));
          dbei.eventType := EVENTTYPE_MESSAGE;
        end;
        try
          tempstr := WidetoUTF8(PWideChar(s), tempstr);
          dbei.cbBlob := lstrlena(tempstr) + 1;
          dbei.pBlob := PByte(tempstr);
          if not IsDuplicateEvent(DContact.hContact, dbei) then
            if db_event_add(wParam(DContact.hContact), @dbei) <> 0 then
              Inc(AddedMessages)
            else
            begin
              s := 'Error adding message to database';
              DoMessage(ITXT_THREAD_ERROR, wparam(PWideChar(s)), 0);
              break;
            end

          else
            Inc(Duplicates);

        finally
          freemem(tempstr);
          curxml := XI.getNextChild(rhxml, 'event', @ic);
          DoMessage(ITXT_THREAD_PROGRESS, ic, 0);
        end;
      end;
    end
    else
    begin
      s := WideFormat(TranslateWideString('It''s not %s file'), ['ICQ5']);
      DoMessage(ITXT_THREAD_ERROR, wparam(PWideChar(s)), 0);
    end;
  end
  else
  begin
    s := TranslateWideString('Can''t determine destination contact');
    DoMessage(ITXT_THREAD_ERROR, wparam(PWideChar(s)), 0);
  end;
end;
{$ENDIF}
