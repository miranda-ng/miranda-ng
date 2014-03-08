{*******************************************************************************
  KOL_unicode.inc
 Some redeclarations from Windows.pas for case, when UNICODE_CTRLS symbol is off.
*******************************************************************************}
{$IFDEF interface_part} ////////////////////////////////////////////////////////
  //// for D3 gumno
type
  PRecoveryAgentInformationA = ^TRecoveryAgentInformationA;
  PRecoveryAgentInformationW = ^TRecoveryAgentInformationW;
  PRecoveryAgentInformation = PRecoveryAgentInformationA;
  _RECOVERY_AGENT_INFORMATIONA = record
    NextEntryOffset: DWORD;
    AgentNameLength: DWORD;
    AgentInformation: array[0..0] of AnsiChar;
  end;
  //// _RECOVERY_AGENT_INFORMATIONA}
  _RECOVERY_AGENT_INFORMATIONW = record
    NextEntryOffset: DWORD;
    AgentNameLength: DWORD;
    AgentInformation: array[0..0] of WideChar;
  end;
  //// _RECOVERY_AGENT_INFORMATIONW}
  _RECOVERY_AGENT_INFORMATION = _RECOVERY_AGENT_INFORMATIONA;
  TRecoveryAgentInformationA = _RECOVERY_AGENT_INFORMATIONA;
  TRecoveryAgentInformationW = _RECOVERY_AGENT_INFORMATIONW;
  TRecoveryAgentInformation = TRecoveryAgentInformationA;
  RECOVERY_AGENT_INFORMATIONA = _RECOVERY_AGENT_INFORMATIONA;
  //// RECOVERY_AGENT_INFORMATIONA}
  RECOVERY_AGENT_INFORMATIONW = _RECOVERY_AGENT_INFORMATIONW;
  //// RECOVERY_AGENT_INFORMATIONW}
  RECOVERY_AGENT_INFORMATION = RECOVERY_AGENT_INFORMATIONA;
  //
  PTextMetricA = ^TTextMetricA;
  PTextMetricW = ^TTextMetricW;
  PTextMetric = PTextMetricA;
  tagTEXTMETRICA = record
    tmHeight: Longint;
    tmAscent: Longint;
    tmDescent: Longint;
    tmInternalLeading: Longint;
    tmExternalLeading: Longint;
    tmAveCharWidth: Longint;
    tmMaxCharWidth: Longint;
    tmWeight: Longint;
    tmOverhang: Longint;
    tmDigitizedAspectX: Longint;
    tmDigitizedAspectY: Longint;
    tmFirstChar: AnsiChar;
    tmLastChar: AnsiChar;
    tmDefaultChar: AnsiChar;
    tmBreakChar: AnsiChar;
    tmItalic: Byte;
    tmUnderlined: Byte;
    tmStruckOut: Byte;
    tmPitchAndFamily: Byte;
    tmCharSet: Byte;
  end;
  tagTEXTMETRICW = record
    tmHeight: Longint;
    tmAscent: Longint;
    tmDescent: Longint;
    tmInternalLeading: Longint;
    tmExternalLeading: Longint;
    tmAveCharWidth: Longint;
    tmMaxCharWidth: Longint;
    tmWeight: Longint;
    tmOverhang: Longint;
    tmDigitizedAspectX: Longint;
    tmDigitizedAspectY: Longint;
    tmFirstChar: WideChar;
    tmLastChar: WideChar;
    tmDefaultChar: WideChar;
    tmBreakChar: WideChar;
    tmItalic: Byte;
    tmUnderlined: Byte;
    tmStruckOut: Byte;
    tmPitchAndFamily: Byte;
    tmCharSet: Byte;
  end;
  tagTEXTMETRIC = tagTEXTMETRICA;
  TTextMetricA = tagTEXTMETRICA;
  TTextMetricW = tagTEXTMETRICW;
  TTextMetric = TTextMetricA;
  TEXTMETRICA = tagTEXTMETRICA;
  TEXTMETRICW = tagTEXTMETRICW;
  TEXTMETRIC = TEXTMETRICA;
  ///
  PNewTextMetricA = ^TNewTextMetricA;
  PNewTextMetricW = ^TNewTextMetricW;
  PNewTextMetric = PNewTextMetricA;
  //// tagNEWTEXTMETRICA}
  tagNEWTEXTMETRICA = record
    tmHeight: Longint;
    tmAscent: Longint;
    tmDescent: Longint;
    tmInternalLeading: Longint;
    tmExternalLeading: Longint;
    tmAveCharWidth: Longint;
    tmMaxCharWidth: Longint;
    tmWeight: Longint;
    tmOverhang: Longint;
    tmDigitizedAspectX: Longint;
    tmDigitizedAspectY: Longint;
    tmFirstChar: AnsiChar;
    tmLastChar: AnsiChar;
    tmDefaultChar: AnsiChar;
    tmBreakChar: AnsiChar;
    tmItalic: Byte;
    tmUnderlined: Byte;
    tmStruckOut: Byte;
    tmPitchAndFamily: Byte;
    tmCharSet: Byte;
    ntmFlags: DWORD;
    ntmSizeEM: UINT;
    ntmCellHeight: UINT;
    ntmAvgWidth: UINT;
  end;
  //// tagNEWTEXTMETRICW}
  tagNEWTEXTMETRICW = record
    tmHeight: Longint;
    tmAscent: Longint;
    tmDescent: Longint;
    tmInternalLeading: Longint;
    tmExternalLeading: Longint;
    tmAveCharWidth: Longint;
    tmMaxCharWidth: Longint;
    tmWeight: Longint;
    tmOverhang: Longint;
    tmDigitizedAspectX: Longint;
    tmDigitizedAspectY: Longint;
    tmFirstChar: WideChar;
    tmLastChar: WideChar;
    tmDefaultChar: WideChar;
    tmBreakChar: WideChar;
    tmItalic: Byte;
    tmUnderlined: Byte;
    tmStruckOut: Byte;
    tmPitchAndFamily: Byte;
    tmCharSet: Byte;
    ntmFlags: DWORD;
    ntmSizeEM: UINT;
    ntmCellHeight: UINT;
    ntmAvgWidth: UINT;
  end;
  // tagNEWTEXTMETRIC}
  tagNEWTEXTMETRIC = tagNEWTEXTMETRICA;
  TNewTextMetricA = tagNEWTEXTMETRICA;
  TNewTextMetricW = tagNEWTEXTMETRICW;
  TNewTextMetric = TNewTextMetricA;
  // NEWTEXTMETRICA}
  NEWTEXTMETRICA = tagNEWTEXTMETRICA;
  // NEWTEXTMETRICW}
  NEWTEXTMETRICW = tagNEWTEXTMETRICW;
  // NEWTEXTMETRIC}
  NEWTEXTMETRIC = NEWTEXTMETRICA;
  PNewTextMetricExA = ^TNewTextMetricExA;
  // tagNEWTEXTMETRICEXA}
  tagNEWTEXTMETRICEXA = packed record
    ntmTm: TNewTextMetricA;
    ntmFontSig: TFontSignature;
  end;
  TNewTextMetricExA = tagNEWTEXTMETRICEXA;
  // NEWTEXTMETRICEXA}
  NEWTEXTMETRICEXA = tagNEWTEXTMETRICEXA;
  PNewTextMetricExW = ^TNewTextMetricExW;
  // tagNEWTEXTMETRICEXW}
  tagNEWTEXTMETRICEXW = packed record
    ntmTm: TNewTextMetricW;
    ntmFontSig: TFontSignature;
  end;
  TNewTextMetricExW = tagNEWTEXTMETRICEXW;
  // NEWTEXTMETRICEXW}
  NEWTEXTMETRICEXW = tagNEWTEXTMETRICEXW;
  PNewTextMetricEx = PNewTextMetricExA;
  { Structure passed to FONTENUMPROC }
  PEnumLogFontA = ^TEnumLogFontA;
  PEnumLogFontW = ^TEnumLogFontW;
  PEnumLogFont = PEnumLogFontA;
  // tagENUMLOGFONTA}
  tagENUMLOGFONTA = packed record
    elfLogFont: TLogFontA;
    elfFullName: array[0..LF_FULLFACESIZE - 1] of AnsiChar;
    elfStyle: array[0..LF_FACESIZE - 1] of AnsiChar;
  end;
  // tagENUMLOGFONTW}
  tagENUMLOGFONTW = packed record
    elfLogFont: TLogFontW;
    elfFullName: array[0..LF_FULLFACESIZE - 1] of WideChar;
    elfStyle: array[0..LF_FACESIZE - 1] of WideChar;
  end;
  // tagENUMLOGFONT}
  tagENUMLOGFONT = tagENUMLOGFONTA;
  TEnumLogFontA = tagENUMLOGFONTA;
  TEnumLogFontW = tagENUMLOGFONTW;
  TEnumLogFont = TEnumLogFontA;
  // ENUMLOGFONTA}
  ENUMLOGFONTA = tagENUMLOGFONTA;
  // ENUMLOGFONTW}
  ENUMLOGFONTW = tagENUMLOGFONTW;
  // ENUMLOGFONT}
  ENUMLOGFONT = ENUMLOGFONTA;
  PEnumLogFontExA = ^TEnumLogFontExA;
  PEnumLogFontExW = ^TEnumLogFontExW;
  PEnumLogFontEx = PEnumLogFontExA;
  // tagENUMLOGFONTEXA}
  tagENUMLOGFONTEXA = packed record
    elfLogFont: TLogFontA;
    elfFullName: array[0..LF_FULLFACESIZE - 1] of AnsiChar;
    elfStyle: array[0..LF_FACESIZE - 1] of AnsiChar;
    elfScript: array[0..LF_FACESIZE - 1] of AnsiChar;
  end;
  // tagENUMLOGFONTEXW}
  tagENUMLOGFONTEXW = packed record
    elfLogFont: TLogFontW;
    elfFullName: array[0..LF_FULLFACESIZE - 1] of WideChar;
    elfStyle: array[0..LF_FACESIZE - 1] of WideChar;
    elfScript: array[0..LF_FACESIZE - 1] of WideChar;
  end;
  // tagENUMLOGFONTEX}
  tagENUMLOGFONTEX = tagENUMLOGFONTEXA;
  TEnumLogFontExA = tagENUMLOGFONTEXA;
  TEnumLogFontExW = tagENUMLOGFONTEXW;
  TEnumLogFontEx = TEnumLogFontExA;
  // ENUMLOGFONTEXA}
  ENUMLOGFONTEXA = tagENUMLOGFONTEXA;
  // ENUMLOGFONTEXW}
  ENUMLOGFONTEXW = tagENUMLOGFONTEXW;
  // ENUMLOGFONTEX}
  ENUMLOGFONTEX = ENUMLOGFONTEXA;
  PExtLogFontA = ^TExtLogFontA;
  PExtLogFontW = ^TExtLogFontW;
  PExtLogFont = PExtLogFontA;
  // tagEXTLOGFONTA}
  tagEXTLOGFONTA = record
    elfLogFont: TLogFontA;
    elfFullName: array[0..LF_FULLFACESIZE - 1] of AnsiChar;
    elfStyle: array[0..LF_FACESIZE - 1] of AnsiChar;
    elfVersion: DWORD;     { 0 for the first release of NT }
    elfStyleSize: DWORD;
    elfMatch: DWORD;
    elfReserved: DWORD;
    elfVendorId: array[0..ELF_VENDOR_SIZE - 1] of Byte;
    elfCulture: DWORD;     { 0 for Latin }
    elfPanose: TPanose;
  end;
  // tagEXTLOGFONTW}
  tagEXTLOGFONTW = record
    elfLogFont: TLogFontW;
    elfFullName: array[0..LF_FULLFACESIZE - 1] of WideChar;
    elfStyle: array[0..LF_FACESIZE - 1] of WideChar;
    elfVersion: DWORD;     { 0 for the first release of NT }
    elfStyleSize: DWORD;
    elfMatch: DWORD;
    elfReserved: DWORD;
    elfVendorId: array[0..ELF_VENDOR_SIZE - 1] of Byte;
    elfCulture: DWORD;     { 0 for Latin }
    elfPanose: TPanose;
  end;
  // tagEXTLOGFONT}
  tagEXTLOGFONT = tagEXTLOGFONTA;
  TExtLogFontA = tagEXTLOGFONTA;
  TExtLogFontW = tagEXTLOGFONTW;
  TExtLogFont = TExtLogFontA;
  // EXTLOGFONTA}
  EXTLOGFONTA = tagEXTLOGFONTA;
  // EXTLOGFONTW}
  EXTLOGFONTW = tagEXTLOGFONTW;
  // EXTLOGFONT}
  EXTLOGFONT = EXTLOGFONTA;
  PDisplayDeviceA = ^TDisplayDeviceA;
  PDisplayDeviceW = ^TDisplayDeviceW;
  PDisplayDevice = PDisplayDeviceA;
  // _DISPLAY_DEVICEA}
  _DISPLAY_DEVICEA = packed record
    cb: DWORD;
    DeviceName: array[0..31] of AnsiChar;
    DeviceString: array[0..127] of AnsiChar;
    StateFlags: DWORD;
  end;
  // _DISPLAY_DEVICEW}
  _DISPLAY_DEVICEW = packed record
    cb: DWORD;
    DeviceName: array[0..31] of WideChar;
    DeviceString: array[0..127] of WideChar;
    StateFlags: DWORD;
  end;
  // _DISPLAY_DEVICE}
  _DISPLAY_DEVICE = _DISPLAY_DEVICEA;
  TDisplayDeviceA = _DISPLAY_DEVICEA;
  TDisplayDeviceW = _DISPLAY_DEVICEW;
  TDisplayDevice = TDisplayDeviceA;
  POutlineTextmetricA = ^TOutlineTextmetricA;
  POutlineTextmetricW = ^TOutlineTextmetricW;
  POutlineTextmetric = POutlineTextmetricA;
  // _OUTLINETEXTMETRICA}
  _OUTLINETEXTMETRICA = record
    otmSize: UINT;
    otmTextMetrics: TTextMetricA;
    otmFiller: Byte;
    otmPanoseNumber: TPanose;
    otmfsSelection: UINT;
    otmfsType: UINT;
    otmsCharSlopeRise: Integer;
    otmsCharSlopeRun: Integer;
    otmItalicAngle: Integer;
    otmEMSquare: UINT;
    otmAscent: Integer;
    otmDescent: Integer;
    otmLineGap: UINT;
    otmsCapEmHeight: UINT;
    otmsXHeight: UINT;
    otmrcFontBox: TRect;
    otmMacAscent: Integer;
    otmMacDescent: Integer;
    otmMacLineGap: UINT;
    otmusMinimumPPEM: UINT;
    otmptSubscriptSize: TPoint;
    otmptSubscriptOffset: TPoint;
    otmptSuperscriptSize: TPoint;
    otmptSuperscriptOffset: TPoint;
    otmsStrikeoutSize: UINT;
    otmsStrikeoutPosition: Integer;
    otmsUnderscoreSize: Integer;
    otmsUnderscorePosition: Integer;
    otmpFamilyName: PAnsiChar;
    otmpFaceName: PAnsiChar;
    otmpStyleName: PAnsiChar;
    otmpFullName: PAnsiChar;
  end;
  // _OUTLINETEXTMETRICW}
  _OUTLINETEXTMETRICW = record
    otmSize: UINT;
    otmTextMetrics: TTextMetricW;
    otmFiller: Byte;
    otmPanoseNumber: TPanose;
    otmfsSelection: UINT;
    otmfsType: UINT;
    otmsCharSlopeRise: Integer;
    otmsCharSlopeRun: Integer;
    otmItalicAngle: Integer;
    otmEMSquare: UINT;
    otmAscent: Integer;
    otmDescent: Integer;
    otmLineGap: UINT;
    otmsCapEmHeight: UINT;
    otmsXHeight: UINT;
    otmrcFontBox: TRect;
    otmMacAscent: Integer;
    otmMacDescent: Integer;
    otmMacLineGap: UINT;
    otmusMinimumPPEM: UINT;
    otmptSubscriptSize: TPoint;
    otmptSubscriptOffset: TPoint;
    otmptSuperscriptSize: TPoint;
    otmptSuperscriptOffset: TPoint;
    otmsStrikeoutSize: UINT;
    otmsStrikeoutPosition: Integer;
    otmsUnderscoreSize: Integer;
    otmsUnderscorePosition: Integer;
    otmpFamilyName: PWideChar;
    otmpFaceName: PWideChar;
    otmpStyleName: PWideChar;
    otmpFullName: PWideChar;
  end;
  // _OUTLINETEXTMETRIC}
  _OUTLINETEXTMETRIC = _OUTLINETEXTMETRICA;
  TOutlineTextmetricA = _OUTLINETEXTMETRICA;
  TOutlineTextmetricW = _OUTLINETEXTMETRICW;
  TOutlineTextmetric = TOutlineTextmetricA;
  // OUTLINETEXTMETRICA}
  OUTLINETEXTMETRICA = _OUTLINETEXTMETRICA;
  // OUTLINETEXTMETRICW}
  OUTLINETEXTMETRICW = _OUTLINETEXTMETRICW;
  // OUTLINETEXTMETRIC}
  OUTLINETEXTMETRIC = OUTLINETEXTMETRICA;
  PPolyTextA = ^TPolyTextA;
  PPolyTextW = ^TPolyTextW;
  PPolyText = PPolyTextA;
  // tagPOLYTEXTA}
  tagPOLYTEXTA = packed record
    x: Integer;
    y: Integer;
    n: UINT;
    PAnsiChar: PAnsiChar;
    uiFlags: UINT;
    rcl: TRect;
    pdx: PINT;
  end;
  // tagPOLYTEXTW}
  tagPOLYTEXTW = packed record
    x: Integer;
    y: Integer;
    n: UINT;
    PAnsiChar: PWideChar;
    uiFlags: UINT;
    rcl: TRect;
    pdx: PINT;
  end;
  // tagPOLYTEXT}
  tagPOLYTEXT = tagPOLYTEXTA;
  TPolyTextA = tagPOLYTEXTA;
  TPolyTextW = tagPOLYTEXTW;
  TPolyText = TPolyTextA;
  // POLYTEXTA}
  POLYTEXTA = tagPOLYTEXTA;
  // POLYTEXTW}
  POLYTEXTW = tagPOLYTEXTW;
  // POLYTEXT}
  POLYTEXT = POLYTEXTA;
    PGCPResultsA = ^TGCPResultsA;
  PGCPResultsW = ^TGCPResultsW;
  PGCPResults = PGCPResultsA;
  // tagGCP_RESULTSA}
  tagGCP_RESULTSA = packed record
    lStructSize: DWORD;
    lpOutString: PAnsiChar;
    lpOrder: PUINT;
    lpDx: PINT;
    lpCaretPos: PINT;
    lpClass: PAnsiChar;
    lpGlyphs: PUINT;
    nGlyphs: UINT;
    nMaxFit: Integer;
  end;
  // tagGCP_RESULTSW}
  tagGCP_RESULTSW = packed record
    lStructSize: DWORD;
    lpOutString: PWideChar;
    lpOrder: PUINT;
    lpDx: PINT;
    lpCaretPos: PINT;
    lpClass: PWideChar;
    lpGlyphs: PUINT;
    nGlyphs: UINT;
    nMaxFit: Integer;
  end;
  // tagGCP_RESULTS}
  tagGCP_RESULTS = tagGCP_RESULTSA;
  TGCPResultsA = tagGCP_RESULTSA;
  TGCPResultsW = tagGCP_RESULTSW;
  TGCPResults = TGCPResultsA;
  // GCP_RESULTSA}
  GCP_RESULTSA = tagGCP_RESULTSA;
  // GCP_RESULTSW}
  GCP_RESULTSW = tagGCP_RESULTSW;
  // GCP_RESULTS}
  GCP_RESULTS = GCP_RESULTSA;
