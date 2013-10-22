/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

/// \file vHavokDisplayHandler.hpp

#ifndef VHAVOKDISPLAYHANDLER_HPP_INCLUDED
#define VHAVOKDISPLAYHANDLER_HPP_INCLUDED

#include <Vision/Runtime/EnginePlugins/Havok/HavokPhysicsEnginePlugin/vHavokPhysicsIncludes.hpp>

#include <Common/Visualize/hkDebugDisplayHandler.h>
#include <Common/Visualize/hkProcess.h>
#include <Common/Visualize/hkProcessFactory.h>
#include <Common/Visualize/Shape/hkDisplayGeometry.h>
#include <Physics2012/Utilities/VisualDebugger/Viewer/Collide/hkpShapeDisplayViewer.h>
#include <Common/Base/Config/hkConfigVersion.h>

///
/// \brief
///   Class that represents a display geometry constructed from a Havok Physics geometry to
///   enable debug rendering in Vision. It consists of a mesh instance that has a 
///   highlighting shader applied.
///
class vHavokDisplayGeometry: public VRefCounter
{
public:

  ///
  /// \brief
  ///   Simple struct for creating the mesh buffer of the Havok Display Geometry.
  ///
  struct SimpleMesh_t
  {
    float pos[3];   ///< Vertex data (x, y, z)
  };

  ///
  /// @name Constructor / Destructor
  /// @{
  ///

  /// 
  /// \brief
  ///   Constructor that creates a mesh instance for a Havok geometry to enable debug rendering in Vision.
  /// 
  /// \param geometry
  ///   Pointer to the Havok geometry.
  ///
  /// \param transform
  ///   Reference of original transform of the Havok geometry.
  ///
  /// \param id
  ///   ID of the Havok geometry.
  ///
  /// \param tag
  ///   Tag that can be used for separating display information into different categories. 
  ///
  /// \see
  ///   vHavokDisplayHandler::addGeometry
  /// 
  vHavokDisplayGeometry(hkDisplayGeometry* geometry, const hkTransform &transform, hkUlong id, int tag);
   
  /// 
  /// \brief
  ///   Destructor
  /// 
  ~vHavokDisplayGeometry();

  ///
  /// @}
  ///

  ///
  /// @name Display Helper
  /// @{
  ///

  /// 
  /// \brief
  ///   Creates a compiled shader technique for this Havok Display geometry. Basically
  ///   a wireframe highlighting shader.
  /// 
  /// \param iColor
  ///   Color of the highlighting wireframe effect.
  ///
  VCompiledTechnique *CreateHighlightEffect(VColorRef iColor);

  /// 
  /// \brief
  ///   Sets the visibility state of this Havok Display Geometry.
  /// 
  /// \param bVisible
  ///   If TRUE, display geometry is visible.
  ///
  /// \sa
  ///   vHavokDisplayHandler::SetVisible
  /// 
  void SetVisible(BOOL bVisible);

  /// 
  /// \brief
  ///   Returns the visibility state of this HavokDisplay Geometry.
  /// 
  /// \return 
  ///   BOOL: If TRUE, display geometry is visible.
  ///
  /// \sa
  ///   vHavokDisplayHandler::IsVisible
  /// 
  BOOL IsVisible() const;
  
  /// 
  /// \brief
  ///   Sets the wireframe highlighting color of this HavokDisplay Geometry.
  /// 
  /// \param iColor
  ///   New Color of the highlighting wireframe effect.
  ///
  /// \sa
  ///   vHavokDisplayHandler::SetColor
  /// 
  void SetColor(VColorRef iColor);

  /// 
  /// \brief
  ///   Sets the owner object of this HavokDisplay Geometry.
  /// 
  /// \param pOwner
  ///   Owner object of this HavokDisplay Geometry.
  ///
  void SetOwner(VisObject3D_cl *pOwner);

  ///
  /// @}
  ///

  ///
  /// @name Update
  /// @{
  ///

  /// 
  /// \brief
  ///   Updates the position and rotation of this HavokDisplay geometry. 
  ///
  /// Called by vHavokDisplayHandler.
  /// 
  /// \param transform
  ///   Reference of new transform of the Havok geometry.
  ///
  /// \sa
  ///   vHavokDisplayHandler::updateGeometry
  ///
  void UpdateTransform(const hkTransform &transform);

  ///
  /// @}
  ///

  hkUlong m_ID;       ///< ID of the geometry (address of the hkpCollidable this geometry was created from)

