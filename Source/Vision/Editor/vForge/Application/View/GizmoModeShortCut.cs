/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

using System;
using CSharpFramework;
using CSharpFramework.View;
using System.Windows.Forms;
using CSharpFramework.ShortCuts;
using CSharpFramework.Scene;

namespace Editor.View
{
  #region GizmoModeShortCut

	/// <summary>
	/// Implements ShortCutAction to set a new Gizmo mode (rotate, move,..)
	/// </summary>
	public class GizmoModeShortCut : ShortCutAction
	{
    /// <summary>
    /// Constructor
    /// </summary>
    /// <param name="defaultKey"></param>
    /// <param name="dragMode"></param>
    /// <param name="alternativeBinding">Assigns an alternative keybinding (enforced by returning a diffent value from ActionName)</param>
    public GizmoModeShortCut(Keys defaultKey, ShapeDragMode dragMode, bool alternativeBinding) : base(defaultKey)
    {
      _newMode = dragMode;
      _alternativeBinding = alternativeBinding;
    }

    /// <summary>
    /// Overridden function
    /// </summary>
    public override bool Enabled
    {
      get
      {
        return EditorManager.Scene != null;
      }
    }

    /// <summary>
    /// Overridden function
    /// </summary>
    /// <returns></returns>
    public override bool Do()
    {
      if (EditorManager.ActiveView==null || EditorManager.ActiveView.Gizmo==null)
        return false;

      // When the gizmo's mode already matches the pressed gizmo mode hotkey, toggle between local and global transform space
      if (_newMode == EditorManager.ActiveView.Gizmo.DragMode)
      {
        switch (_newMode)
        {
          case ShapeDragMode.MOVE:
            EditorManager.ActiveView.Gizmo.LocalTranslation = !EditorManager.ActiveView.Gizmo.LocalTranslation;
            break;
          case ShapeDragMode.ROTATE:
            EditorManager.ActiveView.Gizmo.LocalOrientation = !EditorManager.ActiveView.Gizmo.LocalOrientation;
            break;
          case ShapeDragMode.SCALE:
          case ShapeDragMode.UNIFORMSCALE:
            EditorManager.ActiveView.Gizmo.LocalScaling = !EditorManager.ActiveView.Gizmo.LocalScaling;
            break;
          default:
            break;
        }
      }

      if (_newMode == ShapeDragMode.SCALE || _newMode == ShapeDragMode.UNIFORMSCALE)
      {
        if (EditorManager.ActiveView.Gizmo.UseUniformScaling)
          EditorManager.ActiveView.Gizmo.DragMode = ShapeDragMode.UNIFORMSCALE;
        else
          EditorManager.ActiveView.Gizmo.DragMode = ShapeDragMode.SCALE;
      }
      else
      {
        EditorManager.ActiveView.Gizmo.DragMode = _newMode;
      }
      return true;
    }

    /// <summary>
    /// Overridden function
    /// </summary>
    public override string ActionName
    {
      get
      {
        if (_alternativeBinding)
        {
          return "Gizmo Mode (Alternate): " + _newMode.ToString();
        }
        else
        {
          return "Gizmo Mode: " + _newMode.ToString();
        }
      }
    }

    /// <summary>
    /// Overridden function
    /// </summary>
    public override string Description
    {
      get
      {
        return "Sets the gizmo mode to "+_newMode.ToString();
      }
    }

    ShapeDragMode _newMode;
    bool _alternativeBinding;
  }

  /// <summary>
  /// Implements ShortCutAction to switch the Gizmo mode (rotate, move,..)
  /// </summary>
  public class GizmoSwitchShortCut : ShortCutAction
  {
    /// <summary>
    /// Constructor
    /// </summary>
    /// <param name="defaultKey"></param>
    /// <param name="dragMode"></param>
    /// <param name="cycleForward">Specifices whether this particular shortcut cycles forward or backward through gizmo modes</param>
    /// <param name="alternativeBinding">Assigns an alternative keybinding (enforced by returning a diffent value from ActionName)</param>
    public GizmoSwitchShortCut(Keys defaultKey, bool cycleForward, bool alternateBinding)
      : base(defaultKey)
    {
      _cycleForward = cycleForward;
      _alternateBinding = alternateBinding;
    }

    /// <summary>
    /// Overridden function
    /// </summary>
    public override bool Enabled
    {
      get
      {
        return EditorManager.Scene != null;
      }
    }

    /// <summary>
    /// Overridden function
    /// </summary>
    /// <returns></returns>
    public override bool Do()
    {
      if (EditorManager.ActiveView == null || EditorManager.ActiveView.Gizmo == null)
        return false;

      // Cycle forward through gizmo modes (with wrapping)
      if (_cycleForward)
      {
        if (EditorManager.ActiveView.Gizmo.DragMode == ShapeDragMode.MOVE)
          EditorManager.ActiveView.Gizmo.DragMode = ShapeDragMode.ROTATE;
        else if (EditorManager.ActiveView.Gizmo.DragMode == ShapeDragMode.ROTATE)
          EditorManager.ActiveView.Gizmo.DragMode = ShapeDragMode.UNIFORMSCALE;
        else if (EditorManager.ActiveView.Gizmo.DragMode == ShapeDragMode.UNIFORMSCALE)
          EditorManager.ActiveView.Gizmo.DragMode = ShapeDragMode.SCALE;
        else if (EditorManager.ActiveView.Gizmo.DragMode == ShapeDragMode.SCALE)
          EditorManager.ActiveView.Gizmo.DragMode = ShapeDragMode.MOVE;
      }
	  // Cycle backward through gizmo modes (with wrapping)
      else
      {
        if (EditorManager.ActiveView.Gizmo.DragMode == ShapeDragMode.SCALE)
          EditorManager.ActiveView.Gizmo.DragMode = ShapeDragMode.UNIFORMSCALE;
        else if (EditorManager.ActiveView.Gizmo.DragMode == ShapeDragMode.UNIFORMSCALE)
          EditorManager.ActiveView.Gizmo.DragMode = ShapeDragMode.ROTATE;
        else if (EditorManager.ActiveView.Gizmo.DragMode == ShapeDragMode.ROTATE)
          EditorManager.ActiveView.Gizmo.DragMode = ShapeDragMode.MOVE;
        else if (EditorManager.ActiveView.Gizmo.DragMode == ShapeDragMode.MOVE)
          EditorManager.ActiveView.Gizmo.DragMode = ShapeDragMode.SCALE;
      }

      return true;
    }