const
  MM_MAX_AXES_NAMELEN = 16;
  MM_MAX_NUMAXES = 16;
type
  PAxisInfoA = ^TAxisInfoA;
  tagAXISINFOA = packed record
    axMinValue: Longint;
    axMaxValue: Longint;
    axAxisName: array[0..MM_MAX_AXES_NAMELEN-1] of AnsiChar;
  end;
  TAxisInfoA = tagAXISINFOA;
  PAxisInfoW = ^TAxisInfoW;
  // tagAXISINFOW}
  tagAXISINFOW = packed record
    axMinValue: Longint;
    axMaxValue: Longint;
    axAxisName: array[0..MM_MAX_AXES_NAMELEN-1] of WideChar;
  end;
  TAxisInfoW = tagAXISINFOW;
  PAxisInfo = PAxisInfoA;
  PAxesListA = ^TAxesListA;
  // tagAXESLISTA}
  tagAXESLISTA = packed record
    axlReserved: DWORD;
    axlNumAxes: DWORD;
    axlAxisInfo: array[0..MM_MAX_NUMAXES-1] of TAxisInfoA;
  end;
  TAxesListA = tagAXESLISTA;
  PAxesListW = ^TAxesListW;
  // tagAXESLISTW}
  tagAXESLISTW = packed record
    axlReserved: DWORD;
    axlNumAxes: DWORD;
    axlAxisInfo: array[0..MM_MAX_NUMAXES-1] of TAxisInfoW;
  end;
  TAxesListW = tagAXESLISTW;
  PAxesList = PAxesListA;
  PEnumLogFontExDVA = ^TEnumLogFontExDVA;
  PDesignVector = ^TDesignVector;
  tagDESIGNVECTOR = packed record
    dvReserved: DWORD;
    dvNumAxes: DWORD;
    dvValues: array[0..MM_MAX_NUMAXES-1] of Longint;
  end;
  TDesignVector = tagDESIGNVECTOR;
  tagENUMLOGFONTEXDVA = packed record
    elfEnumLogfontEx: TEnumLogFontExA;
    elfDesignVector: TDesignVector;
  end;
  TEnumLogFontExDVA = tagENUMLOGFONTEXDVA;
  PEnumLogFontExDVW = ^TEnumLogFontExDVW;
  // tagENUMLOGFONTEXDVW}
  tagENUMLOGFONTEXDVW = packed record
    elfEnumLogfontEx: TEnumLogFontExW;
    elfDesignVector: TDesignVector;
  end;
  TEnumLogFontExDVW = tagENUMLOGFONTEXDVW;
  PEnumLogFontExDV = PEnumLogFontExDVA;
  PEnumTextMetricA = ^TEnumTextMetricA;
  // tagENUMTEXTMETRICA}
  tagENUMTEXTMETRICA = packed record
    etmNewTextMetricEx: TNewTextMetricExA;
    etmAxesList: TAxesListA;
  end;
  TEnumTextMetricA = tagENUMTEXTMETRICA;
  PEnumTextMetricW = ^TEnumTextMetricW;
  // tagENUMTEXTMETRICW}
  tagENUMTEXTMETRICW = packed record
    etmNewTextMetricEx: TNewTextMetricExW;
    etmAxesList: TAxesListW;
  end;
  TEnumTextMetricW = tagENUMTEXTMETRICW;
  PEnumTextMetric = PEnumTextMetricA;
  PDocInfoA = ^TDocInfoA;
  PDocInfoW = ^TDocInfoW;
  PDocInfo = PDocInfoA;
  // _DOCINFOA}
  _DOCINFOA = packed record
    cbSize: Integer;
    lpszDocName: PAnsiChar;
    lpszOutput: PAnsiChar;
    lpszDatatype: PAnsiChar;
    fwType: DWORD;
  end;
  // _DOCINFOW}
  _DOCINFOW = packed record
    cbSize: Integer;
    lpszDocName: PWideChar;
    lpszOutput: PWideChar;
    lpszDatatype: PWideChar;
    fwType: DWORD;
  end;
  // _DOCINFO}
  _DOCINFO = _DOCINFOA;
  TDocInfoA = _DOCINFOA;
  TDocInfoW = _DOCINFOW;
  TDocInfo = TDocInfoA;
  // DOCINFOA}
  DOCINFOA = _DOCINFOA;
  // DOCINFOW}
  DOCINFOW = _DOCINFOW;
  // DOCINFO}
  DOCINFO = DOCINFOA;
  PCreateStructA = ^TCreateStructA;
  PCreateStructW = ^TCreateStructW;
  PCreateStruct = PCreateStructA;
  // tagCREATESTRUCTA}
  tagCREATESTRUCTA = packed record
    lpCreateParams: Pointer;
    hInstance: HINST;
    hMenu: HMENU;
    hwndParent: HWND;
    cy: Integer;
    cx: Integer;
    y: Integer;
    x: Integer;
    style: Longint;
    lpszName: PAnsiChar;
    lpszClass: PAnsiChar;
    dwExStyle: DWORD;
  end;
  // tagCREATESTRUCTW}
  tagCREATESTRUCTW = packed record
    lpCreateParams: Pointer;
    hInstance: HINST;
    hMenu: HMENU;
    hwndParent: HWND;
    cy: Integer;
    cx: Integer;
    y: Integer;
    x: Integer;
    style: Longint;
    lpszName: PWideChar;
    lpszClass: PWideChar;
    dwExStyle: DWORD;
  end;
  // tagCREATESTRUCT}
  tagCREATESTRUCT = tagCREATESTRUCTA;
  TCreateStructA = tagCREATESTRUCTA;
  TCreateStructW = tagCREATESTRUCTW;
  TCreateStruct = TCreateStructA;
  // CREATESTRUCTA}
  CREATESTRUCTA = tagCREATESTRUCTA;
  // CREATESTRUCTW}
  CREATESTRUCTW = tagCREATESTRUCTW;
  // CREATESTRUCT}
  CREATESTRUCT = CREATESTRUCTA;
  PWndClassExA = ^TWndClassExA;
  PWndClassExW = ^TWndClassExW;
  PWndClassEx = PWndClassExA;
  // tagWNDCLASSEXA}
  tagWNDCLASSEXA = packed record
    cbSize: UINT;
    style: UINT;
    lpfnWndProc: TFNWndProc;
    cbClsExtra: Integer;
    cbWndExtra: Integer;
    hInstance: HINST;
    hIcon: HICON;
    hCursor: HCURSOR;
    hbrBackground: HBRUSH;
    lpszMenuName: PAnsiChar;
    lpszClassName: PAnsiChar;
    hIconSm: HICON;
  end;
  // tagWNDCLASSEXW}
  tagWNDCLASSEXW = packed record
    cbSize: UINT;
    style: UINT;
    lpfnWndProc: TFNWndProc;
    cbClsExtra: Integer;
    cbWndExtra: Integer;
    hInstance: HINST;
    hIcon: HICON;
    hCursor: HCURSOR;
    hbrBackground: HBRUSH;
    lpszMenuName: PWideChar;
    lpszClassName: PWideChar;
    hIconSm: HICON;
  end;
  // tagWNDCLASSEX}
  tagWNDCLASSEX = tagWNDCLASSEXA;
  TWndClassExA = tagWNDCLASSEXA;
  TWndClassExW = tagWNDCLASSEXW;
  TWndClassEx = TWndClassExA;
  // WNDCLASSEXA}
  WNDCLASSEXA = tagWNDCLASSEXA;
  // WNDCLASSEXW}
  WNDCLASSEXW = tagWNDCLASSEXW;
  // WNDCLASSEX}
  WNDCLASSEX = WNDCLASSEXA;

  PWndClassA = ^TWndClassA;
  PWndClassW = ^TWndClassW;
  PWndClass = PWndClassA;
  // tagWNDCLASSA}
  tagWNDCLASSA = packed record
    style: UINT;
    lpfnWndProc: TFNWndProc;
    cbClsExtra: Integer;
    cbWndExtra: Integer;
    hInstance: HINST;
    hIcon: HICON;
    hCursor: HCURSOR;
    hbrBackground: HBRUSH;
    lpszMenuName: PAnsiChar;
    lpszClassName: PAnsiChar;
  end;
  // tagWNDCLASSW}
  tagWNDCLASSW = packed record
    style: UINT;
    lpfnWndProc: TFNWndProc;
    cbClsExtra: Integer;
    cbWndExtra: Integer;
    hInstance: HINST;
    hIcon: HICON;
    hCursor: HCURSOR;
    hbrBackground: HBRUSH;
    lpszMenuName: PWideChar;
    lpszClassName: PWideChar;
  end;
  // tagWNDCLASS}
  tagWNDCLASS = tagWNDCLASSA;
  TWndClassA = tagWNDCLASSA;
  TWndClassW = tagWNDCLASSW;
  TWndClass = TWndClassA;
  // WNDCLASSA}
  WNDCLASSA = tagWNDCLASSA;
  // WNDCLASSW}
  WNDCLASSW = tagWNDCLASSW;
  // WNDCLASS}
  WNDCLASS = WNDCLASSA;
  HDEVNOTIFY = Pointer;
  PHDEVNOTIFY = ^HDEVNOTIFY;
  ////
  MakeIntAtom = MakeIntAtomA;
  PWin32FindData = PWin32FindDataA;
  TWin32FindData = TWin32FindDataA;
  {$IFDEF _D3orHigher}
  PHWProfileInfo = PHWProfileInfoA;
  THWProfileInfo = THWProfileInfoA;
  {$ENDIF}
  POSVersionInfo = POSVersionInfoA;
  TOSVersionInfo = TOSVersionInfoA;
  PLogColorSpace = PLogColorSpaceA;
  TLogColorSpace = TLogColorSpaceA;
  PLogFont = PLogFontA;
  TLogFont = TLogFontA;
  PDeviceMode = PDeviceModeA;
  TDeviceMode = TDeviceModeA;
  TFNOldFontEnumProc = TFNOldFontEnumProcA;
  TFNFontEnumProc = TFNFontEnumProcA;
  MakeIntResource = PAnsiChar; // MakeIntResourceA;
  //PMenuItemInfo = PMenuItemInfoA;
  //TMenuItemInfo = TMenuItemInfoA;
  //MENUITEMINFO = MENUITEMINFOA;
  PMsgBoxParams = PMsgBoxParamsA;
  TMsgBoxParams = TMsgBoxParamsA;
  PMsgBoxParamsA = ^TMsgBoxParamsA;
  PMsgBoxParamsW = ^TMsgBoxParamsW;
  // tagMSGBOXPARAMSA}
  tagMSGBOXPARAMSA = packed record
    cbSize: UINT;
    hwndOwner: HWND;
    hInstance: HINST;
    lpszText: PAnsiChar;
    lpszCaption: PAnsiChar;
    dwStyle: DWORD;
    lpszIcon: PAnsiChar;
    dwContextHelpId: DWORD;
    lpfnMsgBoxCallback: TPRMsgBoxCallback;
    dwLanguageId: DWORD;
  end;
  // tagMSGBOXPARAMSW}
  tagMSGBOXPARAMSW = packed record
    cbSize: UINT;
    hwndOwner: HWND;
    hInstance: HINST;
    lpszText: PWideChar;
    lpszCaption: PWideChar;
    dwStyle: DWORD;
    lpszIcon: PWideChar;
    dwContextHelpId: DWORD;
    lpfnMsgBoxCallback: TPRMsgBoxCallback;
    dwLanguageId: DWORD;
  end;
  // tagMSGBOXPARAMS}
  tagMSGBOXPARAMS = tagMSGBOXPARAMSA;
  TMsgBoxParamsA = tagMSGBOXPARAMSA;
  TMsgBoxParamsW = tagMSGBOXPARAMSW;
  // MSGBOXPARAMSA}
  MSGBOXPARAMSA = tagMSGBOXPARAMSA;
  // MSGBOXPARAMSW}
  MSGBOXPARAMSW = tagMSGBOXPARAMSW;
  // MSGBOXPARAMS}
  MSGBOXPARAMS = MSGBOXPARAMSA;
  PMDICreateStruct = PMDICreateStructA;
  TMDICreateStruct = TMDICreateStructA;
  PMultiKeyHelp = PMultiKeyHelpA;
  TMultiKeyHelp = TMultiKeyHelpA;
  // HELPPOLY}
  HELPPOLY = DWORD;
  PMultiKeyHelpA = ^TMultiKeyHelpA;
  PMultiKeyHelpW = ^TMultiKeyHelpW;
  // tagMULTIKEYHELPA}
  tagMULTIKEYHELPA = record
    mkSize: DWORD;
    mkKeylist: AnsiChar;
    szKeyphrase: array[0..0] of AnsiChar;
  end;
  // tagMULTIKEYHELPW}
  tagMULTIKEYHELPW = record
    mkSize: DWORD;
    mkKeylist: WideChar;
    szKeyphrase: array[0..0] of WideChar;
  end;
  // tagMULTIKEYHELP}
  tagMULTIKEYHELP = tagMULTIKEYHELPA;
  TMultiKeyHelpA = tagMULTIKEYHELPA;
  TMultiKeyHelpW = tagMULTIKEYHELPW;
  // MULTIKEYHELPA}
  MULTIKEYHELPA = tagMULTIKEYHELPA;
  // MULTIKEYHELPW}
  MULTIKEYHELPW = tagMULTIKEYHELPW;
  // MULTIKEYHELP}
  MULTIKEYHELP = MULTIKEYHELPA;
  PHelpWinInfoA = ^THelpWinInfoA;
  PHelpWinInfoW = ^THelpWinInfoW;
  PHelpWinInfo = PHelpWinInfoA;
  // tagHELPWININFOA}
  tagHELPWININFOA = record
    wStructSize: Integer;
    x: Integer;
    y: Integer;
    dx: Integer;
    dy: Integer;
    wMax: Integer;
    rgchMember: array[0..1] of AnsiChar;
  end;
  // tagHELPWININFOW}
  tagHELPWININFOW = record
    wStructSize: Integer;
    x: Integer;
    y: Integer;
    dx: Integer;
    dy: Integer;
    wMax: Integer;
    rgchMember: array[0..1] of WideChar;
  end;
  // tagHELPWININFO}
  tagHELPWININFO = tagHELPWININFOA;
  THelpWinInfoA = tagHELPWININFOA;
  THelpWinInfoW = tagHELPWININFOW;
  THelpWinInfo = THelpWinInfoA;
  // HELPWININFOA}
  HELPWININFOA = tagHELPWININFOA;
  // HELPWININFOW}
  HELPWININFOW = tagHELPWININFOW;
  // HELPWININFO}
  HELPWININFO = HELPWININFOA;
  // tagNONCLIENTMETRICSA}
  tagNONCLIENTMETRICSA = packed record
    cbSize: UINT;
    iBorderWidth: Integer;
    iScrollWidth: Integer;
    iScrollHeight: Integer;
    iCaptionWidth: Integer;
    iCaptionHeight: Integer;
    lfCaptionFont: TLogFontA;
    iSmCaptionWidth: Integer;
    iSmCaptionHeight: Integer;
    lfSmCaptionFont: TLogFontA;
    iMenuWidth: Integer;
    iMenuHeight: Integer;
    lfMenuFont: TLogFontA;
    lfStatusFont: TLogFontA;
    lfMessageFont: TLogFontA;
  end;
  // tagNONCLIENTMETRICSW}
  tagNONCLIENTMETRICSW = packed record
    cbSize: UINT;
    iBorderWidth: Integer;
    iScrollWidth: Integer;
    iScrollHeight: Integer;
    iCaptionWidth: Integer;
    iCaptionHeight: Integer;
    lfCaptionFont: TLogFontW;
    iSmCaptionWidth: Integer;
    iSmCaptionHeight: Integer;
    lfSmCaptionFont: TLogFontW;
    iMenuWidth: Integer;
    iMenuHeight: Integer;
    lfMenuFont: TLogFontW;
    lfStatusFont: TLogFontW;
    lfMessageFont: TLogFontW;
  end;
  // tagNONCLIENTMETRICS}
  tagNONCLIENTMETRICS = tagNONCLIENTMETRICSA;
  TNonClientMetricsA = tagNONCLIENTMETRICSA;
  TNonClientMetricsW = tagNONCLIENTMETRICSW;
  PNonClientMetricsA = ^TNonClientMetricsA;
  PNonClientMetrics = PNonClientMetricsA;
  TNonClientMetrics = TNonClientMetricsA;
  PNonClientMetricsW = ^TNonClientMetricsW;
  // NONCLIENTMETRICSA}
  NONCLIENTMETRICSA = tagNONCLIENTMETRICSA;
  // NONCLIENTMETRICSW}
  NONCLIENTMETRICSW = tagNONCLIENTMETRICSW;
  // NONCLIENTMETRICS}
  NONCLIENTMETRICS = NONCLIENTMETRICSA;
  // tagICONMETRICSA}
  tagICONMETRICSA = packed record
    cbSize: UINT;
    iHorzSpacing: Integer;
    iVertSpacing: Integer;
    iTitleWrap: Integer;
    lfFont: TLogFontA;
  end;
  // tagICONMETRICSW}
  tagICONMETRICSW = packed record
    cbSize: UINT;
    iHorzSpacing: Integer;
    iVertSpacing: Integer;
    iTitleWrap: Integer;
    lfFont: TLogFontW;
  end;
  // tagICONMETRICS}
  tagICONMETRICS = tagICONMETRICSA;
  TIconMetricsA = tagICONMETRICSA;
  TIconMetricsW = tagICONMETRICSW;
  PIconMetricsA = ^TIconMetricsA;
  PIconMetricsW = ^TIconMetricsW;
  PIconMetrics = PIconMetricsA;
  TIconMetrics = TIconMetricsA;
  // ICONMETRICSA}
  ICONMETRICSA = tagICONMETRICSA;
  // ICONMETRICSW}
  ICONMETRICSW = tagICONMETRICSW;
  // ICONMETRICS}
  ICONMETRICS = ICONMETRICSA;
  PSerialKeys = PSerialKeysA;
  TSerialKeys = TSerialKeysA;
  PSerialKeysA = ^TSerialKeysA;
  PSerialKeysW = ^TSerialKeysW;
  // tagSERIALKEYSA}
  tagSERIALKEYSA = packed record
    cbSize: UINT;
    dwFlags: DWORD;
    lpszActivePort: PAnsiChar;
    lpszPort: PAnsiChar;
    iBaudRate: UINT;
    iPortState: UINT;
    iActive: UINT;
  end;
  // tagSERIALKEYSW}
  tagSERIALKEYSW = packed record
    cbSize: UINT;
    dwFlags: DWORD;
    lpszActivePort: PWideChar;
    lpszPort: PWideChar;
    iBaudRate: UINT;
    iPortState: UINT;
    iActive: UINT;
  end;
  // tagSERIALKEYS}
  tagSERIALKEYS = tagSERIALKEYSA;
  TSerialKeysA = tagSERIALKEYSA;
  TSerialKeysW = tagSERIALKEYSW;
  // SERIALKEYSA}
  SERIALKEYSA = tagSERIALKEYSA;
  // SERIALKEYSW}
  SERIALKEYSW = tagSERIALKEYSW;
  // SERIALKEYS}
  SERIALKEYS = SERIALKEYSA;
  PHighContrast = PHighContrastA;
  THighContrast = THighContrastA;
  PHighContrastA = ^THighContrastA;
  PHighContrastW = ^THighContrastW;
  // tagHIGHCONTRASTA}
  tagHIGHCONTRASTA = packed record
    cbSize: UINT;
    dwFlags: DWORD;
    lpszDefaultScheme: PAnsiChar;
  end;
  // tagHIGHCONTRASTW}
  tagHIGHCONTRASTW = packed record
    cbSize: UINT;
    dwFlags: DWORD;
    lpszDefaultScheme: PWideChar;
  end;
  // tagHIGHCONTRAST}
  tagHIGHCONTRAST = tagHIGHCONTRASTA;
  THighContrastA = tagHIGHCONTRASTA;
  THighContrastW = tagHIGHCONTRASTW;
  // HIGHCONTRASTA}
  HIGHCONTRASTA = tagHIGHCONTRASTA;
  // HIGHCONTRASTW}
  HIGHCONTRASTW = tagHIGHCONTRASTW;
  // HIGHCONTRAST}
  HIGHCONTRAST = HIGHCONTRASTA;
  PSoundsEntry = PSoundsEntryA;
  TSoundsEntry = TSoundsEntryA;
  PSoundsEntryA = ^TSoundsEntryA;
  PSoundsEntryW = ^TSoundsEntryW;
  // tagSOUNDSENTRYA}
  tagSOUNDSENTRYA = packed record
    cbSize: UINT;
    dwFlags: DWORD;
    iFSTextEffect: DWORD;
    iFSTextEffectMSec: DWORD;
    iFSTextEffectColorBits: DWORD;
    iFSGrafEffect: DWORD;
    iFSGrafEffectMSec: DWORD;
    iFSGrafEffectColor: DWORD;
    iWindowsEffect: DWORD;
    iWindowsEffectMSec: DWORD;
    lpszWindowsEffectDLL: PAnsiChar;
    iWindowsEffectOrdinal: DWORD;
  end;
  // tagSOUNDSENTRYW}
  tagSOUNDSENTRYW = packed record
    cbSize: UINT;
    dwFlags: DWORD;
    iFSTextEffect: DWORD;
    iFSTextEffectMSec: DWORD;
    iFSTextEffectColorBits: DWORD;
    iFSGrafEffect: DWORD;
    iFSGrafEffectMSec: DWORD;
    iFSGrafEffectColor: DWORD;
    iWindowsEffect: DWORD;
    iWindowsEffectMSec: DWORD;
    lpszWindowsEffectDLL: PWideChar;
    iWindowsEffectOrdinal: DWORD;
  end;
  // tagSOUNDSENTRY}
  tagSOUNDSENTRY = tagSOUNDSENTRYA;
  TSoundsEntryA = tagSOUNDSENTRYA;
  TSoundsEntryW = tagSOUNDSENTRYW;
  // SOUNDSENTRYA}
  SOUNDSENTRYA = tagSOUNDSENTRYA;
  // SOUNDSENTRYW}
  SOUNDSENTRYW = tagSOUNDSENTRYW;
  // SOUNDSENTRY}
  SOUNDSENTRY = SOUNDSENTRYA;
  PNumberFmt = PNumberFmtA;
  TNumberFmt = TNumberFmtA;
  PNumberFmtA = ^TNumberFmtA;
  PNumberFmtW = ^TNumberFmtW;
  // _numberfmtA}
  _numberfmtA = packed record
    NumDigits: UINT;        { number of decimal digits }
    LeadingZero: UINT;      { if leading zero in decimal fields }
    Grouping: UINT;         { group size left of decimal }
    lpDecimalSep: PAnsiChar;   { ptr to decimal separator AnsiString }
    lpThousandSep: PAnsiChar;  { ptr to thousand separator AnsiString }
    NegativeOrder: UINT;    { negative number ordering }
  end;
  // _numberfmtW}
  _numberfmtW = packed record
    NumDigits: UINT;        { number of decimal digits }
    LeadingZero: UINT;      { if leading zero in decimal fields }
    Grouping: UINT;         { group size left of decimal }
    lpDecimalSep: PWideChar;   { ptr to decimal separator WideString }
    lpThousandSep: PWideChar;  { ptr to thousand separator WideString }
    NegativeOrder: UINT;    { negative number ordering }
  end;
  // _numberfmt}
  _numberfmt = _numberfmtA;
  TNumberFmtA = _numberfmtA;
  TNumberFmtW = _numberfmtW;
  // NUMBERFMTA}
  NUMBERFMTA = _numberfmtA;
  // NUMBERFMTW}
  NUMBERFMTW = _numberfmtW;
  // NUMBERFMT}
  NUMBERFMT = NUMBERFMTA;
  PCurrencyFmt = PCurrencyFmtA;
  PCurrencyFmtA = ^TCurrencyFmtA;
  PCurrencyFmtW = ^TCurrencyFmtW;
  // _currencyfmtA}
  _currencyfmtA = packed record
    NumDigits: UINT;           { number of decimal digits }
    LeadingZero: UINT;         { if leading zero in decimal fields }
    Grouping: UINT;            { group size left of decimal }
    lpDecimalSep: PAnsiChar;      { ptr to decimal separator AnsiString }
    lpThousandSep: PAnsiChar;     { ptr to thousand separator AnsiString }
    NegativeOrder: UINT;       { negative currency ordering }
    PositiveOrder: UINT;       { positive currency ordering }
    lpCurrencySymbol: PAnsiChar;  { ptr to currency symbol AnsiString }
  end;
  // _currencyfmtW}
  _currencyfmtW = packed record
    NumDigits: UINT;           { number of decimal digits }
    LeadingZero: UINT;         { if leading zero in decimal fields }
    Grouping: UINT;            { group size left of decimal }
    lpDecimalSep: PWideChar;      { ptr to decimal separator WideString }
    lpThousandSep: PWideChar;     { ptr to thousand separator WideString }
    NegativeOrder: UINT;       { negative currency ordering }
    PositiveOrder: UINT;       { positive currency ordering }
    lpCurrencySymbol: PWideChar;  { ptr to currency symbol WideString }
  end;
  // _currencyfmt}
  _currencyfmt = _currencyfmtA;
  TCurrencyFmtA = _currencyfmtA;
  TCurrencyFmtW = _currencyfmtW;
  TCurrencyFmt = TCurrencyFmtA;
  // CURRENCYFMTA}
  CURRENCYFMTA = _currencyfmtA;
  // CURRENCYFMTW}
  CURRENCYFMTW = _currencyfmtW;
  // CURRENCYFMT}
  CURRENCYFMT = CURRENCYFMTA;
  PPValue = PPValueA;
{ Provider supplied value/context.}
  PPValueA = ^TPValueA;
  PPValueW = ^TPValueW;
  // pvalueA}
  pvalueA = packed record
    pv_valuename: PAnsiChar;           { The value name pointer }
    pv_valuelen: BOOL;
    pv_value_context: Pointer;
    pv_type: DWORD;
  end;
  // pvalueW}
  pvalueW = packed record
    pv_valuename: PWideChar;           { The value name pointer }
    pv_valuelen: BOOL;
    pv_value_context: Pointer;
    pv_type: DWORD;
  end;
  // pvalue}
  pvalue = pvalueA;
  TPValueA = pvalueA;
  TPValueW = pvalueW;
  TPValue = TPValueA;
  PValueEnt = PValueEntA;
  TValueEnt = TValueEntA;
  PValueEntA = ^TValueEntA;
  PValueEntW = ^TValueEntW;
  // value_entA}
  value_entA = packed record
    ve_valuename: PAnsiChar;
    ve_valuelen: DWORD;
    ve_valueptr: DWORD;
    ve_type: DWORD;
  end;
  // value_entW}
  value_entW = packed record
    ve_valuename: PWideChar;
    ve_valuelen: DWORD;
    ve_valueptr: DWORD;
    ve_type: DWORD;
  end;
  // value_ent}
  value_ent = value_entA;
  TValueEntA = value_entA;
  TValueEntW = value_entW;
  // VALENTA}
  VALENTA = value_entA;
  // VALENTW}
  VALENTW = value_entW;
  // VALENT}
  VALENT = VALENTA;
  TValEnt = TValueEnt;
  PValEnt = PValueEnt;
  PNetResource = PNetResourceA;
  TNetResource = TNetResourceA;
  PNetResourceA = ^TNetResourceA;
  PNetResourceW = ^TNetResourceW;
  // _NETRESOURCEA}
  _NETRESOURCEA = packed record
    dwScope: DWORD;
    dwType: DWORD;
    dwDisplayType: DWORD;
    dwUsage: DWORD;
    lpLocalName: PAnsiChar;
    lpRemoteName: PAnsiChar;
    lpComment: PAnsiChar;
    lpProvider: PAnsiChar;
  end;
  // _NETRESOURCEW}
  _NETRESOURCEW = packed record
    dwScope: DWORD;
    dwType: DWORD;
    dwDisplayType: DWORD;
    dwUsage: DWORD;
    lpLocalName: PWideChar;
    lpRemoteName: PWideChar;
    lpComment: PWideChar;
    lpProvider: PWideChar;
  end;
  // _NETRESOURCE}
  _NETRESOURCE = _NETRESOURCEA;
  TNetResourceA = _NETRESOURCEA;
  TNetResourceW = _NETRESOURCEW;
  // NETRESOURCEA}
  NETRESOURCEA = _NETRESOURCEA;
  // NETRESOURCEW}
  NETRESOURCEW = _NETRESOURCEW;
  // NETRESOURCE}
  NETRESOURCE = NETRESOURCEA;
  PDiscDlgStruct = PDiscDlgStructA;
  PDiscDlgStructA = ^TDiscDlgStructA;
  PDiscDlgStructW = ^TDiscDlgStructW;
  // _DISCDLGSTRUCTA}
  _DISCDLGSTRUCTA = packed record
    cbStructure: DWORD;       { size of this structure in bytes }
    hwndOwner: HWND;          { owner window for the dialog }
    lpLocalName: PAnsiChar;       { local device name }
    lpRemoteName: PAnsiChar;      { network resource name }
    dwFlags: DWORD;
  end;
  // _DISCDLGSTRUCTW}
  _DISCDLGSTRUCTW = packed record
    cbStructure: DWORD;       { size of this structure in bytes }
    hwndOwner: HWND;          { owner window for the dialog }
    lpLocalName: PWideChar;       { local device name }
    lpRemoteName: PWideChar;      { network resource name }
    dwFlags: DWORD;
  end;
  // _DISCDLGSTRUCT}
  _DISCDLGSTRUCT = _DISCDLGSTRUCTA;
  TDiscDlgStructA = _DISCDLGSTRUCTA;
  TDiscDlgStructW = _DISCDLGSTRUCTW;
  TDiscDlgStruct = TDiscDlgStructA;
  // DISCDLGSTRUCTA}
  DISCDLGSTRUCTA = _DISCDLGSTRUCTA;
  // DISCDLGSTRUCTW}
  DISCDLGSTRUCTW = _DISCDLGSTRUCTW;
  // DISCDLGSTRUCT}
  DISCDLGSTRUCT = DISCDLGSTRUCTA;
  PUniversalNameInfo = PUniversalNameInfoA;
  TUniversalNameInfo = TUniversalNameInfoA;
  PUniversalNameInfoA = ^TUniversalNameInfoA;
  PUniversalNameInfoW = ^TUniversalNameInfoW;
  // _UNIVERSAL_NAME_INFOA}
  _UNIVERSAL_NAME_INFOA = packed record
    lpUniversalName: PAnsiChar;
  end;
  // _UNIVERSAL_NAME_INFOW}
  _UNIVERSAL_NAME_INFOW = packed record
    lpUniversalName: PWideChar;
  end;
  // _UNIVERSAL_NAME_INFO}
  _UNIVERSAL_NAME_INFO = _UNIVERSAL_NAME_INFOA;
  TUniversalNameInfoA = _UNIVERSAL_NAME_INFOA;
  TUniversalNameInfoW = _UNIVERSAL_NAME_INFOW;
  // UNIVERSAL_NAME_INFOA}
  UNIVERSAL_NAME_INFOA = _UNIVERSAL_NAME_INFOA;
  // UNIVERSAL_NAME_INFOW}
  UNIVERSAL_NAME_INFOW = _UNIVERSAL_NAME_INFOW;
  // UNIVERSAL_NAME_INFO}
  UNIVERSAL_NAME_INFO = UNIVERSAL_NAME_INFOA;
  PRemoteNameInfo = PRemoteNameInfoA;
  TRemoteNameInfo = TRemoteNameInfoA;
  PRemoteNameInfoA = ^TRemoteNameInfoA;
  PRemoteNameInfoW = ^TRemoteNameInfoW;
  // _REMOTE_NAME_INFOA}
  _REMOTE_NAME_INFOA = packed record
    lpUniversalName: PAnsiChar;
    lpConnectionName: PAnsiChar;
    lpRemainingPath: PAnsiChar;
  end;
  // _REMOTE_NAME_INFOW}
  _REMOTE_NAME_INFOW = packed record
    lpUniversalName: PWideChar;
    lpConnectionName: PWideChar;
    lpRemainingPath: PWideChar;
  end;
  // _REMOTE_NAME_INFO}
  _REMOTE_NAME_INFO = _REMOTE_NAME_INFOA;
  TRemoteNameInfoA = _REMOTE_NAME_INFOA;
  TRemoteNameInfoW = _REMOTE_NAME_INFOW;
  // REMOTE_NAME_INFOA}
  REMOTE_NAME_INFOA = _REMOTE_NAME_INFOA;
  // REMOTE_NAME_INFOW}
  REMOTE_NAME_INFOW = _REMOTE_NAME_INFOW;
  // REMOTE_NAME_INFO}
  REMOTE_NAME_INFO = REMOTE_NAME_INFOA;
  AUDIT_EVENT_TYPE = DWORD;
  {$IFDEF _D3orHigher}
  PObjectTypeList = ^TObjectTypeList;
  _OBJECT_TYPE_LIST = record
    Level: WORD;
    Sbz: WORD;
    ObjectType: PGUID;
  end;
  TObjectTypeList = _OBJECT_TYPE_LIST;
  OBJECT_TYPE_LIST = _OBJECT_TYPE_LIST;
  {$ENDIF _D3orHigher}
  { Alt-Tab Switch window information. }
  PAltTabInfo = ^TAltTabInfo;
  tagALTTABINFO = packed record
    cbSize: DWORD;
    cItems: Integer;
    cColumns: Integer;
    cRows: Integer;
    iColFocus: Integer;
    iRowFocus: Integer;
    cxItem: Integer;
    cyItem: Integer;
    ptStart: TPoint;
  end;
  TAltTabInfo = tagALTTABINFO;

