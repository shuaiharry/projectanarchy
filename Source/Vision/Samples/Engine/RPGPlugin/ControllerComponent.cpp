/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */
#include <Vision/Samples/Engine/RPGPlugin/Precompiled.h>

#include <Vision/Samples/Engine/RPGPlugin/ControllerComponent.h>

#include <Ai/Pathfinding/Character/LocalSteering/hkaiLocalSteeringFilter.h>

#include <Vision/Runtime/EnginePlugins/Havok/HavokBehaviorEnginePlugin/vHavokBehaviorComponent.hpp>

#include <Vision/Samples/Engine/RPGPlugin/VisionHavokConversion.h>

V_IMPLEMENT_SERIAL(RPG_ControllerComponent, RPG_BaseComponent, 0, &g_RPGPluginModule);

START_VAR_TABLE(RPG_ControllerComponent, RPG_BaseComponent, "(RPG) Controller", 0, "(RPG) Controller")
  DEFINE_VAR_FLOAT_AND_NAME(RPG_ControllerComponent, m_sensorSize, "SensorSize", "Character Sensor Size", "128", 0, NULL);
  DEFINE_VAR_FLOAT_AND_NAME(RPG_ControllerComponent, m_desiredSpeed, "DesiredSpeed", "Character Desired Speed", "200", 0, NULL);
END_VAR_TABLE

namespace
{
  // AiWorldListener
  class AiWorldListener : public hkaiWorld::Listener
  {
  public:
    void AddController(RPG_ControllerComponent *controller);

    void RemoveController(RPG_ControllerComponent *controller);

  private:
    void AddListener();

    void RemoveListener();

    // hkaiWorld::Listener
    void postCharacterStepCallback(hkaiWorld::CharacterStepCallbackContext const& ctx) HK_OVERRIDE;

  private:
    VRefCountedCollection<RPG_ControllerComponent> m_controllers;
  }
  s_AiWorldListener;

  // LocalSteeringFilter
  class LocalSteeringFilter : public hkaiLocalSteeringFilter
  {
	hkBool32 isCharacterEnabled(hkaiCharacter const *aiCharacter, hkaiCharacter const *otherAiCharacter) const HK_OVERRIDE;

	hkBool32 isObstacleEnabled(hkaiCharacter const *aiCharacter, hkaiObstacleGenerator const *otherObstacle) const HK_OVERRIDE;
  }
  s_SteeringFilter;
}

// RPG_ControllerComponent
RPG_ControllerComponent::RPG_ControllerComponent()
  : RPG_BaseComponent(),
  m_currentStateId(RPG_ControllerStateId::kNotBeingControlled),
  m_settingState(false),
  m_sensorSize(100.0f),
  m_desiredSpeed(300.0f),
  m_cachedDirection(0.0f, 0.0f, 0.0f),
  m_cachedSpeed(0.0f),
  m_target(NULL)
{
}

void RPG_ControllerComponent::SetState(RPG_ControllerStateId::Enum newStateId)
{
  VASSERT(!m_settingState);
  if(!m_settingState && m_currentStateId != newStateId)
  {
    m_settingState = true;

    RPG_ControllerStateBase *currentState = m_states[m_currentStateId];
    if(currentState)
    {
      //Vision::Error.SystemMessage("Controller 0x%x exiting state %s", this, currentState->GetName());
      currentState->OnExitState(this);
    }

    m_currentStateId = newStateId;

    currentState = m_states[m_currentStateId];
    if(currentState)
    {
      //Vision::Error.SystemMessage("Controller 0x%x entering state %s", this, currentState->GetName());
      currentState->OnEnterState(this);
    }

    m_settingState = false;
  }
}

void RPG_ControllerComponent::RequestPath(hkvVec3 const& dest)
{
  hkVector4 scaledDest;
  RPG_VisionHavokConversion::VisionToHavokPoint(dest, scaledDest);
  m_aiBehavior->requestPath(scaledDest);
}

void RPG_ControllerComponent::CancelRequestedPath()
{
  m_aiBehavior->cancelRequestedPath();
}

