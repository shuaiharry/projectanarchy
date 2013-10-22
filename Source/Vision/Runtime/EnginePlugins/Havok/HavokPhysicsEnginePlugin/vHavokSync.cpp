/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

#include <Vision/Runtime/EnginePlugins/Havok/HavokPhysicsEnginePlugin/HavokPhysicsEnginePluginPCH.h>
#define __vHavokPhysicsModuleIncludes
#include <Vision/Runtime/EnginePlugins/Havok/HavokPhysicsEnginePlugin/vHavokSync.hpp>
#include <Common/Base/Memory/System/Util/hkMemoryInitUtil.h>

#ifdef HAVOK_NEED_SYNC_MACROS

void vHavokPhysicsModuleGetSyncInfo(hkMemoryInitUtil::SyncInfo& s)
{
	s.m_memoryRouter = hkMemoryRouter::getInstancePtr();
	s.m_singletonList = hkSingletonInitList;
	s.m_memorySystem = hkMemorySystem::getInstancePtr();
	s.m_monitors = hkMonitorStream::getInstancePtr();
	s.m_mtCheckSection = hkMultiThreadCheck::m_criticalSection;
#if (HAVOK_SDK_VERSION_MAJOR >= 2010)
	s.m_stackTracerImpl = hkStackTracer::getImplementation();
	s.m_mtCheckStackTree = hkMultiThreadCheck::s_stackTree;
	s.m_mtRefLockedAllPtr = hkMemoryRouter::getInstance().getRefObjectLocalStore();
#endif
}

#endif

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
