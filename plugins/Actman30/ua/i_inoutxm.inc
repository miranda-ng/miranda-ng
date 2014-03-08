{}
var
  xmlparser:TXML_API_W;

const
  ioAction         :PWideChar = 'Action';
  ioUA             :PWideChar = 'UA';

  ioName           :PWideChar = 'name';

  ioTwoState       :PWideChar = 'twostate';
  ioSaveState      :PWideChar = 'savestate';

  ioHotkey         :PWideChar = 'Hotkey';
  ioToolbar        :PWideChar = 'Toolbar';
  ioTabSRMM        :PWideChar = 'TabSRMM';
  ioMenuItem       :PWideChar = 'Menu';

  ioTooltip        :PWideChar = 'tooltip';
  ioTooltipPressed :PWideChar = 'tt_pressed';

  ioType           :PWideChar = 'type';
  ioMenuPopup      :PWideChar = 'Popup';
  ioMenuName       :PWideChar = 'Name';
  ioMenuShow       :PWideChar = 'Show';
  ioMenuUsed       :PWideChar = 'Used';
  ioMenuSeparated  :PWideChar = 'Separated';
  ioNoTranslate    :PWideChar = 'NoTranslate';


function ImportMenuItems(node:HXML;var MenuItem:tUAMenuItem):integer;
begin
  result:=0;

  with xmlparser do
  begin
    with MenuItem do
    begin
      menu_opt:=0;
    // popup
      StrDupW(szMenuPopup,getAttrValue(node,ioMenuPopup));
    // name
      StrDupW(szMenuNameVars,getAttrValue(node,ioMenuName));
    // show
      StrDupW(szMenuShowWhenVars,getAttrValue(node,ioMenuShow));
    // used
      if StrToInt(getAttrValue(node,ioMenuUsed))<>0 then
        menu_opt:=menu_opt or UAF_MENUUSE;
    // separated
      if StrToInt(getAttrValue(node,ioMenuSeparated))<>0 then
        menu_opt:=menu_opt or UAF_MENUSEP;
    // no translate
      if StrToInt(getAttrValue(node,ioNoTranslate))<>0 then
        menu_opt:=menu_opt or UAF_NOTRANS;
    end;
  end;
end;

function ImportUAction(actnode:HXML;var UA:tMyActionItem):integer;
var
  num,i:integer;
  sub:HXML;
begin
  result:=0;
  if actnode=0 then exit;

  with xmlparser do
  begin
    // we don't need that node as is, just it's child for UA
//    actnode:=GetNthChild(actnode,ioUA,0);

    UA.flags:=0;
    // ----- Common -----
    if StrToInt(getAttrValue(actnode,ioTwoState))<>0 then
      UA.flags:=UA.flags or UAF_2STATE;

    if StrToInt(getAttrValue(actnode,ioSaveState))<>0 then
      UA.flags:=UA.flags or UAF_SAVESTATE;

  //      sub:=AddChild(actnode,ioRegister,nil);
    if StrToInt(getAttrValue(actnode,ioHotkey))<>0 then
      UA.flags:=UA.flags or UAF_REGHOTKEY;
    if StrToInt(getAttrValue(actnode,ioToolbar))<>0 then
      UA.flags:=UA.flags or UAF_REGTTBB;
    if StrToInt(getAttrValue(actnode,ioTabSRMM))<>0 then
      UA.flags:=UA.flags or UAF_REGTABB;

    // ----- Hotkey -----
    // nothing

    // ----- Modern CList toolbar -----
    // source - ANSI text
    sub:=GetNthChild(actnode,ioToolbar,0);
    WideToAnsi(GetAttrValue(sub,ioTooltip       ),UA.szTTBTooltip       ,MirandaCP);
    WideToAnsi(GetAttrValue(sub,ioTooltipPressed),UA.szTTBTooltipPressed,MirandaCP);

    // ----- TabSRMM toolbar -----
    sub:=GetNthChild(actnode,ioTabSRMM,0);
    StrDupW(UA.szTabBTooltip       ,getAttrValue(sub,ioTooltip));
    StrDupW(UA.szTabBTooltipPressed,getAttrValue(sub,ioTooltipPressed));

    // ----- Menus -----
    num:=0;
    repeat
      sub:=getNextChild(actnode,ioMenuItem,@num);
      if sub=0 then break;

      i:=StrToInt(getAttrValue(sub,ioType));
      ImportMenuItems(sub,
          UA.UAMenuItem[tMenuType(i)]);
    until false;
  end;
