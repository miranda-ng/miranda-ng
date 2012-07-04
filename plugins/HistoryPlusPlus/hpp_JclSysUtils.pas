//****************************************************************//
// This is cut-and-pasted version of JclSysUtils.pas library from //
// JEDI Code Library (JCL). Copyright (c) see contributors        //
//                                                                //
// For use with History++ plugin                                  //
//                                                                //
// This unit is not covered under GPL license,                    //
// actual license is provided below                               //
//****************************************************************//

{**************************************************************************************************}
{                                                                                                  }
{ Project JEDI Code Library (JCL)                                                                  }
{                                                                                                  }
{ The contents of this file are subject to the Mozilla Public License Version 1.1 (the "License"); }
{ you may not use this file except in compliance with the License. You may obtain a copy of the    }
{ License at http://www.mozilla.org/MPL/                                                           }
{                                                                                                  }
{ Software distributed under the License is distributed on an "AS IS" basis, WITHOUT WARRANTY OF   }
{ ANY KIND, either express or implied. See the License for the specific language governing rights  }
{ and limitations under the License.                                                               }
{                                                                                                  }
{ The Original Code is JclSysUtils.pas.                                                            }
{                                                                                                  }
{ The Initial Developer of the Original Code is Marcel van Brakel.                                 }
{ Portions created by Marcel van Brakel are Copyright (C) Marcel van Brakel. All rights reserved.  }
{                                                                                                  }
{ Contributors:                                                                                    }
{   Alexander Radchenko,                                                                           }
{   Andreas Hausladen (ahuser)                                                                     }
{   Anthony Steele                                                                                 }
{   Bernhard Berger                                                                                }
{   Heri Bender                                                                                    }
{   Jeff                                                                                           }
{   Jeroen Speldekamp                                                                              }
{   Marcel van Brakel                                                                              }
{   Peter Friese                                                                                   }
{   Petr Vones (pvones)                                                                            }
{   Python                                                                                         }
{   Robert Marquardt (marquardt)                                                                   }
{   Robert R. Marsh                                                                                }
{   Robert Rossmair (rrossmair)                                                                    }
{   Rudy Velthuis                                                                                  }
{   Uwe Schuster (uschuster)                                                                       }
{   Wayne Sherman                                                                                  }
{                                                                                                  }
{**************************************************************************************************}
{                                                                                                  }
{ Description: Various pointer and class related routines.                                         }
{                                                                                                  }
{**************************************************************************************************}

// Last modified: $Date: 2005/12/26 20:30:07 $
// For history see end of file

unit hpp_JclSysUtils;

interface

uses  Windows, Classes;

type
  TDynByteArray  = array of Byte;
  Float = Extended;
  PFloat = ^Float;


// Binary search
function SearchSortedList(List: TList; SortFunc: TListSortCompare; Item: Pointer;
  Nearest: Boolean = False): Integer;

type
  TUntypedSearchCompare = function(Param: Pointer; ItemIndex: Integer; const Value): Integer;

function SearchSortedUntyped(Param: Pointer; ItemCount: Integer; SearchFunc: TUntypedSearchCompare;
  const Value; Nearest: Boolean = False): Integer;

// Dynamic array sort and search routines
type
  TDynArraySortCompare = function (Item1, Item2: Pointer): Integer;

procedure SortDynArray(const ArrayPtr: Pointer; ElementSize: Cardinal; SortFunc: TDynArraySortCompare);
// Usage: SortDynArray(Array, SizeOf(Array[0]), SortFunction);
function SearchDynArray(const ArrayPtr: Pointer; ElementSize: Cardinal; SortFunc: TDynArraySortCompare;
  ValuePtr: Pointer; Nearest: Boolean = False): Integer;
// Usage: SearchDynArray(Array, SizeOf(Array[0]), SortFunction, @SearchedValue);

{ Various compare functions for basic types }

function DynArrayCompareByte(Item1, Item2: Pointer): Integer;
function DynArrayCompareShortInt(Item1, Item2: Pointer): Integer;
function DynArrayCompareWord(Item1, Item2: Pointer): Integer;
function DynArrayCompareSmallInt(Item1, Item2: Pointer): Integer;
function DynArrayCompareInteger(Item1, Item2: Pointer): Integer;
function DynArrayCompareCardinal(Item1, Item2: Pointer): Integer;
function DynArrayCompareInt64(Item1, Item2: Pointer): Integer;

function DynArrayCompareSingle(Item1, Item2: Pointer): Integer;
function DynArrayCompareDouble(Item1, Item2: Pointer): Integer;
function DynArrayCompareExtended(Item1, Item2: Pointer): Integer;
function DynArrayCompareFloat(Item1, Item2: Pointer): Integer;

function DynArrayCompareAnsiString(Item1, Item2: Pointer): Integer;
function DynArrayCompareAnsiText(Item1, Item2: Pointer): Integer;
function DynArrayCompareString(Item1, Item2: Pointer): Integer;
function DynArrayCompareText(Item1, Item2: Pointer): Integer;

implementation

uses  SysUtils;

//=== Binary search ==========================================================

function SearchSortedList(List: TList; SortFunc: TListSortCompare; Item: Pointer; Nearest: Boolean): Integer;
var
  L, H, I, C: Integer;
  B: Boolean;
begin
  Result := -1;
  if List <> nil then
  begin
    L := 0;
    H := List.Count - 1;
    B := False;
    while L <= H do
    begin
      I := (L + H) shr 1;
      C := SortFunc(List.List[I], Item);
      if C < 0 then
        L := I + 1
      else
      begin
        H := I - 1;
        if C = 0 then
        begin
          B := True;
          L := I;
        end;
      end;
    end;
    if B then
      Result := L
    else
    if Nearest and (H >= 0) then
      Result := H;
  end;
end;

function SearchSortedUntyped(Param: Pointer; ItemCount: Integer; SearchFunc: TUntypedSearchCompare;
  const Value; Nearest: Boolean): Integer;
var
  L, H, I, C: Integer;
  B: Boolean;
begin
  Result := -1;
  if ItemCount > 0 then
  begin
    L := 0;
    H := ItemCount - 1;
    B := False;
    while L <= H do
    begin
      I := (L + H) shr 1;
      C := SearchFunc(Param, I, Value);
      if C < 0 then
        L := I + 1
      else
      begin
        H := I - 1;
        if C = 0 then
        begin
          B := True;
          L := I;
        end;
      end;
    end;
    if B then
      Result := L
    else
    if Nearest and (H >= 0) then
      Result := H;
  end;
end;

//=== Dynamic array sort and search routines =================================

procedure SortDynArray(const ArrayPtr: Pointer; ElementSize: Cardinal; SortFunc: TDynArraySortCompare);
var
  TempBuf: TDynByteArray;

  function ArrayItemPointer(Item: Integer): Pointer;
  begin
    Result := Pointer(uint_ptr(ArrayPtr) + (uint_ptr(Item) * ElementSize));
  end;

  procedure QuickSort(L, R: Integer);
  var
    I, J, T: Integer;
    P, IPtr, JPtr: Pointer;
  begin
    repeat
      I := L;
      J := R;
      P := ArrayItemPointer((L + R) shr 1);
      repeat
        while SortFunc(ArrayItemPointer(I), P) < 0 do
          Inc(I);
        while SortFunc(ArrayItemPointer(J), P) > 0 do
          Dec(J);
        if I <= J then
        begin
          IPtr := ArrayItemPointer(I);
          JPtr := ArrayItemPointer(J);
          case ElementSize of
            SizeOf(Byte):
              begin
                T := PByte(IPtr)^;
                PByte(IPtr)^ := PByte(JPtr)^;
                PByte(JPtr)^ := T;
              end;
            SizeOf(Word):
              begin
                T := PWord(IPtr)^;
                PWord(IPtr)^ := PWord(JPtr)^;
                PWord(JPtr)^ := T;
              end;
            SizeOf(Integer):
              begin
                T := PInteger(IPtr)^;
                PInteger(IPtr)^ := PInteger(JPtr)^;
                PInteger(JPtr)^ := T;
              end;
          else
            Move(IPtr^, TempBuf[0], ElementSize);
            Move(JPtr^, IPtr^, ElementSize);
            Move(TempBuf[0], JPtr^, ElementSize);
          end;
          if P = IPtr then
            P := JPtr
          else
          if P = JPtr then
            P := IPtr;
          Inc(I);
          Dec(J);
        end;
      until I > J;
      if L < J then
        QuickSort(L, J);
      L := I;
    until I >= R;
  end;

begin
  if ArrayPtr <> nil then
  begin
    SetLength(TempBuf, ElementSize);
    QuickSort(0, PInteger(uint_ptr(ArrayPtr) - SizeOf(pointer))^ - 1); //!!!!
  end;
end;

function SearchDynArray(const ArrayPtr: Pointer; ElementSize: Cardinal; SortFunc: TDynArraySortCompare;
  ValuePtr: Pointer; Nearest: Boolean): Integer;
var
  L, H, I, C: Integer;
  B: Boolean;
begin
  Result := -1;
  if ArrayPtr <> nil then
  begin
    L := 0;
    H := PInteger(uint_ptr(ArrayPtr) - SizeOf(pointer))^ - 1; //!!!!
    B := False;
    while L <= H do
    begin
      I := (L + H) shr 1;
      C := SortFunc(Pointer(uint_ptr(ArrayPtr) + (uint_ptr(I) * ElementSize)), ValuePtr);
      if C < 0 then
        L := I + 1
      else
      begin
        H := I - 1;
        if C = 0 then
        begin
          B := True;
          L := I;
        end;
      end;
    end;
    if B then
      Result := L
    else
    if Nearest and (H >= 0) then
      Result := H;
  end;
end;

{ Various compare functions for basic types }

function DynArrayCompareByte(Item1, Item2: Pointer): Integer;
begin
  Result := PByte(Item1)^ - PByte(Item2)^;
end;

function DynArrayCompareShortInt(Item1, Item2: Pointer): Integer;
begin
  Result := PShortInt(Item1)^ - PShortInt(Item2)^;
end;

function DynArrayCompareWord(Item1, Item2: Pointer): Integer;
begin
  Result := PWord(Item1)^ - PWord(Item2)^;
end;

function DynArrayCompareSmallInt(Item1, Item2: Pointer): Integer;
begin
  Result := PSmallInt(Item1)^ - PSmallInt(Item2)^;
end;

function DynArrayCompareInteger(Item1, Item2: Pointer): Integer;
begin
  Result := PInteger(Item1)^ - PInteger(Item2)^;
end;

function DynArrayCompareCardinal(Item1, Item2: Pointer): Integer;
begin
  Result := PInteger(Item1)^ - PInteger(Item2)^;
end;

function DynArrayCompareInt64(Item1, Item2: Pointer): Integer;
begin
  Result := PInt64(Item1)^ - PInt64(Item2)^;
end;

function DynArrayCompareSingle(Item1, Item2: Pointer): Integer;
begin
  if PSingle(Item1)^ < PSingle(Item2)^ then
    Result := -1
  else
  if PSingle(Item1)^ > PSingle(Item2)^ then
    Result := 1
  else
    Result := 0;
end;

function DynArrayCompareDouble(Item1, Item2: Pointer): Integer;
begin
  if PDouble(Item1)^ < PDouble(Item2)^ then
    Result := -1
  else
  if PDouble(Item1)^ > PDouble(Item2)^ then
    Result := 1
  else
    Result := 0;
end;

function DynArrayCompareExtended(Item1, Item2: Pointer): Integer;
begin
  if PExtended(Item1)^ < PExtended(Item2)^ then
    Result := -1
  else
  if PExtended(Item1)^ > PExtended(Item2)^ then
    Result := 1
  else
    Result := 0;
end;

function DynArrayCompareFloat(Item1, Item2: Pointer): Integer;
begin
  if PFloat(Item1)^ < PFloat(Item2)^ then
    Result := -1
  else
  if PFloat(Item1)^ > PFloat(Item2)^ then
    Result := 1
  else
    Result := 0;
end;

function DynArrayCompareAnsiString(Item1, Item2: Pointer): Integer;
begin
  Result := AnsiCompareStr(PAnsiString(Item1)^, PAnsiString(Item2)^);
end;

function DynArrayCompareAnsiText(Item1, Item2: Pointer): Integer;
begin
  Result := AnsiCompareText(PAnsiString(Item1)^, PAnsiString(Item2)^);
end;

function DynArrayCompareString(Item1, Item2: Pointer): Integer;
begin
  Result := CompareStr(PAnsiString(Item1)^, PAnsiString(Item2)^);
end;

function DynArrayCompareText(Item1, Item2: Pointer): Integer;
begin
  Result := CompareText(PAnsiString(Item1)^, PAnsiString(Item2)^);
end;

end.
