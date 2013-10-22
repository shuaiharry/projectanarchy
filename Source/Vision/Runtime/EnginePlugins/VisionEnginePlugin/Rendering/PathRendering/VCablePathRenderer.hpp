/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

/// \file VCablePathRenderer.hpp

#ifndef VCABLEPATHRENDERER_H_INCLUDED
#define VCABLEPATHRENDERER_H_INCLUDED

#include <Vision/Runtime/EnginePlugins/VisionEnginePlugin/Rendering/PathRendering/VPathRendererBase.hpp>

// Forward declarations
class VisMeshBuffer_cl;


/// \brief
///   Entity class for holding the generated model of a path chain-based cable.
///
/// This class is responsible for transferring the state of the path chain's link bodies to the bones used for animating the model.
class VCableChainEntity : public VisBaseEntity_cl
{
  V_DECLARE_SERIAL_DLLEXP(VCableChainEntity, PATHRND_IMPEXP);

public:
  /// \brief
  ///   Default constructor
  PATHRND_IMPEXP VCableChainEntity();

public:
  /// \brief
  ///   Sets the rendering data used for updating the bone state
  /// \param pRenderinData
  ///   the rendering data to set
  PATHRND_IMPEXP void SetRenderingData(IVPathRenderingData* pRenderinData);

public:
  PATHRND_IMPEXP virtual void ThinkFunction() HKV_OVERRIDE;
  PATHRND_IMPEXP virtual void EditorThinkFunction() HKV_OVERRIDE { ThinkFunction(); }

private:
  IVPathRenderingDataPtr m_spPathRenderingData; ///< The path rendering data used to update the model
  unsigned int m_iLastKnownNumLinks;            ///< The last known number of chain links
  DynArray_cl<hkvVec3> m_LinkPositions;         ///< Link position buffer
  DynArray_cl<hkvMat3> m_LinkRotations;         ///< Link rotation buffer
};


/// \brief
///   A path chain renderer that renders the path chain as a contiguous cable.
class VCablePathRenderer : public VPathRendererBase
{
public:
  /// \brief
  ///   Constructor for this cable path renderer component.
  /// \param iComponentFlags
  ///   Component bit flags that define the behavior of the component.
  /// \see
  ///   IVObjectComponent::IVObjectComponent
  PATHRND_IMPEXP VCablePathRenderer(int iComponentFlags = VIS_OBJECTCOMPONENTFLAG_NONE);

  /// \brief
  ///   Component Destructor
  PATHRND_IMPEXP virtual ~VCablePathRenderer();

protected:
  // Base Class Overrides
  virtual void CommonDeinit() HKV_OVERRIDE;
  virtual bool DoInit() HKV_OVERRIDE;
  virtual void OnUpdate() HKV_OVERRIDE;
  virtual void OnDataChanged() HKV_OVERRIDE;
  virtual void OnVariableValueChanged(VisVariable_cl* pVar, const char* value) HKV_OVERRIDE;

private:
  // Model/Entity Handling
  bool RebuildModel();

public:
  // Serialization
  V_DECLARE_SERIAL_DLLEXP(VCablePathRenderer, PATHRND_IMPEXP)
  V_DECLARE_VARTABLE(VCablePathRenderer, PATHRND_IMPEXP)
  PATHRND_IMPEXP virtual void Serialize(VArchive& ar) HKV_OVERRIDE;

public:  
  // Attributes exposed to vForge
  int VerticesPerRing;     ///< Number of vertices in each of the rings that form the cable's outline
  int RingsPerLink;        ///< Number of vertex rings defining the cable's outline for each chain link
  VString ModelFile;       ///< Model file from which the material definition for the cable's surface is taken
  BOOL CastDynamicShadows; ///< If enabled dynamic shadow is rendered for cable

private:
  // Internal Attributes
  VDynamicMeshPtr m_spChainMesh;                ///< The generated cable mesh
  VSmartPtr<VCableChainEntity> m_spChainEntity; ///< The entity for updating the cable bone state
  unsigned int m_iLastKnownNumLinks;            ///< The last known number of chain links
};


#endif //VCABLEPATHRENDERER_H_INCLUDED

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
