/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */
#ifndef HKAI_SEARCH_PARAMS_H
#define HKAI_SEARCH_PARAMS_H

#include <Common/Base/Container/LocalArray/hkLocalArray.h>

	/// Memory control defaults
struct hkaiSearchParameters
{
	HK_DECLARE_REFLECTION();
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_AI_ASTAR, hkaiSearchParameters );
	//
	//	Default values for controlling search memory usage are given in two variants:
	//	* Default values for single-threaded, synchronous requests (via hkaiPathfindingUtil::findPath)
	//	  These default values are generously large, and can be reduced to conserve memory.
	//	* default values for multi-threaded, asynchronous requests (via hkaiNavMeshAStarJob)
	//	  These default values are sized for executing searches on the SPUs, and cannot be increased
	//	  for jobs processed on the SPU.
	//

	enum ElementSizes
	{
#ifndef HK_REAL_IS_DOUBLE
		OPENSET_ELEMENT_SIZE = 8, // hkaiHeapOpenSet::IndexCostPair
		SEARCH_STATE_ELEMENT_SIZE = 16 + 2, // sizeof(hkaiSearchStateNode) + sizeof(hkInt16)
#else
		OPENSET_ELEMENT_SIZE = 16, // hkaiHeapOpenSet::IndexCostPair
		SEARCH_STATE_ELEMENT_SIZE = 32 + 2, // sizeof(hkaiSearchStateNode) + sizeof(hkInt16)
#endif
		SEARCH_STATE_OVERHEAD = 512 // sizeof(hkInt16)*256
	};

	enum
	{
		OPEN_SET_SINGLE_THREADED_NUM_ELEMENTS = 16384,
		SEARCH_STATE_SINGLE_THREADED_NUM_ELEMENTS = 2*OPEN_SET_SINGLE_THREADED_NUM_ELEMENTS,
		HIERARCHY_OPEN_SET_SINGLE_THREADED_NUM_ELEMENTS = 4096,
		HIERARCHY_SEARCH_STATE_SINGLE_THREADED_NUM_ELEMENTS = 2*HIERARCHY_OPEN_SET_SINGLE_THREADED_NUM_ELEMENTS,

		OPEN_SET_MULTI_THREADED_NUM_ELEMENTS = 1024,
		SEARCH_STATE_MULTI_THREADED_NUM_ELEMENTS = 2*OPEN_SET_MULTI_THREADED_NUM_ELEMENTS,
		HIERARCHY_OPEN_SET_MULTI_THREADED_NUM_ELEMENTS = 256,
		HIERARCHY_SEARCH_STATE_MULTI_THREADED_NUM_ELEMENTS = 2*HIERARCHY_OPEN_SET_MULTI_THREADED_NUM_ELEMENTS,
	};

	/// Memory defaults for single-threaded, synchronous requests
	enum MemoryDefaultsSingleThreaded
	{
		OPEN_SET_SIZE_SINGLE_THREADED				= OPEN_SET_SINGLE_THREADED_NUM_ELEMENTS               * OPENSET_ELEMENT_SIZE                              /* 131072 */,
		SEARCH_STATE_SIZE_SINGLE_THREADED			= SEARCH_STATE_SINGLE_THREADED_NUM_ELEMENTS           * SEARCH_STATE_ELEMENT_SIZE + SEARCH_STATE_OVERHEAD /* 590336 */,
		HIERARCHY_OPEN_SET_SIZE_SINGLE_THREADED		= HIERARCHY_OPEN_SET_SINGLE_THREADED_NUM_ELEMENTS     * OPENSET_ELEMENT_SIZE                              /*  32768 */,
		HIERARCHY_SEARCH_STATE_SIZE_SINGLE_THREADED	= HIERARCHY_SEARCH_STATE_SINGLE_THREADED_NUM_ELEMENTS * SEARCH_STATE_ELEMENT_SIZE + SEARCH_STATE_OVERHEAD /* 147968 */,
	};

	/// Memory defaults for multi-threaded, asynchronous requests
	enum MemoryDefaultsMultiThreaded
	{
		OPEN_SET_SIZE_MULTI_THREADED				= OPEN_SET_MULTI_THREADED_NUM_ELEMENTS               * OPENSET_ELEMENT_SIZE                              /*  8192 */,
		SEARCH_STATE_SIZE_MULTI_THREADED			= SEARCH_STATE_MULTI_THREADED_NUM_ELEMENTS           * SEARCH_STATE_ELEMENT_SIZE + SEARCH_STATE_OVERHEAD /* 37376 */,
		HIERARCHY_OPEN_SET_SIZE_MULTI_THREADED		= HIERARCHY_OPEN_SET_MULTI_THREADED_NUM_ELEMENTS     * OPENSET_ELEMENT_SIZE                              /*  2048 */,
		HIERARCHY_SEARCH_STATE_SIZE_MULTI_THREADED	= HIERARCHY_SEARCH_STATE_MULTI_THREADED_NUM_ELEMENTS * SEARCH_STATE_ELEMENT_SIZE + SEARCH_STATE_OVERHEAD /*  9728 */,
	};

		/// Pair of sizes to indicate how big the open set and search state should be.
	struct BufferSizes
	{
		HK_DECLARE_REFLECTION();
		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_AI_ASTAR, BufferSizes );

		inline BufferSizes();
		inline BufferSizes( hkFinishLoadedObjectFlag f) {};
		inline bool operator==( const BufferSizes& other ) const;

			/// Whether the search is being run in single-threaded or multi-threaded.
			/// Used to determine the default buffer size.
		enum ThreadingType
		{
			SINGLE_THREADED,
			MULTI_THREADED
		};

			/// Whether the buffers are for a full A* search, or the subsearch used by hierarchical A*.
			/// Used to determine the default buffer size.
		enum SearchType
		{
			SEARCH_REGULAR,
			SEARCH_HIERARCHICAL
		};

		inline void update( ThreadingType tt, SearchType st);
		inline void setEmpty();
	

			/// Maximum memory for nodes stored the open set, in bytes. A value of 0 will use the appropriate default.
			/// Note that the open set size can be considerably lower than the search state size, since the open set can never
			/// grow larger than the search state, and nodes in the open set are half the size of search state nodes (8 vs. 16)
		int m_maxOpenSetSizeBytes; //+default(0)

			/// Maximum memory for nodes stored if the search state, in bytes. A value of 0 will use the appropriate default.
		int m_maxSearchStateSizeBytes; //+default(0)
	};

	struct SearchBuffers
	{
		HK_DECLARE_REFLECTION();
		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_AI_ASTAR, SearchBuffers );

		inline SearchBuffers();

			/// Open set buffer.
			/// Assumed to be of size BufferSizes::m_maxOpenSetSizeBytes
		char* m_openSetBuffer; //+nosave

			/// Search state buffer.
			/// Assumed to be of size BufferSizes::m_maxSearchStateSizeBytes
		char* m_searchStateBuffer; //+nosave
	};
};


	/// Memory initialization information.