function AbortSystemShutdown(lpMachineName: PKOLChar): BOOL; stdcall;
function AccessCheckAndAuditAlarm(SubsystemName: PKOLChar;
  HandleId: Pointer; ObjectTypeName, ObjectName: PKOLChar;
  SecurityDescriptor: PSecurityDescriptor; DesiredAccess: DWORD;
  const GenericMapping: TGenericMapping;  ObjectCreation: BOOL;
  var GrantedAccess: DWORD; var AccessStatus, pfGenerateOnClose: BOOL): BOOL; stdcall;
{$IFDEF _D4orHigher}
function AccessCheckByTypeAndAuditAlarm(SubsystemName: PKOLChar;
  HandleId: Pointer; ObjectTypeName, ObjectName: PKOLChar;
  SecurityDescriptor: PSecurityDescriptor; PrincipalSelfSid: PSID; DesiredAccess: DWORD;
  AuditType: AUDIT_EVENT_TYPE; Flags: DWORD; ObjectTypeList: PObjectTypeList;
  ObjectTypeListLength: DWORD; const GenericMapping: TGenericMapping;  ObjectCreation: BOOL;
  var GrantedAccess: DWORD; var AccessStatus, pfGenerateOnClose: BOOL): BOOL; stdcall;
function AccessCheckByTypeResultListAndAuditAlarm(SubsystemName: PKOLChar;
  HandleId: Pointer; ObjectTypeName, ObjectName: PKOLChar;
  SecurityDescriptor: PSecurityDescriptor; PrincipalSelfSid: PSID; DesiredAccess: DWORD;
  AuditType: AUDIT_EVENT_TYPE; Flags: DWORD; ObjectTypeList: PObjectTypeList;
  ObjectTypeListLength: DWORD; const GenericMapping: TGenericMapping;  ObjectCreation: BOOL;
  var GrantedAccess: DWORD; var AccessStatusList: DWORD; var pfGenerateOnClose: BOOL): BOOL; stdcall;
{$ENDIF _D4orHigher}  
function BackupEventLog(hEventLog: THandle; lpBackupFileName: PKOLChar): BOOL; stdcall;
function ClearEventLog(hEventLog: THandle; lpBackupFileName: PKOLChar): BOOL; stdcall;
function CreateProcessAsUser(hToken: THandle; lpApplicationName: PKOLChar;
  lpCommandLine: PKOLChar; lpProcessAttributes: PSecurityAttributes;
  lpThreadAttributes: PSecurityAttributes; bInheritHandles: BOOL;
  dwCreationFlags: DWORD; lpEnvironment: Pointer; lpCurrentDirectory: PKOLChar;
  const lpStartupInfo: TStartupInfo; var lpProcessInformation: TProcessInformation): BOOL; stdcall;
{$IFDEF _D3orHigher}
function GetCurrentHwProfile(var lpHwProfileInfo: THWProfileInfo): BOOL; stdcall;
{$ENDIF _D3orHigher}
function GetFileSecurity(lpFileName: PKOLChar; RequestedInformation: SECURITY_INFORMATION;
  pSecurityDescriptor: PSecurityDescriptor; nLength: DWORD; var lpnLengthNeeded: DWORD): BOOL; stdcall;
