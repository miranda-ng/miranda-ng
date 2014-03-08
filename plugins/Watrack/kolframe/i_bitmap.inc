{}
procedure CalcRect(var src,dst:TRECT;mode:dword);
var
  dh, dw:integer;
begin
  if (Mode and frbkStretch)=frbkStretch then
  begin
    if (Mode and frbkProportional)<>0 then
    begin
      if (dst.right*src.bottom)>(src.right*dst.bottom) then
      begin
        dh:=dst.bottom;
        dw:=dh*src.right div src.bottom
      end
      else
      begin
        dw:=dst.right;
        dh:=dw*src.bottom div src.right;
      end;
    end
    else
    begin
      dw:=dst.right;
      dh:=dst.bottom;
    end;
  end
  else if (Mode and frbkStretchX)<>0 then
  begin
    dw:=dst.right;
    if (Mode and frbkProportional)<>0 then
      dh:=dw*src.bottom div src.right
    else
      dh:=src.bottom;
  end
  else if (Mode and frbkStretchY)<>0 then
  begin
    dh:=dst.bottom;
    if (Mode and frbkProportional)<>0 then
      dw:=dh*src.right div src.bottom
    else
      dw:=src.right;
  end
  else
  begin
    dw:=src.right;
    dh:=src.bottom;
  end;

  if (Mode and frbkBottom)<>0 then
  begin
    if dh<=dst.bottom then
    begin
      dst.top:=(dst.bottom-dh);
    end
    else
    begin
      src.top:=(dh-dst.bottom);
      dh:=dst.bottom;
      src.bottom:=src.top+dh;
    end;
  end;

  if (Mode and frbkRight)<>0 then
  begin
    if dw<=dst.right then
    begin
      dst.left:=(dst.right-dw);
    end
    else
    begin
      src.left:=(dw-dst.right);
      dw:=dst.right;
      src.right:=src.left+dw;
    end;
  end;

  if (Mode and frbkCenterX)<>0 then
  begin
    if dw<=dst.right then
    begin
      dst.left:=(dst.right-dw) div 2;
    end
    else
    begin
      src.left:=(dw-dst.right) div 2;
      dw:=dst.right;
      src.right:=src.left+dw;
    end;
  end;

  if (Mode and frbkCenterY)<>0 then
  begin
    if dh<=dst.bottom then
    begin
      dst.top:=(dst.bottom-dh) div 2;
    end
    else
    begin
      src.top:=(dh-dst.bottom) div 2;
      dh:=dst.bottom;
      src.bottom:=src.top+dh;
    end;
  end;
  dst.right:=dst.left+dw;
  dst.bottom:=dst.top+dh;
end;

function CreateDIB32(dc:HDC;w,h:integer):HBITMAP;
var
  pt:pointer;
  bi:TBITMAPINFO;
begin
  FillChar(bi,SizeOf(TBITMAPINFO),0);
  bi.bmiHeader.biSize    :=SizeOf(TBITMAPINFOHEADER);
  bi.bmiHeader.biWidth   :=w;
  bi.bmiHeader.biHeight  :=h;
  bi.bmiHeader.biPlanes  :=1;
  bi.bmiHeader.biBitCount:=32;
  result:=CreateDIBSection(dc,bi,DIB_RGB_COLORS,pt,0,0);
end;

procedure PreMultiplyChanells(hbmp:HBITMAP);
type
  tPixel=array [0..3] of byte;
var
  bmp:windows.TBITMAP;
  flag:bool;
  pBitmapBits:PByte;
  Len:dword;
  bh,bw,y,x,z:integer;
  pPixel:^tPixel;
  alpha:dword;
//f:THANDLE;
begin
  GetObject(hbmp,SizeOf(bmp),@bmp);
  bh:=bmp.bmHeight;
  bw:=bmp.bmWidth;
  z:=bw*4;
  Len:=bh*z;

  mGetMem(pBitmapBits,Len);
  GetBitmapBits(hbmp,Len,pBitmapBits);
  flag:=true;
  for y:=0 to bh-1 do
  begin
    pointer(pPixel):=PAnsiChar(pBitmapBits)+z*y;

    for x:=0 to bw-1 do
    begin
      if pPixel^[3]<>0 then
        flag:=false
      else
        pPixel^[3]:=255;
      inc(pByte(pPixel),4);
    end
  end;

  if not flag then
  begin
    GetBitmapBits(hbmp,Len,pBitmapBits); // alpha not changed
    for y:=0 to bh-1 do
    begin
      pointer(pPixel):=PAnsiChar(pBitmapBits)+z*y;

      for x:=0 to bw-1 do
      begin
        alpha:=pPixel^[3];
        if alpha<255 then
        begin
          pPixel^[0]:=dword(pPixel^[0])*alpha div 255;
          pPixel^[1]:=dword(pPixel^[1])*alpha div 255;
          pPixel^[2]:=dword(pPixel^[2])*alpha div 255;
        end;
        inc(pByte(pPixel),4);
      end
    end;
  end;
  SetBitmapBits(hbmp,Len,pBitmapBits);
  mFreeMem(pBitmapBits);
