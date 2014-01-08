{$IFDEF BIN_IMPORT_}

var
  DS: PDataSource;
  SS: PSession;
  QR1: PQuery;

var
  OneContact: boolean;
  flags: integer;
  timestamp: LongWord;
  Msg: AnsiString;

function FindUIDinDB(too: string): string;
var
  QR2: PQuery;
begin
  QR2 := NewQuery(SS);
  QR2.Text := 'select to, UID from ChatHistory WHERE to=' + too;
  QR2.Open;
  QR2.First;
  result := QR2.FieldByNameAsStr['UID'];
  QR2.Free;
end;

{$ELSE}

begin
  DoUnMapFile;
  OneContact := (DContact.hContact <> 0) and (DContact.hContact <> INVALID_HANDLE_VALUE);
  try
    DS := NewDataSource('Provider=Microsoft.Jet.OLEDB.4.0;User ID=Admin;' +
      'Data Source=' + FileName +
      ';Mode=Share Deny None;' +
      'Extended Properties="";' +
      'Locale Identifier=1033;' +
      'Persist Security Info=False;');
    SS := NewSession(DS);
    QR1 := NewQuery(SS);
    QR1.Text := 'SELECT Messages.from, date, to, type, subType, subject FROM Messages';
    QR1.Open;
    if (QR1.ColCount = 7) then
    begin
      DoMessage(ITXT_THREAD_START, 0, 0);
      QR1.First;
      DoMessage(ITXT_THREAD_MAXPROGRESS, 0, QR1.RowCount);
      While not QR1.EOF do
      begin
        try
          if (QR1.SFieldByName['type'] = 'Text') and
             (QR1.SFieldByName['subType'] = 'IM') then
          begin
            UIDStr := FindUIDinDB(QR1.FieldByNameAsStr['to']);
            if not OneContact then
            begin
              DContact.ContactUID := UIDStr;
              TryDetermContact(DContact);
            end;
            if (DContact.hContact <> 0) and
              (DContact.hContact <> INVALID_HANDLE_VALUE) then
            begin
              if QR1.SField[1] = '' then
                flags := DBEF_READ or DBEF_UTF or DBEF_SENT
              else
                flags := DBEF_READ or DBEF_UTF;
              timestamp := DateTimeToTimeStamp(QR1.DFieldByName['date'] - 693594);
              Msg := QR1.SFieldByName['subject'];
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
        DoMessage(ITXT_THREAD_PROGRESS, QR1.CurIndex, 0);
      end; // for
    end
    else
    begin
      s := WideFormat(TranslateWideString('It''s not %s file'), ['ICQ6 mdb']);
      DoMessage(ITXT_THREAD_ERROR, wparam(PWideChar(s)), 0);
    end;
  finally
    DS.Free;
    DS := nil;
    SS := nil;
    QR1 := nil;
  end;
end;
{$ENDIF}
