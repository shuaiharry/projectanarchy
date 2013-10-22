/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

#ifndef MENU_SYSTEM_HPP_INCLUDED
#define MENU_SYSTEM_HPP_INCLUDED

#include <Vision/Runtime/EnginePlugins/VisionEnginePlugin/GUI/VMenuIncludes.hpp>
#include <Vision/Runtime/EnginePlugins/VisionEnginePlugin/Components/VEntityLODComponent.hpp>

class SubMenu_Skeletal;
class VDialogSubCtrl;
class LODObject;

#define ENTITY_COUNT    9

class AnimationMainMenu : public VDialog
{
public:
  virtual void OnInitDialog() HKV_OVERRIDE;
  virtual void OnValueChanged(VItemValueChangedEvent *pEvent) HKV_OVERRIDE;

  void SetLODLevel(VEntityLODLevel_e eMode);

protected:
  V_DECLARE_SERIAL_DLLEXP( AnimationMainMenu, DECLSPEC_DLLEXPORT )
  virtual void Serialize(VArchive &ar) HKV_OVERRIDE {}

  // entities
  VisBaseEntity_cl *m_pSoldiers[ENTITY_COUNT];

  // sub menus
  VDialogSubCtrl *m_pSubMenu;
  VDialogPtr m_spSubMenuDialog;
  VListControl *m_pLODSelectionList;
  VCheckBox *m_pCheckBoxStopPlay;
};

class VDialogSubCtrl : public VDlgControlBase
{
public:

  // specific functions
  inline void SetDialog(VDialog *pDialog) 
  {
    m_spDialog = pDialog;
  }

  inline VDialog* GetDialog() const 
  {
    return m_spDialog;
  }

  // overridden control functions
  virtual void OnPaint(VGraphicsInfo &Graphics, const VItemRenderInfo &parentState) HKV_OVERRIDE
  {
    if (m_spDialog) 
      m_spDialog->OnPaint(Graphics,parentState);
  }

  virtual void OnTick(float dtime) HKV_OVERRIDE
  {
    if (m_spDialog) 
      m_spDialog->OnTick(dtime);
  }

  virtual VWindowBase* TestMouseOver(VGUIUserInfo_t &user, const hkvVec2 &vAbsMouse) HKV_OVERRIDE;

  virtual VCursor *GetMouseOverCursor(VGUIUserInfo_t &user) HKV_OVERRIDE
  {
    if (m_spDialog) 
      return m_spDialog->GetMouseOverCursor(user);return VDlgControlBase::GetMouseOverCursor(user);
  }

  virtual VTooltip* GetTooltip(VGUIUserInfo_t &user) HKV_OVERRIDE
  {
    if (m_spDialog) 
      return m_spDialog->GetTooltip(user);return VDlgControlBase::GetTooltip(user);
  }

protected:
  V_DECLARE_SERIAL_DLLEXP( VDialogSubCtrl, DECLSPEC_DLLEXPORT )
  virtual void Serialize( VArchive &ar ) {}

  VDialogPtr m_spDialog;
};

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
