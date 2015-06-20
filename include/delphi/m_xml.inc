{
Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2008 Miranda ICQ/IM project, 
all portions of this codebase are copyrighted to the people 
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}

{$IFNDEF M_XML}
{$DEFINE M_XML}

type
  HXML = THANDLE;

type
  XML_ELEMENT_POS = int; // XML_ELEMENT_POS is not interchangeable with simple indexes

type
  XML_ELEMENT_TYPE = (
    XML_ELEM_TYPE_CHILD,XML_ELEM_TYPE_ATTRIBUTE,
    XML_ELEM_TYPE_TEXT ,XML_ELEM_TYPE_CLEAR);

/// Enumeration for XML parse errors.
type
  XMLError = (
    eXMLErrorNone,
    eXMLErrorMissingEndTag,
    eXMLErrorNoXMLTagFound,
    eXMLErrorEmpty,
    eXMLErrorMissingTagName,
    eXMLErrorMissingEndTagName,
    eXMLErrorUnmatchedEndTag,
    eXMLErrorUnmatchedEndClearTag,
    eXMLErrorUnexpectedToken,
    eXMLErrorNoElements,
    eXMLErrorFileNotFound,
    eXMLErrorFirstTagNotFound,
    eXMLErrorUnknownCharacterEntity,
    eXMLErrorCharacterCodeAbove255,
    eXMLErrorCharConversionError,
    eXMLErrorCannotOpenWriteFile,
    eXMLErrorCannotWriteFile,

    eXMLErrorBase64DataSizeIsNotMultipleOf4,
    eXMLErrorBase64DecodeIllegalCharacter,
    eXMLErrorBase64DecodeTruncatedData,
    eXMLErrorBase64DecodeBufferTooSmall);

function  xmlCreateNode(const name, text:PWideChar; IsDeclaration:boolean):HXML; stdcall;
             external AppDll name 'xmlCreateNode';
procedure xmlDestroyNode(node:HXML); stdcall;
             external AppDll name 'xmlDestroyNode';
                      
function  xmlParseString(const str:PWideChar; datalen:pint; const tag:PWideChar):HXML; stdcall;
             external AppDll name 'xmlParseString';
function  xmlToString(node:HXML;datalen:pint):PWideChar; stdcall;
             external AppDll name 'xmlToString';
                      
function  xmlAddChild(parent:HXML; const name,text:PWideChar):HXML; stdcall;
             external AppDll name 'xmlAddChild';
procedure xmlAddChild2(child,parent:HXML); stdcall;
             external AppDll name 'xmlAddChild2';
function  xmlCopyNode(parent:HXML):HXML; stdcall;
             external AppDll name 'xmlCopyNode';
function  xmlGetChild(parent:HXML;number:int):HXML; stdcall;
             external AppDll name 'xmlGetChild';
function  xmlGetChildCount(h:HXML):int; stdcall;
             external AppDll name 'xmlGetChildCount';
function  xmlGetChildByAttrValue(parent:HXML; const name,attrName,attrValue:PWideChar):HXML; stdcall;
             external AppDll name 'xmlGetChildByAttrValue';
function  xmlGetFirstChild(parent:HXML):HXML; stdcall;
             external AppDll name 'xmlGetFirstChild';
function  xmlGetNthChild(parent:HXML; const name:PWideChar; i:int):HXML; stdcall;
             external AppDll name 'xmlGetNthChild';
function  xmlGetNextChild(parent:HXML; const name:PWideChar; i:pint):HXML; stdcall;
             external AppDll name 'xmlGetNextChild';
function  xmlGetChildByPath(parent:HXML; const path:PWideChar;createNodeIfMissing:boolean):HXML; stdcall;
             external AppDll name 'xmlGetChildByPath';
function  xmlGetNextNode(node:HXML):HXML; stdcall;
             external AppDll name 'xmlGetNextNode';
function  xmlGetName(h:HXML):PWideChar; stdcall;
             external AppDll name 'xmlGetName';
function  xmlGetParent(h:HXML):HXML; stdcall;
             external AppDll name 'xmlGetParent';
function  xmlGetText(h:HXML):PWideChar; stdcall;  // = getTextByIndex(HXML, 0)
             external AppDll name 'xmlGetText';
procedure xmlSetText(h:HXML;value:PWideChar); stdcall; // = setTextByIndex(HXML, LPCTSTR, 0)
             external AppDll name 'xmlSetText';
                      
function  xmlGetAttr(h:HXML;i:int):PWideChar; stdcall;
             external AppDll name 'xmlGetAttr';
function  xmlGetAttrName(h:HXML;i:int):PWideChar; stdcall;
             external AppDll name 'xmlGetAttrName';
function  xmlGetAttrValue(h:HXML;const attrName:PWideChar):PWideChar; stdcall;
             external AppDll name 'xmlGetAttrValue';
function  xmlGetAttrCount(h:HXML):int; stdcall;
             external AppDll name 'xmlGetAttrCount';
procedure xmlAddAttr(h:HXML;const attrName,attrValue:PWideChar); stdcall;
             external AppDll name 'xmlAddAttr';
