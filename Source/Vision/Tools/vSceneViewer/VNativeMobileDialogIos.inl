/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

extern "C" void* VNativeMobileDialogIos_CreateAndShowInstance(void* pWrapper, const char* pTitle, const char* pText,
  const char* pButton0, const char* pButton1, const char* pButton2);

extern "C" void VNativeMobileDialogIos_ChangeText(void* pInstance, const char* pText);
  
extern "C" void VNativeMobileDialogIos_CloseInstance(void* pInstance);

class VNativeMobileDialogIos : public IVNativeMobileDialog
{
public:
  VNativeMobileDialogIos()
  {
    m_eState = WAITING;
    m_iNumButtons = 0;
    m_pDialog = NULL;
  }

  virtual ~VNativeMobileDialogIos()
  {
    Close();
  }

  virtual void SetTitle(const char* pTitle) HKV_OVERRIDE
  {
    m_titleText = pTitle;
  }

  virtual void SetText(const char* pText) HKV_OVERRIDE
  {
    m_text = pText;

    if(m_pDialog)
    {
      VNativeMobileDialogIos_ChangeText(m_pDialog, pText);
    }
  }

  virtual int AddButton(const char* pButtonText) HKV_OVERRIDE
  {
    VASSERT_MSG(m_iNumButtons < 3, "Can only add at most 3 buttons on iOS");
    m_buttonText[m_iNumButtons] = pButtonText;
    return m_iNumButtons++;
  }

  virtual void Show() HKV_OVERRIDE
  {
    m_pDialog = VNativeMobileDialogIos_CreateAndShowInstance(this, m_titleText.AsChar(),
      m_text.AsChar(), m_buttonText[0].AsChar(), m_buttonText[1].AsChar(), m_buttonText[2].AsChar());
  }

  virtual void Close() HKV_OVERRIDE
  {
    if(m_pDialog)
    {
      VNativeMobileDialogIos_CloseInstance(m_pDialog);
      m_pDialog = NULL;
    }
  }

  virtual State GetState()
  {
    return m_eState;
  }

  void* m_pDialog;

  State m_eState;

  VString m_titleText;
  VString m_text;
  VString m_buttonText[3];
  int m_iNumButtons;
};

IVNativeMobileDialog* IVNativeMobileDialog::CreateInstance()
{
  return new VNativeMobileDialogIos();
}

extern "C" void VNativeMobileDialogIos_SetResult(void* pWrapper, int iResult)
{
  reinterpret_cast<VNativeMobileDialogIos*>(pWrapper)->m_eState = static_cast<VNativeMobileDialogIos::State>(iResult);
}

/*
 * Havok SDK - Base file, BUILD(#20131021)
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