float RPG_ControllerComponent::GetPathGoalReachedTolerance() const
{
  hkaiPathFollowingBehavior *pathFollowingBehavior = static_cast<hkaiPathFollowingBehavior*>(m_aiBehavior.val());
  float const radiusMultiplier = hkvMath::Max(0.0f, pathFollowingBehavior->getCharacterRadiusMultiplier() - 1.0f);
  float const tolerance = m_aiCharacter->getRadius() * radiusMultiplier + pathFollowingBehavior->getPathFollowingProperties()->m_goalDistTolerance;
  return RPG_VisionHavokConversion::HavokToVisionScalar(tolerance);
}

void RPG_ControllerComponent::SetSensorSize(float sensorSize)
{
  m_sensorSize = sensorSize;
}

void RPG_ControllerComponent::SetDesiredSpeed(float desiredSpeed)
{
  m_desiredSpeed = desiredSpeed;
}

void RPG_ControllerComponent::ServerTick(float deltaTime)
{
  RPG_Character const *const controllerCharacter = static_cast<RPG_Character*>(GetOwner());

  VASSERT(controllerCharacter);
  if(!controllerCharacter || controllerCharacter->IsDead())
  {
    return;
  }

  // Project Vision entity position to navmesh and update AI character
  {
    hkVector4 characterPos;
    RPG_VisionHavokConversion::VisionToHavokPoint(controllerCharacter->GetPosition(), characterPos);
  
    m_aiCharacter->setPosition(characterPos);
    {
      hkaiWorld *const aiWorld = vHavokAiModule::GetInstance()->GetAiWorld();
      hkaiCharacter *const aiCharacter = m_aiCharacter;

      hkaiCharacterUtil::ProjectToMeshSettings projectToMeshSettings;
      hkaiCharacterUtil::integrateMotionAndProjectToMesh(0.0f, aiWorld, &aiCharacter, 1, projectToMeshSettings);
    }
  }

  if(m_states[m_currentStateId])
    m_states[m_currentStateId]->OnTick(this, deltaTime);
}

void RPG_ControllerComponent::OnProcessAnimationEvent(RPG_Character *characterOwner, hkbEvent const& animationEvent)
{
  if(m_states[m_currentStateId])
    m_states[m_currentStateId]->OnProcessAnimationEvent(this, animationEvent);
}

bool RPG_ControllerComponent::TryAoeAttack()
{
  RPG_Character *const character = GetCharacter();

  if(character->GetCharacterStats().GetMana() > character->GetCharacterActionData().GetAoeAttackManaCost())
  {
    character->GetCharacterStats().SetMana(character->GetCharacterStats().GetMana() - character->GetCharacterActionData().GetAoeAttackManaCost());

    SetState(RPG_ControllerStateId::kAoeAttacking);
    return true;
  }

  return false;
}

bool RPG_ControllerComponent::TryPowerAttack()
{
  RPG_Character *const character = GetCharacter();

  if(character->GetCharacterStats().GetMana() > character->GetCharacterActionData().GetPowerAttackManaCost())
  {
    character->GetCharacterStats().SetMana(character->GetCharacterStats().GetMana() - character->GetCharacterActionData().GetPowerAttackManaCost());

    SetState(RPG_ControllerStateId::kPowerAttacking);
    return true;
  }

  return false;
}

// Update scene begin
//  Step Havok Behavior module
//   Step Havok Physics module
//    Step vHavokCharacterControllers (convert world motion delta to velocity and integrate)
//     Step AI world
//      RPG_ControllerComponent::OnAfterCharacterStep: Calculate character motion delta from AI velocity // This motion delta won't be used until next frame
//     Step physics world
// Fetch physics results
//  Update entity from vHavokCharacterControllers
// Entity think
//  Controller Tick
//   Update AI character position from entity
void RPG_ControllerComponent::OnAfterCharacterStep(float deltaTime)
{
  hkVector4 const& aiVelocity = m_aiCharacter->getVelocity();

  // update m_cachedDirection and m_cachedSpeed
  RPG_VisionHavokConversion::HavokToVisionPoint(aiVelocity, m_cachedDirection);
  m_cachedSpeed = RPG_VisionHavokConversion::VisionToHavokScalar(m_cachedDirection.getLengthAndNormalize());

  // calculate displacement for accumulation
  hkVector4 aiDisplacement;
  aiDisplacement.setMul(deltaTime, aiVelocity);

  hkvVec3 motionDelta;
  RPG_VisionHavokConversion::HavokToVisionPoint(aiDisplacement, motionDelta);

  // update character
  RPG_Character *controllerCharacter = static_cast<RPG_Character*>(GetOwner());
  {
    // vHavokCharacterController consumes motion delta; set world-space displacement from AI
    controllerCharacter->IncMotionDeltaWorldSpace(motionDelta);
  }
}