end;

function FixBitmap(dc:HDC;var hBmp:HBITMAP):HBITMAP;
var
  dc24,dc32:HDC;
  hBitmap32,obmp24,obmp32:HBITMAP;
  bmpInfo:windows.TBITMAP;
begin
  GetObject(hBmp,SizeOf(bmpInfo),@bmpInfo);
  if bmpInfo.bmBitsPixel<>32 then
  begin
    dc32:=CreateCompatibleDC(dc);
    dc24:=CreateCompatibleDC(dc);
    hBitmap32:=CreateDIB32(dc,bmpInfo.bmWidth,bmpInfo.bmHeight);
    obmp24:=SelectObject(dc24,hBmp);
    obmp32:=SelectObject(dc32,hBitmap32);
    BitBlt(dc32,0,0,bmpInfo.bmWidth,bmpInfo.bmHeight,dc24,0,0,SRCCOPY);
    DeleteObject(SelectObject(dc24,obmp24));
    SelectObject(dc32,obmp32);
    DeleteDC(dc24);
    DeleteDC(dc32);
    hBmp:=hBitmap32;
  end;
  PreMultiplyChanells(hBmp);
  result:=hBmp;
end;

procedure TWATFrame.PreparePicture(dc:HDC;rc:TRECT);
var
  bmpinfo:windows.TBITMAP;
  src,dst:TRECT;
  x,y,w,h,dh:integer;
  br:HBRUSH;
  hdcbmp:HDC;
  bf:BLENDFUNCTION;
  hOld:THANDLE;
  D:PWATFrameData;
begin
  D:=CustomData;
  D.BkDC:=CreateCompatibleDC(dc);

  FixBitmap(dc,D.BkBitmap);

  DeleteObject(SelectObject(D.BkDC,CreateDIB32(dc,rc.right-rc.left,rc.bottom-rc.top)));

  //fill empty space by BK color
  br:=CreateSolidBrush(D.BkColor);
  FillRect(D.BkDC,rc,br);
  DeleteObject(br);

  CopyRect(dst,rc);
  hdcbmp:=CreateCompatibleDC(D.BkDC);
  GetObject(D.BkBitmap,SizeOf(bmpinfo),@bmpinfo);
  hOld:=SelectObject(hdcbmp,D.BkBitmap);

  SetRect(src,0,0,bmpinfo.bmWidth,bmpinfo.bmHeight);

  if (D.padding.top+D.padding.bottom)<(dst.bottom-dst.top) then
    dec(dst.bottom,D.padding.top+D.padding.bottom);
  if (D.padding.left+D.padding.right)<(dst.right-dst.left) then
    dec(dst.right,D.padding.left+D.padding.right);

  CalcRect(src,dst,D.BkMode); // calculate final picture rect

  w:=1;
  if (D.BkMode and frbkTileX)<>0 then
  begin
    x:=dst.right;
    while x<rc.right do
    begin
      inc(w);
      inc(x,dst.right);
    end;
  end;
  h:=1;
  if (D.BkMode and frbkTileY)<>0 then
  begin
    y:=dst.bottom;
    while y<rc.bottom do
    begin
      inc(h);
      inc(y,dst.bottom);
    end;
  end;

  bf.BlendOp            :=AC_SRC_OVER;
  bf.BlendFlags         :=0;
  bf.SourceConstantAlpha:=255;
  bf.AlphaFormat        :=1; // AC_SRC_ALPHA introduced in delphi 7

  x:=dst.left+D.padding.left;
  if x<dst.right then
    while w>0 do
    begin
      dh:=h;
      y:=dst.top+D.padding.top;
      if y<dst.bottom then
        while dh>0 do
        begin
         Windows.AlphaBlend(D.BkDC,x,y,dst.right-dst.left,dst.bottom-dst.top,
             hdcbmp,src.left,src.top,src.right-src.left,src.bottom-src.top,bf);
          inc(y,dst.bottom);
          dec(dh);
        end;
      inc(x,dst.right);
      dec(w);
    end;

  SelectObject(hdcbmp,hOld);
  DeleteDC(hdcbmp);
end;
