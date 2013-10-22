/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

#ifndef __VHAVOK_AI_MODULE_HPP
#define __VHAVOK_AI_MODULE_HPP

#include <Vision/Runtime/EnginePlugins/Havok/HavokAiEnginePlugin/vHavokAiIncludes.hpp>

// included here for LoadNavMeshDeprecated
#include <Vision/Runtime/EnginePlugins/Havok/HavokAiEnginePlugin/vHavokAiNavMeshInstance.hpp>
class hkaiWorld;
class hkaiNavMesh;
class hkaiViewerContext;

class hkpWorld;

/// 
/// \brief
///   Module responsible for the AI simulation.
///
class vHavokAiModule : public IVisCallbackHandler_cl, public IHavokStepper
{
public:
	///
	/// @name Constructor / Destructor / Global Accessor
	/// @{
	///

	vHavokAiModule();
	virtual ~vHavokAiModule();

	VHAVOKAI_IMPEXP static vHavokAiModule* GetInstance() 
  {
    return &g_GlobalManager;
  }

	///
	/// @}
	///

	///
	/// @name One time Initialization / Deinitialization to register callbacks
	/// @{
	///

	/// \brief
	///   Should be called at plugin initialization time.
	void OneTimeInit();

	/// \brief
	///   Should be called at plugin de-initialization time.
	void OneTimeDeInit();

	/// \brief
	///   Called after Havok base system initialization.
	void Init();

  /// \brief
  ///   Called before Havok base system deinitialization,.
	void DeInit();

	///
	/// @}
	///

	///
	/// @name IVisCallbackHandler_cl overridden functions
	/// @{
	///

	/// \brief
	///   IVisCallbackHandler_cl implementation
	virtual void OnHandleCallback( IVisCallbackDataObject_cl* pData ) HKV_OVERRIDE;

	virtual int GetCallbackSortingKey(VCallback *pCallback) HKV_OVERRIDE;

	///
	/// @}
	///

	///
	/// @name Functions to create / modify AI world
	/// @{
	///

	/// \brief
	///   Manual access to world construction. 
  ///
  /// Note that world gets automatically created when plugin gets loaded.
	VHAVOKAI_IMPEXP bool CreateAiWorld();

  /// \brief
  ///   Manual access to world destruction. 
	VHAVOKAI_IMPEXP void RemoveAiWorld();

	///
	/// @}
	///

	///
	/// @name Functions to create/modify AI world
	/// @{
	///

  /// \brief
  ///   Manually sets the physics world to be able to connect to it.
  ///
  /// \sa SetConnectToPhysicsWorld
	VHAVOKAI_IMPEXP void SetPhysicsWorld(hkpWorld* physics);

  /// \brief
  ///   Manually connects the Havok AI module to Havok physics world.  
  ///
  /// Note that this connection should already have happened if specified in the exported scene.
	VHAVOKAI_IMPEXP void SetConnectToPhysicsWorld(bool connect, bool stepSilhouettesAfterDisconnecting = false);

	///
	/// @}
	///

	///
	/// @name IHavokStepper overrides
	/// @{
	///

	VHAVOKAI_IMPEXP virtual void Step(float dt) HKV_OVERRIDE;

	/// \brief
  ///   Called by the Havok physics module on de-initialization.
  ///
  /// It is called before anything was actually deleted, with the hkpWorld marked for write.
	VHAVOKAI_IMPEXP virtual void OnDeInitPhysicsModule() HKV_OVERRIDE;

	///
	/// @}
	///

	///
	/// @name Miscellaneous accessor methods
	/// @{
	///

	VHAVOKAI_IMPEXP hkaiWorld* GetAiWorld() 
  { 
    return m_aiWorld; 
  }

	VHAVOKAI_IMPEXP const hkaiWorld* GetAiWorld() const 
  { 
    return m_aiWorld; 
  }

	VHAVOKAI_IMPEXP hkArray<class hkaiBehavior*>& getCharacterBehaviors()				
  { 
    return m_behaviors; 
  }

	VHAVOKAI_IMPEXP const hkArray<class hkaiBehavior*>& getCharacterBehaviors() const
  { 
    return m_behaviors;
  }

	/// deprecated interface
	VHAVOKAI_IMPEXP bool LoadNavMeshDeprecated(const char* filename, VArray<vHavokAiNavMeshInstance*>* navMeshInstancesOut = HK_NULL);

	///
	/// @}
	///

	///
	/// @name Demo code
	/// @{
	///

	/// \brief
	///   For computing a path (note that a lot more options are available by instead issuing pathfinding requests through the hkaiWorld object).
	VHAVOKAI_IMPEXP bool ComputePath(hkvVec3* startPoint, hkvVec3* endPoint, float radius, VArray<hkvVec3>& pathPoints) const;

	/// \brief
	///   For computing and drawing a path.
	VHAVOKAI_IMPEXP bool ComputeAndDrawPath(IVRenderInterface *pRenderer, hkvVec3* startPoint, hkvVec3* endPoint, float radius, float height, float displayOffset, unsigned int color) const;

	/// \brief
	///   For debug rendering all nav mesh instances in AI world.
	VHAVOKAI_IMPEXP void DebugRender(float displayOffsetHavokScale, bool colorRegions = true);

	///
	/// @}
	///

protected:
	/// \brief
	///   Saves or loads ai world global setting to/from the passed chunk file.  
	void GlobalsChunkFileExchange(VChunkFile &file, CHUNKIDTYPE iID);

	/// \brief
	///   Connects to hkpWorld
	void ConnectToPhysicsWorld();

	/// \brief
	///   Disconnects from hkpWorld
	void DisconnectFromPhysicsWorld(bool stepSilhouettesAfterDisconnecting = false);

	hkaiWorld* m_aiWorld;

	hkpWorld* m_physicsWorld;
	bool m_connectToPhysicsWorld;
	class hkaiPhysics2012WorldListener* m_physicsWorldListener;
	hkArray<class hkaiBehavior*> m_behaviors;

	hkaiViewerContext* m_aiViewerContext;

	/// one global instance of our manager
	static vHavokAiModule g_GlobalManager;
};

#endif	// __VHAVOK_AI_MODULE_HPP

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
