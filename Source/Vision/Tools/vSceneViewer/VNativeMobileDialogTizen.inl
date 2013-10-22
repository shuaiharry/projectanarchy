/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

#include <FUi.h>
#include <FGraphics.h>

using namespace Tizen::Graphics;
using namespace Tizen::Ui;
using namespace Tizen::Ui::Controls;

class VNativeMobileDialogTizen : public IVNativeMobileDialog, public IActionEventListener
{
public:
  class PopupEventListener : public IPropagatedKeyEventListener
  {
  public:
    PopupEventListener(VNativeMobileDialogTizen* pInstance) : m_pInstance(pInstance)
    {
    }

    // key events for back-key
    virtual bool OnKeyPressed(Tizen::Ui::Control& source, const Tizen::Ui::KeyEventInfo& keyEventInfo) { return false; };
    virtual bool OnKeyReleased(Tizen::Ui::Control& source, const Tizen::Ui::KeyEventInfo& keyEventInfo)
    {
      KeyCode key = keyEventInfo.GetKeyCode();
      if (key == KEY_BACK || key == KEY_ESC)
      {
        m_pInstance->m_eState = VNativeMobileDialogTizen::CANCELLED;
      }

      return false;
    }

    virtual bool OnPreviewKeyPressed(Tizen::Ui::Control& source, const Tizen::Ui::KeyEventInfo& keyEventInfo) { return false; };
    virtual bool OnPreviewKeyReleased(Tizen::Ui::Control& source, const Tizen::Ui::KeyEventInfo& keyEventInfo) { return false; };
    virtual bool TranslateKeyEventInfo(Tizen::Ui::Control& source, Tizen::Ui::KeyEventInfo& keyEventInfo) { return false; };

    VNativeMobileDialogTizen* m_pInstance;
  };

  VNativeMobileDialogTizen()
  {
    m_pPopup = new Popup;
    m_pEventListener = new PopupEventListener(this);
    
    VerticalBoxLayout layoutTemplate;
    layoutTemplate.Construct(VERTICAL_DIRECTION_DOWNWARD);
    m_pPopup->Construct(layoutTemplate, true, Dimension(620, 620));

    m_pPopup->SetPropagatedKeyEventListener(m_pEventListener);

    m_pLayout = static_cast<VerticalBoxLayout*>(m_pPopup->GetLayoutN());

    // Creates an instance of Label
    m_pLabel = new Label();
    m_pLabel->Construct(Rectangle(0, 0, 550, 300), "");

    // Adds the label to the form
    m_pPopup->AddControl(m_pLabel);

    m_pLayout->SetHorizontalAlignment(*m_pLabel, LAYOUT_HORIZONTAL_ALIGN_CENTER);
    m_pLayout->SetHorizontalFitPolicy(*m_pLabel, FIT_POLICY_CONTENT);
    m_pLayout->SetSpacing(*m_pLabel, 10);

    m_iLastButtonId = 0;

    m_eState = WAITING;
  }

  virtual ~VNativeMobileDialogTizen()
  {
    m_pPopup->Destroy();
    delete m_pEventListener;
  }

  virtual void SetTitle(const char* pTitle) HKV_OVERRIDE
  {
    m_pPopup->SetTitleText(pTitle);
  }

  virtual void SetText(const char* pText) HKV_OVERRIDE
  {
    m_pLabel->SetText(pText);
    m_pPopup->Invalidate(true);
  }

  virtual int AddButton(const char* pButtonText) HKV_OVERRIDE
  {
    Button* pButton = new Button();
    pButton->Construct(Rectangle(0, 0, 250, 60), pButtonText);

    pButton->SetActionId(m_iLastButtonId);
    pButton->AddActionEventListener(*this);

    m_pPopup->AddControl(pButton);

    m_pLayout->SetHorizontalFitPolicy(*pButton, FIT_POLICY_PARENT);
    m_pLayout->SetSpacing(*pButton, 10);

    return m_iLastButtonId++;
  }

  virtual void Show() HKV_OVERRIDE
  {
    m_pPopup->SetShowState(true);
    m_pPopup->Show();
  }

  virtual void Close() HKV_OVERRIDE
  {
    m_pPopup->SetShowState(false);
  }

  virtual State GetState() HKV_OVERRIDE
  {
    return m_eState;
  }

  virtual void OnActionPerformed(const Tizen::Ui::Control& source, int actionId)
  {
    m_eState = static_cast<State>(actionId);
  }

  Popup* m_pPopup;
  PopupEventListener *m_pEventListener;

  Label* m_pLabel;
  VerticalBoxLayout* m_pLayout;
  int m_iLastButtonId;

  State m_eState;
};

IVNativeMobileDialog* IVNativeMobileDialog::CreateInstance()
{
  return new VNativeMobileDialogTizen();
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
