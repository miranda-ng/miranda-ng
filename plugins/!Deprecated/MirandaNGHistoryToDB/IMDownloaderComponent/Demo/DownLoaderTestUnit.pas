unit DownLoaderTestUnit;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, uIMDownLoader, StdCtrls, ToolWin, ComCtrls, Menus;

type
  TMainForm = class(TForm)
    IMDownloader_Demo: TIMDownloader;
    Edit1: TEdit;
    ToolBar1: TToolBar;
    StatusBar1: TStatusBar;
    RichEdit1: TRichEdit;
    SaveDialog1: TSaveDialog;
    TBDownload: TToolButton;
    TBStopDownload: TToolButton;
    TBView: TToolButton;
    ViewAsTest: TMenuItem;
    SaveToFile: TMenuItem;
    PopupMenu1: TPopupMenu;
    ProgressBar1: TProgressBar;
    procedure FormShow(Sender: TObject);
    procedure IMDownloader_DemoError(Sender: TObject; E: TIMDownLoadError);
    procedure IMDownloader_DemoAccepted(Sender: TObject);
    procedure IMDownloader_DemoStartDownload(Sender: TObject);
    procedure IMDownloader_DemoBreak(Sender: TObject);
    procedure ViewAsTestClick(Sender: TObject);
    procedure SaveToFileClick(Sender: TObject);
    procedure TBDownloadClick(Sender: TObject);
    procedure TBStopDownloadClick(Sender: TObject);
    procedure TBViewClick(Sender: TObject);
    procedure IMDownloader_DemoDownloading(Sender: TObject; AcceptedSize, MaxSize: Cardinal);
    procedure IMDownloader_DemoHeaders(Sender: TObject; Headers: string);
    procedure IMDownloader_DemoMD5Checked(Sender: TObject; MD5Correct,
      SizeCorrect: Boolean; MD5Str: string);
  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  MainForm: TMainForm;

const
  uURL = 'http://im-history.ru/update/get.php?file=HistoryToDB-Update';

implementation

{$R *.dfm}

procedure TMainForm.FormShow(Sender: TObject);
begin
  Edit1.Text := uURL;
  IMDownloader_Demo.DirPath := ExtractFilePath(Application.ExeName);
end;

procedure TMainForm.IMDownloader_DemoAccepted(Sender: TObject);
begin
  ProgressBar1.Visible := False;
  TBStopDownload.Visible := false;
  TBDownload.Visible := true;
  TBView.Visible := true;
  Edit1.ReadOnly := false;
  StatusBar1.SimpleText :=
    'Скачивание успешно завершено. Всего получено данных в байтах: ' + IntToStr
    (IMDownloader_Demo.AcceptedSize);
  //RichEdit1.Lines.Append('MD5 файла в памяти: '+MD5DigestToStr(MD5Stream(IMDownloader_Demo.OutStream)));
end;

procedure TMainForm.IMDownloader_DemoBreak(Sender: TObject);
begin
  ProgressBar1.Visible := False;
  TBStopDownload.Visible := false;
  TBStopDownload.Enabled := true;
  TBDownload.Visible := true;
  TBView.Visible := IMDownloader_Demo.AcceptedSize > 0;
  Edit1.ReadOnly := false;
  StatusBar1.SimpleText :=
    'Скачивание остановлено. Всего получено данных в байтах: ' + IntToStr
    (IMDownloader_Demo.AcceptedSize);
end;

procedure TMainForm.IMDownloader_DemoDownloading(Sender: TObject; AcceptedSize,
  MaxSize: Cardinal);
begin
  StatusBar1.SimpleText := 'Получено байт: ' + IntToStr(AcceptedSize);
  ProgressBar1.Visible := MaxSize > AcceptedSize;
  ProgressBar1.Max := MaxSize;
  ProgressBar1.Position := AcceptedSize;
end;

procedure TMainForm.IMDownloader_DemoError(Sender: TObject; E: TIMDownLoadError);
var
  s: string;
begin
  ProgressBar1.Visible := False;
  TBStopDownload.Visible := false;
  TBDownload.Visible := true;
  TBView.Visible := IMDownloader_Demo.AcceptedSize > 0;
  Edit1.ReadOnly := false;
  case E of
    deInternetOpen: s := 'Ошибка при открытии сессии. ';
    deInternetOpenUrl: s := 'Ошибка при запрашивании файла. ';
    deDownloadingFile: s := 'Ошибка при чтении файла. ';
    deRequest: s := 'Ошибка при запросе данных через прокси-сервер. ';
  end;
  StatusBar1.SimpleText :=
    s + 'Всего получено данных в байтах: ' + IntToStr
    (IMDownloader_Demo.AcceptedSize);
end;

procedure TMainForm.IMDownloader_DemoHeaders(Sender: TObject; Headers: string);
begin
  RichEdit1.Lines.Text := Headers;
end;

procedure TMainForm.IMDownloader_DemoMD5Checked(Sender: TObject; MD5Correct,
  SizeCorrect: Boolean; MD5Str: string);
begin
  if MD5Correct then
    RichEdit1.Lines.Append('Контрольная сумма MD5 = '+MD5Str+' - ВЕРНА!')
  else
    RichEdit1.Lines.Append('Контрольная сумма MD5 = '+MD5Str+' - НЕ ВЕРНА!');
  if SizeCorrect then
    RichEdit1.Lines.Append('Размер файла = '+IntToStr(IMDownloader_Demo.AcceptedSize)+' - ВЕРНЫЙ!')
  else
    RichEdit1.Lines.Append('Размер файла = '+IntToStr(IMDownloader_Demo.AcceptedSize)+' - НЕ ВЕРНЫЙ!');
end;

procedure TMainForm.IMDownloader_DemoStartDownload(Sender: TObject);
begin
  TBDownload.Visible := false;
  TBStopDownload.Visible := true;
  TBView.Visible := false;
  Edit1.ReadOnly := true;
  StatusBar1.SimpleText := 'Инициализация скачивания...';
end;

procedure TMainForm.ViewAsTestClick(Sender: TObject);
begin
  RichEdit1.Lines.LoadFromStream(IMDownloader_Demo.OutStream);
end;

procedure TMainForm.SaveToFileClick(Sender: TObject);
begin
  if Edit1.Text = uURL then
    SaveDialog1.FileName := 'HistoryToDBCreateDB.rar';
  if SaveDialog1.Execute then
    IMDownloader_Demo.OutStream.SaveToFile(SaveDialog1.FileName);
end;

procedure TMainForm.TBDownloadClick(Sender: TObject);
begin
  IMDownloader_Demo.URL := Edit1.Text;
  //IMDownloader_Demo.Proxy := '192.168.42.240:1522';
  IMDownloader_Demo.Proxy := '172.29.72.168:8080';
  IMDownloader_Demo.Download;
end;

procedure TMainForm.TBStopDownloadClick(Sender: TObject);
begin
  StatusBar1.SimpleText := 'Останавливаем скачку';
  TBStopDownload.Enabled := false;
  IMDownloader_Demo.BreakDownload;
end;

procedure TMainForm.TBViewClick(Sender: TObject);
begin
  TBView.DropdownMenu.Popup(TBView.ClientOrigin.X,
    TBView.ClientOrigin.Y + TBView.Height);
end;

end.
