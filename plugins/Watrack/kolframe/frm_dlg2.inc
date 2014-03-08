{Frame text}
const
  MaxTxtScrollSpeed = 20;

function FrameTextDlg(Dialog:HWND; hMessage:uint; wParam:WPARAM; lParam:LPARAM):LRESULT; stdcall;
const
  DlgInited      :boolean=false;
  TemplateChanged:Boolean=false;
var
  tmp,tmp1:integer;
  tmpb:longbool;
  pcf:TCHOOSEFONT;
  lf:LOGFONTW;
begin
  result:=0;

  case hMessage of
    WM_DESTROY: begin
      // new - if Apply, old - if cancel
      if FrameCtrl<>nil then // for case when FrameCtrl was destryed already
        if PWATFrameData(FrameCtrl.CustomData).TextBlock<>nil then
        PWATFrameData(FrameCtrl.CustomData).TextBlock.FontData:=TextLF;
    end;

    WM_INITDIALOG: begin
      DlgInited:=false;
      TranslateDialogDefault(Dialog);
      with PWATFrameData(FrameCtrl.CustomData).TextBlock^ do
      begin
        case LoByte(Effects) of
          effWrap: tmp:=IDC_EFF_WRAP;
          effRoll: tmp:=IDC_EFF_ROLL;
          effPong: tmp:=IDC_EFF_PONG;
        else // like effCut
          tmp:=IDC_EFF_CUT;
        end;
        CheckDlgButton(Dialog,tmp,BST_CHECKED);

        SetDlgItemInt(Dialog,IDC_TIMER,UpdateTime,false);

        SetDlgItemInt(Dialog,IDC_ROLLSTEP,RollStep,false);
        SetDlgItemInt(Dialog,IDC_ROLLGAP ,RollGap ,false);
  //      SetDlgItemInt(Dialog,IDC_ROLLTAIL,RollTail,false);

        CheckDlgButton(Dialog,IDC_ALCENTER,ord((Effects and effCenter)<>0));

        SetDlgItemTextW(Dialog,IDC_FRAME_TEXT,PWATFrameData(FrameCtrl.CustomData).Template);
      end;

      SendDlgItemMessage(Dialog,IDC_MACRO_HELP,BM_SETIMAGE,IMAGE_ICON,
          CallService(MS_SKIN_LOADICON,SKINICON_OTHER_HELP,0));

      TemplateChanged:=false;
      DlgInited:=true;
    end;

    WM_COMMAND: begin
      case (wParam shr 16) of
        EN_CHANGE: begin
          if Loword(wParam)=IDC_FRAME_TEXT then
            TemplateChanged:=True;
        end;

        BN_CLICKED: begin
          case LoWord(wParam) of
            IDC_MACRO_HELP: CallService(MS_WAT_MACROHELP,Dialog,0);
            IDC_FRMFONT: begin
              with PWATFrameData(FrameCtrl.CustomData).TextBlock^ do
              begin
                lf:=FontData;
  //            lf:=TextLF;
                FillChar(pcf,sizeOf(pcf),0);
                with pcf do
                begin
                  lStructSize:=SizeOf(pcf);
                  lpLogFont:=@lf;
                  Flags:=CF_EFFECTS+CF_FORCEFONTEXIST+CF_LIMITSIZE+CF_NOVERTFONTS+
                         CF_SCREENFONTS+CF_INITTOLOGFONTSTRUCT;
                  rgbColors:=TextColor;
                  nSizeMin:=6;
                  nSizeMax:=32;
                end;
                if ChooseFont(pcf) then
                begin
                  FontData:=lf; // paint directly
                  TextColor:=pcf.rgbColors;
                  SendMessage(GetParent(Dialog),PSM_CHANGED,0,0);
                end
                else
                  exit;
              end;
            end;
          end;
        end;
      end;

      if DlgInited then
        case wParam shr 16 of
          BN_CLICKED,
          EN_CHANGE: begin
            SendMessage(GetParent(Dialog),PSM_CHANGED,0,0);
          end;
        end;
    end;

    WM_NOTIFY: begin
      if integer(PNMHdr(lParam)^.code)=PSN_APPLY then
      begin
//        redraw:=false;
        with PWATFrameData(FrameCtrl.CustomData).TextBlock^ do
        begin
          tmp:=GetDlgItemInt(Dialog,IDC_TIMER,tmpb,false);
          if tmp>MaxTxtScrollSpeed then
            tmp:=MaxTxtScrollSpeed;

          if tmp<>UpdateTime then
          begin
            UpdateTime:=tmp;
{
            if UpdTimer<>0 then
              KillTimer(FrameWnd,UpdTimer);
            if (UpdInterval>0) and (FrameWnd<>0) then
              UpdTimer:=SetTimer(FrameWnd,TMR_TEXT,(MaxTxtScrollSpeed+1-UpdInterval)*100,nil)
            else
              UpdTimer:=0;
}
          end;

          // Text effects
          if      IsDlgButtonChecked(Dialog,IDC_EFF_CUT )<>BST_UNCHECKED then tmp:=effCut
          else if IsDlgButtonChecked(Dialog,IDC_EFF_WRAP)<>BST_UNCHECKED then tmp:=effWrap
          else if IsDlgButtonChecked(Dialog,IDC_EFF_ROLL)<>BST_UNCHECKED then tmp:=effRoll
          else if IsDlgButtonChecked(Dialog,IDC_EFF_PONG)<>BST_UNCHECKED then tmp:=effPong;
          if IsDlgButtonChecked(Dialog,IDC_ALCENTER)<>BST_UNCHECKED then
            tmp:=tmp or effCenter;
          Effects:=tmp;

          tmp1:=GetDlgItemInt(Dialog,IDC_ROLLSTEP,tmpb,false);
          if tmp1<>RollStep then
          begin
            RollStep:=tmp1;
          end;
          tmp1:=GetDlgItemInt(Dialog,IDC_ROLLGAP ,tmpb,false);
          if tmp1<>RollGap then
          begin
            RollGap:=tmp1;
          end;
  {
          tmp1:=GetDlgItemInt(Dialog,IDC_ROLLTAIL,tmpb,false);
          if tmp1<>RollTail then
          begin
            RollTail:=tmp1;
          end;
  }

          if TemplateChanged then
          begin
            mFreeMem(PWATFrameData(FrameCtrl.CustomData).Template);
            PWATFrameData(FrameCtrl.CustomData).Template:=GetDlgText(Dialog,IDC_FRAME_TEXT);
          end;

          TextLF:=FontData; // OK - saving for future?

          SaveTextSettings(TemplateChanged);
          TemplateChanged:=false;
        end;
      end;
    end;
  else
    {result:=}DefWindowProc(Dialog,hMessage,wParam,lParam);
  end;
end;
