/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

using System;
using System.IO;
using System.Windows.Forms;
using CSharpFramework;
using System.Linq;

namespace Editor.Dialogs
{
   public partial class EULA : Form
   {
      bool _bAcceptedEula = false;
      bool _bContainsValidEulaText = false;
      string _sEulaText;

      public bool HasAcceptedEula()
      {
         return _bAcceptedEula;
      }

      public bool ContainsValidEulaText()
      {
         return _bContainsValidEulaText;
      }

      public EULA()
      {
         InitializeComponent();

         string sEulaPath = System.IO.Path.GetDirectoryName(Application.ExecutablePath) + "\\..\\..\\..\\..\\Data\\Common\\Anarchy\\EULA.txt";
         _sEulaText = "<not set>";

         try
         {
            _sEulaText = System.IO.File.ReadAllText(sEulaPath);
            _bContainsValidEulaText = true;
         }
         catch (Exception)
         {
            _sEulaText = "Failed to read the EULA from:'\n" + sEulaPath + "\n" +
                         "Please make sure that you didn't delete this file.\n" +
                         "'Please check out the Anarchy SDK folders for the EULA text, or go to www.ProjectAnarchy.com for more details.";
            _bContainsValidEulaText = false;
         }

         EulaTextBox.Text = _sEulaText;
         EulaTextBox.LinkClicked += new LinkClickedEventHandler(EulaTextBox_LinkClicked);
      }

      /// <summary>
      /// Returns a MD5 hash string of the loaded EULA text.
      /// </summary>
      /// <returns>Empty string if there is currently now valid eula text</returns>
      public string GenerateEulaTextHash()
      {
         if (_bContainsValidEulaText)
         {
            byte[] hash = System.Security.Cryptography.MD5.Create().ComputeHash(System.Text.Encoding.UTF8.GetBytes(_sEulaText));
            return BitConverter.ToString(hash);
         }
         else
            return "";
      }

      protected override void Dispose(bool disposing)
      {
         if (disposing && (components != null))
         {
            components.Dispose();
         }
         base.Dispose(disposing);
      }

      void EulaTextBox_LinkClicked(System.Object sender, System.Windows.Forms.LinkClickedEventArgs e)
      {
         System.Diagnostics.Process.Start(e.LinkText);
      }

      private const int CP_NOCLOSE_BUTTON = 0x200;
      protected override CreateParams CreateParams
      {
         get
         {
            CreateParams myCp = base.CreateParams;
            myCp.ClassStyle = myCp.ClassStyle | CP_NOCLOSE_BUTTON;
            return myCp;
         }
      }

      private void AcceptEulaButton_Click(object sender, EventArgs e)
      {
         _bAcceptedEula = true;
         Close();
      }

      private void RejectEulaButton_Click(object sender, EventArgs e)
      {
         if (MessageBox.Show("Rejecting the EULA means you may not use Project Anarchy.\n\nDo you really want to reject the EULA?", "Project Anarchy EULA", MessageBoxButtons.YesNo, MessageBoxIcon.Question, MessageBoxDefaultButton.Button2) == DialogResult.No)
            return;

         Close();
      }

      public static bool ShowEula()
      {
         string sAcceptedEulaToken = Path.Combine(EditorManager.GetDirectory(EditorManager.DirectoryType.UserDataPath), "UserAcceptedEula.token");

         using (EULA EulaDlg = new EULA())
         {
            string sEulaHash = EulaDlg.GenerateEulaTextHash();

            // already confirmed an Eula
            if (System.IO.File.Exists(sAcceptedEulaToken) && EulaDlg.ContainsValidEulaText())
            {
               // check hash to confirm if it was exactly this eula
               try
               {
                  if (System.IO.File.ReadLines(sAcceptedEulaToken).First().Equals(sEulaHash))
                     return true;
               }
               catch
               {
                  // proceed normally
               }
            }


            EulaDlg.ShowDialog();

            if (!EulaDlg.HasAcceptedEula())
            {
               MessageBox.Show("You rejected the Project Anarchy EULA.\nvForge will now close.", "vForge", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
               return false;
            }
            else
            {
               try
               {
                  System.IO.File.WriteAllText(sAcceptedEulaToken, sEulaHash + "\nThe Project Anarchy EULA has been accepted by this user.");
               }
               catch (Exception)
               {
                  MessageBox.Show("Failed to write this file: \n'" + sAcceptedEulaToken + "'\nPlease make sure vForge has the necessary access rights to create this file.", "vForge", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
               }
            }
         }
         return true;
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
