{Frame + background}

const
  WS_EX_LAYERED = $00080000;

function SetLayeredWindowAttributes(Hwnd: THANDLE; crKey: COLORREF; bAlpha: byte; dwFlags: dword): Boolean; stdcall;
   external user32 name 'SetLayeredWindowAttributes';

const
  defFrameText = '%artist% - %title%';

const
//  opt_HiddenByMe:PAnsiChar = 'frame/hiddenbyme';
  opt_ShowCtrls :PAnsiChar = 'frame/showcontrols';
  opt_FrmUsePic :PAnsiChar = 'frame/frmusepic';
  opt_FrmUseCvr :PAnsiChar = 'frame/frmusecover';
  opt_FrmBkColor:PAnsiChar = 'frame/frmbkcolor';
  opt_FrmBkPic  :PAnsiChar = 'frame/frmbkpic';
  opt_FrmBkMode :PAnsiChar = 'frame/frmbkmode';
  opt_FrmAlpha  :PAnsiChar = 'frame/frmalpha';
  opt_HideFrameM:PAnsiChar = 'frame/hideframem';
  opt_HideFrameP:PAnsiChar = 'frame/hideframep';
  opt_FrmTimer  :PAnsiChar = 'frame/frametimer';
  opt_PadLeft   :PAnsiChar = 'frame/paddingleft';
  opt_PadTop    :PAnsiChar = 'frame/paddingtop';
  opt_PadRight  :PAnsiChar = 'frame/paddingright';
  opt_PadBottom :PAnsiChar = 'frame/paddingbottom';
  opt_Manual    :PAnsiChar = 'frame/manualplacement';

procedure TWATFrame.ResetFrame;
var
  D:PWATFrameData;
begin
  D:=CustomData;
  if D.Trackbar <>nil then ResetTrackbar(D.Trackbar);
  if D.TextBlock<>nil then D.TextBlock.BlockText:=nil;
// frame back to default
  RefreshPicture(nil);
end;

procedure FrameTimerProc(wnd:HWND;uMsg:uint;idEvent:uint_ptr;dwTime:dword); stdcall;
var
  psi:pSongInfo;
  D:PWATFrameData;
begin
  D:=FrameCtrl.CustomData;

  if IsFrameHidden(D.FrameId) then
  begin
    if not D.wasHidden then
    begin
      D.wasHidden:=true;
      D.TextBlock.UpdateTime:=0;
    end;
    exit;
  end
  else if D.wasHidden and (D.TextBlock.UpdateTime=0) then
  begin
    D.wasHidden:=false;
    if (D.ShowControls and scText)<>0 then
      D.TextBlock.UpdateTime:=DBReadWord(0,PluginShort,opt_TxtTimer,10);
  end;

  if D.Trackbar<>nil then
  begin
    if (CallService(MS_WAT_GETMUSICINFO,WAT_INF_CHANGES,tlparam(@psi))<>WAT_RES_NOTFOUND) then
    begin
      SetTrackBarPosition(D.Trackbar,(psi^.time*1000) div D.UpdInterval)
    end;
  end;

  UpdateTextBlock(D,false); // false - check for %percent%/%time%

  FrameCtrl.Update;
end;

procedure TWATFrame.AdjustFrame;
var
  h:integer;
  D:PWATFrameData;
begin
  D:=CustomData;

  if D.ManualPlacement then
  begin
    if D.Trackbar <>nil then D.Trackbar .Anchor(false,false,false,false);
    if D.TextBlock<>nil then D.TextBlock.Anchor(false,false,false,false);
    DesignerLoadSettings;
    exit;
  end;

  h:=Height; // or need to get FRAME height

  if D.Trackbar<>nil then
  begin
    D.Trackbar.SetSize(Width-16,18);
    dec(h,D.Trackbar.Height);
    D.Trackbar.SetPosition(8,h);
    D.Trackbar.Anchor(true,false,true,true);
  end;

  if (D.ShowControls and scButtons)<>0 then
  begin
    AdjustButtons(h-16-BtnGap);
    dec(h,16+2*BtnGap);
  end;

  if D.TextBlock<>nil then
  begin
    D.TextBlock.Top   :=awkTextPad;
    D.TextBlock.Height:=h-D.TextBlock.Top;
    D.TextBlock.Anchor(true,true,true,true);
  end;
end;

procedure TWATFrame.SaveSettings;
var
  D:PWATFrameData;
