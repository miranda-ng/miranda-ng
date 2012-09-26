////////////////////////////////////////////////////////////////////////////////
// All code below is exclusively owned by author of Chess4Net - Pavel Perminov
// (packpaul@mail.ru, packpaul1@gmail.com).
// Any changes, modifications, borrowing and adaptation are a subject for
// explicit permition from the owner.

unit IniSettingsUnit;

interface

uses
  TntIniFiles,
  //
  ChessBoardUnit, ChessRulesEngine;

type
  TIniSettingsID = (isidLastMoveHilighted, isidFlashOnMove, isidCoordinatesShown,
    isidStayOnTop, isidExtraExit, isidActiveLanguage, isidDontShowLastVersion,
    isidDontShowCredits, isidClock, isidTrainingMode, isidCanPauseGame,
    isidCanAdjournGame, isidExternalBaseName, isidUseUserBase, isidAllowTakebacks,
    isidAutoFlag, isidAdjourned);

  TIniSettings = class
  private
    m_IniFile: TTntIniFile;

    m_strOpponentId: string;

    constructor FCreate;
    function FGetIniFileName: string;

    function FGetBooleanValue(ID: TIniSettingsID): boolean;
    procedure FSetBooleanValue(ID: TIniSettingsID; bValue: boolean);

    function FGetIntegerValue(ID: TIniSettingsID): integer;
    procedure FSetIntegerValue(ID: TIniSettingsID; iValue: integer);

    function FGetStringValue(ID: TIniSettingsID): string;
    procedure FSetStringValue(ID: TIniSettingsID; const strValue: string);

    function FGetAnimation: TAnimation;
    procedure FSetAnimation(Value: TAnimation);

    function FGetCommonSectionName: string;

    function FGetPlayerColor: TFigureColor;
    procedure FSetPlayerColor(Value: TFigureColor);

  public
    destructor Destroy; override;

    class function Instance: TIniSettings;
    class procedure FreeInstance; reintroduce;

    procedure SetOpponentId(const strValue: string);
    function HasCommonSettings: boolean;

    // private settings
    property Animation: TAnimation
      read FGetAnimation write FSetAnimation;
    property LastMoveHilighted: boolean index isidLastMoveHilighted
      read FGetBooleanValue write FSetBooleanValue;
    property FlashOnMove: boolean index isidFlashOnMove
      read FGetBooleanValue write FSetBooleanValue;
    property CoordinatesShown: boolean index isidCoordinatesShown
      read FGetBooleanValue write FSetBooleanValue;
    property StayOnTop: boolean index isidStayOnTop
      read FGetBooleanValue write FSetBooleanValue;
    property ExtraExit: boolean index isidExtraExit
      read FGetBooleanValue write FSetBooleanValue;
    property ActiveLanguage: integer index isidActiveLanguage
      read FGetIntegerValue write FSetIntegerValue;
    property DontShowLastVersion: integer index isidDontShowLastVersion
      read FGetIntegerValue write FSetIntegerValue;
    property DontShowCredits: boolean index isidDontShowCredits
      read FGetBooleanValue write FSetBooleanValue;

    // common settings
    property PlayerColor: TFigureColor
      read FGetPlayerColor write FSetPlayerColor;
    property Clock: string index isidClock
      read FGetStringValue write FSetStringValue;
    property TrainingMode: boolean index isidTrainingMode
      read FGetBooleanValue write FSetBooleanValue;
    property CanPauseGame: boolean index isidCanPauseGame
      read FGetBooleanValue write FSetBooleanValue;
    property CanAdjournGame: boolean index isidCanAdjournGame
      read FGetBooleanValue write FSetBooleanValue;
    property ExternalBaseName: string index isidExternalBaseName
      read FGetStringValue write FSetStringValue;
    property UseUserBase: boolean index isidUseUserBase
      read FGetBooleanValue write FSetBooleanValue;
    property AllowTakebacks: boolean index isidAllowTakebacks
      read FGetBooleanValue write FSetBooleanValue;
    property AutoFlag: boolean index isidAutoFlag
      read FGetBooleanValue write FSetBooleanValue;
    property Adjourned: string index isidAdjourned
      read FGetStringValue write FSetStringValue;
  end;

implementation

uses
  SysUtils,
  //
  GlobalsUnit, GlobalsLocalUnit;