struct hkaiSearchMemoryInfo
{
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_BASE,hkaiSearchMemoryInfo);
	char* m_openSetBuffer;
	char* m_searchStateBuffer;
	int   m_openSetSize; // size in bytes
	int   m_searchStateSize; // size in bytes

	inline void setEmpty();
	inline void init( hkArray<char>& openSetArray, hkArray<char>& searchStateArray );
#ifndef HK_PLATFORM_SPU
	inline void init( hkArray<char>::Temp& openSetArray, hkArray<char>::Temp& searchStateArray );
#endif
};

#ifndef HK_PLATFORM_SPU

	/// Version of hkaiSearchMemoryInfo that keeps its own memory in an hkArray.
struct hkaiSearchMemoryStorage : public hkaiSearchMemoryInfo
{
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_BASE,hkaiSearchMemoryStorage);
	inline hkaiSearchMemoryStorage() {}
	inline ~hkaiSearchMemoryStorage() {}
	inline void initBuffers( const hkaiSearchParameters::SearchBuffers& buffers, const hkaiSearchParameters::BufferSizes& bufferSizes);

		// If the buffer is non-null, set the array to use it, otherwise resize the array
	inline static void HK_CALL _setupArray(hkArray<char>::Temp& array, char* buffer, int size);

	hkArray<char>::Temp m_openSetStorage;
	hkArray<char>::Temp m_searchStateStorage;
};
#endif

	/// Version of hkaiSearchMemoryInfo that keeps its own memory in an hkLocalArray.
struct hkaiSearchMemoryLocalStorage : public hkaiSearchMemoryInfo
{
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_BASE,hkaiSearchMemoryLocalStorage);
	inline hkaiSearchMemoryLocalStorage( const hkaiSearchParameters::BufferSizes& bufferSizes );

	hkLocalArray<char> m_openSetStorage;
	hkLocalArray<char> m_searchStateStorage;
};


#include <Ai/Pathfinding/Astar/Search/hkaiSearchParams.inl>

#endif // HKAI_SEARCH_PARAMS_H

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
