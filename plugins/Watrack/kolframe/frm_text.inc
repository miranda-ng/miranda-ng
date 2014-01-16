{Text}

const
  TextLF:TLOGFONTW = (
    lfHeight        :-10;
    lfWidth         :0;
    lfEscapement    :0;
    lfOrientation   :0;
    lfWeight        :FW_DONTCARE;
    lfItalic        :0;
    lfUnderline     :0;
    lfStrikeOut     :0;
    lfCharSet       :DEFAULT_CHARSET;
    lfOutPrecision  :OUT_DEFAULT_PRECIS;
    lfClipPrecision :CLIP_DEFAULT_PRECIS;
    lfQuality       :DEFAULT_QUALITY;
    lfPitchAndFamily:DEFAULT_PITCH or FF_DONTCARE{;
    lfFaceName      :#0});

const
  opt_FrmTxtClr :PAnsiChar = 'frame/frametextcolor';
  opt_FrmFont   :PAnsiChar = 'frame/framefont';
  opt_FrmEffect :PAnsiChar = 'frame/txteffect';
  opt_RollStep  :PAnsiChar = 'frame/rollstep';
  opt_RollGap   :PAnsiChar = 'frame/rollgap';
//  opt_RollTail  :PAnsiChar = 'frame/rolltail';
  opt_AlgnCenter:PAnsiChar = 'frame/aligncenter';
  opt_TxtTimer  :PAnsiChar = 'frame/texttimer';
  opt_FrameText :PAnsiChar = 'frame/frametext';

procedure UpdateTextBlock(D:PWATFrameData;force:boolean);
var
  tmp:pWideChar;
begin
  if (D.ShowControls and scText)=0 then exit;
  if D.TextBlock=nil then exit;

  if not force then
  begin
    if (StrPosW(D.Template,'%percent%')=nil) and
       (StrPosW(D.Template,'%time%'   )=nil) then // need to |remake
      exit;
  end;
  tmp:=pWideChar(CallService(MS_WAT_REPLACETEXT,0,tlparam(D.Template)));
  D.TextBlock.BlockText:=tmp;
  mFreeMem(tmp);
end;

procedure SaveTextSettings(withtemplate:boolean);
var
  D:PWATFrameData;
begin
  D:=FrameCtrl.CustomData;
  if D.TextBlock=nil then exit;

//  DBWriteByte   (0,PluginShort,opt_RollTail  ,RollTail);
  DBWriteDWord(0,PluginShort,opt_FrmTxtClr,D.TextBlock.TextColor); // reaction on chunk?
  DBWriteByte (0,PluginShort,opt_RollStep ,D.TextBlock.RollStep);
  DBWriteByte (0,PluginShort,opt_RollGap  ,D.TextBlock.RollGap);
  DBWriteWord (0,PluginShort,opt_FrmEffect,D.TextBlock.Effects);
  DBWriteWord (0,PluginShort,opt_TxtTimer ,D.TextBlock.UpdateTime);

  DBWriteStruct(0,PluginShort,opt_FrmFont,@TextLF,SizeOf(TLOGFONT));

  if withtemplate then
  begin
    DBWriteUnicode(0,PluginShort,opt_FrameText,D.Template);
    UpdateTextBlock(D,true);
  end;
end;

procedure LoadTextSettings(TB:pTextBlock);
begin
  if TB=nil then exit;
//  RollTail    :=DBReadByte (0,PluginShort,opt_RollTail  ,20);
  TB.RollStep  :=DBReadByte (0,PluginShort,opt_RollStep ,2);
  TB.RollGap   :=DBReadByte (0,PluginShort,opt_RollGap  ,16);
  TB.TextColor :=DBReadDWord(0,PluginShort,opt_FrmTxtClr,0);
  TB.Effects   :=DBReadWord (0,PluginShort,opt_FrmEffect,effCut or effCenter);
  DBReadStruct(0,PluginShort,opt_FrmFont,@TextLF,SizeOf(TextLF));
  TB.FontData  :=TextLF;
  TB.UpdateTime:=DBReadWord (0,PluginShort,opt_TxtTimer ,10);
end;

function MakeTextBlock(AOwner:PControl;BkColor:TCOLORREF):pTextBlock;
begin
  result:=MakeNewTextBlock(AOwner,BkColor);
//  result.OnMouseDown:=TOnMouse(MakeMethod(nil, @MouseDown));
  LoadTextSettings(result);
end;