BOOL RPG_ControllerComponent::CanAttachToObject(VisTypedEngineObject_cl *object, VString& errorOut)
{
  if(!RPG_BaseComponent::CanAttachToObject(object, errorOut))
  {
    return FALSE;
  }

  bool const isCharacterEntity = (object->IsOfType(V_RUNTIME_CLASS(RPG_Character)) == TRUE);
  if(!isCharacterEntity)
  {
    errorOut = "RPG_Character (or derived) required";
    return FALSE;
  }

  return TRUE;
}

void RPG_ControllerComponent::SetOwner(VisTypedEngineObject_cl *newOwner)
{
  RPG_BaseComponent::SetOwner(newOwner);

  RPG_Character *characterOwner = vstatic_cast<RPG_Character*>(newOwner);

  if(characterOwner)
  {
    hkvVec3 const& pos = characterOwner->GetPosition();
    m_cachedDirection = characterOwner->GetDirection();

    hkaiAvoidanceSolver::AvoidanceProperties *avoidanceProperties = new hkaiAvoidanceSolver::AvoidanceProperties();
    hkReal const sensorSize = RPG_VisionHavokConversion::VisionToHavokScalar(m_sensorSize);
    {
      avoidanceProperties->m_localSensorAabb.m_max.setAll3(sensorSize);
      avoidanceProperties->m_localSensorAabb.m_min.setNeg3(avoidanceProperties->m_localSensorAabb.m_max);
    }

    hkaiCharacter::Cinfo characterCinfo;
    {
      RPG_VisionHavokConversion::VisionToHavokPoint(pos, characterCinfo.m_initialPosition);
      characterCinfo.m_up = hkVector4::getConstant<HK_QUADREAL_0010>();
      {
        hkReal const radius = RPG_VisionHavokConversion::VisionToHavokScalar(characterOwner->GetCollisionRadius());
        hkReal const desiredSpeed = RPG_VisionHavokConversion::VisionToHavokScalar(m_desiredSpeed);
        hkReal const height = RPG_VisionHavokConversion::VisionToHavokScalar(characterOwner->GetCollisionHeight());
        {
          characterCinfo.m_radius = radius;
          characterCinfo.m_desiredSpeed = desiredSpeed;
        }
      }
      characterCinfo.m_avoidanceProperties = avoidanceProperties;
      characterCinfo.m_userData = (hkUlong)this;
    }

    m_aiCharacter = new hkaiCharacter(characterCinfo);
    {
      m_aiCharacter->setSteeringFilter(&s_SteeringFilter);
      avoidanceProperties->removeReference();
    }
    m_aiCharacter->removeReference();

    hkaiPathFollowingBehavior::Cinfo behaviorCinfo;
    m_aiBehavior = new hkaiPathFollowingBehavior(m_aiCharacter, vHavokAiModule::GetInstance()->GetAiWorld(), behaviorCinfo);
    m_aiBehavior->removeReference();

    vHavokAiModule::GetInstance()->getCharacterBehaviors().pushBack(m_aiBehavior);

    s_AiWorldListener.AddController(this);
  }
  else
  {
    s_AiWorldListener.RemoveController(this);

    int const aiBehaviorIndex = vHavokAiModule::GetInstance()->getCharacterBehaviors().indexOf(m_aiBehavior);
    VASSERT(aiBehaviorIndex >= 0);
    if(aiBehaviorIndex >= 0)
    {
      vHavokAiModule::GetInstance()->getCharacterBehaviors().removeAt(aiBehaviorIndex);
    }

    m_aiBehavior = NULL;
    m_aiCharacter = NULL;

    // Destroy states
    for(int i = 0; i < RPG_ControllerStateId::kNumStates; ++i)
    {
      m_states[i] = NULL;
    }
  }
}

