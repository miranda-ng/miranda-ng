using System;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO;
using System.Globalization;
using System.Resources;
using System.Net;

namespace Langpack_Suite
{
    public partial class MainForm : Form
    {
        private CultureInfo culture;
        public FolderBrowserDialog LangpackFolder;
        public String[] arguments;
        public bool vitype, quiet = false, outfile = false;
        public string output = "", exepath = ""; 
        Searcher search;
        ToolTip tipper;
        public MainForm()
        {
            arguments = Environment.GetCommandLineArgs();
            exepath = arguments[0];
            InitializeComponent();
            search = new Searcher(this);
            tipper = new ToolTip();
            tipper.AutoPopDelay = 5000;
            tipper.InitialDelay = 1000;
            tipper.ReshowDelay = 500;
            tipper.ShowAlways = true;

            DirectoryInfo root = Directory.GetParent(Directory.GetParent(exepath).ToString());
            string[] dirs = Directory.GetDirectories(root.ToString());
            foreach (string dir in dirs)
            {
                if (!dir.Equals(Directory.GetCurrentDirectory()) && File.Exists(dir + "\\=HEAD=.txt"))
                {
                    int pos = dir.LastIndexOf('\\');
                    string item = dir.Substring(pos + 1);
                    LangpacksComboBox.Items.Add(item);
                }
            }

            for (int i = 1; i < arguments.Length; i++)
            {
                string key = arguments[i].Substring(0, 2);
                if (key.Equals("\\q"))
                {
                    quiet = true;
                }
                if (key.Equals("\\n"))
                {
                    LangpackNameEdit.Text = arguments[i].Substring(2);
                    if (LangpackNameEdit.Text.Contains(":\\") || LangpackNameEdit.Text.Contains(".\\"))
                    {
                        output = arguments[i].Substring(2);
                        outfile = true;
                    }
                }
                if (key.Equals("\\o"))
                {
                    string fname = arguments[i].Substring(2);
                    if (fname.Contains("\\"))
                        OwnFileEdit.Text = fname;
                    else
                        OwnFileEdit.Text = Directory.GetParent(Directory.GetParent(exepath).ToString()).ToString() + "\\" + LangpacksComboBox.Text + "\\" + fname;
                    OwnFileCheckBox.Checked = true;
                    OwnFilesCheckBoxChange();
                }
                if (key.Equals("\\u"))
                {
                    string fname = arguments[i].Substring(2);
                    VIURLTextBox.Text = fname;
                    GetVICheckBox.Checked = true;
                    GetVICheckBoxChange();
                }
                if (key.Equals("\\p"))
                {
                    string fname = arguments[i].Substring(2);
                    LangpacksComboBox.Text = fname;
                    LangpacksComboBoxChange();
                }
            }
            if (quiet)
            {
                Hide();
                GenerateLang();
                Close();
                Environment.Exit(1);
            }
            culture = CultureInfo.CurrentCulture;
            if (culture.ToString() == "ru-RU")
            {
                LanguageComboBox.SelectedIndex = 1;
            }
            else if (culture.ToString() == "uk-UA")
            {
                LanguageComboBox.SelectedIndex = 2;
            }
            else if (culture.ToString() == "be-BY")
            {
                LanguageComboBox.SelectedIndex = 3;
            }
            else if (culture.ToString() == "de-DE")
            {
                LanguageComboBox.SelectedIndex = 4;
            }
			else if (culture.ToString() == "cs-CZ")
            {
                LanguageComboBox.SelectedIndex = 5;
            }
            else
            {
                culture = CultureInfo.CreateSpecificCulture("en-US");
                LanguageComboBox.SelectedIndex = 0;
            }
            adjustCulture();
        }

