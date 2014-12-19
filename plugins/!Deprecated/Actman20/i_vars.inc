{variables}
type
  pHKList = ^tHKList;
  tHKList  = array [0..1023] of tHKRecord;
  pActList = ^tActList;
  tActList = array [0..1023] of tHKAction;
const
  StructDelim = #7;
const
  HKListPage  = 8;
  ActListPage = 16;
var
  MaxGroups :cardinal=0; // current array size
  MaxActions:cardinal=0; // current array size
var
  GroupList :pHKList =nil;
  ActionList:pActList=nil;
var
  NoDescription:PWideChar;
var
  NewGroupList :pHKList  = nil;
  NewActionList:pActList = nil;
  NewMaxGroups :cardinal;
  NewMaxActions:cardinal;
var
  xmlfilename:array [0..511] of WideChar;
var
  fCLfilter:boolean   = true;
  fCLformat:pWideChar = nil;
var
  ApiCard:tmApiCard;