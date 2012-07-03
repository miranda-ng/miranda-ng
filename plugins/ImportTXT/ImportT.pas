unit ImportT;
interface

uses windows;

type

 RHeader = record
   Pattern:String;
   Incoming:integer;
   Outgoing:integer;
   InNick:integer;
   OutNick:integer;
   InUID:integer;
   OutUID:integer;
   end;

 RPreMessage = record
   PreRN:integer;
   AfterRN:integer;
   PreSP:integer;
   AfterSP:integer;
   end;

 RMessage = record
   Pattern:String;
   Incoming:String;
   Outgoing:String;
   Direction:integer;
   Day:integer;
   Month:integer;
   Year:integer;
   Hours:integer;
   Minutes:integer;
   Seconds:integer;
   end;
   
 RFileName = record
   Pattern:String;
   InNick:integer;
   OutNick:integer;
   InUID:integer;
   OutUID:integer;
   end;

 pRTxtPattern = ^RTxtPattern;
 RTxtPattern = record
   Name:String;
   IType:byte; //1 -text,2- binary, 3 - ...
   Charset:word;
   Codepage:Cardinal;
   DefExtension:String;
   BinProc:word;
   UseHeader:Byte;
   UseFileName:ByteBool;
   UsePreMsg:ByteBool;
   Msg:RMessage;
   Header:RHeader;
   PreMsg:RPreMessage;
   FName:RFileName;
   end;
   
const
  inANSI = 1;
  inUTF8 = 2;
  inUCS2 = 3;

type
  PDestProto = ^TDestProto;
  TDestProto = record
    ProtoName: AnsiString;
    ProtoUID: WideString;
    ProtoNick: WideString;
  end;

type
  PDestContact = ^TDestContact;
  TDestContact = record
    hContact: THandle;
    ProtoName: AnsiString;
    ContactUID: AnsiString;
    ContactNick: WideString;
  end;

implementation  

end.
