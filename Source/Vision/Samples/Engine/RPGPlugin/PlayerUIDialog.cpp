/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */
#include <Vision/Samples/Engine/RPGPlugin/Precompiled.h>

#include <Vision/Samples/Engine/RPGPlugin/PlayerUIDialog.h>
#include <Vision/Samples/Engine/RPGPlugin/PlayerUI.h>

#include <Vision/Samples/Engine/RPGPlugin/Character.h>
#include <Vision/Samples/Engine/RPGPlugin/GameManager.h>
#include <Vision/Samples/Engine/RPGPlugin/GUIManager.h>
#if (RPG_UI_SCALEFORM)
#include <Vision/Samples/Engine/RPGPlugin/GUIManager_Scaleform.h>
#endif
#include <Vision/Samples/Engine/RPGPlugin/PlayerControllerComponent.h>
#include <Vision/Samples/Engine/RPGPlugin/VisionEffectHelper.h>
#include <Vision/Samples/Engine/RPGPlugin/VisionHavokConversion.h>

// DEMO
#include <Vision/Samples/Engine/RPGPlugin/HighlightableComponentManager.h>
#include <Vision/Samples/Engine/RPGPlugin/HighlightableComponent.h>
#include <Vision/Samples/Engine/RPGPlugin/AttackableComponent.h>

// TODO: correct AI header for ray query
#include <Ai/Pathfinding/Character/Behavior/hkaiPathFollowingBehavior.h>

#include <Vision/Runtime/Base/System/Memory/VMemDbg.hpp>

namespace
{
  class RPG_ClosestAttackableRaycastResult : public VisPhysicsRaycastBase_cl
  {
  public:
    RPG_ClosestAttackableRaycastResult(RPG_BaseEntity const *const entityToIgnore);
    virtual void Reset();
    virtual bool onHit(VisPhysicsHit_t &hit);
    virtual bool allHits();

  public:
    RPG_BaseEntity const *const m_entityToIgnore;
    VisPhysicsHit_t m_closestHit;
    VisPhysicsHit_t m_closestAttackableHit;
    bool m_hit;
    bool m_foundAttackable;
  };

  const int RPG_INPUT_MAP_NUM_INPUTS = 64;
  const int RPG_INPUT_MAP_NUM_ALTERNATIVES = 20;
}

RPG_ClosestAttackableRaycastResult::RPG_ClosestAttackableRaycastResult(RPG_BaseEntity const *const entityToIgnore)
: VisPhysicsRaycastBase_cl()
, m_entityToIgnore(entityToIgnore)
, m_closestHit()
, m_closestAttackableHit()
, m_hit(false)
, m_foundAttackable(false)
{
  iCollisionBitmask = hkpGroupFilter::calcFilterInfo(0, 0, 0, 0);
}

void RPG_ClosestAttackableRaycastResult::Reset()
{
  VisPhysicsRaycastBase_cl::Reset();
  m_closestHit.Reset();
  m_closestAttackableHit.Reset();
  m_hit = false;
  m_foundAttackable = false;
  iCollisionBitmask = hkpGroupFilter::calcFilterInfo(0, 0, 0, 0);
}

bool RPG_ClosestAttackableRaycastResult::onHit( VisPhysicsHit_t &hit )
{
  if(!hit.pHitObject)
  {
    return true;
  }
  
  if(hit.pHitObject == m_entityToIgnore)
  {
    return true; // ignore self
  }

  if(hit.pHitObject->IsFrom(RPG_DamageableEntity))
  {
    RPG_DamageableEntity* hitEntity = static_cast<RPG_DamageableEntity*>(hit.pHitObject);
    RPG_AttackableComponent* attackableComponent = static_cast<RPG_AttackableComponent*>(hitEntity->Components().GetComponentOfType(V_RUNTIME_CLASS(RPG_AttackableComponent)));
    if (attackableComponent)
    {
      // hit an attackable entity
      if(hit.fHitFraction < m_closestAttackableHit.fHitFraction)
      {
        m_closestAttackableHit = hit;
        m_foundAttackable = true;
      }
    }
  }

  if(hit.fHitFraction < m_closestHit.fHitFraction)
  {
    m_closestHit = hit;
  }
  
  m_hit = true;

  return true; // keep looking
}

bool RPG_ClosestAttackableRaycastResult::allHits()
{
  return true;
}

// PlayerUIDialog
PlayerUIDialog::PlayerUIDialog(RPG_PlayerControllerComponent *playerController)
  : VDialog(),
  m_playerController(playerController)
  , m_cursor()
  , m_selectDestinationVFX("Particles/UI/RPG_UI_SelectDestination.xml")
  , m_selectDestinationVFXInterval(0.1f)
  , m_lastSelectDestinationVFXTime(0.0f)
  , m_cursorTextureFilename("GUI/Textures/Mouse.dds")
  , m_touchInput(NULL)
{
}

