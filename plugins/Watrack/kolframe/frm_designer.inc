{Frame designer}
const
  // trackbar
  opt_tbleft  :pAnsiChar='frame/designer/trackbar/left';
  opt_tbtop   :pAnsiChar='frame/designer/trackbar/top';
  opt_tbwidth :pAnsiChar='frame/designer/trackbar/width';
  opt_tbheight:pAnsiChar='frame/designer/trackbar/height';
  // text block
  opt_tleft   :pAnsiChar='frame/designer/text/left';
  opt_ttop    :pAnsiChar='frame/designer/text/top';
  opt_twidth  :pAnsiChar='frame/designer/text/width';
  opt_theight :pAnsiChar='frame/designer/text/height';
  // buttons (with number)
  opt_bleft   :pAnsiChar='frame/designer/buttons/left';
  opt_btop    :pAnsiChar='frame/designer/buttons/top';

procedure TWATFrame.DesignerSaveSettings;
var
  i:integer;
  D:PWATFrameData;
  lleft,ltop:array [0..63] of AnsiChar;
  pleft,ptop:PAnsiChar;
begin
  D:=CustomData;
  if D.TrackBar<>nil then
  begin
    DBWriteWord(0,PluginShort,opt_tbleft  ,D.Trackbar.Left);
    DBWriteWord(0,PluginShort,opt_tbtop   ,D.Trackbar.Top);
    DBWriteWord(0,PluginShort,opt_tbwidth ,D.Trackbar.Width);
    DBWriteWord(0,PluginShort,opt_tbheight,D.Trackbar.Height);
  end;

  if D.TextBlock<>nil then
  begin
    DBWriteWord(0,PluginShort,opt_tleft  ,D.TextBlock.Left);
    DBWriteWord(0,PluginShort,opt_ttop   ,D.TextBlock.Top);
    DBWriteWord(0,PluginShort,opt_twidth ,D.TextBlock.Width);
    DBWriteWord(0,PluginShort,opt_theight,D.TextBlock.Height);
  end;

  if (D.ShowControls and scButtons)<>0 then
  begin
    if D.btnarray[0]<>nil then
    begin
      pleft:=StrCopyE(lleft,opt_bleft);
      ptop :=StrCopyE(ltop ,opt_btop);
      for i:=0 to HIGH(D.btnarray) do
      begin
        IntToStr(pleft,i); DBWriteWord(0,PluginShort,lleft,D.btnarray[i].Left);
        IntToStr(ptop ,i); DBWriteWord(0,PluginShort,ltop ,D.btnarray[i].Top);
      end;
    end;
  end;
end;

procedure TWATFrame.DesignerLoadSettings;
var
  i:integer;
  D:PWATFrameData;
  lleft,ltop:array [0..63] of AnsiChar;
  pleft,ptop:PAnsiChar;
  for_check:integer;
begin
  D:=CustomData;

  if (D.TrackBar<>nil) and
    ((D.ShowControls and scTrackbar)<>0) and
    ((D.Loaded and scTrackbar)=0) then
  begin
    D.Loaded:=D.Loaded or scTrackbar;
    for_check:=DBReadWord(0,PluginShort,opt_tbwidth);
    if for_check<>0 then
    begin
      D.Trackbar.SetPosition(
        DBReadWord(0,PluginShort,opt_tbleft),
        DBReadWord(0,PluginShort,opt_tbtop));
      D.Trackbar.SetSize(
        for_check,
        {18}DBReadWord(0,PluginShort,opt_tbheight));
    end;
  end;

  if (D.TextBlock<>nil) and
    ((D.ShowControls and scText)<>0) and
    ((D.Loaded and scText)=0) then
  begin
    D.Loaded:=D.Loaded or scText;
    for_check:=DBReadWord(0,PluginShort,opt_twidth);
    if for_check<>0 then
    begin
      D.TextBlock.SetPosition(
        DBReadWord(0,PluginShort,opt_tleft),
        DBReadWord(0,PluginShort,opt_ttop));
      D.TextBlock.SetSize(
        for_check,
        DBReadWord(0,PluginShort,opt_theight));
    end;
  end;

  if ((D.ShowControls and scButtons)<>0) and
     ((D.Loaded and scButtons)=0) then
  begin
    if D.btnarray[0]<>nil then
    begin
      D.Loaded:=D.Loaded or scButtons;
      pleft:=StrCopyE(lleft,opt_bleft);
      pleft^:='0'; (pleft+1)^:=#0;
      for_check:=SmallInt(DBReadWord(0,PluginShort,lleft,word(-1)));
      if for_check>=0 then
      begin
        ptop :=StrCopyE(ltop,opt_btop);
        for i:=0 to HIGH(D.btnarray) do
        begin
          IntToStr(pleft,i);
          IntToStr(ptop ,i);
          D.btnarray[i].SetPosition(
            DBReadWord(0,PluginShort,lleft,word(-1)),
            DBReadWord(0,PluginShort,ltop ,word(-1)));
        end;
      end;
    end;
  end;
end;

procedure TWATFrame.CreateDesigner(Sender:PControl;var Mouse:TMouseEventData);
var
  D:PWATFrameData;
begin
  D:=CustomData;
  if not D.ManualPlacement then exit;

  if D.Designer=nil then
    D.Designer:=NewDesigner(@self);

  if not D.Designer.Active then
  begin
    // Trackbar
    if D.Trackbar<>nil then
    begin
      D.Trackbar.Anchor(false,false,false,false);
      D.Designer.Connect('Trackbar',D.Trackbar);
    end;
    // TextBlock
    if D.TextBlock<>nil then
    begin
      D.TextBlock.Anchor(false,false,false,false);
      D.Designer.Connect('Panel',D.TextBlock);
    end;
    // Icons
    if (D.ShowControls and scButtons)<>0 then MakeButtonsDesigner;

    D.Designer.Active:=true;
  end
  else
  begin
    D.Designer.Active:=False;

    DesignerSaveSettings;
    if D.Trackbar <>nil then D.Designer.Disconnect(D.Trackbar);
    if D.TextBlock<>nil then D.Designer.Disconnect(D.TextBlock);
    if (D.ShowControls and scButtons)<>0 then FreeButtonsDesigner;
    Sender.Update;
  end;
end;