  VisMeshBufferObject_cl* GetMeshInstance () const 
  { 
    return m_spMeshInstance; 
  }

private:
  VisMeshBufferObjectPtr m_spMeshInstance;    ///< Mesh buffer object for the physics shape representation.
  hkvVec3 m_vPos;                             ///< Current position of the mesh buffer object.
  hkvMat3 m_vRot;                             ///< Current rotation matrix of the mesh buffer object.
};

/// 
/// \brief
///   Collection of all Havok Display Geometries held by the HavokDisplay Handler.
/// 
class vHavokDisplayGeometryList
{
public:
  /// \brief
  ///   Default constructor.
  vHavokDisplayGeometryList();

private:
  /// \brief
  ///   Not copyable.
  vHavokDisplayGeometryList(vHavokDisplayGeometryList const&);

  /// \brief
  ///   Not copyable.
  vHavokDisplayGeometryList& operator=(vHavokDisplayGeometryList const&);

public:
  /// \brief
  ///   Destructor.
  ~vHavokDisplayGeometryList();

  /// \brief
  ///   Adds a display geometry to this collection.
  ///
  /// \param pGeometry
  ///   the geometry to add
  void Add(vHavokDisplayGeometry *pGeometry);

  /// \brief
  ///   Ensures that this collection can be expanded to the given capacity 
  ///   without having to grow the internal list while adding elements.
  ///
  /// \param iCapacity
  ///   The capacity to reserve.
  ///
  /// Calls to this function are merely hints. Adding elements will succeed as 
  /// well without ensuring sufficient capacity first, but may be slower due to
  /// frequent re-allocations.
  void EnsureCapacity(unsigned int iCapacity);

  /// \brief
  ///   Returns the size of this collection.
  unsigned int GetSize() const;

  /// \brief
  ///   returns if this collection contains one or more display geometries with the
  ///   specified ID, only if at least one of these is visible.
  ///
  /// \param iID
  ///   ID of the display geometries
  ///
  BOOL IsVisible(hkUlong iID) const;

  /// \brief
  ///   Removes all display geometries with the specified ID from this collection.
  ///
  /// \param iID
  ///   ID of the display geometries to remove
  ///
  void Remove(hkUlong iID);

  /// \brief
  ///   Sets the color of all display geometries in this collection matching the
  ///   specified ID.
  ///
  /// \param iID
  ///   ID of the display geometries
  /// \param iColor
  ///   color to set
  ///
  void SetColor(hkUlong iID, VColorRef iColor);

  /// \brief
  ///   Sets the owner of all display geometries in this collection matching the
  ///   specified ID.
  ///
  /// \param iID
  ///   ID of the display geometries
  /// \param pOwner
  ///   owner to set
  ///
  void SetOwner(hkUlong iID, VisObject3D_cl *pOwner);

  /// \brief
  ///   Sets the visibility of all display geometries in this collection matching the
  ///   specified ID.
  ///
  /// \param iID
  ///   ID of the display geometries
  /// \param bVisible
  ///   visibility flag
  ///
  void SetVisible(hkUlong iID, BOOL bVisible);

  /// \brief
  ///   Updates the transform of all display geometries in this collection matching the
  ///   specified ID.
  ///
  /// \param iID
  ///   ID of the display geometries
  /// \param transform
  ///   transform to set
  ///
  void UpdateTransform(hkUlong iID, const hkTransform &transform);

private:
  DynArray_cl<vHavokDisplayGeometry*> m_List; ///< The list of geometries managed by this instance
  unsigned int m_iSize;                       ///< The number of geometries managed by this instance
};

/// 
/// \brief
///   Vision specific implementation of the hkDebugDisplayHandler. 
/// 
class vHavokDisplayHandler: public VRefCounter, public hkDebugDisplayHandler, public IHavokStepper
{
public:
  ///
  /// @name Constructor / Destructor
  /// @{
  ///

  ///
  /// \brief
  ///   Constructor
  ///
  /// Initializes the Havok Display Handler.
  /// 
  /// \param pPhysicsWorld
  ///   The Havok Physics world object. Container for the simulation's physical objects.
  ///
  vHavokDisplayHandler(hkpWorld* pPhysicsWorld);
  
  ///
  /// \brief
  ///   Destructor
  ///
  /// De-initializes the HavokDisplay Handler.
  ///
  virtual ~vHavokDisplayHandler();

  ///
  /// @}
  ///