const
  INI_FILE_NAME = 'Chess4Net.ini';

  PRIVATE_SECTION_NAME = 'Private';
  COMMON_SECTION_PREFIX = 'Common';
  ANIMATION_KEY_NAME = 'Animation';
  HILIGHT_LAST_MOVE_KEY_NAME = 'HilightLastMove';
  FLASH_ON_MOVE_NAME = 'FlashOnMove';
  SHOW_COORDINATES_KEY_NAME = 'ShowCoordinates';
  STAY_ON_TOP_KEY_NAME = 'StayOnTop';
  EXTRA_EXIT_KEY_NAME = 'ExtraExit';
  CAN_PAUSE_GAME_KEY_NAME = 'CanPauseGame';
  CAN_ADJOURN_GAME_KEY_NAME = 'CanAdjournGame';
  ALLOW_TAKEBACKS_KEY_NAME = 'AllowTakebacks';
  EXTERNAL_BASE_NAME_KEY_NAME = 'ExternalBaseName';
  USE_USER_BASE_KEY_NAME = 'UseUserBase';
  AUTO_FLAG_KEY_NAME = 'AutoFlag';
  TRAINING_MODE_KEY_NAME = 'TrainingMode';
  PLAYER_COLOR_KEY_NAME = 'PlayerColor';
  CLOCK_KEY_NAME = 'Clock';
  ADJOURNED_KEY_NAME = 'Adjourned';
  LANGUAGE_KEY_NAME = 'Language';
  DONT_SHOW_CREDITS = 'DontShowCredits';
  DONT_SHOW_LAST_VERSION = 'DontShowLastVersion';

var
  g_Instance: TIniSettings = nil;

////////////////////////////////////////////////////////////////////////////////
// TIniSettings

constructor TIniSettings.FCreate;
begin
  inherited Create;
  m_IniFile := TTntIniFile.Create(FGetIniFileName);
end;


destructor TIniSettings.Destroy;
begin
  m_IniFile.Free;
  inherited;
end;


class function TIniSettings.Instance: TIniSettings;
begin
  if (not Assigned(g_Instance)) then
    g_Instance := TIniSettings.FCreate;
  Result := g_Instance;
end;


class procedure TIniSettings.FreeInstance;
begin
  FreeAndNil(g_Instance);
end;


function TIniSettings.FGetIniFileName: string;
begin
  Result := Chess4NetIniFilePath + INI_FILE_NAME;
end;


function TIniSettings.FGetAnimation: TAnimation;
begin
  Result := TAnimation(m_IniFile.ReadInteger(PRIVATE_SECTION_NAME, ANIMATION_KEY_NAME, Ord(aQuick)))
end;


procedure TIniSettings.FSetAnimation(Value: TAnimation);
begin
  m_IniFile.WriteInteger(PRIVATE_SECTION_NAME, ANIMATION_KEY_NAME, Ord(Value));
end;


function TIniSettings.FGetBooleanValue(ID: TIniSettingsID): boolean;
begin
  case ID of
    isidLastMoveHilighted:
      Result := m_IniFile.ReadBool(PRIVATE_SECTION_NAME, HILIGHT_LAST_MOVE_KEY_NAME, FALSE);
    isidFlashOnMove:
      Result := m_IniFile.ReadBool(PRIVATE_SECTION_NAME, FLASH_ON_MOVE_NAME, FALSE);
    isidCoordinatesShown:
      Result := m_IniFile.ReadBool(PRIVATE_SECTION_NAME, SHOW_COORDINATES_KEY_NAME, TRUE);
    isidStayOnTop:
      Result := m_IniFile.ReadBool(PRIVATE_SECTION_NAME, STAY_ON_TOP_KEY_NAME, FALSE);
    isidExtraExit:
      Result := m_IniFile.ReadBool(PRIVATE_SECTION_NAME, EXTRA_EXIT_KEY_NAME, FALSE);
    isidDontShowCredits:
      Result := m_IniFile.ReadBool(PRIVATE_SECTION_NAME, DONT_SHOW_CREDITS, FALSE);
    isidTrainingMode:
      Result := m_IniFile.ReadBool(FGetCommonSectionName, TRAINING_MODE_KEY_NAME, FALSE);
    isidCanPauseGame:
      Result := m_IniFile.ReadBool(FGetCommonSectionName, CAN_PAUSE_GAME_KEY_NAME, FALSE);
    isidCanAdjournGame:
      Result := m_IniFile.ReadBool(FGetCommonSectionName, CAN_ADJOURN_GAME_KEY_NAME, FALSE);
    isidUseUserBase:
      Result := m_IniFile.ReadBool(FGetCommonSectionName, USE_USER_BASE_KEY_NAME, FALSE);
    isidAllowTakebacks:
      Result := m_IniFile.ReadBool(FGetCommonSectionName, ALLOW_TAKEBACKS_KEY_NAME, FALSE);
    isidAutoFlag:
      Result := m_IniFile.ReadBool(FGetCommonSectionName, AUTO_FLAG_KEY_NAME, FALSE);
  else
    Result := FALSE;  
    Assert(FALSE);
  end;
end;


