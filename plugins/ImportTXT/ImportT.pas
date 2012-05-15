unit ImportT;
interface

type

 RHeader = record
   Pattern:string;
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
   Pattern:string;
   Incoming:string;
   Outgoing:string;
   Direction:integer;
   Day:integer;
   Month:integer;
   Year:integer;
   Hours:integer;
   Minutes:integer;
   Seconds:integer;
   end;
   
 RFileName = record
   Pattern:string;
   InNick:integer;
   OutNick:integer;
   InUID:integer;
   OutUID:integer;
   end;

 RTxtPattern = record
   Name:string;
   IType:byte; //1 -text,2- binary, 3 - ...
   Charset:word;
   Codepage:Cardinal;
   DefExtension:string;
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

type PDestProto = ^TDestProto;
     TDestProto = record
      ProtoName: string;
      ProtoUID: string;
      ProtoNick: string;
     end;

type PDestContact = ^TDestContact;
     TDestContact = record
      hContact: THandle;
      ProtoName: string;
      ContactUID: string;
      ContactNick: string;
     end;

implementation  

end.
