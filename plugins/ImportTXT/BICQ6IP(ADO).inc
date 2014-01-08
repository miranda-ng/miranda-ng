{$IFDEF BIN_IMPORT_}

var
  QR1: TADOQuery;

var
  OneContact: boolean;
  flags: integer;
  timestamp: LongWord;
  Msg: AnsiString;
  ADOConnection:TADOConnection;

function FindUIDinDB(too: string): string;
var
  QR2: TADOQuery;
begin
  QR2 := TADOQuery.Create(nil);
  QR2.Connection := ADOConnection;
  QR2.SQL.Text := 'select to, UID from ChatHistory WHERE to=' + too;
  QR2.Open;
  QR2.First;
  result := QR2.FieldByName('UID').AsString;
  QR2.Close;
  QR2.Free;
end;

{$ELSE}

begin
  DoUnMapFile;
  OneContact := (DContact.hContact <> 0) and (DContact.hContact <> INVALID_HANDLE_VALUE);
  try
//    coInitialize(nil);
    ADOConnection:=TADOConnection.Create(nil);
    ADOConnection.Connected:=false;
    ADOConnection.LoginPrompt:=false;
    ADOConnection.Provider:='Microsoft.Jet.OLEDB.4.0';
    ADOConnection.ConnectionString :=
      'User ID=Admin;' +
      'Data Source=' + FileName +
      ';Mode=Share Deny None;' +
      'Extended Properties="";' +
      'Locale Identifier=1033;' +
      'Persist Security Info=False;';

    ADOConnection.Connected:=true;

    QR1 := TADOQuery.Create(nil);
    QR1.Connection := ADOConnection;
    QR1.SQL.Text := 'SELECT Messages.from, date, to, type, subType, subject FROM Messages';
    QR1.Open;
    if (QR1.FieldCount = 7) then
    begin
      DoMessage(ITXT_THREAD_START, 0, 0);
      QR1.First;
      DoMessage(ITXT_THREAD_MAXPROGRESS, 0, QR1.RecordCount);
      While not QR1.EOF do
      begin
        try
          if (QR1.FieldByName('type').AsString = 'Text') and
             (QR1.FieldByName('subType').AsString = 'IM') then
          begin
            UIDStr := FindUIDinDB(QR1.FieldByName('to').AsString);
            if not OneContact then
            begin
              DContact.ContactUID := UIDStr;
              TryDetermContact(DContact);
            end;
            if (DContact.hContact <> 0) and
              (DContact.hContact <> INVALID_HANDLE_VALUE) then
            begin
              if QR1.Fields[1].AsString = '' then
                flags := DBEF_READ or DBEF_UTF or DBEF_SENT
              else
                flags := DBEF_READ or DBEF_UTF;
              timestamp := DateTimeToTimeStamp(QR1.FieldByName('date').AsDateTime - 693594);
              Msg := QR1.FieldByName('subject').AsString;
              tempstr := ANSIToUTF8(PAnsiChar(Msg), tempstr, cp);
              Msg := tempstr;
              FreeMem(tempstr);
              AddMsgToDB(DContact.hContact, flags, timestamp, Msg, AddedMessages, Duplicates);
            end;
          end;
        except
          ShowException(ExceptObject, ExceptAddr)
        end;
        QR1.Next;
        DoMessage(ITXT_THREAD_PROGRESS, QR1.RecNo, 0);
      end; // for
    end
    else
    begin
      s := WideFormat(TranslateWideString('It''s not %s file'), ['ICQ6 mdb']);
      DoMessage(ITXT_THREAD_ERROR, wparam(PWideChar(s)), 0);
    end;
  finally
    QR1.Close;
    QR1.Free;
    ADOConnection.Close;
    ADOConnection.Free;
  end;
end;
{$ENDIF}
