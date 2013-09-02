; Демонстрационный скрипт ISWin7 v0.4.2
;
; Поддержка Windows Vista была некорретна вследствии чего была удалена
;
; Это тестовый скрипт позволяет сделать прозрачными края или всё окно целиком.
; Работает в Windows 7, Windows 8
;
; В Windows XP и Windows Vista он пропускает инициализацию
; что не мешает работоспособности сетапника в этих операционных системах
;
; Создатель: VoLT
;
; В дальнейшем развитии ожидается полная поддержка всех свистелок и перделок Windows 7
;
; Выражаю благодарность за тесты камраду s00p (трекер с nnm-club.ru)
; Хочу так же сказать спасибо группе R.G. ReCoding (трекер rustorka.com) за идеи и хорошую компанию

[Setup]
AppName=My Program
AppVerName=My Program version 1.5
DefaultDirName={pf}\My Program
DefaultGroupName=My Program
UninstallDisplayIcon={app}\MyProg.exe
Compression=lzma
SolidCompression=yes
OutputDir=.

[Files]
Source: ISWin7.dll; DestDir: {tmp}; Flags: dontcopy

         // Handle равен хэндлу окна WizardForm.Handle.
         // Параметры Left, Top, Right или Bottom задают рамку Стекла,
         // Если один из этих параметров равен -1 то стекло делается на всё окно
[Code]   
function win7_init(Handle:HWND; Left, Top, Right, Bottom : Integer): Boolean;
external 'win7_init@files:ISWin7.dll stdcall';

procedure win7_free;
external 'win7_free@files:ISWin7.dll stdcall';

procedure InitializeWizard();
begin
  // Для более красивого отображения уменьшаем нижнюю границу
  WizardForm.Bevel.Height := 1;
  // Инициализируем библиотеку
  if win7_init(WizardForm.Handle, 0, 0, 0, 47) then
  begin
    WizardForm.Caption := 'Включено';
  end
  else
  begin
    WizardForm.Caption := 'Выключено';
  end;
end;

procedure DeinitializeSetup();
begin
  // Отключаем библиотеку
  win7_free;
end;

procedure CurPageChanged(CurPageID: Integer);
begin
  WizardForm.ReadyPage.Hide;
end;




















