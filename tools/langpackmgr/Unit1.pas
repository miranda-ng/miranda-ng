unit Unit1;

interface

uses
  Winapi.Windows, Winapi.Messages, System.SysUtils, System.Variants, System.Classes, Vcl.Graphics,
  Vcl.Controls, Vcl.Forms, Vcl.Dialogs, Vcl.StdCtrls, Vcl.Buttons, ShellApi;

type
  TForm1 = class(TForm)
    ListBox1: TListBox;
    Memo1: TMemo;
    Memo2: TMemo;
    Button1: TButton;
    Button2: TButton;
    Button3: TButton;
    ComboBox1: TComboBox;
    procedure ListBox12Click(Sender: TObject);
    procedure FormCreate(Sender: TObject);
    procedure ListBox1Click(Sender: TObject);
    procedure langlist;
    procedure fileslist;
    procedure stringview;
    procedure savestring;
    procedure Button2Click(Sender: TObject);
    procedure Button3Click(Sender: TObject);
    procedure Button1Click(Sender: TObject);

  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  Form1: TForm1;
  locale:string;
  z,j,i,qq,l:integer;
  sr:tsearchrec;
  res:integer;
  s:array[0..1,1..6000] of string;
  u:array[1..6000] of integer;
  first:string;
  filename:string;
implementation

{$R *.dfm}

procedure TForm1.Button1Click(Sender: TObject);
begin
memo2.lines:=memo1.lines;
end;

procedure TForm1.Button2Click(Sender: TObject);
var str:string; i:integer;
 begin
 str:='http://translate.google.com/?hl=&ie=yhgg#auto/';
 if  locale='belarusian' then str:=str+'be/';
 if  locale='bulgarian' then str:=str+'bg/';
 if  locale='chinese' then str:=str+'zh-CN/';
 if  locale='czech' then str:=str+'cs/';
 if  locale='duth' then str:=str+'da/';
 if  locale='estonian' then str:=str+'et/';
 if  locale='french' then str:=str+'fr/';
 if  locale='german' then str:=str+'de/';
 if  locale='herbrew' then str:=str+'iw/';
 if  locale='hungarian' then str:=str+'hu/';
 if  locale='italian' then str:=str+'it/';
 if  locale='japanese' then str:=str+'ja/';
 if  locale='korean' then str:=str+'ko/';
 if  locale='norwegian' then str:=str+'no/';
 if  locale='polish' then str:=str+'pl/';
 if  locale='portuguese_br' then str:=str+'pt/';
 if  locale='russian' then str:=str+'ru/';
 if  locale='slovak' then str:=str+'sk/';
 if  locale='spanish' then str:=str+'es/';
 if  locale='turkish' then str:=str+'tr/';
 if  locale='ukrainian' then str:=str+'uk/';
 for i:=0 to memo1.Lines.Count-1 do
    begin
    str:=str+memo1.Lines[i];
    if i<memo1.Lines.Count-1 then
    str:=str+'+%0A+';
    end;
 ShellExecute(0, 'open',PChar(str), nil, nil, SW_SHOW);
end;

procedure TForm1.Button3Click(Sender: TObject);
var sList:TStringList;
  begin
    savestring;
    sList:=TstringList.Create;
    sList.add(first);
    for i := 1 to j do
      begin
        if copy(s[0,i],1,1)=';' then
          sList.add(s[0,i]);
        if s[1,i]<>'' then
          begin
            sList.add(s[0,i]);
            sList.add(s[1,i]);
          end;
      end;
  sList.SavetoFile(extractfilePath(application.exename)
  +'/'+locale+'/'+filename+'.txt',TEncoding.UTF8);
  sList.free;
end;

procedure tform1.fileslist;
begin
  ListBox1.Items.Add('=CORE=');
  Res:=FindFirst(ExtractFilePath(Application.ExeName)
  +'\english\plugins\*.txt', faAnyFile, SR);
  while Res = 0 do
    begin
      listBox1.Items.Add('plugins\'+copy(extractfilename(SR.Name),1,
      length(extractfilename(SR.Name))-4));
      Res:=FindNext(SR);
    end;
  FindClose(SR);
  Res:=FindFirst(ExtractFilePath(Application.ExeName)
  +'\english\weather\*.txt', faAnyFile, SR);
  while Res = 0 do
    begin
      listBox1.Items.Add('weather\'+copy(extractfilename(SR.Name),1,
      length(extractfilename(SR.Name))-4));
      Res:=FindNext(SR);
    end;
  FindClose(SR);
end;

procedure tform1.langlist;
  begin
    res:=findfirst(extractfilePath(application.exename)+'*',faanyfile,SR);
    while Res = 0 do
      begin
        if ((SR.attr and fadirectory)=fadirectory)
        and ((SR.name='.')or(SR.name='..')) then
          begin res:=findnext(SR); continue; end;
        if ((SR.attr and fadirectory)=fadirectory)
        and (fileexists(extractfilepath(application.exename)+
        '/'+SR.name+'/=HEAD=.txt')) and (sr.name<>'english') then
          ListBox1.Items.Add(SR.name);
        Res:=FindNext(SR);
      end;
    FindClose(SR);
  end;

procedure tform1.savestring;
var sa:integer;
begin
 s[1,u[l]]:='';
 for sa:=0 to memo2.Lines.Count-1 do
   begin
     s[1,u[l]]:=s[1,u[l]]+memo2.Lines[sa];
     if sa<>memo2.Lines.Count-1 then  s[1,u[l]]:=s[1,u[l]]+'\n';
   end;
