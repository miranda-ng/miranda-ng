{$IFNDEF M_ANISMILEY}
{$DEFINE M_ANISMILEY}

const
  IASF_UNICODE = 1;

type
  TINSERTANISMILEY = record
    cbSize     :size_t;
    hWnd       :HWND;
    szFilename :TChar;
    dwBackColor:COLORREF;
    nHeight    :int;
    dwFlags    :dword;
  end;
  tagINSERTANISMILEY = TINSERTANISMILEY;

const
  MS_INSERTANISMILEY :PAnsiChar = 'mAnimator/InsertSmiley';
(*
    function InsertAnimatedSmiley(wnd:HWND;filename:TChar;Back:TCOLORREF;height:int):bool;
    const
      bServiceExists:int=-1
    var
      ias:TINSERTANISMILEY;
    begin
      if bServiceExists=-1 then
          bServiceExists:=ServiceExists(MS_INSERTANISMILEY);
      if bServiceExists=0 then
        result:=false
      else
      begin
        ias.cbSize     :=SizeOf(TINSERTANISMILEY);
        ias.hWnd       :=wnd;
        ias.tcsFilename:=Filename.w;
        ias.dwFlags    :=IASF_UNICODE;
        ias.nHeight    :=Height;
        ias.dwBackColor:=Back;
        result:=CallService(MS_INSERTANISMILEY,WPARAM(@ias),0)<>0;
      end;
    end;

    bool InsertAnimatedSmiley(HWND _hwnd, TCHAR * _szFilename, COLORREF _dwBack, int _nHeight)
    {
        static int bServiceExists=-1;        
         ias={0};
        if (bServiceExists==-1)
            bServiceExists=ServiceExists(MS_INSERTANISMILEY);        
        if (!bServiceExists) return FALSE;
        ias.cbSize=sizeof(INSERTANISMILEY);
        ias.hWnd=_hwnd;
        ias.tcsFilename=_szFilename;
        ias.dwFlags=IASF_TCHAR;
        ias.nHeight=_nHeight;
        ias.dwBackColor=_dwBack;
        return (bool) CallService(MS_INSERTANISMILEY,(WPARAM)&ias, 0);
    };
*)


{
  NM_FIREVIEWCHANGE is WM_NOTIFY Message for notify parent of host window about smiley are going to be repaint

  The proposed action is next: Owner of RichEdit windows received NM_FIREVIEWCHANGE through WM_NOTIFY
  twice first time before painting|invalidating (FVCN_PREFIRE) and second time - after (FVCN_POSTFIRE).
  The Owner window may change any values of received FVCNDATA_NMHDR structure in order to raise needed action.
  For example it may substitute FVCA_INVALIDATE to FVCA_CUSTOMDRAW event to force painting on self offscreen context.
  
  It can be:
  FVCA_CUSTOMDRAW - in this case you need to provide valid HDC to draw on and valid RECT of smiley
  FVCA_INVALIDATE - to invalidate specified rect of window 
  FVCA_NONE    - skip any action. But be aware - animation will be stopped till next repainting of smiley.
  FVCA_SENDVIEWCHANGE - to notify richedit ole about object changed. Be aware Richedit will fully reconstruct itself

  Another point is moment of received smiley rect - it is only valid if FVCA_DRAW is initially set, 
  and it is PROBABLY valid if FVCA_INVALIDATE is set. And it most probably invalid in case of FVCA_SENDVIEWCHANGE.
  The smiley position is relative last full paint HDC. Usually it is relative to top-left corner of host 
  richedit (NOT it client area) in windows coordinates. 
}

const
// Type of Event one of
  FVCN_PREFIRE  = 1;
  FVCN_POSTFIRE = 2;

// Action of event are going to be done
  FVCA_NONE           = 0;
  FVCA_DRAW           = 1;   // do not modify hdc in case of _DRAW, Use _CUSTOMDRAW
  FVCA_CUSTOMDRAW     = 2;
  FVCA_INVALIDATE     = 3;
  FVCA_SENDVIEWCHANGE = 4;
  FVCA_SKIPDRAW       = 5;

// Extended NMHDR structure for WM_NOTIFY
type
  TFVCNDATA_NMHDR = record
    //NMHDR structure
    hwndFrom     :HWND;      // Window of smiley host
    idFrom       :uint_ptr;  // ignored
    code         :uint;      // NM_FIREVIEWCHANGE
    
    cbSize       :size_t;
    bEvent       :byte;      // FVCN_ value - pre- or post- painting
    bAction      :byte;      // FVCA_ keys
    hDC          :HDC;       // Canvas to draw on
    rcRect       :TRECT;     // Valid/should be in case of FVCA_DRAW
    clrBackground:TCOLORREF; // color to fill background if fTransparent is not set
    fTransparent :bool;      // if need to fill back color
    lParam       :LPARAM;    // used by host window PreFire and PostFire event
  end;

const
// Code of WM_NOTIFY message (code)
  NM_FIREVIEWCHANGE = NM_FIRST+1;

{$ENDIF}