  ///
  /// @name hkDebugDisplayHandler Virtual Overrides
  /// @{
  ///

  ///
  /// \brief
  ///   Adds a list of geometries to the display world managed by this display handler. 
  ///   See Havok Physics documentation for detailed information.
  ///
  virtual hkResult addGeometry(const hkArrayBase<hkDisplayGeometry *> &geometries, const hkTransform &transform, 
    hkUlong id, int tag, hkUlong shapeIdHint, hkGeometry::GeometryType geomType) HKV_OVERRIDE;

  ///
  /// \brief
  ///   Adds a geometry to the display world managed by this display handler. 
  ///   See Havok Physics documentation for detailed information.
  ///
  virtual hkResult addGeometry(hkDisplayGeometry *geometry, hkUlong id, int tag, hkUlong shapeIdHint) HKV_OVERRIDE;

  ///
  /// \brief
  ///   Adds an instance of a geometry that has already been added to 
  ///   the display world managed by this display handler. 
  ///   See Havok Physics documentation for detailed information.
  ///
  virtual hkResult addGeometryInstance(hkUlong originalInstanceId, const hkTransform& transform, hkUlong id, 
    int tag, hkUlong shapeIdHint) HKV_OVERRIDE;

  ///
  /// \brief
  ///   Updates the transform of a body in the display world.
  ///   See Havok Physics documentation for detailed information.
  ///
  virtual hkResult updateGeometry(const hkTransform &transform, hkUlong id, int tag) HKV_OVERRIDE;

#if (HAVOK_SDK_VERSION_MAJOR >= 2010)
  ///
  /// \brief
  ///   Updates the transform of a body in the display world.
  ///   See Havok Physics documentation for detailed information.
  ///
  virtual hkResult updateGeometry(const hkMatrix4& transform, hkUlong id, int tag) HKV_OVERRIDE;
#endif

  ///
  /// \brief
  ///   Removes a geometry from the display world managed by this display handler.
  ///   See Havok Physics documentation for detailed information.
  ///
  virtual hkResult removeGeometry(hkUlong id, int tag, hkUlong shapeIdHint) HKV_OVERRIDE;

  ///
  /// \brief
  ///   Sets the visibility of a given child geometry in a display object.
  ///   See Havok Physics documentation for detailed information.
  ///
  virtual hkResult setGeometryVisibility(int geometryIndex, bool isEnabled, hkUlong id, int tag) HKV_OVERRIDE;

  ///
  /// \brief
  ///   Sets the color of a geometry previously added to the display world. 
  ///   See Havok Physics documentation for detailed information.
  ///
  virtual hkResult setGeometryColor(hkColor::Argb color, hkUlong id, int tag) HKV_OVERRIDE;

  /// \brief
  ///   Sets the transparency of a geometry previously added to the display world.
  ///   See Havok Physics documentation for detailed information.
  ///
  virtual hkResult setGeometryTransparency(float alpha, hkUlong id, int tag) HKV_OVERRIDE;

  /// \brief
  ///   Set whether a geometry is pickable.
  ///   See Havok Physics documentation for detailed information.
  ///
  virtual hkResult setGeometryPickable( hkBool isPickable, hkUlong id, int tag ) HKV_OVERRIDE;

  ///
  /// \brief
  ///   Updates the camera from the display world managed by this display handler.
  ///   See Havok Physics documentation for detailed information.
  ///
  virtual hkResult updateCamera(const hkVector4& from, const hkVector4& to, const hkVector4& up, 
    hkReal nearPlane, hkReal farPlane, hkReal fov, const char* name) HKV_OVERRIDE;
  
  ///
  /// \brief
  ///   Updates the behavior from the display world managed by this display handler.
  ///   See Havok Physics documentation for detailed information.
  ///
  virtual hkResult updateBehavior(const hkArrayBase<int> &wordVarIdx, const hkArrayBase<int> &wordStack, 
    const hkArrayBase<int> &quadVarIdx, const hkArrayBase<hkVector4> &quadStack, 
    const hkArrayBase<char *> &activeNodes, const hkArrayBase<int> &activeStateIds, 
    const hkArrayBase<int> &activeTransitions, const hkArrayBase<hkQsTransform> &transforms);

  ///
  /// \brief
  ///   Puts a display point into the display buffer for display in the next frame.
  ///   See Havok Physics documentation for detailed information.
  ///
  virtual hkResult displayPoint(const hkVector4& position, hkColor::Argb color, int id, int tag) HKV_OVERRIDE;