function GetUserName(lpBuffer: PKOLChar; var nSize: DWORD): BOOL; stdcall;
function InitiateSystemShutdown(lpMachineName, lpMessage: PKOLChar;
  dwTimeout: DWORD; bForceAppsClosed, bRebootAfterShutdown: BOOL): BOOL; stdcall;
function LogonUser(lpszUsername, lpszDomain, lpszPassword: PKOLChar;
  dwLogonType, dwLogonProvider: DWORD; var phToken: THandle): BOOL; stdcall;
function LookupAccountName(lpSystemName, lpAccountName: PKOLChar;
  Sid: PSID; var cbSid: DWORD; ReferencedDomainName: PKOLChar;
  var cbReferencedDomainName: DWORD; var peUse: SID_NAME_USE): BOOL; stdcall;
function LookupAccountSid(lpSystemName: PKOLChar; Sid: PSID;
  Name: PKOLChar; var cbName: DWORD; ReferencedDomainName: PKOLChar;
  var cbReferencedDomainName: DWORD; var peUse: SID_NAME_USE): BOOL; stdcall;
function LookupPrivilegeDisplayName(lpSystemName, lpName: PKOLChar;
  lpDisplayName: PKOLChar; var cbDisplayName, lpLanguageId: DWORD): BOOL; stdcall;
function LookupPrivilegeName(lpSystemName: PKOLChar;
  var lpLuid: TLargeInteger; lpName: PKOLChar; var cbName: DWORD): BOOL; stdcall;
function LookupPrivilegeValue(lpSystemName, lpName: PKOLChar;
  var lpLuid: TLargeInteger): BOOL; stdcall;
function ObjectCloseAuditAlarm(SubsystemName: PKOLChar;
  HandleId: Pointer; GenerateOnClose: BOOL): BOOL; stdcall;
function ObjectDeleteAuditAlarm(SubsystemName: PKOLChar;
  HandleId: Pointer; GenerateOnClose: BOOL): BOOL; stdcall;
function ObjectOpenAuditAlarm(SubsystemName: PKOLChar; HandleId: Pointer;
  ObjectTypeName: PKOLChar; ObjectName: PKOLChar; pSecurityDescriptor: PSecurityDescriptor;
  ClientToken: THandle; DesiredAccess, GrantedAccess: DWORD;
  var Privileges: TPrivilegeSet; ObjectCreation, AccessGranted: BOOL;
  var GenerateOnClose: BOOL): BOOL; stdcall;
function ObjectPrivilegeAuditAlarm(SubsystemName: PKOLChar;
  HandleId: Pointer; ClientToken: THandle; DesiredAccess: DWORD;
  var Privileges: TPrivilegeSet; AccessGranted: BOOL): BOOL; stdcall;
function OpenBackupEventLog(lpUNCServerName, lpFileName: PKOLChar): THandle; stdcall;
function OpenEventLog(lpUNCServerName, lpSourceName: PKOLChar): THandle; stdcall;
function PrivilegedServiceAuditAlarm(SubsystemName, ServiceName: PKOLChar;
  ClientToken: THandle; var Privileges: TPrivilegeSet; AccessGranted: BOOL): BOOL; stdcall;
function ReadEventLog(hEventLog: THandle; dwReadFlags, dwRecordOffset: DWORD;
  lpBuffer: Pointer; nNumberOfBytesToRead: DWORD;
  var pnBytesRead, pnMinNumberOfBytesNeeded: DWORD): BOOL; stdcall;
function RegConnectRegistry(lpMachineName: PKOLChar; hKey: HKEY;
  var phkResult: HKEY): Longint; stdcall;
function RegCreateKey(hKey: HKEY; lpSubKey: PKOLChar;
  var phkResult: HKEY): Longint; stdcall;
function RegCreateKeyEx(hKey: HKEY; lpSubKey: PKOLChar;
  Reserved: DWORD; lpClass: PKOLChar; dwOptions: DWORD; samDesired: REGSAM;
  lpSecurityAttributes: PSecurityAttributes; var phkResult: HKEY;
  lpdwDisposition: PDWORD): Longint; stdcall;
function RegDeleteKey(hKey: HKEY; lpSubKey: PKOLChar): Longint; stdcall;
function RegDeleteValue(hKey: HKEY; lpValueName: PKOLChar): Longint; stdcall;
function RegEnumKeyEx(hKey: HKEY; dwIndex: DWORD; lpName: PKOLChar;
  var lpcbName: DWORD; lpReserved: Pointer; lpClass: PKOLChar;
  lpcbClass: PDWORD; lpftLastWriteTime: PFileTime): Longint; stdcall;
function RegEnumKey(hKey: HKEY; dwIndex: DWORD; lpName: PKOLChar; cbName: DWORD): Longint; stdcall;
function RegEnumValue(hKey: HKEY; dwIndex: DWORD; lpValueName: PKOLChar;
  var lpcbValueName: DWORD; lpReserved: Pointer; lpType: PDWORD;
  lpData: PByte; lpcbData: PDWORD): Longint; stdcall;
function RegLoadKey(hKey: HKEY; lpSubKey, lpFile: PKOLChar): Longint; stdcall;
function RegOpenKey(hKey: HKEY; lpSubKey: PKOLChar; var phkResult: HKEY): Longint; stdcall;
function RegOpenKeyEx(hKey: HKEY; lpSubKey: PKOLChar;
  ulOptions: DWORD; samDesired: REGSAM; var phkResult: HKEY): Longint; stdcall;
function RegQueryInfoKey(hKey: HKEY; lpClass: PKOLChar;
  lpcbClass: PDWORD; lpReserved: Pointer;
  lpcSubKeys, lpcbMaxSubKeyLen, lpcbMaxClassLen, lpcValues,
  lpcbMaxValueNameLen, lpcbMaxValueLen, lpcbSecurityDescriptor: PDWORD;
  lpftLastWriteTime: PFileTime): Longint; stdcall;
function RegQueryMultipleValues(hKey: HKEY; var ValList;
  NumVals: DWORD; lpValueBuf: PKOLChar; var ldwTotsize: DWORD): Longint; stdcall;
function RegQueryValue(hKey: HKEY; lpSubKey: PKOLChar;
  lpValue: PKOLChar; var lpcbValue: Longint): Longint; stdcall;
function RegQueryValueEx(hKey: HKEY; lpValueName: PKOLChar;
  lpReserved: Pointer; lpType: PDWORD; lpData: PByte; lpcbData: PDWORD): Longint; stdcall;
function RegReplaceKey(hKey: HKEY; lpSubKey: PKOLChar;
   lpNewFile: PKOLChar; lpOldFile: PKOLChar): Longint; stdcall;
function RegRestoreKey(hKey: HKEY; lpFile: PKOLChar; dwFlags: DWORD): Longint; stdcall;
function RegSaveKey(hKey: HKEY; lpFile: PKOLChar;
  lpSecurityAttributes: PSecurityAttributes): Longint; stdcall;
function RegSetValue(hKey: HKEY; lpSubKey: PKOLChar;
  dwType: DWORD; lpData: PKOLChar; cbData: DWORD): Longint; stdcall;
function RegSetValueEx(hKey: HKEY; lpValueName: PKOLChar;
  Reserved: DWORD; dwType: DWORD; lpData: Pointer; cbData: DWORD): Longint; stdcall;
function RegUnLoadKey(hKey: HKEY; lpSubKey: PKOLChar): Longint; stdcall;
function RegisterEventSource(lpUNCServerName, lpSourceName: PKOLChar): THandle; stdcall;
function ReportEvent(hEventLog: THandle; wType, wCategory: Word;
  dwEventID: DWORD; lpUserSid: Pointer; wNumStrings: Word;
  dwDataSize: DWORD; lpStrings, lpRawData: Pointer): BOOL; stdcall;
function SetFileSecurity(lpFileName: PKOLChar; SecurityInformation: SECURITY_INFORMATION;
  pSecurityDescriptor: PSecurityDescriptor): BOOL; stdcall;
function AddAtom(lpString: PKOLChar): ATOM; stdcall;
function BeginUpdateResource(pFileName: PKOLChar; bDeleteExistingResources: BOOL): THandle; stdcall;
function BuildCommDCB(lpDef: PKOLChar; var lpDCB: TDCB): BOOL; stdcall;
function BuildCommDCBAndTimeouts(lpDef: PKOLChar; var lpDCB: TDCB;
  var lpCommTimeouts: TCommTimeouts): BOOL; stdcall;
function CallNamedPipe(lpNamedPipeName: PKOLChar; lpInBuffer: Pointer;
  nInBufferSize: DWORD; lpOutBuffer: Pointer; nOutBufferSize: DWORD;
  var lpBytesRead: DWORD; nTimeOut: DWORD): BOOL; stdcall;
function CommConfigDialog(lpszName: PKOLChar; hWnd: HWND; var lpCC: TCommConfig): BOOL; stdcall;
function CompareString(Locale: LCID; dwCmpFlags: DWORD; lpString1: PKOLChar;
  cchCount1: Integer; lpString2: PKOLChar; cchCount2: Integer): Integer; stdcall;
function CopyFile(lpExistingFileName, lpNewFileName: PKOLChar; bFailIfExists: BOOL): BOOL; stdcall;
{$IFDEF _D3orHigher}
function CopyFileEx(lpExistingFileName, lpNewFileName: PKOLChar;
  lpProgressRoutine: TFNProgressRoutine; lpData: Pointer; pbCancel: PBool;
  dwCopyFlags: DWORD): BOOL; stdcall;
{$ENDIF _D3orHigher}
function CreateDirectory(lpPathName: PKOLChar;
  lpSecurityAttributes: PSecurityAttributes): BOOL; stdcall;
function CreateDirectoryEx(lpTemplateDirectory, lpNewDirectory: PKOLChar;
  lpSecurityAttributes: PSecurityAttributes): BOOL; stdcall;
function CreateEvent(lpEventAttributes: PSecurityAttributes;
  bManualReset, bInitialState: BOOL; lpName: PKOLChar): THandle; stdcall;
function CreateFile(lpFileName: PKOLChar; dwDesiredAccess, dwShareMode: DWORD;
  lpSecurityAttributes: PSecurityAttributes; dwCreationDisposition, dwFlagsAndAttributes: DWORD;
  hTemplateFile: THandle): THandle; stdcall;
function CreateFileMapping(hFile: THandle; lpFileMappingAttributes: PSecurityAttributes;
  flProtect, dwMaximumSizeHigh, dwMaximumSizeLow: DWORD; lpName: PKOLChar): THandle; stdcall;
function CreateHardLink(lpFileName, lpExistingFileName: PKOLChar;
  lpSecurityAttributes: PSecurityAttributes): BOOL; stdcall;
function CreateMailslot(lpName: PKOLChar; nMaxMessageSize: DWORD;
  lReadTimeout: DWORD; lpSecurityAttributes: PSecurityAttributes): THandle; stdcall;
function CreateNamedPipe(lpName: PKOLChar;
  dwOpenMode, dwPipeMode, nMaxInstances, nOutBufferSize, nInBufferSize, nDefaultTimeOut: DWORD;
  lpSecurityAttributes: PSecurityAttributes): THandle; stdcall;
function CreateProcess(lpApplicationName: PKOLChar; lpCommandLine: PKOLChar;
  lpProcessAttributes, lpThreadAttributes: PSecurityAttributes;
  bInheritHandles: BOOL; dwCreationFlags: DWORD; lpEnvironment: Pointer;
  lpCurrentDirectory: PKOLChar; const lpStartupInfo: TStartupInfo;
  var lpProcessInformation: TProcessInformation): BOOL; stdcall;
function CreateSemaphore(lpSemaphoreAttributes: PSecurityAttributes;
  lInitialCount, lMaximumCount: Longint; lpName: PKOLChar): THandle; stdcall;
function CreateWaitableTimer(lpTimerAttributes: PSecurityAttributes; bManualReset: BOOL; lpTimerName: PKOLChar): THandle; stdcall;
function DefineDosDevice(dwFlags: DWORD; lpDeviceName, lpTargetPath: PKOLChar): BOOL; stdcall;
function DeleteFile(lpFileName: PKOLChar): BOOL; stdcall;
function EndUpdateResource(hUpdate: THandle; fDiscard: BOOL): BOOL; stdcall;
function EnumCalendarInfo(lpCalInfoEnumProc: TFNCalInfoEnumProc; Locale: LCID;
  Calendar: CALID; CalType: CALTYPE): BOOL; stdcall;
function EnumDateFormats(lpDateFmtEnumProc: TFNDateFmtEnumProc;
  Locale: LCID; dwFlags: DWORD): BOOL; stdcall;
function EnumResourceLanguages(hModule: HMODULE; lpType, lpName: PKOLChar;
  lpEnumFunc: ENUMRESLANGPROC; lParam: Longint): BOOL; stdcall;
function EnumResourceNames(hModule: HMODULE; lpType: PKOLChar;
  lpEnumFunc: ENUMRESNAMEPROC; lParam: Longint): BOOL; stdcall;
function EnumResourceTypes(hModule: HMODULE; lpEnumFunc: ENUMRESTYPEPROC;
  lParam: Longint): BOOL; stdcall;
function EnumSystemCodePages(lpCodePageEnumProc: TFNCodepageEnumProc; dwFlags: DWORD): BOOL; stdcall;
function EnumSystemLocales(lpLocaleEnumProc: TFNLocaleEnumProc; dwFlags: DWORD): BOOL; stdcall;
function EnumTimeFormats(lpTimeFmtEnumProc: TFNTimeFmtEnumProc;
  Locale: LCID; dwFlags: DWORD): BOOL; stdcall;
function ExpandEnvironmentStrings(lpSrc: PKOLChar; lpDst: PKOLChar; nSize: DWORD): DWORD; stdcall;
procedure FatalAppExit(uAction: UINT; lpMessageText: PKOLChar); stdcall;
function FillConsoleOutputCharacter(hConsoleOutput: THandle; cCharacter: KOLChar;
  nLength: DWORD; dwWriteCoord: TCoord; var lpNumberOfCharsWritten: DWORD): BOOL; stdcall;
function FindAtom(lpString: PKOLChar): ATOM; stdcall;
function FindFirstChangeNotification(lpPathName: PKOLChar;
  bWatchSubtree: BOOL; dwNotifyFilter: DWORD): THandle; stdcall;
function FindFirstFile(lpFileName: PKOLChar; var lpFindFileData: TWIN32FindData): THandle; stdcall;
{$IFDEF _D3orHigher}
function FindFirstFileEx(lpFileName: PKOLChar; fInfoLevelId: TFindexInfoLevels;
  lpFindFileData: Pointer; fSearchOp: TFindexSearchOps; lpSearchFilter: Pointer;
  dwAdditionalFlags: DWORD): BOOL; stdcall;
{$ENDIF _D3orHigher}
function FindNextFile(hFindFile: THandle; var lpFindFileData: TWIN32FindData): BOOL; stdcall;
function FindResource(hModule: HMODULE; lpName, lpType: PKOLChar): HRSRC; stdcall;
function FindResourceEx(hModule: HMODULE; lpType, lpName: PKOLChar; wLanguage: Word): HRSRC; stdcall;
function FoldString(dwMapFlags: DWORD; lpSrcStr: PKOLChar; cchSrc: Integer;
  lpDestStr: PKOLChar; cchDest: Integer): Integer; stdcall;
function FormatMessage(dwFlags: DWORD; lpSource: Pointer; dwMessageId: DWORD; dwLanguageId: DWORD;
  lpBuffer: PKOLChar; nSize: DWORD; Arguments: Pointer): DWORD; stdcall;
function FreeEnvironmentStrings(EnvBlock: PKOLChar): BOOL; stdcall;
function GetAtomName(nAtom: ATOM; lpBuffer: PKOLChar; nSize: Integer): UINT; stdcall;
function GetBinaryType(lpApplicationName: PKOLChar; var lpBinaryType: DWORD): BOOL; stdcall;
function GetCommandLine: PKOLChar; stdcall;
function GetCompressedFileSize(lpFileName: PKOLChar; lpFileSizeHigh: PDWORD): DWORD; stdcall;
function GetComputerName(lpBuffer: PKOLChar; var nSize: DWORD): BOOL; stdcall;
function GetConsoleTitle(lpConsoleTitle: PKOLChar; nSize: DWORD): DWORD; stdcall;
function GetCurrencyFormat(Locale: LCID; dwFlags: DWORD; lpValue: PKOLChar;
  lpFormat: PCurrencyFmt; lpCurrencyStr: PKOLChar; cchCurrency: Integer): Integer; stdcall;
function GetCurrentDirectory(nBufferLength: DWORD; lpBuffer: PKOLChar): DWORD; stdcall;
function GetDateFormat(Locale: LCID; dwFlags: DWORD; lpDate: PSystemTime;
  lpFormat: PKOLChar; lpDateStr: PKOLChar; cchDate: Integer): Integer; stdcall;
function GetDefaultCommConfig(lpszName: PKOLChar;
  var lpCC: TCommConfig; var lpdwSize: DWORD): BOOL; stdcall;
