/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

#ifndef HKV_FLAG_FIELD_ARRAY_HPP_INCLUDED
#define HKV_FLAG_FIELD_ARRAY_HPP_INCLUDED

/// \brief
///   Encapsulates an array of flag fields.
///
/// The benefit of using this class vs. embedding the flags inside the flagged objects
/// is that querying how many times a flag is set is an O(1) operation.
///
/// \tparam T
///   the type to contain each elements' flags in. Must be an integral type; preferrably unsigned.
template <typename T>
class hkvFlagFieldArray
{
public:
  /// \brief Constants used by this class
  enum {
    NumFlags = sizeof(T) * 8 ///< the maximum number of possible flags
  };

  typedef T ElementType; ///< an alias to the template paramter type \c T

public:
  /// \brief
  ///   Constructor; initializes an empty flag field array.
  hkvFlagFieldArray();

public:
  /// \brief
  ///   Clears this array; i.e., sets the size and all flag counters to \c 0.
  void clear();

  /// \brief
  ///   Sets the specified flags of all elements contained in this array.
  ///
  /// This method uses two parameters to specify what actually gets set. While the
  /// \c flags parameter contains the bit pattern to set, the \c mask parameter 
  /// determines which bits actually get changed. For each bit that is \c 1 in the
  /// \c mask, the corresponding bit of \c flags gets set. All bits that are \c 0 in
  /// the mask remain unchanged.
  ///
  /// The default value for \c mask includes all bits.
  ///
  /// \param flags
  ///   the flags bit pattern to set
  /// \param mask
  ///   the mask defining which bits are affected
  void setAll(T flags, T mask = ((T)-1));

  /// \brief
  ///   Sets the specified flags on all elements that satisfy the specified check.
  ///
  /// This method uses two parameters to specify what actually gets set. While the
  /// \c flags parameter contains the bit pattern to set, the \c mask parameter 
  /// determines which bits actually get changed. For each bit that is \c 1 in the
  /// \c mask, the corresponding bit of \c flags gets set. All bits that are \c 0 in
  /// the mask remain unchanged.
  ///
  /// The default value for \c mask includes all bits.
  ///
  /// Checking works in a similar way using \c checkFlags and \c checkMask. Only the 
  /// flags included in \c checkMask are checked, and the masked bits in \c checkFlags
  /// must match the masked bits of the element exactly for it to be changed.
  ///
  /// \param checkFlags
  ///   the flags bit pattern to check for
  /// \param flags
  ///   the flags bit pattern to set
  /// \param checkMask
  ///   the mask defining which bits are affected by the check
  /// \param mask
  ///   the mask defining which bits are affected when setting the bit pattern
  /// \return
  ///   the number of entries that have been changed
  size_t setFlagsConditional(T checkFlags, T flags, T checkMask = ((T)-1), T mask = ((T)-1));

  /// \brief
  ///   Sets the specified flags of one element in this array.
  ///
  /// This method uses two parameters to specify what actually gets set. While the
  /// \c flags parameter contains the bit pattern to set, the \c mask parameter 
  /// determines which bits actually get changed. For each bit that is \c 1 in the
  /// \c mask, the corresponding bit of \c flags gets set. All bits that are \c 0 in
  /// the mask remain unchanged.
  ///
  /// The default value for \c mask includes all bits.
  ///
  /// \param elementIndex
  ///   index of the element to change
  /// \param flags
  ///   the flags bit pattern to set
  /// \param mask
  ///   the mask defining which bits are affected
  /// \return
  ///   \c true if any bit was changed; \c false otherwise
  bool setFlags(size_t elementIndex, T flags, T mask = ((T)-1));

  /// \brief
  ///   Returns the flags of the specified element.
  /// \param elementIndex
  ///   index of the element whose flags to return
  /// \return
  ///   the flags of the specified element
  T getFlags(size_t elementIndex) const;

  /// \brief
  ///   Returns the number of elements that have at least one of the specified flags
  ///   set.
  ///
  /// While it is possible to pass a bit pattern with multiple bits set, this only makes
  /// sense (i.e., returns meaningful values) when these flags are mutually exclusive. This
  /// is due to the fact that this method counts flag occurrences, not unique elements.
  ///
  /// \param flags
  ///   bit pattern of flags to query
  /// \return
  ///   the sum of occurrences of each of the specified flags
  size_t getNumFlaggedElements(T flags) const;

  /// \brief
  ///   Swaps two array elements.
  /// \param elementIndex1
  ///   index of the first element
  /// \param elementIndex2
  ///   index of the second element
  void swapElements(size_t elementIndex1, size_t elementIndex2);

  /// \brief
  ///   Returns the size of this array.
  /// \return
  ///   the size of this array
  size_t getSize() const;

