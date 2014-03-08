{Frame}
const
  FSC_BACKGROUND = 1;
  FSC_BEHAVIOUR  = 2;
  FSC_SHOW       = 4;
//  FSC_ALPHA      = 8;

function MakePicFilter:PWideChar;
var
  buf:array [0..255] of WideChar;
  size:integer;
  pc:pWideChar;
begin
  FillChar(buf,SizeOf(buf),0);
  pc:=StrCopyEW(StrCopyEW(buf,TranslateW('All Bitmaps')),' (*.bmp;*.jpg;*.gif;*.png)');
  pc:=StrCopyEW(pc+1,'*.BMP;*.RLE;*.JPG;*.JPEG;*.GIF;*.PNG');
  size:=(pc+2-@buf)*SizeOf(WideChar);
  mGetMem(result,size);
  move(buf,result^,size);
end;

procedure SwitchBk(Dialog:HWND);
var
  en:boolean;
begin
  en:=IsDlgButtonChecked(Dialog,IDC_FRMUSEPIC)<>BST_UNCHECKED;
  EnableWindow(GetDlgItem(Dialog,IDC_FRMBKPIC ),en);
  EnableWindow(GetDlgItem(Dialog,IDC_FRMPICBTN),en);
  EnableWindow(GetDlgItem(Dialog,IDC_CENTERX  ),en);
  EnableWindow(GetDlgItem(Dialog,IDC_CENTERY  ),en);
  EnableWindow(GetDlgItem(Dialog,IDC_TILEX    ),en);
  EnableWindow(GetDlgItem(Dialog,IDC_TILEY    ),en);
  EnableWindow(GetDlgItem(Dialog,IDC_STRETCHX ),en);
  EnableWindow(GetDlgItem(Dialog,IDC_STRETCHY ),en);
  EnableWindow(GetDlgItem(Dialog,IDC_PROP     ),en);
  EnableWindow(GetDlgItem(Dialog,IDC_BOTTOM   ),en);
  EnableWindow(GetDlgItem(Dialog,IDC_RIGHT    ),en);
  EnableWindow(GetDlgItem(Dialog,IDC_USECOVER ),en);
end;

function FrameViewDlg(Dialog:HWND; hMessage:uint; wParam:WPARAM; lParam:LPARAM):LRESULT; stdcall;
const
  DlgInited:boolean=false;
var
  tmp:cardinal;
  buf1:PAnsiChar;
  buf:PAnsiChar;
  p:PAnsiChar;
  tmpb:longbool;
  pcw,tmpPicName:pWideChar;
  D:PWATFrameData;