end;


procedure tform1.stringview;
var
im,m:integer;
sse:string;
  begin
    sse:=copy(s[0,u[listbox1.itemindex]],2,
    length(s[0,u[listbox1.itemindex]])-2);
    m:=1;
    for im := 1 to length(sse)-1 do
        begin
          if copy(sse,im,2)='\n' then
          begin
            memo1.Lines.Add(copy(sse,m,im-m));
            m:=im+2;
          end;
        end;
    memo2.Lines.Clear;
    l:=listbox1.itemindex;
    if m=1 then  memo1.Lines.Add(sse)
    else memo1.Lines.Add(copy(sse,m,length(sse)-m+2));
   if s[1,u[listbox1.itemindex]]<>'' then
    begin
      m:=1;
      for im := 1 to length(s[1,u[listbox1.itemindex]])-1 do
        begin
          if copy(s[1,u[listbox1.itemindex]],im,2)='\n' then
          begin
            memo2.Lines.Add(copy(s[1,u[listbox1.itemindex]],m,im-m));
            m:=im+2;
          end;
        end;
    if m=1 then  memo2.Lines.Add(s[1,u[listbox1.itemindex]])
    else memo2.Lines.Add(copy(s[1,u[listbox1.itemindex]],m,
    length(s[1,u[listbox1.itemindex]])-m+2));
    end;
   end;

procedure TForm1.FormCreate(Sender: TObject);
  begin
    langlist;
    qq:=0;
  end;

procedure TForm1.ListBox1Click(Sender: TObject);
  begin
    if qq=0 then
      begin
      if listbox1.itemindex<>-1 then
        begin
        memo1.Lines.Clear;
        memo1.Lines.LoadFromFile(extractfilePath(application.exename)
        +'/'+listbox1.items[listbox1.itemindex]+'/=HEAD=.txt');
        end;
      end;
    if (qq=2) and (listbox1.itemindex>0)then
      begin
        savestring;
        {}
        memo1.Lines.clear;
        memo2.Lines.clear;
        stringview;
      end;
  end;

procedure TForm1.ListBox12Click(Sender: TObject);
var llist:TStringList;
f:textfile;
begin
case qq of

  0:begin
    ListBox1.height:=300;
    Combobox1.Visible:=true;
    Memo1.Lines.Clear;
    Memo1.Height:=150;
    Memo2.Visible:=true;
    Button1.Visible:=true;
    Button2.Visible:=true;
    Button3.Visible:=true;
    locale:=listbox1.items[listbox1.itemindex];
    listbox1.items.clear;
    form1.Caption:='Miranda NG Langpack Manager: '+locale;
    listbox1.items.add('..');
    fileslist;
    qq:=1;
  end;

  1:begin
    if listbox1.items[listbox1.itemindex]='..' then
      begin
        ListBox1.height:=343;
        Combobox1.Visible:=false;
        ListBox1.Items.clear;
        Memo1.Lines.Clear;
        Memo1.Height:=343;
        Memo2.Visible:=false;
        Button1.Visible:=false;
        Button2.Visible:=false;
        Button3.Visible:=false;
        langlist;
        listbox1.itemindex:=0;
        form1.Caption:='Miranda NG Langpack Manager';
        qq:=0;
      end
    else
      begin
        for i := 1 to 6000 do
          begin s[0,i]:='';s[1,i]:='';end;
        j:=0;
        lList:=TstringList.Create;
        lList.LoadFromFile(extractfilePath(application.exename)+'/english/'
        +listbox1.items[listbox1.itemindex]+'.txt',TEncoding.UTF8);
        first:=lList[0];
        for i := 1 to lList.Count-1 do
            begin
              j:=j+1;
              s[0,j]:=lList[i];
            end;
 //
      if not FileExists(extractfilePath(application.exename)
      +'/'+locale+'/'+listbox1.items[listbox1.itemindex]+'.txt')
      then
      begin
      assignfile(f,extractfilePath(application.exename)
      +'/'+locale+'/'+listbox1.items[listbox1.itemindex]+'.txt');
      rewrite(f);
      closefile(f);
      end;
//
        lList.LoadFromFile(extractfilePath(application.exename)+'/'+locale+'/'
        +listbox1.items[listbox1.itemindex]+'.txt',TEncoding.UTF8);
        for z:= 1 to j do
          for i := 1 to lList.count-2 do
            if (copy(s[0,z],1,1)='[')and(s[0,z]=lList[i]) then
              s[1,z]:=lList[i+1];
        lList.free;
        form1.Caption:='Miranda NG Langpack Manager - '
        +locale+':'+listbox1.items[listbox1.itemindex];
        filename:=listbox1.items[listbox1.itemindex];
        listbox1.items.Clear;
        listbox1.items.add('..');
        z:=0;
        for i := 1 to j do
          if (copy(s[0,i],1,1)='[')and(s[1,i]='') then
            begin
              z:=z+1;
              u[z]:=i;
              listbox1.items.add(copy(s[0,i],2,length(s[0,i])-2));
          end;

        qq:=2;
      end;
  end;

  2:begin
    if listbox1.items[listbox1.itemindex]='..' then
      begin
        listbox1.Items.Clear;
        listbox1.Items.Add('..');
        fileslist;
        memo1.Lines.Clear;
        memo2.Lines.Clear;
        form1.Caption:='Miranda NG Langpack Manager - '+locale;
        qq:=1;
      end;
  end;
end;
end;
end.
