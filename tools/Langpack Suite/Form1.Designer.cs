namespace Langpack_Suite
{
    partial class MainForm
    {
        /// <summary>
        /// Требуется переменная конструктора.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Освободить все используемые ресурсы.
        /// </summary>
        /// <param name="disposing">истинно, если управляемый ресурс должен быть удален; иначе ложно.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Код, автоматически созданный конструктором форм Windows

        /// <summary>
        /// Обязательный метод для поддержки конструктора - не изменяйте
        /// содержимое данного метода при помощи редактора кода.
        /// </summary>
        private void InitializeComponent()
        {
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MainForm));
            this.CreateLangpackBtn = new System.Windows.Forms.Button();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.LangpacksComboBox = new System.Windows.Forms.ComboBox();
            this.label7 = new System.Windows.Forms.Label();
            this.LangpackNameEdit = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.InfMessageLangBox = new System.Windows.Forms.TextBox();
            this.tabControl1 = new System.Windows.Forms.TabControl();
            this.tabPage1 = new System.Windows.Forms.TabPage();
            this.groupBox3 = new System.Windows.Forms.GroupBox();
            this.OwnFileCheckBox = new System.Windows.Forms.CheckBox();
            this.SelectOwnFileBtn = new System.Windows.Forms.Button();
            this.OwnFileEdit = new System.Windows.Forms.TextBox();
            this.groupBox6 = new System.Windows.Forms.GroupBox();
            this.label6 = new System.Windows.Forms.Label();
            this.VIURLTextBox = new System.Windows.Forms.TextBox();
            this.GetVICheckBox = new System.Windows.Forms.CheckBox();
            this.tabPage2 = new System.Windows.Forms.TabPage();
            this.label4 = new System.Windows.Forms.Label();
            this.SVNURLEditBox = new System.Windows.Forms.TextBox();
            this.InfMessageLinkBox = new System.Windows.Forms.TextBox();
            this.LinkListGenBtn = new System.Windows.Forms.Button();
            this.label3 = new System.Windows.Forms.Label();
            this.tabPage3 = new System.Windows.Forms.TabPage();
            this.InfMessageFindBox = new System.Windows.Forms.TextBox();
            this.CommentFindBtn = new System.Windows.Forms.Button();
            this.NotTranslFindBtn = new System.Windows.Forms.Button();
            this.DupesFindBtn = new System.Windows.Forms.Button();
            this.groupBox5 = new System.Windows.Forms.GroupBox();
            this.AutoModeBtn = new System.Windows.Forms.RadioButton();
            this.ManualModeBtn = new System.Windows.Forms.RadioButton();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.LangPathLbl = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.AssignLangBtn = new System.Windows.Forms.Button();
            this.label5 = new System.Windows.Forms.Label();
            this.LanguageComboBox = new System.Windows.Forms.ComboBox();
            this.VarButton = new System.Windows.Forms.Button();
            this.groupBox2.SuspendLayout();
            this.tabControl1.SuspendLayout();
            this.tabPage1.SuspendLayout();
            this.groupBox3.SuspendLayout();
            this.groupBox6.SuspendLayout();
            this.tabPage2.SuspendLayout();
            this.tabPage3.SuspendLayout();
            this.groupBox5.SuspendLayout();
            this.groupBox1.SuspendLayout();
            this.SuspendLayout();
            // 
            // CreateLangpackBtn
            // 
            this.CreateLangpackBtn.Enabled = false;
            this.CreateLangpackBtn.Location = new System.Drawing.Point(99, 208);
            this.CreateLangpackBtn.Name = "CreateLangpackBtn";
            this.CreateLangpackBtn.Size = new System.Drawing.Size(141, 31);
            this.CreateLangpackBtn.TabIndex = 1;
            this.CreateLangpackBtn.Text = "Create Langpack";
            this.CreateLangpackBtn.UseVisualStyleBackColor = true;
            this.CreateLangpackBtn.Click += new System.EventHandler(this.CreateLangpackBtn_Click);
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add(this.LangpacksComboBox);
            this.groupBox2.Controls.Add(this.label7);
            this.groupBox2.Controls.Add(this.LangpackNameEdit);
            this.groupBox2.Controls.Add(this.label1);
            this.groupBox2.Location = new System.Drawing.Point(6, 3);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(317, 75);
            this.groupBox2.TabIndex = 2;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "Output File";
            // 
            // LangpacksComboBox
            // 
            this.LangpacksComboBox.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.LangpacksComboBox.FormattingEnabled = true;
            this.LangpacksComboBox.Location = new System.Drawing.Point(137, 17);
            this.LangpacksComboBox.Name = "LangpacksComboBox";
            this.LangpacksComboBox.Size = new System.Drawing.Size(160, 21);
            this.LangpacksComboBox.TabIndex = 11;
            this.LangpacksComboBox.SelectedIndexChanged += new System.EventHandler(this.LangpacksComboBox_SelectedIndexChanged);
            // 
            // label7
            // 
            this.label7.Location = new System.Drawing.Point(6, 20);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(130, 18);
            this.label7.TabIndex = 10;
            this.label7.Text = "Available Langpacks:";
            this.label7.TextAlign = System.Drawing.ContentAlignment.TopRight;
            // 
            // LangpackNameEdit
            // 
            this.LangpackNameEdit.Location = new System.Drawing.Point(137, 44);
            this.LangpackNameEdit.Name = "LangpackNameEdit";
            this.LangpackNameEdit.Size = new System.Drawing.Size(160, 20);
            this.LangpackNameEdit.TabIndex = 1;
            this.LangpackNameEdit.Text = "Langpack_";
            // 
            // label1
            // 
            this.label1.Location = new System.Drawing.Point(6, 47);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(130, 13);
            this.label1.TabIndex = 0;
            this.label1.Text = "Langpack File Name:";
            this.label1.TextAlign = System.Drawing.ContentAlignment.TopRight;
            // 
            // InfMessageLangBox
            // 
            this.InfMessageLangBox.BackColor = System.Drawing.SystemColors.Control;
            this.InfMessageLangBox.Location = new System.Drawing.Point(6, 242);
            this.InfMessageLangBox.Multiline = true;
            this.InfMessageLangBox.Name = "InfMessageLangBox";
            this.InfMessageLangBox.ReadOnly = true;
            this.InfMessageLangBox.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.InfMessageLangBox.Size = new System.Drawing.Size(317, 70);
            this.InfMessageLangBox.TabIndex = 3;
            // 
            // tabControl1
            // 
            this.tabControl1.Controls.Add(this.tabPage1);
            this.tabControl1.Controls.Add(this.tabPage2);
            this.tabControl1.Controls.Add(this.tabPage3);
            this.tabControl1.Location = new System.Drawing.Point(-3, 0);
            this.tabControl1.Name = "tabControl1";
            this.tabControl1.SelectedIndex = 0;
            this.tabControl1.Size = new System.Drawing.Size(337, 344);
            this.tabControl1.TabIndex = 8;
            // 
            // tabPage1
            // 
            this.tabPage1.BackColor = System.Drawing.SystemColors.Control;
            this.tabPage1.Controls.Add(this.InfMessageLangBox);
            this.tabPage1.Controls.Add(this.groupBox2);
            this.tabPage1.Controls.Add(this.CreateLangpackBtn);
            this.tabPage1.Controls.Add(this.groupBox3);
            this.tabPage1.Controls.Add(this.groupBox6);
            this.tabPage1.Location = new System.Drawing.Point(4, 22);
            this.tabPage1.Name = "tabPage1";
            this.tabPage1.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage1.Size = new System.Drawing.Size(329, 318);
            this.tabPage1.TabIndex = 0;
            this.tabPage1.Text = "Generator";
            // 
            // groupBox3
            // 
            this.groupBox3.Controls.Add(this.OwnFileCheckBox);
            this.groupBox3.Controls.Add(this.SelectOwnFileBtn);
            this.groupBox3.Controls.Add(this.OwnFileEdit);
            this.groupBox3.Location = new System.Drawing.Point(6, 151);
            this.groupBox3.Name = "groupBox3";
            this.groupBox3.Size = new System.Drawing.Size(317, 53);
            this.groupBox3.TabIndex = 5;
            this.groupBox3.TabStop = false;
            // 
            // OwnFileCheckBox
            // 
            this.OwnFileCheckBox.AutoSize = true;
            this.OwnFileCheckBox.Location = new System.Drawing.Point(6, 1);
            this.OwnFileCheckBox.Name = "OwnFileCheckBox";
            this.OwnFileCheckBox.Size = new System.Drawing.Size(170, 17);
            this.OwnFileCheckBox.TabIndex = 4;
            this.OwnFileCheckBox.Text = "Add your own file to Langpack";
            this.OwnFileCheckBox.UseVisualStyleBackColor = true;
            this.OwnFileCheckBox.CheckedChanged += new System.EventHandler(this.OwnFilesCheckBox_CheckedChanged);
            // 
            // SelectOwnFileBtn
            // 
            this.SelectOwnFileBtn.Enabled = false;
            this.SelectOwnFileBtn.Location = new System.Drawing.Point(222, 19);
            this.SelectOwnFileBtn.Name = "SelectOwnFileBtn";
            this.SelectOwnFileBtn.Size = new System.Drawing.Size(75, 23);
            this.SelectOwnFileBtn.TabIndex = 1;
            this.SelectOwnFileBtn.Text = "Select";
            this.SelectOwnFileBtn.UseVisualStyleBackColor = true;
            this.SelectOwnFileBtn.Click += new System.EventHandler(this.SelectOwnFilesBtn_Click);
            // 
            // OwnFileEdit
            // 
            this.OwnFileEdit.Enabled = false;
            this.OwnFileEdit.Location = new System.Drawing.Point(22, 21);
            this.OwnFileEdit.Name = "OwnFileEdit";
            this.OwnFileEdit.Size = new System.Drawing.Size(186, 20);
            this.OwnFileEdit.TabIndex = 0;
            // 
            // groupBox6
            // 
            this.groupBox6.Controls.Add(this.label6);
            this.groupBox6.Controls.Add(this.VIURLTextBox);
            this.groupBox6.Controls.Add(this.GetVICheckBox);
            this.groupBox6.Location = new System.Drawing.Point(6, 82);
            this.groupBox6.Name = "groupBox6";
            this.groupBox6.Size = new System.Drawing.Size(317, 67);
            this.groupBox6.TabIndex = 8;
            this.groupBox6.TabStop = false;
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Enabled = false;
            this.label6.Location = new System.Drawing.Point(23, 46);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(223, 13);
            this.label6.TabIndex = 2;
            this.label6.Text = "e.g. http://vi.miranda-ng.org/detail/mataes";
            // 
            // VIURLTextBox
            // 
            this.VIURLTextBox.Enabled = false;
            this.VIURLTextBox.Location = new System.Drawing.Point(22, 23);
            this.VIURLTextBox.Name = "VIURLTextBox";
            this.VIURLTextBox.Size = new System.Drawing.Size(275, 20);
            this.VIURLTextBox.TabIndex = 1;
            // 
            // GetVICheckBox
            // 
            this.GetVICheckBox.AutoSize = true;
            this.GetVICheckBox.Location = new System.Drawing.Point(6, 0);
            this.GetVICheckBox.Name = "GetVICheckBox";
            this.GetVICheckBox.Size = new System.Drawing.Size(261, 17);
            this.GetVICheckBox.TabIndex = 0;
            this.GetVICheckBox.Text = "Get VersionInfo from Internet or type full local path";
            this.GetVICheckBox.UseVisualStyleBackColor = true;
            this.GetVICheckBox.CheckedChanged += new System.EventHandler(this.GetVICheckBox_CheckedChanged);
            // 
            // tabPage2
            // 
            this.tabPage2.BackColor = System.Drawing.SystemColors.Control;
            this.tabPage2.Controls.Add(this.label4);
            this.tabPage2.Controls.Add(this.SVNURLEditBox);
            this.tabPage2.Controls.Add(this.InfMessageLinkBox);
            this.tabPage2.Controls.Add(this.LinkListGenBtn);
            this.tabPage2.Controls.Add(this.label3);
            this.tabPage2.Location = new System.Drawing.Point(4, 22);
            this.tabPage2.Name = "tabPage2";
            this.tabPage2.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage2.Size = new System.Drawing.Size(329, 318);
            this.tabPage2.TabIndex = 1;
            this.tabPage2.Text = "LinkListGen";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(11, 121);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(69, 13);
            this.label4.TabIndex = 6;
            this.label4.Text = "URL of SVN:";
            // 
            // SVNURLEditBox
            // 
            this.SVNURLEditBox.Location = new System.Drawing.Point(7, 137);
            this.SVNURLEditBox.Name = "SVNURLEditBox";
            this.SVNURLEditBox.Size = new System.Drawing.Size(317, 20);
            this.SVNURLEditBox.TabIndex = 5;
            this.SVNURLEditBox.Text = "http://svn.miranda-ng.org/main/trunk/langpacks/english";
            // 
            // InfMessageLinkBox
            // 
            this.InfMessageLinkBox.BackColor = System.Drawing.SystemColors.Control;
            this.InfMessageLinkBox.Location = new System.Drawing.Point(6, 242);
            this.InfMessageLinkBox.Multiline = true;
            this.InfMessageLinkBox.Name = "InfMessageLinkBox";
            this.InfMessageLinkBox.ReadOnly = true;
            this.InfMessageLinkBox.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.InfMessageLinkBox.Size = new System.Drawing.Size(317, 70);
            this.InfMessageLinkBox.TabIndex = 4;
            // 
            // LinkListGenBtn
            // 
            this.LinkListGenBtn.Location = new System.Drawing.Point(101, 178);
            this.LinkListGenBtn.Name = "LinkListGenBtn";
            this.LinkListGenBtn.Size = new System.Drawing.Size(122, 41);
            this.LinkListGenBtn.TabIndex = 1;
            this.LinkListGenBtn.Text = "Create File";
            this.LinkListGenBtn.UseVisualStyleBackColor = true;
            this.LinkListGenBtn.Click += new System.EventHandler(this.LinkListGenBtn_Click);
            // 
            // label3
            // 
            this.label3.Location = new System.Drawing.Point(6, 15);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(316, 97);
            this.label3.TabIndex = 0;
            this.label3.Text = resources.GetString("label3.Text");
            // 
            // tabPage3
            // 
            this.tabPage3.BackColor = System.Drawing.SystemColors.Control;
            this.tabPage3.Controls.Add(this.InfMessageFindBox);
            this.tabPage3.Controls.Add(this.CommentFindBtn);
            this.tabPage3.Controls.Add(this.NotTranslFindBtn);
            this.tabPage3.Controls.Add(this.DupesFindBtn);
            this.tabPage3.Controls.Add(this.groupBox5);
            this.tabPage3.Controls.Add(this.groupBox1);
            this.tabPage3.Location = new System.Drawing.Point(4, 22);
            this.tabPage3.Name = "tabPage3";
            this.tabPage3.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage3.Size = new System.Drawing.Size(329, 318);
            this.tabPage3.TabIndex = 2;
            this.tabPage3.Text = "Dupes Search";
            // 
            // InfMessageFindBox
            // 
            this.InfMessageFindBox.BackColor = System.Drawing.SystemColors.Control;
            this.InfMessageFindBox.Location = new System.Drawing.Point(6, 242);
            this.InfMessageFindBox.Multiline = true;
            this.InfMessageFindBox.Name = "InfMessageFindBox";
            this.InfMessageFindBox.ReadOnly = true;
            this.InfMessageFindBox.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.InfMessageFindBox.Size = new System.Drawing.Size(317, 70);
            this.InfMessageFindBox.TabIndex = 6;
            // 
            // CommentFindBtn
            // 
            this.CommentFindBtn.Enabled = false;
            this.CommentFindBtn.Location = new System.Drawing.Point(192, 160);
            this.CommentFindBtn.Name = "CommentFindBtn";
            this.CommentFindBtn.Size = new System.Drawing.Size(134, 34);
            this.CommentFindBtn.TabIndex = 5;
            this.CommentFindBtn.Text = "Find Commented";
            this.CommentFindBtn.UseVisualStyleBackColor = true;
            this.CommentFindBtn.Click += new System.EventHandler(this.CommentFindBtn_Click);
            // 
            // NotTranslFindBtn
            // 
            this.NotTranslFindBtn.Enabled = false;
            this.NotTranslFindBtn.Location = new System.Drawing.Point(80, 160);
            this.NotTranslFindBtn.Name = "NotTranslFindBtn";
            this.NotTranslFindBtn.Size = new System.Drawing.Size(108, 34);
            this.NotTranslFindBtn.TabIndex = 4;
            this.NotTranslFindBtn.Text = "Find Not Translated";
            this.NotTranslFindBtn.UseVisualStyleBackColor = true;
            this.NotTranslFindBtn.Click += new System.EventHandler(this.NotTranslFindBtn_Click);
            // 
            // DupesFindBtn
            // 
            this.DupesFindBtn.Enabled = false;
            this.DupesFindBtn.Location = new System.Drawing.Point(6, 160);
            this.DupesFindBtn.Name = "DupesFindBtn";
            this.DupesFindBtn.Size = new System.Drawing.Size(69, 34);
            this.DupesFindBtn.TabIndex = 3;
            this.DupesFindBtn.Text = "Find Dupes";
            this.DupesFindBtn.UseVisualStyleBackColor = true;
            this.DupesFindBtn.Click += new System.EventHandler(this.DupesFindBtn_Click);
            // 
            // groupBox5
            // 
            this.groupBox5.Controls.Add(this.AutoModeBtn);
            this.groupBox5.Controls.Add(this.ManualModeBtn);
            this.groupBox5.Location = new System.Drawing.Point(6, 105);
            this.groupBox5.Name = "groupBox5";
            this.groupBox5.Size = new System.Drawing.Size(320, 45);
            this.groupBox5.TabIndex = 2;
            this.groupBox5.TabStop = false;
            this.groupBox5.Text = "Working mode";
            // 
            // AutoModeBtn
            // 
            this.AutoModeBtn.AutoSize = true;
            this.AutoModeBtn.Enabled = false;
            this.AutoModeBtn.Location = new System.Drawing.Point(203, 18);
            this.AutoModeBtn.Name = "AutoModeBtn";
            this.AutoModeBtn.Size = new System.Drawing.Size(72, 17);
            this.AutoModeBtn.TabIndex = 1;
            this.AutoModeBtn.TabStop = true;
            this.AutoModeBtn.Text = "Automatic";
            this.AutoModeBtn.UseVisualStyleBackColor = true;
            // 
            // ManualModeBtn
            // 
            this.ManualModeBtn.AutoSize = true;
            this.ManualModeBtn.Enabled = false;
            this.ManualModeBtn.Location = new System.Drawing.Point(9, 18);
            this.ManualModeBtn.Name = "ManualModeBtn";
            this.ManualModeBtn.Size = new System.Drawing.Size(60, 17);
            this.ManualModeBtn.TabIndex = 0;
            this.ManualModeBtn.TabStop = true;
            this.ManualModeBtn.Text = "Manual";
            this.ManualModeBtn.UseVisualStyleBackColor = true;
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.LangPathLbl);
            this.groupBox1.Controls.Add(this.label2);
            this.groupBox1.Controls.Add(this.AssignLangBtn);
            this.groupBox1.Location = new System.Drawing.Point(6, 6);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(320, 82);
            this.groupBox1.TabIndex = 1;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Assign Langpack File";
            // 
            // LangPathLbl
            // 
            this.LangPathLbl.AutoSize = true;
            this.LangPathLbl.Location = new System.Drawing.Point(44, 54);
            this.LangPathLbl.Name = "LangPathLbl";
            this.LangPathLbl.Size = new System.Drawing.Size(67, 13);
            this.LangPathLbl.TabIndex = 2;
            this.LangPathLbl.Text = "not assigned";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(6, 54);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(38, 13);
            this.label2.TabIndex = 1;
            this.label2.Text = "Name:";
            // 
            // AssignLangBtn
            // 
            this.AssignLangBtn.Location = new System.Drawing.Point(114, 19);
            this.AssignLangBtn.Name = "AssignLangBtn";
            this.AssignLangBtn.Size = new System.Drawing.Size(75, 23);
            this.AssignLangBtn.TabIndex = 0;
            this.AssignLangBtn.Text = "Assign";
            this.AssignLangBtn.UseVisualStyleBackColor = true;
            this.AssignLangBtn.Click += new System.EventHandler(this.AssignLangBtn_Click);
            // 
            // label5
            // 
            this.label5.Location = new System.Drawing.Point(83, 351);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(119, 18);
            this.label5.TabIndex = 9;
            this.label5.Text = "Language:";
            this.label5.TextAlign = System.Drawing.ContentAlignment.TopRight;
            // 
            // LanguageComboBox
            // 
            this.LanguageComboBox.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.LanguageComboBox.FormattingEnabled = true;
            this.LanguageComboBox.Items.AddRange(new object[] {
            "English",
            "Русский",
            "Українська",
            "Беларускі",
            "Deutsch",
            "Čeština"});
            this.LanguageComboBox.Location = new System.Drawing.Point(204, 348);
            this.LanguageComboBox.Name = "LanguageComboBox";
            this.LanguageComboBox.Size = new System.Drawing.Size(121, 21);
            this.LanguageComboBox.TabIndex = 10;
            this.LanguageComboBox.SelectedIndexChanged += new System.EventHandler(this.LanguageComboBox_SelectedIndexChanged);
            // 
            // VarButton
            // 
            this.VarButton.Image = ((System.Drawing.Image)(resources.GetObject("VarButton.Image")));
            this.VarButton.Location = new System.Drawing.Point(7, 346);
            this.VarButton.Name = "VarButton";
            this.VarButton.Size = new System.Drawing.Size(22, 22);
            this.VarButton.TabIndex = 11;
            this.VarButton.UseVisualStyleBackColor = true;
            this.VarButton.Click += new System.EventHandler(this.VarButton_Click);
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(329, 373);
            this.Controls.Add(this.VarButton);
            this.Controls.Add(this.LanguageComboBox);
            this.Controls.Add(this.label5);
            this.Controls.Add(this.tabControl1);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.Fixed3D;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MaximizeBox = false;
            this.Name = "MainForm";
            this.Text = "Miranda NG Langpack Suite";
            this.groupBox2.ResumeLayout(false);
            this.groupBox2.PerformLayout();
            this.tabControl1.ResumeLayout(false);
            this.tabPage1.ResumeLayout(false);
            this.tabPage1.PerformLayout();
            this.groupBox3.ResumeLayout(false);
            this.groupBox3.PerformLayout();
            this.groupBox6.ResumeLayout(false);
            this.groupBox6.PerformLayout();
            this.tabPage2.ResumeLayout(false);
            this.tabPage2.PerformLayout();
            this.tabPage3.ResumeLayout(false);
            this.tabPage3.PerformLayout();
            this.groupBox5.ResumeLayout(false);
            this.groupBox5.PerformLayout();
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Button CreateLangpackBtn;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.TextBox LangpackNameEdit;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TextBox InfMessageLangBox;
        private System.Windows.Forms.TabControl tabControl1;
        private System.Windows.Forms.TabPage tabPage1;
        private System.Windows.Forms.TabPage tabPage2;
        private System.Windows.Forms.TextBox SVNURLEditBox;
        private System.Windows.Forms.TextBox InfMessageLinkBox;
        private System.Windows.Forms.Button LinkListGenBtn;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.TabPage tabPage3;
        public System.Windows.Forms.TextBox InfMessageFindBox;
        private System.Windows.Forms.Button CommentFindBtn;
        private System.Windows.Forms.Button NotTranslFindBtn;
        private System.Windows.Forms.Button DupesFindBtn;
        private System.Windows.Forms.GroupBox groupBox5;
        private System.Windows.Forms.RadioButton AutoModeBtn;
        private System.Windows.Forms.RadioButton ManualModeBtn;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.Label LangPathLbl;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Button AssignLangBtn;
        private System.Windows.Forms.Label label5;
        public System.Windows.Forms.ComboBox LanguageComboBox;
        private System.Windows.Forms.GroupBox groupBox3;
        private System.Windows.Forms.CheckBox OwnFileCheckBox;
        private System.Windows.Forms.Button SelectOwnFileBtn;
        private System.Windows.Forms.TextBox OwnFileEdit;
        private System.Windows.Forms.GroupBox groupBox6;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.TextBox VIURLTextBox;
        private System.Windows.Forms.CheckBox GetVICheckBox;
        private System.Windows.Forms.Button VarButton;
        public System.Windows.Forms.ComboBox LangpacksComboBox;
        private System.Windows.Forms.Label label7;
    }
}

