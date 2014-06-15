(*
    History++ plugin for Miranda IM: the free IM client for Microsoft* Windows*

    Copyright (C) 2006-2009 theMIROn, 2003-2006 Art Fedorov.
    History+ parts (C) 2001 Christian Kastner

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*)

{-----------------------------------------------------------------------------
 hpp_itemprocess (historypp project)

 Version:   1.5
 Created:   05.08.2004
 Author:    Oxygen

 [ Description ]

 Module for people to help get aquanted with ME_HPP_RICHEDIT_ITEMPROCESS
 Has samples for SmileyAdd, TextFormat, Math Module and new procedure
 called SeparateDialogs. It makes message black if previous was hour ago,
 kinda of conversation separation

 [ History ]

 1.5 (05.08.2004)
   First version

 [ Modifications ]
 none

 [ Known Issues ]
 none

 Contributors: theMIROn, Art Fedorov
-----------------------------------------------------------------------------}

{.$DEFINE USE_URL_BBCODE}

unit hpp_itemprocess;

interface

uses
  Windows;

var
  rtf_ctable_text: AnsiString;

function DoSupportBBCodesHTML(const S: AnsiString): AnsiString;
function DoSupportBBCodesRTF(const S: AnsiString; StartColor: integer; doColorBBCodes: boolean): AnsiString;
function DoStripBBCodes(const S: String): String;

function DoSupportSmileys(awParam:WPARAM; alParam: LPARAM): Integer;
function DoSupportAvatarHistory(awParam:WPARAM; alParam: LPARAM): Integer;

implementation

uses
  Messages,
  SysUtils, StrUtils,
  m_api,
  hpp_global, hpp_richedit, hpp_events{, RichEdit -- used for CHARRANGE and EM_EXTSETSEL};

const
  EM_EXSETSEL = WM_USER + 55; // from RichEdit

type

  TRTFColorTable = record
    sz: PAnsiChar;
    col: COLORREF;
  end;

  TBBCodeClass = (bbStart,bbEnd);
  TBBCodeType = (bbSimple, bbColor, bbSize, bbUrl, bbImage);

  TBBCodeString = record
    ansi: PAnsiChar;
    wide: String;
  end;

  TBBCodeInfo = record
    prefix: TBBCodeString;
    suffix: TBBCodeString;
    bbtype: TBBCodeType;
    rtf: PAnsiChar;
    html: PAnsiChar;
    minRE: Integer;
  end;

const
  rtf_ctable: array[0..7] of TRTFColorTable = (
    //                 BBGGRR
    (sz:'black';  col:$000000),
    (sz:'blue';   col:$FF0000),
    (sz:'green';  col:$00FF00),
    (sz:'red';    col:$0000FF),
    (sz:'magenta';col:$FF00FF),
    (sz:'cyan';   col:$FFFF00),
    (sz:'yellow'; col:$00FFFF),
    (sz:'white';  col:$FFFFFF));

const
  bbCodesCount = {$IFDEF USE_URL_BBCODE}7{$ELSE}6{$ENDIF};

