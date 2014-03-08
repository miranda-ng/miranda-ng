{special tab: parts settings}

function DlgProcOptions0(Dialog:HWND;hMessage:uint;wParam:WPARAM;lParam:LPARAM):LRESULT; stdcall;
const
  hasApply:boolean=false;
var
  i:integer;
  ptr:pwModule;
  wnd:HWND;
  rc:TRECT;
begin
  result:=0;
  case hMessage of
    WM_DESTROY: begin
      if hasApply then
      begin
        ptr:=ModuleLink;
        while ptr<>nil do
        begin
          if ptr^.ModuleName<>nil then
          begin
            i:=SendMessageW(ptr^.Button,BM_GETCHECK,0,0);
            if (i=BST_CHECKED) xor (ptr^.ModuleStat<>0) then
            begin
              if i=BST_CHECKED then
              begin
                ptr^.ModuleStat:=1;
                if @ptr^.Init<>nil then
                  if ptr^.Init(false)=0 then
                    ptr^.ModuleStat:=0;
              end
              else
              begin
                ptr^.ModuleStat:=0;
                if @ptr^.DeInit<>nil then
                  ptr^.DeInit(true);
              end;
            end;
//            if ptr^.ModuleStat then
          end;
          ptr:=ptr^.Next;
        end;
      end;
    end;

    WM_INITDIALOG: begin

      hasApply:=false;

      ptr:=ModuleLink;
      i:=0;
      while ptr<>nil do
      begin
        if ptr^.ModuleName<>nil then
        begin
          ptr^.Button:=CreateWindowW('BUTTON',TranslateW(ptr^.ModuleName),
              WS_CHILD+WS_VISIBLE+BS_AUTOCHECKBOX,
              14,20+i*20,150,14,Dialog,0,hInstance,nil);
          SendMessageW(ptr^.Button,WM_SETFONT,GetStockObject(DEFAULT_GUI_FONT),0);
          if ptr^.ModuleStat<>0 then
            SendMessageW(ptr^.Button,BM_SETCHECK,BST_CHECKED,0);
          inc(i);
        end;
        ptr:=ptr^.Next;
      end;
      if i>0 then
      begin
        wnd:=GetDlgItem(Dialog,IDC_MODULEGROUP);
        GetWindowRect(wnd,rc);
        SetWindowPos (wnd,0,0,0,rc.Right-rc.Left,(i+1)*20,
            SWP_NOMOVE+SWP_NOZORDER+SWP_NOACTIVATE);
      end;

      TranslateDialogDefault(Dialog);
    end;

    WM_COMMAND: begin
      if (wParam shr 16)=BN_CLICKED then
        SendMessage(GetParent(Dialog),PSM_CHANGED,0,0);
    end;

    WM_NOTIFY: begin
      if integer(PNMHdr(lParam)^.code)=PSN_APPLY then
      begin
        hasApply:=true;
      end;
    end;
  else
    {result:=}DefWindowProc(Dialog,hMessage,wParam,lParam);
  end;
end;
