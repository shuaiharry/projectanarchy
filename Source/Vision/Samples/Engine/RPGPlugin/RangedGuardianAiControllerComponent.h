/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */
#ifndef RPG_PLUGIN_RANGED_GUARDIAN_AI_CONTROLLER_COMPONENT_H__
#define RPG_PLUGIN_RANGED_GUARDIAN_AI_CONTROLLER_COMPONENT_H__

#include <Vision/Samples/Engine/RPGPlugin/AiControllerComponent.h>

// Ranged Guardian AI controller component
class RPG_RangedGuardianAiControllerComponent : public RPG_AiControllerComponent
{
public:
  // Constructor needs to be public for FORCE_LINKDYNCLASS on mobile
  RPG_RangedGuardianAiControllerComponent();

private:
  void SetOwner(VisTypedEngineObject_cl *newOwner);

  V_DECLARE_SERIAL_DLLEXP(RPG_RangedGuardianAiControllerComponent, RPG_PLUGIN_IMPEXP);
  V_DECLARE_VARTABLE(RPG_RangedGuardianAiControllerComponent, RPG_PLUGIN_IMPEXP);
};

namespace RPG_RangedGuardianAiControllerState
{
  // Idling
  class Idling : public RPG_ControllerStateBase
  {
    void OnTick(RPG_ControllerComponent *controller, float deltaTime) HKV_OVERRIDE;

    char const *GetName() const HKV_OVERRIDE { return "Ranged::Idling"; }
  };

  // Moving
  class Moving : public RPG_ControllerState::Moving
  {
    void OnTick(RPG_ControllerComponent *controller, float deltaTime) HKV_OVERRIDE;

    char const *GetName() const HKV_OVERRIDE { return "Ranged::Moving"; }
  };
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
