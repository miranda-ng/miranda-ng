{$IFNDEF M_W7UI}
{$DEFINE M_W7UI}

const
	MIS_ICOLIB   = 0;
	MIS_GENERAL  = 1;
	MIS_PROTOCOL = 2;

type
  PMIRANDAJUMPLISTITEM = ^TMIRANDAJUMPLISTITEM;
  TMIRANDAJUMPLISTITEM = record
    iconSource:int;
    iconName  :pAnsiChar;
    iconIdx   :int;
    szTitle   :pWideChar;
    szPrefix  :pWideChar;
    szArgument:pWideChar;
  end;

const
// Force jumplist rebuild
  MS_JUMPLIST_REBUILD:pAnsiChar = 'w7/JumpList/Rebuild';

// ---
  ME_JUMPLIST_BUILDCATEGORIES:pAnsiChar = 'w7/JumpList/BuildCategories';

// lParam = (WCHAR *)category name
  ME_JUMPLIST_BUILDITEMS:pAnsiChar = 'w7/JumpList/BuildItems';

// lParam = (WCHAR *)category name
  MS_JUMPLIST_ADDCATEGORY:pAnsiChar = 'w7/JumpList/AddCategory';

// lParam = (MIRANDAJUMPLISTITEM *)item
  MS_JUMPLIST_ADDITEM:pAnsiChar = 'w7/JumpList/AddItem';

// wParam = prefix
// lParam = argument
  ME_JUMPLIST_PROCESS:pAnsiChar = 'w7/JumpList/Process';

procedure MJumpList_AddCategory(name:pWideChar);
begin
	CallService(MS_JUMPLIST_ADDCATEGORY, 0, LPARAM(name));
end;

procedure MJumpList_AddItem(mir_icon:pAnsiChar; title, prefix, argument:pWideChar); overload;
var
  item:TMIRANDAJUMPLISTITEM;
begin
	item.iconSource:=MIS_ICOLIB;
	item.iconName  :=mir_icon;
	item.iconIdx   :=0;
	item.szTitle   :=title;
	item.szPrefix  :=prefix;
	item.szArgument:=argument;
	CallService(MS_JUMPLIST_ADDITEM, 0, LPARAM(@item));
end;

procedure MJumpList_AddItem(skinicon:int; title, prefix, argument:pWideChar); overload;
var
  item:TMIRANDAJUMPLISTITEM;
begin
	item.iconSource:=MIS_GENERAL;
	item.iconName  :=0;
	item.iconIdx   :=skinicon;
	item.szTitle   :=title;
	item.szPrefix  :=prefix;
	item.szArgument:=argument;
	CallService(MS_JUMPLIST_ADDITEM, 0, LPARAM(@item));
end;

procedure MJumpList_AddItem(proto:pansiChar; skinicon:int; title, prefix, argument:pWideChar); overload;
var
  item:TMIRANDAJUMPLISTITEM;
begin
	item.iconSource:=MIS_PROTOCOL;
	item.iconName  :=proto;
	item.iconIdx   :=skinicon;
	item.szTitle   :=title;
	item.szPrefix  :=prefix;
	item.szArgument:=argument;
	CallService(MS_JUMPLIST_ADDITEM, 0, LPARAM(@item));
end;

{$ENDIF}
