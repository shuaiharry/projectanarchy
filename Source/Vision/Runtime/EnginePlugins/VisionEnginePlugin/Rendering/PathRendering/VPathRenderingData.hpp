/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

/// \file VPathRenderingData.hpp

#ifndef VPATHRENDERINGDATA_H_INCLUDED
#define VPATHRENDERINGDATA_H_INCLUDED

#include <Vision/Runtime/EnginePlugins/VisionEnginePlugin/Rendering/PathRendering/IVPathRenderingData.hpp>
#include <Vision/Runtime/Engine/SceneElements/VisApiPath.hpp>

/// \brief
///   Adpater class used to provide rendering data for VisPath_cl object.
class VPathRenderingData : public IVPathRenderingData
{
public:
  /// \brief
  ///   Default constructor.
  PATHRND_IMPEXP VPathRenderingData();

  /// \brief
  ///   Destructor.
  PATHRND_IMPEXP virtual ~VPathRenderingData();

public:
  /// \brief
  ///   Sets the object instance containing the rendering data
  /// \param pObject
  ///   Typed Object that contains the path data.
  /// \return
  ///   Whether the set object is compatible with this adapter.
  PATHRND_IMPEXP virtual bool SetPathObject(VTypedObject* pObject) HKV_OVERRIDE;

  /// \brief
  ///   Returns the diameter of the path.
  PATHRND_IMPEXP virtual float GetDiameter() const HKV_OVERRIDE;

  /// \brief
  ///   Returns the length of each link of the path.
  PATHRND_IMPEXP virtual float GetLinkLength() const HKV_OVERRIDE;

  /// \brief
  ///   Returns the gap between the links of the path.
  PATHRND_IMPEXP virtual float GetLinkGap() const HKV_OVERRIDE;

  /// \brief
  ///   Returns the number of links of this path.
  PATHRND_IMPEXP virtual unsigned int GetNumLinks() const HKV_OVERRIDE;

  /// \brief
  ///   Returns the amount (in world units) by which the last path link is 
  ///   shorter or longer than required. The value is negative if the link
  ///   is longer, and positive if the link is shorter.
  PATHRND_IMPEXP virtual float GetLastLinkExtra() const HKV_OVERRIDE;

  /// \brief
  ///   Inquires the transformation of the specified link in world space.
  /// \param iIndex
  ///   index of the link whose transformation to get
  /// \param mRotation
  ///   output parameter; receives the rotation of the link
  /// \param vTranslation
  ///   output parameter; receives the translation of the link
  /// \return
  ///   \c true if a valid transformation could be determined, \c false 
  ///   if not. In the latter case, the contents of mRotation and vTranslation
  ///   are undefined.
  PATHRND_IMPEXP virtual bool GetLinkTransform(unsigned int iIndex, hkvMat3& mRotation, hkvVec3& vTranslation) const HKV_OVERRIDE;

  /// \brief
  ///   Inquires the transformation of all links in world space, optionally
  ///   removing the roll values or reversing the link order.
  /// \param pRotations
  ///   pointer to an array of rotation matrices. This array must be allocated
  ///   by the caller and be at least of the size returned by GetNumLinks().
  /// \param pTranslations
  ///   pointer to an array of translation vectors. This array must be allocated
  ///   by the caller and be at least of the size returned by GetNumLinks().
  /// \param bRemoveRoll
  ///   whether to remove the roll component in the output transforms
  /// \param bReverseOrder
  ///   whether to output the link transforms in reverse order, and with a yaw
  ///   of 180 degrees applied
  PATHRND_IMPEXP virtual void GetLinkTransforms(hkvMat3* pRotations, hkvVec3* pTranslations, bool bRemoveRoll, bool bReverseOrder) const HKV_OVERRIDE;

  /// \brief
  ///   Returns whether the path object has changed since last initialization. Needed to syncronize path renderer with path object.
  PATHRND_IMPEXP virtual bool HasDataChanged() HKV_OVERRIDE;

protected:
  /// \brief
  ///   Gets called after all components of the owner object are attached and initialized.
  /// \param pOwner
  ///   Owner object that contains the path data.
  PATHRND_IMPEXP virtual void Init(VisTypedEngineObject_cl* pOwner) HKV_OVERRIDE;

private:

#ifndef _VISION_DOC

	class VLinkTransform : public VRefCounter
	{
	public:
		hkvMat3 m_mRotation;
		hkvVec3 m_vPosition;
	};
	typedef VSmartPtr<VLinkTransform> VLinkTransformPtr;

  unsigned int CalcPivotPoints(DynObjArray_cl<hkvVec3>& pivots);
  VLinkTransform* CreateLinkTransform(const hkvVec3& vPivot1, const hkvVec3& vPivot2) const;
  void RemoveLinkRoll(hkvMat3 const& mPrevRot, hkvMat3& mRot, bool& bDirection) const;

#endif //_VISION_DOC

public:
  V_DECLARE_SERIAL_DLLEXP(VPathRenderingData, PATHRND_IMPEXP)

protected:
  VisPathPtr m_spPathObject;
  float m_fDiameter;
  float m_fLinkLength;
  float m_fLinkGap;
  unsigned int m_iNumLinks;
  float m_fLastLinkExtra;
  int m_iLastPathUpdateFrame;
  DynObjArray_cl<VSmartPtr<VLinkTransform> > m_LinkTransforms; ///< The list of link transformations.
};


/// \brief
///   Component class that provides additional properties like diameter, link length and link gap needed for rendering a
///   path of type VisPath_cl. Note when this component is not attached to an VisPath_cl instance, default parameters are
///   used for rendering.
class VPathRenderingMetaData : public IVObjectComponent
{
public:
  /// \brief
  ///   Constructor for this path rendering meta data component.
  /// \param iComponentFlags
  ///   Component bit flags that define the behavior of the component.
  /// \see
  ///   IVObjectComponent::IVObjectComponent
  PATHRND_IMPEXP VPathRenderingMetaData(int iComponentFlags = VIS_OBJECTCOMPONENTFLAG_NONE);

  /// \brief
  ///   Destructor; removes the renderer if it's still active.
  PATHRND_IMPEXP virtual ~VPathRenderingMetaData();

public:

#ifndef _VISION_DOC

  // Base Class Overrides
  PATHRND_IMPEXP virtual BOOL CanAttachToObject(VisTypedEngineObject_cl* pObject, VString& sErrorMsgOut) HKV_OVERRIDE;
  PATHRND_IMPEXP virtual void OnVariableValueChanged(VisVariable_cl* pVar, const char* value) HKV_OVERRIDE;

  /// Serialization
  V_DECLARE_SERIAL_DLLEXP(VPathRenderingMetaData, PATHRND_IMPEXP)
  V_DECLARE_VARTABLE(VPathRenderingMetaData, PATHRND_IMPEXP)
  PATHRND_IMPEXP virtual void Serialize(VArchive& ar) HKV_OVERRIDE;

#endif //_VISION_DOC

protected:
  // Attributes exposed to vForge
  float Diameter;
  float LinkLength;
  float LinkGap;

  friend class VPathRenderingData;
};


#endif //VPATHRENDERINGDATA_H_INCLUDED

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
