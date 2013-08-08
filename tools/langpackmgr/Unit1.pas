unit Unit1;

interface

uses
  Winapi.Windows, Winapi.Messages, System.SysUtils, System.Variants,
  System.Classes, Vcl.Graphics,
  Vcl.Controls, Vcl.Forms, Vcl.Dialogs, Vcl.Menus, Vcl.ExtDlgs, Vcl.StdCtrls,
  Vcl.CheckLst, Vcl.ComCtrls, Vcl.Buttons, ShellApi;

type
  TForm1 = class(TForm)

    SaveTextFileDialog1: TSaveTextFileDialog;

    MainMenu1: TMainMenu;
    File1: TMenuItem;
    Open1: TMenuItem;

    ListBox1: TListBox;
    ListBox2: TListBox;

    Button1: TButton;
    Button2: TButton;

    BitBtn1: TBitBtn;
    BitBtn2: TBitBtn;
    BitBtn3: TBitBtn;
    Edit1: TEdit;
    Label1: TLabel;
    Memo1: TMemo;
    Memo2: TMemo;

    procedure Open1Click(Sender: TObject); // процедура выбора файла =head=
    procedure FormCreate(Sender: TObject);
    procedure Button2Click(Sender: TObject); // переключатель
    procedure ListBox1Click(Sender: TObject);// выбор обрабатываемого файла
    procedure listfiles;// получение списка файлов в listbox
    procedure progress; // парсинг текущего файла перевода
    procedure parsing;// общий прогресс проверки перевода
    procedure ListBox2Click(Sender: TObject);
    procedure viewline;
    procedure BitBtn1Click(Sender: TObject);
    procedure BitBtn2Click(Sender: TObject);
    procedure Button1Click(Sender: TObject);
    procedure BitBtn3Click(Sender: TObject);
  private
    { Private declarations }

  public
    { Public declarations }
  end;

var
  Form1: TForm1;
  openDialog : TOpenDialog;
  openfile:textfile;

  stmp: string;                      // название  текущего файла перевода
  lang: string;                     // выбранный перевод
  sfull: array [0..6000] of string; // считывается выбранный перевод
  ifull: integer;                    // строк в выбранном переводе
  se: array [0..200,0..6000] of string; // английский файл перевода
  st: array [0..200,0..6000] of string; // строки перевода

  sfilter:array [1..30] of string;  // названия файлов основного перевода
  ifilter: integer;                 // количество файлов основного перевода
  bfilter: boolean;                 // флаг файла основного перевода

  n,t:integer;                      // всего строк/переведено строк
  il,ii,iindex:integer;                // счетчики
  newlines:boolean;

  adres:  array [0..6000]of integer;
  ilines: array [0..200] of integer; // строк в файле
  nlines: array [0..200] of integer; // всего строк для перевода
  tlines: array [0..200] of integer; // строк переведено

  lineindex: array [0..2000] of integer;

implementation

{$R *.dfm}

procedure TForm1.FormCreate(Sender: TObject);
begin
  assignfile(openfile,ExtractFilePath(Application.ExeName)+
  '\corebasic.txt',CP_UTF8);
  reset(openfile);
  readln(openfile);
  ifilter:=0;
  while not eof(openfile) do
    begin
      ifilter:=ifilter+1;
      readln(openfile,sfilter[ifilter]);
    end;
  listfiles;
end;

///////////////////////////////////////////////////////////////////////////////
procedure TForm1.ListBox1Click(Sender: TObject);
begin
  iindex:=ListBox1.ItemIndex;
  listbox2.Items.Clear;
  memo1.Lines.Clear;
  memo1.Lines.add('Plugin:'+listbox1.Items[iindex]);
  memo1.Lines.add('Lines:'+inttostr(tlines[iindex])
  +'/'+inttostr(nlines[iindex]));
  memo1.Lines.add('Completed:'
  +copy(floattostr(100*tlines[iindex]/nlines[iindex]),1,4)+'%');
  il:=-1;
  for ii:=0 to ilines[iindex] do
    begin
      if (copy(se[iindex,ii],1,1)='[') and (st[iindex,ii]='') then
        begin
          listbox2.Items.add(se[iindex,ii]);
          adres[listbox2.items.Count-1]:=ii;
          il:=il+1;
          lineindex[il]:=ii;
        end;
    end;
end;
///////////////////////////////////////////////////////////////////////////////
procedure TForm1.ListBox2Click(Sender: TObject);
var il,index:integer;
begin
  index:=strtoint(label1.Caption)-1;
  if index<>-1 then
  begin
    // запись строки
    if memo2.Lines.Count=memo1.Lines.Count then
     st[iindex,adres[index]]:='';
     il:=0;
     while il<memo2.Lines.Count do
     begin
       st[iindex,adres[index]]:=
       st[iindex,adres[index]]+memo2.lines[il];
       if il<>memo2.Lines.Count-1 then
       st[iindex,adres[index]]:=
       st[iindex,adres[index]]+'\n';
       il:=il+1;
     end;
  end;
  label1.Caption:=inttostr(listbox2.ItemIndex+1);
  memo1.Lines.Clear;
  memo2.Lines.Clear;
  viewline;