  ///
  /// \brief
  ///   Puts a display line into the display buffer for display in the next frame.
  ///   See Havok Physics documentation for detailed information.
  ///
  virtual hkResult displayLine(const hkVector4& start, const hkVector4& end, hkColor::Argb color, 
    int id, int tag) HKV_OVERRIDE;

  ///
  /// \brief
  ///   Puts a display triangle into the display buffer for display in the next frame.
  ///   See Havok Physics documentation for detailed information.
  ///
  virtual hkResult displayTriangle(const hkVector4& a, const hkVector4& b, const hkVector4& c, 
    hkColor::Argb color, int id, int tag) HKV_OVERRIDE;

  ///
  /// \brief
  ///   Outputs user text to the display.
  ///   See Havok Physics documentation for detailed information.
  ///
  virtual hkResult displayText(const char* text, hkColor::Argb color, int id, int tag) HKV_OVERRIDE;

  ///
  /// \brief
  ///   Outputs 3D text. Same as displayText() but with position.
  ///   See Havok Physics documentation for detailed information.
  ///
  virtual hkResult display3dText(const char* text, const hkVector4& pos, hkColor::Argb color, 
    int id, int tag) HKV_OVERRIDE;

  // compatibility with VisionIntegration branch
#if defined(HAVOK_SDK_VERSION_MAJOR) && (HAVOK_SDK_VERSION_MAJOR >= 2012)
  ///
  /// \brief
  ///   Puts a display point in 2D [-1..1] normalized windows coordinates into the display buffer for 
  ///   display in the next frame. 
  ///   See Havok Physics documentation for detailed information.
  ///
  virtual hkResult displayPoint2d(const hkVector4& position, hkColor::Argb color, int id, int tag) HKV_OVERRIDE;

  ///
  /// \brief
  ///   Puts a display line in 2D [-1..1] normalized windows coordinates into the display buffer for display in 
  ///   the next frame.
  ///   See Havok Physics documentation for detailed information.
  ///
  virtual hkResult displayLine2d(const hkVector4& start, const hkVector4& end, hkColor::Argb color, 
    int id, int tag) HKV_OVERRIDE;

  ///
  /// \brief
  ///   Puts a display triangle in 2D [-1..1] normalized windows coordinates into the display 
  ///   buffer for display in the next frame.
  ///   See Havok Physics documentation for detailed information.
  ///
  virtual hkResult displayTriangle2d(const hkVector4& a, const hkVector4& b, const hkVector4& c, 
    hkColor::Argb color, int id, int tag) HKV_OVERRIDE;

  ///
  /// \brief
  ///   Outputs text in 2D [-1..1] normalized windows coordinates. Also scales the text wrt the 
  ///   font size using sizeScale.
  ///   See Havok Physics documentation for detailed information.
  ///
  virtual hkResult displayText2d(const char* text, const hkVector4& pos, hkReal sizeScale, 
    hkColor::Argb color, int id, int tag) HKV_OVERRIDE;
#endif

  ///
  /// \brief
  ///   Display general annotation data. 
  ///   See Havok Physics documentation for detailed information.
  ///
  virtual hkResult displayAnnotation(const char* text, int id, int tag) HKV_OVERRIDE;

  ///
  /// \brief
  ///   Displays the geometries.
  ///   See Havok Physics documentation for detailed information.
  ///
  virtual hkResult displayGeometry(const hkArrayBase<hkDisplayGeometry*>& geometries, const hkTransform& transform, 
    hkColor::Argb color, int id, int tag) HKV_OVERRIDE;
    
  ///
  /// \brief
  ///   Displays the geometries without transform.
  ///   See Havok Physics documentation for detailed information.
  ///
  virtual hkResult displayGeometry(const hkArrayBase<hkDisplayGeometry*>& geometries, hkColor::Argb color, 
    int id, int tag) HKV_OVERRIDE;

  ///
  /// \brief
  ///   Send memory statistics dump.
  ///   See Havok Physics documentation for detailed information.
  ///
  virtual hkResult sendMemStatsDump(const char* data, int length) HKV_OVERRIDE;

  ///
  /// \brief
  ///   Ensures that the current immediate mode display information will be preserved 
  ///   and merged with all new immediate mode data for the next step/frame.
  ///   See Havok Physics documentation for detailed information.
  ///
  virtual hkResult holdImmediate() HKV_OVERRIDE;

