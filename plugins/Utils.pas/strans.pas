{}
unit strans;

interface

uses windows{$IFDEF Miranda}, m_api, mirutils{$ENDIF};

// <align>|[<key>]<type> [(<type alias>)] [<alias>] [arr.len] [value]|
const
  char_separator = '|';
  char_hex       = '$';
  char_return    = '*';
  char_script    = '%';
  char_size      = '@';
{$IFDEF Miranda}
  char_mmi       = '&';
{$ENDIF}
const
  SST_BYTE    = 0;
  SST_WORD    = 1;
  SST_DWORD   = 2;
  SST_QWORD   = 3;
  SST_NATIVE  = 4;
  SST_BARR    = 5;
  SST_WARR    = 6;
  SST_BPTR    = 7;
  SST_WPTR    = 8;
  SST_LAST    = 9;
  SST_PARAM   = 10;
  SST_UNKNOWN = -1;
const
  SF_RETURN = $00000001;
  SF_SCRIPT = $00000002;
  SF_MMI    = $00000004;
  SF_SIZE   = $00000008;
  SF_LAST   = $00000080;

type
  TStructType = record
    typ  :integer;
    short:PAnsiChar;
    full :PAnsiChar;
  end;
const
  MaxStructTypes = 11;
const
  StructElems: array [0..MaxStructTypes-1] of TStructType = (
    (typ:SST_BYTE  ; short:'byte'  ; full:'Byte'),
    (typ:SST_WORD  ; short:'word'  ; full:'Word'),
    (typ:SST_DWORD ; short:'dword' ; full:'DWord'),
    (typ:SST_QWORD ; short:'qword' ; full:'QWord'),
    (typ:SST_NATIVE; short:'native'; full:'NativeInt'),
    (typ:SST_BARR  ; short:'b.arr' ; full:'Byte Array'),
    (typ:SST_WARR  ; short:'w.arr' ; full:'Word Array'),
    (typ:SST_BPTR  ; short:'b.ptr' ; full:'Pointer to bytes'),
    (typ:SST_WPTR  ; short:'w.ptr' ; full:'Pointer to words'),
{$IFDEF Miranda}
    (typ:SST_LAST  ; short:'last'  ; full:'Last result'),
    (typ:SST_PARAM ; short:'param' ; full:'Parameter')
{$ELSE}
    (typ:SST_LAST  ; short:'last'  ; full:'Parameter 1'),
    (typ:SST_PARAM ; short:'param' ; full:'Parameter 2')
{$ENDIF}
  );

type
  tOneElement = record
    etype :integer;
    flags :integer; // SF_MMI,SF_SCRIPT,SF_RETURN
    len   :integer; // value length (for arrays and pointers)
    align :integer;
    alias :array [0..63] of AnsiChar;
    talias:array [0..63] of AnsiChar; // type alias
    svalue:array [0..31] of AnsiChar; // numeric value text
    case boolean of
      false: (value:int64);
      true : (text :pointer);
  end;

//----- Editor connect -----

function GetOneElement(txt:PAnsiChar;var res:tOneElement;
                       SizeOnly:boolean;num:integer=0):integer;
procedure FreeElement(var element:tOneElement);

//----- Execute -----

function MakeStructure(txt:PAnsiChar;aparam,alast:LPARAM; isNumber:boolean=true):pointer;

function GetStructureResult(var struct):PWideChar;

procedure FreeStructure(var struct);


implementation

uses common;

type
  puint64   = ^uint64;
  pint_ptr  = ^int_ptr;
  puint_ptr = ^uint_ptr;
  TWPARAM   = WPARAM;
  TLPARAM   = LPARAM;

type
  pShortTemplate = ^tShortTemplate;
  tShortTemplate = packed record
    etype :byte;
    flags :byte;
    offset:word;
  end;

type
  // int_ptr = to use aligned structure data at start
  PStructResult = ^TStructResult;
  TStructResult = record
    typ   :int_ptr;
    len   :int_ptr;
    offset:int_ptr;
  end;

// adjust offset to field
function AdjustSize(var summ:int_ptr;eleadjust:integer;adjust:integer):integer;
var
  rest,lmod:integer;