procedure TIniSettings.FSetBooleanValue(ID: TIniSettingsID; bValue: boolean);
begin
  case ID of
    isidLastMoveHilighted:
      m_IniFile.WriteBool(PRIVATE_SECTION_NAME, HILIGHT_LAST_MOVE_KEY_NAME, bValue);
    isidFlashOnMove:
      m_IniFile.WriteBool(PRIVATE_SECTION_NAME, FLASH_ON_MOVE_NAME, bValue);
    isidCoordinatesShown:
      m_IniFile.WriteBool(PRIVATE_SECTION_NAME, SHOW_COORDINATES_KEY_NAME, bValue);
    isidStayOnTop:
      m_IniFile.WriteBool(PRIVATE_SECTION_NAME, STAY_ON_TOP_KEY_NAME, bValue);
    isidExtraExit:
      m_IniFile.WriteBool(PRIVATE_SECTION_NAME, EXTRA_EXIT_KEY_NAME, bValue);
    isidDontShowCredits:
      m_IniFile.WriteBool(PRIVATE_SECTION_NAME, DONT_SHOW_CREDITS, bValue);
    isidTrainingMode:
      m_IniFile.WriteBool(FGetCommonSectionName, TRAINING_MODE_KEY_NAME, bValue);
    isidCanPauseGame:
      m_IniFile.WriteBool(FGetCommonSectionName, CAN_PAUSE_GAME_KEY_NAME, bValue);
    isidCanAdjournGame:
      m_IniFile.WriteBool(FGetCommonSectionName, CAN_ADJOURN_GAME_KEY_NAME, bValue);
    isidUseUserBase:
      m_IniFile.WriteBool(FGetCommonSectionName, USE_USER_BASE_KEY_NAME, bValue);
    isidAllowTakebacks:
      m_IniFile.WriteBool(FGetCommonSectionName, ALLOW_TAKEBACKS_KEY_NAME, bValue);
    isidAutoFlag:
      m_IniFile.WriteBool(FGetCommonSectionName, AUTO_FLAG_KEY_NAME, bValue);
  else
    Assert(FALSE);
  end;
end;


function TIniSettings.FGetIntegerValue(ID: TIniSettingsID): integer;
begin
  case ID of
    isidActiveLanguage:
      Result := m_IniFile.ReadInteger(PRIVATE_SECTION_NAME, LANGUAGE_KEY_NAME, 1) - 1;
    isidDontShowLastVersion:
      Result := m_IniFile.ReadInteger(PRIVATE_SECTION_NAME, DONT_SHOW_LAST_VERSION, CHESS4NET_VERSION);
  else
    Result := 0;
    Assert(FALSE);
  end;
end;


procedure TIniSettings.FSetIntegerValue(ID: TIniSettingsID; iValue: integer);
begin
  case ID of
    isidActiveLanguage:
      m_IniFile.WriteInteger(PRIVATE_SECTION_NAME, LANGUAGE_KEY_NAME, iValue + 1);
    isidDontShowLastVersion:
      m_IniFile.WriteInteger(PRIVATE_SECTION_NAME, DONT_SHOW_LAST_VERSION, iValue);
  else
    Assert(FALSE);
  end;
end;


function TIniSettings.FGetStringValue(ID: TIniSettingsID): string;
begin
  case ID of
    isidClock:
      Result := m_IniFile.ReadString(FGetCommonSectionName, CLOCK_KEY_NAME, INITIAL_CLOCK_TIME);
    isidExternalBaseName:
      Result := m_IniFile.ReadString(FGetCommonSectionName, EXTERNAL_BASE_NAME_KEY_NAME, '');
    isidAdjourned:
      Result := m_IniFile.ReadString(FGetCommonSectionName, ADJOURNED_KEY_NAME, '');
  else
    Result := '';
    Assert(FALSE);
  end;
end;


procedure TIniSettings.FSetStringValue(ID: TIniSettingsID; const strValue: string);
begin
  case ID of
    isidClock:
      m_IniFile.WriteString(FGetCommonSectionName, CLOCK_KEY_NAME, strValue);
    isidExternalBaseName:
      m_IniFile.WriteString(FGetCommonSectionName, EXTERNAL_BASE_NAME_KEY_NAME, strValue);
    isidAdjourned:
    begin
      m_IniFile.WriteString(FGetCommonSectionName, ADJOURNED_KEY_NAME, strValue);
      m_IniFile.UpdateFile;
    end;
  else
    Assert(FALSE);
  end;
end;


procedure TIniSettings.SetOpponentId(const strValue: string);
begin
  m_strOpponentId := strValue;
end;


function TIniSettings.HasCommonSettings: boolean;
begin
  Result := m_IniFile.SectionExists(FGetCommonSectionName);
end;


function TIniSettings.FGetCommonSectionName: string;
begin
  Result := COMMON_SECTION_PREFIX + ' ' + m_strOpponentId;
end;


function TIniSettings.FGetPlayerColor: TFigureColor;
begin
  Result := TFigureColor(m_IniFile.ReadInteger(
    FGetCommonSectionName, PLAYER_COLOR_KEY_NAME, Ord(fcBlack)))
end;


procedure TIniSettings.FSetPlayerColor(Value: TFigureColor);
begin
  m_IniFile.WriteInteger(FGetCommonSectionName, PLAYER_COLOR_KEY_NAME, Ord(Value));
end;

initialization

finalization
  TIniSettings.FreeInstance;

end.
