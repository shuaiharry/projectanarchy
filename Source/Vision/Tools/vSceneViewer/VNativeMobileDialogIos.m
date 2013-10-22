/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

#include <UIKit/UIKit.h>

extern void* VNativeMobileDialogIos_SetResult(void* pWrapper, int iResult);

@interface VNativeMobileDialogIos_ObjC : NSObject
{
  UIAlertView* m_pAlertView;
  void* m_pWrapper;
}
@end

@implementation VNativeMobileDialogIos_ObjC

-(id)initWithWrapper: (void*)pWrapper title:(const char*)pTitle text:(const char*)pText button0:(const char*)pButton0 button1:(const char*)pButton1 button2:(const char*)pButton2
{
  self = [super init];
  
  if(self != nil)
  {
    m_pAlertView = [[UIAlertView alloc]
                 initWithTitle: [NSString stringWithUTF8String:pTitle]
                 message:[NSString stringWithUTF8String:pText]
                    delegate:self
                    cancelButtonTitle: nil
                    otherButtonTitles: nil];
    
    if(pButton0 && pButton0[0])
    {
      [m_pAlertView addButtonWithTitle:[NSString stringWithUTF8String:pButton0]];
    }
    
    if(pButton1 && pButton1[0])
    {
      [m_pAlertView addButtonWithTitle:[NSString stringWithUTF8String:pButton1]];
    }
    
    if(pButton2 && pButton2[0])
    {
      [m_pAlertView addButtonWithTitle:[NSString stringWithUTF8String:pButton2]];
    }
    
    [m_pAlertView show];
    
    m_pWrapper = pWrapper;
  }
  
  return self;
}

-(void)close
{
  [m_pAlertView dismissWithClickedButtonIndex:-1 animated:NO];
}

-(void)setText:(NSString*)text
{
  [m_pAlertView setMessage:text];
}

- (void)alertView:(UIAlertView *)anAlertView clickedButtonAtIndex:(NSInteger)anIndex
{
  if(anIndex >= 0)
  {
   VNativeMobileDialogIos_SetResult(m_pWrapper, anIndex);
  }
}

@end

void* VNativeMobileDialogIos_CreateAndShowInstance(void* pWrapper, const char* pTitle, const char* pText,
                                                   const char* pButton0, const char* pButton1, const char* pButton2)
{
  return [[VNativeMobileDialogIos_ObjC alloc] initWithWrapper:pWrapper title:pTitle text:pText button0:pButton0 button1:pButton1 button2:pButton2];
}

void VNativeMobileDialogIos_ChangeText(void* pInstance, const char* pText)
{
  [(VNativeMobileDialogIos_ObjC*)pInstance setText:[NSString stringWithUTF8String:pText]];
}

void VNativeMobileDialogIos_CloseInstance(void* pInstance)
{
  [(VNativeMobileDialogIos_ObjC*)pInstance close];
  [(VNativeMobileDialogIos_ObjC*)pInstance dealloc];
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
