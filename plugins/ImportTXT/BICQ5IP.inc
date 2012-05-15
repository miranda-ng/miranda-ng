{$ifdef BIN_IMPORT_}
var
    XI:XML_API_W;
    rhxml:HXML;
    curxml:HXML;
    chldxml:HXML;
    ptxt:PWideChar;
    txtlen:int;
    ic,tm:integer;
    res:LongBool;

{$else}
begin
  If pluginLink^.ServiceExists(MS_SYSTEM_GET_XI)>0 then
   begin
    XI.cbSize:=SizeOf(XML_API_W);
    res:=longbool(pluginLink^.CallService(MS_SYSTEM_GET_XI,0,Int(@XI)));
    if not res then
     begin
      XI.cbSize:=SizeOf(XML_API_W)-4;
      res:=longbool(pluginLink^.CallService(MS_SYSTEM_GET_XI,0,Int(@XI)));
      if not res then
       begin
        s:=TranslateWideString('Error at initialization XML parser');
        DoMessage(ITXT_THREAD_ERROR,integer(PWideChar(s)),0);
        exit;
       end;
     end;
   end
                                                   else
   begin
    s:=TranslateWideString('Your version of Miranda don''t support XML parsing');
    DoMessage(ITXT_THREAD_ERROR,integer(PWideChar(s)),0);
    exit;
   end;
  if (DContact.hContact=0) or (DContact.hContact=INVALID_HANDLE_VALUE) then
   if (fContact.hContact<>0) and (fContact.hContact<>INVALID_HANDLE_VALUE) then
       DContact:=fContact;
  if (DContact.hContact<>0) and (DContact.hContact<>INVALID_HANDLE_VALUE) then
   begin
    proto:=GetContactProto(DContact.hContact);
    DoMessage(ITXT_THREAD_DEST_CONTACT,DContact.hContact,0);
    DoMessage(ITXT_THREAD_START,0,0);
    GetMem(ptxt,SizeOf(WideChar)*FileLen+2);
    try
    ptxt:=UTF8toWide(pFileText,ptxt,SizeOf(WideChar)*FileLen+2);
    ptxt[FileLen+1]:=#0000;
    rhxml:=XI.parseString(ptxt,@txtlen,'root');
    finally
    freemem(ptxt);
    end;
    if (rhxml=0) then exit;
    DoMessage(ITXT_THREAD_MAXPROGRESS,0,xi.getChildCount(rhxml));
    chldxml:=xi.getFirstChild(rhxml);
    if xi.getName(chldxml)='version' then
      begin
       ic:=0;
       curxml:=xi.getNextChild(rhxml,'event',@ic);
       while (curxml<>0) do
        begin
          fillchar(dbei,sizeof(dbei),0);
          dbei.cbSize:=sizeof(dbei);
          dbei.szModule:=PChar(proto);
          if TryStrToInt(xi.getText(xi.getChildByPath(curxml,'time',false)),tm) then dbei.timestamp:=tm;
          if xi.getText(xi.getChildByPath(curxml,'incoming',false))='Yes' then dbei.flags:=DBEF_READ
                                                                          else dbei.flags:=DBEF_SENT  or DBEF_READ;
          if isMirandaUnicode then dbei.flags:=dbei.flags or DBEF_UTF;

          if xi.getText(xi.getChildByPath(curxml,'type',false))^='2' then
           begin
            s:=xi.getText(xi.getChildByPath(curxml,'text',false))+': '+
               xi.getText(xi.getChildByPath(curxml,'data',false));
            dbei.eventType:= EVENTTYPE_URL;
           end
                                                                    else
           begin
            s:=xi.getText(xi.getChildByPath(curxml,'text',false));
            dbei.eventType:= EVENTTYPE_MESSAGE;
           end;
          try
          if IsMirandaUnicode then tempstr:=WidetoUTF8(PWideChar(s),tempstr)
                              else tempstr:=WideToANSI(PWideChar(s),tempstr,cp);
          dbei.cbBlob:=lstrlen(tempstr)+1;
          dbei.pBlob:=PByte(tempstr);
          if not IsDuplicateEvent(DContact.hContact,dbei) then
           if pluginLink^.CallService(MS_DB_EVENT_ADD, wParam(DContact.hContact), lParam(@dbei))<>0 then Inc(AddedMessages)
                                                                                                    else begin
                                                          s:= 'Error adding message to DB';
                                                          DoMessage(ITXT_THREAD_ERROR,integer(PWideChar(s)),0);  break; end

                                                          else Inc(Duplicates);

          finally
          FreeMem(tempstr);
          curxml:=xi.getNextChild(rhxml,'event',@ic);
          DoMessage(ITXT_THREAD_PROGRESS,ic,0);
          end;
        end;
      end
                                     else
      begin
       s:=WideFormat(TranslateWideString('Its not %s file'),['ICQ5']);
       DoMessage(ITXT_THREAD_ERROR,integer(PWideChar(s)),0);
      end;
   end
                                                                             else
   begin
    s:=TranslateWideString('Can''t determine destination contact');
    DoMessage(ITXT_THREAD_ERROR,integer(PWideChar(s)),0);
   end;
end;
{$endif}