begin
  result:=0;
  case hMessage of
    WM_INITDIALOG: begin
      DlgInited:=false;

      TranslateDialogDefault(Dialog);
      D:=FrameCtrl.CustomData;

      SendDlgItemMessage(Dialog,IDC_FRMALPHA,TBM_SETRANGE,0,MAKELONG(0,255));
      SendDlgItemMessage(Dialog,IDC_FRMALPHA,TBM_SETPOS,1,D.FrmAlpha);

      if (D.ShowControls and scButtons )<>0 then CheckDlgButton(Dialog,IDC_SHOWCTRLS,BST_CHECKED);
      if (D.ShowControls and scTrackBar)<>0 then CheckDlgButton(Dialog,IDC_SHOWBAR  ,BST_CHECKED);
      if (D.ShowControls and scText    )<>0 then CheckDlgButton(Dialog,IDC_SHOWTEXT ,BST_CHECKED);
      if (D.ShowControls and scVolume  )<>0 then CheckDlgButton(Dialog,IDC_SHOWVOLUM,BST_CHECKED);
      if (D.ShowControls and scGap     )<>0 then CheckDlgButton(Dialog,IDC_BTNGAP   ,BST_CHECKED);

      MakeHint(Dialog,IDC_MANUALPLACE,
        'Use manual frame elements placement. Doubleclick on free space to start designer.');
      CheckDlgButton(Dialog,IDC_MANUALPLACE,ORD(D.ManualPlacement));

      CheckDlgButton(Dialog,IDC_HIDEFRAMEPLAYER,ORD(D.HideNoPlayer));
      CheckDlgButton(Dialog,IDC_HIDEFRAMEMUSIC ,ORD(D.HideNoMusic));
      EnableWindow(GetDlgItem(Dialog,IDC_HIDEFRAMEMUSIC),D.HideNoPlayer);

      SendDlgItemMessage(Dialog,IDC_FRMCOLOR,CPM_SETCOLOUR,0,D.BkColor);
      SendDlgItemMessage(Dialog,IDC_FRMCOLOR,CPM_SETDEFAULTCOLOUR,0,GetSysColor(COLOR_BTNFACE));
      SetDlgItemInt(Dialog,IDC_FRMCOLOR,D.BkColor,false);

      if D.UseBkPicture then
        CheckDlgButton(Dialog,IDC_FRMUSEPIC,ORD(D.UseBkPicture));
      if D.BkDefFile=nil then
        p:=''
      else
        p:=D.BkDefFile;
      SetDlgItemTextA(Dialog,IDC_FRMBKPIC,p);
      CheckDlgButton(Dialog,IDC_USECOVER,ORD(D.UseCover));

      tmp:=D.UpdInterval;
      if (tmp>=1000) and (tmp mod 1000=0) then
        tmp:=tmp div 1000;
      SetDlgItemInt(Dialog,IDC_TIMER,tmp,false);

      SetDlgItemInt(Dialog,IDC_PADDING_LEFT  ,D.padding.left  ,false);
      SetDlgItemInt(Dialog,IDC_PADDING_TOP   ,D.padding.top   ,false);
      SetDlgItemInt(Dialog,IDC_PADDING_RIGHT ,D.padding.right ,false);
      SetDlgItemInt(Dialog,IDC_PADDING_BOTTOM,D.padding.bottom,false);

      if (D.BkMode and frbkCenterX     )<>0 then CheckDlgButton(Dialog,IDC_CENTERX ,BST_CHECKED);
      if (D.BkMode and frbkCenterY     )<>0 then CheckDlgButton(Dialog,IDC_CENTERY ,BST_CHECKED);
      if (D.BkMode and frbkTileX       )<>0 then CheckDlgButton(Dialog,IDC_TILEX   ,BST_CHECKED);
      if (D.BkMode and frbkTileY       )<>0 then CheckDlgButton(Dialog,IDC_TILEY   ,BST_CHECKED);
      if (D.BkMode and frbkStretchX    )<>0 then CheckDlgButton(Dialog,IDC_STRETCHX,BST_CHECKED);
      if (D.BkMode and frbkStretchY    )<>0 then CheckDlgButton(Dialog,IDC_STRETCHY,BST_CHECKED);
      if (D.BkMode and frbkProportional)<>0 then CheckDlgButton(Dialog,IDC_PROP    ,BST_CHECKED);
      if (D.BkMode and frbkBottom      )<>0 then CheckDlgButton(Dialog,IDC_BOTTOM  ,BST_CHECKED);
      if (D.BkMode and frbkRight       )<>0 then CheckDlgButton(Dialog,IDC_RIGHT   ,BST_CHECKED);

      SwitchBk(Dialog);

      DlgInited:=true;
    end;

    WM_HSCROLL: begin
      if DlgInited then
        SendMessage(GetParent(Dialog),PSM_CHANGED,0,0);
    end;

    WM_COMMAND: begin
      if (wParam shr 16)=BN_CLICKED then
      begin
        case LoWord(wParam) of
          IDC_HIDEFRAMEPLAYER: begin
            EnableWindow(GetDlgItem(Dialog,IDC_HIDEFRAMEMUSIC),
                IsDlgButtonChecked(Dialog,IDC_HIDEFRAMEPLAYER)<>BST_UNCHECKED);
          end;

          IDC_FRMUSEPIC: begin
            SwitchBk(Dialog);
          end;

          IDC_FRMPICBTN: begin
            pcw:=MakePicFilter;
            tmpPicName:=GetDlgText(Dialog,IDC_FRMBKPIC);
            mGetMem(buf,1024);
            if ShowDlgW(pWideChar(buf),tmpPicName,pcw) then
              SetDlgItemTextW(Dialog,IDC_FRMBKPIC,pWideChar(buf));
            mFreeMem(buf);
            mFreeMem(tmpPicName);
            mFreeMem(pcw);
          end;
        end;
      end;

      if DlgInited then
        if ((wParam shr 16)=BN_CLICKED) or
           ((wParam shr 16)=EN_CHANGE) or
           ((wParam shr 16)=CPN_COLOURCHANGED) then
        begin
          SendMessage(GetParent(Dialog),PSM_CHANGED,0,0);
        end;
    end;

    WM_NOTIFY: begin
      if integer(PNMHdr(lParam)^.code)=PSN_APPLY then
      begin
        D:=FrameCtrl.CustomData;
        // Alpha channel
        tmp:=SendDlgItemMessage(Dialog,IDC_FRMALPHA,TBM_GETPOS,0,0);
        if tmp<>Cardinal(D.FrmAlpha) then
        begin
          D.FrmAlpha:=tmp;
          if D.FrameId>=0 then
            FrameCtrl.SetAlpha(D.FrmAlpha);
        end;

        // show/hide controls
        tmp:=0;
        if IsDlgButtonChecked(Dialog,IDC_SHOWCTRLS)<>BST_UNCHECKED then tmp:=tmp or scButtons;
        if IsDlgButtonChecked(Dialog,IDC_SHOWBAR  )<>BST_UNCHECKED then tmp:=tmp or scTrackBar;
        if IsDlgButtonChecked(Dialog,IDC_SHOWTEXT )<>BST_UNCHECKED then tmp:=tmp or scText;
        if IsDlgButtonChecked(Dialog,IDC_SHOWVOLUM)<>BST_UNCHECKED then tmp:=tmp or scVolume;
        if IsDlgButtonChecked(Dialog,IDC_BTNGAP   )<>BST_UNCHECKED then tmp:=tmp or scGap;
        D.ShowControls:=tmp;

        tmp:=IsDlgButtonChecked(Dialog,IDC_MANUALPLACE);
        // From Auto to Manual = keep position
        if (tmp<>BST_UNCHECKED) and not D.ManualPlacement then D.Loaded:=$FFFF;
        D.ManualPlacement:=tmp<>BST_UNCHECKED;

        // Frame background
        tmpb:=IsDlgButtonChecked(Dialog,IDC_USECOVER)<>BST_UNCHECKED;
        if tmpb<>D.UseCover then
        begin
          D.UseCover:=tmpb;
        end;
        tmpb:=IsDlgButtonChecked(Dialog,IDC_FRMUSEPIC)<>BST_UNCHECKED;
        if tmpb<>D.UseBkPicture then
        begin
          D.UseBkPicture:=tmpb;
        end;
        tmp:=SendDlgItemMessage(Dialog,IDC_FRMCOLOR,CPM_GETCOLOUR,0,0);
        if tmp<>D.BkColor then
        begin
          D.BkColor:=tmp;
        end;

        mGetMem(buf1,1024{*SizeOf(WideChar)});
        buf1^:=#0;
        buf:=GetDlgText(Dialog,IDC_FRMBKPIC,true);
        PathToRelative(buf,buf1);
