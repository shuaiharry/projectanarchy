/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using CSharpFramework;
using CSharpFramework.Helper;
using Editor.Actions;

namespace Editor.Dialogs
{
  /// <summary>
  /// Dialog for changing the screenshot settings and taking screenshots.
  /// </summary>
  public partial class ScreenshotDlg : Form
  {
    public ScreenshotDlg()
    {
      InitializeComponent();

      _settings = EditorApp.Scene.Settings.ScreenCaptureSettings.Clone() as ScreenshotSettings;
      RefreshGUI();
    }

    // Populate the GUI from the settings object.
    private void RefreshGUI()
    {
      textBoxOutputDir.Text = _settings.Directory;
      textBoxFileName.Text = _settings.FileName;

      comboBoxExtension.SelectedItem = comboBoxExtension.Items.Cast<object>().FirstOrDefault(item => item.ToString() == _settings.FileExtension);

      numericUpDownFovX.Value = Math.Max(numericUpDownFovX.Minimum, Math.Min(numericUpDownFovX.Maximum, (decimal)_settings.FovX));
      numericUpDownFovY.Value = Math.Max(numericUpDownFovY.Minimum, Math.Min(numericUpDownFovY.Maximum, (decimal)_settings.FovY));
      numericUpDownResX.Value = (decimal)_settings.ViewportWidth;
      numericUpDownResY.Value = (decimal)_settings.ViewportHeight;

      groupBoxCustomViewport.Enabled = _settings.UseCustomViewport;
      radioButtonCustomViewport.Checked = _settings.UseCustomViewport;

      checkBoxOpenFile.Checked = _settings.OpenExternally;

      labelMul1.Text = "\u00D7";  // Unicode multiplication sign - needs to be set here, else the Designer will keep inserting the raw unicode character into the code
      labelMul2.Text = "\u00D7";
    }

    /// <summary>
    /// Saves a screenshot of the current engine view with the given settings.
    /// </summary>
    public static void SaveScreenshot(ScreenshotSettings settings)
    {
      try
      {
        // Create a new file name (e.g. Screenshot0.png, Screenshot1.png, ...)
        string filename = FileHelper.CreateUniqueFilename(settings.Directory, settings.FileName, settings.FileExtension);
        if (filename == null)
        {
          throw new DirectoryNotFoundException(String.Format("Directory {0} could not be found", settings.Directory));
        }

        filename = Path.Combine(settings.Directory, Path.ChangeExtension(filename, settings.FileExtension));

        // Render and save
        EditorManager.EngineManager.SaveScreenShot(filename, settings.ViewportWidth, settings.ViewportHeight, settings.FovX, settings.FovY);

        if (settings.OpenExternally)
        {
          // Open the screenshot file with the associated external program
          Process.Start(filename);
        }
      }
      catch (Exception ex)
      {
        EditorManager.DumpException(ex);
        EditorManager.ShowMessageBox("Saving screenshot failed.\n\nDetailed message:\n" + ex.Message, "Saving screenshot", MessageBoxButtons.OK, MessageBoxIcon.Error);
      }
    }


    private void buttonOutputDir_Click(object sender, EventArgs e)
    {
      FolderBrowserDialog dialog = new FolderBrowserDialog();
      dialog.SelectedPath = textBoxOutputDir.Text;

      if (dialog.ShowDialog() == DialogResult.OK)
      {
        textBoxOutputDir.Text = dialog.SelectedPath;
      }
    }

    private void buttonSaveScreenShot_Click(object sender, EventArgs e)
    {
      SaveScreenshot(_settings);
    }

    private void buttonOK_Click(object sender, EventArgs e)
    {
      // Save the changed settings
      EditorSceneSettings newSettings = EditorApp.Scene.Settings.Clone() as EditorSceneSettings;
      newSettings.ScreenCaptureSettings = _settings;
      EditorManager.Actions.Add(new SceneSettingsChangedAction(EditorApp.Scene.Settings, newSettings));

      DialogResult = DialogResult.OK;
    }

    private void buttonCancel_Click(object sender, EventArgs e)
    {
      DialogResult = DialogResult.Cancel;
    }

    private ScreenshotSettings _settings;

    private void radioButtonCurrentViewPort_CheckedChanged(object sender, EventArgs e)
    {
      _settings.UseCustomViewport = radioButtonCustomViewport.Checked;
      RefreshGUI();
    }

    private void textBoxOutputDir_TextChanged(object sender, EventArgs e)
    {
      _settings.Directory = textBoxOutputDir.Text;
      RefreshGUI();
    }

    private void textBoxFileName_TextChanged(object sender, EventArgs e)
    {
      _settings.FileName = textBoxFileName.Text;
      RefreshGUI();
    }

    private void comboBoxExtension_SelectedIndexChanged(object sender, EventArgs e)
    {
      _settings.FileExtension = comboBoxExtension.SelectedItem.ToString();
      RefreshGUI();
    }

    private void numericUpDownResX_ValueChanged(object sender, EventArgs e)
    {
      _settings.ViewportWidth = (int)numericUpDownResX.Value;
      RefreshGUI();
    }

    private void numericUpDownResY_ValueChanged(object sender, EventArgs e)
    {
      _settings.ViewportHeight = (int)numericUpDownResY.Value;
      RefreshGUI();
    }

    private void numericUpDownFovX_ValueChanged(object sender, EventArgs e)
    {
      _settings.FovX = (float)numericUpDownFovX.Value;
      RefreshGUI();
    }

    private void numericUpDownFovY_ValueChanged(object sender, EventArgs e)
    {
      _settings.FovY = (float)numericUpDownFovY.Value;
      RefreshGUI();
    }

    private void checkBoxOpenFile_CheckedChanged(object sender, EventArgs e)
    {
      _settings.OpenExternally = checkBoxOpenFile.Checked;
      RefreshGUI();
    }
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
