/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

#include <Vision/Samples/Engine/RPGPlugin/Precompiled.h>
#include <Vision/Samples/Engine/RPGPlugin/VisionHavokBehaviorWorldListener.h>
#include <Vision/Samples/Engine/RPGPlugin/Character.h>

#include <Vision/Runtime/EnginePlugins/VisionEnginePlugin/Scripting/VScriptIncludes.hpp>
#include <Vision/Runtime/EnginePlugins/Havok/HavokBehaviorEnginePlugin/vHavokBehaviorComponent.hpp>

#include <Common/Base/Reflection/Registry/hkVtableClassRegistry.h>

RPG_HavokBehaviorWorldListener RPG_HavokBehaviorWorldListener::s_instance;

RPG_HavokBehaviorWorldListener::RPG_HavokBehaviorWorldListener()
{
}

void RPG_HavokBehaviorWorldListener::eventRaisedCallback( hkbCharacter* character, const hkbEvent& behaviorEvent, bool raisedBySdk )
{
  vHavokBehaviorComponent *behaviorComponent = (vHavokBehaviorComponent*)(character->m_userData);
  VASSERT(behaviorComponent);
  if(behaviorComponent)
    Vision::Game.SendMsg(behaviorComponent->GetOwner(), RPG_VisionUserMessages::kHavokAnimationEvent, (INT_PTR)&behaviorEvent, raisedBySdk ? 1 : 0);
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