procedure xmlAddAttrInt(h:HXML; const attrName:PWideChar;attrValue:int); stdcall;
             external AppDll name 'xmlAddAttrInt';

procedure xmlFree(arg:pointer); stdcall;
             external AppDll name 'xmlFree';

function  xmlIsDeclaration(node:HXML):boolean; stdcall;
             external AppDll name 'xmlIsDeclaration';
function  xmlToStringWithFormatting(node:HXML; var datalen:int):PWideChar; stdcall;
             external AppDll name 'xmlToStringWithFormatting';
function  xmlDeepCopy(node:HXML):HXML; stdcall;
             external AppDll name 'xmlDeepCopy';
procedure xmlSetAttrByIndex(node:HXML; i:int; value:PWideChar); stdcall;
             external AppDll name 'xmlSetAttrByIndex';
procedure xmlSetAttrByName(node:HXML; name:PWideChar; value:PWideChar); stdcall;
             external AppDll name 'xmlSetAttrByName';
function  xmlAddChildEx(parent:HXML; name:PWideChar; isDeclaration:boolean; n:XML_ELEMENT_POS):HXML; stdcall;
             external AppDll name 'xmlAddChildEx';
procedure xmlAddChildEx2(child:HXML; parent:HXML; n:XML_ELEMENT_POS); stdcall;
             external AppDll name 'xmlAddChildEx2';
function  xmlGetTextCount(node:HXML):int; stdcall;
             external AppDll name 'xmlGetTextCount';
function  xmlGetTextByIndex(node:HXML; i:int):PWideChar; stdcall;
             external AppDll name 'xmlGetTextByIndex';
procedure xmlAddText(node:HXML; txt:PWideChar; n:XML_ELEMENT_POS); stdcall;
             external AppDll name 'xmlAddText';
procedure xmlSetTextByIndex(node:HXML; i:int; txt:PWideChar); stdcall;
             external AppDll name 'xmlSetTextByIndex';
function  xmlGetClearCount(node:HXML):int; stdcall;
             external AppDll name 'xmlGetClearCount';
function  xmlGetClear(node:HXML; i:int; var openTag:PWideChar; var closeTag:PWideChar):PWideChar; stdcall;
             external AppDll name 'xmlGetClear';
procedure xmlAddClear(node:HXML; lpszValue:PWideChar; openTag:PWideChar; closeTag:PWideChar; n:XML_ELEMENT_POS); stdcall;
             external AppDll name 'xmlAddClear';
procedure xmlSetClear(node:HXML; i:int; lpszValue:PWideChar); stdcall;
             external AppDll name 'xmlSetClear';
function  xmlGetElementCount(node:HXML):int; stdcall;
             external AppDll name 'xmlGetElementCount';
function  xmlGetElement(node:HXML; n:XML_ELEMENT_POS; var _type:XML_ELEMENT_TYPE; 
                         var child:HXML;var value:PWideChar; var name:PWideChar;
                         var openTag:PWideChar; var closeTag:PWideChar):int; stdcall;
             external AppDll name 'xmlGetElement';

procedure xmlDeleteNodeContent(node:HXML); stdcall; // forces the deletion of the content of this node and the subtree
             external AppDll name 'xmlDeleteNodeContent';
procedure xmlDeleteAttrByIndex(node:HXML; i:int); stdcall;
             external AppDll name 'xmlDeleteAttrByIndex';
procedure xmlDeleteAttrByName(node:HXML; name:PWideChar); stdcall;
             external AppDll name 'xmlDeleteAttrByName';
procedure xmlDeleteText(node:HXML; i:int); stdcall;
             external AppDll name 'xmlDeleteText';
procedure xmlDeleteClear(node:HXML; i:int); stdcall;
             external AppDll name 'xmlDeleteClear';

function  xmlPositionOfChildByIndex(node:HXML; i:int):XML_ELEMENT_POS; stdcall;
             external AppDll name 'xmlPositionOfChildByIndex';
function  xmlPositionOfChildByNode(node:HXML; node1:HXML):XML_ELEMENT_POS; stdcall;
             external AppDll name 'xmlPositionOfChildByNode';
function  xmlPositionOfChildByName(node:HXML; name:PWideChar; i:int):XML_ELEMENT_POS; stdcall;
             external AppDll name 'xmlPositionOfChildByName';
function  xmlPositionOfText(node:HXML; i:int):XML_ELEMENT_POS; stdcall;
             external AppDll name 'xmlPositionOfText';
function  xmlPositionOfClear(node:HXML; i:int):XML_ELEMENT_POS; stdcall;
             external AppDll name 'xmlPositionOfClear';

function  xmlParseFile(filename:PAnsiChar; datalen:pint; tag:PAnsiChar):HXML; stdcall;
             external AppDll name 'xmlParseFile';
function  xmlToFile(node:HXML; filename:PAnsiChar; withformattiing:int):XMLError; stdcall;
             external AppDll name 'xmlToFile';

{$ENDIF}