function GetDiskFreeSpace(lpRootPathName: PKOLChar;
  var lpSectorsPerCluster, lpBytesPerSector, lpNumberOfFreeClusters, lpTotalNumberOfClusters: DWORD): BOOL; stdcall;
function GetDiskFreeSpaceEx(lpDirectoryName: PKOLChar;
  var lpFreeBytesAvailableToCaller, lpTotalNumberOfBytes; lpTotalNumberOfFreeBytes: PLargeInteger): BOOL; stdcall;
function GetDriveType(lpRootPathName: PKOLChar): UINT; stdcall;
function GetEnvironmentStrings: PKOLChar; stdcall;
function GetEnvironmentVariable(lpName: PKOLChar; lpBuffer: PKOLChar; nSize: DWORD): DWORD; stdcall; //overload;
function GetFileAttributes(lpFileName: PKOLChar): DWORD; stdcall;
{$IFDEF _D3orHigher}
function GetFileAttributesEx(lpFileName: PKOLChar;
  fInfoLevelId: TGetFileExInfoLevels; lpFileInformation: Pointer): BOOL; stdcall;
{$ENDIF _D3orHigher}
function GetFullPathName(lpFileName: PKOLChar; nBufferLength: DWORD;
  lpBuffer: PKOLChar; var lpFilePart: PKOLChar): DWORD; stdcall;
function GetLocaleInfo(Locale: LCID; LCType: LCTYPE; lpLCData: PKOLChar; cchData: Integer): Integer; stdcall;
function GetLogicalDriveStrings(nBufferLength: DWORD; lpBuffer: PKOLChar): DWORD; stdcall;
function GetModuleFileName(hModule: HINST; lpFilename: PKOLChar; nSize: DWORD): DWORD; stdcall;
function GetModuleHandle(lpModuleName: PKOLChar): HMODULE; stdcall;
function GetNamedPipeHandleState(hNamedPipe: THandle;
  lpState, lpCurInstances, lpMaxCollectionCount, lpCollectDataTimeout: PDWORD;
  lpUserName: PKOLChar; nMaxUserNameSize: DWORD): BOOL; stdcall;
function GetNumberFormat(Locale: LCID; dwFlags: DWORD; lpValue: PKOLChar;
  lpFormat: PNumberFmt; lpNumberStr: PKOLChar; cchNumber: Integer): Integer; stdcall;
function GetPrivateProfileInt(lpAppName, lpKeyName: PKOLChar; nDefault: Integer; lpFileName: PKOLChar): UINT; stdcall;
function GetPrivateProfileSection(lpAppName: PKOLChar; lpReturnedString: PKOLChar; nSize: DWORD; lpFileName: PKOLChar): DWORD; stdcall;
function GetPrivateProfileSectionNames(lpszReturnBuffer: PKOLChar; nSize: DWORD; lpFileName: PKOLChar): DWORD; stdcall;
function GetPrivateProfileString(lpAppName, lpKeyName, lpDefault: PKOLChar;
  lpReturnedString: PKOLChar; nSize: DWORD; lpFileName: PKOLChar): DWORD; stdcall;
function GetProfileInt(lpAppName, lpKeyName: PKOLChar; nDefault: Integer): UINT; stdcall;
function GetProfileSection(lpAppName: PKOLChar; lpReturnedString: PKOLChar; nSize: DWORD): DWORD; stdcall;
function GetProfileString(lpAppName, lpKeyName, lpDefault: PKOLChar;
  lpReturnedString: PKOLChar; nSize: DWORD): DWORD; stdcall;
function GetShortPathName(lpszLongPath: PKOLChar; lpszShortPath: PKOLChar;
  cchBuffer: DWORD): DWORD; stdcall;
procedure GetStartupInfo(var lpStartupInfo: TStartupInfo); stdcall;
function GetStringTypeEx(Locale: LCID; dwInfoType: DWORD;
  lpSrcStr: PKOLChar; cchSrc: Integer; var lpCharType): BOOL; stdcall;
function GetSystemDirectory(lpBuffer: PKOLChar; uSize: UINT): UINT; stdcall;
function GetTempFileName(lpPathName, lpPrefixString: PKOLChar;
  uUnique: UINT; lpTempFileName: PKOLChar): UINT; stdcall;
function GetTempPath(nBufferLength: DWORD; lpBuffer: PKOLChar): DWORD; stdcall;
function GetTimeFormat(Locale: LCID; dwFlags: DWORD; lpTime: PSystemTime;
  lpFormat: PKOLChar; lpTimeStr: PKOLChar; cchTime: Integer): Integer; stdcall;
function GetVersionEx(var lpVersionInformation: TOSVersionInfo): BOOL; stdcall;
function GetVolumeInformation(lpRootPathName: PKOLChar;
  lpVolumeNameBuffer: PKOLChar; nVolumeNameSize: DWORD; lpVolumeSerialNumber: PDWORD;
  var lpMaximumComponentLength, lpFileSystemFlags: DWORD;
  lpFileSystemNameBuffer: PKOLChar; nFileSystemNameSize: DWORD): BOOL; stdcall;
function GetWindowsDirectory(lpBuffer: PKOLChar; uSize: UINT): UINT; stdcall;
function GlobalAddAtom(lpString: PKOLChar): ATOM; stdcall;
function GlobalFindAtom(lpString: PKOLChar): ATOM; stdcall;
function GlobalGetAtomName(nAtom: ATOM; lpBuffer: PKOLChar; nSize: Integer): UINT; stdcall;
function IsBadStringPtr(lpsz: PKOLChar; ucchMax: UINT): BOOL; stdcall;
function LCMapString(Locale: LCID; dwMapFlags: DWORD; lpSrcStr: PKOLChar;
  cchSrc: Integer; lpDestStr: PKOLChar; cchDest: Integer): Integer; stdcall;
function LoadLibrary(lpLibFileName: PKOLChar): HMODULE; stdcall;
function LoadLibraryEx(lpLibFileName: PKOLChar; hFile: THandle; dwFlags: DWORD): HMODULE; stdcall;
function MoveFile(lpExistingFileName, lpNewFileName: PKOLChar): BOOL; stdcall;
function MoveFileEx(lpExistingFileName, lpNewFileName: PKOLChar; dwFlags: DWORD): BOOL; stdcall;
{$IFDEF _D3orHigher}
function MoveFileWithProgress(lpExistingFileName, lpNewFileName: PKOLChar; lpProgressRoutine: TFNProgressRoutine;
  lpData: Pointer; dwFlags: DWORD): BOOL; stdcall;
{$ENDIF _D3orHigher}
function OpenEvent(dwDesiredAccess: DWORD; bInheritHandle: BOOL; lpName: PKOLChar): THandle; stdcall;
function OpenFileMapping(dwDesiredAccess: DWORD; bInheritHandle: BOOL; lpName: PKOLChar): THandle; stdcall;
function OpenMutex(dwDesiredAccess: DWORD; bInheritHandle: BOOL; lpName: PKOLChar): THandle; stdcall;
function OpenSemaphore(dwDesiredAccess: DWORD; bInheritHandle: BOOL; lpName: PKOLChar): THandle; stdcall;
function OpenWaitableTimer(dwDesiredAccess: DWORD; bInheritHandle: BOOL;
  lpTimerName: PKOLChar): THandle; stdcall;
procedure OutputDebugString(lpOutputString: PKOLChar); stdcall;
function PeekConsoleInput(hConsoleInput: THandle; var lpBuffer: TInputRecord;
  nLength: DWORD; var lpNumberOfEventsRead: DWORD): BOOL; stdcall;
function QueryDosDevice(lpDeviceName: PKOLChar; lpTargetPath: PKOLChar; ucchMax: DWORD): DWORD; stdcall;
function QueryRecoveryAgents(p1: PKOLChar; var p2: Pointer; var p3: TRecoveryAgentInformation): DWORD; stdcall;
function ReadConsole(hConsoleInput: THandle; lpBuffer: Pointer;
  nNumberOfCharsToRead: DWORD; var lpNumberOfCharsRead: DWORD; lpReserved: Pointer): BOOL; stdcall;
function ReadConsoleInput(hConsoleInput: THandle; var lpBuffer: TInputRecord;
  nLength: DWORD; var lpNumberOfEventsRead: DWORD): BOOL; stdcall;
function ReadConsoleOutput(hConsoleOutput: THandle; lpBuffer: Pointer;
  dwBufferSize, dwBufferCoord: TCoord; var lpReadRegion: TSmallRect): BOOL; stdcall;
function ReadConsoleOutputCharacter(hConsoleOutput: THandle; lpCharacter: PKOLChar;
  nLength: DWORD; dwReadCoord: TCoord; var lpNumberOfCharsRead: DWORD): BOOL; stdcall;
function RemoveDirectory(lpPathName: PKOLChar): BOOL; stdcall;
function ScrollConsoleScreenBuffer(hConsoleOutput: THandle;
  const lpScrollRectangle: TSmallRect; lpClipRectangle: PSmallRect;
  dwDestinationOrigin: TCoord; var lpFill: TCharInfo): BOOL; stdcall;
function SearchPath(lpPath, lpFileName, lpExtension: PKOLChar;
  nBufferLength: DWORD; lpBuffer: PKOLChar; var lpFilePart: PKOLChar): DWORD; stdcall;
function SetComputerName(lpComputerName: PKOLChar): BOOL; stdcall;
function SetConsoleTitle(lpConsoleTitle: PKOLChar): BOOL; stdcall;
function SetCurrentDirectory(lpPathName: PKOLChar): BOOL; stdcall;
function SetDefaultCommConfig(lpszName: PKOLChar; lpCC: PCommConfig; dwSize: DWORD): BOOL; stdcall;
function SetEnvironmentVariable(lpName, lpValue: PKOLChar): BOOL; stdcall;
function SetFileAttributes(lpFileName: PKOLChar; dwFileAttributes: DWORD): BOOL; stdcall;
function SetLocaleInfo(Locale: LCID; LCType: LCTYPE; lpLCData: PKOLChar): BOOL; stdcall;
function SetVolumeLabel(lpRootPathName: PKOLChar; lpVolumeName: PKOLChar): BOOL; stdcall;
function UpdateResource(hUpdate: THandle; lpType, lpName: PKOLChar;
  wLanguage: Word; lpData: Pointer; cbData: DWORD): BOOL; stdcall;
function VerLanguageName(wLang: DWORD; szLang: PKOLChar; nSize: DWORD): DWORD; stdcall;
function WaitNamedPipe(lpNamedPipeName: PKOLChar; nTimeOut: DWORD): BOOL; stdcall;
function WriteConsole(hConsoleOutput: THandle; const lpBuffer: Pointer;
  nNumberOfCharsToWrite: DWORD; var lpNumberOfCharsWritten: DWORD; lpReserved: Pointer): BOOL; stdcall;
function WriteConsoleInput(hConsoleInput: THandle; const lpBuffer: TInputRecord;
  nLength: DWORD; var lpNumberOfEventsWritten: DWORD): BOOL; stdcall;
function WriteConsoleOutput(hConsoleOutput: THandle; lpBuffer: Pointer;
  dwBufferSize, dwBufferCoord: TCoord; var lpWriteRegion: TSmallRect): BOOL; stdcall;
function WriteConsoleOutputCharacter(hConsoleOutput: THandle;lpCharacter: PKOLChar;
  nLength: DWORD; dwWriteCoord: TCoord; var lpNumberOfCharsWritten: DWORD): BOOL; stdcall;
function WritePrivateProfileSection(lpAppName, lpString, lpFileName: PKOLChar): BOOL; stdcall;
function WritePrivateProfileString(lpAppName, lpKeyName, lpString, lpFileName: PKOLChar): BOOL; stdcall;
function WriteProfileSection(lpAppName, lpString: PKOLChar): BOOL; stdcall;
function WriteProfileString(lpAppName, lpKeyName, lpString: PKOLChar): BOOL; stdcall;
function lstrcat(lpString1, lpString2: PKOLChar): PKOLChar; stdcall;
function lstrcmp(lpString1, lpString2: PKOLChar): Integer; stdcall;
function lstrcmpi(lpString1, lpString2: PKOLChar): Integer; stdcall;
function lstrcpy(lpString1, lpString2: PKOLChar): PKOLChar; stdcall;
function lstrcpyn(lpString1, lpString2: PKOLChar; iMaxLength: Integer): PKOLChar; stdcall;
function lstrlen(lpString: PKOLChar): Integer; stdcall;
function MultinetGetConnectionPerformance(lpNetResource: PNetResource;
  lpNetConnectInfoStruc: PNetConnectInfoStruct): DWORD; stdcall;
function WNetAddConnection2(var lpNetResource: TNetResource;
  lpPassword, lpUserName: PKOLChar; dwFlags: DWORD): DWORD; stdcall;
function WNetAddConnection3(hwndOwner: HWND; var lpNetResource: TNetResource;
  lpPassword, lpUserName: PKOLChar; dwFlags: DWORD): DWORD; stdcall;
function WNetAddConnection(lpRemoteName, lpPassword, lpLocalName: PKOLChar): DWORD; stdcall;
function WNetCancelConnection2(lpName: PKOLChar; dwFlags: DWORD; fForce: BOOL): DWORD; stdcall;
function WNetCancelConnection(lpName: PKOLChar; fForce: BOOL): DWORD; stdcall;
function WNetConnectionDialog1(var lpConnDlgStruct: TConnectDlgStruct): DWORD; stdcall;
function WNetDisconnectDialog1(var lpConnDlgStruct: TDiscDlgStruct): DWORD; stdcall;
function WNetEnumResource(hEnum: THandle; var lpcCount: DWORD;
  lpBuffer: Pointer; var lpBufferSize: DWORD): DWORD; stdcall;
function WNetGetConnection(lpLocalName: PKOLChar;
  lpRemoteName: PKOLChar; var lpnLength: DWORD): DWORD; stdcall;
function WNetGetLastError(var lpError: DWORD; lpErrorBuf: PKOLChar;
  nErrorBufSize: DWORD; lpNameBuf: PKOLChar; nNameBufSize: DWORD): DWORD; stdcall;
function WNetGetNetworkInformation(lpProvider: PKOLChar;
  var lpNetInfoStruct: TNetInfoStruct): DWORD; stdcall;
function WNetGetProviderName(dwNetType: DWORD; lpProviderName: PKOLChar;
  var lpBufferSize: DWORD): DWORD; stdcall;
function WNetGetResourceParent(lpNetResource: PNetResource;
  lpBuffer: Pointer; var cbBuffer: DWORD): DWORD; stdcall;
function WNetGetUniversalName(lpLocalPath: PKOLChar; dwInfoLevel: DWORD;
  lpBuffer: Pointer; var lpBufferSize: DWORD): DWORD; stdcall;
function WNetGetUser(lpName: PKOLChar; lpUserName: PKOLChar; var lpnLength: DWORD): DWORD; stdcall;
function WNetOpenEnum(dwScope, dwType, dwUsage: DWORD;
  lpNetResource: PNetResource; var lphEnum: THandle): DWORD; stdcall;
function WNetSetConnection(lpName: PKOLChar; dwProperties: DWORD; pvValues: Pointer): DWORD; stdcall;
function WNetUseConnection(hwndOwner: HWND;
  var lpNetResource: TNetResource; lpUserID: PKOLChar;
  lpPassword: PKOLChar; dwFlags: DWORD; lpAccessName: PKOLChar;
  var lpBufferSize: DWORD; var lpResult: DWORD): DWORD; stdcall;
function GetFileVersionInfo(lptstrFilename: PKOLChar; dwHandle, dwLen: DWORD;
  lpData: Pointer): BOOL; stdcall;
function GetFileVersionInfoSize(lptstrFilename: PKOLChar; var lpdwHandle: DWORD): DWORD; stdcall;
function VerFindFile(uFlags: DWORD; szFileName, szWinDir, szAppDir, szCurDir: PKOLChar;
  var lpuCurDirLen: UINT; szDestDir: PKOLChar; var lpuDestDirLen: UINT): DWORD; stdcall;
function VerInstallFile(uFlags: DWORD;
  szSrcFileName, szDestFileName, szSrcDir, szDestDir, szCurDir, szTmpFile: PKOLChar;
  var lpuTmpFileLen: UINT): DWORD; stdcall;
function VerQueryValue(pBlock: Pointer; lpSubBlock: PKOLChar;
  var lplpBuffer: Pointer; var puLen: UINT): BOOL; stdcall;
function GetPrivateProfileStruct(lpszSection, lpszKey: PKOLChar;
  lpStruct: Pointer; uSizeStruct: UINT; szFile: PKOLChar): BOOL; stdcall;
function WritePrivateProfileStruct(lpszSection, lpszKey: PKOLChar;
  lpStruct: Pointer; uSizeStruct: UINT; szFile: PKOLChar): BOOL; stdcall;
function AddFontResource(FileName: PKOLChar): Integer; stdcall;
function AddFontResourceEx(p1: PKOLChar; p2: DWORD; p3: PDesignVector): Integer; stdcall;
function CopyEnhMetaFile(p1: HENHMETAFILE; p2: PKOLChar): HENHMETAFILE; stdcall;
function CopyMetaFile(p1: HMETAFILE; p2: PKOLChar): HMETAFILE; stdcall;
function CreateColorSpace(var ColorSpace: TLogColorSpace): HCOLORSPACE; stdcall;
function CreateDC(lpszDriver, lpszDevice, lpszOutput: PKOLChar;
  lpdvmInit: PDeviceMode): HDC; stdcall;
function CreateEnhMetaFile(DC: HDC; FileName: PKOLChar; Rect: PRect; Desc: PKOLChar): HDC; stdcall;
function CreateFont(nHeight, nWidth, nEscapement, nOrientaion, fnWeight: Integer;
  fdwItalic, fdwUnderline, fdwStrikeOut, fdwCharSet, fdwOutputPrecision,
  fdwClipPrecision, fdwQuality, fdwPitchAndFamily: DWORD; lpszFace: PKOLChar): HFONT; stdcall;
function CreateFontIndirect(const p1: TLogFont): HFONT; stdcall;
function CreateFontIndirectEx(const p1: PEnumLogFontExDV): HFONT; stdcall;
function CreateIC(lpszDriver, lpszDevice, lpszOutput: PKOLChar; lpdvmInit: PDeviceMode): HDC; stdcall;
function CreateMetaFile(p1: PKOLChar): HDC; stdcall;
function CreateScalableFontResource(p1: DWORD; p2, p3, p4: PKOLChar): BOOL; stdcall;
function DeviceCapabilities(pDriverName, pDeviceName, pPort: PKOLChar;
  iIndex: Integer; pOutput: PKOLChar; DevMode: PDeviceMode): Integer; stdcall;
