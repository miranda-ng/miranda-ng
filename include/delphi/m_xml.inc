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

const
  XML_API_SIZEOF_V1 = SizeOf(size_t)+26*sizeof(dword);

type
  TXML_API_A = record
    cbSize             :size_t;

    createNode         :function(const name, text:PAnsiChar; IsDeclaration:boolean):HXML;cdecl;
    destroyNode        :procedure(node:HXML);cdecl;

    parseString        :function(const str:PAnsiChar; datalen:pint; const tag:PAnsiChar):HXML;cdecl;
    toString           :function(node:HXML;datalen:pint):PAnsiChar;cdecl;

    addChild           :function(parent:HXML; const name,text:PAnsiChar):HXML;cdecl;
    addChild2          :procedure(child,parent:HXML);cdecl;
    copyNode           :function(parent:HXML):HXML;cdecl;
    getChild           :function(parent:HXML;number:int):HXML;cdecl;
    getChildCount      :function(h:HXML):int;cdecl;
    getChildByAttrValue:function(parent:HXML; const name,attrName,attrValue:PAnsiChar):HXML;cdecl;
    getFirstChild      :function(parent:HXML):HXML;cdecl;
    getNthChild        :function(parent:HXML; const name:PAnsiChar; i:int):HXML;cdecl;
    getNextChild       :function(parent:HXML; const name:PAnsiChar; i:pint):HXML;cdecl;
    getChildByPath     :function(parent:HXML; const path:PAnsiChar;createNodeIfMissing:boolean):HXML;cdecl;
    getNextNode        :function(node:HXML):HXML;cdecl;
    getName            :function(h:HXML):PAnsiChar;cdecl;
    getParent          :function(h:HXML):HXML;cdecl;
    getText            :function(h:HXML):PAnsiChar;cdecl;  // = getTextByIndex(HXML, 0)
    setText            :procedure(h:HXML;value:PAnsiChar);cdecl; // = setTextByIndex(HXML, LPCTSTR, 0)

    getAttr            :function(h:HXML;i:int):PAnsiChar;cdecl;
    getAttrName        :function(h:HXML;i:int):PAnsiChar;cdecl;
    getAttrValue       :function(h:HXML;const attrName:PAnsiChar):PAnsiChar;cdecl;
    getAttrCount       :function(h:HXML):int;cdecl;
    addAttr            :procedure(h:HXML;const attrName,attrValue:PAnsiChar);cdecl;
    addAttrInt         :procedure(h:HXML; const attrName:PAnsiChar;attrValue:int);cdecl;

    freeMem            :procedure(arg:pointer);cdecl;

  	// #if MIRANDA_VER >= 0x0900, methods added in XML API v2
    isDeclaration         :function(node:HXML):boolean;cdecl;
    toStringWithFormatting:function(node:HXML; var datalen:int):PAnsiChar;cdecl;
    deepCopy              :function(node:HXML):HXML;cdecl;
    setAttrByIndex        :procedure(node:HXML; i:int; value:PAnsiChar);cdecl;
    setAttrByName         :procedure(node:HXML; name:PAnsiChar; value:PAnsiChar);cdecl;
    addChildEx            :function(parent:HXML; name:PAnsiChar; isDeclaration:boolean;
                           n:XML_ELEMENT_POS):HXML;cdecl;
    addChildEx2           :procedure(child:HXML; parent:HXML; n:XML_ELEMENT_POS);cdecl;
    getTextCount          :function(node:HXML):int;cdecl;
    getTextByIndex        :function(node:HXML; i:int):PAnsiChar;cdecl;
    addText               :procedure(node:HXML; txt:PAnsiChar; n:XML_ELEMENT_POS);cdecl;
    setTextByIndex        :procedure(node:HXML; i:int; txt:PAnsiChar);cdecl;
    getClearCount         :function(node:HXML):int;cdecl;
    getClear              :function(node:HXML; i:int; var openTag:PAnsiChar;
                           var closeTag:PAnsiChar):PAnsiChar;cdecl;
    addClear              :procedure(node:HXML; lpszValue:PAnsiChar; openTag:PAnsiChar;
                           closeTag:PAnsiChar; n:XML_ELEMENT_POS);cdecl;
    setClear              :procedure(node:HXML; i:int; lpszValue:PAnsiChar);cdecl;
    getElementCount       :function(node:HXML):int;cdecl;
    getElement            :function(node:HXML; n:XML_ELEMENT_POS; var _type:XML_ELEMENT_TYPE;
                           var child:HXML;var value:PAnsiChar; var name:PAnsiChar;
                           var openTag:PAnsiChar; var closeTag:PAnsiChar):int;cdecl;
    // With getElement() it's possible to enumerate all the different contents
    // (attribute,child,text, clear) of the current node. The order is reflecting the order
    // of the original file/string. NOTE: 0 <= i < getElementCount().
    // type, child, value, name, openTag, closeTag will be filled on return, depending on type:
    // for XML_ELEM_TYPE_CHILD    , child is valid;
    // for XML_ELEM_TYPE_ATTRIBUTE, name and value are valid;
    // for XML_ELEM_TYPE_TEXT     , value is valid;
    // for XML_ELEM_TYPE_CLEAR    , value, openTag and closeTag are valid.

    deleteNodeContent:procedure(node:HXML);cdecl; // forces the deletion of the content of this node and the subtree
    deleteAttrByIndex:procedure(node:HXML; i:int);cdecl;
    deleteAttrByName :procedure(node:HXML; name:PAnsiChar);cdecl;
    deleteText       :procedure(node:HXML; i:int);cdecl;
    deleteClear      :procedure(node:HXML; i:int);cdecl;

    positionOfChildByIndex:function(node:HXML; i:int):XML_ELEMENT_POS;cdecl;
    positionOfChildByNode :function(node:HXML; node1:HXML):XML_ELEMENT_POS;cdecl;
    positionOfChildByName :function(node:HXML; name:PAnsiChar; i:int):XML_ELEMENT_POS;cdecl;
    positionOfText        :function(node:HXML; i:int):XML_ELEMENT_POS;cdecl;
    positionOfClear       :function(node:HXML; i:int):XML_ELEMENT_POS;cdecl;

    parseFile:function(filename:PAnsiChar; datalen:pint; tag:PAnsiChar):HXML;
    toFile   :function(node:HXML; filename:PAnsiChar; withformattiing:int):XMLError;
  end;

  TXML_API_W = record
    cbSize             :size_t;

    createNode         :function(const name, text:PWideChar; IsDeclaration:boolean):HXML;cdecl;
    destroyNode        :procedure(node:HXML);cdecl;

    parseString        :function(const str:PWideChar; datalen:pint; const tag:PWideChar):HXML;cdecl;
    toString           :function(node:HXML;datalen:pint):PWideChar;cdecl;

    addChild           :function(parent:HXML; const name,text:PWideChar):HXML;cdecl;
    addChild2          :procedure(child,parent:HXML);cdecl;
    copyNode           :function(parent:HXML):HXML;cdecl;
    getChild           :function(parent:HXML;number:int):HXML;cdecl;
    getChildCount      :function(h:HXML):int;cdecl;
    getChildByAttrValue:function(parent:HXML; const name,attrName,attrValue:PWideChar):HXML;cdecl;
    getFirstChild      :function(parent:HXML):HXML;cdecl;
    getNthChild        :function(parent:HXML; const name:PWideChar; i:int):HXML;cdecl;
    getNextChild       :function(parent:HXML; const name:PWideChar; i:pint):HXML;cdecl;
    getChildByPath     :function(parent:HXML; const path:PWideChar;createNodeIfMissing:boolean):HXML;cdecl;
    getNextNode        :function(node:HXML):HXML;cdecl;
    getName            :function(h:HXML):PWideChar;cdecl;
    getParent          :function(h:HXML):HXML;cdecl;
    getText            :function(h:HXML):PWideChar;cdecl;  // = getTextByIndex(HXML, 0)
    setText            :procedure(h:HXML;value:PWideChar);cdecl; // = setTextByIndex(HXML, LPCTSTR, 0)

    getAttr            :function(h:HXML;i:int):PWideChar;cdecl;
    getAttrName        :function(h:HXML;i:int):PWideChar;cdecl;
    getAttrValue       :function(h:HXML;const attrName:PWideChar):PWideChar;cdecl;
    getAttrCount       :function(h:HXML):int;cdecl;
    addAttr            :procedure(h:HXML;const attrName,attrValue:PWideChar);cdecl;
    addAttrInt         :procedure(h:HXML; const attrName:PWideChar;attrValue:int);cdecl;

    freeMem            :procedure(arg:pointer);cdecl;

  	// #if MIRANDA_VER >= 0x0900, methods added in XML API v2
    isDeclaration         :function(node:HXML):boolean;cdecl;
    toStringWithFormatting:function(node:HXML; var datalen:int):PWideChar;cdecl;
    deepCopy              :function(node:HXML):HXML;cdecl;
    setAttrByIndex        :procedure(node:HXML; i:int; value:PWideChar);cdecl;
    setAttrByName         :procedure(node:HXML; name:PWideChar; value:PWideChar);cdecl;
    addChildEx            :function(parent:HXML; name:PWideChar; isDeclaration:boolean;
                           n:XML_ELEMENT_POS):HXML;cdecl;
    addChildEx2           :procedure(child:HXML; parent:HXML; n:XML_ELEMENT_POS);cdecl;
    getTextCount          :function(node:HXML):int;cdecl;
    getTextByIndex        :function(node:HXML; i:int):PWideChar;cdecl;
    addText               :procedure(node:HXML; txt:PWideChar; n:XML_ELEMENT_POS);cdecl;
    setTextByIndex        :procedure(node:HXML; i:int; txt:PWideChar);cdecl;
    getClearCount         :function(node:HXML):int;cdecl;
    getClear              :function(node:HXML; i:int; var openTag:PWideChar;
                           var closeTag:PWideChar):PWideChar;cdecl;
    addClear              :procedure(node:HXML; lpszValue:PWideChar; openTag:PWideChar;
                           closeTag:PWideChar; n:XML_ELEMENT_POS);cdecl;
    setClear              :procedure(node:HXML; i:int; lpszValue:PWideChar);cdecl;
    getElementCount       :function(node:HXML):int;cdecl;
    getElement            :function(node:HXML; n:XML_ELEMENT_POS; var _type:XML_ELEMENT_TYPE;
                           var child:HXML;var value:PWideChar; var name:PWideChar;
                           var openTag:PWideChar; var closeTag:PWideChar):int;cdecl;
    // With getElement() it's possible to enumerate all the different contents
    // (attribute,child,text, clear) of the current node. The order is reflecting the order
    // of the original file/string. NOTE: 0 <= i < getElementCount().
    // type, child, value, name, openTag, closeTag will be filled on return, depending on type:
    // for XML_ELEM_TYPE_CHILD    , child is valid;
    // for XML_ELEM_TYPE_ATTRIBUTE, name and value are valid;
    // for XML_ELEM_TYPE_TEXT     , value is valid;
    // for XML_ELEM_TYPE_CLEAR    , value, openTag and closeTag are valid.

    deleteNodeContent:procedure(node:HXML);cdecl; // forces the deletion of the content of this node and the subtree
    deleteAttrByIndex:procedure(node:HXML; i:int);cdecl;
    deleteAttrByName :procedure(node:HXML; name:PWideChar);cdecl;
    deleteText       :procedure(node:HXML; i:int);cdecl;
    deleteClear      :procedure(node:HXML; i:int);cdecl;

    positionOfChildByIndex:function(node:HXML; i:int):XML_ELEMENT_POS;cdecl;
    positionOfChildByNode :function(node:HXML; node1:HXML):XML_ELEMENT_POS;cdecl;
    positionOfChildByName :function(node:HXML; name:PWideChar; i:int):XML_ELEMENT_POS;cdecl;
    positionOfText        :function(node:HXML; i:int):XML_ELEMENT_POS;cdecl;
    positionOfClear       :function(node:HXML; i:int):XML_ELEMENT_POS;cdecl;

    parseFile:function(filename:PAnsiChar; datalen:pint; tag:PAnsiChar):HXML;
    toFile   :function(node:HXML; filename:PAnsiChar; withformattiing:int):XMLError;
  end;

// every protocol should declare this variable to use the XML API
//const
// extern XML_API xi;

const
{
a service to obtain the XML API 

wParam = 0;
lParam = (LPARAM)(XML_API*).

returns TRUE if all is Ok, and FALSE otherwise
}
  MS_SYSTEM_GET_XI:PAnsiChar = 'Miranda/System/GetXmlApi';
(*
__forceinline int mir_getXI( XML_API* dest )
{
  dest->cbSize = sizeof(*dest);
  return CallService( MS_SYSTEM_GET_XI, 0, (LPARAM)dest );
}
*)
{$ENDIF}