// RPG_ControllerStateBase
void RPG_ControllerStateBase::OnEnterState(RPG_ControllerComponent *const controller)
{
}

void RPG_ControllerStateBase::OnExitState(RPG_ControllerComponent *const controller)
{
}

void RPG_ControllerStateBase::OnProcessAnimationEvent(RPG_ControllerComponent *const controller, hkbEvent const& animationEvent)
{
}

void RPG_ControllerStateBase::OnTick(RPG_ControllerComponent *const controller, float const deltaTime)
{
}

// RPG_ControllerState::NotBeingControlled
void RPG_ControllerState::NotBeingControlled::OnProcessAnimationEvent(RPG_ControllerComponent *const controller, hkbEvent const& animationEvent)
{
  RPG_ControllerStateBase::OnProcessAnimationEvent(controller, animationEvent);

  RPG_Character *const character = controller->GetCharacter();

  if(character->GetIdForAnimationEvent(RPG_CharacterAnimationEvent::kTakeControl) == animationEvent.getId())
  {
    controller->SetState(RPG_ControllerStateId::kIdling);
  }
  else if(character->GetIdForAnimationEvent(RPG_CharacterAnimationEvent::kSpawnEffect) == animationEvent.getId())
  {
    character->CreateCharacterEffect(FX_Spawn);
  }
}

// RPG_ControllerState::Moving
void RPG_ControllerState::Moving::OnEnterState(RPG_ControllerComponent *const controller)
{
  RPG_ControllerStateBase::OnEnterState(controller);

  RPG_Character *const character = controller->GetCharacter();

  character->CreateCharacterEffect(FX_Moving);

  character->RaiseAnimationEvent(RPG_CharacterAnimationEvent::kMove);
}

void RPG_ControllerState::Moving::OnExitState(RPG_ControllerComponent *const controller)
{
  RPG_ControllerStateBase::OnExitState(controller);

  controller->CancelRequestedPath();

  RPG_Character *const character = controller->GetCharacter();

  character->PauseCharacterEffect(FX_Moving);

  character->RaiseAnimationEvent(RPG_CharacterAnimationEvent::kMoveEnd);
}

// RPG_ControllerUtil
void RPG_ControllerUtil::CalcDirection(hkvVec3& resultDir, hkvVec3 const& currentDir, hkvVec3 const& desiredDir, float t)
{
  if(currentDir.dot(desiredDir) < 0.99f)
  {
    float const dot = currentDir.dot(desiredDir);
    float const theta = hkvMath::acosRad(dot) * hkvMath::clamp(t, 0.f, 1.f);
    hkvVec3 const vec = (desiredDir - currentDir * dot).getNormalized();
    resultDir = currentDir * hkvMath::cosRad(theta) + vec * hkvMath::sinRad(theta);
  }
  else
  {
    resultDir = desiredDir;
  }
}

bool RPG_ControllerUtil::GetClosestPointOnNavMesh(hkvVec3 const& position, float testRadius, hkvVec3& outPosition)
{
  VASSERT(vHavokAiModule::GetInstance()->GetAiWorld());

  hkaiWorld const *const aiWorld = vHavokAiModule::GetInstance()->GetAiWorld();
  if(!aiWorld)
    return false;

  hkVector4 inPos, outPos;
  RPG_VisionHavokConversion::VisionToHavokPoint(position, inPos);
  hkaiPackedKey faceKey = aiWorld->getDynamicQueryMediator()->getClosestPoint(inPos, testRadius, outPos);

  if(faceKey != HKAI_INVALID_PACKED_KEY)
  {
    RPG_VisionHavokConversion::HavokToVisionPoint(outPos, outPosition);
    return true;
  }

  return false;
}

bool RPG_ControllerUtil::IsPointOnNavmesh(hkvVec3 const& position, float const testRadius)
{
  hkvVec3 outPos;
  return GetClosestPointOnNavMesh(position, testRadius, outPos);
}