function EnumFontFamilies(DC: HDC; p2: PKOLChar; p3: TFNFontEnumProc; p4: LPARAM): BOOL; stdcall;
function EnumFontFamiliesEx(DC: HDC; var p2: TLogFont;
  p3: TFNFontEnumProc; p4: LPARAM; p5: DWORD): BOOL; stdcall;
function EnumFonts(DC: HDC; lpszFace: PKOLChar; fntenmprc: TFNFontEnumProc;
  lpszData: PKOLChar): Integer; stdcall;
function EnumICMProfiles(DC: HDC; ICMProc: TFNICMEnumProc; p3: LPARAM): Integer; stdcall;
function ExtTextOut(DC: HDC; X, Y: Integer; Options: Longint;
  Rect: PRect; Str: PKOLChar; Count: Longint; Dx: PInteger): BOOL; stdcall;
function GetCharABCWidths(DC: HDC; FirstChar, LastChar: UINT; const ABCStructs): BOOL; stdcall;
function GetCharABCWidthsFloat(DC: HDC; FirstChar, LastChar: UINT; const ABCFloatSturcts): BOOL; stdcall;
function GetCharWidth32(DC: HDC; FirstChar, LastChar: UINT; const Widths): BOOL; stdcall;
function GetCharWidth(DC: HDC; FirstChar, LastChar: UINT; const Widths): BOOL; stdcall;
function GetCharWidthFloat(DC: HDC; FirstChar, LastChar: UINT; const Widths): BOOL; stdcall;
function GetCharacterPlacement(DC: HDC; p2: PKOLChar; p3, p4: BOOL;
  var p5: TGCPResults; p6: DWORD): DWORD; stdcall;
function GetEnhMetaFile(p1: PKOLChar): HENHMETAFILE; stdcall;
function GetEnhMetaFileDescription(p1: HENHMETAFILE; p2: UINT; p3: PKOLChar): UINT; stdcall;
function GetGlyphIndices(DC: HDC; p2: PKOLChar; p3: Integer; p4: PWORD; p5: DWORD): DWORD; stdcall;
function GetGlyphOutline(DC: HDC; uChar, uFormat: UINT;
  const lpgm: TGlyphMetrics; cbBuffer: DWORD; lpvBuffer: Pointer; const lpmat2: TMat2): DWORD; stdcall;
function GetICMProfile(DC: HDC; var Size: DWORD; Name: PKOLChar): BOOL; stdcall;
function GetLogColorSpace(p1: HCOLORSPACE; var ColorSpace: TLogColorSpace; Size: DWORD): BOOL; stdcall;
function GetMetaFile(p1: PKOLChar): HMETAFILE; stdcall;
function GetObject(p1: HGDIOBJ; p2: Integer; p3: Pointer): Integer; stdcall;
function GetOutlineTextMetrics(DC: HDC; p2: UINT; OTMetricStructs: Pointer): UINT; stdcall;
function GetTextExtentExPoint(DC: HDC; p2: PKOLChar;
  p3, p4: Integer; p5, p6: PInteger; var p7: TSize): BOOL; stdcall;
function GetTextExtentPoint32(DC: HDC; Str: PKOLChar; Count: Integer;
  var Size: TSize): BOOL; stdcall;
function GetTextExtentPoint(DC: HDC; Str: PKOLChar; Count: Integer;
  var Size: TSize): BOOL; stdcall;
function GetTextFace(DC: HDC; Count: Integer; Buffer: PKOLChar): Integer; stdcall;
function GetTextMetrics(DC: HDC; var TM: TTextMetric): BOOL; stdcall;
function PolyTextOut(DC: HDC; const PolyTextArray; Strings: Integer): BOOL; stdcall;
function RemoveFontResource(FileName: PKOLChar): BOOL; stdcall;
function RemoveFontResourceEx(p1: PKOLChar; p2: DWORD; p3: PDesignVector): BOOL; stdcall;
function ResetDC(DC: HDC; const InitData: TDeviceMode): HDC; stdcall;
function SetICMProfile(DC: HDC; Name: PKOLChar): BOOL; stdcall;
function StartDoc(DC: HDC; const p2: TDocInfo): Integer; stdcall;
function TextOut(DC: HDC; X, Y: Integer; Str: PKOLChar; Count: Integer): BOOL; stdcall;
function UpdateICMRegKey(p1: DWORD; p2, p3: PKOLChar; p4: UINT): BOOL; stdcall;
function wglUseFontBitmaps(DC: HDC; p2, p3, p4: DWORD): BOOL; stdcall;
function wglUseFontOutlines(p1: HDC; p2, p3, p4: DWORD;
  p5, p6: Single; p7: Integer; p8: PGlyphMetricsFloat): BOOL; stdcall;
function AnsiToOem(const lpszSrc: LPCSTR; lpszDst: LPSTR): BOOL; stdcall;
function AnsiToOemBuff(lpszSrc: LPCSTR; lpszDst: LPSTR; cchDstLength: DWORD): BOOL; stdcall;
function AnsiUpper(lpsz: LPSTR): LPSTR; stdcall;
function AnsiUpperBuff(lpsz: LPSTR; cchLength: DWORD): DWORD; stdcall;
function AnsiLower(lpsz: LPSTR): LPSTR; stdcall;
function AnsiLowerBuff(lpsz: LPSTR; cchLength: DWORD): DWORD; stdcall;
function AnsiNext(const lpsz: LPCSTR): LPSTR; stdcall;
function AnsiPrev(const lpszStart: LPCSTR; const lpszCurrent: LPCSTR): LPSTR; stdcall;
function AppendMenu(hMenu: HMENU; uFlags, uIDNewItem: UINT;
  lpNewItem: PKOLChar): BOOL; stdcall;
//function BroadcastSystemMessage(Flags: DWORD; Recipients: PDWORD;
//  uiMessage: UINT; wParam: WPARAM; lParam: LPARAM): Longint; stdcall;
//function BroadcastSystemMessageW(Flags: DWORD; Recipients: PDWORD;
//  uiMessage: UINT; wParam: WPARAM; lParam: LPARAM): Longint; stdcall;
function CallMsgFilter(var lpMsg: TMsg; nCode: Integer): BOOL; stdcall;
function CallWindowProc(lpPrevWndFunc: TFNWndProc; hWnd: HWND; Msg: UINT; wParam: WPARAM; lParam: LPARAM): LRESULT; stdcall;
function ChangeDisplaySettings(var lpDevMode: TDeviceMode; dwFlags: DWORD): Longint; stdcall;
function ChangeDisplaySettingsEx(lpszDeviceName: PKOLChar; var lpDevMode: TDeviceMode;
        wnd: HWND; dwFlags: DWORD; lParam: Pointer): Longint; stdcall;
function ChangeMenu(hMenu: HMENU; cmd: UINT; lpszNewItem: PKOLChar;
  cmdInsert: UINT; flags: UINT): BOOL; stdcall;
function CharLower(lpsz: PKOLChar): PKOLChar; stdcall;
function CharLowerBuff(lpsz: PKOLChar; cchLength: DWORD): DWORD; stdcall;
function CharNext(lpsz: PKOLChar): PKOLChar; stdcall;
function CharNextEx(CodePage: Word; lpCurrentChar: LPCSTR; dwFlags: DWORD): LPSTR; stdcall;
function CharPrev(lpszStart: PKOLChar; lpszCurrent: PKOLChar): PKOLChar; stdcall;
function CharPrevEx(CodePage: Word; lpStart, lpCurrentChar: LPCSTR; dwFlags: DWORD): LPSTR; stdcall;
function CharToOem(lpszSrc: PKOLChar; lpszDst: PKOLChar): BOOL; stdcall;
function CharToOemBuff(lpszSrc: PKOLChar; lpszDst: PKOLChar; cchDstLength: DWORD): BOOL; stdcall;
function CharUpper(lpsz: PKOLChar): PKOLChar; stdcall;
function CharUpperBuff(lpsz: PKOLChar; cchLength: DWORD): DWORD; stdcall;
function CopyAcceleratorTable(hAccelSrc: HACCEL; var lpAccelDst; cAccelEntries: Integer): Integer; stdcall;
function CreateAcceleratorTable(var Accel; Count: Integer): HACCEL; stdcall;
function CreateDesktop(lpszDesktop, lpszDevice: PKOLChar;
  pDevmode: PDeviceMode; dwFlags: DWORD; dwDesiredAccess:
  DWORD; lpsa: PSecurityAttributes): HDESK; stdcall;
function CreateDialogIndirectParam(hInstance: HINST; const lpTemplate: TDlgTemplate;
  hWndParent: HWND; lpDialogFunc: TFNDlgProc; dwInitParam: LPARAM): HWND; stdcall;
function CreateDialogParam(hInstance: HINST; lpTemplateName: PKOLChar;
  hWndParent: HWND; lpDialogFunc: TFNDlgProc; dwInitParam: LPARAM): HWND; stdcall;
function CreateMDIWindow(lpClassName, lpWindowName: PKOLChar;
  dwStyle: DWORD; X, Y, nWidth, nHeight: Integer;
  hWndParent: HWND; hInstance: HINST; lParam: LPARAM): HWND; stdcall;
function CreateWindowEx(dwExStyle: DWORD; lpClassName: PKOLChar;
  lpWindowName: PKOLChar; dwStyle: DWORD; X, Y, nWidth, nHeight: Integer;
  hWndParent: HWND; hMenu: HMENU; hInstance: HINST; lpParam: Pointer): HWND; stdcall;
function CreateWindowStation(lpwinsta: PKOLChar; dwReserved, dwDesiredAccess: DWORD;
  lpsa: PSecurityAttributes): HWINSTA; stdcall;
function DefDlgProc(hDlg: HWND; Msg: UINT; wParam: WPARAM; lParam: LPARAM): LRESULT; stdcall;
function DefFrameProc(hWnd, hWndMDIClient: HWND; uMsg: UINT; wParam: WPARAM; lParam: LPARAM): LRESULT; stdcall;
function DefMDIChildProc(hWnd: HWND; uMsg: UINT; wParam: WPARAM; lParam: LPARAM): LRESULT; stdcall;
function DefWindowProc(hWnd: HWND; Msg: UINT; wParam: WPARAM; lParam: LPARAM): LRESULT; stdcall;
function DialogBoxIndirectParam(hInstance: HINST; const lpDialogTemplate: TDlgTemplate;
  hWndParent: HWND; lpDialogFunc: TFNDlgProc; dwInitParam: LPARAM): Integer; stdcall;
function DialogBoxParam(hInstance: HINST; lpTemplateName: PKOLChar;
  hWndParent: HWND; lpDialogFunc: TFNDlgProc; dwInitParam: LPARAM): Integer; stdcall;
function DispatchMessage(const lpMsg: TMsg): Longint; stdcall;
function DlgDirList(hDlg: HWND; lpPathSpec: PKOLChar;
  nIDListBox, nIDStaticPath: Integer; uFileType: UINT): Integer; stdcall;
function DlgDirListComboBox(hDlg: HWND; lpPathSpec: PKOLChar;
  nIDComboBox, nIDStaticPath: Integer; uFiletype: UINT): Integer; stdcall;
function DlgDirSelectComboBoxEx(hDlg: HWND; lpString: PKOLChar;
  nCount, nIDComboBox: Integer): BOOL; stdcall;
function DlgDirSelectEx(hDlg: HWND; lpString: PKOLChar; nCount, nIDListBox: Integer): BOOL; stdcall;
function DrawState(DC: HDC; Brush: HBRUSH; CBFunc: TFNDrawStateProc;
  lData: LPARAM; wData: WPARAM; x, y, cx, cy: Integer; Flags: UINT): BOOL; stdcall;
function DrawText(hDC: HDC; lpString: PKOLChar; nCount: Integer;
  var lpRect: TRect; uFormat: UINT): Integer; stdcall;
function DrawTextEx(DC: HDC; lpchText: PKOLChar; cchText: Integer; var p4: TRect;
  dwDTFormat: UINT; DTParams: PDrawTextParams): Integer; stdcall;
function EnumDesktops(hwinsta: HWINSTA; lpEnumFunc: TFNDeskTopEnumProc; lParam: LPARAM): BOOL; stdcall;
function EnumDisplaySettings(lpszDeviceName: PKOLChar; iModeNum: DWORD;
  var lpDevMode: TDeviceMode): BOOL; stdcall;
function EnumDisplayDevices(Unused: Pointer; iDevNum: DWORD;
  var lpDisplayDevice: TDisplayDevice; dwFlags: DWORD): BOOL; stdcall;
function EnumProps(hWnd: HWND; lpEnumFunc: TFNPropEnumProc): Integer; stdcall;
function EnumPropsEx(hWnd: HWND; lpEnumFunc: TFNPropEnumProcEx; lParam: LPARAM): Integer; stdcall;
function EnumWindowStations(lpEnumFunc: TFNWinStaEnumProc; lParam: LPARAM): BOOL; stdcall;
function FindWindow(lpClassName, lpWindowName: PKOLChar): HWND; stdcall;
function FindWindowEx(Parent, Child: HWND; ClassName, WindowName: PKOLChar): HWND; stdcall;
function GetAltTabInfo(hwnd: HWND; iItem: Integer; var pati: TAltTabInfo;
  pszItemText: PKOLChar; cchItemText: UINT): BOOL; stdcall;
function GetClassInfo(hInstance: HINST; lpClassName: PKOLChar;
  var lpWndClass: TWndClass): BOOL; stdcall;
function GetClassInfoEx(Instance: HINST; Classname: PKOLChar; var WndClass: TWndClassEx): BOOL; stdcall;
function GetClassLong(hWnd: HWND; nIndex: Integer): DWORD; stdcall;
function GetClassName(hWnd: HWND; lpClassName: PKOLChar; nMaxCount: Integer): Integer; stdcall;
function GetClipboardFormatName(format: UINT; lpszFormatName: PKOLChar;
  cchMaxCount: Integer): Integer; stdcall;
function GetDlgItemText(hDlg: HWND; nIDDlgItem: Integer;
  lpString: PKOLChar; nMaxCount: Integer): UINT; stdcall;
function GetKeyNameText(lParam: Longint; lpString: PKOLChar; nSize: Integer): Integer; stdcall;
function GetKeyboardLayoutName(pwszKLID: PKOLChar): BOOL; stdcall;
function GetMenuItemInfo(p1: HMENU; p2: UINT; p3: BOOL; var p4: TMenuItemInfo): BOOL; stdcall;
function GetMenuString(hMenu: HMENU; uIDItem: UINT; lpString: PKOLChar;
  nMaxCount: Integer; uFlag: UINT): Integer; stdcall;
function GetMessage(var lpMsg: TMsg; hWnd: HWND;
  wMsgFilterMin, wMsgFilterMax: UINT): BOOL; stdcall;
function GetProp(hWnd: HWND; lpString: PKOLChar): THandle; stdcall;
function GetTabbedTextExtent(hDC: HDC; lpString: PKOLChar;
  nCount, nTabPositions: Integer; var lpnTabStopPositions): DWORD; stdcall;
function GetUserObjectInformation(hObj: THandle; nIndex: Integer; pvInfo: Pointer;
  nLength: DWORD; var lpnLengthNeeded: DWORD): BOOL; stdcall;
function GetWindowLong(hWnd: HWND; nIndex: Integer): Longint; stdcall;
function GetWindowModuleFileName(hwnd: HWND; pszFileName: PKOLChar; cchFileNameMax: UINT): UINT; stdcall;
function GetWindowText(hWnd: HWND; lpString: PKOLChar; nMaxCount: Integer): Integer; stdcall;
function GetWindowTextLength(hWnd: HWND): Integer; stdcall;
function GrayString(hDC: HDC; hBrush: HBRUSH; lpOutputFunc: TFNGrayStringProc;
  lpData: LPARAM; nCount, X, Y, nWidth, nHeight: Integer): BOOL; stdcall;
function InsertMenu(hMenu: HMENU; uPosition, uFlags, uIDNewItem: UINT;
  lpNewItem: PKOLChar): BOOL; stdcall;
function InsertMenuItem(p1: HMENU; p2: UINT; p3: BOOL; const p4: TMenuItemInfo): BOOL; stdcall;
function IsCharAlpha(ch: KOLChar): BOOL; stdcall;
function IsCharAlphaNumeric(ch: KOLChar): BOOL; stdcall;
function IsCharLower(ch: KOLChar): BOOL; stdcall;
function IsCharUpper(ch: KOLChar): BOOL; stdcall;
function IsDialogMessage(hDlg: HWND; var lpMsg: TMsg): BOOL; stdcall;
function LoadAccelerators(hInstance: HINST; lpTableName: PKOLChar): HACCEL; stdcall;
function LoadBitmap(hInstance: HINST; lpBitmapName: PKOLChar): HBITMAP; stdcall;
function LoadCursor(hInstance: HINST; lpCursorName: PKOLChar): HCURSOR; stdcall;
function LoadCursorFromFile(lpFileName: PKOLChar): HCURSOR; stdcall;
function LoadIcon(hInstance: HINST; lpIconName: PKOLChar): HICON; stdcall;
function LoadImage(hInst: HINST; ImageName: PKOLChar; ImageType: UINT; X, Y: Integer; Flags: UINT): THandle; stdcall;
function LoadKeyboardLayout(pwszKLID: PKOLChar; Flags: UINT): HKL; stdcall;
function LoadMenu(hInstance: HINST; lpMenuName: PKOLChar): HMENU; stdcall;
function LoadMenuIndirect(lpMenuTemplate: Pointer): HMENU; stdcall;
function LoadString(hInstance: HINST; uID: UINT; lpBuffer: PKOLChar; nBufferMax: Integer): Integer; stdcall;
function MapVirtualKey(uCode, uMapType: UINT): UINT; stdcall;
function MapVirtualKeyEx(uCode, uMapType: UINT; dwhkl: HKL): UINT; stdcall;
function MessageBox(hWnd: HWND; lpText, lpCaption: PKOLChar; uType: UINT): Integer; stdcall;
function MessageBoxEx(hWnd: HWND; lpText, lpCaption: PKOLChar;
  uType: UINT; wLanguageId: Word): Integer; stdcall;
function MessageBoxIndirect(const MsgBoxParams: TMsgBoxParams): BOOL; stdcall;
function ModifyMenu(hMnu: HMENU; uPosition, uFlags, uIDNewItem: UINT;
  lpNewItem: PKOLChar): BOOL; stdcall;
