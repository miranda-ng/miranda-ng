{}
//----- Color functions -----

var
  colorhook:THANDLE;

type
  tqscolor = (
    bkg_norm,fgr_norm,
    bkg_odd ,fgr_odd,
    bkg_dis ,fgr_dis,
    bkg_del ,fgr_del,
    bkg_hid ,fgr_hid,
    bkg_meta,fgr_meta,
    bkg_sub ,fgr_sub
  );
type
  tQSColorRec = record
    color   : TCOLORREF;
    setting : PAnsiChar;
    descr   : PAnsiChar;
  end;
const
  QSColors: array [tqscolor] of tQSColorRec =(
    (color: $00FFFFFF; setting: 'back_norm'; descr: 'Normal background'          ),
    (color: $00000000; setting: 'fore_norm'; descr: 'Normal foreground'          ),
    (color: $00EBE6DE; setting: 'back_odd' ; descr: 'Odd background'             ),
    (color: $00000000; setting: 'fore_odd' ; descr: 'Odd foreground'             ),
    (color: $008080FF; setting: 'back_dis' ; descr: 'Disabled account background'),
    (color: $00000000; setting: 'fore_dis' ; descr: 'Disabled account foreground'),
    (color: $008000FF; setting: 'back_del' ; descr: 'Deleted account background' ),
    (color: $00000000; setting: 'fore_del' ; descr: 'Deleted account foreground' ),
    (color: $0080FFFF; setting: 'back_hid' ; descr: 'Hidden contact background'  ),
    (color: $00000000; setting: 'fore_hid' ; descr: 'Hidden contact foreground'  ),
    (color: $00BAE699; setting: 'back_meta'; descr: 'Metacontact background'     ),
    (color: $00000000; setting: 'fore_meta'; descr: 'Metacontact foreground'     ),
    (color: $00B3CCC1; setting: 'back_sub' ; descr: 'Subcontact background'  ),
    (color: $00000000; setting: 'fore_sub' ; descr: 'Subcontact foreground'  )
  );


procedure RegisterColors;
var
  cid:TColourID;
  i:tqscolor;
begin
  cid.cbSize:=SizeOf(cid);
  cid.flags :=0;
  StrCopy(cid.group,qs_module);
  StrCopy(cid.dbSettingsGroup,qs_module);

  for i:=Low(tqscolor) to High(tqscolor) do
  begin
    StrCopy(cid.name   ,QSColors[i].descr);
    StrCopy(cid.setting,QSColors[i].setting);
    cid.defcolour:=QSColors[i].color;
    cid.order    :=ORD(i);
    ColourRegister(@cid);
  end;
end;

function ColorReload(wParam:WPARAM;lParam:LPARAM):int;cdecl;
var
  cid:TColourID;
  i:tqscolor;
begin
  result:=0;
  cid.cbSize:=SizeOf(cid);
  StrCopy(cid.group,qs_module);
  for i:=Low(tqscolor) to High(tqscolor) do
  begin
    StrCopy(cid.name ,QSColors[i].descr);
    QSColors[i].color:=CallService(MS_COLOUR_GETA,tlparam(@cid),0);
  end;
end;

//----- Item fill -----

function int2strw(i:uint_ptr;signed:bool=false):PWideChar;
var
  buf:array [0..31] of WideChar;
begin
  if signed then
    StrDupW(result,IntToStr(buf,int_ptr(i)))
  else
    StrDupW(result,IntToStr(buf,i));
end;

function int2hexw(i:uint_ptr):PWideChar;
var
  buf:array [0..31] of WideChar;
begin
  StrDupW(result,IntToHex(buf,i));
end;

function BuildLastSeenTime(date:integer):PWideChar;
var
  pc:pWideChar;
  buf:array [0..19] of WideChar;
  year,month,day,hours,min:integer;
