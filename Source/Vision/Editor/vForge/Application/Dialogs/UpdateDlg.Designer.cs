/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

namespace Editor.Dialogs
{
  partial class UpdateDlg
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
      System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(UpdateDlg));
      this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
      this.downloadButton = new System.Windows.Forms.Button();
      this.ignoreButton = new System.Windows.Forms.Button();
      this.label1 = new System.Windows.Forms.Label();
      this.currentVersion = new System.Windows.Forms.Label();
      this.releaseText = new System.Windows.Forms.RichTextBox();
      this.yourVersion = new System.Windows.Forms.Label();
      this.tableLayoutPanel1.SuspendLayout();
      this.SuspendLayout();
      // 
      // tableLayoutPanel1
      // 
      this.tableLayoutPanel1.ColumnCount = 2;
      this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 50F));
      this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 50F));
      this.tableLayoutPanel1.Controls.Add(this.downloadButton, 1, 4);
      this.tableLayoutPanel1.Controls.Add(this.ignoreButton, 0, 4);
      this.tableLayoutPanel1.Controls.Add(this.label1, 0, 0);
      this.tableLayoutPanel1.Controls.Add(this.currentVersion, 0, 1);
      this.tableLayoutPanel1.Controls.Add(this.releaseText, 0, 3);
      this.tableLayoutPanel1.Controls.Add(this.yourVersion, 0, 2);
      this.tableLayoutPanel1.Dock = System.Windows.Forms.DockStyle.Fill;
      this.tableLayoutPanel1.Location = new System.Drawing.Point(0, 0);
      this.tableLayoutPanel1.Name = "tableLayoutPanel1";
      this.tableLayoutPanel1.RowCount = 5;
      this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
      this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
      this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
      this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
      this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
      this.tableLayoutPanel1.Size = new System.Drawing.Size(584, 312);
      this.tableLayoutPanel1.TabIndex = 0;
      // 
      // downloadButton
      // 
      this.downloadButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
      this.downloadButton.Location = new System.Drawing.Point(452, 281);
      this.downloadButton.Margin = new System.Windows.Forms.Padding(6);
      this.downloadButton.Name = "downloadButton";
      this.downloadButton.Size = new System.Drawing.Size(126, 25);
      this.downloadButton.TabIndex = 2;
      this.downloadButton.Text = "Download now";
      this.downloadButton.UseVisualStyleBackColor = true;
      this.downloadButton.Click += new System.EventHandler(this.downloadButton_Click);
      // 
      // ignoreButton
      // 
      this.ignoreButton.Location = new System.Drawing.Point(6, 281);
      this.ignoreButton.Margin = new System.Windows.Forms.Padding(6);
      this.ignoreButton.Name = "ignoreButton";
      this.ignoreButton.Size = new System.Drawing.Size(122, 25);
      this.ignoreButton.TabIndex = 1;
      this.ignoreButton.Text = "Ignore";
      this.ignoreButton.UseVisualStyleBackColor = true;
      this.ignoreButton.Click += new System.EventHandler(this.ignoreButton_Click);
      // 
      // label1
      // 
      this.label1.AutoSize = true;
      this.tableLayoutPanel1.SetColumnSpan(this.label1, 2);
      this.label1.Dock = System.Windows.Forms.DockStyle.Fill;
      this.label1.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
      this.label1.Location = new System.Drawing.Point(40, 10);
      this.label1.Margin = new System.Windows.Forms.Padding(40, 10, 3, 0);
      this.label1.Name = "label1";
      this.label1.Size = new System.Drawing.Size(541, 15);
      this.label1.TabIndex = 3;
      this.label1.Text = "A new version of Project Anarchy is available:";
      // 
      // currentVersion
      // 
      this.currentVersion.AutoSize = true;
      this.tableLayoutPanel1.SetColumnSpan(this.currentVersion, 2);
      this.currentVersion.Dock = System.Windows.Forms.DockStyle.Fill;
      this.currentVersion.Font = new System.Drawing.Font("Microsoft Sans Serif", 11F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
      this.currentVersion.Location = new System.Drawing.Point(40, 35);
      this.currentVersion.Margin = new System.Windows.Forms.Padding(40, 10, 3, 0);
      this.currentVersion.Name = "currentVersion";
      this.currentVersion.Size = new System.Drawing.Size(541, 18);
      this.currentVersion.TabIndex = 4;
      this.currentVersion.Text = "2013";
      // 
      // releaseText
      // 
      this.releaseText.AutoWordSelection = true;
      this.tableLayoutPanel1.SetColumnSpan(this.releaseText, 2);
      this.releaseText.Dock = System.Windows.Forms.DockStyle.Fill;
      this.releaseText.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
      this.releaseText.Location = new System.Drawing.Point(40, 86);
      this.releaseText.Margin = new System.Windows.Forms.Padding(40, 10, 40, 3);
      this.releaseText.Name = "releaseText";
      this.releaseText.ReadOnly = true;
      this.releaseText.Size = new System.Drawing.Size(504, 186);
      this.releaseText.TabIndex = 5;
      this.releaseText.Text = "";
      // 
      // yourVersion
      // 
      this.yourVersion.AutoSize = true;
      this.tableLayoutPanel1.SetColumnSpan(this.yourVersion, 2);
      this.yourVersion.Dock = System.Windows.Forms.DockStyle.Fill;
      this.yourVersion.Location = new System.Drawing.Point(40, 63);
      this.yourVersion.Margin = new System.Windows.Forms.Padding(40, 10, 3, 0);
      this.yourVersion.Name = "yourVersion";
      this.yourVersion.Size = new System.Drawing.Size(541, 13);
      this.yourVersion.TabIndex = 6;
      this.yourVersion.Text = "Your version is";
      // 
      // UpdateDlg
      // 
      this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
      this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
      this.AutoSize = true;
      this.ClientSize = new System.Drawing.Size(584, 312);
      this.Controls.Add(this.tableLayoutPanel1);
      this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
      this.MaximizeBox = false;
      this.MinimizeBox = false;
      this.ShowInTaskbar = false;
      this.MinimumSize = new System.Drawing.Size(400, 350);
      this.Name = "UpdateDlg";
      this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
      this.Text = "Software Update";
      this.tableLayoutPanel1.ResumeLayout(false);
      this.tableLayoutPanel1.PerformLayout();
      this.ResumeLayout(false);

    }

    #endregion

    private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
    private System.Windows.Forms.Button ignoreButton;
    private System.Windows.Forms.Button downloadButton;
    private System.Windows.Forms.Label label1;
    private System.Windows.Forms.Label currentVersion;
    private System.Windows.Forms.RichTextBox releaseText;
    private System.Windows.Forms.Label yourVersion;
  }
}

/*
 * Havok SDK - Base file, BUILD(#20131019)
 * 
 * Confidential Information of Havok.  (C) Copyright 1999-2013
 * Telekinesys Research Limited t/a Havok. All Rights Reserved. The Havok
 * Logo, and the Havok buzzsaw logo are trademarks of Havok.  Title, ownership
 * rights, and intellectual property rights in the Havok software remain in
 * Havok and/or its suppliers.
 * 
 * Use of this software for evaluation purposes is subject to and indicates
 * acceptance of the End User licence Agreement for this product. A copy of
 * the license is included with this software and is also available from salesteam@havok.com.
 * 
 */