begin
  D:=CustomData;
  DBWriteByte  (0,PluginShort,opt_Manual    ,ord(D.ManualPlacement));
  DBWriteByte  (0,PluginShort,opt_HideFrameM,ord(D.HideNoMusic));
  DBWriteByte  (0,PluginShort,opt_HideFrameP,ord(D.HideNoPlayer));
  DBWriteByte  (0,PluginShort,opt_FrmUsePic ,ord(D.UseBkPicture));
  DBWriteByte  (0,PluginShort,opt_FrmUseCvr ,ord(D.UseCover));
  DBWriteDWord (0,PluginShort,opt_FrmBkColor,D.BkColor);
  DBWriteWord  (0,PluginShort,opt_FrmBkMode ,D.BkMode);
  DBWriteDWord (0,PluginShort,opt_ShowCtrls ,D.ShowControls);
  DBWriteByte  (0,PluginShort,opt_FrmAlpha  ,D.FrmAlpha);
  DBWriteWord  (0,PluginShort,opt_FrmTimer  ,D.UpdInterval);
  DBWriteWord  (0,PluginShort,opt_PadLeft   ,D.padding.left);
  DBWriteWord  (0,PluginShort,opt_PadTop    ,D.padding.top);
  DBWriteWord  (0,PluginShort,opt_PadRight  ,D.padding.right);
  DBWriteWord  (0,PluginShort,opt_PadBottom ,D.padding.bottom);
  DBWriteString(0,PluginShort,opt_FrmBkPic  ,D.BkDefFile);

  CheckControls;
  AdjustFrame;
  RefreshPicture;
  InvalidateRect(FrameCtrl.GetWindowHandle,nil,true);
  FrameCtrl.Update;

  if D.UpdTimer<>0 then // FrameWnd MUST be present
  begin
    KillTimer(0,D.UpdTimer);
    D.UpdTimer:=0;
  end;
  if D.UpdInterval>0 then
  begin
    D.UpdTimer:=SetTimer(0,0,D.UpdInterval,@FrameTimerProc);
  end;
end;

procedure TWATFrame.LoadSettings;
var
  D:PWATFrameData;
begin
  D:=CustomData;
  D.ManualPlacement:=DBReadByte (0,PluginShort,opt_Manual    ,0)<>0;
  D.HideNoMusic    :=DBReadByte (0,PluginShort,opt_HideFrameM,0)<>0;
  D.HideNoPlayer   :=DBReadByte (0,PluginShort,opt_HideFrameP,0)<>0;
  D.UseBkPicture   :=DBReadByte (0,PluginShort,opt_FrmUsePic ,0)<>0;
  D.UseCover       :=DBReadByte (0,PluginShort,opt_FrmUseCvr ,0)<>0;
  D.BkColor        :=DBReadDWord(0,PluginShort,opt_FrmBkColor,$00E0E0E0);
  D.BkMode         :=DBReadWord (0,PluginShort,opt_FrmBkMode ,frbkCenter);
  D.ShowControls   :=DBReadDWord(0,PluginShort,opt_ShowCtrls ,scAll);
  D.FrmAlpha       :=DBReadByte (0,PluginShort,opt_FrmAlpha  ,255);

  D.UpdInterval:=DBReadWord(0,PluginShort,opt_FrmTimer,200);
  if D.UpdInterval<100 then
    D.UpdInterval:=D.UpdInterval*1000;

  D.padding.left  :=DBReadWord(0,PluginShort,opt_PadLeft  ,0);
  D.padding.top   :=DBReadWord(0,PluginShort,opt_PadTop   ,0);
  D.padding.right :=DBReadWord(0,PluginShort,opt_PadRight ,0);
  D.padding.bottom:=DBReadWord(0,PluginShort,opt_PadBottom,0);

  D.BkDefFile:=DBReadString(0,PluginShort,opt_FrmBkPic,nil);
  //!!!! saving NOT in TextBlock
  D.Template:=DBReadUnicode(0,PluginShort,opt_FrameText,DefFrameText);
end;

{$include i_bitmap.inc}

procedure TWATFrame.SetAlpha(value:integer);
const
  LWA_COLORKEY = $00000001;
  LWA_ALPHA    = $00000002;
var
  wnd:HWND;
  x:cardinal;
begin
  if IsFrameFloated(PWATFrameData(CustomData).FrameId) then
  begin
    wnd:=GetParent(FrameCtrl.GetWindowHandle);
    x:=GetWindowLongW(wnd,GWL_EXSTYLE);
    if value<>255 then
    begin
      if (x and WS_EX_LAYERED)=0 then
        SetWindowLongW(wnd,GWL_EXSTYLE,x or WS_EX_LAYERED);
      SetLayeredWindowAttributes(wnd,0,value,LWA_ALPHA);
    end
    else if (x and WS_EX_LAYERED)<>0 then
      SetWindowLongW(wnd,GWL_EXSTYLE,x and not WS_EX_LAYERED);
  end;