bool RPG_ControllerUtil::CalcFaceTowards(hkvVec3& resultDir, hkvVec3 const& fromDir, hkvVec3 const& fromPos, hkvVec3 const& toPos, float const t)
{
  hkvVec3 const up(0.0f, 0.0f, 1.0f);
  hkvVec3 const fromTo(toPos - fromPos);
  hkvVec3 fromToProjectedDir(fromTo - fromTo.dot(up) * up);
  if(fromToProjectedDir.getLengthAndNormalize() > 0.1f)
  {
    RPG_ControllerUtil::CalcDirection(resultDir, fromDir, fromToProjectedDir, t);
    return true;
  }
  return false;
}

void RPG_ControllerUtil::FaceTowards(RPG_ControllerComponent *controller, hkvVec3 const& toPos, float t)
{
  hkvVec3 dir;
  if(RPG_ControllerUtil::CalcFaceTowards(dir, controller->GetCharacter()->GetDirection(), controller->GetCharacter()->GetPosition(), toPos, t))
    controller->GetCharacter()->SetDirection(dir);
}

void RPG_ControllerUtil::GetProjectedDirAndDistanceFromTarget(RPG_Character const *character, RPG_DamageableEntity *const target, hkvVec3& outDir, float& outDist)
{
  hkvVec3 const characterFromTarget(character->GetPosition() - target->GetPosition());

  hkvVec3 const up(0.0f, 0.0f, 1.0f);
  outDir = characterFromTarget - characterFromTarget.dot(up) * up;
  outDist = outDir.getLengthAndNormalize();
}

float RPG_ControllerUtil::GetMinimumDistanceToAttack(RPG_Character const *character, RPG_DamageableEntity *const target)
{
  float weaponRange = 0.0f;
  if(character->GetInventoryHandler().GetEquippedWeapon())
    weaponRange = character->GetInventoryHandler().GetEquippedWeapon()->GetRange();

  float const minRange = target->GetCollisionRadius() + hkvMath::Max(character->GetCollisionRadius(), weaponRange);
  float minDist = hkvMath::Max(0.0f, minRange + character->GetController()->GetPathGoalReachedTolerance());
  return minDist;
}

// AiWorldListener
void AiWorldListener::AddController(RPG_ControllerComponent *controller)
{
  VASSERT(controller);
  if(controller)
  {
    if(m_controllers.Count() == 0)
    {
      AddListener();
    }

    m_controllers.AddUnique(controller);
  }
}

void AiWorldListener::RemoveController(RPG_ControllerComponent *controller)
{
  VASSERT(controller);
  if(controller)
  {
    m_controllers.Remove(controller);

    if(m_controllers.Count() == 0)
    {
      RemoveListener();
    }
  }
}

void AiWorldListener::AddListener()
{
  vHavokAiModule::GetInstance()->GetAiWorld()->addListener(this);
}

void AiWorldListener::RemoveListener()
{
  vHavokAiModule::GetInstance()->GetAiWorld()->removeListener(this);
}

void AiWorldListener::postCharacterStepCallback(hkaiWorld::CharacterStepCallbackContext const& ctx)
{
  int const numControllers = m_controllers.Count();
  for(int i = 0; i < numControllers; i++)
  {
    RPG_ControllerComponent *const controller = m_controllers.GetAt(i);
    controller->OnAfterCharacterStep(ctx.m_timestep);
  }
}

hkBool32 LocalSteeringFilter::isCharacterEnabled(hkaiCharacter const *aiCharacter, hkaiCharacter const *otherAiCharacter) const
{
  RPG_ControllerComponent const *controller = (RPG_ControllerComponent const*)aiCharacter->m_userData;
  RPG_Character const *otherCharacter = ((RPG_ControllerComponent const*)otherAiCharacter->m_userData)->GetCharacter();

  // Don't steer around the character we are targetting
  return controller->GetTarget() == otherCharacter ? 0 : 1;
}

hkBool32 LocalSteeringFilter::isObstacleEnabled(hkaiCharacter const *aiCharacter, hkaiObstacleGenerator const *otherObstacle) const
{
  RPG_ControllerComponent const *controller = (RPG_ControllerComponent const*)aiCharacter->m_userData;
  RPG_DamageableEntity const *otherEntity = ((RPG_DamageableEntity const*)otherObstacle->m_userData);

  // Don't steer around the character we are targetting
  return controller->GetTarget() == otherEntity ? 0 : 1;
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
