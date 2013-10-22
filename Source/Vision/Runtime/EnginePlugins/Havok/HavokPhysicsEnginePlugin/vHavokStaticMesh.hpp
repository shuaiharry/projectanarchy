/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

/// \file vHavokStaticMesh.hpp

#ifndef VHAVOKSTATICMESH_HPP_INCLUDED
#define VHAVOKSTATICMESH_HPP_INCLUDED

#include <Common/Base/hkBase.h>
#include <Physics2012/Dynamics/Entity/hkpRigidBody.h>

#include <Vision/Runtime/EnginePlugins/Havok/HavokPhysicsEnginePlugin/vHavokPhysicsIncludes.hpp>

class vHavokPhysicsModule;
class VisStaticMeshInstance_cl;

///
/// \brief 
///   Representation of static mesh instances in the Havok Physics binding.
///
/// The vHavokStaticMesh object takes one or multiple Vision static mesh instances as an input,
/// creates a Havok Physics rigid body for these meshes and adds them into the Havok simulation.
///
/// Ultimatively, the Havok Physics module is responsible for the automatic creation and deletion of 
/// vHavokStaticMesh instances. It is not recommended to create vHavokStaticMesh instances in
/// custom game code.
/// 
/// Please note that the constructor alone does not add the object to the physics simulation. This
/// only done when the Init function is called.
///
class vHavokStaticMesh : public VRefCounter
{
public:
  ///
  /// @name Construction / Destruction
  /// @{
  ///

  ///
  /// \brief
  ///   Constructor.
  ///
  /// The constructor does not add the object to the Havok simulation. Use one of the 
  /// Init functions to add the object to the physics simulation.
  ///
  /// \param module
  ///   Reference of the Havok Physics Module.
  /// 
  VHAVOK_IMPEXP vHavokStaticMesh(vHavokPhysicsModule &module);

  ///
  /// \brief
  ///   Destructor. Deletes the Havok Physics rigid body if still alive.
  ///
  VHAVOK_IMPEXP virtual ~vHavokStaticMesh();

  ///
  /// \brief
  ///   Removes the static mesh from the simulation without necessarily deleting this instance.
  ///
  VHAVOK_IMPEXP virtual void DisposeObject();

  ///
  /// @}
  ///

  ///
  /// @name Initialization
  /// @{
  ///

  ///
  /// \brief
  ///   Initializes the object using a Vision static mesh instance and adds the physics
  ///   representation to the simulation.
  ///
  /// \param meshInstance
  ///   Vision mesh instance that shall be part of the Havok Physics simulation.
  ///
  VHAVOK_IMPEXP virtual void Init(VisStaticMeshInstance_cl &meshInstance);

  ///
  /// \brief
  ///   Initializes the object using Vision static mesh instance collection and 
  ///   adds the physics representation to the simulation.
  ///
  /// \param meshInstances
  ///   Vision mesh instance collection that shall be part of the Havok Physics simulation.
  ///
  VHAVOK_IMPEXP virtual void Init(VisStaticMeshInstCollection &meshInstances);

  ///
  /// @}
  ///

  ///
  /// @name Access to Havok Physics Internals
  /// @{
  ///

  ///
  /// \brief
  ///   Gets the internal Havok Physics rigid body instance (NULL if not yet initialized).
  ///
  /// \returns
  ///   Pointer to the internal Havok Physics Rigid Body (can be NULL).
  /// 
  inline hkpRigidBody *GetHkRigidBody() 
  { 
    return m_pRigidBody; 
  }

  ///
  /// \brief
  ///   Returns the Havok Physics shape that belongs to the rigid body instance (NULL if not yet initialized).
  /// 
  /// \returns
  ///   Pointer to the internal Havok Physics Rigid Shape (can be NULL).
  /// 
  VHAVOK_IMPEXP const hkpShape *GetHkShape() const; 

  ///
  /// @}
  ///

  ///
  /// @name Synchronization
  /// @{
  ///