  /// \brief
  ///   Resizes this array.
  ///
  /// If the new size is larger than the current size, new elements are 
  /// initialized to \c 0 (no flags set).
  ///
  /// \param newSize
  ///   the new size of the array
  void setSize(size_t newSize);

private:
  bool setSingleFlag(size_t elementIndex, hkUint8 flagIndex, bool flagValue);

private:
  std::vector<T> m_data;
  size_t m_counters[NumFlags];
};


template <typename T>
hkvFlagFieldArray<T>::hkvFlagFieldArray()
{
  clear();
}


template <typename T>
void hkvFlagFieldArray<T>::clear()
{
  m_data.resize(0);
  memset(m_counters, 0, sizeof(size_t) * NumFlags);
}


template <typename T>
void hkvFlagFieldArray<T>::setAll(T flags, T mask = ((T)-1))
{
  T invMask = ~mask;
  T maskedFlags = flags & mask;

  const size_t numElements = m_data.size();
  for (size_t elementIdx = 0; elementIdx < numElements; ++elementIdx)
  {
    m_data[elementIdx] = (m_data[elementIdx] & invMask) | maskedFlags;
  }

  for (hkUint8 flagIdx = 0; flagIdx < NumFlags; ++flagIdx)
  {
    // If the bit is affected by the mask, set the count depending on whether
    // the bit is set or not.
    T flagMask = (T)1 << flagIdx;
    if ((mask & flagMask) != 0)
    {
      m_counters[flagIdx] = ((flags & flagMask) != 0) ? m_data.size() : 0;
    }
  }
}


template <typename T>
size_t hkvFlagFieldArray<T>::setFlagsConditional(T checkFlags, T flags, T checkMask = ((T)-1), T mask = ((T)-1))
{
  T maskedCheckFlags = checkFlags & checkMask;
  size_t numChanged = 0;

  const size_t numElements = m_data.size();
  for (size_t elementIdx = 0; elementIdx < numElements; ++elementIdx)
  {
    if ((m_data[elementIdx] & checkMask) == maskedCheckFlags)
    {
      if (setFlags(elementIdx, flags, mask))
      {
        numChanged++;
      }
    }
  }

  return numChanged;
}


template <typename T>
bool hkvFlagFieldArray<T>::setFlags(size_t elementIndex, T flags, T mask = ((T)-1))
{
  bool changed = false;
  for (hkUint8 flagIdx = 0; flagIdx < NumFlags; ++flagIdx)
  {
    T flagMask = (T)1 << flagIdx;
    if ((mask & flagMask) != 0)
    {
      if (setSingleFlag(elementIndex, flagIdx, ((flags & flagMask) != 0)))
      {
        changed = true;
      }
    }
  }
  return changed;
}


template <typename T>
T hkvFlagFieldArray<T>::getFlags(size_t elementIndex) const
{
  return m_data[elementIndex];
}


template <typename T>
size_t hkvFlagFieldArray<T>::getNumFlaggedElements(T flags) const
{
  size_t result = 0;

  for (hkUint8 flagIndex = 0; flagIndex < NumFlags; ++flagIndex)
  {
    T flagMask = (T)1 << flagIndex;
    if ((flags & flagMask) != 0)
    {
      result += m_counters[flagIndex];
    }
  }

  return result;
}


template <typename T>
void hkvFlagFieldArray<T>::swapElements(size_t elementIndex1, size_t elementIndex2)
{
  T temp = m_data[elementIndex1];
  m_data[elementIndex1] = m_data[elementIndex2];
  m_data[elementIndex2] = temp;
}


template <typename T>
size_t hkvFlagFieldArray<T>::getSize() const
{
  return m_data.size();
}


template <typename T>
void hkvFlagFieldArray<T>::setSize(size_t newSize)
{
  const size_t currentSize = m_data.size();
  if (newSize > currentSize)
  {
    m_data.resize(newSize, 0);
  }
  else if (newSize < currentSize)
  {
    for (size_t i = newSize; i < currentSize; ++i)
    {
      setFlags(i, 0);
    }
    m_data.resize(newSize);
  }
}


template <typename T>
bool hkvFlagFieldArray<T>::setSingleFlag(size_t elementIndex, hkUint8 flagIndex, bool flagValue)
{
  T flagMask = (T)1 << flagIndex;

  // Early out if no change
  bool changing = flagValue ^ ((m_data[elementIndex] & flagMask) != 0);
  if (!changing)
  {
    return false;
  }

  // Adjust the counter
  m_counters[flagIndex] += flagValue ? 1 : -1;

  // Adjust the flag
  if (flagValue)
  {
    m_data[elementIndex] |= flagMask;
  }
  else
  {
    m_data[elementIndex] &= ~flagMask;
  }

  return true;
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