  ///
  /// \brief
  ///   Batch update speed increases.
  ///   See Havok Physics documentation for detailed information.
  ///
  virtual void lockForUpdate() HKV_OVERRIDE;

  ///
  /// \brief
  ///   Unlock for Update.
  ///   See Havok Physics documentation for detailed information.
  ///
  virtual void unlockForUpdate() HKV_OVERRIDE;

  /// \brief
  ///   Implements IHavokStepper.
  ///   See Havok Physics documentation for detailed information.
  ///
  virtual void Step(float dt) HKV_OVERRIDE;

#if (HAVOK_SDK_VERSION_MAJOR >= 2010)
  /// \brief
  ///   Adds a mesh to the display world managed by this display handler.
  virtual hkResult addMesh(hkMeshBody* mesh, hkUlong id, int tag) 
  { 
    return HK_FAILURE; 
  }

  /// \brief
  ///   Removes a mesh from the display world managed by this display handler.
  virtual hkResult removeMesh(hkUlong id, int tag) 
  { 
    return HK_FAILURE; 
  }

  /// \brief
  ///   Updates the root transform of a mesh with a hkMatrix4.
  virtual hkResult updateMesh( const hkMatrix4& transform, hkUlong id, int tag ) 
  { 
    return HK_FAILURE; 
  }

  /// \brief
  ///   Updates the skin of a mesh given a pose using hkMatrix4s.
  virtual hkResult skinMeshes(hkUlong* ids, int numIds, const hkMatrix4* poseModel, int numPoseModel, 
    const hkMatrix4& worldFromModel, int tag )
  { 
    return HK_FAILURE; 
  }

  /// \brief
  ///   Skins a body in the display world. Only geometries of type hkDisplayMesh can be skinned.
  virtual hkResult skinGeometry(hkUlong* ids, int numIds, const hkMatrix4* poseModel, int numPoseModel, 
    const hkMatrix4& worldFromModel, int tag)
  { 
    return HK_FAILURE; 
  }

#endif

  ///
  /// @}
  ///

  ///
  /// @name Display Helper
  /// @{
  ///
  
  /// 
  /// \brief
  ///   Sets the visibility state of the Havok Display Geometry identified by the given ID.
  /// 
  /// \param id
  ///   ID of the geometry (address of the hkpCollidable this geometry was created from).
  ///
  /// \param bVisible
  ///   If TRUE, display geometry is set to be visible.
  ///
  /// \sa
  ///   vHavokDisplayHandler::SetVisible
  /// 
  void SetVisible(hkUlong id, BOOL bVisible);
  
  /// 
  /// \brief
  ///   Gets the visibility state of the Havok Display Geometry identified by the given ID.
  /// 
  /// \param id
  ///   ID of the geometry (address of the hkpCollidable this geometry was created from).
  ///
  /// \return 
  ///   BOOL: If TRUE, display geometry is visible.
  ///
  /// \see
  ///   vHavokDisplayHandler::IsVisible
  /// 
  BOOL IsVisible(hkUlong id) const;

  /// 
  /// \brief
  ///   Sets the wireframe highlighting color of the Havok Display Geometry identified by the given ID.
  /// 
  /// \param id
  ///   ID of the geometry (address of the hkpCollidable this geometry was created from).
  ///
  /// \param iColor
  ///   New Color of the highlighting wireframe effect.
  ///
  /// \see
  ///   vHavokDisplayHandler::SetColor
  /// 
  void SetColor(hkUlong id, VColorRef iColor);

  /// 
  /// \brief
  ///   Sets the owner object so we can attach the Havok Physics shape to it
  /// 
  /// \param id
  ///   ID of the geometry (address of the hkpCollidable this geometry was created from).
  ///
  /// \param pOwner
  ///   Owner object of the Havok Physics component.
  ///
  /// \see
  ///   vHavokDisplayHandler::SetColor
  /// 
  void SetOwner(hkUlong id, VisObject3D_cl *pOwner);

  inline hkArray<hkProcessContext*>& getContexts() 
  { 
    return m_contexts; 
  }

  ///
  /// @}
  ///

private:
  hkpPhysicsContext* m_pContext;                  ///< Physics Context that holds all physics viewers
  hkArray<hkProcess*> m_pLocalProcesses;          ///< Local Processes handled by this display handler
  vHavokDisplayGeometryList m_Geometries;         ///< List of all geometries in the physics world
  hkArray<hkProcessContext*> m_contexts;
};

#endif // VHAVOKDISPLAYHANDLER_HPP_INCLUDED

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
