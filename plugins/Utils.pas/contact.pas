{Contact list in combo}
unit contact;

interface

uses windows, m_api;

procedure FillContactList(list:HWND; filter:boolean=true;format:pWideChar=nil);
function FindContact(list:HWND;contact:TMCONTACT):integer;

implementation

uses messages, common, dbsettings, mirutils;

const
  defformat = '%name% - %uid% (%account%:%group%)';

procedure FillContactList(list:HWND; filter:boolean=true;format:pWideChar=nil);
var
  hContact:TMCONTACT;
  buf:array [0..511] of WideChar;
  buf1:array [0..63] of WideChar;
  p:PWideChar;
  uid:pAnsiChar;
  ldbv:TDBVARIANT;
  acc:pAnsiChar;
  lName,
  lGroup,
  lAccount,
  lUID:boolean;
begin
  if format=nil then format:=defformat;

  SendMessage(list,CB_RESETCONTENT,0,0);
  hContact:=db_find_first();

  lName   :=StrPosW(format,'%name%')<>nil;
  lGroup  :=StrPosW(format,'%group%')<>nil;
  lAccount:=StrPosW(format,'%account%')<>nil;
  lUID    :=StrPosW(format,'%uid%')<>nil;

  while hContact<>0 do
  begin
    if ((not filter) and ((IsContactActive(hContact)+1)>=0)) or // + disabled (not deleted)
            (filter  and  (IsContactActive(hContact)   >=0)) then
    begin
      StrCopyW(buf,format);
      if lName then
        StrReplaceW(buf,'%name%',
          PWideChar(CallService(MS_CLIST_GETCONTACTDISPLAYNAME,hContact,GCDNF_UNICODE)));

      if lGroup then
      begin
        p:=DBReadUnicode(hContact,strCList,'Group',nil);
        StrReplaceW(buf,'%group%',p);
        mFreeMem(p);
      end;

      if lAccount then
      begin
        acc:=GetContactProtoAcc(hContact);
        StrReplaceW(buf,'%account%',FastAnsiToWideBuf(acc,buf1));
      end
      else
        acc:=nil;

      if lUID then
      begin
        if acc=nil then
          acc:=GetContactProtoAcc(hContact);
        if IsChat(hContact) then
        begin
          p:=DBReadUnicode(hContact,acc,'ChatRoomID');
          StrReplaceW(buf,'%uid%',p);
          mFreeMem(p);
        end
        else
        begin
          uid:=pAnsiChar(CallProtoService(acc,PS_GETCAPS,PFLAG_UNIQUEIDSETTING,0));
          if uid<>pAnsiChar(CALLSERVICE_NOTFOUND) then
          begin
            if DBReadSetting(hContact,acc,uid,@ldbv)=0 then
            begin
              case ldbv._type of
                DBVT_DELETED: p:='[deleted]';
                DBVT_BYTE   : p:=IntToStr(buf1,ldbv.bVal);
                DBVT_WORD   : p:=IntToStr(buf1,ldbv.wVal);
                DBVT_DWORD  : p:=IntToStr(buf1,ldbv.dVal);
                DBVT_UTF8   : UTF8ToWide(ldbv.szVal.A,p);
                DBVT_ASCIIZ : AnsiToWide(ldbv.szVal.A,p,MirandaCP);
                DBVT_WCHAR  : p:=ldbv.szVal.W;
                DBVT_BLOB   : p:='blob';
              end;
              StrReplaceW(buf,'%uid%',p);
              if ldbv._type in [DBVT_UTF8,DBVT_ASCIIZ] then
                mFreeMem(p);
              DBFreeVariant(@ldbv);
            end;
          end;
          StrReplaceW(buf,'%uid%',nil);
        end;
      end;

      SendMessage(list,CB_SETITEMDATA,
          SendMessageW(list,CB_ADDSTRING,0,tlparam(@buf)),
          hContact);
    end;
    hContact:=db_find_next(hContact);
  end;
end;

function FindContact(list:HWND;contact:TMCONTACT):integer;
var
  j:integer;
begin
  result:=0;
  j:=SendMessage(list,CB_GETCOUNT,0,0);
  while j>0 do
  begin
    dec(j);
    if TMCONTACT(SendMessage(list,CB_GETITEMDATA,j,0))=contact then
    begin
      result:=j;
      break;
    end;
  end;
end;

end.