// VDialog
void PlayerUIDialog::OnActivate()
{
  VDialog::OnActivate();

  // get the dimensions of the active render context and scale the UI to those dimensions
  int renderContextWidth, renderContextHeight;
  VisRenderContext_cl::GetCurrentContext()->GetSize(renderContextWidth, renderContextHeight);
  SetSize(static_cast<float>(renderContextWidth), static_cast<float>(renderContextHeight));

  m_inputMap = new VInputMap(RPG_INPUT_MAP_NUM_INPUTS, RPG_INPUT_MAP_NUM_ALTERNATIVES);
  InitInputMap();

  RPG_GuiManager::GetInstance()->OnActivate(this);
}

void PlayerUIDialog::OnDeactivate()
{
  VDialog::OnDeactivate();
  RPG_GuiManager::GetInstance()->OnDeactivate();

  m_inputMap = NULL;
}

void PlayerUIDialog::OnSetFocus(bool focus)
{
  VDialog::OnSetFocus(focus);
#if defined(_VISION_MOBILE)
  m_cursor = GetMenuManager()->LoadCursorResource("GUI/Textures/MouseMobile.tga");
#else
  VASSERT_MSG(!m_cursorTextureFilename.IsEmpty(), "Cursor texture filename hasn't been specified.");
  m_cursor = GetMenuManager()->LoadCursorResource(m_cursorTextureFilename.AsChar());
  VASSERT_MSG(m_cursor, "Cursor failed to load.");
#endif

#if defined(SUPPORTS_MULTITOUCH) && (HAVOK_VISION_RESTRUCTURING) && !defined(_VISION_ANDROID)

  m_touchInput = NULL;

  IVInputDevice* inputDevice = &VInputManagerPC::GetInputDevice(INPUT_DEVICE_TOUCHSCREEN);
  if(inputDevice != &VInputManagerPC::s_NoInputDevice)
    m_touchInput = static_cast<IVMultiTouchInput*>(inputDevice);

#endif
}

void PlayerUIDialog::OnTick(float deltaTime)
{
  VDialog::OnTick(deltaTime);

#if defined (SUPPORTS_MULTITOUCH) && (HAVOK_VISION_RESTRUCTURING) && !defined(_VISION_ANDROID)

  if (m_touchInput != NULL)
  {     
    int count = m_touchInput->GetNumberOfTouchPoints();
    if (count > 1)
      return;
  }

#endif

  RPG_Character *characterEntity = static_cast<RPG_Character *>(m_playerController->GetOwner());
  if (!characterEntity)
  {
    // @todo: handle player death properly
    Vision::Error.Warning("Player character has probably died, which isn't yet handled. Set the character's Unlimited Health entity property to true to prevent this.");
    return;
  }

  RPG_PlayerControllerInput input;
  {
    if(m_inputMap->GetTrigger(PI_PrimaryAction) > 0.0f)
    {
      if(m_inputMap->GetTrigger(PI_ShiftModifier) > 0.0f)
      {
        input.m_buttons |= RPG_PlayerControllerInput::B_RANGED;
      }
      else if(m_inputMap->GetTrigger(PI_SecondaryShiftModifier) > 0.0f)
      {
        input.m_buttons |= RPG_PlayerControllerInput::B_POWER;
      }
      else
      {
        input.m_buttons |= RPG_PlayerControllerInput::B_PRIMARY;
      }
    }

    if(m_inputMap->GetTrigger(PI_SecondaryAction) > 0.0f)
    {
      input.m_buttons |= RPG_PlayerControllerInput::B_POWER;
    }

    if(m_inputMap->GetTrigger(PI_SpecialAction01) > 0.0f)
    {
      input.m_buttons |= RPG_PlayerControllerInput::B_AOE;
    }

    GetFirstAttackableEntityUnderCursor(input.m_targetEntity, input.m_targetPoint, characterEntity);
    if(!input.m_targetEntity)
    {
      hkVector4 hitPoint, characterPos;
      RPG_VisionHavokConversion::VisionToHavokPoint(characterEntity->GetPosition(), characterPos);
      if(GetClosestPointOnNavMeshUnderCursor(hitPoint, characterPos))
      {
        RPG_VisionHavokConversion::HavokToVisionPoint(hitPoint, input.m_targetPoint);
      }
    }

    m_playerController->SetInput(input);
  }

  RPG_GuiManager::GetInstance()->OnTick(deltaTime);
}

VCursor *PlayerUIDialog::GetCurrentCursor(VGUIUserInfo_t& user)
{
  return m_cursor;
}