        private void adjustCulture()
        {
            ResourceManager rm = new ResourceManager("LangpackSuite.myRes", typeof(MainForm).Assembly);
            string CurrentText = "";
            CurrentText = rm.GetString("tipper", culture);
            tipper.SetToolTip(this.VarButton, CurrentText);
            CurrentText = rm.GetString("tabPage1", culture);
            tabPage1.Text = CurrentText;
            CurrentText = rm.GetString("groupBox2", culture);
            groupBox2.Text = CurrentText;
            CurrentText = rm.GetString("label1", culture);
            label1.Text = CurrentText;
            CurrentText = rm.GetString("GetVICheckBox", culture);
            GetVICheckBox.Text = CurrentText;
            CurrentText = rm.GetString("label6", culture);
            label6.Text = CurrentText;
            CurrentText = rm.GetString("OwnFileCheckBox", culture);
            OwnFileCheckBox.Text = CurrentText;
            CurrentText = rm.GetString("SelectOwnFileBtn", culture);
            SelectOwnFileBtn.Text = CurrentText;
            CurrentText = rm.GetString("CreateLangpackBtn", culture);
            CreateLangpackBtn.Text = CurrentText;
            CurrentText = rm.GetString("LanguageLbl", culture);
            label5.Text = CurrentText;
            CurrentText = rm.GetString("tabPage2", culture);
            tabPage2.Text = CurrentText;
            CurrentText = rm.GetString("label3", culture);
            label3.Text = CurrentText;
            CurrentText = rm.GetString("label4", culture);
            label4.Text = CurrentText;
            CurrentText = rm.GetString("LinkListGenBtn", culture);
            LinkListGenBtn.Text = CurrentText;
            CurrentText = rm.GetString("tabPage3", culture);
            tabPage3.Text = CurrentText;
            CurrentText = rm.GetString("groupBox1", culture);
            groupBox1.Text = CurrentText;
            CurrentText = rm.GetString("AssignLangBtn", culture);
            AssignLangBtn.Text = CurrentText;
            CurrentText = rm.GetString("label2", culture);
            label2.Text = CurrentText;
            CurrentText = rm.GetString("LangPathLbl", culture);
            string stmp = LangPathLbl.Text.ToUpper();
            if (!stmp.StartsWith("LANGPACK_"))
                LangPathLbl.Text = CurrentText;
            CurrentText = rm.GetString("groupBox5", culture);
            groupBox5.Text = CurrentText;
            CurrentText = rm.GetString("ManualModeBtn", culture);
            ManualModeBtn.Text = CurrentText;
            CurrentText = rm.GetString("AutoModeBtn", culture);
            AutoModeBtn.Text = CurrentText;
            CurrentText = rm.GetString("DupesFindBtn", culture);
            DupesFindBtn.Text = CurrentText;
            CurrentText = rm.GetString("NotTranslFindBtn", culture);
            NotTranslFindBtn.Text = CurrentText;
            CurrentText = rm.GetString("CommentFindBtn", culture);
            CommentFindBtn.Text = CurrentText;
            CurrentText = rm.GetString("AvailLangpacks", culture);
            label7.Text = CurrentText;
        }