//        CallService(MS_UTILS_PATHTORELATIVE,twparam(buf),tlparam(buf1));
        if StrCmp(buf1,D.BkDefFile)<>0 then
        begin
          mFreeMem(D.BkDefFile);
          StrDup(D.BkDefFile,buf1);
        end;
        mFreeMem(buf);
        mFreeMem(buf1);

        // Picture effects
        tmp:=0;
        if IsDlgButtonchecked(Dialog,IDC_CENTERX )<>BST_UNCHECKED then tmp:=tmp or frbkCenterX;
        if IsDlgButtonchecked(Dialog,IDC_CENTERY )<>BST_UNCHECKED then tmp:=tmp or frbkCenterY;
        if IsDlgButtonchecked(Dialog,IDC_BOTTOM  )<>BST_UNCHECKED then tmp:=tmp or frbkBottom;
        if IsDlgButtonchecked(Dialog,IDC_RIGHT   )<>BST_UNCHECKED then tmp:=tmp or frbkRight;
        if IsDlgButtonchecked(Dialog,IDC_TILEX   )<>BST_UNCHECKED then tmp:=tmp or frbkTileX;
        if IsDlgButtonchecked(Dialog,IDC_TILEY   )<>BST_UNCHECKED then tmp:=tmp or frbkTileY;
        if IsDlgButtonchecked(Dialog,IDC_STRETCHX)<>BST_UNCHECKED then tmp:=tmp or frbkStretchX;
        if IsDlgButtonchecked(Dialog,IDC_STRETCHY)<>BST_UNCHECKED then tmp:=tmp or frbkStretchY;
        if IsDlgButtonchecked(Dialog,IDC_PROP    )<>BST_UNCHECKED then tmp:=tmp or frbkProportional;

        if tmp<>D.BkMode then
        begin
          D.BkMode:=tmp;
        end;

        // Hide frame option
        tmpb :=IsDlgButtonChecked(Dialog,IDC_HIDEFRAMEPLAYER)<>BST_UNCHECKED;
        if tmpb<>D.HideNoPlayer then
        begin
          D.HideNoPlayer:=tmpb;
        end;
        tmpb:=IsDlgButtonChecked(Dialog,IDC_HIDEFRAMEMUSIC)<>BST_UNCHECKED;
        if tmpb<>D.HideNoMusic then
        begin
          D.HideNoMusic:=tmpb;
        end;

        // Padding
        tmp:=GetDlgItemInt(Dialog,IDC_PADDING_LEFT,tmpb,false);
        if integer(tmp)<>D.padding.left then
        begin
          D.padding.left:=tmp;
        end;
        tmp:=GetDlgItemInt(Dialog,IDC_PADDING_TOP,tmpb,false);
        if integer(tmp)<>D.padding.top then
        begin
          D.padding.top:=tmp;
        end;
        tmp:=GetDlgItemInt(Dialog,IDC_PADDING_RIGHT,tmpb,false);
        if integer(tmp)<>D.padding.right then
        begin
          D.padding.right:=tmp;
        end;
        tmp:=GetDlgItemInt(Dialog,IDC_PADDING_BOTTOM,tmpb,false);
        if integer(tmp)<>D.padding.bottom then
        begin
          D.padding.bottom:=tmp;
        end;

        tmp:=GetDlgItemInt(Dialog,IDC_TIMER,tmpb,false);
        if tmp>0 then
        begin
          if tmp<100 then
            tmp:=tmp*1000;
          if tmp<200 then
            tmp:=200;
        end;

        if tmp<>D.UpdInterval then
        begin
          D.UpdInterval:=tmp;
        end;

        FrameCtrl.SaveSettings;
      end;
    end;
  else
    {result:=}DefWindowProc(Dialog,hMessage,wParam,lParam);
  end;
end;
