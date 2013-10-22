/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

/// \file vHavokShapeFactory.hpp

#ifndef VHAVOKSHAPEFACTORY_HPP_INCLUDED
#define VHAVOKSHAPEFACTORY_HPP_INCLUDED

#include <Vision/Runtime/EnginePlugins/Havok/HavokPhysicsEnginePlugin/vHavokPhysicsModule.hpp>
#include <Vision/Runtime/EnginePlugins/Havok/HavokPhysicsEnginePlugin/vHavokPhysicsIncludes.hpp>
#include <Vision/Runtime/EnginePlugins/Havok/HavokPhysicsEnginePlugin/vHavokRigidBody.hpp>

#include<Common/Base/Container/StringMap/hkStorageStringMap.h>

#define HKVIS_MESH_SHAPE_TOLERANCE 0.1f
#define HKVIS_CONVEX_SHAPE_TOLERANCE 0.01f

typedef hkStorageStringMap<hkpShape*> vHavokShapeCache;

/// 
/// \brief
///   Factory class with static functions to create Havok shapes.
/// 
class vHavokShapeFactory
{
public:

  ///
  /// @name Init/ Deinit
  /// @{
  ///

  /// 
  /// \brief
  ///   Initializes shape factory.
  /// 
  static void Init();

  /// 
  /// \brief
  ///   Deinitializes shape factory.
  /// 
  static void Deinit();

  ///
  /// @}
  ///

  ///
  /// @name Havok Shape Creation
  /// @{
  ///

  /// 
  /// \brief
  ///   Enumeration of Havok Physics shape creation flags
  ///
  enum VShapeCreationFlags_e
  {
    VShapeCreationFlags_CACHE_SHAPE         = V_BIT(0), ///< Allow runtime/ disc shape caching 
    VShapeCreationFlags_USE_VCOLMESH        = V_BIT(1), ///< Try using collision mesh (.vcolmesh)
    VShapeCreationFlags_SHRINK              = V_BIT(2), ///< Shrink by the convex radius
    VShapeCreationFlags_ALLOW_PERTRICOLINFO = V_BIT(3)  ///< Allow per triangle collision info for static mesh shapes
  };

  /// 
  /// \brief
  ///   Factory function to create a Havok Physics shape based on a convex hull (hkvConvexVerticesShape).
  /// 
  /// This function takes the vertex data of the given mesh including the corresponding collision mesh
  /// and creates a convex geometry which is used for the Havok Physics shape representation. This shape gets cached 
  /// on disk relative to the mesh's file name, also taking the scaling into account.
  ///
  /// \param pMesh
  ///   Input: Pointer to the mesh.
  ///
  /// \param vScale
  ///   Input: Scaling vector for the Havok Physics convex shape.
  ///
  /// \param szShapeCacheId
  ///   Output: ID string of corresponding shape into the cache table (points to memory in cache table).
  ///   NULL if shape could not be retrieved.
  ///
  /// \param iCreationFlags
  ///   Input: Bit mask for multiple VShapeCreationFlags_e flags.
  ///
  /// \returns
  ///   Pointer to Havok Physics shape based on a convex hull.
  /// 
  VHAVOK_IMPEXP static hkRefNew<hkpShape> CreateConvexHullShapeFromMesh(VBaseMesh* pMesh, const hkvVec3& vScale, 
    const char **szShapeCacheId, int iCreationFlags = (VShapeCreationFlags_CACHE_SHAPE | VShapeCreationFlags_USE_VCOLMESH));

  /// 
  /// \brief
  ///   Factory function to create a Havok Physics shape based on the given mesh data (hkvBvCompressedMeshShape).
  /// 
  /// This function takes the geometry data of the given mesh including the corresponding collision mesh
  /// and creates a Havok Physics mesh shape. This shape gets cached 
  /// on disk relative to the mesh's file name, also taking the scaling, collision behavior and welding type into account.
  ///
  /// \param pMesh
  ///   Input: Pointer to the mesh.
  ///
  /// \param vScale
  ///   Input: Scaling vector for the Havok Physics mesh shape.
  ///
  /// \param szShapeCacheId
  ///   Output: ID string of corresponding shape for the cache table (points to memory in cache table), 
  ///   if shape could be retrieved.
  ///
  /// \param iCreationFlags
  ///   Input: Bit mask for VShapeCreationFlags_e flags.
  ///
  /// \param eWeldingType
  ///   Input: Specifies the welding type. 
  ///      
  /// \returns
  ///   Pointer to Havok Physics shape based on the given mesh data.
  /// 
  VHAVOK_IMPEXP static hkRefNew<hkpShape> CreateShapeFromMesh(VBaseMesh* pMesh, const hkvVec3& vScale, 
    const char **szShapeCacheId, int iCreationFlags = (VShapeCreationFlags_CACHE_SHAPE | VShapeCreationFlags_USE_VCOLMESH), 
    VisWeldingType_e eWeldingType=VIS_WELDING_TYPE_NONE);