end;

function Import(fname:PWideChar;aflags:dword):integer;
var
  i,j,act:integer;
  root,actnode:HXML;
  pcw,res:pWideChar;
  f:THANDLE;
  num,num1:integer;
  ptr,ptr1:pChain;
begin
  result:=0;

  if (fname=nil) or (fname^=#0) then
    exit;
  i:=GetFSize(fname);
  if i=0 then
    exit;

  num:=CallService(MS_ACT_GETLIST,0,LPARAM(@ptr));
  if num=0 then exit;
  ptr1:=ptr;

  mGetMem (res ,i+SizeOf(WideChar));
  FillChar(res^,i+SizeOf(WideChar),0);
  f:=Reset(fname);
  BlockRead(f,res^,i);
  CloseHandle(f);

  xmlparser.cbSize:={XML_API_SIZEOF_V1;//}SizeOf(TXML_API_W);
  CallService(MS_SYSTEM_GET_XI,0,lparam(@xmlparser));
  with xmlparser do
  begin
    root:=parseString(ChangeUnicode(res),@i,nil);
    j:=0;
    repeat
      actnode:=getNthChild(root,ioAction,j);
      if actnode=0 then break;
      // search id by name?
      pcw:=GetAttrValue(actnode,ioName);
      ptr:=ptr1;
      inc(pbyte(ptr),4);
      for i:=0 to num-1 do
      begin
        if (ptr.flags and ACF_SELECTED)<>0 then
        begin
          if StrCmpw(pcw,ptr.descr)=0 then
          begin
            // delete old UA for overwrited actions
            if (ptr.flags and ACF_OVERLOAD)<>0 then
            begin
              for act:=0 to HIGH(UActionList) do
              begin
                if ptr.id=UActionList[act].dwActID then
                begin
                  DeleteUAction(act,true);
                  break;
                end;
              end;
            end;
            num1:=AddUAction(-1,ptr);
            ImportUAction(getNthChild(actnode,ioUA,0),UActionList[num1]);
            break;
          end;
        end;
        inc(ptr);
      end;
      
      inc(j);
    until false;

    DestroyNode(root);
  end;
  CallService(MS_ACT_FREELIST,0,LPARAM(ptr1));
  mFreeMem(res);
  result:=1;
  if settings<>0 then
  begin
    FillActionList(settings);
    ShowAction(settings,-1);
  end;
end;

//--------------------------

function ExportMenuItems(node:HXML;const MenuItem:tUAMenuItem):HXML;
begin
  with xmlparser do
  begin
    result:=AddChild(node,ioMenuItem,nil);
    with MenuItem do
    begin
    // popup
      if (szMenuPopup<>nil) and (szMenuPopup^<>#0) then
        AddAttr(result,ioMenuPopup,szMenuPopup);
    // name
      if (szMenuNameVars<>nil) and (szMenuNameVars^<>#0) then
        AddAttr(result,ioMenuName,szMenuNameVars);
    // show
      if (szMenuShowWhenVars<>nil) and (szMenuShowWhenVars^<>#0) then
        AddAttr(result,ioMenuShow,szMenuShowWhenVars);
    // used
      AddAttrInt(result,ioMenuUsed,ord((menu_opt AND UAF_MENUUSE)<>0));
    // separated
      AddAttrInt(result,ioMenuSeparated,ord((menu_opt AND UAF_MENUSEP)<>0));
    // no translate
      AddAttrInt(result,ioNoTranslate,ord((menu_opt AND UAF_NOTRANS)<>0));
    end;
  end;
end;

procedure WriteUAction(root:HXML;id:dword;name:pWideChar);
var
  i:integer;
  lmenu:tMenuType;
  pc:pWideChar;
  actnode,sub:HXML;
  UA:pMyActionItem;
begin
  with xmlparser do
  begin
    for i:=0 to HIGH(UActionList) do
    begin
      if UActionList[i].dwActID=id then
      begin
        UA:=@UActionList[i];
        actnode:=getChildByAttrValue(root,ioAction,ioName,name);
        if actnode=0 then break;
        // we don't need that node as is, just it's child for UA
        actnode:=addChild(actnode,ioUA,nil);

        // ----- Common -----
        AddAttrInt(actnode,ioTwoState ,ORD((UA.flags and UAF_2STATE   )<>0));
        AddAttrInt(actnode,ioSaveState,ORD((UA.flags and UAF_SAVESTATE)<>0));

  //      sub:=AddChild(actnode,ioRegister,nil);
        AddAttrInt(actnode,ioHotkey ,ORD((UA.flags and UAF_REGHOTKEY)<>0));
        AddAttrInt(actnode,ioToolbar,ORD((UA.flags and UAF_REGTTBB )<>0));
        AddAttrInt(actnode,ioTabSRMM,ORD((UA.flags and UAF_REGTABB )<>0));

        // ----- Hotkey -----
        // nothing

        // ----- Modern CList toolbar -----
        // source - ANSI text
        if ((UA.szTTBTooltip       <>nil) and (UA.szTTBTooltip^       <>#0)) or
           ((UA.szTTBTooltipPressed<>nil) and (UA.szTTBTooltipPressed^<>#0)) then
        begin
          sub:=AddChild(actnode,ioToolbar,nil);
          if (UA.szTTBTooltip<>nil) and (UA.szTTBTooltip^<>#0) then
          begin
            AnsiToWide(UA.szTTBTooltip,pc,MirandaCP);
            AddAttr(sub,ioTooltip,pc);
            mFreeMem(pc);
          end;
          if (UA.szTTBTooltipPressed<>nil) and (UA.szTTBTooltipPressed^<>#0) then
          begin
            AnsiToWide(UA.szTTBTooltipPressed,pc,MirandaCP);
            AddAttr(sub,ioTooltipPressed,pc);
            mFreeMem(pc);
          end;
        end;

        // ----- TabSRMM toolbar -----
        if ((UA.szTabBTooltip       <>nil) and (UA.szTabBTooltip^       <>#0)) or
           ((UA.szTabBTooltipPressed<>nil) and (UA.szTabBTooltipPressed^<>#0)) then
        begin
          sub:=AddChild(actnode,ioTabSRMM,nil);
          if (UA.szTabBTooltip<>nil) and (UA.szTabBTooltip^<>#0) then
            AddAttr(sub,ioTooltip,UA.szTabBTooltip);
          if (UA.szTabBTooltipPressed<>nil) and (UA.szTabBTooltipPressed^<>#0) then
            AddAttr(sub,ioTooltipPressed,UA.szTabBTooltipPressed);
        end;

        // ----- Menus -----
        for lmenu:=main_menu to HIGH(tMenuType) do
        begin
          sub:=ExportMenuItems(actnode,UA.UAMenuItem[lmenu]);
          AddAttrInt(sub,ioType,ORD(lmenu));
        end;

        break;
      end;
    end;
  end;
end;

function Export(fname:pWideChar;aflags:dword):integer;
var
  i,num:integer;
  f:THANDLE;
  root:HXML;
  res:pWideChar;
  ptr,ptr1:pChain;
begin
  result:=0;
  xmlparser.cbSize:={XML_API_SIZEOF_V1;//}SizeOf(TXML_API_W);
  CallService(MS_SYSTEM_GET_XI,0,lparam(@xmlparser));
  with xmlparser do
  begin
    // we need append file, not rewrite
    i:=GetFSize(fname);
    if i=0 then exit;

    mGetMem (res ,i+SizeOf(WideChar));
    FillChar(res^,i+SizeOf(WideChar),0);
    f:=Reset(fname);
    BlockRead(f,res^,i);
    CloseHandle(f);
    root:=parseString(res,@i,nil);
    mFreeMem(res);

    num:=CallService(MS_ACT_GETLIST,0,LPARAM(@ptr));
    if num>0 then
    begin
      ptr1:=ptr;
      inc(pbyte(ptr),4);
      for i:=0 to num-1 do
      begin
        if ((aflags and ACIO_SELECTED)=0) or
           ((ptr.flags and ACCF_EXPORT)<>0) then
        begin
          WriteUAction(root,ptr.id,ptr.descr);
        end;
        inc(ptr);
      end;
      CallService(MS_ACT_FREELIST,0,LPARAM(ptr1));
    end;

    res:=toString(root,@i);

    f:=Rewrite(fname);
    BlockWrite(f,res^,i*SizeOf(WideChar));
    CloseHandle(f);
    xmlparser.FreeMem(res);
    DestroyNode(root);
  end;
  result:=1;
end;

function ActInOut(wParam:WPARAM;lParam:LPARAM):int_ptr; cdecl;
begin
  if (wParam and ACIO_EXPORT)=0 then
  begin
    result:=Import(pWideChar(lParam),wParam);
  end
  else
  begin
    result:=Export(pWideChar(lParam),wParam);
  end;
end;