  ///
  /// \brief
  ///   Retrieves the transformation of the static mesh(es) from Havok Physics and 
  ///   applies them to the Vision instances.
  ///
  /// The physics module takes care of updating the transformation during the physics simulation. You
  /// will usually not have to call this function manually.
  ///
  VHAVOK_IMPEXP void UpdateHavok2Vision();

  ///
  /// \brief
  ///   Retrieves the transformation of the static mesh(es) from Vision and 
  //    applies them to the Havok Physics representation.
  ///
  /// The physics module takes care of updating the transformation during the physics simulation. You
  /// will usually not have to call this function manually.
  ///
  VHAVOK_IMPEXP void UpdateVision2Havok();

  ///
  /// @}
  ///

  ///
  /// @name Access to Vision Internals
  /// @{
  ///

  ///
  /// \brief
  ///   Returns the number of the static mesh instances represented by this class.
  /// 
  /// \returns
  ///   Number of static meshes.
  ///
  VHAVOK_IMPEXP int GetNumWrappedStaticMeshes();

  /// \brief
  ///   Returns the number of valid static mesh instances wrapped by this instance.
  ///
  /// The number of valid static mesh instances may be lower than the number of wrapped
  /// static mesh instances. This is the case if this instance was initialized with a group 
  //  of mesh instances and some mesh instances have been removed in the meantime.
  ///
  /// \return
  ///   the number of valid static meshes
  ///
  VHAVOK_IMPEXP int GetNumValidStaticMeshes() const;

  /// \brief
  ///   Returns the static mesh instance at the given index in the internal list.
  ///
  /// \param index
  ///   Index of the static mesh instance in the internal list.
  /// \returns
  ///   Pointer to the static mesh instance at the given index in the internal list.
  ///
  /// \note
  ///   It is possible that a \c NULL pointer is returned. This is the case if a 
  ///   static mesh managed by this object has been removed in the meantime.
  ///
  VHAVOK_IMPEXP VisStaticMeshInstance_cl* GetWrappedStaticMesh(int index);


  /// \brief
  ///   Removes a static mesh instance from this object.
  ///
  /// \param pMesh
  ///   the static mesh instance to remove
  ///
  VHAVOK_IMPEXP void RemoveStaticMesh(VisStaticMeshInstance_cl *pMesh);

  ///
  /// @}
  ///

  ///
  /// @name Debug Functionality
  /// @{
  ///

  /// \brief
  ///   Enables or disabled debug rendering of the Havok Physics representation.
  VHAVOK_IMPEXP void SetDebugRendering (bool bEnable);

  ///
  /// @}
  ///

protected:
  ///
  /// \brief
  ///   Creates the Havok Physics shape and rigid body, and adds them to the simulation.
  ///
  void CreateHkRigidBody();

  ///
  /// \brief
  ///   Removes physics representation from the simulation and releases the rigid body reference.
  ///
  void RemoveHkRigidBody();

  ///
  /// \brief
  ///   Common deinitialisation code that is used both for DisposeObject and on destruction.
  ///
  void CommonDeinit();
  
  hkpRigidBody *m_pRigidBody;                   ///< Pointer to the internal Havok Physics rigid body instance
  bool m_bInitialized;                          ///< Indicates whether object has been initialized with one of the Init functions.

  VisStaticMeshInstCollection m_staticMeshes;   ///< List of Vision static mesh instances represented by this class. 
                                                ///< Not reference-counted in order to avoid cyclic dependencies and resource cleanup warnings.
  vHavokPhysicsModule &m_module;                ///< Reference to the physics module this object belongs to.
  int m_iNumValidStaticMeshes;                  ///< Number of valid static mesh instances managed by this instance.
  hkvVec3 m_vScale;								              ///< The scale this static mesh was created with.
  const char *m_szShapeCacheId;                 ///< ID of shape of the rigid body in runtime cache table (points to memory in cache table). 
};

#endif // VHAVOKSTATICMESH_HPP_INCLUDED

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