        private void GenerateLang()
        {
            string s = "", st = "", we = "", FolderName = "";

            InfMessageLangBox.Text = "";
            ResourceManager rm = new ResourceManager("LangpackSuite.myRes", typeof(MainForm).Assembly);
            string LocaleText = rm.GetString("GenText", culture);

            if (VIURLTextBox.Text.Contains(":\\"))
                vitype = true;
            else
                vitype = false;

            InfMessageLangBox.Text = LocaleText + "\r\n";

            FolderName = Directory.GetParent(Directory.GetParent(exepath).ToString()).ToString() + "\\" + LangpacksComboBox.Text;
            if (!outfile)
                if (LangpackNameEdit.Text.Contains(":\\") || LangpackNameEdit.Text.Contains(".\\"))
                    output = LangpackNameEdit.Text;
                else
                    output = FolderName + "\\" + LangpackNameEdit.Text + ".txt";
                
            if (File.Exists(output))
                File.Delete(output);

            if (File.Exists(FolderName + "\\=HEAD=.txt"))
                File.Copy(FolderName + "\\=HEAD=.txt", output);
            else
            {
                LocaleText = rm.GetString("WrongSelect", culture);
                InfMessageLangBox.Text = InfMessageLangBox.Text + LocaleText + "\r\n";
                LocaleText = rm.GetString("HeadNotFound", culture);
                InfMessageLangBox.Text = InfMessageLangBox.Text + LocaleText + "\r\n";
                LocaleText = rm.GetString("GenStop", culture);
                InfMessageLangBox.Text = InfMessageLangBox.Text + LocaleText;
                return;
            }

            StreamWriter LangPack = new StreamWriter(output, true);
            StreamReader ReadFile;

            if (GetVICheckBox.Checked && VIURLTextBox.Text != "" && !vitype)
            {
                string FullURL = VIURLTextBox.Text.ToUpper();
                if (!FullURL.StartsWith("HTTP"))
                {
                    FullURL = "HTTP://" + FullURL;
                }
                WebClient wc = new WebClient();
                byte[] data = wc.DownloadData(FullURL.ToLower());
                byte[] allfile = new byte[data.Length + 3];
                allfile[0] = 239; allfile[1] = 187; allfile[2] = 191;
                data.CopyTo(allfile, 3);
                HtmlAgilityPack.HtmlDocument doc = new HtmlAgilityPack.HtmlDocument();
                string str = Encoding.UTF8.GetString(allfile);
                doc.LoadHtml(str);
                str = doc.DocumentNode.SelectSingleNode("//body").InnerText.Trim();
                FileStream _FileStream = new System.IO.FileStream("VersionInfo.txt", System.IO.FileMode.Create, System.IO.FileAccess.Write);
                allfile = Encoding.UTF8.GetBytes(str);
                _FileStream.Write(allfile, 0, allfile.Length);
                _FileStream.Close();
            }

            string vipath = "";
            if (vitype)
                vipath = VIURLTextBox.Text;
            else
                vipath = Directory.GetCurrentDirectory() + "\\VersionInfo.txt";

            if (File.Exists(vipath))
            {
                LocaleText = rm.GetString("VIGen", culture);
                InfMessageLangBox.Text = InfMessageLangBox.Text + LocaleText + "\r\n";
                LangPack.WriteLine("");
                string InclPlug = "Plugins-included: ";
                byte[] buffer = new byte[3];
                byte[] bom = { 239, 187, 191 };
                FileStream rf = new FileStream(vipath, FileMode.Open, FileAccess.Read);
                rf.Read(buffer, 0, 3);
                rf.Close();
                if (buffer[0] == bom[0] && buffer[1] == bom[1] && buffer[2] == bom[2])
                    ReadFile = new StreamReader(vipath);
                else
                    ReadFile = new StreamReader(vipath, Encoding.Default);
                while ((s = ReadFile.ReadLine()) != null)
                {
                    if (s.StartsWith("Active Plugins"))
                    {
                        s = ReadFile.ReadLine().Trim();
                        while (s != "")
                        {
                            if (s.StartsWith(" ") || s.Contains(".dll"))
                            {
                                int l = s.IndexOf(".");
                                if (s.StartsWith(" "))
                                    s = s.Substring(2, l - 2);
                                else
                                    s = s.Substring(0, l);
                                if (InclPlug == "Plugins-included: ")
                                    InclPlug = InclPlug + s;
                                else
                                    InclPlug = InclPlug + ", " + s;
                            }
                            s = ReadFile.ReadLine().Trim();
                        }
                    }
                    if (s.StartsWith("Unloadable Plugins"))
                    {
                        s = ReadFile.ReadLine().Trim();
                        while (s != "")
                        {
                            if (s.StartsWith(" ") || s.Contains("DbChecker.dll"))
                            {
                                int l = s.IndexOf(".");
                                if (s.StartsWith(" "))
                                    s = s.Substring(2, l - 2);
                                else
                                    s = s.Substring(0, l);
                                if (InclPlug == "Plugins-included: ")
                                    InclPlug = InclPlug + s;
                                else
                                    InclPlug = InclPlug + ", " + s;
                            }
                            s = ReadFile.ReadLine().Trim();
                        }
                        break;
                    }
                }
                LangPack.WriteLine(InclPlug);
                ReadFile.Close();

                if (OwnFileCheckBox.Checked && OwnFileEdit.Text != "")
                {
                    if (File.Exists(OwnFileEdit.Text))
                    {
                        LangPack.WriteLine("");
                        ReadFile = new StreamReader(OwnFileEdit.Text);
                        while ((s = ReadFile.ReadLine()) != null)
                        {
                            LangPack.WriteLine(s);
                        }
                        ReadFile.Close();
                    }
                    else
                    {
                        int f = OwnFileEdit.Text.LastIndexOf('\\');
                        string file = OwnFileEdit.Text.Substring(f + 1);
                        LocaleText = rm.GetString("FileText", culture);
                        string LocaleText2 = rm.GetString("FileNotFound", culture);
                        InfMessageLangBox.Text = InfMessageLangBox.Text + LocaleText + file + LocaleText2 + "\r\n";
                    }
                }

                LangPack.WriteLine("");
                if (File.Exists(FolderName + "\\=CORE=.txt"))
                {
                    ReadFile = new StreamReader(FolderName + "\\=CORE=.txt");
                    while ((s = ReadFile.ReadLine()) != null)
                    {
                        LangPack.WriteLine(s);
                    }
                    ReadFile.Close();
                }
                else
                {
                    LocaleText = rm.GetString("CoreNotFound", culture);
                    InfMessageLangBox.Text = InfMessageLangBox.Text + LocaleText + "\r\n";
                    LocaleText = rm.GetString("GenStop", culture);
                    InfMessageLangBox.Text = InfMessageLangBox.Text + LocaleText;
                    LangPack.Close();
                    ReadFile.Close();
                    return;
                }

                rf = new FileStream(vipath, FileMode.Open, FileAccess.Read);
                rf.Read(buffer, 0, 3);
                rf.Close();
                if (buffer[0] == bom[0] && buffer[1] == bom[1] && buffer[2] == bom[2])
                    ReadFile = new StreamReader(vipath);
                else
                    ReadFile = new StreamReader(vipath, Encoding.Default);
                while ((s = ReadFile.ReadLine()) != null)
                {
                    if (s.StartsWith("Active Plugins"))
                    {
                        s = ReadFile.ReadLine().Trim();
                        while (s != "")
                        {
                            StreamReader plug_in;
                            if (s.StartsWith(" ") || s.Contains(".dll"))
                            {
                                string back_s = s;
                                int l = s.IndexOf(".");
                                if (s.StartsWith(" "))
                                    s = s.Substring(2, l - 2);
                                else
                                    s = s.Substring(0, l);
                                string stmp = s.ToUpper();
                                if (stmp == "IMPORT_SA")
                                    s = "Import";
                                if (stmp == "WEATHER")
                                {
                                    LangPack.WriteLine("");
                                    if (File.Exists(FolderName + "\\plugins\\" + s + ".txt"))
                                    {
                                        plug_in = new StreamReader(FolderName + "\\plugins\\" + s + ".txt");
                                        while ((st = plug_in.ReadLine()) != null)
                                        {
                                            LangPack.WriteLine(st);
                                        }
                                        plug_in.Close();
                                    }
                                    else
                                    {
                                        LocaleText = rm.GetString("FileText", culture);
                                        string LocaleText2 = rm.GetString("File2NotFound", culture);
                                        InfMessageLangBox.Text = InfMessageLangBox.Text + LocaleText + s + LocaleText2 + "\r\n";
                                    }

                                    plug_in = new StreamReader(vipath);
                                    while ((st = plug_in.ReadLine()) != null)
                                    {
                                        if (st.StartsWith("Weather ini files:"))
                                        {
                                            st = plug_in.ReadLine();
                                            st = plug_in.ReadLine();
                                            if (st != "" && st != null)
                                                st = st.Trim();
                                            while (st != "" && st != null)
                                            {
                                                int w = st.IndexOf(".");
                                                st = st.Substring(0, w);
                                                LangPack.WriteLine("");
                                                if (File.Exists(FolderName + "\\weather\\" + st + ".txt"))
                                                {
                                                    StreamReader weather = new StreamReader(FolderName + "\\weather\\" + st + ".txt");
                                                    while ((we = weather.ReadLine()) != null)
                                                    {
                                                        LangPack.WriteLine(we);
                                                    }
                                                    weather.Close();
                                                }
                                                else
                                                {
                                                    LocaleText = rm.GetString("FileText", culture);
                                                    string LocaleText2 = rm.GetString("File2NotFound", culture);
                                                    InfMessageLangBox.Text = InfMessageLangBox.Text + LocaleText + st + LocaleText2 + "\r\n";
                                                }
                                                st = plug_in.ReadLine();
                                            }
                                        }
                                    }
                                    plug_in.Close();
                                    continue;
                                }
                                LangPack.WriteLine("");
                                if (File.Exists(FolderName + "\\plugins\\" + s + ".txt"))
                                {
                                    plug_in = new StreamReader(FolderName + "\\plugins\\" + s + ".txt");
                                    while ((st = plug_in.ReadLine()) != null)
                                    {
                                        LangPack.WriteLine(st);
                                    }
                                    plug_in.Close();
                                }
                                else
                                {
                                    LocaleText = rm.GetString("FileText", culture);
                                    string LocaleText2 = rm.GetString("File2NotFound", culture);
                                    InfMessageLangBox.Text = InfMessageLangBox.Text + LocaleText + s + LocaleText2 + "\r\n";
                                }
                            }
                            s = ReadFile.ReadLine().Trim();
                        }
                    }
                    if (s.StartsWith("Unloadable Plugins"))
                    {
                        s = ReadFile.ReadLine().Trim();
                        while (s != "")
                        {
                            StreamReader plug_in;
                            if (s.StartsWith(" ") || s.Contains(".dll"))
                            {
                                string back_s = s;
                                int l = s.IndexOf(".");
                                if (s.StartsWith(" "))
                                    s = s.Substring(2, l - 2);
                                else
                                    s = s.Substring(0, l);
                                LangPack.WriteLine("");
                                if (File.Exists(FolderName + "\\plugins\\" + s + ".txt"))
                                {
                                    plug_in = new StreamReader(FolderName + "\\plugins\\" + s + ".txt");
                                    while ((st = plug_in.ReadLine()) != null)
                                    {
                                        LangPack.WriteLine(st);
                                    }
                                    plug_in.Close();
                                }
                                else
                                {
                                    LocaleText = rm.GetString("FileText", culture);
                                    string LocaleText2 = rm.GetString("File2NotFound", culture);
                                    InfMessageLangBox.Text = InfMessageLangBox.Text + LocaleText + s + LocaleText2 + "\r\n";
                                }
                            }
                            s = ReadFile.ReadLine().Trim();
                        }
                        break;
                    }
                }
                ReadFile.Close();
                LangPack.Close();
            }
            else
            {
                LocaleText = rm.GetString("WOVIGen", culture);
                InfMessageLangBox.Text = InfMessageLangBox.Text + LocaleText + "\r\n";
                LangPack.WriteLine("");
                if (File.Exists(FolderName + "\\=VERSION=.txt"))
                {
                    ReadFile = new StreamReader(FolderName + "\\=VERSION=.txt");
                    while ((s = ReadFile.ReadLine()) != null)
                    {
                        LangPack.WriteLine(s);
                    }
                    ReadFile.Close();
                }
                else
                {
                    LocaleText = rm.GetString("VersionNotFound", culture);
                    InfMessageLangBox.Text = InfMessageLangBox.Text + LocaleText + "\r\n";
                }

                if (OwnFileCheckBox.Checked && OwnFileEdit.Text != "")
                {
                    if (File.Exists(OwnFileEdit.Text))
                    {
                        LangPack.WriteLine("");
                        ReadFile = new StreamReader(OwnFileEdit.Text);
                        while ((s = ReadFile.ReadLine()) != null)
                        {
                            LangPack.WriteLine(s);
                        }
                        ReadFile.Close();
                    }
                    else
                    {
                        int f = OwnFileEdit.Text.LastIndexOf('\\');
                        string file = OwnFileEdit.Text.Substring(f + 1);
                        LocaleText = rm.GetString("FileText", culture);
                        string LocaleText2 = rm.GetString("FileNotFound", culture);
                        InfMessageLangBox.Text = InfMessageLangBox.Text + LocaleText + file + LocaleText2 + "\r\n";
                    }
                }

                LangPack.WriteLine("");
                if (File.Exists(FolderName + "\\=CORE=.txt"))
                {
                    ReadFile = new StreamReader(FolderName + "\\=CORE=.txt");
                    while ((s = ReadFile.ReadLine()) != null)
                    {
                        LangPack.WriteLine(s);
                    }
                    ReadFile.Close();
                }
                else
                {
                    LocaleText = rm.GetString("CoreNotFound", culture);
                    InfMessageLangBox.Text = InfMessageLangBox.Text + LocaleText + "\r\n";
                    LocaleText = rm.GetString("GenStop", culture);
                    InfMessageLangBox.Text = InfMessageLangBox.Text + LocaleText;
                    LangPack.Close();
                    return;
                }

                DirectoryInfo PluginsDir = new DirectoryInfo(FolderName + "\\Plugins");
                foreach (FileInfo fi in PluginsDir.GetFiles())
                {
                    string stmp = fi.Name.ToUpper();
                    if (stmp == "WEATHER.TXT")
                    {
                        LangPack.WriteLine("");
                        ReadFile = new StreamReader(fi.FullName);
                        while ((s = ReadFile.ReadLine()) != null)
                        {
                            LangPack.WriteLine(s);
                        }
                        ReadFile.Close();

                        DirectoryInfo WeatherDir = new DirectoryInfo(FolderName + "\\Weather");
                        if (Directory.Exists(WeatherDir.ToString()))
                        {
                            foreach (FileInfo wi in WeatherDir.GetFiles())
                            {
                                LangPack.WriteLine("");
                                ReadFile = new StreamReader(wi.FullName);
                                while ((s = ReadFile.ReadLine()) != null)
                                {
                                    LangPack.WriteLine(s);
                                }
                                ReadFile.Close();
                            }
                        }
                        else
                        {
                            LocaleText = rm.GetString("WeatherLinkNotFound", culture);
                            InfMessageLangBox.Text = InfMessageLangBox.Text + LocaleText + "\r\n";
                        }
                        continue;
                    }
                    LangPack.WriteLine("");
                    ReadFile = new StreamReader(fi.FullName);
                    while ((s = ReadFile.ReadLine()) != null)
                    {
                        LangPack.WriteLine(s);
                    }
                    ReadFile.Close();
                }
                LangPack.Close();
            }

            if (!quiet)
            {
                LocaleText = rm.GetString("Done", culture);
                InfMessageLangBox.Text = InfMessageLangBox.Text + LocaleText;
                string LocaleText3 = rm.GetString("Inf", culture);
                MessageBox.Show(LocaleText, LocaleText3, MessageBoxButtons.OK, MessageBoxIcon.Information);
            }
        }