var
  bbCodes: array[0..bbCodesCount,bbStart..bbEnd] of TBBCodeInfo = (
    ((prefix:(ansi:'[b]');      suffix:(ansi:nil); bbtype:bbSimple; rtf:'{\b ';      html:'<b>';  minRE: 10),
     (prefix:(ansi:'[/b]');     suffix:(ansi:nil); bbtype:bbSimple; rtf:'}';         html:'</b>')),
    ((prefix:(ansi:'[i]');      suffix:(ansi:nil); bbtype:bbSimple; rtf:'{\i ';      html:'<i>';  minRE: 10),
     (prefix:(ansi:'[/i]');     suffix:(ansi:nil); bbtype:bbSimple; rtf:'}';         html:'</i>')),
    ((prefix:(ansi:'[u]');      suffix:(ansi:nil); bbtype:bbSimple; rtf:'{\ul ';     html:'<u>';  minRE: 10),
     (prefix:(ansi:'[/u]');     suffix:(ansi:nil); bbtype:bbSimple; rtf:'}';         html:'</u>')),
    ((prefix:(ansi:'[s]');      suffix:(ansi:nil); bbtype:bbSimple; rtf:'{\strike '; html:'<s>';  minRE: 10),
     (prefix:(ansi:'[/s]');     suffix:(ansi:nil); bbtype:bbSimple; rtf:'}';         html:'</s>')),
    ((prefix:(ansi:'[color=');  suffix:(ansi:']'); bbtype:bbColor;  rtf:'{\cf%u ';   html:'<font style="color:%s">'; minRE: 10),
     (prefix:(ansi:'[/color]'); suffix:(ansi:nil); bbtype:bbSimple; rtf:'}';         html:'</font>')),
    {$IFDEF USE_URL_BBCODE}
    ((prefix:(ansi:'[url=');    suffix:(ansi:']'); bbtype:bbUrl;    rtf:'{\field{\*\fldinst{HYPERLINK ":%s"}}{\fldrslt{\ul\cf%u'; html:'<a href="%s">'; minRE: 41),
     (prefix:(ansi:'[/url]');   suffix:(ansi:nil); bbtype:bbSimple; rtf:'}}}';      html:'</a>')),
    {$ENDIF}
    ((prefix:(ansi:'[size=');   suffix:(ansi:']'); bbtype:bbSize;   rtf:'{\fs%u ';   html:'<font style="font-size:%spt">'; minRE: 10),
     (prefix:(ansi:'[/size]');  suffix:(ansi:nil); bbtype:bbSimple; rtf:'}';         html:'</font>')),
    ((prefix:(ansi:'[img]');    suffix:(ansi:nil); bbtype:bbImage;  rtf:'[{\revised\ul\cf%u '; html:'['; minRE: 20),
     (prefix:(ansi:'[/img]');   suffix:(ansi:nil); bbtype:bbSimple; rtf:'}]';        html:']'))
  );

const
  MAX_FMTBUF     = 4095;

var
  TextBuffer: THppBuffer;

function GetColorRTF(const code: AnsiString; colcount: integer): integer;
var
  i: integer;
begin
  Result := 0;
  if colcount >= 0 then
    for i := 0 to High(rtf_ctable) do
      if rtf_ctable[i].sz = code then
      begin
        Result := colcount + i;
        break;
      end;
end;

function StrReplace(strStart, str, strEnd: PAnsiChar; var strTrail: PAnsiChar): PAnsiChar;
var
  len,delta: integer;
  tmpStartPos,tmpEndPos,tmpTrailPos: Integer;
  tmpStart,tmpEnd,tmpTrail: PAnsiChar;
begin
  if str = nil then
    len := 0
  else
    len := StrLen(str);
  delta := len - (strTrail - strStart);
  tmpStartPos := strStart - TextBuffer.Buffer;
  tmpTrailPos := strTrail - TextBuffer.Buffer;
  tmpEndPos := strEnd - TextBuffer.Buffer;
  TextBuffer.Reallocate(tmpEndPos + delta + 1);
  tmpStart := PAnsiChar(TextBuffer.Buffer) + tmpStartPos;
  tmpTrail := PAnsiChar(TextBuffer.Buffer) + tmpTrailPos;
  tmpEnd := PAnsiChar(TextBuffer.Buffer) + tmpEndPos;
  strTrail := tmpTrail + delta;
  StrMove(strTrail, tmpTrail, tmpEnd - tmpTrail + 1);
  if len > 0 then
    StrMove(tmpStart, str, len);
  Result := tmpEnd + delta;
end;

function StrAppend(str, strEnd: PAnsiChar): PAnsiChar;
var
  len: integer;
  tmpEndPos: integer;
  tmpEnd: PAnsiChar;
begin
  if str = nil then
  begin
    Result := strEnd;
    exit;
  end;
  len := StrLen(str);
  tmpEndPos := strEnd - TextBuffer.Buffer;
  TextBuffer.Reallocate(tmpEndPos + len + 1);
  tmpEnd := PAnsiChar(TextBuffer.Buffer) + tmpEndPos;
  StrMove(tmpEnd, str, len + 1);
  Result := tmpEnd + len;
end;

function StrSearch(str,prefix,suffix: PAnsiChar; var strStart,strEnd,strCode: PAnsiChar; var lenCode: integer): Boolean;
begin
  Result := false;
  strStart := StrPos(str, prefix);
  if strStart = nil then
    exit;
  strCode := strStart + StrLen(prefix);
  if suffix = nil then
  begin
    lenCode := 0;
    strEnd := strCode
  end
  else
  begin
    strEnd := StrPos(strCode, suffix);
    if strEnd = nil then
      exit;
    lenCode := strEnd - strCode;
    strEnd := strEnd + StrLen(suffix);
  end;
  Result := true;
