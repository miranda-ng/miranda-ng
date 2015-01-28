{}
const
  MaxLangs = 11;
  LangArray:array [0..MaxLangs-1] of record
    code:array [0..1] of AnsiChar;
    name:pWideChar;
  end= (
    (code:#0#0 ; name: 'no language';),
    (code:'zh' ; name: 'Chinese'    ;),
    (code:'en' ; name: 'English'    ;),
    (code:'fr' ; name: 'French'     ;),
    (code:'de' ; name: 'German'     ;),
    (code:'hi' ; name: 'Hindi'      ;),
    (code:'it' ; name: 'Italian'    ;),
    (code:'ja' ; name: 'Japanese'   ;),
    (code:'pt' ; name: 'Portuguese' ;),
    (code:'ru' ; name: 'Russian'    ;),
    (code:'es' ; name: 'Spanish'    ;)
  );

procedure ClearInfo(Dlg:HWND);
begin
  SetDlgItemTextW(Dlg,IDC_DATA_ARTIST,'');
  SetDlgItemTextW(Dlg,IDC_DATA_ALBUM ,'');
  SetDlgItemTextW(Dlg,IDC_DATA_TRACK ,'');
  SetDlgItemTextW(Dlg,IDC_DATA_TAGS  ,'');
  SetDlgItemTextW(Dlg,IDC_DATA_INFO  ,'');
end;

procedure ClearData(var data:tLastFMInfo);
begin
  mFreeMem(data.artist);
  mFreeMem(data.album);
  mFreeMem(data.title);
  mFreeMem(data.tags);
  mFreeMem(data.info);
  mFreeMem(data.image);
end;

function DlgProcOptions(Dialog:HWND;hMessage:uint;wParam:WPARAM;lParam:LPARAM):LRESULT; stdcall;
var
  tmp:longbool;
  bmp,wnd:HWND;
  lang:integer;
  data:tLastFMInfo;
begin
  result:=0;
  case hMessage of
    WM_INITDIALOG: begin
      TranslateDialogDefault(Dialog);
      SetDlgItemTextA(Dialog,IDC_LOGIN,lfm_login);
      SetDlgItemTextA(Dialog,IDC_PASS ,lfm_password);
      SetDlgItemInt  (Dialog,IDC_TRIES,lfm_tries,false);
      wnd:=GetDlgItem(Dialog,IDC_LANGUAGE);
      for lang:=0 to MaxLangs-1 do
        with LangArray[lang] do
          CB_AddStrDataW(wnd,TranslateW(name),
          ord(code[0])+(ord(code[1]) shl 8),lang);
      CB_SelectData(wnd,lfm_lang);
    end;

    WM_COMMAND: begin
      case wParam shr 16 of
         BN_CLICKED: begin
           ClearInfo(Dialog);
           FillChar(data,SizeOf(data),0);
           lfm_lang:=CB_GetData(GetDlgItem(Dialog,IDC_LANGUAGE));

           if loword(wParam)=IDC_INFO_ARTIST then
           begin
             SetDlgItemTextW(Dialog,IDC_ALT,TranslateW('Similar artists'));
             GetArtistInfo(data,lfm_lang);
             SetDlgItemTextW(Dialog,IDC_DATA_ALBUM,data.similar);
           end
           else
           begin
             SetDlgItemTextW(Dialog,IDC_ALT,TranslateW('Album'));
             if loword(wParam)=IDC_INFO_TRACK then
               GetTrackInfo (data,lfm_lang)
             else
               GetAlbumInfo (data,lfm_lang);
             SetDlgItemTextW(Dialog,IDC_DATA_ALBUM,data.album);
           end;

           SetDlgItemTextW(Dialog,IDC_DATA_ARTIST,data.artist);
           SetDlgItemTextW(Dialog,IDC_DATA_TRACK ,data.title);
           SetDlgItemTextW(Dialog,IDC_DATA_TAGS  ,data.tags);
           SetDlgItemTextW(Dialog,IDC_DATA_INFO  ,data.info);
           bmp:=LoadImageURL(data.image,64);
           if bmp<>0 then
             DeleteObject(SendDlgItemMessage(Dialog,IDC_DATA_PIC,STM_SETIMAGE,IMAGE_BITMAP,bmp));

           ClearData(data);
         end;
         EN_CHANGE: 
           case loword(wParam) of
             IDC_LOGIN,IDC_PASS,IDC_TRIES:
               SendMessage(GetParent(Dialog),PSM_CHANGED,0,0);
           end;
      end;
    end;

    WM_NOTIFY: begin
      if integer(PNMHdr(lParam)^.code)=PSN_APPLY then
      begin
        mFreeMem(lfm_login   ); lfm_login   :=GetDlgText(Dialog,IDC_LOGIN,true);
        mFreeMem(lfm_password); lfm_password:=GetDlgText(Dialog,IDC_PASS ,true);
        mFreeMem(session_id);
        mFreeMem(np_url);
        mFreeMem(sub_url);
        lfm_tries:=GetDlgItemInt(Dialog,IDC_TRIES,tmp,false);
        lfm_lang:=CB_GetData(GetDlgItem(Dialog,IDC_LANGUAGE));

        SaveOpt;
      end;
    end;
  else
    {result:=}DefWindowProc(Dialog,hMessage,wParam,lParam);
  end;
end;