end;

procedure TWATFrame.FrameResize(Sender: PObj);
var
  tmpBmp:HBITMAP;
  D:PWATFrameData;
begin
  D:=CustomData;
  if D.BkDC<>0 then
  begin
    tmpBmp:=GetCurrentObject(D.BkDC,OBJ_BITMAP);
    DeleteDC(D.BkDC);
    D.BkDC:=0;
    DeleteObject(tmpBmp);
  end;
  AdjustFrame;
end;

procedure BkTimerProc(wnd:HWND;uMsg:uint;idEvent:uint_ptr;dwTime:dword); stdcall;
var
  D:PWATFrameData;
begin
  D:=FrameCtrl.CustomData;
  KillTimer(0,D.BkTimer);
  D.BkTimer:=0;
  DeleteObject(D.BkBitmap);
  D.BkBitmap:=0;
end;

procedure TWATFrame.RefreshPicture(cover:PAnsiChar=nil);
var
  D:PWATFrameData;
begin
  D:=CustomData;
  if D.BkBitmap<>0 then
    BkTimerProc(0,0,0,0); // remove old picture

  FrameResize(nil); // clear frame bitmap buffer

  if D.UseBkPicture then
    D.BkBitmap:=LoadBkPicture(cover,true,D.BkDefFile);

  if D.BkBitmap=HBITMAP(-1) then // same file
    D.BkBitmap:=0;
  Update;
end;

procedure TWATFrame.Paint(Sender: PControl; DC: HDC);
var
  rc: TRect;
  br:HBRUSH;
  D:PWATFrameData;
