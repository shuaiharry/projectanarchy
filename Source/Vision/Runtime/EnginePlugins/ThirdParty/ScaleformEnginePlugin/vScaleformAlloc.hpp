/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

#ifndef __SCALEFORM_ALLOC_CPP
#define __SCALEFORM_ALLOC_CPP

#include "Render/Render_MemoryManager.h"

/// \brief  Scaleform memory allocator implementation for Vision.
class VScaleformSysAlloc : public Scaleform::SysAlloc
{ 
public: 

  /// \brief  Implementation of Scaleform SysAlloc interface
  virtual void* Alloc(Scaleform::UPInt size, Scaleform::UPInt align)
  {
    return vMemAlignedAlloc(size, (int)align);
  }

  /// \brief  Implementation of Scaleform SysAlloc interface
  virtual void Free(void* ptr, Scaleform::UPInt size, Scaleform::UPInt align)
  {
    vMemAlignedFree(ptr);
  }

  /// \brief  Implementation of Scaleform SysAlloc interface
  virtual void* Realloc(void* oldPtr, Scaleform::UPInt oldSize, Scaleform::UPInt newSize, Scaleform::UPInt align)
  {
    void *newPtr = Alloc(newSize, (int)align);
    memcpy(newPtr, oldPtr, hkvMath::Min (oldSize, newSize));
    Free(oldPtr,oldSize,align);
    return newPtr;
  }
};

#if defined (_VISION_PS3)

/// \brief  Scaleform memory manager for PS3
class VScaleformMemoryManager : public Scaleform::Render::MemoryManager
{
public:
  /// \brief  Alloc implementation of the memory manager
  virtual void* Alloc(Scaleform::UPInt size, Scaleform::UPInt align, Scaleform::Render::MemoryType type, unsigned arena = 0)
  {
#if defined (_VISION_PS3)
    VGcmMemory gcmMemory;
    if( !VVideo::GetGCMAllocator()->AlignedAlloc( size, align, type == Scaleform::Render::Memory_PS3_MainVideo ? V_GCM_LOCATION_MAIN_MAPPED : V_GCM_LOCATION_LOCAL, 
      &gcmMemory, V_GCM_MEMORY_HINT_NONE, false ) )
    {
      return NULL;
    } 
    return gcmMemory.m_pAddress;
#endif
  }

  /// \brief  Free implementation of the memory manager
  virtual void Free(void* pmem, Scaleform::UPInt size)
  {
#if defined (_VISION_PS3) 
    VGcmMemory gcmMemory;

    void* pMainStart = Vision::Video.GetMappedMainMemoryAddress();
    void* pMainEnd = (char*)pMainStart + Vision::Video.GetGcmConfig()->uiMappedMainMemorySize;
    if(pmem >= pMainStart && pmem < pMainEnd)
      gcmMemory.m_uiLocation = V_GCM_LOCATION_MAIN_MAPPED;
    else
      gcmMemory.m_uiLocation = V_GCM_LOCATION_LOCAL;

    gcmMemory.m_pAddress = pmem;
    gcmMemory.m_uiSize = size;
    VVideo::GetGCMAllocator()->Free(&gcmMemory);
#endif
  }
};

#endif


#endif //__SCALEFORM_ALLOC_CPP

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