end;
///////////////////////////////////////////////////////////////////////////////
procedure TForm1.Open1Click(Sender: TObject);
begin
 chdir(ExtractFilePath(Application.ExeName));
 opendialog:=TOpenDialog.Create(self);
 opendialog.filter:='Langpack Head File|=HEAD=.txt';
 opendialog.initialDir:=GetCurrentDir;
 opendialog.options:=[ofFileMustExist];
 if (opendialog.execute) then
  begin
    lang:=copy(openDialog.filename,
    length(ExtractFilePath(Application.ExeName))+1,
    length(openDialog.filename)-
    length(ExtractFilePath(Application.ExeName))-11);
    chdir(ExtractFilePath(openDialog.filename));
    form1.Caption:='LangPackMgr: '+lang;
    ListBox1.enabled:=true;
    parsing;
  end;
end;
///////////////////////////////////////////////////////////////////////////////
procedure tform1.parsing;
begin
  memo1.lines.clear;
  listbox2.Items.clear;
  n:=0;t:=0;
  for iindex:=0 to listbox1.items.Count-1 do
    progress;
  if button2.Caption='Custom Plugins'
    then memo1.Lines.Add('Language:'+lang+' (Standart Bulid)')
    else memo1.Lines.Add('Language:'+lang+' (Custom Plugins)');
  memo1.Lines.Add('Translated:'+copy(floattostr(100*t/n),1,4)+'%');
end;
///////////////////////////////////////////////////////////////////////////////
procedure tform1.progress;
begin
  if copy(listbox1.Items[iindex],1,2)='=C' then
    stmp:='=CORE=' else
  if copy(listbox1.Items[iindex],1,2)='p|' then
    stmp:='\plugins\'+copy(listbox1.Items[iindex],3,
  length(listbox1.Items[iindex])-2) else
  if copy(listbox1.Items[iindex],1,2)='w|' then
    stmp:='\weather\'+copy(listbox1.Items[iindex],3,
  length(listbox1.Items[iindex])-2);