function OemToAnsi(const lpszSrc: LPCSTR; lpszDst: LPSTR): BOOL; stdcall;
function OemToAnsiBuff(lpszSrc: LPCSTR; lpszDst: LPSTR; cchDstLength: DWORD): BOOL; stdcall;
function OemToChar(lpszSrc: PKOLChar; lpszDst: PKOLChar): BOOL; stdcall;
function OemToCharBuff(lpszSrc: PKOLChar; lpszDst: PKOLChar; cchDstLength: DWORD): BOOL; stdcall;
function OpenDesktop(lpszDesktop: PKOLChar; dwFlags: DWORD; fInherit: BOOL;
  dwDesiredAccess: DWORD): HDESK; stdcall;
function OpenWindowStation(lpszWinSta: PKOLChar; fInherit: BOOL;
  dwDesiredAccess: DWORD): HWINSTA; stdcall;
function PeekMessage(var lpMsg: TMsg; hWnd: HWND;
  wMsgFilterMin, wMsgFilterMax, wRemoveMsg: UINT): BOOL; stdcall;
function PostMessage(hWnd: HWND; Msg: UINT; wParam: WPARAM; lParam: LPARAM): BOOL; stdcall;
function PostThreadMessage(idThread: DWORD; Msg: UINT; wParam: WPARAM; lParam: LPARAM): BOOL; stdcall;
function RealGetWindowClass(hwnd: HWND; pszType: PKOLChar; cchType: UINT): UINT; stdcall;
function RegisterClass(const lpWndClass: TWndClass): ATOM; stdcall;
function RegisterClassEx(const WndClass: TWndClassEx): ATOM; stdcall;
function RegisterClipboardFormat(lpszFormat: PKOLChar): UINT; stdcall;
function RegisterDeviceNotification(hRecipient: THandle; NotificationFilter: Pointer; Flags: DWORD): HDEVNOTIFY; stdcall;
function RegisterWindowMessage(lpString: PKOLChar): UINT; stdcall;
function RemoveProp(hWnd: HWND; lpString: PKOLChar): THandle; stdcall;
function SendDlgItemMessage(hDlg: HWND; nIDDlgItem: Integer;
  Msg: UINT; wParam: WPARAM; lParam: LPARAM): Longint; stdcall;
function SendMessage(hWnd: HWND; Msg: UINT; wParam: WPARAM; lParam: LPARAM): LRESULT; stdcall;
function SendMessageCallback(hWnd: HWND; Msg: UINT; wParam: WPARAM;
  lParam: LPARAM; lpResultCallBack: TFNSendAsyncProc; dwData: DWORD): BOOL; stdcall;
function SendMessageTimeout(hWnd: HWND; Msg: UINT; wParam: WPARAM;
  lParam: LPARAM; fuFlags, uTimeout: UINT; var lpdwResult: DWORD): LRESULT; stdcall;
function SendNotifyMessage(hWnd: HWND; Msg: UINT; wParam: WPARAM;
  lParam: LPARAM): BOOL; stdcall;
function SetClassLong(hWnd: HWND; nIndex: Integer; dwNewLong: Longint): DWORD; stdcall;
function SetDlgItemText(hDlg: HWND; nIDDlgItem: Integer; lpString: PKOLChar): BOOL; stdcall;
function SetMenuItemInfo(p1: HMENU; p2: UINT; p3: BOOL; const p4: TMenuItemInfo): BOOL; stdcall;
function SetProp(hWnd: HWND; lpString: PKOLChar; hData: THandle): BOOL; stdcall;
function SetUserObjectInformation(hObj: THandle; nIndex: Integer;
  pvInfo: Pointer; nLength: DWORD): BOOL; stdcall;
function SetWindowLong(hWnd: HWND; nIndex: Integer; dwNewLong: Longint): Longint; stdcall;
function SetWindowText(hWnd: HWND; lpString: PKOLChar): BOOL; stdcall;
function SetWindowsHook(nFilterType: Integer; pfnFilterProc: TFNHookProc): HHOOK; stdcall;
function SetWindowsHookEx(idHook: Integer; lpfn: TFNHookProc; hmod: HINST; dwThreadId: DWORD): HHOOK; stdcall;
function SystemParametersInfo(uiAction, uiParam: UINT;
  pvParam: Pointer; fWinIni: UINT): BOOL; stdcall;
function TabbedTextOut(hDC: HDC; X, Y: Integer; lpString: PKOLChar; nCount, nTabPositions: Integer;
  var lpnTabStopPositions; nTabOrigin: Integer): Longint; stdcall;
function TranslateAccelerator(hWnd: HWND; hAccTable: HACCEL; var lpMsg: TMsg): Integer; stdcall;
function UnregisterClass(lpClassName: PKOLChar; hInstance: HINST): BOOL; stdcall;
function VkKeyScan(ch: KOLChar): SHORT; stdcall;
function VkKeyScanEx(ch: KOLChar; dwhkl: HKL): SHORT; stdcall;
function WinHelp(hWndMain: HWND; lpszHelp: PKOLChar; uCommand: UINT; dwData: DWORD): BOOL; stdcall;
function wsprintf(Output: PKOLChar; Format: PKOLChar): Integer; stdcall;
function wvsprintf(Output: PKOLChar; Format: PKOLChar; arglist: {$IFDEF UNICODE} PAnsiChar {$ELSE} va_list {$ENDIF}): Integer; stdcall;
function CreateMutex(lpMutexAttributes: PSecurityAttributes; bInitialOwner: BOOL; lpName: PAnsiChar): THandle;

const
  IDC_ARROW = MakeIntResource(32512);
  IDC_IBEAM = MakeIntResource(32513);
  IDC_WAIT = MakeIntResource(32514);
  IDC_CROSS = MakeIntResource(32515);
  IDC_UPARROW = MakeIntResource(32516);
  IDC_SIZE = MakeIntResource(32640);
  IDC_ICON = MakeIntResource(32641);
  IDC_SIZENWSE = MakeIntResource(32642);
  IDC_SIZENESW = MakeIntResource(32643);
  IDC_SIZEWE = MakeIntResource(32644);
  IDC_SIZENS = MakeIntResource(32645);
  IDC_SIZEALL = MakeIntResource(32646);
  IDC_NO = MakeIntResource(32648);
  IDC_HANDs = MakeIntResource(32649);
  IDC_APPSTARTING = MakeIntResource(32650);
  IDC_HELP = MakeIntResource(32651);
{$ENDIF interface_part} ////////////////////////////////////////////////////////

{$IFDEF implementation_part} ///////////////////////////////////////////////////
function AbortSystemShutdown; external advapi32 name 'AbortSystemShutdownA';
function AccessCheckAndAuditAlarm; external advapi32 name 'AccessCheckAndAuditAlarmA';
{$IFDEF _D4orHigher}
function AccessCheckByTypeAndAuditAlarm; external advapi32 name 'AccessCheckByTypeAndAuditAlarmA';
function AccessCheckByTypeResultListAndAuditAlarm; external advapi32 name 'AccessCheckByTypeResultListAndAuditAlarmA';
{$ENDIF _D4orHigher}
function BackupEventLog; external advapi32 name 'BackupEventLogA';
function ClearEventLog; external advapi32 name 'ClearEventLogA';
function CreateProcessAsUser; external advapi32 name 'CreateProcessAsUserA';
{$IFDEF _D3orHigher}
function GetCurrentHwProfile; external advapi32 name 'GetCurrentHwProfileA';
{$ENDIF _D3orHigher}
function GetFileSecurity; external advapi32 name 'GetFileSecurityA';
function GetUserName; external advapi32 name 'GetUserNameA';
function InitiateSystemShutdown; external advapi32 name 'InitiateSystemShutdownA';
function LogonUser; external advapi32 name 'LogonUserA';
function LookupAccountName; external advapi32 name 'LookupAccountNameA';
function LookupAccountSid; external advapi32 name 'LookupAccountSidA';
function LookupPrivilegeDisplayName; external advapi32 name 'LookupPrivilegeDisplayNameA';
function LookupPrivilegeName; external advapi32 name 'LookupPrivilegeNameA';
function LookupPrivilegeValue; external advapi32 name 'LookupPrivilegeValueA';
function ObjectCloseAuditAlarm; external advapi32 name 'ObjectCloseAuditAlarmA';
function ObjectDeleteAuditAlarm; external advapi32 name 'ObjectDeleteAuditAlarmA';
function ObjectOpenAuditAlarm; external advapi32 name 'ObjectOpenAuditAlarmA';
function ObjectPrivilegeAuditAlarm; external advapi32 name 'ObjectPrivilegeAuditAlarmA';
function OpenBackupEventLog; external advapi32 name 'OpenBackupEventLogA';
function OpenEventLog; external advapi32 name 'OpenEventLogA';
function PrivilegedServiceAuditAlarm; external advapi32 name 'PrivilegedServiceAuditAlarmA';
function ReadEventLog; external advapi32 name 'ReadEventLogA';
function RegConnectRegistry; external advapi32 name 'RegConnectRegistryA';
function RegCreateKey; external advapi32 name 'RegCreateKeyA';
function RegCreateKeyEx; external advapi32 name 'RegCreateKeyExA';
function RegDeleteKey; external advapi32 name 'RegDeleteKeyA';
function RegDeleteValue; external advapi32 name 'RegDeleteValueA';
function RegEnumKeyEx; external advapi32 name 'RegEnumKeyExA';
function RegEnumKey; external advapi32 name 'RegEnumKeyA';
function RegEnumValue; external advapi32 name 'RegEnumValueA';
function RegLoadKey; external advapi32 name 'RegLoadKeyA';
function RegOpenKey; external advapi32 name 'RegOpenKeyA';
function RegOpenKeyEx; external advapi32 name 'RegOpenKeyExA';
function RegQueryInfoKey; external advapi32 name 'RegQueryInfoKeyA';
function RegQueryMultipleValues; external advapi32 name 'RegQueryMultipleValuesA';
function RegQueryValue; external advapi32 name 'RegQueryValueA';
function RegQueryValueEx; external advapi32 name 'RegQueryValueExA';
function RegReplaceKey; external advapi32 name 'RegReplaceKeyA';
function RegRestoreKey; external advapi32 name 'RegRestoreKeyA';
function RegSaveKey; external advapi32 name 'RegSaveKeyA';
function RegSetValue; external advapi32 name 'RegSetValueA';
function RegSetValueEx; external advapi32 name 'RegSetValueExA';
function RegUnLoadKey; external advapi32 name 'RegUnLoadKeyA';
function RegisterEventSource; external advapi32 name 'RegisterEventSourceA';
function ReportEvent; external advapi32 name 'ReportEventA';
function SetFileSecurity; external advapi32 name 'SetFileSecurityA';
function AddAtom; external kernel32 name 'AddAtomA';
function BeginUpdateResource; external kernel32 name 'BeginUpdateResourceA';
function BuildCommDCB; external kernel32 name 'BuildCommDCBA';
function BuildCommDCBAndTimeouts; external kernel32 name 'BuildCommDCBAndTimeoutsA';
function CallNamedPipe; external kernel32 name 'CallNamedPipeA';
function CommConfigDialog; external kernel32 name 'CommConfigDialogA';
function CompareString; external kernel32 name 'CompareStringA';
function CopyFile; external kernel32 name 'CopyFileA';
{$IFDEF _D3orHigher}
function CopyFileEx; external kernel32 name 'CopyFileExA';
{$ENDIF _D3orHigher}
function CreateDirectory; external kernel32 name 'CreateDirectoryA';
function CreateDirectoryEx; external kernel32 name 'CreateDirectoryExA';
function CreateEvent; external kernel32 name 'CreateEventA';
function CreateFile; external kernel32 name 'CreateFileA';
function CreateFileMapping; external kernel32 name 'CreateFileMappingA';
function CreateHardLink; external kernel32 name 'CreateHardLinkA';
function CreateMailslot; external kernel32 name 'CreateMailslotA';
function CreateNamedPipe; external kernel32 name 'CreateNamedPipeA';
function CreateProcess; external kernel32 name 'CreateProcessA';
function CreateSemaphore; external kernel32 name 'CreateSemaphoreA';
function CreateWaitableTimer; external kernel32 name 'CreateWaitableTimerA';
function DefineDosDevice; external kernel32 name 'DefineDosDeviceA';
function DeleteFile; external kernel32 name 'DeleteFileA';
function EndUpdateResource; external kernel32 name 'EndUpdateResourceA';
function EnumCalendarInfo; external kernel32 name 'EnumCalendarInfoA';
function EnumDateFormats; external kernel32 name 'EnumDateFormatsA';
function EnumResourceLanguages; external kernel32 name 'EnumResourceLanguagesA';
function EnumResourceNames; external kernel32 name 'EnumResourceNamesA';
function EnumResourceTypes; external kernel32 name 'EnumResourceTypesA';
function EnumSystemCodePages; external kernel32 name 'EnumSystemCodePagesA';
function EnumSystemLocales; external kernel32 name 'EnumSystemLocalesA';
function EnumTimeFormats; external kernel32 name 'EnumTimeFormatsA';
function ExpandEnvironmentStrings; external kernel32 name 'ExpandEnvironmentStringsA';
procedure FatalAppExit; external kernel32 name 'FatalAppExitA';
function FillConsoleOutputCharacter; external kernel32 name 'FillConsoleOutputCharacterA';
function FindAtom; external kernel32 name 'FindAtomA';
function FindFirstChangeNotification; external kernel32 name 'FindFirstChangeNotificationA';
function FindFirstFile; external kernel32 name 'FindFirstFileA';
{$IFDEF _D3orHigher}
function FindFirstFileEx; external kernel32 name 'FindFirstFileExA';
{$ENDIF _D3orHigher}
function FindNextFile; external kernel32 name 'FindNextFileA';
function FindResource; external kernel32 name 'FindResourceA';
function FindResourceEx; external kernel32 name 'FindResourceExA';
function FoldString; external kernel32 name 'FoldStringA';
function FormatMessage; external kernel32 name 'FormatMessageA';
function FreeEnvironmentStrings; external kernel32 name 'FreeEnvironmentStringsA';
function GetAtomName; external kernel32 name 'GetAtomNameA';
function GetBinaryType; external kernel32 name 'GetBinaryTypeA';
function GetCommandLine; external kernel32 name 'GetCommandLineA';
function GetCompressedFileSize; external kernel32 name 'GetCompressedFileSizeA';
function GetComputerName; external kernel32 name 'GetComputerNameA';
function GetConsoleTitle; external kernel32 name 'GetConsoleTitleA';
function GetCurrencyFormat; external kernel32 name 'GetCurrencyFormatA';
function GetCurrentDirectory; external kernel32 name 'GetCurrentDirectoryA';
function GetDateFormat; external kernel32 name 'GetDateFormatA';
function GetDefaultCommConfig; external kernel32 name 'GetDefaultCommConfigA';
function GetDiskFreeSpace; external kernel32 name 'GetDiskFreeSpaceA';
function GetDiskFreeSpaceEx; external kernel32 name 'GetDiskFreeSpaceExA';
function GetDriveType; external kernel32 name 'GetDriveTypeA';
function GetEnvironmentStrings; external kernel32 name 'GetEnvironmentStringsA';
function GetEnvironmentVariable(lpName: PKOLChar; lpBuffer: PKOLChar;
  nSize: DWORD): DWORD; external kernel32 name 'GetEnvironmentVariableA';