  /// 
  /// \brief
  ///   Factory function to create a Havok Physics shape based on a Havok BvCompressedMesh Shape (hkvBvCompressedMeshShape).
  /// 
  /// \param meshInstances
  ///   Input: Collection of static meshes the hkvBvCompressedMeshShape gets created for.
  ///
  /// \param iCreationFlags
  ///   Input: Bit mask for multiple VShapeCreationFlags_e flags.
  ///
  /// \param szShapeCacheId
  ///   Output: ID string of corresponding shape into the cache table (points to memory in cache table), 
  ///   if shape could be retrieved.
  ///
  /// \returns
  ///   Pointer to Havok Physics shape based on hkvBvCompressedMeshShape.
  /// 
  VHAVOK_IMPEXP static hkRefNew<hkpShape> CreateShapeFromStaticMeshInstances(
    const VisStaticMeshInstCollection &meshInstances, int iCreationFlags, const char **szShapeCacheId);

#if defined(SUPPORTS_TERRAIN)

  /// 
  /// \brief
  ///   Factory function to create a Havok Physics shape based on a height field Shape (hkvSampledHeightfield). 
  ///   It can optionally be wrapped by a triangle sampler so that collisions are exact.
  /// 
  /// \param terrain
  ///   Terrain sector which will be represented by the Havok Physics height field shape.
  ///
  /// \returns
  ///   Pointer to Havok Physics shape.
  /// 
  VHAVOK_IMPEXP static hkpShape* CreateShapeFromTerrain(const VTerrainSector &terrain);

#endif

  ///
  /// @}
  ///

  ///
  /// @name Havok Physics Shape Caching
  /// @{
  ///
 
  /// 
  /// \brief
  ///   Adds a Havok Physics shape with the given string ID to the cache table.
  /// 
  /// When adding a shape to the cache the reference count is incremented by 1. This way, 
  /// a reference to the shape is always kept even after all rigid bodies that reference it 
  /// have been removed. This way shapes don't have to be created multiple times, once they
  /// are added.
  ///
  /// \param szShapeId
  ///   ID string for the shape to be added.
  /// 
  /// \param pShape
  ///   Pointer to Havok Physics shape to be cached.
  /// 
  /// \return
  ///   ID string of shape (points to memory in cache table).
  /// 
  static const char* AddShape(const char *szShapeId, hkpShape *pShape);

  /// 
  /// \brief
  ///   Removes a single shape from the cache if there is no other reference to it.
  /// 
  /// \param szShapeId
  ///   The ID of the shape to remove.
  ///
  static void RemoveShape(const char *szShapeId);

  /// 
  /// \brief
  ///   Purges all cached shapes.
  /// 
  /// When clearing the cache, the reference of the shape factory to all cached shapes
  /// is released and the internal table is cleared.
  ///
  /// \return
  ///   Returns TRUE if no other references to the shapes exist. It is advised to ensure
  ///   that this is always the case when this function is called.
  ///
  /// \sa AddShape
  ///
  static bool ClearCache();

  /// 
  /// \brief
  ///   Finds a Havok Physics shape in the cache table.
  /// 
  /// \param szShapeId
  ///   Input: ID string of the collision shape to be found.
  /// 
  /// \param szShapeCacheId
  ///   Output (optional): ID string of cached shape (points to memory in cache table).
  ///   Is \c NULL if shape was not found.
  /// 
  /// \return
  ///   hkpShape* : Pointer to Havok shape, or \c NULL if shape could not be found.
  ///
  static hkpShape* FindShape(const char *szShapeId, const char **szShapeCacheId=NULL);

  ///
  /// @}
  ///

  ///
  /// @name Helper
  /// @{
  ///

  /// 
  /// \brief
  ///   Gets the pivot offset of the bounding box relative to the mesh's pivot.
  ///   
  /// \param pMesh
  ///   Pointer to the mesh.
  ///
  /// \param vScaleEntity
  ///   Scaling of the mesh instance.
  ///
  /// \returns
  ///   Pivot offset vector (in Vision units).
  /// 
  static hkvVec3 GetPivotOffset(const VDynamicMesh *pMesh, const hkvVec3& vScaleEntity);