begin
  D:=CustomData;
  GetClientRect(Sender.Handle,rc);
  if D.UseBkPicture then
  begin
    if D.BkDC=0 then
    begin
      if D.BkBitmap=0 then
      begin
        if (D.BkFile<>nil) and (D.BkFile^<>#0) then
          D.BkBitmap:=CallService(MS_UTILS_LOADBITMAP,0,tlparam(D.BkFile));
      end;

      if D.BkBitmap<>0 then
      begin
        PreparePicture(dc,rc);
        D.BkTimer:=SetTimer(0,0,10000,@BkTimerProc);
      end;
    end;
    if D.BkDC<>0 then
    begin
      BitBlt(dc,rc.left,rc.top,rc.right-rc.left,rc.bottom-rc.top,
             D.BkDC,rc.left,rc.top,SRCCOPY);
      exit;
    end;
  end;

  InflateRect(rc,1,1);
  br:=CreateSolidBrush(D.BkColor);
  FillRect(dc,rc,br);
  DeleteObject(br);
end;

// JUST LOAD picture, no matter, which transforms
// Backname = from settings, Covername = from data (higher priority)
// -1 - same file, 0 - can't load, other - new bitmap
function TWATFrame.LoadBkPicture(CoverFName:PAnsiChar;check:boolean=false;
             BackFName:PAnsiChar=nil):integer;
var
  tmpstr:PAnsiChar;
  D:PWATFrameData;
begin
  result:=0;
  D:=CustomData;

  // check the same file, ie only 'next pic'
  if (CoverFName<>nil) and (CoverFName^<>#0) then
  begin
    if check and (StrCmp(CoverFName,D.BkFile)=0) then
    begin
      result:=-1;
      Exit;
    end;

    result:=CallService(MS_UTILS_LOADBITMAP,0,tlparam(CoverFName));
    if result<>0 then
    begin
      mFreeMem(D.BkFile);
      StrDup(D.BkFile,CoverFName);
      Exit;
    end;
  end;

  if (BackFName<>nil) and (BackFName^<>#0) then
  begin
    tmpstr:=ParseVarString(BackFName);
    if (tmpstr<>nil) and (tmpstr^<>#0) then
    begin
      if (not check) or (StrCmp(tmpstr,D.BkFile)<>0) then
      begin
        result:=CallService(MS_UTILS_LOADBITMAP,0,tlparam(tmpstr));
        if result<>0 then
        begin
          mFreeMem(D.BkFile);
          StrDup(D.BkFile,tmpstr);
        end;
      end
      else
        result:=-1;
    end;
    mFreeMem(tmpstr);
  end;
end;

procedure TWATFrame.ClearBitmapData;
var
  D:PWATFrameData;
  tmpBmp:HBITMAP;
begin
  D:=CustomData;

  if D.BkTimer<>0 then
  begin
    KillTimer(0,D.BkTimer);
    D.BkTimer:=0;
  end;
  if D.BkDC<>0 then
  begin
    tmpBmp:=GetCurrentObject(D.BkDC,OBJ_BITMAP);
    DeleteDC(D.BkDC);
    D.BkDC:=0;
    DeleteObject(tmpBmp);
  end;
  if D.BkBitmap<>0 then
  begin
    DeleteObject(D.BkBitmap);
    D.BkBitmap:=0;
  end;
  mFreeMem(D.BkFile);
end;

procedure TWATFrame.MyDestroy(sender:PObj);
var
  D:PWATFrameData;
begin
  D:=CustomData;
  if D.UpdTimer<>0 then
  begin
    KillTimer(0,D.UpdTimer);
    D.UpdTimer:=0;
  end;

  mFreeMem(D.Template);
  mFreeMem(D.BkDefFile);
  ClearBitmapData;

  if D.Designer<>nil then
  begin
    D.Designer.Free;
    D.Designer:=nil;
  end;

end;

procedure TWATFrame.RefreshAllFrameIcons;
var
  D:PWATFrameData;
begin
  D:=CustomData;
  if (D.ShowControls and scButtons)<>0 then RefreshButtonIcons;
  if D.Trackbar<>nil then RefreshTrackbarIcons(D.Trackbar);
end;

procedure TWATFrame.CheckControls;
var
  D:PWATFrameData;
  psi:pSongInfo;
begin
  D:=CustomData;

  if (D.ShowControls and scTrackBar)<>0 then
  begin
    if D.Trackbar=nil then
    begin
      RegisterButtonIcons;
      D.Trackbar:=MakeNewTrackbar(@self);
      // for case when TB creating after track start (fastest way)
      // can use (CallService(MS_WAT_GETMUSICINFO,WAT_INF_CHANGES,tlparam(@psi))<>WAT_RES_NOTFOUND)
      psi:=pointer(CallService(MS_WAT_RETURNGLOBAL,WAT_INF_UNICODE,1));
      TrackbarSetRange(D.Trackbar,D.UpdInterval,psi^.total);
    end;
  end
  else if D.Trackbar<>nil then
  begin
    D.Trackbar.Free;
    D.Trackbar:=nil;
  end;

  if (D.ShowControls and scButtons)<>0 then
  begin
    if D.btnarray[0]=nil then
    begin
      if RegisterButtonIcons then
        MakeNewButtonGroup;
    end
  end
  else if D.btnarray[0]<>nil then
    FreeButtons;
{
  if (D.ShowControls and scText)<>0 then
  begin
}
    if D.TextBlock=nil then
    begin
      D.TextBlock:=MakeTextBlock(@self,D.BkColor);
    end;

    if (D.ShowControls and scText)<>0 then                         
      D.TextBlock.UpdateTime:=DBReadWord(0,PluginShort,opt_TxtTimer,10);
{
  end
  else if D.TextBlock<>nil then
  begin
    D.TextBlock.Free;
    D.TextBlock:=nil;
  end;
}
  if D.UseBkPicture then
  begin
    D.BkBitmap:=LoadBkPicture(nil,true,D.BkDefFile);
    if D.BkBitmap=HBITMAP(-1) then
      D.BkBitmap:=0;
  end
  else
    ClearBitmapData;
end;

function CreateFrameWindow(parent:HWND):THANDLE;
var
  D:PWATFrameData;
begin
  result:=0;

  FrameCtrl:=PWATFrame(NewAlienPanel(parent,esNone));
  if FrameCtrl<>nil then
  begin
    GetMem  (D ,SizeOf(TWATFrameData));
    FillChar(D^,SizeOf(TWATFrameData),0); // clear all including buttons
    with FrameCtrl^ do
    begin
      CustomData:=D;
      LoadSettings;

      result:=GetWindowHandle;

      CheckControls;

      MinWidth :=80;
      MinHeight:=30;

      OnPaint      :=FrameCtrl.Paint;
      OnResize     :=FrameCtrl.FrameResize;
//      OnMouseDown  :=TOnMouse(MakeMethod(nil, @MouseDown));
      OnMouseDblClk:=FrameCtrl.CreateDesigner;
    end;
    FrameCtrl.OnDestroy:=FrameCtrl.MyDestroy;
// theoretically, must get Resize here.... or after
//    FrameCtrl.AdjustFrame;
  end;
end;
