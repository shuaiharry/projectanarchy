/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

namespace Editor.Dialogs
{
  partial class EULA
  {
    /// <summary>
    /// Required designer variable.
    /// </summary>
    private System.ComponentModel.IContainer components = null;

    #region Windows Form Designer generated code

    /// <summary>
    /// Required method for Designer support - do not modify
    /// the contents of this method with the code editor.
    /// </summary>
    private void InitializeComponent()
    {
      System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(EULA));
      this.EulaTextBox = new System.Windows.Forms.RichTextBox();
      this.tableLayoutPanel2 = new System.Windows.Forms.TableLayoutPanel();
      this.AcceptEulaButton = new System.Windows.Forms.Button();
      this.RejectEulaButton = new System.Windows.Forms.Button();
      this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
      this.tableLayoutPanel2.SuspendLayout();
      this.tableLayoutPanel1.SuspendLayout();
      this.SuspendLayout();
      // 
      // EulaTextBox
      // 
      this.EulaTextBox.AutoWordSelection = true;
      this.EulaTextBox.Dock = System.Windows.Forms.DockStyle.Fill;
      this.EulaTextBox.Location = new System.Drawing.Point(3, 3);
      this.EulaTextBox.Name = "EulaTextBox";
      this.EulaTextBox.ReadOnly = true;
      this.EulaTextBox.Size = new System.Drawing.Size(680, 700);
      this.EulaTextBox.TabIndex = 0;
      this.EulaTextBox.Text = "";
      // 
      // tableLayoutPanel2
      // 
      this.tableLayoutPanel2.ColumnCount = 1;
      this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
      this.tableLayoutPanel2.Controls.Add(this.EulaTextBox, 0, 0);
      this.tableLayoutPanel2.Controls.Add(this.tableLayoutPanel1, 0, 1);
      this.tableLayoutPanel2.Dock = System.Windows.Forms.DockStyle.Fill;
      this.tableLayoutPanel2.Location = new System.Drawing.Point(0, 0);
      this.tableLayoutPanel2.Name = "tableLayoutPanel2";
      this.tableLayoutPanel2.RowCount = 2;
      this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
      this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
      this.tableLayoutPanel2.Size = new System.Drawing.Size(686, 748);
      this.tableLayoutPanel2.TabIndex = 4;
      // 
      // AcceptEulaButton
      // 
      this.AcceptEulaButton.Location = new System.Drawing.Point(521, 3);
      this.AcceptEulaButton.Name = "AcceptEulaButton";
      this.AcceptEulaButton.Size = new System.Drawing.Size(75, 23);
      this.AcceptEulaButton.TabIndex = 1;
      this.AcceptEulaButton.Text = "Accept";
      this.AcceptEulaButton.UseVisualStyleBackColor = true;
      this.AcceptEulaButton.Click += new System.EventHandler(this.AcceptEulaButton_Click);
      // 
      // RejectEulaButton
      // 
      this.RejectEulaButton.Location = new System.Drawing.Point(602, 3);
      this.RejectEulaButton.Name = "RejectEulaButton";
      this.RejectEulaButton.Size = new System.Drawing.Size(75, 23);
      this.RejectEulaButton.TabIndex = 2;
      this.RejectEulaButton.Text = "Reject";
      this.RejectEulaButton.UseVisualStyleBackColor = true;
      this.RejectEulaButton.Click += new System.EventHandler(this.RejectEulaButton_Click);
      // 
      // tableLayoutPanel1
      // 
      this.tableLayoutPanel1.ColumnCount = 3;
      this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
      this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
      this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
      this.tableLayoutPanel1.Controls.Add(this.RejectEulaButton, 2, 0);
      this.tableLayoutPanel1.Controls.Add(this.AcceptEulaButton, 1, 0);
      this.tableLayoutPanel1.Dock = System.Windows.Forms.DockStyle.Fill;
      this.tableLayoutPanel1.Location = new System.Drawing.Point(3, 709);
      this.tableLayoutPanel1.Name = "tableLayoutPanel1";
      this.tableLayoutPanel1.RowCount = 1;
      this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 30F));
      this.tableLayoutPanel1.Size = new System.Drawing.Size(680, 36);
      this.tableLayoutPanel1.TabIndex = 3;
      // 
      // EULA
      // 
      this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
      this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
      this.ClientSize = new System.Drawing.Size(686, 748);
      this.Controls.Add(this.tableLayoutPanel2);
      this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
      this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
      this.MaximizeBox = false;
      this.MinimizeBox = false;
      this.ShowInTaskbar = false;
      this.Name = "EULA";
      this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
      this.Text = "Project Anarchy EULA";
      this.tableLayoutPanel2.ResumeLayout(false);
      this.tableLayoutPanel1.ResumeLayout(false);
      this.ResumeLayout(false);

    }

    #endregion

    private System.Windows.Forms.RichTextBox EulaTextBox;
    private System.Windows.Forms.TableLayoutPanel tableLayoutPanel2;
    private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
    private System.Windows.Forms.Button RejectEulaButton;
    private System.Windows.Forms.Button AcceptEulaButton;
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