  /// 
  /// \brief
  ///   Gets the pivot offset of the bounding box relative to the mesh's pivot.
  ///   
  /// \param pMesh
  ///   Pointer to the mesh.
  ///
  /// \param fUniformScaleEntity
  ///   Uniform scaling of the mesh instance.
  ///
  /// \returns
  ///   Pivot offset vector (in Vision units).
  /// 
  static hkvVec3 GetPivotOffset(const VDynamicMesh *pMesh, float fUniformScaleEntity);

  /// 
  /// \brief
  ///   Returns the ID for a convex shape.
  ///   
  /// \param szIDString
  ///   Pointer to a char array with a size of at least 512 bytes.
  ///
  /// \param szMeshName
  ///   Mesh name to use.
  ///
  /// \param vScale
  ///   Scale of the mesh instance.
  ///
  /// \param bShrinkByCvxRadius
  ///   Specifies whether convex shape is shrunken by convex radius so that its surface 
  ///   is as close as possible to the graphical representation.
  /// 
  static void GetIDStringForConvexShape(char *szIDString, const char *szMeshName, const hkvVec3& vScale, 
    bool bShrinkByCvxRadius);

  /// 
  /// \brief
  ///   Returns the ID for a convex shape.
  ///   
  /// \param szIDString
  ///   Pointer to a char array with a size of at least 512 bytes.
  ///
  /// \param szMeshName
  ///   Mesh name to use.
  ///
  /// \param vScale
  ///   Scale of the mesh instance.
  ///
  /// \param eCollisionBehavior
  ///   Determines the collision behavior.
  ///
  /// \param eWeldingType
  ///   Specifies the type of welding. 
  ///
  static void GetIDStringForMeshShape(char *szIDString, const char *szMeshName, const hkvVec3& vScale, 
    VisStaticMeshInstance_cl::VisCollisionBehavior_e eCollisionBehavior, VisWeldingType_e eWeldingType);

  /// 
  /// \brief
  ///   Builds a hkGeometry instance from the specified sub mesh of a collision mesh.
  /// 
  /// \param pColMesh
  ///   The collision mesh containing to the sub mesh, from which to build the geometry.
  /// 
  /// \param iSubmeshIndex
  ///   Index of the sub mesh within the collision mesh  
  ///   
  /// \param transform
  ///   Transformation matrix applied to the hkGeometry object.
  ///
  /// \param bConvex
  ///   Specifies whether the hkGeometry object will be used for convex or mesh shapes.
  /// 
  /// \param [out] geom 
  ///   Reference to a hkGeometry object which receives the results.
  ///
  static void BuildGeomFromCollisionMesh(const IVCollisionMesh *pColMesh, int iSubmeshIndex, 
    const hkvMat4 &transform, bool bConvex, hkGeometry& geom);

  /// \brief
  ///   Extracts the non-uniform scaling from a full transformation matrix.
  static void ExtractScaling(const hkvMat4 &mat, hkvVec3& destScaling);  
  
  /// \brief
  ///   Creates a convex hull shape for the given static mesh instances.
  static hkpShape* CreateConvexShapeFromStaticMeshInstances(const VisStaticMeshInstCollection &meshInstances, 
    hkvMat4 &transform, bool shrinkByCvxRadius);

  /// \brief
  ///   Creates a mesh shape for the given static mesh instances.
  static hkpShape* CreateMeshShapeFromStaticMeshInstances(const VisStaticMeshInstCollection &meshInstances, 
    hkvMat4 &transform, bool bAllowPerTriCollisionInfo, VisWeldingType_e eWeldingType);
  
#ifdef SUPPORTS_SNAPSHOT_CREATION

  /// \brief
  ///   Gets HKT file dependencies of a static mesh instance for precaching in a resource snapshot.
  static void GetHktDependencies(VResourceSnapshot &snapshot, VisStaticMeshInstance_cl *pMeshInstance);

  /// \brief
  ///   Gets HKT file dependencies of an entity for precaching in a resource snapshot.
  static void GetHktDependencies(VResourceSnapshot &snapshot, VisBaseEntity_cl *pEntity);

  /// \brief
  ///   Gets HKT file dependencies of a terrain sector for precaching in a resource snapshot.
  static void GetHktDependencies(VResourceSnapshot &snapshot, VTerrainSector *pSector);

#endif

  ///
  /// @}
  ///

private:
  static vHavokShapeCache *m_pShapeCacheTable;  ///< Cached shapes stored in hash map.
};

#endif // VHAVOKSHAPEFACTORY_HPP_INCLUDED

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