        private void OwnFilesCheckBoxChange()
        {
            if (OwnFileCheckBox.Checked)
            {
                OwnFileEdit.Enabled = true;
                SelectOwnFileBtn.Enabled = true;
            }
            else
            {
                OwnFileEdit.Enabled = false;
                SelectOwnFileBtn.Enabled = false;
            }
        }

        private void OwnFilesCheckBox_CheckedChanged(object sender, EventArgs e)
        {
            OwnFilesCheckBoxChange();
        }

        private void SelectOwnFilesBtn_Click(object sender, EventArgs e)
        {
            OpenFileDialog SelectOwnFilesOpenFileDialog = new OpenFileDialog();
            SelectOwnFilesOpenFileDialog.Filter = "txt files (*.txt)|*.txt|All files (*.*)|*.*";
            SelectOwnFilesOpenFileDialog.FilterIndex = 1;
            SelectOwnFilesOpenFileDialog.RestoreDirectory = true;
            SelectOwnFilesOpenFileDialog.InitialDirectory = Directory.GetParent(Directory.GetParent(exepath).ToString()).ToString() + "\\" + LangpacksComboBox.Text;
            if (SelectOwnFilesOpenFileDialog.ShowDialog() == DialogResult.OK)
            {
                try
                {
                    OwnFileEdit.Text = SelectOwnFilesOpenFileDialog.FileName;
                }
                catch (FileNotFoundException)
                {
                    if (!quiet)
                    {
                        ResourceManager rm = new ResourceManager("LangpackSuite.myRes", typeof(MainForm).Assembly);
                        string LocaleText = rm.GetString("NotOpen", culture);
                        string LocaleText2 = rm.GetString("Error", culture);
                        MessageBox.Show(LocaleText, LocaleText2, MessageBoxButtons.OK, MessageBoxIcon.Error);
                    }
                    return;
                }
            }
        }