begin
  year:=(date div (60*24*31*356))+1980;

  if year<>0 then
  begin
    date:= date mod (60*24*31*356);
    pc:=@buf;

    month:=date div (60*24*31);
    date :=date mod (60*24*31);
    day  :=date div (60*24);
    date :=date mod (60*24);
    hours:=date div 60;
    min  :=date mod 60;

    IntToStr(pc,day,2);

    inc(pc,2);
    pc^:='.'; inc(pc);
    IntToStr(pc,month,2);
    inc(pc,2);
    pc^:='.'; inc(pc);
    IntToStr(pc,year,4);
    inc(pc,4);
    pc^:=' '; inc(pc);
    pc^:='-'; inc(pc);
    pc^:=' '; inc(pc);
    IntToStr(pc,hours,2);
    inc(pc,2);
    pc^:=':'; inc(pc);
    IntToStr(pc,min,2);

    StrDupW(result,@buf);
  end
  else
    result:=nil;
end;

function BuildLastSeenTimeInt(cont:THANDLE;modulename:PAnsiChar):cardinal;
var
  Day,Month,Year,Hours,Minutes:word;
begin
  Year:=DBReadWord(cont,modulename,'Year',0);
  if Year<>0 then
  begin
    Month  :=DBReadWord(cont,modulename,'Month'  ,0);
    Day    :=DBReadWord(cont,modulename,'Day'    ,0);
    Hours  :=DBReadWord(cont,modulename,'Hours'  ,0);
    Minutes:=DBReadWord(cont,modulename,'Minutes',0);
    result:=Minutes+Hours*60+Day*60*24+Month*60*24*31+(Year-1980)*60*24*31*356; // was 366
  end
  else
    result:=0;
end;

function IPtoStr(ip:dword):PWideChar;
var
  p:PWideChar;
  buf:array [0..16] of WideChar;
begin
  p:=@buf;
  IntToStr(buf,ip shr 24);
  while p^<>#0 do inc(p); p^:='.'; inc(p);
  IntToStr(p,(ip shr 16) and $FF);
  while p^<>#0 do inc(p); p^:='.'; inc(p);
  IntToStr(p,HIByte(ip));
  while p^<>#0 do inc(p); p^:='.'; inc(p);
  IntToStr(p,LOByte(ip));
  StrDupW(result,buf);
end;

function TimeToStrW(data:dword):PWideChar;
var
  strdatetime:array [0..63] of WideChar;
  dbtts:TDBTIMETOSTRING;
begin
  dbtts.cbDest    :=sizeof(strdatetime);
  dbtts.szDest.w  :=@strdatetime;
  dbtts.szFormat.w:='d - t';
  CallService(MS_DB_TIME_TIMESTAMPTOSTRINGT,data,lparam(@dbtts));
  StrDupW(result,strdatetime);
end;

function FindMeta(hMeta:THANDLE;var MetaNum:WPARAM):LPARAM;
var
  i:integer;
begin
  result:=0;

  for i:=0 to HIGH(FlagBuf) do
  begin
    with FlagBuf[i] do
    begin
      if contact=hMeta then
      begin
        if wparam=0 then // new meta
        begin
          inc(LastMeta);
          wparam :=LastMeta;
          lparam :=0;
        end;
        MetaNum:=wparam;
        inc(lparam);
        result:=lparam;
        break;
      end;
    end;
  end;

end;

function DoMeta(hContact:TMCONTACT):pointer;
var
  pw:pWideChar;
  i:integer;
begin
  result:=nil;

  for i:=0 to HIGH(FlagBuf) do
  begin
    with FlagBuf[i] do
    begin
      if contact=hContact then
      begin
        if (flags and QSF_META)<>0 then // adding new meta count
        begin
          if wparam=0 then
          begin
            inc(LastMeta);
            wparam:=LastMeta;
//            lparam:=0;
          end;
        end
        else if (flags and QSF_SUBMETA)<>0 then
        begin
          lparam:=FindMeta(db_mc_getMeta(hContact),wparam);
        end;

        if wparam>0 then
        begin
          mGetMem(result,32);
          pw:=result;
          pw[0]:='[';
          IntToStr(pw+1,wparam,3);
          pw[4]:=']';
          if lparam>0 then
          begin
            pw[5]:=' ';
            IntToStr(pw+6,lparam);
          end
          else
            pw[5]:=#0;
        end;
        break;
      end;
    end;
  end;

end;

procedure LoadOneItem(hContact:THANDLE;column:pcolumnitem;proto:integer; var res:tQSRec);
var
  tmp:int_ptr;
  lmodule,srv:PAnsiChar;
  DbEvent:HDBEVENT;
  cni:TCONTACTINFO;
  dbei:TDBEVENTINFO;
  b:bool;
