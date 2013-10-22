/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */
#ifndef RPG_PLUGIN_PLAYER_UI_DIALOG_H__
#define RPG_PLUGIN_PLAYER_UI_DIALOG_H__

#include <Vision/Runtime/Engine/Physics/IVisApiPhysicsModule.hpp>

class RPG_BaseEntity;
class RPG_Character;
class RPG_DamageableEntity;

class RPG_PlayerControllerComponent;
typedef VSmartPtr<RPG_PlayerControllerComponent> RPG_PlayerControllerComponentPtr;

enum RPG_PlayerInputs_e
{
  PI_Invalid = -1,
  PI_PrimaryAction =0,
  PI_PrimaryActionX,
  PI_PrimaryActionY,
  PI_SecondaryAction,
  PI_SpecialAction01,
  PI_ShiftModifier,
  PI_SecondaryShiftModifier,
  PI_ToggleTestPattern,
  PI_ToggleMinimap,
  PI_ToggleInventoryDialog,
  PI_ToggleCharacterDialog,
  PI_ToggleHud,
  PI_TestEffect,
  PI_TestCameraShake,
  PI_CheatUnlimitedHealth,
  PI_CheatUnlimitedMana,
  PI_Count
};

// Player UI dialog
class PlayerUIDialog : public VDialog
{
public:
  PlayerUIDialog(RPG_PlayerControllerComponent *playerController);

  VInputMap *GetInputMap() { return m_inputMap; }

private:
  //@{
  // VDialog
  void OnActivate() HKV_OVERRIDE;
  void OnDeactivate() HKV_OVERRIDE;
  void OnSetFocus(bool focus) HKV_OVERRIDE;
  void OnTick(float deltaTime) HKV_OVERRIDE;

  VCursor *GetCurrentCursor(VGUIUserInfo_t& user) HKV_OVERRIDE;
  //@}

  //@{
  // Input handling
  bool GetClosestPointOnNavMeshUnderCursor(hkVector4& point, hkVector4 const& searchPoint);
  bool GetFirstAttackableEntityUnderCursor(RPG_DamageableEntity*& attackableEntity, hkvVec3& fallbackTargetPoint, RPG_BaseEntity const *const characterEntity);
  //@}

  //@{
  // Cheats
  void CheatToggleUnlimitedHealth();
  void CheatToggleUnlimitedMana();
  //@}

  void InitInputMap();
  hkvVec2 GetCursorPosition(IVGUIContext const *context) const;

private:
  RPG_PlayerControllerComponentPtr m_playerController;

  VSmartPtr<VInputMap> m_inputMap;

  VCursorPtr m_cursor;
  VString m_selectDestinationVFX;                 ///< Effect to play when player clicks on a navigation destination
  VString m_cursorTextureFilename;          
  float m_selectDestinationVFXInterval;           ///< Minimum time between select destination effect spawns
  float m_lastSelectDestinationVFXTime;           ///< Last time a select destination effect was played

#if defined(SUPPORTS_MULTITOUCH)

  IVMultiTouchInput* m_touchInput;

#endif

};

#endif  // RPG_PLUGIN_PLAYER_UI_DIALOG_H__

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