        private void CreateLangpackBtn_Click(object sender, EventArgs e)
        {
            GenerateLang();
        }

        private void LinkListGenBtn_Click(object sender, EventArgs e)
        {
            string FolderName = "", SVNURL = "";
            InfMessageLinkBox.Text = "";
            ResourceManager rm = new ResourceManager("LangpackSuite.myRes", typeof(MainForm).Assembly);
            string LocaleText = rm.GetString("LinkListGen", culture);
            InfMessageLinkBox.Text = LocaleText + "\r\n";

            FolderName = Directory.GetParent(Directory.GetParent(exepath).ToString()).ToString() + "\\" + LangpacksComboBox.Text;
            if (File.Exists(FolderName + "\\LinkList.txt"))
                File.Delete(FolderName + "\\LinkList.txt");

            if (SVNURLEditBox.Text[SVNURLEditBox.Text.Length - 1] != '/')
                SVNURL = SVNURLEditBox.Text + "/";
            else
                SVNURL = SVNURLEditBox.Text;

            StreamWriter LinkList_out = new StreamWriter(FolderName + "\\LinkList.txt", false, Encoding.UTF8);
            DirectoryInfo RootDir = new DirectoryInfo(FolderName);
            LocaleText = rm.GetString("RootLink", culture);
            InfMessageLinkBox.Text = InfMessageLinkBox.Text + LocaleText + "\r\n";
            foreach (FileInfo fi in RootDir.GetFiles())
            {
                if (fi.Name.StartsWith("="))
                {
                    LinkList_out.WriteLine(SVNURL + fi.Name);
                }
            }

            LinkList_out.WriteLine(SVNURL + "LangpackSuite.exe");

            if (Directory.Exists(FolderName + "\\Plugins"))
            {
                DirectoryInfo PlugDir = new DirectoryInfo(FolderName + "\\Plugins");
                LocaleText = rm.GetString("PlugLink", culture);
                InfMessageLinkBox.Text = InfMessageLinkBox.Text + LocaleText + "\r\n";
                foreach (FileInfo fi in PlugDir.GetFiles())
                {
                    LinkList_out.WriteLine(SVNURL + "Plugins/" + fi.Name);
                }
            }
            else
            {
                LocaleText = rm.GetString("PlugLinkNotFound", culture);
                InfMessageLinkBox.Text = InfMessageLinkBox.Text + LocaleText + "\r\n";
            }

            if (Directory.Exists(FolderName + "\\Weather"))
            {
                DirectoryInfo WeatherDir = new DirectoryInfo(FolderName + "\\Weather");
                LocaleText = rm.GetString("WeatherLink", culture);
                InfMessageLinkBox.Text = InfMessageLinkBox.Text + LocaleText + "\r\n";
                foreach (FileInfo fi in WeatherDir.GetFiles())
                {
                    LinkList_out.WriteLine(SVNURL + "Weather/" + fi.Name);
                }
            }
            else
            {
                LocaleText = rm.GetString("WeatherLinkNotFound", culture);
                InfMessageLinkBox.Text = InfMessageLinkBox.Text + LocaleText + "\r\n";
            }

            LinkList_out.Close();

            if (!quiet)
            {
                LocaleText = rm.GetString("Done", culture);
                string LocaleText2 = rm.GetString("Inf", culture);
                InfMessageLinkBox.Text = InfMessageLinkBox.Text + LocaleText;
                MessageBox.Show(LocaleText, LocaleText2, MessageBoxButtons.OK, MessageBoxIcon.Information);
            }
        }

