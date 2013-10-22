/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

using System;
using System.IO;
using System.Net;
using System.Reflection;
using System.Text;
using System.Web.Script.Serialization;
using System.Windows.Forms;
using CSharpFramework;

namespace Editor.Dialogs
{
  public partial class UpdateDlg : Form
  {
    public struct AnarchyVersionInfo
    {
      public string currentVersion;
      public string updateURL;
      public string releaseText;
    }

    private string _updateUrl;
    private bool _askLater = false;

    public bool AskLater { get { return _askLater; } }

    private string RemoveHtml(string htmlString)
    {
      string res = htmlString;
      res = res.Replace("</p>", "\n");
      res = res.Replace("<li>", "* ");
      res = res.Replace("</li>", "\n");

      res = res.Replace("</P>", "\n");
      res = res.Replace("<LI>", "* ");
      res = res.Replace("</LI>", "\n");

      char[] array = new char[res.Length];
      int arrayIndex = 0;
      bool inside = false;

      for (int i = 0; i < res.Length; i++)
      {
        char let = res[i];
        if (let == '<')
        {
          inside = true;
          continue;
        }
        if (let == '>')
        {
          inside = false;
          continue;
        }
        if (!inside)
        {
          array[arrayIndex] = let;
          arrayIndex++;
        }
      }
      return new string(array, 0, arrayIndex);
    }

    public UpdateDlg(AnarchyVersionInfo versionInfo)
    {
      InitializeComponent();

      _updateUrl = versionInfo.updateURL;

      currentVersion.Text = versionInfo.currentVersion;

      Version thisVersion = Assembly.GetEntryAssembly().GetName().Version;
      yourVersion.Text = String.Format("Your version is {0}\n\nThe new version includes the following features:", thisVersion);

      releaseText.Text = RemoveHtml(versionInfo.releaseText);
    }


    public static void CheckForUpdates()
    {
      string lastKnownVersionToken = Path.Combine(EditorManager.GetDirectory(EditorManager.DirectoryType.UserDataPath), "LastKnownVersion.token");

      WebClient webClient = new WebClient();
      webClient.DownloadStringCompleted += (sender, eventArgs) =>
      {
        try
        {
          if (eventArgs.Error != null)
            return;

          string json = eventArgs.Result.Trim(new char[] { ' ', '\t', '\n', '[', ']' });

          JavaScriptSerializer serializer = new JavaScriptSerializer();
          var anarchyVersionInfo = serializer.Deserialize<AnarchyVersionInfo>(json);
          Version versionOnServer = new Version(anarchyVersionInfo.currentVersion);

          Version lastKnownVersion;
          if (File.Exists(lastKnownVersionToken))
            lastKnownVersion = new Version(File.ReadAllText(lastKnownVersionToken));
          else
            lastKnownVersion = Assembly.GetEntryAssembly().GetName().Version;

          if (lastKnownVersion < versionOnServer)
          {
            UpdateDlg dlg = new UpdateDlg(anarchyVersionInfo);
            dlg.ShowDialog();

            if (dlg.AskLater)
            {
              File.WriteAllText(lastKnownVersionToken, versionOnServer.ToString());
            }
          }
        }
        catch
        {
        }
      };

      try
      {
        webClient.DownloadStringAsync(new Uri("http://www.projectanarchy.com/svc/version.json"));
      }
      catch
      {
      }
    }

    private void downloadButton_Click(object sender, EventArgs e)
    {
      System.Diagnostics.Process.Start(_updateUrl);
      Close();
    }

    private void ignoreButton_Click(object sender, EventArgs e)
    {
      _askLater = true;
      Close();
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
