using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Windows.Forms;
using System.Globalization;
using System.Resources;

namespace Langpack_Suite
{
    class Searcher
    {
        private CultureInfo culture;
        List<string> str = new List<string>();
        List<string> dup = new List<string>();
        public string filename;
        public byte mode = 1;
        MainForm _mainForm;

        public Searcher(MainForm mainForm)
        {
            _mainForm = mainForm;
        }

        public void DupesSearch()
        {
            string s = "";
            int i = 0;
            str.Clear();
            dup.Clear();
            StreamReader fstr_in = new StreamReader(filename);
            while ((s = fstr_in.ReadLine()) != null)
            {
                Console.WriteLine(s);
                if (s.StartsWith("["))
                {
                    str.Add(s);
                    i += 1;
                }
                else if ((s.StartsWith(";")) && (s.Contains("[")))
                {
                    int q = s.IndexOf("[");
                    s = s.Remove(0, q);
                    int qq = s.LastIndexOf("]");
                    s += " ";
                    s = s.Remove(qq + 1);
                    str.Add(s);
                }
            }
            fstr_in.Close();
            for (i = 0; i <= str.Count - 1; i++)
            {
                for (int j = 0; j <= str.Count - 1; j++)
                {
                    if ((str[i].Equals(str[j])) && (i != j))
                    {
                        if (dup.Count == 0)
                        {
                            dup.Add(str[i]);
                        }
                        else
                        {
                            int k = 0;
                            for (int z = 0; z <= dup.Count - 1; z++)
                            {
                                if (str[i].Equals(dup[z]))
                                {
                                    k += 1;
                                }
                            }
                            if (k == 0)
                            {
                                dup.Add(str[i]);
                            }
                        }
                    }
                }
            }
            ResourceManager rm;
            string LocaleText;
            if (_mainForm.LanguageComboBox.SelectedIndex == 0)
                culture = CultureInfo.CreateSpecificCulture("en-EN");
            if (_mainForm.LanguageComboBox.SelectedIndex == 1)
                culture = CultureInfo.CreateSpecificCulture("ru-RU");
            if (dup.Count > 0)
            {
                rm = new ResourceManager("LangpackSuite.myRes", typeof(MainForm).Assembly);
                LocaleText = rm.GetString("DupFound", culture);
                _mainForm.InfMessageFindBox.Text = _mainForm.InfMessageFindBox.Text + LocaleText + "\r\n";
                if (mode == 1)
                {
                    StreamWriter fstr_out = new StreamWriter(filename + "_dupes");
                    for (i = 0; i <= dup.Count - 1; i++)
                    {
                        try
                        {
                            fstr_out.Write(dup[i] + "\r\n");
                        }
                        catch (IOException)
                        {
                            LocaleText = rm.GetString("NotWrite", culture);
                            string LocaleText2 = rm.GetString("Error", culture);
                            MessageBox.Show(LocaleText, LocaleText2, MessageBoxButtons.OK, MessageBoxIcon.Error);
                            return;
                        }
                    }
                    fstr_out.Close();
                    LocaleText = rm.GetString("Done", culture);
                    _mainForm.InfMessageFindBox.Text = _mainForm.InfMessageFindBox.Text + LocaleText + "\r\n";
                    string LocaleText3 = rm.GetString("Inf", culture);
                    MessageBox.Show(LocaleText, LocaleText3, MessageBoxButtons.OK, MessageBoxIcon.Information);
                }
                else
                {
                    //автоматический поиск и удаление дублей
                }
            }
            else
            {
                rm = new ResourceManager("LangpackSuite.myRes", typeof(MainForm).Assembly);
                LocaleText = rm.GetString("DupNotFound", culture);
                _mainForm.InfMessageFindBox.Text = _mainForm.InfMessageFindBox.Text + LocaleText + "\r\n";
                string LocaleText3 = rm.GetString("Inf", culture);
                MessageBox.Show(LocaleText, LocaleText3, MessageBoxButtons.OK, MessageBoxIcon.Information);
            }
        }