begin
  res.data:=uint_ptr(-1);
  mFreeMem(res.text);
  with column^ do
  begin

    case setting_type of
      QST_SCRIPT: begin
        res.text:=ParseVarString(script,hContact);
      end;

      QST_SERVICE: begin
        if wparam._type=ACF_CURRENT then wparam.value:=hContact;
        if lparam._type=ACF_CURRENT then lparam.value:=hContact;

        if (restype and ACF_SCRIPT_SERVICE)<>0 then
          srv:=ParseVarString(service,hContact,nil)
        else
          srv:=service;
        tmp:=int_ptr(CallService(srv,TWPARAM(wparam.value),TLPARAM(lparam.value)));
        if (restype and ACF_SCRIPT_SERVICE)<>0 then
          mFreeMem(srv);

        if tmp=CALLSERVICE_NOTFOUND then exit;
        if (restype and ACF_RSTRING)<>0 then
          AnsiToWide(PAnsiChar(tmp),res.text)
        else if (restype and ACF_RUNICODE)<>0 then
          StrDupW(res.text,PWideChar(tmp))
        else// if (restype and ACF_RNUMBER)<>0 then
        begin
          res.data:=tmp;
          if (restype and ACF_RHEXNUM)<>0 then
            res.text:=int2hexw(tmp)
          else
          begin
            b:=(restype and ACF_RSIGNED)<>0;
            res.text:=int2strw(tmp,b);
          end;
        end;
      end;

      QST_CONTACTINFO: begin
        FillChar(cni,SizeOf(cni),0);
        cni.cbSize  :=sizeof(cni);
        cni.dwFlag  :=cnftype or CNF_UNICODE;
        cni.hContact:=hContact;
        cni.szProto :=GetProtoName(proto);
        if CallService(MS_CONTACT_GETCONTACTINFO,0,tlparam(@cni))=0 then
        begin
          case cni._type of
            CNFT_ASCIIZ: begin
              if cni.retval.szVal.w<>nil then
              begin
                StrDupW(res.text,cni.retval.szVal.w);
                mir_free(cni.retval.szVal.w);
              end;
              exit;
            end;
            CNFT_BYTE :begin
              res.data:=cni.retval.bVal;
              if cnftype=CNF_GENDER then
              begin
                if not (res.data in [70,77]) then
                  res.data:=DBReadByte(hContact,'UserInfo','Gender',0);
                exit;
              end
            end;
            CNFT_WORD :res.data:=cni.retval.wVal;
            CNFT_DWORD:res.data:=cni.retval.dVal;
          end;
          res.text:=int2strw(res.data);
        end;
      end;

      QST_SETTING: begin
        if module<>nil then
          lmodule:=module
        else
          lmodule:=GetProtoName(proto);

        case datatype of
          QSTS_STRING: begin
            res.text:=DBReadUnicode(hContact,lmodule,setting,nil)
          end;

          QSTS_BYTE: begin
            res.data:=DBReadByte(hContact,lmodule,setting,0);
            res.text:=int2strw(res.data);
          end;

          QSTS_WORD: begin
            res.data:=DBReadWord(hContact,lmodule,setting,0);
            res.text:=int2strw(res.data);
          end;

          QSTS_DWORD: begin
            if (module=nil) and (setting=nil) then
            begin
              res.data:=hContact;
              res.text:=int2hexw(res.data);
            end
            else
            begin
              res.data:=DBReadDWord(hContact,lmodule,setting,0);
              res.text:=int2strw(res.data);
            end;
          end;

          QSTS_SIGNED: begin
            res.data:=DBReadDWord(hContact,lmodule,setting,0);
            res.text:=int2strw(res.data,true);
          end;

          QSTS_HEXNUM: begin
            res.data:=DBReadDWord(hContact,lmodule,setting,0);
            res.text:=int2hexw(res.data);
          end;

          QSTS_IP: begin
            res.data:=DBReadDWord(hContact,lmodule,setting,0);
            if res.data<>0 then
              res.text:=IPtoStr(res.data);
          end;

          QSTS_TIMESTAMP: begin
            res.data:=DBReadDWord(hContact,lmodule,setting,0);
            if res.data<>0 then
              res.text:=TimeToStrW(res.data);
          end;
        end;
      end;

      QST_OTHER: case other of
        QSTO_LASTSEEN: begin
          res.data:=BuildLastSeenTimeInt(hContact,'SeenModule');
          res.text:=BuildLastSeenTime   (res.data);
        end;

        QSTO_LASTEVENT: begin
          DbEvent:=db_event_last(hContact);
          if DbEvent<>0 then
          begin
            ZeroMemory(@dbei,sizeof(dbei));
            dbei.cbSize:=SizeOf(dbei);
            db_event_get(DbEvent, @dbei);
            res.data:=dbei.timestamp;
            res.text:=TimeToStrW(res.data);
          end
          else
            res.data:=0;
        end;

        QSTO_METACONTACT: begin
          res.text:=DoMeta(hContact);
        end;

        QSTO_EVENTCOUNT: begin
          res.text:=int2strw(db_event_count(hContact));
        end;
      end;

    end;
  end;
