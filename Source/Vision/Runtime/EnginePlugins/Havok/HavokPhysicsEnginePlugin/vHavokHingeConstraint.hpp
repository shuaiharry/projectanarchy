/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

#ifndef V_HAVOK_HINGE_CONSTRAINT_HPP_INCLUDED
#define V_HAVOK_HINGE_CONSTRAINT_HPP_INCLUDED

#include <Vision/Runtime/EnginePlugins/Havok/HavokPhysicsEnginePlugin/vHavokConstraint.hpp>

/// \brief
///   Descriptor for vHavokPhysics Hinge constraint.
class vHavokHingeConstraintDesc : public vHavokConstraintDesc
{
public:
  /// \brief
  ///   Constructor
  VHAVOK_IMPEXP vHavokHingeConstraintDesc();

  /// \brief
  ///   Sets the default value for all members.
  VHAVOK_IMPEXP virtual void Reset();

  ///
  /// @name Serialization and Type Management
  /// @{
  ///

  V_DECLARE_SERIAL_DLLEXP(vHavokHingeConstraintDesc, VHAVOK_IMPEXP)
  VHAVOK_IMPEXP VOVERRIDE void Serialize(VArchive &ar);

  ///
  /// @}
  ///

public:
  hkvVec3 m_vHingePivot;  ///< The hinge's pivot point in world space
  hkvVec3 m_vRotAxis;     ///< Hinge's rotation axis
  bool m_bUseLimits;      ///< Whether to use the rotation's angle limits or not
  hkReal m_fAngleMin;     ///< The minimum angle of rotation
  hkReal m_fAngleMax;     ///< The maximum angle of rotation

private:
  static const unsigned int s_iSerialVersion;
};

// ----------------------------------------------------------------------------
// vHavokHingeConstraint
// ----------------------------------------------------------------------------

/// \brief
///   Implementation of the vHavok Hinge constraint.
class vHavokHingeConstraint : public vHavokConstraint
{
public:
  ///
  /// @name Serialization and Type Management
  /// @{
  ///

  V_DECLARE_SERIAL_DLLEXP(vHavokHingeConstraint, VHAVOK_IMPEXP)
  VHAVOK_IMPEXP VOVERRIDE void Serialize(VArchive &ar);

  ///
  /// @}
  ///

protected:
  ///
  /// @name Base Class Overrides
  /// @{
  ///

  VHAVOK_IMPEXP virtual hkpConstraintData* CreateConstraintData() HKV_OVERRIDE;

  VHAVOK_IMPEXP virtual vHavokConstraintDesc *CreateConstraintDesc() HKV_OVERRIDE;

  VHAVOK_IMPEXP virtual void InitConstraintDataFromDesc(hkpConstraintData& data, 
    vHavokConstraintDesc const& desc) HKV_OVERRIDE;

  VHAVOK_IMPEXP virtual void ApplyCustomArchiveTransformationToJointDesc(vHavokConstraintDesc *pDesc, 
    hkvVec3& vCustomArchivePositionOfs, hkvMat3 &vCustomArchiveRotationOfs) HKV_OVERRIDE;

public:
  VHAVOK_IMPEXP virtual void SaveToDesc(vHavokConstraintDesc& desc) HKV_OVERRIDE;

  ///
  /// @}
  ///

private:
  static const unsigned int s_iSerialVersion;

  hkvVec3 m_vSavedPivot;  ///< Saved hinge pivot point (world space)
  hkvVec3 m_vSavedAxis;   ///< Saved hinge pivot axis (world space)
  bool m_bUseLimits;      ///< Whether angle limts are active

};

#endif // V_HAVOK_HINGE_CONSTRAINT_HPP_INCLUDED

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
