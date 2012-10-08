{Frame data}

const
  //show controls
  scButtons    = $0001;
  scTrackBar   = $0002;
  scText       = $0004;
  scVolume     = $0008;
  scGap        = $0010;
  scAll        = $000F;

  ppLeft   = 0;
  ppRight  = 1;
  //effects
  effCut  = 0;
  effWrap = 1;
  effRoll = 2;
  effPong = 3;

  // Back bitmap mode
  frbkCenterX      = $0001;
  frbkCenterY      = $0002;
  frbkCenter       = frbkCenterX or frbkCenterY;
  frbkTileX        = $0004;
  frbkTileY        = $0008;
  frbkTile         = frbkTileX or frbkTileY;
  frbkStretchX     = $0010;
  frbkStretchY     = $0020;
  frbkStretch      = frbkStretchX or frbkStretchY;
  frbkProportional = $0040;
  frbkBottom       = $0080;
  frbkRight        = $0100;

const
  numbuttons = 7;
  VolBtnDist = 6;
  BtnGap     = 3;