end;

//----- Initial table filling -----

procedure AddContact(num:integer;hContact:TMCONTACT);
var
  col:pcolumnitem;
  tmpstr:array [0..63] of AnsiChar;
  i:integer;
begin
  FillChar(FlagBuf[num],SizeOf(tQSFRec),0);
  with FlagBuf[num] do
  begin
    contact:=hContact;
    flags  :=0;
    i:=IsContactActive(hContact,tmpstr);
    proto:=FindProto(tmpstr);

    case i of
      -2: flags:=flags or QSF_ACCDEL;  // deleted account
      -1: flags:=flags or QSF_ACCOFF;  // disabled account
//      0 : ; // hidden contact
      1 : flags:=flags or QSF_META;    // metacontact
      2 : flags:=flags or QSF_SUBMETA; // subMetacontact
    end;
    if i>0 then
      flags:=flags or QSF_INLIST;      // normal contact

    if (proto=0) or (i<0) then
      status:=ID_STATUS_OFFLINE
    else
      status:=DBReadWord(contact,GetProtoName(proto),'Status',ID_STATUS_OFFLINE);

    for i:=0 to qsopt.numcolumns-1 do
    begin
      col:=@qsopt.columns[i];
      // col.flags must me same as colorder[i].flags
      if (col.flags and COL_ON)<>0 then
        LoadOneItem(contact,col,proto,MainBuf[num,i]);
    end;
  end;

end;

function PrepareToFill:boolean;
var
  cnt,cnt1:integer;
  hContact:TMCONTACT;
  i:integer;
begin
  result:=false;
  if qsopt.numcolumns=0 then
    exit;
  // calculating contacts
  cnt:=CallService(MS_DB_CONTACT_GETCOUNT,0,0);
  if cnt=0 then
    exit;

  result:=true;

  // Allocate mem
  SetLength(MainBuf,cnt,qsopt.numcolumns);
  SetLength(FlagBuf,cnt);

  for i:=0 to cnt-1 do
    FillChar(MainBuf[i][0],qsopt.numcolumns*SizeOf(tQSRec),0);

  for i:=0 to qsopt.numcolumns-1 do
  begin
    with qsopt.columns[i] do
    begin
      if (flags and COL_ON)<>0 then
        flags := flags or COL_INIT;
    end;
  end;

  // filling buffer
  LastMeta:=0;
  cnt1:=0;
  hContact:=db_find_first();
  while hContact<>0 do
  begin
    //!! check account
    AddContact(cnt1,hContact);
    inc(cnt1);
    if cnt1=cnt then break; // additional checking
    hContact:=db_find_next(hContact);
  end;

end;

//----- Status bar -----
type
  pSBDataRecord = ^tSBDataRecord;
  tSBDataRecord = record
    flags :cardinal;
    total :cardinal; // in clist
    found :cardinal; // by pattern
    online:cardinal; // clist online
    liston:cardinal; // pattern online
  end;
  tSBData = array [0..63] of tSBDataRecord;

procedure DrawSBW(const SBData:tSBData);
var
  aPartPos:array [0..63 ] of integer;
  buf     :array [0..255] of WideChar;
  fmtstr  :array [0..255] of WideChar;
  all:integer;
  i,j:integer;
  p,pc,po,pd,poff,pa:PWideChar;
  rc:TRECT;
  dc:HDC;
  icon:HICON;
  protocnt:integer;
