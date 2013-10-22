/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

#ifndef HK_PLATFORM_SPU

template<typename DATA>
HK_FORCE_INLINE void hkaiUserDataUtils::copyUserData( DATA* dst, const DATA* src, int striding )
{
	// Special-case the 0 and 1
	if (striding == 1)
	{
		*dst = *src;
	}
	else if (striding == 0)
	{
		return;
	}
	else
	{
		for (int i=0; i<striding; i++)
		{
			dst[i] = src[i];
		}
	}
}

template< typename DATA >
HK_FORCE_INLINE void hkaiUserDataUtils::copyUserData( hkArrayBase<DATA>& dataArray, int dstIndex, int srcIndex, int striding)
{
	copyUserData( dataArray.begin() + dstIndex*striding, dataArray.begin() + srcIndex*striding, striding);
}



template<typename DATA>
inline hkResult hkaiUserDataUtils::restride( hkArray<DATA>& data, int oldStride, int newStride, int N, int fillValue )
{
	HK_ASSERT(0x6c3bd4eb, data.getSize() == oldStride*N );

	if (newStride == oldStride)
	{
		return HK_SUCCESS;
	}

	if (newStride == 0)
	{
		data.clear();
		return HK_SUCCESS;
	}

	typename hkArray<DATA>::Temp oldData;
	hkResult tempRes = oldData.reserve( data.getSize() );
	if(tempRes != HK_SUCCESS)
		return HK_FAILURE;

	hkResult dataRes = data.reserve( N * newStride );
	if(dataRes != HK_SUCCESS)
		return HK_FAILURE;

	if ( oldStride == 0 )
	{
		// Simple fill, no copying needed
		data.setSize( N*newStride, fillValue );
		return HK_SUCCESS;
	}

	oldData = data;
	data.setSize(N * newStride);
	const int minStride = hkMath::min2(oldStride, newStride);

	for (int i=0; i<N; i++)
	{
		DATA* dst = &data[i*newStride];
		const DATA* src = &oldData[i*oldStride];
		hkaiUserDataUtils::copyUserData(dst, src, minStride);

		if (newStride > oldStride)
		{
			for (int j=oldStride; j<newStride; j++)
			{
				dst[j] = fillValue;
			}
		}
	}

	return HK_SUCCESS;
}
#endif //!SPU

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