//
  for ifull := 0 to 4000 do sfull[ifull]:='';
    if fileexists(ExtractFilePath(Application.ExeName)
    +'\'+lang+'\'+stmp+'.txt') then
      begin
        assignfile(openfile,ExtractFilePath(Application.ExeName)
        +'\'+lang+'\'+stmp+'.txt',CP_UTF8);
        reset(openfile);
        ifull:=0;
    while not Eof(openfile) do
      begin
        ifull:=ifull+1;
        ReadLn(openfile,sfull[ifull]);
      end;
    closefile(openfile);
    end;
  for ii := 0 to 6000 do st[iindex,ii]:='';
  assignfile(openfile,ExtractFilePath(Application.ExeName)
  +'\english\'+stmp+'.txt',CP_UTF8);
  reset(openfile);
  ilines[iindex]:=-1;
  nlines[iindex]:=0;
  tlines[iindex]:=0;
  while not Eof(openfile) do
    begin
      ilines[iindex]:=ilines[iindex]+1;
      ReadLn(openfile,se[iindex,ilines[iindex]]);
    if (copy(se[iindex,ilines[iindex]],1,1)='[') then
      begin
        nlines[iindex]:= nlines[iindex]+1;
    for ii:=0 to ifull-1 do
      if se[iindex,ilines[iindex]]=sfull[ii] then
      if (copy(sfull[ii+1],1,1)<>';')
      and(copy(sfull[ii+1],1,1)<>'[')
      and(copy(sfull[ii+1],1,1)<>'')
      then
      begin
        st[iindex,ilines[iindex]]:=sfull[ii+1];
        tlines[iindex]:=tlines[iindex]+1;
      end;
    end;
  end;
  closefile(openfile);
  n:=n+nlines[iindex];
  t:=t+tlines[iindex];
end;
///////////////////////////////////////////////////////////////////////////////
procedure tform1.listfiles;
Var
  SR:TSearchRec;
  Res:Integer;
  i:integer;
begin
  n:=0;t:=0;
  ListBox1.Items.Clear;
  if button2.Caption='Custom Plugins' then
    begin
      Form1.ListBox1.Items.Add('=CORE=');
      for i := 1 to ifilter do
        Form1.ListBox1.Items.Add('p|'+sfilter[i]);
    end else
    begin
      // plugins
      Res:=FindFirst(ExtractFilePath(Application.ExeName)
      +'\english\plugins\*.txt', faAnyFile, SR);
      while Res = 0 do
        begin
          bfilter:=false;
            for i:=1 to ifilter do
              if sfilter[i]=copy(extractfilename(SR.Name),
              1,length(extractfilename(SR.Name))-4) then bfilter:=true;
            if bfilter=false then
              Form1.ListBox1.Items.Add('p|'+copy(extractfilename(SR.Name),
              1,length(extractfilename(SR.Name))-4));
            Res:=FindNext(SR);
        end;
      FindClose(SR);
      // weather
      Res:=FindFirst(ExtractFilePath(Application.ExeName)
      +'\english\weather\*.txt', faAnyFile, SR);
      while Res = 0 do
        begin
          Form1.ListBox1.Items.Add('w|'+copy(extractfilename(SR.Name),
          1,length(extractfilename(SR.Name))-4));
          Res:=FindNext(SR);
        end;
      end;
end;
// копирование шаблона/////////////////////////////////////////////////////////
procedure TForm1.BitBtn1Click(Sender: TObject);
begin
memo2.Lines:=memo1.Lines;
end;
// гугл //////////////////////////////////////////////////////////
procedure TForm1.BitBtn2Click(Sender: TObject);
var str:string; i:integer;
 begin
 str:='http://translate.google.com/?hl=&ie=&langpair=en#en/'+edit1.Text+'/';
for i:=0 to memo1.Lines.Count-1 do
begin
str:=str+memo1.Lines[i];
if i<memo1.Lines.Count-1 then
str:=str+'+%0A+';
end;

   ShellExecute(0, 'open',PChar(str), nil, nil, SW_SHOW);
end;

procedure TForm1.BitBtn3Click(Sender: TObject);
begin
 memo1.Lines.Clear;
 memo2.Lines.Clear;
 st[iindex,adres[listbox2.ItemIndex]]:='';
 viewline;
end;

// флаг переключения //////////////////////////////////////////////////////////


procedure TForm1.Button2Click(Sender: TObject);
begin
  if button2.Caption='Custom Plugins'
    then button2.Caption:='Core and basic'
    else button2.Caption:='Custom Plugins';
  listfiles;
  if  ListBox1.enabled=true then
    parsing;
end;

procedure tform1.viewline;
var im,m:integer;
sse:string;
begin
  m:=1; sse:=copy(se[iindex,lineindex[listbox2.itemindex]],2,
  length(se[iindex,lineindex[listbox2.itemindex]])-2);
      for im := 1 to length(sse)-1 do
        begin
          if copy(sse,im,2)='\n' then
          begin
            memo1.Lines.Add(copy(sse,m,im-m));
            m:=im+2;
          end;
        end;
    if m=1 then  memo1.Lines.Add(sse)
    else memo1.Lines.Add(copy(sse,m,length(sse)-m+2));

  if st[iindex,adres[listbox2.ItemIndex]]<>'' then
    begin
      m:=1;
      for im := 1 to length(st[iindex,adres[listbox2.ItemIndex]])-1 do
        begin
          if copy(st[iindex,adres[listbox2.ItemIndex]],im,2)='\n' then
          begin
            memo2.Lines.Add(copy(st[iindex,adres[listbox2.ItemIndex]],m,im-m));
            m:=im+2;
          end;
        end;
    if m=1 then  memo2.Lines.Add(st[iindex,adres[listbox2.ItemIndex]])
    else memo2.Lines.Add(copy(st[iindex,adres[listbox2.ItemIndex]],m,
    length(st[iindex,adres[listbox2.ItemIndex]])-m+2));
    end;
  memo2.SetFocus;
end;
///////////////////////////////////////////////////////////////////////////////
///

// Процедура сохранения обработанного файла
procedure TForm1.Button1Click(Sender: TObject);
var il,index:integer;
begin
  index:=strtoint(label1.Caption)-1;
  if index<>-1 then
  begin
    // запись строки
    if memo2.Lines.Count=memo1.Lines.Count then
     st[iindex,adres[index]]:='';
     il:=0;
     while il<memo2.Lines.Count do
     begin
       st[iindex,adres[index]]:=
       st[iindex,adres[index]]+memo2.lines[il];
       if il<>memo2.Lines.Count-1 then
       st[iindex,adres[index]]:=
       st[iindex,adres[index]]+'\n';
       il:=il+1;
     end;
  end;
memo1.Lines.Clear;
memo2.Lines.Clear;
 if copy(listbox1.Items[iindex],1,2)='=C' then
    stmp:='=CORE=' else
  if copy(listbox1.Items[iindex],1,2)='p|' then
    stmp:='\plugins\'+copy(listbox1.Items[iindex],3,
  length(listbox1.Items[iindex])-2) else
  if copy(listbox1.Items[iindex],1,2)='w|' then
    stmp:='\weather\'+copy(listbox1.Items[iindex],3,
  length(listbox1.Items[iindex])-2);
assignfile(openfile,ExtractFilePath(Application.ExeName)
        +'\'+lang+'\'+stmp+'.txt',CP_UTF8);
rewrite(openfile);
writeLn(openfile,se[iindex,0]);
 for ii := 1 to ilines[iindex] do
 begin
  if copy(se[iindex,ii],1,1)=';' then writeln(openfile,se[iindex,ii]);
  if (copy(se[iindex,ii],1,1)='[') and (st[iindex,ii]<>'')
  then
  begin
  writeln(openfile,se[iindex,ii]);
  writeln(openfile,st[iindex,ii]);
  end;
 end;
closefile(openfile);
//
  parsing;
end;
end.