end;

(* commented out fo future use
function ParseLinksInRTF(S: AnsiString): AnsiString;
const
  urlStopChars = [' ','{','}','\','[',']'];
  url41fmt = '{\field{\*\fldinst{HYPERLINK "%s"}}{\fldrslt{{\v #}\ul\cf1 %0:s}}}';
var
  bufPos,bufEnd: PAnsiChar;
  urlStart,urlEnd: PAnsiChar;
  newCode: PAnsiChar;
  fmt_buffer: array[0..MAX_FMTBUF] of AnsiChar;
  code: AnsiString;
begin
  ShrinkTextBuffer;
  AllocateTextBuffer(Length(S)+1);
  bufEnd := StrECopy(buffer,PAnsiChar(S));
  bufPos := StrPos(buffer,'://');
  while Assigned(bufPos) do begin
    urlStart := bufPos;
    urlEnd := bufPos+3;
    while urlStart > buffer do begin
      Dec(urlStart);
      if urlStart[0] in urlStopChars then begin
        Inc(urlStart);
        break;
      end;
    end;
    while urlEnd < bufEnd do begin
      Inc(UrlEnd);
      if urlEnd[0] in urlStopChars then break;
    end;
    if (urlStart<bufPos) and (urlEnd>bufPos+3) then begin
      SetString(code,urlStart,urlEnd-urlStart);
      newCode := StrLFmt(fmt_buffer,MAX_FMTBUF,url41fmt,[code]);
      bufEnd := StrReplace(urlStart,newCode,bufEnd,urlEnd);
      bufPos := urlEnd;
    end;
    bufPos := StrPos(bufPos,'://');
  end;
  SetString(Result,buffer,bufEnd-buffer);
end;
*)

function DoSupportBBCodesRTF(const S: AnsiString; StartColor: integer; doColorBBCodes: boolean): AnsiString;
var
  bufPos,bufEnd: PAnsiChar;
  strStart,strTrail: PAnsiChar;
  strCode,newCode: PAnsiChar;
  i,n,lenCode: Integer;
  sfound,efound: Boolean;
  fmt_buffer: array[0..MAX_FMTBUF] of AnsiChar;
  code: AnsiString;
begin
  TextBuffer.Lock;
  TextBuffer.Allocate(Length(S)+1);
  bufEnd := StrECopy(TextBuffer.Buffer,PAnsiChar(S));
  for i := 0 to High(bbCodes) do
  begin
    if hppRichEditVersion < bbCodes[i, bbStart].minRE then
      continue;
    bufPos := TextBuffer.Buffer;
    repeat
      newCode := nil;
      sfound := StrSearch(TextBuffer.Buffer, bbCodes[i, bbStart].prefix.ansi,
        bbCodes[i, bbStart].suffix.ansi, strStart, strTrail, strCode, lenCode);
      if sfound then
      begin
        case bbCodes[i, bbStart].bbtype of
          bbSimple:
            newCode := bbCodes[i, bbStart].rtf;
          bbColor:
            begin
              if doColorBBCodes then
              begin
                SetString(code, strCode, lenCode);
                n := GetColorRTF(code, StartColor);
                newCode := StrLFmt(fmt_buffer, MAX_FMTBUF, bbCodes[i, bbStart].rtf, [n]);
              end;
            end;
          bbSize:
            begin
              SetString(code, strCode, lenCode);
              if TryStrToInt(String(code), n) then
                newCode := StrLFmt(fmt_buffer, MAX_FMTBUF, bbCodes[i, bbStart].rtf, [n shl 1]);
            end;
{$IFDEF USE_URL_BBCODE}
          bbUrl:
            begin
              SetString(code, strCode, lenCode);
              if doColorBBCodes then
                n := 2
              else // link color
                n := 0;
              newCode := StrLFmt(fmt_buffer, MAX_FMTBUF, bbCodes[i, bbStart].rtf, [PAnsiChar(code), n]);
            end;
{$ENDIF}
          bbImage:
            begin
              if doColorBBCodes then
                n := 2
              else // link color
                n := 0;
              newCode := StrLFmt(fmt_buffer, MAX_FMTBUF, bbCodes[i, bbStart].rtf, [n]);
            end;
        end;
        bufEnd := StrReplace(strStart, newCode, bufEnd, strTrail);
        bufPos := strTrail;
      end;
      repeat
        efound := StrSearch(bufPos, bbCodes[i, bbEnd].prefix.ansi,
          bbCodes[i, bbEnd].suffix.ansi, strStart, strTrail, strCode, lenCode);
        if sfound and (newCode <> nil) then
          strCode := bbCodes[i, bbEnd].rtf
        else
          strCode := nil;
        if efound then
        begin
          bufEnd := StrReplace(strStart, strCode, bufEnd, strTrail);
          bufPos := strTrail;
        end
        else
          bufEnd := StrAppend(strCode, bufEnd);
      until sfound or not efound;
    until not sfound;
  end;
  SetString(Result, PAnsiChar(TextBuffer.Buffer), bufEnd - TextBuffer.Buffer);
  TextBuffer.Unlock;
end;

function DoSupportBBCodesHTML(const S: AnsiString): AnsiString;
var
  bufPos,bufEnd: PAnsiChar;
  strStart,strTrail,strCode: PAnsiChar;
  i,lenCode: Integer;
  sfound,efound: Boolean;
  fmt_buffer: array[0..MAX_FMTBUF] of AnsiChar;
  code: AnsiString;
begin
  TextBuffer.Lock;
  TextBuffer.Allocate(Length(S) + 1);
  bufEnd := StrECopy(TextBuffer.Buffer, PAnsiChar(S));
  for i := 0 to High(bbCodes) do
  begin
    bufPos := TextBuffer.Buffer;
    repeat
      sfound := StrSearch(TextBuffer.Buffer, bbCodes[i, bbStart].prefix.ansi,
        bbCodes[i, bbStart].suffix.ansi, strStart, strTrail, strCode, lenCode);
      if sfound then
      begin
        if bbCodes[i, bbStart].bbtype = bbSimple then
          strCode := bbCodes[i, bbStart].html
        else
        begin
          SetString(code, strCode, lenCode);
          strCode := StrLFmt(fmt_buffer, MAX_FMTBUF, bbCodes[i, bbStart].html,
            [PAnsiChar(code)]);
        end;
        bufEnd := StrReplace(strStart, strCode, bufEnd, strTrail);
        bufPos := strTrail;
      end;
      repeat
        efound := StrSearch(bufPos, bbCodes[i, bbEnd].prefix.ansi,
          bbCodes[i, bbEnd].suffix.ansi, strStart, strTrail, strCode, lenCode);
        if sfound then
          strCode := bbCodes[i, bbEnd].html
        else
          strCode := nil;
        if efound then
        begin
          bufEnd := StrReplace(strStart, strCode, bufEnd, strTrail);
          bufPos := strTrail;
        end
        else
          bufEnd := StrAppend(strCode, bufEnd);
      until sfound or not efound;
    until not sfound;
  end;
  SetString(Result,PAnsiChar(TextBuffer.Buffer),bufEnd-TextBuffer.Buffer);
  TextBuffer.Unlock;
end;

function DoStripBBCodes(const S: String): String;
var
  WideStream: String;
  i,spos,epos,cpos,slen: integer;
  trail: String;
  bbClass: TBBCodeClass;
begin
  WideStream := S;
  for i := 0 to High(bbCodes) do
    for bbClass := bbStart to bbEnd do
    begin
      if bbCodes[i, bbClass].bbtype = bbSimple then
        WideStream := StringReplace(WideStream, bbCodes[i, bbClass].prefix.wide, '', [rfReplaceAll])
      else
        repeat
          spos := Pos(bbCodes[i, bbClass].prefix.wide, WideStream);
          epos := 0;
          if spos > 0 then
          begin
            cpos := spos + Length(bbCodes[i, bbClass].prefix.wide);
            slen := Length(bbCodes[i, bbClass].suffix.wide);
            if slen = 0 then
              epos := cpos
            else
              epos := PosEx(bbCodes[i, bbClass].suffix.wide, WideStream, cpos);
            if epos > 0 then
            begin
              cpos := epos + slen;
              trail := Copy(WideStream, cpos, Length(WideStream) - cpos + 1);
              SetLength(WideStream, spos - 1);
              WideStream := WideStream + trail;
            end;
          end;
        until (spos = 0) or (epos = 0);
    end;
  Result := WideStream;
end;

function DoSupportSmileys(awParam{hRichEdit}:WPARAM; alParam{PItemRenderDetails}: LPARAM): Integer;
const
  mesSent: Array[False..True] of Integer = (0,SAFLRE_OUTGOING);
var
  sare: TSMADD_RICHEDIT3;
  ird: PItemRenderDetails;
begin
  ird := Pointer(alParam);
  sare.cbSize := SizeOf(sare);
  sare.hwndRichEditControl := awParam;
  sare.rangeToReplace := nil;
  sare.ProtocolName := ird^.pProto;
  //sare.flags := SAFLRE_INSERTEMF + mesSent[ird^.IsEventSent];
  sare.flags := mesSent[ird^.IsEventSent] or SAFLRE_FIREVIEW;
  sare.disableRedraw := True;
  sare.hContact := ird^.hContact;
  CallService(MS_SMILEYADD_REPLACESMILEYS,0,LPARAM(@sare));
  Result := 0;
end;


function DoSupportAvatarHistory(awParam:WPARAM; alParam: LPARAM): int;
const
  crlf: AnsiString = '{\rtf1{\line }}';
var
  ird: PItemRenderDetails;
  Link: AnsiString;
  hBmp: hBitmap;
  cr: CHARRANGE;
  hppProfileDir:AnsiString;
begin
  Result := 0;
  ird := Pointer(alParam);
  if ird.wEventType <> EVENTTYPE_AVATARCHANGE then
    exit;
  if (ird.pExtended = nil) or (lstrlenA(ird.pExtended) < 4) then
    exit;
  if ((ird.pExtended[0] = '\') and (ird.pExtended[1] = '\')) or
    ((ird.pExtended[0] in ['A' .. 'Z', 'a' .. 'z']) and (ird.pExtended[1] = ':') and
    (ird.pExtended[2] = '\')) then
    Link := ird.pExtended
  else
  begin
    // Get profile dir
    SetLength(hppProfileDir, MAX_PATH);
    CallService(MS_DB_GETPROFILEPATH, MAX_PATH, lParam(@hppProfileDir[1]));
    SetLength(hppProfileDir, StrLen(pAnsiChar(@hppProfileDir[1])));
    Link := AnsiString(hppProfileDir) + '\' + ird.pExtended; //!!
  end;
  hBmp := CallService(MS_UTILS_LOADBITMAP, 0, LPARAM(@Link[1]));
  if hBmp <> 0 then
  begin
    cr.cpMin := SendMessage(awParam, WM_GETTEXTLENGTH, 0, 0);
    cr.cpMax := cr.cpMin;
    SendMessage(awParam, EM_EXSETSEL, 0, LPARAM(@cr));
    SetRichRTF(awParam, crlf, true, false, true);
    RichEdit_InsertBitmap(awParam, hBmp, Cardinal(-1));
  end;
end;

procedure Filltables;
var
  i: integer;
begin
  for i := 0 to High(rtf_ctable) do
  begin
    rtf_ctable_text := rtf_ctable_text + AnsiString(format('\red%d\green%d\blue%d;',
      [rtf_ctable[i].col and $FF,
      (rtf_ctable[i].col shr 8) and $FF,
      (rtf_ctable[i].col shr 16) and $FF]));
  end;

  for i := 0 to High(bbCodes) do
  begin
    bbCodes[i, bbStart].prefix.wide := String(bbCodes[i, bbStart].prefix.ansi);
    bbCodes[i, bbStart].suffix.wide := String(bbCodes[i, bbStart].suffix.ansi);
    bbCodes[i, bbEnd  ].prefix.wide := String(bbCodes[i, bbEnd  ].prefix.ansi);
    bbCodes[i, bbEnd  ].suffix.wide := String(bbCodes[i, bbEnd  ].suffix.ansi);
  end;
end;

initialization
  rtf_ctable_text := '';

  FillTables;

  TextBuffer := THppBuffer.Create;

finalization
  TextBuffer.Destroy;

end.