begin
  p:=@buf;
  // p:=FormatSimpleW('%i users found (%i) Online: %i',[SBData[0].found,Length(FlagBuf),SBData[0].online]);
  p:=StrEndW(IntToStr(p,SBData[0].found));
  p:=StrCopyEW(p,TranslateW(' users found ('));
  p:=StrEndW(IntToStr(p,Length(FlagBuf)));
  p:=StrCopyEW(p,TranslateW(') Online: '));
  IntToStr(p,SBData[0].online);

  dc:=GetDC(StatusBar);
  DrawTextW(dc,pWidechar(@buf),-1,rc,DT_CALCRECT);
  ReleaseDC(StatusBar,dc);
  all:=rc.right-rc.left;
  aPartPos[0]:=all;
  protocnt:=GetNumProto;
  i:=1;
  while i<=protocnt do
  begin
    inc(all,55);
    aPartPos[i]:=all;
    inc(i);
  end;
  aPartPos[i]:=-1;
  SendMessageW(StatusBar,SB_SETPARTS,protocnt+2,lparam(@aPartPos));
  SendMessageW(StatusBar,SB_SETTEXTW,0,lparam(@buf){p});
  // mFreeMem(p);

  po  :=TranslateW('Online');
  pd  :=TranslateW('deleted');
  poff:=TranslateW('off');
  pa  :=TranslateW('active');

  for i:=1 to protocnt do
  begin
    if ((SBData[i].flags and (QSF_ACCDEL or QSF_ACCOFF))<>0) then
    begin
      icon:=CallService(MS_SKIN_LOADPROTOICON,0,ID_STATUS_OFFLINE);
    end
    else
    begin
      icon:=CallService(MS_SKIN_LOADPROTOICON,wparam(GetProtoName(i)),ID_STATUS_ONLINE);
    end;

    FastAnsiToWideBuf(GetProtoName(i),fmtstr);

    SendMessageW(StatusBar,SB_SETICON,i,icon);

    j:=High(buf);//(SizeOf(buf) div SizeOf(WideChar))-1;
    buf[j]:=#0;

    // fill by spaces
    p:=@buf[0];
    while j>0 do
    begin
      dec(j);
      p^:=' ';
      inc(p);
    end;

    if (SBData[i].flags and QSF_ACCDEL)<>0 then
    begin
      buf [0]:='!';
      pc:=pd;
    end
    else if (SBData[i].flags and QSF_ACCOFF)<>0 then
    begin
      buf [0]:='?';
      pc:=poff
    end
    else
      pc:=pa;

    IntToStr(pWideChar(@buf[2]),SBData[i].found);
    StrEndW(buf)^:=' ';
    SendMessageW(StatusBar,SB_SETTEXTW,i,lparam(@buf));

// create tooltip
//  FormatSimpleW('%s (%s): %i (%i); %s %i (%i)',
//                [fmtstr,pc,found,total,po,liston,online]);
    p:=@buf;
    p:=StrCopyEW(p,fmtstr); // Protocol
    p^:=' '; inc(p);
    p^:='('; inc(p);
    p:=StrCopyEW(p,pc);     // Protocol status
    p^:=')'; inc(p);
    p^:=':'; inc(p);
    p^:=' '; inc(p);

    with SBData[i] do
    begin
      p:=StrEndW(IntToStr(p,found));
      p^:=' '; inc(p);
      p^:='('; inc(p);
      p:=StrEndW(IntToStr(p,total));
      p^:=')'; inc(p);
      p^:=';'; inc(p);
      p^:=' '; inc(p);
      p:=StrCopyEW(p,po);
      p^:=' '; inc(p);
      p:=StrEndW(IntToStr(p,liston));
      p^:=' '; inc(p);
      p^:='('; inc(p);
      p:=StrEndW(IntToStr(p,online));
      p^:=')'; inc(p);
    end;
    p^:=#0;
    SendMessageW(StatusBar,SB_SETTIPTEXTW,i,lparam(@buf));
  end;

end;

procedure UpdateSB;
var
  SBData: tSBData;
  j:integer;
  p:pSBDataRecord;