bool PlayerUIDialog::GetClosestPointOnNavMeshUnderCursor(hkVector4& point, hkVector4 const& searchPoint)
{
  IVGUIContext *const context = GetContext();
  VASSERT(context);
  hkvVec2 const mousePos = GetCursorPosition(context);

  hkvVec3 traceDirOut;
  VisRenderContext_cl::GetCurrentContext()->GetTraceDirFromScreenPos(mousePos.x, mousePos.y, traceDirOut, 1.0f);
  hkvVec3 const& camPos = VisRenderContext_cl::GetCurrentContext()->GetCamera()->GetPosition();

  hkVector4 rayStartHavok, rayEndHavok;
  RPG_VisionHavokConversion::VisionToHavokPoint(camPos, rayStartHavok);
  RPG_VisionHavokConversion::VisionToHavokPoint(camPos + traceDirOut * 5000.0f, rayEndHavok);

  hkaiNavMeshQueryMediator::HitDetails hit;
  if(vHavokAiModule::GetInstance()->GetAiWorld()->getDynamicQueryMediator()->castRay(rayStartHavok, rayEndHavok, hit))
  {
    point.setInterpolate(rayStartHavok, rayEndHavok, hit.m_hitFraction);
    return true;
  }
  else
  {
    // If the ray missed the nav mesh:
    // 1. Find the nav mesh face closest to the search point
    // 2. Intersect the ray with the plane of this face
    // 3. Find the closest point on the nav mesh to the plane point
    hkVector4 dummyPoint;
    hkaiPackedKey const searchFaceKey = 
      vHavokAiModule::GetInstance()->GetAiWorld()->getDynamicQueryMediator()->getClosestPoint(searchPoint, 1.f, dummyPoint);
    if(searchFaceKey != HKAI_INVALID_PACKED_KEY)
    {
      hkVector4 facePlane;
      hkaiNavMeshInstance const *meshInstance = vHavokAiModule::GetInstance()->GetAiWorld()->getStreamingCollection()->getInstanceAt(hkaiGetRuntimeIdFromPacked(searchFaceKey));
      {
        hkaiNavMeshUtils::calcFacePlane(*meshInstance, hkaiGetIndexFromPacked(searchFaceKey), facePlane);
      }

      hkSimdReal const f = facePlane.dot4xyz1(rayStartHavok);
      hkSimdReal const t = facePlane.dot4xyz1(rayEndHavok);
      if(f.isGreaterEqualZero() && t.isLessZero())
      {
        hkVector4 planePoint;
        {
          hkSimdReal const hitFraction = f / (f - t);
          planePoint.setInterpolate(rayStartHavok, rayEndHavok, hitFraction);
        }

        hkaiPackedKey faceKey = vHavokAiModule::GetInstance()->GetAiWorld()->getDynamicQueryMediator()->getClosestPoint(planePoint, 5.f, point);
        return (faceKey != HKAI_INVALID_PACKED_KEY);
      }
    }
  }

  return false;
}

bool PlayerUIDialog::GetFirstAttackableEntityUnderCursor(RPG_DamageableEntity*& attackableEntity, hkvVec3& fallbackTargetPoint, RPG_BaseEntity const *const characterEntity)
{
  IVGUIContext *const context = GetContext();
  VASSERT(context);
  hkvVec2 const mousePos = GetCursorPosition(context);

  hkvVec3 traceDirOut;
  VisRenderContext_cl::GetCurrentContext()->GetTraceDirFromScreenPos(mousePos.x, mousePos.y, traceDirOut, 1.0f);
  hkvVec3 const& camPos = VisRenderContext_cl::GetCurrentContext()->GetCamera()->GetPosition();

  RPG_ClosestAttackableRaycastResult result(characterEntity);
  result.Reset();
  result.vRayStart = camPos;
  result.vRayEnd = camPos + traceDirOut * 5000.0f;

  Vision::GetApplication()->GetPhysicsModule()->PerformRaycast(&result);

  if(result.m_hit)
  {
    if(result.m_foundAttackable)
    {
      attackableEntity = static_cast<RPG_DamageableEntity*>(result.m_closestAttackableHit.pHitObject);
      fallbackTargetPoint = attackableEntity->GetPosition();
    }
    else
    {
      attackableEntity = NULL;
      fallbackTargetPoint = result.m_closestHit.vImpactPoint;
    }

    return true;
  }
  return false;
}

void PlayerUIDialog::CheatToggleUnlimitedHealth()
{
  RPG_Character *characterEntity = static_cast<RPG_Character *>(m_playerController->GetOwner());
  VASSERT(characterEntity);
  characterEntity->GetCharacterStats().SetUnlimitedHealth(!characterEntity->GetCharacterStats().HasUnlimitedHealth());

  VString msg;
  if (characterEntity->GetCharacterStats().HasUnlimitedHealth())
  {
    msg = "CHEAT - Unlimited Health: ON";
  }
  else
  {
    msg = "CHEAT - Unlimited Health: OFF";
  }
  Vision::Error.SystemMessage(msg.AsChar());
  Vision::Message.Add(1, msg.AsChar());
}

