{ ############################################################################ }
{ #                                                                          # }
{ #  MirandaNG HistoryToDB Plugin v2.5                                       # }
{ #                                                                          # }
{ #  License: GPLv3                                                          # }
{ #                                                                          # }
{ #  Author: Grigorev Michael (icq: 161867489, email: sleuthhound@gmail.com) # }
{ #                                                                          # }
{ ############################################################################ }

//*****************************************************************//
//                                                                 //
//  TMapStream                                                     //
//  Copyright© BrandsPatch LLC                                     //
//  http://www.explainth.at                                        //
//                                                                 //
//  All Rights Reserved                                            //
//                                                                 //
//  Permission is granted to use, modify and redistribute          //
//  the code in this Delphi unit on the condition that this        //
//  notice is retained unchanged.                                  //
//                                                                 //
//  BrandsPatch declines all responsibility for any losses,        //
//  direct or indirect, that may arise  as a result of using       //
//  this code.                                                     //
//                                                                 //
//*****************************************************************//
unit MapStream;

interface

uses Windows, SysUtils, Classes, SyncObjs;

const MAXINTMINUS = MAXINT - 1;

type TMapBytes = array[0..MAXINTMINUS] of Byte;
     PMapBytes = ^TMapBytes;

type TMapStream = class(TObject)
private
  FHandle: THandle;
  FPosition: Integer;
  FSize: Integer;
  FTimeOut: Integer;
  procedure SetPosition(Value: Integer);
  function CountGood(Count: Integer):Boolean;
  function GrabEvent: Boolean;
  procedure ReleaseEvent;
protected
  FEvent: TEvent;
  FMemory: PMapBytes;
public
  property Position: Integer read FPosition write SetPosition;
  constructor CreateEx(const AName: String; ASize,ATimeOut: Integer);
  destructor Destroy; override;
  function Clear: Boolean;
  function CopyFrom(AStream: TStream;Count: Integer): Boolean;
  function ReadBuffer(P: Pointer; Count: Integer): Boolean;
  function WriteBuffer(P: Pointer; Count: Integer): Boolean;
end;

type ENoMapping = class(Exception);

implementation

// MapStream Create & Destroy
constructor TMapStream.CreateEx(const AName:String;ASize,ATimeOut:Integer);
begin
  inherited Create;
  FSize := ASize;
  FTimeOut := ATimeOut;
  if (FSize < 1) or (FSize > MAXINTMINUS) then FSize:=MAXWORD;
  if (FTimeOut < 1) or (FTimeOut > 5000) then FTimeOut:=2000;
  //2000ms timeout for safety
  FHandle:=CreateFileMapping($FFFFFFFF,nil,PAGE_READWRITE,0,FSize,PChar(AName));
  //See the Windows Kernel32 CreateFileMapping function for information
  if (FHandle = 0) then ENoMapping.Create(Format('%s file mapping failed.',[AName]))
  else begin
    FMemory:=MapViewOfFile(FHandle,FILE_MAP_ALL_ACCESS,0,0,0);

    FEvent:=TEvent.Create(nil,True,True,Format('ExplainThat_%s_MAP',[AName]));
    {1. map a view of the file mapping into the address
        space of the parent application

    2.  create an event to prevent simultaneous access
        to the map by different apps. We try to make the
        event name unique so we don't end up with a handle to
        an unrelated event that already exists
    }
  end;
end;

destructor TMapStream.Destroy;
begin
  UnMapViewOfFile(FMemory);
  CloseHandle(FHandle);
  FEvent.Free;
  inherited;
end;

function TMapStream.CountGood(Count:Integer):Boolean;
begin
  Result:=(FPosition + Count < FSize);
end;

function TMapStream.GrabEvent:Boolean;
begin
  Result:=True;
  with FEvent do
  begin
    case WaitFor(FTimeOut) of
      wrSignaled:ReSetEvent;
      {locks the event for exclusive use by this app. Funny name, ReSetEvent, not
       our choice!}
      else Result:=False;
    end;
  end;
end;

procedure TMapStream.ReleaseEvent;
begin
  FEvent.SetEvent; //unlock the event so other apps can use it
end;

// MapStream Manipulation
function TMapStream.Clear:Boolean;
begin
  if GrabEvent then
  try
    FillChar(FMemory^[0],FSize,0);
    FPosition:=0;
    Result:=True;
  finally ReleaseEvent end else Result:=False;
end;

function TMapStream.CopyFrom(AStream:TStream;Count:Integer):Boolean;

  function SizeGood:Boolean;
  var i,ASize:Integer;
  begin
    ASize:=AStream.Size;
    if (Count = 0) or (Count > ASize) then
    begin
      Count:=ASize;
      AStream.Position:=0;
    end;
    Result:=(FPosition + Count < FSize);
    {make sure the copy block is not too big. Incidentally, also make Count = 0
     as in Delphi.TStream}
  end;

begin
  if SizeGood and GrabEvent then
  try
    AStream.ReadBuffer(Byte(FMemory^[FPosition]),Count);
    Result:=True;
  finally ReleaseEvent end else Result:=False;
end;

function TMapStream.ReadBuffer(P:Pointer;Count:Integer):Boolean;
begin
  if CountGood(Count) and GrabEvent then
  try
    Move(FMemory^[FPosition],P^,Count);
    inc(FPosition,Count);
    Result:=True;
  finally ReleaseEvent end else Result:=False;
end;

function TMapStream.WriteBuffer(P:Pointer;Count:Integer):Boolean;
begin
  if CountGood(Count) and GrabEvent then
  try
    Move(P^,FMemory^[FPosition],Count);
    inc(FPosition,Count);
    Result:=True;
  finally ReleaseEvent end else Result:=False;
end;

procedure TMapStream.SetPosition(Value:Integer);
begin
  if (Value < FSize) and (Value >= 0) then FPosition:=Value;
end;

end.