begin
  FillChar(SBData,SizeOf(SBData),0);

  // for all contacts
  for j:=0 to HIGH(FlagBuf) do
  begin
    p:=@SBData[FlagBuf[j].proto];
    p^.flags:=FlagBuf[j].flags;

    inc(p^.total);

    if (p^.flags and QSF_ACTIVE)<>0 then
    begin
      inc(p^.found);
      inc(SBData[0].found);
    end;

    if FlagBuf[j].status<>ID_STATUS_OFFLINE then
    begin
      inc(p^.online);
      inc(SBData[0].online);
      if (p^.flags and QSF_ACTIVE)<>0 then
      begin
        inc(p^.liston);
        inc(SBData[0].liston);
      end;
    end;

  end;

  DrawSBW(SBData);
end;

//----- Patterns -----

const
  pattern:pWideChar = nil; // edit field text
const
  maxpattern = 8;
var
  patterns:array [0..maxpattern-1] of record
    str:PWideChar;
    res:bool;
  end;
const
  patstr:PWideChar=nil; // work pattern buffer
  numpattern:integer=0;

procedure MakePatternW;
var
  lpatptr:PWideChar;
  wasquote:bool;
begin
  numpattern:=0;
  mFreeMem(patstr);
  if (pattern<>nil) and (pattern^<>#0) then
  begin
    wasquote:=false;
    StrDupW(patstr,pattern);
    lpatptr:=patstr;
    repeat
      while lpatptr^=' ' do inc(lpatptr);
      if lpatptr^<>#0 then
      begin
        if lpatptr^='"' then
        begin
          inc(lpatptr);
          wasquote:=true;
        end
        else
        begin
          patterns[numpattern].str:=lpatptr;
          inc(numpattern);
          while lpatptr^<>#0 do
          begin
            if wasquote then
            begin
              if lpatptr^='"' then
              begin
                wasquote:=false;
                break;
              end;
            end
            else if lpatptr^=' ' then
              break;
            inc(lpatptr);
          end;
          if lpatptr^<>#0 then
          begin
            lpatptr^:=#0;
            inc(lpatptr);
          end;
        end;
        if numpattern=maxpattern then break;
      end;
    until lpatptr^=#0;
  end;
end;

function CheckPatternW(cnt:integer):boolean;
var
  lstr:array [0..1023] of WideChar;
  i,j:integer;
begin
  if numpattern>0 then
  begin
    for i:=0 to numpattern-1 do
      patterns[i].res:=false;

    for i:=0 to qsopt.numcolumns-1 do
    begin
      if ((qsopt.columns[i].flags and (COL_ON or COL_FILTER))=(COL_ON or COL_FILTER)) and
         (MainBuf[cnt,i].text<>nil) then
      begin
        StrCopyW(lstr,MainBuf[cnt,i].text,HIGH(lstr));
        CharLowerW(lstr);
        for j:=0 to numpattern-1 do
          if not patterns[j].res then
          begin
            if StrPosW(lstr,patterns[j].str)<>nil then //!!
              patterns[j].res:=true;
          end;
      end;
    end;

    result:=true;
    for i:=0 to numpattern-1 do
      result:=result and patterns[i].res;
  end
  else
    result:=true;
end;

//----- support (column index converters) -----

{
   ListView - ListView (visible) column
   QS       - Buffer column
   Column   - qsopt.columns
}

function ListViewToColumn(col:integer):LPARAM;
var
  i:integer;
begin
  for i:=0 to qsopt.numcolumns-1 do
  begin
    if (qsopt.columns[i].flags and COL_ON)<>0 then
    begin
      dec(col);
      if col<0 then
      begin
        result:=i;
        exit;
      end;
    end;
  end;
  result:=-1;
end;

function ColumnToListView(col:integer):LPARAM;
var
  i:integer;
begin
  result:=-1;
  for i:=0 to qsopt.numcolumns-1 do
  begin
    if (qsopt.columns[i].flags and COL_ON)<>0 then
      inc(result);

    dec(col);
    if col<0 then
      break;
  end;
end;

// return buffer index for contact
function FindBufNumber(hContact:THANDLE):integer;
var
  i:integer;
begin
  for i:=0 to HIGH(FlagBuf) do
  begin
    if FlagBuf[i].contact=hContact then
    begin
      result:=i;
      exit;
    end;
  end;
  result:=-1;
end;

function IsColumnMinimized(num:integer):bool;
begin
  result:=ListView_GetColumnWidth(grid,num)<=10;
end;
