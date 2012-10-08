{Panel = group of icons}

procedure TWATFrame.AdjustButtons(atop:integer);
var
  i,lWidth,lOffs,gap:integer;
  lleft:integer;
  D:PWATFrameData;
begin
  D:=CustomData;

  if D.btnarray[0]=nil then exit;

  if (D.ShowControls and scGap)<>0 then
    gap:=BtnGap
  else
    gap:=0;

  D.btnarray[0].Visible:=(D.ShowControls and scVolume)<>0;
  D.btnarray[1].Visible:=(D.ShowControls and scVolume)<>0;

  lWidth:=(Length(D.btnarray)-2)*(16+gap)-gap;
  if (D.ShowControls and scVolume)<>0 then
    Inc(lWidth, VolBtnDist+2*(16+gap));
  lLeft:=(Width-lWidth) div 2;

  lOffs:=0;
  if (D.ShowControls and scVolume)<>0 then
  begin
    D.btnarray[0].Left:=lleft+lOffs;
    D.btnarray[0].Top :=atop;
    Inc(lOffs,16+gap);
    D.btnarray[1].Left:=lleft+lOffs;
    D.btnarray[1].Top :=atop;
    Inc(lOffs,16+gap+VolBtnDist);
  end;

  for i:=2 to HIGH(D.btnarray) do
  begin
    D.btnarray[i].Left:=lleft+lOffs;
    D.btnarray[i].Top :=atop;
    Inc(lOffs,16+gap);
  end;

end;

procedure TWATFrame.RefreshButtonIcons;
var
  i:integer;
  D:PWATFrameData;
begin
  D:=CustomData;

  if D.btnarray[0]<>nil then exit;

  for i:=0 to HIGH(D.btnarray) do
    D.btnarray[i].RefreshIcon;
end;

procedure TWATFrame.FreeButtonsDesigner;
var
  i:integer;
  D:PWATFrameData;
begin
  D:=CustomData;

  if D.btnarray[0]<>nil then exit;

  for i:=0 to HIGH(D.btnarray) do
    D.Designer.Disconnect(D.btnarray[i]);
end;

procedure TWATFrame.MakeButtonsDesigner;
var
  i:integer;
  D:PWATFrameData;
begin
  D:=CustomData;

  if D.btnarray[0]<>nil then exit;

  for i:=0 to HIGH(D.btnarray) do
    D.Designer.Connect('Button',D.btnarray[i],DESIGNER_NORESIZE);
end;

procedure TWATFrame.FreeButtons;
var
  i:integer;
  D:PWATFrameData;
begin
  D:=CustomData;

  if D.btnarray[0]<>nil then exit;

  for i:=0 to HIGH(D.btnarray) do
  begin
    D.btnarray[i].Free;
    D.btnarray[i]:=nil;
  end;
end;

procedure TWATFrame.MakeNewButtonGroup;
var
  D:PWATFrameData;
begin
  D:=CustomData;

  D.btnarray[0]:=CreateIcoButton(@self,waticons.GetIcon,DoAction,WAT_CTRL_VOLDN,300);
  D.btnarray[1]:=CreateIcoButton(@self,waticons.GetIcon,DoAction,WAT_CTRL_VOLUP,300);
  D.btnarray[2]:=CreateIcoButton(@self,waticons.GetIcon,DoAction,WAT_CTRL_PREV,2000);
  D.btnarray[3]:=CreateIcoButton(@self,waticons.GetIcon,DoAction,WAT_CTRL_PLAY);
  D.btnarray[4]:=CreateIcoButton(@self,waticons.GetIcon,DoAction,WAT_CTRL_PAUSE);
  D.btnarray[5]:=CreateIcoButton(@self,waticons.GetIcon,DoAction,WAT_CTRL_STOP);
  D.btnarray[6]:=CreateIcoButton(@self,waticons.GetIcon,DoAction,WAT_CTRL_NEXT,2000);
end;

