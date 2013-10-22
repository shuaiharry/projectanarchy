/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

using System;
using System.IO;
using System.Runtime.Serialization;
using CSharpFramework;

namespace Editor
{
  [Serializable]
  public class ScreenshotSettings : ISerializable, ICloneable
  {
    /// <summary>
    /// Constructor
    /// </summary>
    public ScreenshotSettings()
    {
      Directory = Path.GetDirectoryName(EditorApp.Project.PathName);
      FileName = "ScreenShot";
      FileExtension = ".png";
      UseCustomViewport = false;
      OpenExternally = true;
    }

    #region ISerializable

    /// <summary>
    /// Serializing constructor
    /// </summary>
    ScreenshotSettings(SerializationInfo info, StreamingContext context)
    {
      Directory = info.GetString("_directory");
      FileName = info.GetString("_fileName");
      FileExtension = info.GetString("_fileExtension");
      _fovX = info.GetSingle("_fovX");
      UseCustomViewport = info.GetBoolean("_useCustomViewport");
      _viewportHeight = info.GetInt32("_viewportHeight");
      _viewportWidth = info.GetInt32("_viewportWidth");
      OpenExternally = info.GetBoolean("_openExternally");
    }

    public void GetObjectData(SerializationInfo info, StreamingContext context)
    {
      info.AddValue("_directory", Directory);
      info.AddValue("_fileName", FileName);
      info.AddValue("_fileExtension", FileExtension);
      info.AddValue("_fovX", _fovX);
      info.AddValue("_useCustomViewport", UseCustomViewport);
      info.AddValue("_viewportHeight", _viewportHeight);
      info.AddValue("_viewportWidth", _viewportWidth);
      info.AddValue("_openExternally", OpenExternally);
    }

    #endregion ISerializable

    #region ICloneable
    /// <summary>
    /// Clone
    /// </summary>
    public object Clone()
    {
      return MemberwiseClone();
    }
    #endregion ICloneable

    #region Properties

    /// <summary>
    /// Directory path to store screenshot files.
    /// </summary>
    public string Directory { get; set; }

    /// <summary>
    /// File name for screenshot files.
    /// </summary>
    public string FileName { get; set; }

    /// <summary>
    /// File extension for screenshot files.
    /// </summary>
    public string FileExtension { get; set; }

    /// <summary>
    /// Open screenshot files with the associated external program after saving.
    /// </summary>
    public bool OpenExternally { get; set; }

    /// <summary>
    /// Whether to use a custom viewport size, or the current one for creating screenshots.
    /// </summary>
    public bool UseCustomViewport { get; set; }

    /// <summary>
    /// The width of the created screenshot.
    /// </summary>
    public int ViewportWidth
    {
      get
      {
        if (UseCustomViewport)
        {
          return _viewportWidth;
        }
        else
        {
          return EditorManager.ActiveView.Width;
        }
      }

      set
      {
        _viewportWidth = value;
      }
    }

    /// <summary>
    /// The height of the created screenshot.
    /// </summary>
    public int ViewportHeight
    {
      get
      {
        if (UseCustomViewport)
        {
          return _viewportHeight;
        }
        else
        {
          return EditorManager.ActiveView.Height;
        }
      }

      set
      {
        _viewportHeight = value;
      }
    }

    /// <summary>
    /// The horizontal field of view for the created screenshot.
    /// </summary>
    public float FovX
    {
      get
      {
        if (UseCustomViewport)
        {
          return _fovX;
        }
        else
        {
          return (EditorApp.Scene.MainLayer as V3DLayer).FOV;
        }
      }
      set
      {
        _fovX = value;
      }
    }

    /// <summary>
    /// The vertical field of view for the created screenshot. Autocomputed from FovX.
    /// </summary>
    public float FovY
    {
      get
      {
        // See hkvMathHelpers::getFovY.
        float fAspectRatio = (float)ViewportWidth / (float)ViewportHeight;
        return (float)(2.0f * Math.Atan(Math.Tan(0.5f * FovX / 180.0f * Math.PI) / fAspectRatio) * 180.0f / Math.PI);
      }
      set
      {
        // See hkvMathHelpers::getFovX.
        float fAspectRatio = (float)ViewportWidth / (float)ViewportHeight;
        FovX = (float)(2.0f * Math.Atan(Math.Tan(0.5f * value / 180.0f * Math.PI) * fAspectRatio) * 180.0f / Math.PI);
      }
    }

    private int _viewportWidth = 4096;
    private int _viewportHeight = 4096;
    private float _fovX = 90.0f;
  }

    #endregion Properties
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