void PlayerUIDialog::CheatToggleUnlimitedMana()
{
  RPG_Character *characterEntity = static_cast<RPG_Character *>(m_playerController->GetOwner());
  VASSERT(characterEntity);
  characterEntity->GetCharacterStats().SetUnlimitedMana(!characterEntity->GetCharacterStats().HasUnlimitedMana());

  VString msg;
  if (characterEntity->GetCharacterStats().HasUnlimitedMana())
  {
    msg = "CHEAT - Unlimited Mana: ON";
  }
  else
  {
    msg = "CHEAT - Unlimited Mana: OFF";
  }
  Vision::Error.SystemMessage(msg.AsChar());
  Vision::Message.Add(1, msg.AsChar());
}

void PlayerUIDialog::InitInputMap()
{
#ifdef SUPPORTS_MOUSE
  m_inputMap->MapTrigger(PI_PrimaryAction, V_MOUSE, CT_MOUSE_LEFT_BUTTON);
  m_inputMap->MapTrigger(PI_SecondaryAction, V_MOUSE, CT_MOUSE_RIGHT_BUTTON);
  m_inputMap->MapTrigger(PI_SpecialAction01, V_MOUSE, CT_MOUSE_MIDDLE_BUTTON);
#endif

#ifdef SUPPORTS_KEYBOARD
  // player actions
  m_inputMap->MapTrigger(PI_ShiftModifier, V_KEYBOARD, CT_KB_LSHIFT);
  m_inputMap->MapTrigger(PI_SecondaryShiftModifier, V_KEYBOARD, CT_KB_RSHIFT);
  m_inputMap->MapTrigger(PI_SpecialAction01, V_KEYBOARD, CT_KB_1, VInputOptions::Once());

  // gui
  m_inputMap->MapTrigger(PI_ToggleTestPattern, V_KEYBOARD, CT_KB_S, VInputOptions::Once());
  m_inputMap->MapTrigger(PI_ToggleCharacterDialog, V_KEYBOARD, CT_KB_C, VInputOptions::Once());
  m_inputMap->MapTrigger(PI_ToggleInventoryDialog, V_KEYBOARD, CT_KB_I, VInputOptions::Once());
  m_inputMap->MapTrigger(PI_ToggleMinimap, V_KEYBOARD, CT_KB_M, VInputOptions::Once());
  m_inputMap->MapTrigger(PI_ToggleHud, V_KEYBOARD, CT_KB_H, VInputOptions::Once());

  // testing
  m_inputMap->MapTrigger(PI_TestEffect, V_KEYBOARD, CT_KB_Z, VInputOptions::Once());
  m_inputMap->MapTrigger(PI_TestCameraShake, V_KEYBOARD, CT_KB_X, VInputOptions::Once());
  
  // cheats
  m_inputMap->MapTrigger(PI_CheatUnlimitedHealth, V_KEYBOARD, CT_KB_Z, VInputOptions::Once());
  m_inputMap->MapTrigger(PI_CheatUnlimitedMana, V_KEYBOARD, CT_KB_X, VInputOptions::Once());
#endif

#if defined(_VISION_MOBILE)
  // vSceneViewer profiling menu uses -900 for depth
  // Explicitly place the touchscreen area 'behind' that at -950
  // Though now the rest of the GUI probably doesn't work!
  float const depth = -950.0f;
  int const width = Vision::Video.GetXRes();
  int const height = Vision::Video.GetYRes();

  VTouchArea *touchScreen = new VTouchArea(VInputManager::GetTouchScreen(), VRectanglef(0.0f, 0.0f, width, height), depth);
  m_inputMap->MapTrigger(PI_PrimaryAction, touchScreen, CT_TOUCH_ANY);
  m_inputMap->MapTrigger(PI_PrimaryActionX, touchScreen, CT_TOUCH_ABS_X);
  m_inputMap->MapTrigger(PI_PrimaryActionY, touchScreen, CT_TOUCH_ABS_Y);
#endif
}

// Read the mapped absolute touch position on mobile, mouse cursor pos on desktop
hkvVec2 PlayerUIDialog::GetCursorPosition(IVGUIContext const *context) const
{
#if defined(_VISION_MOBILE)
  float const touchX = m_inputMap->GetTrigger(PI_PrimaryActionX);
  float const touchY = m_inputMap->GetTrigger(PI_PrimaryActionY);
  return hkvVec2(touchX, touchY);
#else
  return context->GetCurrentMousePos();
#endif
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