        private void AssignLangBtn_Click(object sender, EventArgs e)
        {
            string FolderName = "";
            InfMessageFindBox.Text = "";

            FolderName = Directory.GetParent(Directory.GetParent(exepath).ToString()).ToString() + "\\" + LangpacksComboBox.Text;
            DirectoryInfo RootDir = new DirectoryInfo(FolderName);
            ResourceManager rm = new ResourceManager("LangpackSuite.myRes", typeof(MainForm).Assembly);
            string LocaleText = rm.GetString("FindLang", culture);
            InfMessageFindBox.Text = InfMessageFindBox.Text + LocaleText + "\r\n";
            foreach (FileInfo fi in RootDir.GetFiles())
            {
                string stmp = fi.Name.ToUpper();
                if (stmp.StartsWith("LANGPACK_"))
                {
                    search.filename = fi.FullName;
                    LangPathLbl.Text = fi.Name;
                    DupesFindBtn.Enabled = true;
                    NotTranslFindBtn.Enabled = true;
                    CommentFindBtn.Enabled = true;
                    ManualModeBtn.Enabled = true;
                    ManualModeBtn.Checked = true;
                    search.mode = 1;
                    LocaleText = rm.GetString("AssignLang", culture);
                    InfMessageFindBox.Text = InfMessageFindBox.Text + LocaleText + "\r\n";
                    break;
                }
            }
            if (search.filename == null)
            {
                LocaleText = rm.GetString("LangNotFound", culture);
                InfMessageFindBox.Text = InfMessageFindBox.Text + LocaleText + "\r\n";
                LocaleText = rm.GetString("DupStop", culture);
                InfMessageFindBox.Text = InfMessageFindBox.Text + LocaleText;
                return;
            }

        }

