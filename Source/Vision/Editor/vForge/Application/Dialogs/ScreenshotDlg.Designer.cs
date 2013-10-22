namespace Editor.Dialogs
{
  partial class ScreenshotDlg
  {
    /// <summary>
    /// Required designer variable.
    /// </summary>
    private System.ComponentModel.IContainer components = null;

    /// <summary>
    /// Clean up any resources being used.
    /// </summary>
    /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
    protected override void Dispose(bool disposing)
    {
      if (disposing && (components != null))
      {
        components.Dispose();
      }
      base.Dispose(disposing);
    }

    #region Windows Form Designer generated code

    /// <summary>
    /// Required method for Designer support - do not modify
    /// the contents of this method with the code editor.
    /// </summary>
    private void InitializeComponent()
    {
      System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(ScreenshotDlg));
      this.buttonCancel = new System.Windows.Forms.Button();
      this.textBoxOutputDir = new System.Windows.Forms.TextBox();
      this.textBoxFileName = new System.Windows.Forms.TextBox();
      this.buttonOutputDir = new System.Windows.Forms.Button();
      this.groupBoxCustomViewport = new System.Windows.Forms.GroupBox();
      this.labelMul2 = new System.Windows.Forms.Label();
      this.labelMul1 = new System.Windows.Forms.Label();
      this.labelFieldOfView = new System.Windows.Forms.Label();
      this.labelResolution = new System.Windows.Forms.Label();
      this.numericUpDownFovY = new System.Windows.Forms.NumericUpDown();
      this.numericUpDownFovX = new System.Windows.Forms.NumericUpDown();
      this.numericUpDownResY = new System.Windows.Forms.NumericUpDown();
      this.numericUpDownResX = new System.Windows.Forms.NumericUpDown();
      this.radioButtonCurrentViewPort = new System.Windows.Forms.RadioButton();
      this.labelOutputDir = new System.Windows.Forms.Label();
      this.labelFileName = new System.Windows.Forms.Label();
      this.buttonOK = new System.Windows.Forms.Button();
      this.buttonSaveScreenShot = new System.Windows.Forms.Button();
      this.comboBoxExtension = new System.Windows.Forms.ComboBox();
      this.radioButtonCustomViewport = new System.Windows.Forms.RadioButton();
      this.checkBoxOpenFile = new System.Windows.Forms.CheckBox();
      this.captionBar = new CSharpFramework.Controls.DialogCaptionBar();
      this.groupBoxCustomViewport.SuspendLayout();
      ((System.ComponentModel.ISupportInitialize)(this.numericUpDownFovY)).BeginInit();
      ((System.ComponentModel.ISupportInitialize)(this.numericUpDownFovX)).BeginInit();
      ((System.ComponentModel.ISupportInitialize)(this.numericUpDownResY)).BeginInit();
      ((System.ComponentModel.ISupportInitialize)(this.numericUpDownResX)).BeginInit();
      this.SuspendLayout();
      // 
      // buttonCancel
      // 
      this.buttonCancel.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
      this.buttonCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
      this.buttonCancel.Location = new System.Drawing.Point(378, 320);
      this.buttonCancel.Name = "buttonCancel";
      this.buttonCancel.Size = new System.Drawing.Size(75, 23);
      this.buttonCancel.TabIndex = 10;
      this.buttonCancel.Text = "&Cancel";
      this.buttonCancel.UseVisualStyleBackColor = true;
      this.buttonCancel.Click += new System.EventHandler(this.buttonCancel_Click);
      // 
      // textBoxOutputDir
      // 
      this.textBoxOutputDir.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
      this.textBoxOutputDir.Location = new System.Drawing.Point(89, 72);
      this.textBoxOutputDir.Name = "textBoxOutputDir";
      this.textBoxOutputDir.Size = new System.Drawing.Size(330, 20);
      this.textBoxOutputDir.TabIndex = 1;
      this.textBoxOutputDir.TextChanged += new System.EventHandler(this.textBoxOutputDir_TextChanged);
      // 
      // textBoxFileName
      // 
      this.textBoxFileName.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
      this.textBoxFileName.Location = new System.Drawing.Point(89, 98);
      this.textBoxFileName.Name = "textBoxFileName";
      this.textBoxFileName.Size = new System.Drawing.Size(258, 20);
      this.textBoxFileName.TabIndex = 4;
      this.textBoxFileName.TextChanged += new System.EventHandler(this.textBoxFileName_TextChanged);
      // 
      // buttonOutputDir
      // 
      this.buttonOutputDir.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
      this.buttonOutputDir.Location = new System.Drawing.Point(426, 70);
      this.buttonOutputDir.Name = "buttonOutputDir";
      this.buttonOutputDir.Size = new System.Drawing.Size(27, 23);
      this.buttonOutputDir.TabIndex = 2;
      this.buttonOutputDir.Text = "...";
      this.buttonOutputDir.UseVisualStyleBackColor = true;
      this.buttonOutputDir.Click += new System.EventHandler(this.buttonOutputDir_Click);
      // 
      // groupBoxCustomViewport
      // 
      this.groupBoxCustomViewport.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
      this.groupBoxCustomViewport.Controls.Add(this.labelMul2);
      this.groupBoxCustomViewport.Controls.Add(this.labelMul1);
      this.groupBoxCustomViewport.Controls.Add(this.labelFieldOfView);
      this.groupBoxCustomViewport.Controls.Add(this.labelResolution);
      this.groupBoxCustomViewport.Controls.Add(this.numericUpDownFovY);
      this.groupBoxCustomViewport.Controls.Add(this.numericUpDownFovX);
      this.groupBoxCustomViewport.Controls.Add(this.numericUpDownResY);
      this.groupBoxCustomViewport.Controls.Add(this.numericUpDownResX);
      this.groupBoxCustomViewport.Enabled = false;
      this.groupBoxCustomViewport.Location = new System.Drawing.Point(12, 166);
      this.groupBoxCustomViewport.Name = "groupBoxCustomViewport";
      this.groupBoxCustomViewport.Size = new System.Drawing.Size(441, 95);
      this.groupBoxCustomViewport.TabIndex = 8;
      this.groupBoxCustomViewport.TabStop = false;
      this.groupBoxCustomViewport.Text = "    ";
      // 
      // labelMul2
      // 
      this.labelMul2.AutoSize = true;
      this.labelMul2.Location = new System.Drawing.Point(282, 54);
      this.labelMul2.Name = "labelMul2";
      this.labelMul2.Size = new System.Drawing.Size(13, 13);
      this.labelMul2.TabIndex = 6;
      this.labelMul2.Text = " ";
      // 
      // labelMul1
      // 
      this.labelMul1.AutoSize = true;
      this.labelMul1.Location = new System.Drawing.Point(282, 30);
      this.labelMul1.Name = "labelMul1";
      this.labelMul1.Size = new System.Drawing.Size(13, 13);
      this.labelMul1.TabIndex = 2;
      this.labelMul1.Text = " ";
      // 
      // labelFieldOfView
      // 
      this.labelFieldOfView.AutoSize = true;
      this.labelFieldOfView.Location = new System.Drawing.Point(63, 54);
      this.labelFieldOfView.Name = "labelFieldOfView";
      this.labelFieldOfView.Size = new System.Drawing.Size(67, 13);
      this.labelFieldOfView.TabIndex = 4;
      this.labelFieldOfView.Text = "Field of &View";
      // 
      // labelResolution
      // 
      this.labelResolution.AutoSize = true;
      this.labelResolution.Location = new System.Drawing.Point(63, 28);
      this.labelResolution.Name = "labelResolution";
      this.labelResolution.Size = new System.Drawing.Size(57, 13);
      this.labelResolution.TabIndex = 0;
      this.labelResolution.Text = "&Resolution";
      // 
      // numericUpDownFovY
      // 
      this.numericUpDownFovY.Location = new System.Drawing.Point(301, 52);
      this.numericUpDownFovY.Maximum = new decimal(new int[] {
            179,
            0,
            0,
            0});
      this.numericUpDownFovY.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
      this.numericUpDownFovY.Name = "numericUpDownFovY";
      this.numericUpDownFovY.Size = new System.Drawing.Size(120, 20);
      this.numericUpDownFovY.TabIndex = 7;
      this.numericUpDownFovY.Value = new decimal(new int[] {
            10,
            0,
            0,
            0});
      this.numericUpDownFovY.ValueChanged += new System.EventHandler(this.numericUpDownFovY_ValueChanged);
      // 
      // numericUpDownFovX
      // 
      this.numericUpDownFovX.Location = new System.Drawing.Point(155, 52);
      this.numericUpDownFovX.Maximum = new decimal(new int[] {
            179,
            0,
            0,
            0});
      this.numericUpDownFovX.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
      this.numericUpDownFovX.Name = "numericUpDownFovX";
      this.numericUpDownFovX.Size = new System.Drawing.Size(120, 20);
      this.numericUpDownFovX.TabIndex = 5;
      this.numericUpDownFovX.Value = new decimal(new int[] {
            10,
            0,
            0,
            0});
      this.numericUpDownFovX.ValueChanged += new System.EventHandler(this.numericUpDownFovX_ValueChanged);
      // 
      // numericUpDownResY
      // 
      this.numericUpDownResY.Location = new System.Drawing.Point(301, 26);
      this.numericUpDownResY.Maximum = new decimal(new int[] {
            8192,
            0,
            0,
            0});
      this.numericUpDownResY.Minimum = new decimal(new int[] {
            128,
            0,
            0,
            0});
      this.numericUpDownResY.Name = "numericUpDownResY";
      this.numericUpDownResY.Size = new System.Drawing.Size(120, 20);
      this.numericUpDownResY.TabIndex = 3;
      this.numericUpDownResY.Value = new decimal(new int[] {
            128,
            0,
            0,
            0});
      this.numericUpDownResY.ValueChanged += new System.EventHandler(this.numericUpDownResY_ValueChanged);
      // 
      // numericUpDownResX
      // 
      this.numericUpDownResX.Location = new System.Drawing.Point(155, 26);
      this.numericUpDownResX.Maximum = new decimal(new int[] {
            8192,
            0,
            0,
            0});
      this.numericUpDownResX.Minimum = new decimal(new int[] {
            128,
            0,
            0,
            0});
      this.numericUpDownResX.Name = "numericUpDownResX";
      this.numericUpDownResX.Size = new System.Drawing.Size(120, 20);
      this.numericUpDownResX.TabIndex = 1;
      this.numericUpDownResX.Value = new decimal(new int[] {
            128,
            0,
            0,
            0});
      this.numericUpDownResX.ValueChanged += new System.EventHandler(this.numericUpDownResX_ValueChanged);
      // 
      // radioButtonCurrentViewPort
      // 
      this.radioButtonCurrentViewPort.AutoSize = true;
      this.radioButtonCurrentViewPort.Checked = true;
      this.radioButtonCurrentViewPort.Location = new System.Drawing.Point(24, 141);
      this.radioButtonCurrentViewPort.Name = "radioButtonCurrentViewPort";
      this.radioButtonCurrentViewPort.Size = new System.Drawing.Size(103, 17);
      this.radioButtonCurrentViewPort.TabIndex = 6;
      this.radioButtonCurrentViewPort.TabStop = true;
      this.radioButtonCurrentViewPort.Text = "Current View&port";
      this.radioButtonCurrentViewPort.UseVisualStyleBackColor = true;
      this.radioButtonCurrentViewPort.CheckedChanged += new System.EventHandler(this.radioButtonCurrentViewPort_CheckedChanged);
      // 
      // labelOutputDir
      // 
      this.labelOutputDir.AutoSize = true;
      this.labelOutputDir.Location = new System.Drawing.Point(12, 75);
      this.labelOutputDir.Name = "labelOutputDir";
      this.labelOutputDir.Size = new System.Drawing.Size(55, 13);
      this.labelOutputDir.TabIndex = 0;
      this.labelOutputDir.Text = "Output &Dir";
      // 
      // labelFileName
      // 
      this.labelFileName.AutoSize = true;
      this.labelFileName.Location = new System.Drawing.Point(13, 101);
      this.labelFileName.Name = "labelFileName";
      this.labelFileName.Size = new System.Drawing.Size(54, 13);
      this.labelFileName.TabIndex = 3;
      this.labelFileName.Text = "&File Name";
      // 
      // buttonOK
      // 
      this.buttonOK.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
      this.buttonOK.DialogResult = System.Windows.Forms.DialogResult.OK;
      this.buttonOK.Location = new System.Drawing.Point(297, 320);
      this.buttonOK.Name = "buttonOK";
      this.buttonOK.Size = new System.Drawing.Size(75, 23);
      this.buttonOK.TabIndex = 9;
      this.buttonOK.Text = "&OK";
      this.buttonOK.UseVisualStyleBackColor = true;
      this.buttonOK.Click += new System.EventHandler(this.buttonOK_Click);
      // 
      // buttonSaveScreenShot
      // 
      this.buttonSaveScreenShot.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
      this.buttonSaveScreenShot.Location = new System.Drawing.Point(12, 320);
      this.buttonSaveScreenShot.Name = "buttonSaveScreenShot";
      this.buttonSaveScreenShot.Size = new System.Drawing.Size(107, 23);
      this.buttonSaveScreenShot.TabIndex = 11;
      this.buttonSaveScreenShot.Text = "&Save Screenshot";
      this.buttonSaveScreenShot.UseVisualStyleBackColor = true;
      this.buttonSaveScreenShot.Click += new System.EventHandler(this.buttonSaveScreenShot_Click);
      // 
      // comboBoxExtension
      // 
      this.comboBoxExtension.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
      this.comboBoxExtension.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
      this.comboBoxExtension.FormattingEnabled = true;
      this.comboBoxExtension.Items.AddRange(new object[] {
            ".png",
            ".jpg",
            ".bmp"});
      this.comboBoxExtension.Location = new System.Drawing.Point(353, 98);
      this.comboBoxExtension.Name = "comboBoxExtension";
      this.comboBoxExtension.Size = new System.Drawing.Size(68, 21);
      this.comboBoxExtension.TabIndex = 5;
      this.comboBoxExtension.SelectedIndexChanged += new System.EventHandler(this.comboBoxExtension_SelectedIndexChanged);
      // 
      // radioButtonCustomViewport
      // 
      this.radioButtonCustomViewport.AutoSize = true;
      this.radioButtonCustomViewport.Location = new System.Drawing.Point(24, 164);
      this.radioButtonCustomViewport.Name = "radioButtonCustomViewport";
      this.radioButtonCustomViewport.Size = new System.Drawing.Size(60, 17);
      this.radioButtonCustomViewport.TabIndex = 7;
      this.radioButtonCustomViewport.Text = "Custo&m";
      this.radioButtonCustomViewport.UseVisualStyleBackColor = true;
      // 
      // checkBoxOpenFile
      // 
      this.checkBoxOpenFile.AutoSize = true;
      this.checkBoxOpenFile.Location = new System.Drawing.Point(24, 278);
      this.checkBoxOpenFile.Name = "checkBoxOpenFile";
      this.checkBoxOpenFile.Size = new System.Drawing.Size(181, 17);
      this.checkBoxOpenFile.TabIndex = 12;
      this.checkBoxOpenFile.Text = "Open screenshot file after saving";
      this.checkBoxOpenFile.UseVisualStyleBackColor = true;
      this.checkBoxOpenFile.CheckedChanged += new System.EventHandler(this.checkBoxOpenFile_CheckedChanged);
      // 
      // captionBar
      // 
      this.captionBar.BackColor = System.Drawing.SystemColors.Window;
      this.captionBar.Caption = "Screenshot Settings";
      this.captionBar.CompactView = false;
      this.captionBar.Description = "Edit settings for taking screenshots.";
      this.captionBar.Dock = System.Windows.Forms.DockStyle.Top;
      this.captionBar.Image = ((System.Drawing.Image)(resources.GetObject("captionBar.Image")));
      this.captionBar.Location = new System.Drawing.Point(0, 0);
      this.captionBar.Name = "captionBar";
      this.captionBar.SetFontColor = System.Drawing.SystemColors.ControlText;
      this.captionBar.ShowBorder = false;
      this.captionBar.ShowBottomLine = true;
      this.captionBar.ShowCaptionText = true;
      this.captionBar.ShowImage = true;
      this.captionBar.Size = new System.Drawing.Size(465, 56);
      this.captionBar.TabIndex = 4;
      // 
      // ScreenshotDlg
      // 
      this.ShowInTaskbar = false;
      this.AcceptButton = this.buttonOK;
      this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
      this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
      this.CancelButton = this.buttonCancel;
      this.ClientSize = new System.Drawing.Size(465, 355);
      this.Controls.Add(this.checkBoxOpenFile);
      this.Controls.Add(this.radioButtonCustomViewport);
      this.Controls.Add(this.comboBoxExtension);
      this.Controls.Add(this.buttonSaveScreenShot);
      this.Controls.Add(this.buttonOK);
      this.Controls.Add(this.labelFileName);
      this.Controls.Add(this.labelOutputDir);
      this.Controls.Add(this.radioButtonCurrentViewPort);
      this.Controls.Add(this.groupBoxCustomViewport);
      this.Controls.Add(this.captionBar);
      this.Controls.Add(this.buttonOutputDir);
      this.Controls.Add(this.textBoxFileName);
      this.Controls.Add(this.textBoxOutputDir);
      this.Controls.Add(this.buttonCancel);
      this.MaximizeBox = false;
      this.MinimizeBox = false;
      this.MinimumSize = new System.Drawing.Size(481, 393);
      this.Name = "ScreenshotDlg";
      this.ShowIcon = false;
      this.Text = "Screenshot Settings";
      this.groupBoxCustomViewport.ResumeLayout(false);
      this.groupBoxCustomViewport.PerformLayout();
      ((System.ComponentModel.ISupportInitialize)(this.numericUpDownFovY)).EndInit();
      ((System.ComponentModel.ISupportInitialize)(this.numericUpDownFovX)).EndInit();
      ((System.ComponentModel.ISupportInitialize)(this.numericUpDownResY)).EndInit();
      ((System.ComponentModel.ISupportInitialize)(this.numericUpDownResX)).EndInit();
      this.ResumeLayout(false);
      this.PerformLayout();

    }

    #endregion

    private System.Windows.Forms.Button buttonCancel;
    private System.Windows.Forms.TextBox textBoxOutputDir;
    private System.Windows.Forms.TextBox textBoxFileName;
    private System.Windows.Forms.Button buttonOutputDir;
    private CSharpFramework.Controls.DialogCaptionBar captionBar;
    private System.Windows.Forms.GroupBox groupBoxCustomViewport;
    private System.Windows.Forms.NumericUpDown numericUpDownFovY;
    private System.Windows.Forms.NumericUpDown numericUpDownFovX;
    private System.Windows.Forms.NumericUpDown numericUpDownResY;
    private System.Windows.Forms.NumericUpDown numericUpDownResX;
    private System.Windows.Forms.RadioButton radioButtonCurrentViewPort;
    private System.Windows.Forms.Label labelFieldOfView;
    private System.Windows.Forms.Label labelResolution;
    private System.Windows.Forms.Label labelOutputDir;
    private System.Windows.Forms.Label labelFileName;
    private System.Windows.Forms.Label labelMul2;
    private System.Windows.Forms.Label labelMul1;
    private System.Windows.Forms.Button buttonOK;
    private System.Windows.Forms.Button buttonSaveScreenShot;
    private System.Windows.Forms.ComboBox comboBoxExtension;
    private System.Windows.Forms.RadioButton radioButtonCustomViewport;
    private System.Windows.Forms.CheckBox checkBoxOpenFile;
  }
}