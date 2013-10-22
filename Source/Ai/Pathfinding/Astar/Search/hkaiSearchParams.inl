/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */


inline hkaiSearchParameters::BufferSizes::BufferSizes()
:	m_maxOpenSetSizeBytes(0),
	m_maxSearchStateSizeBytes(0)
{
}


inline hkaiSearchParameters::SearchBuffers::SearchBuffers()
:	m_openSetBuffer(HK_NULL),
	m_searchStateBuffer(HK_NULL)
{
}

// OPEN_SET_SIZE_SINGLE_THREADED				
// SEARCH_STATE_SIZE_SINGLE_THREADED			
// HIERARCHY_OPEN_SET_SIZE_SINGLE_THREADED		
// HIERARCHY_SEARCH_STATE_SIZE_SINGLE_THREADE
// 
// 
// OPEN_SET_SIZE_MULTI_THREADED				
// SEARCH_STATE_SIZE_MULTI_THREADED			
// HIERARCHY_OPEN_SET_SIZE_MULTI_THREADED		
// HIERARCHY_SEARCH_STATE_SIZE_MULTI_THREADED


inline void hkaiSearchParameters::BufferSizes::update( ThreadingType tt, SearchType st )
{
	int openSetDefault = 0;
	int searchStateDefault = 0;

	if( tt == SINGLE_THREADED)
	{
		if (st == SEARCH_REGULAR)
		{
			openSetDefault		= OPEN_SET_SIZE_SINGLE_THREADED;
			searchStateDefault	= SEARCH_STATE_SIZE_SINGLE_THREADED;
		}
		else // HIERARCHICAL
		{
			openSetDefault		= HIERARCHY_OPEN_SET_SIZE_SINGLE_THREADED;
			searchStateDefault	= HIERARCHY_SEARCH_STATE_SIZE_SINGLE_THREADED;
		}
	}
	else
	{
		if (st == SEARCH_REGULAR)
		{
			openSetDefault		= OPEN_SET_SIZE_MULTI_THREADED;
			searchStateDefault	= SEARCH_STATE_SIZE_MULTI_THREADED;
		}
		else // HIERARCHICAL
		{
			openSetDefault		= HIERARCHY_OPEN_SET_SIZE_MULTI_THREADED;
			searchStateDefault	= HIERARCHY_SEARCH_STATE_SIZE_MULTI_THREADED;
		}
	}

	m_maxOpenSetSizeBytes = (m_maxOpenSetSizeBytes > 0 ? m_maxOpenSetSizeBytes : openSetDefault);
	m_maxSearchStateSizeBytes = (m_maxSearchStateSizeBytes > 0 ? m_maxSearchStateSizeBytes : searchStateDefault);

#ifdef HK_PLATFORM_SPU
	// We can't go over the existing defaults on SPU. Smaller is fine, though.	
	m_maxOpenSetSizeBytes = hkMath::min2<int> ( openSetDefault, m_maxOpenSetSizeBytes );
	m_maxSearchStateSizeBytes = hkMath::min2<int> ( searchStateDefault, m_maxSearchStateSizeBytes );
#endif
}


inline void hkaiSearchParameters::BufferSizes::setEmpty()
{
	m_maxOpenSetSizeBytes = 0;
	m_maxSearchStateSizeBytes = 0;
}


inline bool hkaiSearchParameters::BufferSizes::operator==( const BufferSizes& other ) const
{
	return (m_maxOpenSetSizeBytes == other.m_maxOpenSetSizeBytes) &&
		(m_maxSearchStateSizeBytes == other.m_maxSearchStateSizeBytes);
}


inline void hkaiSearchMemoryInfo::setEmpty( )
{
	m_openSetBuffer = HK_NULL;
	m_openSetSize = 0;
	m_searchStateBuffer = HK_NULL;
	m_searchStateSize = 0;
}

inline void hkaiSearchMemoryInfo::init( hkArray<char>& openSetArray, hkArray<char>& searchStateArray )
{
	m_openSetBuffer = openSetArray.begin();
	m_openSetSize = openSetArray.getCapacity();
	m_searchStateBuffer = searchStateArray.begin();
	m_searchStateSize = searchStateArray.getCapacity();
}

inline hkaiSearchMemoryLocalStorage::hkaiSearchMemoryLocalStorage( const hkaiSearchParameters::BufferSizes& bufferSizes )
:	m_openSetStorage(bufferSizes.m_maxOpenSetSizeBytes), 
	m_searchStateStorage(bufferSizes.m_maxSearchStateSizeBytes)
{
	init( m_openSetStorage, m_searchStateStorage );
}

#ifndef HK_PLATFORM_SPU
inline void hkaiSearchMemoryInfo::init( hkArray<char>::Temp& openSetArray, hkArray<char>::Temp& searchStateArray )
{
	m_openSetBuffer = openSetArray.begin();
	m_openSetSize = openSetArray.getCapacity();
	m_searchStateBuffer = searchStateArray.begin();
	m_searchStateSize = searchStateArray.getCapacity();
}

inline void hkaiSearchMemoryStorage::initBuffers( const hkaiSearchParameters::SearchBuffers& buffers, const hkaiSearchParameters::BufferSizes& bufferSizes )
{
	_setupArray(m_openSetStorage,     buffers.m_openSetBuffer,     bufferSizes.m_maxOpenSetSizeBytes );
	_setupArray(m_searchStateStorage, buffers.m_searchStateBuffer, bufferSizes.m_maxSearchStateSizeBytes );
	init(m_openSetStorage, m_searchStateStorage);
}

inline void HK_CALL hkaiSearchMemoryStorage::_setupArray(hkArray<char>::Temp& array, char* buffer, int size)
{
	HK_ASSERT(0x7752effc, array.isEmpty());
	if (buffer)
		array.setDataUserFree(buffer, 0, size);
	else
		array.setSize(size);
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