        public void NotTransSearch()
        {
            string s = "", ss = "";
            str.Clear();
            dup.Clear();
            StreamReader fstr_in = new StreamReader(filename);
            while ((s = fstr_in.ReadLine()) != null)
            {
                Console.WriteLine(s);
                str.Add(s);
            }
            fstr_in.Close();
            for (int i = 0; i <= str.Count - 1; i++)
            {
                if (i == str.Count - 1)
                {
                    s = str[i].ToString();
                    if (s.StartsWith("["))
                    {
                        dup.Add(str[i]);
                    }
                }
                else
                {
                    s = str[i].ToString();
                    ss = str[i + 1].ToString();
                    if ((s.StartsWith("[")) && (ss.StartsWith("[")))
                    {
                        dup.Add(str[i]);
                    }
                }
            }
            ResourceManager rm;
            string LocaleText;
            if (_mainForm.LanguageComboBox.SelectedIndex == 0)
                culture = CultureInfo.CreateSpecificCulture("en-EN");
            if (_mainForm.LanguageComboBox.SelectedIndex == 1)
                culture = CultureInfo.CreateSpecificCulture("ru-RU");
            if (dup.Count > 0)
            {
                rm = new ResourceManager("LangpackSuite.myRes", typeof(MainForm).Assembly);
                LocaleText = rm.GetString("NotTransFound", culture);
                _mainForm.InfMessageFindBox.Text = _mainForm.InfMessageFindBox.Text + LocaleText + "\r\n";
                if (mode == 1)
                {
                    StreamWriter fstr_out = new StreamWriter(filename + "_ntrans");
                    for (int i = 0; i <= dup.Count - 1; i++)
                    {
                        try
                        {
                            fstr_out.Write(dup[i] + "\r\n");
                        }
                        catch (IOException)
                        {
                            LocaleText = rm.GetString("NotWrite", culture);
                            string LocaleText2 = rm.GetString("Error", culture);
                            MessageBox.Show(LocaleText, LocaleText2, MessageBoxButtons.OK, MessageBoxIcon.Error);
                            return;
                        }
                    }
                    fstr_out.Close();
                    LocaleText = rm.GetString("Done", culture);
                    _mainForm.InfMessageFindBox.Text = _mainForm.InfMessageFindBox.Text + LocaleText + "\r\n";
                    string LocaleText3 = rm.GetString("Inf", culture);
                    MessageBox.Show(LocaleText, LocaleText3, MessageBoxButtons.OK, MessageBoxIcon.Information);
                }
                else
                {
                    //автоматический поиск и удаление не переведенных строк
                }
            }
            else
            {
                rm = new ResourceManager("LangpackSuite.myRes", typeof(MainForm).Assembly);
                LocaleText = rm.GetString("NotTransNotFound", culture);
                _mainForm.InfMessageFindBox.Text = _mainForm.InfMessageFindBox.Text + LocaleText + "\r\n";
                string LocaleText3 = rm.GetString("Inf", culture);
                MessageBox.Show(LocaleText, LocaleText3, MessageBoxButtons.OK, MessageBoxIcon.Information);
            }
        }
        public void CommentsSearch()
        {
            string s = "";
            str.Clear();
            dup.Clear();
            StreamReader fstr_in = new StreamReader(filename);
            while ((s = fstr_in.ReadLine()) != null)
            {
                Console.WriteLine(s);
                str.Add(s);
            }
            fstr_in.Close();
            for (int i = 0; i <= str.Count - 1; i++)
            {
                s = str[i].ToString();
                if ((s.StartsWith(";")) && (s.Contains("[")))
                {
                    dup.Add(str[i]);
                }
            }
            ResourceManager rm;
            string LocaleText;
            if (_mainForm.LanguageComboBox.SelectedIndex == 0)
                culture = CultureInfo.CreateSpecificCulture("en-EN");
            if (_mainForm.LanguageComboBox.SelectedIndex == 1)
                culture = CultureInfo.CreateSpecificCulture("ru-RU");
            if (dup.Count > 0)
            {
                rm = new ResourceManager("LangpackSuite.myRes", typeof(MainForm).Assembly);
                LocaleText = rm.GetString("ComFound", culture);
                _mainForm.InfMessageFindBox.Text = _mainForm.InfMessageFindBox.Text + LocaleText + "\r\n";
                if (mode == 1)
                {
                    StreamWriter fstr_out = new StreamWriter(filename + "_com");
                    for (int i = 0; i <= dup.Count - 1; i++)
                    {
                        try
                        {
                            fstr_out.Write(dup[i] + "\r\n");
                        }
                        catch (IOException)
                        {
                            LocaleText = rm.GetString("NotWrite", culture);
                            string LocaleText2 = rm.GetString("Error", culture);
                            MessageBox.Show(LocaleText, LocaleText2, MessageBoxButtons.OK, MessageBoxIcon.Error);
                            return;
                        }
                    }
                    fstr_out.Close();
                    LocaleText = rm.GetString("Done", culture);
                    _mainForm.InfMessageFindBox.Text = _mainForm.InfMessageFindBox.Text + LocaleText + "\r\n";
                    string LocaleText3 = rm.GetString("Inf", culture);
                    MessageBox.Show(LocaleText, LocaleText3, MessageBoxButtons.OK, MessageBoxIcon.Information);
                }
                else
                {
                    //автоматический поиск и удаление закомментированных строк
                }
            }
            else
            {
                rm = new ResourceManager("LangpackSuite.myRes", typeof(MainForm).Assembly);
                LocaleText = rm.GetString("ComNotFound", culture);
                _mainForm.InfMessageFindBox.Text = _mainForm.InfMessageFindBox.Text + LocaleText + "\r\n";
                string LocaleText3 = rm.GetString("Inf", culture);
                MessageBox.Show(LocaleText, LocaleText3, MessageBoxButtons.OK, MessageBoxIcon.Information);
            }
        }
    }
}

