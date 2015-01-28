{}

const
  kinopoisk_info = 'http://www.kinopoisk.ru/level/1/film/';

procedure ClearInfo(Dlg:HWND);
begin
  SetDlgItemTextW(Dlg,IDC_DATA_SERIES ,'');
  SetDlgItemTextW(Dlg,IDC_DATA_EPISODE,'');
  SetDlgItemTextW(Dlg,IDC_DATA_TAGS   ,'');
  SetDlgItemTextW(Dlg,IDC_DATA_TAGS   ,'');
  SetDlgItemTextW(Dlg,IDC_DATA_INFO   ,'');
end;

function DlgProcOptions(Dialog:HWND;hMessage:uint;wParam:WPARAM;lParam:LPARAM):LRESULT; stdcall;
const
  inited:bool=false;
var
  tmp:longbool;
//  bmp,wnd:HWND;
//  buf:array [0..255] of AnsiChar;
begin
  result:=0;
  case hMessage of
    WM_INITDIALOG: begin
      inited:=false;
      TranslateDialogDefault(Dialog);

      SetDlgItemTextA(Dialog,IDC_LOGIN,msh_login);
      SetDlgItemTextA(Dialog,IDC_PASS ,msh_password);
      SetDlgItemInt  (Dialog,IDC_TRIES,msh_tries,false);
//      SetDlgItemInt  (Dialog,IDC_TIME ,msh_timeout,false);
//      ClearInfo(Dialog);
//      EnableWindow(GetDlgItem(Dialog,IDC_KINOPOISK),false);

      SendDlgItemMessage(Dialog,IDC_SCROBPOS,TBM_SETRANGE,0,MAKELONG(0,100));
      SendDlgItemMessage(Dialog,IDC_SCROBPOS,TBM_SETPOS,1,msh_scrobpos);
      inited:=true;
    end;

    WM_HSCROLL: begin
      SendMessage(GetParent(Dialog),PSM_CHANGED,0,0);
    end;

    WM_COMMAND: begin
      if inited then
      begin
  (*
        case Loword(wParam) of
          IDC_KINOPOISK: begin
            StrCopy(StrCopyE(buf,kinopoisk_info),MSData.kinopoisk_id);
            CallService(MS_UTILS_OPENURL,TWPARAM(True),TLPARAM(@buf));
            result:=1;
            exit;
          end;
        end;
  *)
        case wParam shr 16 of
          BN_CLICKED: begin
 (*
            case LoWord(wParam) of
              IDC_INFO_SERIES: begin
                ClearInfo(Dialog);
                ClearData;

              SetDlgItemTextW(Dialog,IDC_DATA_SERIES ,MSData.series);
              SetDlgItemTextW(Dialog,IDC_DATA_EPISODE,MSData.episode);
 //             SetDlgItemTextW(Dialog,IDC_DATA_TAGS   ,data.genre);
              SetDlgItemTextW(Dialog,IDC_DATA_INFO   ,MSData.info);

              bmp:=LoadImageURL(MSData.image,80);
              if bmp<>0 then
                DeleteObject(SendDlgItemMessage(Dialog,IDC_DATA_PIC,STM_SETIMAGE,IMAGE_BITMAP,bmp));

              EnableWindow(GetDligItem(Dialog,IDC_KINOPOISK),true);
            end;
 *)
          end;

          EN_CHANGE: begin
            case loword(wParam) of
              IDC_LOGIN,IDC_PASS,IDC_TRIES{,IDC_TIME}:
                SendMessage(GetParent(Dialog),PSM_CHANGED,0,0);
            end;
          end;
        end;
      end;
    end;

    WM_NOTIFY: begin
      case integer(PNMHdr(lParam)^.code) of
        PSN_APPLY: begin
          msh_scrobpos:=SendDlgItemMessage(Dialog,IDC_SCROBPOS,TBM_GETPOS,0,0);
          msh_tries   :=GetDlgItemInt(Dialog,IDC_TRIES,tmp,false);
          mFreeMem(msh_login   ); msh_login   :=GetDlgText(Dialog,IDC_LOGIN,true);
          mFreeMem(msh_password); msh_password:=GetDlgText(Dialog,IDC_PASS ,true);
  {
          mFreeMem(session_id);
          mFreeMem(np_url);
          mFreeMem(sub_url);
  }
  //        msh_timeout:=GetDlgItemInt(Dialog,IDC_TIME ,tmp,false);

          SaveOpt;
        end;
      end;
    end;
  else
    {result:=}DefWindowProc(Dialog,hMessage,wParam,lParam);
  end;
end;
