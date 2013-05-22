{ ############################################################################ }
{ #                                                                          # }
{ #  MirandaNG HistoryToDB Plugin v2.5                                       # }
{ #                                                                          # }
{ #  License: GPLv3                                                          # }
{ #                                                                          # }
{ #  Author: Grigorev Michael (icq: 161867489, email: sleuthhound@gmail.com) # }
{ #                                                                          # }
{ ############################################################################ }

unit About;

interface

uses Windows, Messages, SysUtils, Classes, Graphics, Forms, Controls, StdCtrls,
  Buttons, ExtCtrls, ShellAPI, Global, ComCtrls;

type
  TAboutForm = class(TForm)
    AboutImage: TImage;
    CloseButton: TButton;
    AboutPageControl: TPageControl;
    VersionTabSheet: TTabSheet;
    ThankYouTabSheet: TTabSheet;
    BAbout: TBevel;
    LProgramName: TLabel;
    LCopyright: TLabel;
    LabelAuthor: TLabel;
    LVersionNum: TLabel;
    LVersion: TLabel;
    LLicense: TLabel;
    LLicenseType: TLabel;
    LWeb: TLabel;
    LabelWebSite: TLabel;
    BThankYou: TBevel;
    ThankYou: TLabel;
    procedure FormCreate(Sender: TObject);
    procedure FormShow(Sender: TObject);
    procedure FormClose(Sender: TObject; var Action: TCloseAction);
    procedure CloseButtonClick(Sender: TObject);
    procedure LabelAuthorClick(Sender: TObject);
    procedure LabelWebSiteClick(Sender: TObject);
    procedure MemoThankYouEnter(Sender: TObject);
  private
    { Private declarations }
    // Для мультиязыковой поддержки
    procedure OnLanguageChanged(var Msg: TMessage); message WM_LANGUAGECHANGED;
    procedure LoadLanguageStrings;
  public
    { Public declarations }
  end;

var
  AboutForm: TAboutForm;

implementation

{$R *.dfm}
{$R icons.res}

procedure TAboutForm.CloseButtonClick(Sender: TObject);
begin
  Close;
end;

procedure TAboutForm.FormClose(Sender: TObject; var Action: TCloseAction);
begin
  // Переменная для режима анти-босс
  Global_AboutForm_Showing := False;
end;

procedure TAboutForm.FormCreate(Sender: TObject);
var
  AboutBitmap: TBitmap;
begin
  // Для мультиязыковой поддержки
  AboutFormHandle := Handle;
  SetWindowLong(Handle, GWL_HWNDPARENT, 0);
  SetWindowLong(Handle, GWL_EXSTYLE, GetWindowLong(Handle, GWL_EXSTYLE) or WS_EX_APPWINDOW);
  // Грузим битовый образы из файла ресурсов
  AboutBitmap := TBitmap.Create;
  try
    AboutBitmap.LoadFromResourceName(HInstance, 'About');
    AboutImage.Picture.Assign(AboutBitmap);
  finally
    AboutBitmap.Free;
  end;
  LabelAuthor.Cursor := crHandPoint;
  LabelWebSite.Cursor := crHandPoint;
  // Загружаем язык интерфейса
  LoadLanguageStrings;
end;

procedure TAboutForm.FormShow(Sender: TObject);
begin
  // Переменная для режима анти-босс
  Global_AboutForm_Showing := True;
  // Указываем версию в окне "О плагине"
  LVersionNum.Caption := IntToStr(htdVerMajor) + '.' + IntToStr(htdVerMinor) + '.' + IntToStr(htdVerRelease) + '.' + IntToStr(htdVerBuild) + ' ' + htdPlatform;
end;

procedure TAboutForm.LabelAuthorClick(Sender: TObject);
begin
  ShellExecute(0, 'open', 'mailto:sleuthhound@gmail.com', nil, nil, SW_RESTORE);
end;

procedure TAboutForm.LabelWebSiteClick(Sender: TObject);
begin
  ShellExecute(0, 'open', 'http://www.im-history.ru', nil, nil, SW_RESTORE);
end;

// Мега-хак для запрета выделения в Memo :-D
procedure TAboutForm.MemoThankYouEnter(Sender: TObject);
begin
  CloseButton.SetFocus;
end;

// Для мультиязыковой поддержки
procedure TAboutForm.OnLanguageChanged(var Msg: TMessage);
begin
  LoadLanguageStrings;
end;

// Для мультиязыковой поддержки
procedure TAboutForm.LoadLanguageStrings;
begin
  Caption := GetLangStr('AboutFormCaption');
  LProgramName.Caption := htdPluginShortName;
  CloseButton.Caption := GetLangStr('CloseButton');
  LVersion.Caption := GetLangStr('Version');
  LLicense.Caption := GetLangStr('License');
  VersionTabSheet.Caption := GetLangStr('AboutFormCaption');
  ThankYouTabSheet.Caption := GetLangStr('LThankYou');
  // Позиционируем лейблы
  LVersionNum.Left := LVersion.Left + 1 + LVersion.Width;
  LLicenseType.Left := LLicense.Left + 1 + LLicense.Width;
  // Благодарности
  if CoreLanguage = 'Russian' then
    ThankYou.Caption := ThankYouText_Rus
  else
    ThankYou.Caption := ThankYouText_Eng;
  // End
end;

end.
 