begin
  // packed, byte or array of byte
  if adjust=0 then
    adjust:=SizeOf(pointer);

  if (adjust=1) or (eleadjust=1) then
  else
  begin
    case adjust of
      2: begin
        lmod:=2;
      end;
      4: begin
        if eleadjust>2 then
          lmod:=4
        else
          lmod:=2;
      end;
      8: begin
        if eleadjust>4 then
          lmod:=8
        else if eleadjust>2 then
          lmod:=4
        else
          lmod:=2;
      end;
    else
      lmod:=2;
    end;
    rest:=summ mod lmod;
    if rest>0 then
    begin
      summ:=summ+(lmod-rest);
    end;
  end;
  
  result:=summ;
end;

procedure SkipSpace(var txt:PAnsiChar); {$IFDEF FPC}inline;{$ENDIF}
begin
  while (txt^ in [' ',#9]) do inc(txt);
end;

function GetOneElement(txt:PAnsiChar;var res:tOneElement;
                       SizeOnly:boolean;num:integer=0):integer;
var
  pc,pc1:PAnsiChar;
  i,llen:integer;
begin
  FillChar(res,SizeOf(res),0);

  if num>0 then // Skip needed element amount
  begin
  end;

  SkipSpace(txt);
  // process flags
  while not (txt^ in sWordOnly) do
  begin
    case txt^ of
      char_return: res.flags:=res.flags or SF_RETURN;
      char_size  : res.flags:=res.flags or SF_SIZE;
{$IFDEF Miranda}
      char_script: res.flags:=res.flags or SF_SCRIPT;
      char_mmi   : res.flags:=res.flags or SF_MMI;
{$ENDIF}
    end;
    inc(txt);
  end;

  SkipSpace(txt);
  // element type
  pc:=txt;
  llen:=0;
  repeat
    inc(pc);
    inc(llen);
  until pc^ IN [#0,#9,' ',char_separator];
  // recogninze data type
  i:=0;
  while i<MaxStructTypes do
  begin
    if StrCmp(txt,StructElems[i].short,llen)=0 then //!!
      break;
    inc(i);
  end;
  if i>=MaxStructTypes then
  begin
    result   :=SST_UNKNOWN;
    res.etype:=SST_UNKNOWN;
    exit;
  end;
  result:=StructElems[i].typ;
  res.etype:=result;

  if (not SizeOnly) or (result in [SST_WARR,SST_BARR,SST_WPTR,SST_BPTR]) then
  begin
    // type alias, inside parentheses
    SkipSpace(pc);
    if not (pc^ in [#0,char_separator]) then
    begin
      if (pc^='(') and ((pc+1)^ in sIdFirst) then
      begin
        inc(pc); // skip space and parenthesis
        pc1:=@res.talias;
        repeat
          pc1^:=pc^;
          inc(pc1);
          inc(pc);
        until (pc^=')'){ or (pc^=' ')} or (pc^=char_separator);
        if pc^=')' then inc(pc);
      end;
    end;

    // alias, starting from letter
    // start: points to separator or space
    SkipSpace(pc);
    if not (pc^ in [#0,char_separator]) then
    begin
      if pc^ in sIdFirst then
      begin
//        inc(pc); // skip space
        pc1:=@res.alias;
        repeat
          pc1^:=pc^;
          inc(pc1);
          inc(pc);
        until (pc^=' ') or (pc^=char_separator);
      end;
    end;

    // next - values
    // if has empty simple value, then points to next element but text-to-number will return 0 anyway
//    if pc^=' ' then inc(pc); // points to value or nothing if no args
    SkipSpace(pc);
    case result of
      SST_LAST,SST_PARAM: ;

      SST_BYTE,SST_WORD,SST_DWORD,SST_QWORD,SST_NATIVE: begin
        begin
          if (res.flags and SF_SCRIPT)=0 then
          begin
            pc1:=@res.svalue;
            if pc^=char_hex then
            begin
              inc(pc);
              while pc^ in sHexNum do
              begin
                pc1^:=pc^;
                inc(pc1);
                inc(pc);
              end;
              res.value:=HexToInt(res.svalue)
            end
            else
            begin
              while pc^ in sNum do
              begin
                pc1^:=pc^;
                inc(pc1);
                inc(pc);
              end;
              res.value:=StrToInt(res.svalue);
            end;
          end
          else
          begin
            txt:=pc;
            //!!
            while not (pc^ in [#0,char_separator]) do inc(pc);
            if txt<>pc then
              StrDup(PAnsiChar(res.text),txt,pc-txt)
            else
              res.text:=nil;
          end;
        end;
      end;

      SST_BARR,SST_WARR,SST_BPTR,SST_WPTR: begin
        // if pc^ in sNum then
        res.len:=StrToInt(pc);
        if not SizeOnly then
        begin
          while pc^ in sNum do inc(pc);

          // skip space
//          if pc^=' ' then inc(pc);
          SkipSpace(pc);

          //!!
          if not (pc^ in [#0,char_separator]) then
          begin
            txt:=pc;
            while not (pc^ in [#0,char_separator]) do inc(pc);
            if txt<>pc then
              StrDup(PAnsiChar(res.text),txt,pc-txt)
            else
              res.text:=nil;
          end;
        end;
      end;
    end;
  end;

  case result of
    SST_LAST,
    SST_PARAM : begin res.len:=SizeOf(LPARAM); res.align:=SizeOf(LPARAM); end;
    SST_BYTE  : begin res.len:=1; res.align:=1; end;
    SST_WORD  : begin res.len:=2; res.align:=2; end;
    SST_DWORD : begin res.len:=4; res.align:=4; end;
    SST_QWORD : begin res.len:=8; res.align:=8; end;
    SST_NATIVE: begin res.len:=SizeOf(LPARAM); res.align:=SizeOf(LPARAM); end; // SizeOf(NativeInt)
    SST_BARR  : res.align:=1;
    SST_WARR  : res.align:=2;
    SST_BPTR  : res.align:=SizeOf(pointer);
    SST_WPTR  : res.align:=SizeOf(pointer);
  end;
end;

// within translation need to check array size limit
// "limit" = array size, elements, not bytes!
procedure TranslateBlob(dst:PByte;const element:tOneElement);
var
  datatype:integer;
  clen,len:integer;
  src:PAnsiChar;
  srcw:PWideChar absolute src;
  buf:array [0..9] of AnsiChar;
  bufw:array [0..4] of WideChar absolute buf;
begin
  if element.text=nil then exit;

  if element.etype in [SST_WARR,SST_WPTR] then
  begin
    if (element.flags and SF_SCRIPT)<>0 then
      datatype:=2  // Wide to Wide (if script done)
    else
      datatype:=1; // UTF to Wide
  end
  else // Ansi to Ansi
    datatype:=0;

  pint64(@buf)^:=0;

  src:=element.text;
  case datatype of
    0: begin // Ansi source for ansi
      len:=StrLen(src);
      if (element.len<>0) and (element.len<len) then
        len:=element.len;

      if StrScan(src,char_hex)=nil then
      begin
        move(src^,dst^,len);
      end
      else
      begin
        while (src^<>#0) and (len>0) do
        begin
          if (src^=char_hex) and ((src+1)^ in sHexNum) and ((src+2)^ in sHexNum) then
          begin
            buf[0]:=(src+1)^;
            buf[1]:=(src+2)^;
            inc(src,2+1);
            dst^:=HexToInt(buf);
          end
          else
          begin
            dst^:=ord(src^);
            inc(src);
          end;
          inc(dst);
          dec(len);
        end;
      end;
    end;

    1: begin // UTF8 source for unicode
      // char_hex not found in UTF8 as Ansi - no reason to check as UTF8
      // right, if char_hex is in ASCI7 only
{
      if StrScan(src,char_hex)=nil then
      begin
        // convert UTF8 to Wide without zero
      end
      else
}
      begin
        len:=element.len;
        while (src^<>#0) and (len>0) do
        begin
          if (src^=char_hex) and
             ((src+1)^ in sHexNum) and
             ((src+2)^ in sHexNum) then
          begin
            buf[0]:=(src+1)^;
            buf[1]:=(src+2)^;
            if ((src+3)^ in sHexNum) and
               ((src+4)^ in sHexNum) then
            begin
              buf[2]:=(src+3)^;
              buf[3]:=(src+4)^;
              pWord(dst)^:=HexToInt(buf);
              inc(src,4+1);
              inc(dst,2);
            end
            else
            begin
              buf[2]:=#0;
              dst^:=HexToInt(buf);
              inc(dst);
              inc(src,2+1);
            end;
          end
          else
          begin
            PWideChar(dst)^:=CharUTF8ToWide(src,@clen);
            inc(src,clen{CharUTF8Len(src)});
            inc(dst,2);
            dec(len);
          end;
        end;
      end;
    end;
{$IFDEF Miranda}
    2: begin // Unicode source for unicode
      len:=StrLenW(srcw);
      if (element.len<>0) and (element.len<len) then
        len:=element.len;

      if StrScanW(srcw,char_hex)=nil then
      begin
        move(srcw^,dst^,len*SizeOf(WideChar));
      end
      else
      begin
        while (srcw^<>#0) and (len>0) do
        begin
          if (srcw^=char_hex) and
             (ord(srcw[1])<255) and (AnsiChar(srcw[1]) in sHexNum) and
             (ord(srcw[2])<255) and (AnsiChar(srcw[2]) in sHexNum) then
          begin
            bufw[0]:=srcw[1];
            bufw[1]:=srcw[2];
            if (ord(srcw[3])<255) and (AnsiChar(srcw[3]) in sHexNum) and
               (ord(srcw[4])<255) and (AnsiChar(srcw[4]) in sHexNum) then
            begin
              bufw[2]:=srcw[3];
              bufw[3]:=srcw[4];
              pWord(dst)^:=HexToInt(bufw);
              inc(src,4);
            end
            else
            begin
              bufw[2]:=#0;
              dst^:=HexToInt(bufw);
              dec(dst);
              inc(srcw,2);
            end;
          end
          else
            PWideChar(dst)^:=srcw^;
          inc(srcw);
          inc(dst,2);
          dec(len);
        end;
      end;
    end;
{$ENDIF}
  end;
end;

procedure FreeElement(var element:tOneElement);
begin
  case element.etype of
    SST_PARAM,SST_LAST: begin
    end;
    SST_BYTE,SST_WORD,SST_DWORD,
    SST_QWORD,SST_NATIVE: begin
      if (element.flags and SF_SCRIPT)<>0 then
        mFreeMem(element.text);
    end;
    SST_BARR,SST_WARR,
    SST_BPTR,SST_WPTR: begin
      mFreeMem(element.text);
    end;
  end;
end;

function MakeStructure(txt:PAnsiChar;aparam,alast:LPARAM; isNumber:boolean=true):pointer;
var
  summ:int_ptr;
  lsrc:PAnsiChar;
  res:PByte;
  ppc,p,pc:PAnsiChar;
{$IFDEF Miranda}
  buf:array [0..31] of WideChar;
  pLast: PWideChar;
  valuein,value:PWideChar;
{$ENDIF}
  amount,align:integer;
  lmod,code,alen,ofs:integer;
  element:tOneElement;
  tmpl:pShortTemplate;
  addsize:integer;
begin
  result:=nil;
  if (txt=nil) or (txt^=#0) then
    exit;

  pc:=txt;

  ppc:=pc;
  summ:=0;

  if pc^ in sNum then
  begin
    align:=ord(pc^)-ord('0');//StrToInt(pc);
    lsrc:=StrScan(pc,char_separator)+1; // not just +2 for future features
  end
  else
  begin
    align:=0;
    lsrc:=pc;
  end;

  code:=SST_UNKNOWN;
  alen:=0;
  ofs :=0;

  amount:=0;
  // size calculation
  while lsrc^<>#0 do
  begin
    p:=StrScan(lsrc,char_separator);
//    if p<>nil then p^:=#0;

    GetOneElement(lsrc,element,true);
    AdjustSize(summ,element.align,align);

    if ((element.flags and SF_RETURN)<>0) and (code=SST_UNKNOWN) then
    begin
      code:=element.etype;
      alen:=element.len;
      ofs :=summ;
    end;

    if (element.etype=SST_BPTR) or (element.etype=SST_WPTR) then
      inc(summ,SizeOf(pointer))
    else
      inc(summ,element.len);

    inc(amount);

    if p=nil then break;
    lsrc:=p+1;
  end;

  // memory allocation with result record and template
  addsize:=SizeOF(TStructResult)+SizeOF(tShortTemplate)*amount+SizeOf(dword);
  lmod:=addsize mod SizeOf(pointer);
  if lmod<>0 then
    inc(addsize,SizeOf(pointer)-lmod);

  inc(summ,addsize);

  mGetMem (tmpl,summ);
  FillChar(tmpl^,summ,0);

  res:=PByte(PAnsiChar(tmpl)+addsize-SizeOf(TStructResult)-SizeOf(dword));
  pdword(res)^:=amount; inc(res,SizeOf(dword));
  with PStructResult(res)^ do
  begin
    typ   :=code;
    len   :=alen;
    offset:=ofs;
  end;

  inc(res,SizeOf(TStructResult));
  result:=res;

  pc:=ppc;

  // translation
  if pc^ in sNum then
    // pc:=pc+2;
    pc:=StrScan(pc,char_separator)+1;

  while pc^<>#0 do
  begin
    p:=StrScan(pc,char_separator);
    GetOneElement(pc,element,false);

    if (element.flags and SF_SIZE)<>0 then
    begin
      element.value:=summ-addsize;
    end;

    if (element.flags and SF_SCRIPT)<>0 then
    begin
{$IFDEF Miranda}
      if isNumber then
        pLast:=IntToStr(buf,alast)
      else
        pLast:=PWideChar(alast);
      // BPTR,BARR - Ansi
      // WPTR,WARR - Unicode
      // BYTE,WORD,DWORD,QWORD,NATIVE - ???
      // in value must be converted to unicode/ansi but not UTF8
      UTF8ToWide(element.text,valuein);
      value:=ParseVarString(valuein,aparam,pLast);
      mFreeMem(valuein);
      case element.etype of
        // Numbers - just get number values
        SST_BYTE,
        SST_WORD,
        SST_DWORD,
        SST_QWORD,
        SST_NATIVE: begin
  {
          StrCopy(element.svalue,value,31);
          element.value:=StrToInt(element.svalue);
  }
          element.value:=StrToInt(value);
          mFreeMem(value);
        end;
        // Byte strings - replace Ansi value
        SST_BARR,
        SST_BPTR: begin
          mFreeMem(element.text);
          WideToAnsi(value,PAnsiChar(element.text),MirandaCP);
          mFreeMem(value);
        end;
        // Wide strings - replace UTF8 by Wide
        SST_WARR,
        SST_WPTR: begin
        // really, need to translate Wide to UTF8 again?
        mFreeMem(element.text);
        element.text:=value;
        end;
      end;
{$ENDIF}
    end;

    AdjustSize(int_ptr(res),element.align,align);

    tmpl^.etype :=element.etype;
    tmpl^.flags :=element.flags;
    tmpl^.offset:=uint_ptr(res)-uint_ptr(result);

    case element.etype of
      SST_LAST: begin
        pint_ptr(res)^:=alast;
      end;
      SST_PARAM: begin
        pint_ptr(res)^:=aparam;
      end;
      SST_BYTE: begin
        PByte(res)^:=element.value;
      end;
      SST_WORD: begin
        pWord(res)^:=element.value;
      end;
      SST_DWORD: begin
        pDWord(res)^:=element.value;
      end;
      SST_QWORD: begin
        pint64(res)^:=element.value;
      end;
      SST_NATIVE: begin
        pint_ptr(res)^:=element.value;
      end;
      SST_BARR: begin
        TranslateBlob(PByte(res),element);
      end;
      SST_WARR: begin
        TranslateBlob(PByte(res),element);
      end;
      SST_BPTR: begin
        if element.len=0 then
          element.len:=StrLen(element.text);

        if element.len=0 then
          pint_ptr(res)^:=0
        else
        begin
          inc(element.len); // with Zero at the end
{$IFDEF Miranda}
          if (element.flags and SF_MMI)<>0 then
            lsrc:=mir_alloc(element.len*SizeOf(AnsiChar))
          else
{$ENDIF}
          mGetMem (lsrc ,element.len*SizeOf(AnsiChar));
          FillChar(lsrc^,element.len*SizeOf(AnsiChar),0);
          TranslateBlob(PByte(lsrc),element);
          pint_ptr(res)^:=uint_ptr(lsrc);
        end;
      end;
      SST_WPTR: begin
        if element.len=0 then
        begin
{$IFDEF Miranda}
          if (element.flags and SF_SCRIPT)<>0 then
            element.len:=StrLenW(element.text)
          else
{$ENDIF}
          element.len:=UTF8Len(element.text);
        end;

        if element.len=0 then
          pint_ptr(res)^:=0
        else
        begin
          inc(element.len); // with Zero at the end
{$IFDEF Miranda}
          if (element.flags and SF_MMI)<>0 then
            lsrc:=mir_alloc(element.len*SizeOf(WideChar))
          else
{$ENDIF}
          mGetMem (lsrc ,element.len*SizeOf(WideChar));
          FillChar(lsrc^,element.len*SizeOf(WideChar),0);
//!!!!! variables script gives unicode, need to recognize it
          TranslateBlob(PByte(lsrc),element);
          pint_ptr(res)^:=uint_ptr(lsrc);
        end;
      end;
    end;
    if (element.etype=SST_BPTR) or (element.etype=SST_WPTR) then
      inc(int_ptr(res),SizeOf(pointer))
    else
      inc(int_ptr(res),element.len);

    FreeElement(element);
    if p=nil then break;
    pc:=p+1;
    inc(tmpl);
  end;
  tmpl^.flags:=tmpl^.flags or SF_LAST;
end;

function GetResultPrivate(var struct;atype:pinteger=nil;alen:pinteger=nil):uint_ptr;
var
  loffset,ltype:integer;
begin
  with PStructResult(PAnsiChar(struct)-SizeOF(TStructResult))^ do
  begin
    ltype  :=typ   ;
    loffset:=offset;
    if atype<>nil then atype^:=typ;
    if alen <>nil then alen ^:=len;
  end;

  case ltype of
    SST_LAST : result:=0;
    SST_PARAM: result:=0;

    SST_BYTE  : result:=PByte    (PAnsiChar(struct)+loffset)^;
    SST_WORD  : result:=pWord    (PAnsiChar(struct)+loffset)^;
    SST_DWORD : result:=pDword   (PAnsiChar(struct)+loffset)^;
    SST_QWORD : result:=puint64  (PAnsiChar(struct)+loffset)^;
    SST_NATIVE: result:=puint_ptr(PAnsiChar(struct)+loffset)^;

    SST_BARR: result:=uint_ptr(PAnsiChar(struct)+loffset); //??
    SST_WARR: result:=uint_ptr(PAnsiChar(struct)+loffset); //??

    SST_BPTR: result:=puint_ptr(PAnsiChar(struct)+loffset)^; //??
    SST_WPTR: result:=puint_ptr(PAnsiChar(struct)+loffset)^; //??
  else
    result:=0;
  end;
end;

function GetStructureResult(var struct):PWideChar;
var
  buf:array [0..31] of WideChar;
  pc:pAnsiChar;
  data:uint_ptr;
  code:integer;
  len:integer;
begin
  data:=GetResultPrivate(struct,@code,@len);
  case code of
    SST_BYTE,SST_WORD,SST_DWORD,
    SST_QWORD,SST_NATIVE: begin
      StrDupW(result,IntToStr(buf,data));
    end;

    SST_BARR: begin
      StrDup(pc,pAnsiChar(data),len);
      AnsiToWide(pc,result{$IFDEF Miranda},MirandaCP{$ENDIF});
      mFreeMem(pc);
    end;

    SST_WARR: begin
      StrDupW(result,pWideChar(data),len);
    end;

    SST_BPTR: begin
      AnsiToWide(pAnsiChar(data),result{$IFDEF Miranda},MirandaCP{$ENDIF});
    end;

    SST_WPTR: begin
      StrDupW(result,pWideChar(data));
    end;
  else
    result:=nil;
  end;
{
  FreeStructure(struct); //??
  uint_ptr(struct):=0;
}
end;

procedure FreeStructure(var struct);
var
  value:PAnsiChar;
  tmpl:pShortTemplate;
  num,lmod:integer;
  tmp:pointer;
begin
  tmp:=pointer(PAnsiChar(struct)-SizeOF(TStructResult)-SizeOf(dword));
  num:=pdword(tmp)^;
  tmpl:=pointer(PAnsiChar(tmp)-num*SizeOf(tShortTemplate));
  lmod:=uint_ptr(tmpl) mod SizeOf(pointer);
  // align to pointer size border
  if lmod<>0 then
    tmpl:=pointer(PAnsiChar(tmpl)-(SizeOf(pointer)-lmod));

  tmp:=tmpl;

  repeat
    case tmpl^.etype of
      SST_BPTR,SST_WPTR: begin
        //??
        value:=PAnsiChar(pint_ptr(PAnsiChar(struct)+tmpl^.offset)^);
{$IFDEF Miranda}
        if (tmpl^.flags and SF_MMI)<>0 then
          mir_free(value)
        else
{$ENDIF}
        mFreeMem(value);
      end;
    end;
    inc(tmpl);
  until (tmpl^.flags and SF_LAST)<>0;

  mFreeMem(tmp);
end;

end.
