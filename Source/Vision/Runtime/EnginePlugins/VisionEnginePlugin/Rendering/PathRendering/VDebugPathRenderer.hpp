/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

/// \file VDebugPathRenderer.hpp

#ifndef VDEBUGPATHRENDERER_H_INCLUDED
#define VDEBUGPATHRENDERER_H_INCLUDED

#include <Vision/Runtime/EnginePlugins/VisionEnginePlugin/Rendering/PathRendering/VPathRendererBase.hpp>


/// \brief
///   A simple debug path renderer that draws a line for each link of the constraint chain
class VDebugPathRenderer : public VPathRendererBase
{
public:
  /// \brief
  ///   Constructor for this debug path renderer component.
  /// \param iComponentFlags
  ///   Component bit flags that define the behavior of the component.
  /// \see
  ///   IVObjectComponent::IVObjectComponent
  PATHRND_IMPEXP VDebugPathRenderer(int iComponentFlags = VIS_OBJECTCOMPONENTFLAG_NONE);

  /// \brief
  ///   Component Destructor
  PATHRND_IMPEXP virtual ~VDebugPathRenderer();

protected:
  // Base Class Overrides
  virtual void OnRender(IVisCallbackDataObject_cl* pCallbackData) HKV_OVERRIDE;

public:
  // Serialization
  V_DECLARE_SERIAL_DLLEXP(VDebugPathRenderer, PATHRND_IMPEXP)
  V_DECLARE_VARTABLE(VDebugPathRenderer, PATHRND_IMPEXP)
  PATHRND_IMPEXP virtual void Serialize(VArchive& ar) HKV_OVERRIDE;

public:  
  // Attributes exposed to vForge
  VColorRef RenderColor; ///< Color in which the constraint chain is visualized
};


#endif //VDEBUGPATHRENDERER_H_INCLUDED

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