    /// <summary>
    /// Overridden function
    /// </summary>
    public override string ActionName
    {
      get
      {
        // Provide unique ActionNames for each GizmoSwitchShortCut permutation
        if (_alternateBinding)
        {
          if (_cycleForward)
          {
            return "Gizmo Mode (Alternate): CYCLE MODE";
          }
          else
          {
            return "Gizmo Mode (Alternate): REVERSE-CYCLE MODE";
          }
        }
        else
        {
          if (_cycleForward)
          {
            return "Gizmo Mode: CYCLE MODE";
          }
          else
          {
            return "Gizmo Mode: REVERSE-CYCLE MODE";
          }
        }
      }
    }

    /// <summary>
    /// Overridden function
    /// </summary>
    public override string Description
    {
      get
      {
        if (_cycleForward)
        {
          return "Cycles the Gizmo Mode: Move > Rotate > Uniform Scale > Scale > Move ";
        }
        else
        {
          return "Reverse-cycles the Gizmo Mode: Move > Scale > Uniform Scale > Rotate > Move ";
        }
      }
    }

    bool _cycleForward;
    bool _alternateBinding;
  }

  #endregion

  #region ToggleLocalOrientation

  /// <summary>
  /// Special shortcut that toggles the local orientation status
  /// </summary>
  public class ToggleLocalOrientation : ShortCutAction
  {
    public ToggleLocalOrientation(Keys defaultKey)
      : base(defaultKey)
    {
    }

    public override string ActionName
    {
      get
      {
        return "Gizmo: Toggle local orientation";
      }
    }

    public override string Category
    {
      get
      {
        return base.Category;
      }
    }

    public override bool Enabled
    {
      get
      {
        return EditorManager.Scene!=null;
      }
    }

    public override string Description
    {
      get
      {
        return "Toggles the gizmo mode between local and world space orientation";
      }
    }

    public override bool Do()
    {
      if (EditorManager.ActiveView == null || EditorManager.ActiveView.Gizmo == null)
        return false;

      switch(EditorManager.ActiveView.Gizmo.DragMode)
      {
        case ShapeDragMode.MOVE:
          EditorManager.ActiveView.Gizmo.LocalTranslation = !EditorManager.ActiveView.Gizmo.LocalTranslation;
          break;
        case ShapeDragMode.ROTATE:
          EditorManager.ActiveView.Gizmo.LocalOrientation = !EditorManager.ActiveView.Gizmo.LocalOrientation;
          break;
        case ShapeDragMode.SCALE: // scale and uniformscale are the same case, so intended fall through
        case ShapeDragMode.UNIFORMSCALE:
          EditorManager.ActiveView.Gizmo.LocalScaling = !EditorManager.ActiveView.Gizmo.LocalScaling;
          break;
      }
      
      return true;
    }
  }

  #endregion

  #region Camera Style

  public class CameraStyleShortCut : ShortCutAction
  {
    public CameraStyleShortCut(Keys defaultKey, EditorSettingsBase.CameraStyle_e style)
      : base(defaultKey)
    {
      Style = style;
    }

    EditorSettingsBase.CameraStyle_e Style = EditorSettingsBase.CameraStyle_e.MaxStyle;

    public override string ActionName
    {
      get
      {
        return "Camera Style: " + Style.ToString();
      }
    }

    public override string Category
    {
      get
      {
        return base.Category;
      }
    }

    public override bool Enabled
    {
      get
      {
        return true;
      }
    }

    public override string Description
    {
      get
      {
        return "Sets the camera move style. Same as setting it in the vForge settings.";
      }
    }

    public override bool Do()
    {
      EditorManager.Settings.CameraStyle = Style;
      return true;
    }
  }

  #endregion

  #region Shape Selection Mode

  public class ShapeSelectionModeShortCut : ShortCutAction
  {
    public ShapeSelectionModeShortCut(Keys defaultKey, EditorSettingsBase.ShapeSelectionMode_e mode)
      : base(defaultKey)
    {
      Mode = mode;
    }

    EditorSettingsBase.ShapeSelectionMode_e Mode = EditorSettingsBase.ShapeSelectionMode_e.ActiveLayer;

    public override string ActionName
    {
      get
      {
        return "Shape Selection: " + Mode.ToString();
      }
    }

    public override string Category
    {
      get
      {
        return base.Category;
      }
    }

    public override bool Enabled
    {
      get
      {
        return true;
      }
    }

    public override string Description
    {
      get
      {
        return "Sets the shape selection filter mode. Same as setting it in the vForge settings.";
      }
    }

    public override bool Do()
    {
      EditorManager.Settings.ShapeSelectionMode = Mode;
      return true;
    }
  }

  #endregion

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
