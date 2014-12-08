{Dialog list filling}
  procedure InsertString(wnd:HWND;num:dword;str:PAnsiChar);
  var
    buf:array [0..127] of WideChar;
  begin
    SendMessageW(wnd,CB_SETITEMDATA,
        SendMessageW(wnd,CB_ADDSTRING,0,
            lparam(TranslateW(FastAnsiToWideBuf(str,buf)))),
        num);
  {
    SendMessageW(wnd,CB_INSERTSTRING,num,
        LPARAM(TranslateW(FastAnsiToWideBuf(str,buf))));
  }
  end;

  procedure MakeMathOperList(wnd:HWND);
  begin
    SendMessage(wnd,CB_RESETCONTENT,0,0);
    InsertString(wnd,cardinal(aeNot),'! not');
    InsertString(wnd,cardinal(aeAdd),'+ add');
    InsertString(wnd,cardinal(aeSub),'- sub');
    InsertString(wnd,cardinal(aeMul),'* mul');
    InsertString(wnd,cardinal(aeDiv),'/ div');
    InsertString(wnd,cardinal(aeMod),'% mod');
    InsertString(wnd,cardinal(aeAnd),'& and');
    InsertString(wnd,cardinal(aeOr ),'| or');
    InsertString(wnd,cardinal(aeXor),'^ xor');
    InsertString(wnd,cardinal(aeSet),'= set');
    SendMessage(wnd,CB_SETCURSEL,0,0);
  end;

  procedure MakeParamTypeList(wnd:HWND);
  begin
    SendMessage(wnd,CB_RESETCONTENT,0,0);
    InsertString(wnd,ptNumber ,'number value');
    InsertString(wnd,ptString ,'ANSI string');
    InsertString(wnd,ptUnicode,'Unicode string');
    InsertString(wnd,ptCurrent,'current contact');
    InsertString(wnd,ptResult ,'last result');
    InsertString(wnd,ptParam  ,'parameter');
    InsertString(wnd,ptStruct ,'structure');
    SendMessage(wnd,CB_SETCURSEL,0,0);
  end;

  procedure MakeResultTypeList(wnd:HWND);
  begin
    SendMessage(wnd,CB_RESETCONTENT,0,0);
    InsertString(wnd,sresInt   ,'Integer');
    InsertString(wnd,sresHex   ,'Hexadecimal');
    InsertString(wnd,sresString,'String');
    InsertString(wnd,sresStruct,'Structure');
    SendMessage(wnd,CB_SETCURSEL,0,0);
  end;

  procedure MakeFileEncList(wnd:HWND);
  begin
    SendMessage(wnd,CB_RESETCONTENT,0,0);
    InsertString(wnd,0,'Ansi');
    InsertString(wnd,1,'UTF8');
    InsertString(wnd,2,'UTF8+sign');
    InsertString(wnd,3,'UTF16');
    InsertString(wnd,4,'UTF16+sign');
    SendMessage(wnd,CB_SETCURSEL,0,0);
  end;

  procedure MakeDataTypeList(wnd:HWND);
  begin
    SendMessage(wnd,CB_RESETCONTENT,0,0);
    InsertString(wnd,0,'Byte');
    InsertString(wnd,1,'Word');
    InsertString(wnd,2,'DWord');
    InsertString(wnd,3,'Ansi');
    InsertString(wnd,4,'Unicode');
    SendMessage(wnd,CB_SETCURSEL,0,0);
  end;