        private void DupesFindBtn_Click(object sender, EventArgs e)
        {
            search.DupesSearch();
        }

        private void NotTranslFindBtn_Click(object sender, EventArgs e)
        {
            search.NotTransSearch();
        }

        private void CommentFindBtn_Click(object sender, EventArgs e)
        {
            search.CommentsSearch();
        }

        private void LanguageComboBox_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (LanguageComboBox.SelectedIndex == 0)
                culture = CultureInfo.CreateSpecificCulture("en-US");
            if (LanguageComboBox.SelectedIndex == 1)
                culture = CultureInfo.CreateSpecificCulture("ru-RU");
            if (LanguageComboBox.SelectedIndex == 2)
                culture = CultureInfo.CreateSpecificCulture("uk-UA");
            if (LanguageComboBox.SelectedIndex == 3)
                culture = CultureInfo.CreateSpecificCulture("be-BY");
            if (LanguageComboBox.SelectedIndex == 4)
                culture = CultureInfo.CreateSpecificCulture("de-DE");
			if (LanguageComboBox.SelectedIndex == 5)
                culture = CultureInfo.CreateSpecificCulture("cs-CZ");
            adjustCulture();

        }

        private void GetVICheckBoxChange()
        {
            if (GetVICheckBox.Checked)
            {
                VIURLTextBox.Enabled = true;
            }
            else
            {
                VIURLTextBox.Enabled = false;
            }
        }

        private void GetVICheckBox_CheckedChanged(object sender, EventArgs e)
        {
            GetVICheckBoxChange();
        }

        private void VarButton_Click(object sender, EventArgs e)
        {
            ResourceManager rm = new ResourceManager("LangpackSuite.myRes", typeof(MainForm).Assembly);
            string LocaleText = rm.GetString("VarText", culture);
            string LocaleHead = rm.GetString("VarHead", culture);
            MessageBox.Show(LocaleText, LocaleHead);
        }

        private void LangpacksComboBoxChange()
        {
            LangpackNameEdit.Text = "Langpack_" + LangpacksComboBox.Text;
            CreateLangpackBtn.Enabled = true;
        }

        private void LangpacksComboBox_SelectedIndexChanged(object sender, EventArgs e)
        {
            LangpacksComboBoxChange();
        }
    }
}