function GetFileAttributes; external kernel32 name 'GetFileAttributesA';
{$IFDEF _D3orHigher}
function GetFileAttributesEx; external kernel32 name 'GetFileAttributesExA';
{$ENDIF _D3orHigher}
function GetFullPathName; external kernel32 name 'GetFullPathNameA';
function GetLocaleInfo; external kernel32 name 'GetLocaleInfoA';
function GetLogicalDriveStrings; external kernel32 name 'GetLogicalDriveStringsA';
function GetModuleFileName; external kernel32 name 'GetModuleFileNameA';
function GetModuleHandle; external kernel32 name 'GetModuleHandleA';
function GetNamedPipeHandleState; external kernel32 name 'GetNamedPipeHandleStateA';
function GetNumberFormat; external kernel32 name 'GetNumberFormatA';
function GetPrivateProfileInt; external kernel32 name 'GetPrivateProfileIntA';
function GetPrivateProfileSection; external kernel32 name 'GetPrivateProfileSectionA';
function GetPrivateProfileSectionNames; external kernel32 name 'GetPrivateProfileSectionNamesA';
function GetPrivateProfileString; external kernel32 name 'GetPrivateProfileStringA';
function GetProfileInt; external kernel32 name 'GetProfileIntA';
function GetProfileSection; external kernel32 name 'GetProfileSectionA';
function GetProfileString; external kernel32 name 'GetProfileStringA';
function GetShortPathName; external kernel32 name 'GetShortPathNameA';
procedure GetStartupInfo; external kernel32 name 'GetStartupInfoA';
function GetStringTypeEx; external kernel32 name 'GetStringTypeExA';
function GetSystemDirectory; external kernel32 name 'GetSystemDirectoryA';
function GetTempFileName; external kernel32 name 'GetTempFileNameA';
function GetTempPath; external kernel32 name 'GetTempPathA';
function GetTimeFormat; external kernel32 name 'GetTimeFormatA';
function GetVersionEx; external kernel32 name 'GetVersionExA';
function GetVolumeInformation; external kernel32 name 'GetVolumeInformationA';
function GetWindowsDirectory; external kernel32 name 'GetWindowsDirectoryA';
function GlobalAddAtom; external kernel32 name 'GlobalAddAtomA';
function GlobalFindAtom; external kernel32 name 'GlobalFindAtomA';
function GlobalGetAtomName; external kernel32 name 'GlobalGetAtomNameA';
function IsBadStringPtr; external kernel32 name 'IsBadStringPtrA';
function LCMapString; external kernel32 name 'LCMapStringA';
function LoadLibrary; external kernel32 name 'LoadLibraryA';
function LoadLibraryEx; external kernel32 name 'LoadLibraryExA';
function MoveFile; external kernel32 name 'MoveFileA';
function MoveFileEx; external kernel32 name 'MoveFileExA';
{$IFDEF _D3orHigher}
function MoveFileWithProgress; external kernel32 name 'MoveFileWithProgressA';
{$ENDIF _D3orHigher}
function OpenEvent; external kernel32 name 'OpenEventA';
function OpenFileMapping; external kernel32 name 'OpenFileMappingA';
function OpenMutex; external kernel32 name 'OpenMutexA';
function OpenSemaphore; external kernel32 name 'OpenSemaphoreA';
function OpenWaitableTimer; external kernel32 name 'OpenWaitableTimerA';
procedure OutputDebugString; external kernel32 name 'OutputDebugStringA';
function PeekConsoleInput; external kernel32 name 'PeekConsoleInputA';
function QueryDosDevice; external kernel32 name 'QueryDosDeviceA';
function QueryRecoveryAgents; external kernel32 name 'QueryRecoveryAgentsA';
function ReadConsole; external kernel32 name 'ReadConsoleA';
function ReadConsoleInput; external kernel32 name 'ReadConsoleInputA';
function ReadConsoleOutput; external kernel32 name 'ReadConsoleOutputA';
function ReadConsoleOutputCharacter; external kernel32 name 'ReadConsoleOutputCharacterA';
function RemoveDirectory; external kernel32 name 'RemoveDirectoryA';
function ScrollConsoleScreenBuffer; external kernel32 name 'ScrollConsoleScreenBufferA';
function SearchPath; external kernel32 name 'SearchPathA';
function SetComputerName; external kernel32 name 'SetComputerNameA';
function SetConsoleTitle; external kernel32 name 'SetConsoleTitleA';
function SetCurrentDirectory; external kernel32 name 'SetCurrentDirectoryA';
function SetDefaultCommConfig; external kernel32 name 'SetDefaultCommConfigA';
function SetEnvironmentVariable; external kernel32 name 'SetEnvironmentVariableA';
function SetFileAttributes; external kernel32 name 'SetFileAttributesA';
function SetLocaleInfo; external kernel32 name 'SetLocaleInfoA';
function SetVolumeLabel; external kernel32 name 'SetVolumeLabelA';
function UpdateResource; external kernel32 name 'UpdateResourceA';
function VerLanguageName; external kernel32 name 'VerLanguageNameA';
function WaitNamedPipe; external kernel32 name 'WaitNamedPipeA';
function WriteConsole; external kernel32 name 'WriteConsoleA';
function WriteConsoleInput; external kernel32 name 'WriteConsoleInputA';
function WriteConsoleOutput; external kernel32 name 'WriteConsoleOutputA';
function WriteConsoleOutputCharacter; external kernel32 name 'WriteConsoleOutputCharacterA';
function WritePrivateProfileSection; external kernel32 name 'WritePrivateProfileSectionA';
function WritePrivateProfileString; external kernel32 name 'WritePrivateProfileStringA';
function WriteProfileSection; external kernel32 name 'WriteProfileSectionA';
function WriteProfileString; external kernel32 name 'WriteProfileStringA';
function lstrcat; external kernel32 name 'lstrcatA';
function lstrcmp; external kernel32 name 'lstrcmpA';
function lstrcmpi; external kernel32 name 'lstrcmpiA';
function lstrcpy; external kernel32 name 'lstrcpyA';
function lstrcpyn; external kernel32 name 'lstrcpynA';
function lstrlen; external kernel32 name 'lstrlenA';
function MultinetGetConnectionPerformance; external mpr name 'MultinetGetConnectionPerformanceA';
function WNetAddConnection2; external mpr name 'WNetAddConnection2A';
function WNetAddConnection3; external mpr name 'WNetAddConnection3A';
function WNetAddConnection; external mpr name 'WNetAddConnectionA';
function WNetCancelConnection2; external mpr name 'WNetCancelConnection2A';
function WNetCancelConnection; external mpr name 'WNetCancelConnectionA';
function WNetConnectionDialog1; external mpr name 'WNetConnectionDialog1A';
function WNetDisconnectDialog1; external mpr name 'WNetDisconnectDialog1A';
function WNetEnumResource; external mpr name 'WNetEnumResourceA';
function WNetGetConnection; external mpr name 'WNetGetConnectionA';
function WNetGetLastError; external mpr name 'WNetGetLastErrorA';
function WNetGetNetworkInformation; external mpr name 'WNetGetNetworkInformationA';
function WNetGetProviderName; external mpr name 'WNetGetProviderNameA';
function WNetGetResourceParent; external mpr name 'WNetGetResourceParentA';
function WNetGetUniversalName; external mpr name 'WNetGetUniversalNameA';
function WNetGetUser; external mpr name 'WNetGetUserA';
function WNetOpenEnum; external mpr name 'WNetOpenEnumA';
function WNetSetConnection; external mpr name 'WNetSetConnectionA';
function WNetUseConnection; external mpr name 'WNetUseConnectionA';
function GetFileVersionInfo; external version name 'GetFileVersionInfoA';
function GetFileVersionInfoSize; external version name 'GetFileVersionInfoSizeA';
function VerFindFile; external version name 'VerFindFileA';
function VerInstallFile; external version name 'VerInstallFileA';
function VerQueryValue; external version name 'VerQueryValueA';
function GetPrivateProfileStruct; external kernel32 name 'GetPrivateProfileStructA';
function WritePrivateProfileStruct; external kernel32 name 'WritePrivateProfileStructA';
function AddFontResource; external gdi32 name 'AddFontResourceA';
function AddFontResourceEx; external gdi32 name 'AddFontResourceExA';
function CopyEnhMetaFile; external gdi32 name 'CopyEnhMetaFileA';
function CopyMetaFile; external gdi32 name 'CopyMetaFileA';
function CreateColorSpace; external gdi32 name 'CreateColorSpaceA';
function CreateDC; external gdi32 name 'CreateDCA';
function CreateEnhMetaFile; external gdi32 name 'CreateEnhMetaFileA';
function CreateFont; external gdi32 name 'CreateFontA';
function CreateFontIndirect; external gdi32 name 'CreateFontIndirectA';
function CreateFontIndirectEx; external gdi32 name 'CreateFontIndirectExA';
function CreateIC; external gdi32 name 'CreateICA';
function CreateMetaFile; external gdi32 name 'CreateMetaFileA';
function CreateScalableFontResource; external gdi32 name 'CreateScalableFontResourceA';
function DeviceCapabilities; external gdi32 name 'DeviceCapabilitiesA';
function EnumFontFamilies; external gdi32 name 'EnumFontFamiliesA';
function EnumFontFamiliesEx; external gdi32 name 'EnumFontFamiliesExA';
function EnumFonts; external gdi32 name 'EnumFontsA';
function EnumICMProfiles; external gdi32 name 'EnumICMProfilesA';
function ExtTextOut; external gdi32 name 'ExtTextOutA';
function GetCharABCWidths; external gdi32 name 'GetCharABCWidthsA';
function GetCharABCWidthsFloat; external gdi32 name 'GetCharABCWidthsFloatA';
function GetCharWidth32; external gdi32 name 'GetCharWidth32A';
function GetCharWidth; external gdi32 name 'GetCharWidthA';
function GetCharWidthFloat; external gdi32 name 'GetCharWidthFloatA';
function GetCharacterPlacement; external gdi32 name 'GetCharacterPlacementA';
function GetEnhMetaFile; external gdi32 name 'GetEnhMetaFileA';
function GetEnhMetaFileDescription; external gdi32 name 'GetEnhMetaFileDescriptionA';
function GetGlyphIndices; external gdi32 name 'GetGlyphIndicesA';
function GetGlyphOutline; external gdi32 name 'GetGlyphOutlineA';
function GetICMProfile; external gdi32 name 'GetICMProfileA';
function GetLogColorSpace; external gdi32 name 'GetLogColorSpaceA';
function GetMetaFile; external gdi32 name 'GetMetaFileA';
function GetObject; external gdi32 name 'GetObjectA';
function GetOutlineTextMetrics; external gdi32 name 'GetOutlineTextMetricsA';
function GetTextExtentExPoint; external gdi32 name 'GetTextExtentExPointA';
function GetTextExtentPoint32; external gdi32 name 'GetTextExtentPoint32A';
function GetTextExtentPoint; external gdi32 name 'GetTextExtentPointA';
function GetTextFace; external gdi32 name 'GetTextFaceA';
function GetTextMetrics; external gdi32 name 'GetTextMetricsA';
function PolyTextOut; external gdi32 name 'PolyTextOutA';
function RemoveFontResource; external gdi32 name 'RemoveFontResourceA';
function RemoveFontResourceEx; external gdi32 name 'RemoveFontResourceExA';
function ResetDC; external gdi32 name 'ResetDCA';
function SetICMProfile; external gdi32 name 'SetICMProfileA';
function StartDoc; external gdi32 name 'StartDocA';
function TextOut; external gdi32 name 'TextOutA';
function UpdateICMRegKey; external gdi32 name 'UpdateICMRegKeyA';
function wglUseFontBitmaps; external opengl32 name 'wglUseFontBitmapsA';
function wglUseFontOutlines; external opengl32 name 'wglUseFontOutlinesA';
function AnsiToOem; external user32 name 'CharToOemA';
function AnsiToOemBuff; external user32 name 'CharToOemBuffA';
function AnsiUpper; external user32 name 'CharUpperA';
function AnsiUpperBuff; external user32 name 'CharUpperBuffA';
function AnsiLower; external user32 name 'CharLowerA';
function AnsiLowerBuff; external user32 name 'CharLowerBuffA';
function AnsiNext; external user32 name 'CharNextA';
function AnsiPrev; external user32 name 'CharPrevA';
function AppendMenu; external user32 name 'AppendMenuA';
//function BroadcastSystemMessage; external user32 name 'BroadcastSystemMessageA';
//function BroadcastSystemMessageA; external user32 name 'BroadcastSystemMessageA';
function CallMsgFilter; external user32 name 'CallMsgFilterA';
function CallWindowProc; external user32 name 'CallWindowProcA';
function ChangeDisplaySettings; external user32 name 'ChangeDisplaySettingsA';
function ChangeDisplaySettingsEx; external user32 name 'ChangeDisplaySettingsExA';
function ChangeMenu; external user32 name 'ChangeMenuA';
function CharLower; external user32 name 'CharLowerA';
function CharLowerBuff; external user32 name 'CharLowerBuffA';
function CharNext; external user32 name 'CharNextA';
function CharNextEx; external user32 name 'CharNextExA';
function CharPrev; external user32 name 'CharPrevA';
function CharPrevEx; external user32 name 'CharPrevExA';
function CharToOem; external user32 name 'CharToOemA';
function CharToOemBuff; external user32 name 'CharToOemBuffA';
function CharUpper; external user32 name 'CharUpperA';
function CharUpperBuff; external user32 name 'CharUpperBuffA';
function CopyAcceleratorTable; external user32 name 'CopyAcceleratorTableA';
function CreateAcceleratorTable; external user32 name 'CreateAcceleratorTableA';
function CreateDesktop; external user32 name 'CreateDesktopA';
function CreateDialogIndirectParam; external user32 name 'CreateDialogIndirectParamA';
function CreateDialogParam; external user32 name 'CreateDialogParamA';
function CreateMDIWindow; external user32 name 'CreateMDIWindowA';
function CreateWindowEx; external user32 name 'CreateWindowExA';
function CreateWindowStation; external user32 name 'CreateWindowStationA';
function DefDlgProc; external user32 name 'DefDlgProcA';
function DefFrameProc; external user32 name 'DefFrameProcA';
function DefMDIChildProc; external user32 name 'DefMDIChildProcA';
function DefWindowProc; external user32 name 'DefWindowProcA';
function DialogBoxIndirectParam; external user32 name 'DialogBoxIndirectParamA';
function DialogBoxParam; external user32 name 'DialogBoxParamA';
function DispatchMessage; external user32 name 'DispatchMessageA';
function DlgDirList; external user32 name 'DlgDirListA';
function DlgDirListComboBox; external user32 name 'DlgDirListComboBoxA';
function DlgDirSelectComboBoxEx; external user32 name 'DlgDirSelectComboBoxExA';
function DlgDirSelectEx; external user32 name 'DlgDirSelectExA';
function DrawState; external user32 name 'DrawStateA';
function DrawText; external user32 name 'DrawTextA';
function DrawTextEx; external user32 name 'DrawTextExA';
function EnumDesktops; external user32 name 'EnumDesktopsA';
function EnumDisplaySettings; external user32 name 'EnumDisplaySettingsA';
function EnumDisplayDevices; external user32 name 'EnumDisplayDevicesA';
function EnumProps; external user32 name 'EnumPropsA';
function EnumPropsEx; external user32 name 'EnumPropsExA';
function EnumWindowStations; external user32 name 'EnumWindowStationsA';
function FindWindow; external user32 name 'FindWindowA';
function FindWindowEx; external user32 name 'FindWindowExA';
function GetAltTabInfo; external user32 name 'GetAltTabInfoA';
function GetClassInfo; external user32 name 'GetClassInfoA';
function GetClassInfoEx; external user32 name 'GetClassInfoExA';
function GetClassLong; external user32 name 'GetClassLongA';
function GetClassName; external user32 name 'GetClassNameA';
function GetClipboardFormatName; external user32 name 'GetClipboardFormatNameA';
function GetDlgItemText; external user32 name 'GetDlgItemTextA';
function GetKeyNameText; external user32 name 'GetKeyNameTextA';
function GetKeyboardLayoutName; external user32 name 'GetKeyboardLayoutNameA';
function GetMenuItemInfo; external user32 name 'GetMenuItemInfoA';
function GetMenuString; external user32 name 'GetMenuStringA';
function GetMessage; external user32 name 'GetMessageA';
function GetProp; external user32 name 'GetPropA';
function GetTabbedTextExtent; external user32 name 'GetTabbedTextExtentA';
function GetUserObjectInformation; external user32 name 'GetUserObjectInformationA';
function GetWindowLong; external user32 name 'GetWindowLongA';
function GetWindowModuleFileName; external user32 name 'GetWindowModuleFileNameA';
function GetWindowText; external user32 name 'GetWindowTextA';
function GetWindowTextLength; external user32 name 'GetWindowTextLengthA';
function GrayString; external user32 name 'GrayStringA';
function InsertMenu; external user32 name 'InsertMenuA';
function InsertMenuItem; external user32 name 'InsertMenuItemA';
function IsCharAlpha; external user32 name 'IsCharAlphaA';
function IsCharAlphaNumeric; external user32 name 'IsCharAlphaNumericA';
function IsCharLower; external user32 name 'IsCharLowerA';
function IsCharUpper; external user32 name 'IsCharUpperA';
function IsDialogMessage; external user32 name 'IsDialogMessageA';
function LoadAccelerators; external user32 name 'LoadAcceleratorsA';
function LoadBitmap; external user32 name 'LoadBitmapA';
function LoadCursor; external user32 name 'LoadCursorA';
function LoadCursorFromFile; external user32 name 'LoadCursorFromFileA';
function LoadIcon; external user32 name 'LoadIconA';
function LoadImage; external user32 name 'LoadImageA';
function LoadKeyboardLayout; external user32 name 'LoadKeyboardLayoutA';
function LoadMenu; external user32 name 'LoadMenuA';
function LoadMenuIndirect; external user32 name 'LoadMenuIndirectA';
function LoadString; external user32 name 'LoadStringA';
function MapVirtualKey; external user32 name 'MapVirtualKeyA';
function MapVirtualKeyEx; external user32 name 'MapVirtualKeyExA';
function MessageBox; external user32 name 'MessageBoxA';
function MessageBoxEx; external user32 name 'MessageBoxExA';
function MessageBoxIndirect; external user32 name 'MessageBoxIndirectA';
function ModifyMenu; external user32 name 'ModifyMenuA';
function OemToAnsi; external user32 name 'OemToCharA';
function OemToAnsiBuff; external user32 name 'OemToCharBuffA';
function OemToChar; external user32 name 'OemToCharA';
function OemToCharBuff; external user32 name 'OemToCharBuffA';
function OpenDesktop; external user32 name 'OpenDesktopA';
function OpenWindowStation; external user32 name 'OpenWindowStationA';
function PeekMessage; external user32 name 'PeekMessageA';
function PostMessage; external user32 name 'PostMessageA';
function PostThreadMessage; external user32 name 'PostThreadMessageA';
function RealGetWindowClass; external user32 name 'RealGetWindowClassA';
function RegisterClass; external user32 name 'RegisterClassA';
function RegisterClassEx; external user32 name 'RegisterClassExA';
function RegisterClipboardFormat; external user32 name 'RegisterClipboardFormatA';
function RegisterDeviceNotification; external user32 name 'RegisterDeviceNotificationA';
function RegisterWindowMessage; external user32 name 'RegisterWindowMessageA';
function RemoveProp; external user32 name 'RemovePropA';
function SendDlgItemMessage; external user32 name 'SendDlgItemMessageA';
function SendMessage; external user32 name 'SendMessageA';
function SendMessageCallback; external user32 name 'SendMessageCallbackA';
function SendMessageTimeout; external user32 name 'SendMessageTimeoutA';
function SendNotifyMessage; external user32 name 'SendNotifyMessageA';
function SetClassLong; external user32 name 'SetClassLongA';
function SetDlgItemText; external user32 name 'SetDlgItemTextA';
function SetMenuItemInfo; external user32 name 'SetMenuItemInfoA';
function SetProp; external user32 name 'SetPropA';
function SetUserObjectInformation; external user32 name 'SetUserObjectInformationA';
function SetWindowLong; external user32 name 'SetWindowLongA';
function SetWindowText; external user32 name 'SetWindowTextA';
function SetWindowsHook; external user32 name 'SetWindowsHookA';
function SetWindowsHookEx; external user32 name 'SetWindowsHookExA';
function SystemParametersInfo; external user32 name 'SystemParametersInfoA';
function TabbedTextOut; external user32 name 'TabbedTextOutA';
function TranslateAccelerator; external user32 name 'TranslateAcceleratorA';
function UnregisterClass; external user32 name 'UnregisterClassA';
function VkKeyScan; external user32 name 'VkKeyScanA';
function VkKeyScanEx; external user32 name 'VkKeyScanExA';
function WinHelp; external user32 name 'WinHelpA';
function wsprintf; external user32 name 'wsprintfA';
function wvsprintf; external user32 name 'wvsprintfA';
// NT 4.0 bug workaround - NT 4.0 doesn't test bInitialOwner for zero/nonzero, it tests for 1
function _CreateMutex(lpMutexAttributes: PSecurityAttributes;
  bInitialOwner: Integer; lpName: PAnsiChar): THandle; stdcall;
  external kernel32 name 'CreateMutexA';
function CreateMutex(lpMutexAttributes: PSecurityAttributes; bInitialOwner: BOOL; lpName: PAnsiChar): THandle;
begin
  Result := _CreateMutex(lpMutexAttributes, Integer(Boolean(bInitialOwner)), lpName);
end;

{$ENDIF implementation_part} ///////////////////////////////////////////////////
