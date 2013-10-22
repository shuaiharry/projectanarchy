/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

#include <Vision/Runtime/EnginePlugins/VisionEnginePlugin/VisionEnginePluginPCH.h>
#include <Vision/Runtime/EnginePlugins/VisionEnginePlugin/Rendering/PathRendering/VPathRenderingData.hpp>
#include <Vision/Runtime/EnginePlugins/VisionEnginePlugin/Rendering/PathRendering/VPathRendererBase.hpp>

/////////////////////////////////////////////////////////////////////
// VPathRenderingData
/////////////////////////////////////////////////////////////////////
V_IMPLEMENT_SERIAL(VPathRenderingData, IVPathRenderingData, 0, &g_VisionEngineModule);

VPathRenderingData::VPathRenderingData() :
  m_fDiameter(1.0f),
  m_fLinkLength(5.0f),
  m_fLinkGap(0.0f),
  m_iNumLinks(0),
  m_fLastLinkExtra(0.0f),
  m_iLastPathUpdateFrame(0)
{
  m_LinkTransforms.SetDefaultValue(NULL);
}

VPathRenderingData::~VPathRenderingData()
{
	m_LinkTransforms.Reset();
}

bool VPathRenderingData::SetPathObject(VTypedObject* pObject)
{
  if (!pObject->IsOfType(V_RUNTIME_CLASS(VisPath_cl)))
    return false;

  m_spPathObject = (VisPath_cl*)pObject;
  return true;
}

float VPathRenderingData::GetDiameter() const
{
  return m_fDiameter;
}

float VPathRenderingData::GetLinkLength() const
{
  return m_fLinkLength;
}

float VPathRenderingData::GetLinkGap() const
{
  return m_fLinkGap;
}

unsigned int VPathRenderingData::GetNumLinks() const
{
  return m_iNumLinks;
}

float VPathRenderingData::GetLastLinkExtra() const
{
  return m_fLastLinkExtra;
}

bool VPathRenderingData::GetLinkTransform(unsigned int iIndex, hkvMat3& mRotation, hkvVec3& vTranslation) const
{
  if (iIndex >= GetNumLinks())
    return false;

  mRotation = m_LinkTransforms.Get(iIndex)->m_mRotation;
  vTranslation = m_LinkTransforms.Get(iIndex)->m_vPosition;

  return true;
}

void VPathRenderingData::GetLinkTransforms(hkvMat3* pRotations, hkvVec3* pTranslations, bool bRemoveRoll, bool bReverseOrder) const
{
  // Helper to detect link direction reversal when removing roll
  bool bDirection = false;

  hkvMat3 mReverseRot(hkvNoInitialization);
  if (bReverseOrder)
    mReverseRot.setFromEulerAngles(0, 0, 180);
  else
    mReverseRot.setIdentity();

  for (unsigned int i = 0; i < GetNumLinks(); ++i)
  {
    hkvMat3& mRot = pRotations[i];
    hkvVec3& vTrans = pTranslations[i];
    unsigned int iLink = bReverseOrder ? (GetNumLinks() - i - 1) : i;
    GetLinkTransform(iLink, mRot, vTrans);

    if (bReverseOrder)
      mRot = mRot.multiply(mReverseRot);

    if ((i > 0) && bRemoveRoll)
      RemoveLinkRoll(pRotations[i - 1], mRot, bDirection);
  }
}

bool VPathRenderingData::HasDataChanged()
{
  // check whether path changed since last initialization
  return m_iLastPathUpdateFrame != m_spPathObject->GetLastChangedFrame();
}

void VPathRenderingData::RemoveLinkRoll(hkvMat3 const& mPrevRot, hkvMat3& mRot, bool& bDirection) const
{
  hkvMat3 mPrevRotInv(mPrevRot);
  VVERIFY(mPrevRotInv.invert() == HKV_SUCCESS);

  // Calculate the rotation of the current link in the space of the
  // previous link
  hkvMat3 mThisInPrev = mPrevRotInv.multiply(mRot);

  // Check whether we are changing direction (that is, we have a sharp angle 
  // between two links). In this case, we need to change how we adjust the roll.
  hkvVec3 v2(mThisInPrev * hkvVec3(1, 0, 0));
  bool bChangingDirection = bDirection ? v2[0] > 0 : v2[0] < 0;
  if (bChangingDirection)
    bDirection = !bDirection;

  // Adjust the roll component
  float rgfThisInPrev[3];
  mThisInPrev.getAsEulerAngles(rgfThisInPrev[2], rgfThisInPrev[1], rgfThisInPrev[0]);

  if (bDirection)
    rgfThisInPrev[2] = 180.f;
  else
    rgfThisInPrev[2] = 0.f;

  mRot.setFromEulerAngles(rgfThisInPrev[2], rgfThisInPrev[1], rgfThisInPrev[0]);

  // Transform back to global space
  mRot = mPrevRot.multiply(mRot);
}


void VPathRenderingData::Init(VisTypedEngineObject_cl* pOwner)
{
  VASSERT(pOwner != NULL);
  VASSERT(m_spPathObject != NULL);

  VPathRenderingMetaData* pMetaData = pOwner->Components().GetComponentOfType<VPathRenderingMetaData>();
  if (pMetaData != NULL)
  {
    m_fDiameter = pMetaData->Diameter;
    m_fLinkLength = pMetaData->LinkLength;
    m_fLinkGap = pMetaData->LinkGap;
  }
  else
  {
    // no component with additional properties present, use default values
    m_fDiameter = 1.0f;
    m_fLinkLength = m_spPathObject->GetLen() / hkvMath::Max(1.0f, (m_spPathObject->GetPathNodeCount() - 1) * 20.0f);
    m_fLinkGap = 0.0f;
  }

  m_iNumLinks = 0;
  m_fLastLinkExtra = 0.0f;
  m_LinkTransforms.Reset();

  m_iLastPathUpdateFrame = m_spPathObject->GetLastChangedFrame();

  // Calculate the pivot points
  DynObjArray_cl<hkvVec3> pivots;
  unsigned int iNumPivots = CalcPivotPoints(pivots);
  if (iNumPivots < 2)
    return;

  // Compute the link transformations
  m_iNumLinks = iNumPivots - 1;

  for (unsigned int i = 0; i < m_iNumLinks; ++i)
  {
    VLinkTransform* pLinkTransform = CreateLinkTransform(pivots[i], pivots[i + 1]);
    m_LinkTransforms[i] = pLinkTransform;
  }
}

VPathRenderingData::VLinkTransform* VPathRenderingData::CreateLinkTransform(const hkvVec3& vPivot1, const hkvVec3& vPivot2) const
{
  VLinkTransform* pLinkTransform = new VLinkTransform();

  hkvVec3 vDirection = vPivot2 - vPivot1;
  pLinkTransform->m_mRotation.setLookInDirectionMatrix(vDirection);
  pLinkTransform->m_vPosition = vPivot1 + (vDirection * 0.5f);

  return pLinkTransform;
}

inline float ComputePathSlopeAtTime(VisPath_cl* pPath, float fTime)
{
  const float fSlopeSpan = 0.0001f;
  float fT1 = fTime;
  float fT2 = fTime + fSlopeSpan;
  if (fT2 > 1.0f)
  {
    fT1 += 1.0f - fT2;
    fT2 = 1.0f;
  }
  hkvVec3 vP1, vP2;
  pPath->EvalPoint(fT1, vP1);
  pPath->EvalPoint(fT2, vP2);

  float fPathSlope = (vP1 - vP2).getLength() / fSlopeSpan;

  return hkvMath::Max(fPathSlope, HKVMATH_DEFAULT_EPSILON);
}

unsigned int VPathRenderingData::CalcPivotPoints(DynObjArray_cl<hkvVec3>& pivots)
{
  float fPathLength = m_spPathObject->GetLen();
  if (fPathLength <= 0.0f || m_fLinkLength < HKVMATH_LARGE_EPSILON)
    return 0;

  unsigned int iNumPivots = 0;
  float fCurrentT = 0.0f;
  hkvVec3 vCurrentPivot;

  // We already know the first pivot point: the start of the path
  m_spPathObject->EvalPoint(fCurrentT, vCurrentPivot);
  pivots[iNumPivots++] = vCurrentPivot;

#ifdef HK_DEBUG_SLOW
  int iLoopCounter = 0;
#endif //HK_DEBUG_SLOW

  for (;;)
  {
    // evaluate next point on path that is m_fLinkLength units away
    float fCurrentPathSlope = ComputePathSlopeAtTime(m_spPathObject, fCurrentT);

    hkvVec3 vLastPivot = vCurrentPivot;
    float fDeltaT = m_fLinkLength / fCurrentPathSlope / 10.0f;
    float fLength = 0.0f;

    // approximate a position close before the searched position
#if 1
    for (;;)
    {
      m_spPathObject->EvalPoint(fCurrentT + fDeltaT, vCurrentPivot);
      float fSegmentLength = (vCurrentPivot - vLastPivot).getLength();
      if (fSegmentLength > m_fLinkLength)
        break;

      fCurrentT += fDeltaT;
      fLength = fSegmentLength;

      if (fCurrentT >= 1.0f)
        break;
    }
#else
    for (;;)
    {
      m_spPathObject->EvalPoint(fCurrentT + fDeltaT, vCurrentPivot);
      float fSegmentLength = (vCurrentPivot - vLastPivot).getLength();
      if (fLength + fSegmentLength > m_fLinkLength)
        break;

      fCurrentT += fDeltaT;
      fLength += fSegmentLength;
      vLastPivot = vCurrentPivot;

      if (fCurrentT >= 1.0f)
        break;
    }
#endif

    // now we are very close before the searched position, approximate remaining length
    float fRemainingLength = m_fLinkLength - fLength;
    if (fRemainingLength > 0.0f)
    {
      fCurrentPathSlope = ComputePathSlopeAtTime(m_spPathObject, fCurrentT);
      fCurrentT += fRemainingLength / fCurrentPathSlope;
      m_spPathObject->EvalPoint(fCurrentT, vCurrentPivot);
    }

    if (fCurrentT >= 1.0f)
      break;

    // We can assume we have found the best approximation for the next pivot point. Add it to our list.
    pivots[iNumPivots++] = vCurrentPivot;

#ifdef HK_DEBUG_SLOW
    if (++iLoopCounter >= 65536)
    {
      // this really should never happen
      Vision::Error.Warning("VPathRenderingData::CalcPivotPoints: Endless loop detected.");
      return 0;
    }
#endif //HK_DEBUG_SLOW
  }

  hkvVec3 vEndAnchor;
  m_spPathObject->EvalPoint(1.0f, vEndAnchor);
  hkvVec3 vLastPivot = pivots.Get(iNumPivots - 1);
  m_fLastLinkExtra = (vLastPivot - vEndAnchor).getLength();

  return iNumPivots;

/* ORIGINAL CODE FROM HAVOK ENGINE PLUGIN - SEEMS NOT TO WORK IN SOME SITUATIONS
  float fLinkLength = m_fLinkLength;

  // Sampling interval is 10 samples per link (on a linear path)
  float fPathStepT = fLinkLength / fPathLength / 10.0f; 
  // Epsilon is the difference in t at which we stop approximation
  const float fEpsilonT = fPathStepT / 100.f;

  unsigned int iNumPivots = 0;

  // We already know the first pivot point: the start of the path
  hkvVec3 vCurrentPivot;
  m_spPathObject->EvalPoint(0.0f, vCurrentPivot);
  pivots[iNumPivots++] = vCurrentPivot;

  // We start at t=0
  float fCurrentT = 0.0f;
  float fPrevT = 0.0f;
  float fPrevPivotT = 0.0f;

  while (true)
  {
    hkvVec3 vPrevPivot = vCurrentPivot;
    fPrevPivotT = fCurrentT;

    // Coarse approximation: search until the distance becomes larger 
    // than the desired link length
    bool bDone = false;
    float fDistance = 0.0f;
    while (fDistance < fLinkLength)
    {
      // If we start an iteration and we are already at the end of the
      // path, break immediately: we're done.
      if (fCurrentT == 1.0f)
      {
        bDone = true;
        break;
      }

      // Go further along the path
      fPrevT = fCurrentT;
      fCurrentT += fPathStepT;
      if (fCurrentT > 1.0f)
        fCurrentT = 1.0f;

      // Evaluate the point at which we are
      m_spPathObject->EvalPointSmooth(fCurrentT, vCurrentPivot);
      fDistance = (vCurrentPivot - vPrevPivot).getLength();
    }

    // If the coarse approximation says we can't get another link in there,
    // believe it and stop here.
    if (bDone)
    {
      vCurrentPivot = vPrevPivot;
      break;
    }

    // Fine approximation: get closer in binary steps, until delta t
    // becomes smaller than epsilon t. Start in the middle between the
    // previous and the current t value.
    while (true)
    {
      float fDeltaT = (fCurrentT - fPrevT) / 2.0f;
      VASSERT(fDeltaT >= 0.0f);
      if (fDeltaT < fEpsilonT)
        break;

      // Evaluate where we are. If the distance at our test point is
      // smaller than the link length, continue at the half with the
      // bigger t values; otherweise, continue with smaller t values,
      // for which prev and current t are already set correctly.
      fCurrentT = fPrevT + fDeltaT;
      m_spPathObject->EvalPointSmooth(fCurrentT, vCurrentPivot);
      fDistance = (vCurrentPivot - vPrevPivot).getLength();
      if (fDistance < fLinkLength)
      {
        fPrevT = fCurrentT;
        fCurrentT += fDeltaT;
        VASSERT(fCurrentT <= 1.0f);
      }
    }

    // We can assume we have found the best approximation for the next
    // pivot point. Add it to our list.
    pivots[iNumPivots++] = vCurrentPivot;

    // Calculate the approximation error of this link by comparing the length
    // of the chain link against the length of the actual path in between
    float fPivotDistance = (vCurrentPivot - vPrevPivot).getLength();
    float fPathDistance = 0;
    {
      const unsigned int iNumSteps = 20;
      float fEvalDeltaT = (fCurrentT - fPrevPivotT) / (float)iNumSteps;
      for (unsigned int i = 0; i < iNumSteps; ++i)
      {
        hkvVec3 v1, v2;
        m_spPathObject->EvalPointSmooth(fPrevPivotT + (fEvalDeltaT * (float)i), v1);
        m_spPathObject->EvalPointSmooth(fPrevPivotT + (fEvalDeltaT * (float)(i+1)), v2);
        fPathDistance += (v2 - v1).getLength();
      }
    }

    m_fApproximationError += hkvMath::Abs(fPivotDistance - fPathDistance) / fLinkLength;
  }

  // Calculate the average approximation error.
  m_fApproximationError /= (float)(iNumPivots - 1);

  // Calculate the last link mismatch
  {
    hkvVec3 vEndAnchor;
    m_spPathObject->EvalPoint(1.0f, vEndAnchor);

    hkvVec3 vLastPivot = pivots.Get(iNumPivots - 1);
    m_fLastLinkExtra = (vLastPivot - vEndAnchor).getLength();
  }

  return iNumPivots;
*/
}


/////////////////////////////////////////////////////////////////////
// VPathRenderingMetaData
/////////////////////////////////////////////////////////////////////
V_IMPLEMENT_SERIAL(VPathRenderingMetaData, IVObjectComponent, 0, &g_VisionEngineModule);

#define PATHRENDERINGMETADATA_SERIALIZATION_VERSION_0        0   // initial version
#define PATHRENDERINGMETADATA_SERIALIZATION_VERSION_CURRENT  PATHRENDERINGMETADATA_SERIALIZATION_VERSION_0


VPathRenderingMetaData::VPathRenderingMetaData(int iComponentFlags) :
  IVObjectComponent(iComponentFlags),
  Diameter(1.0f),
  LinkLength(10.0f),
  LinkGap(0.0f)
{
}

VPathRenderingMetaData::~VPathRenderingMetaData()
{
}

BOOL VPathRenderingMetaData::CanAttachToObject(VisTypedEngineObject_cl* pObject, VString& sErrorMsgOut)
{
  if (!IVObjectComponent::CanAttachToObject(pObject, sErrorMsgOut))
    return FALSE;

  if (!pObject->IsOfType(V_RUNTIME_CLASS(VisPath_cl)))
    return FALSE;

  return TRUE;
}

void VPathRenderingMetaData::OnVariableValueChanged(VisVariable_cl* pVar, const char* value)
{
  IVObjectComponent::OnVariableValueChanged(pVar, value);

  // path renderers need to be re-initialized when variables are changed
  if (GetOwner() != NULL)
  {
    for (int i = 0; i < GetOwner()->Components().Count(); i++)
    {
      IVObjectComponent* pComp = GetOwner()->Components().GetAt(i);
      if (pComp != NULL && pComp->IsOfType(V_RUNTIME_CLASS(VPathRendererBase)))
        ((VPathRendererBase*)pComp)->OnDataChanged();
    }
  }
}

void VPathRenderingMetaData::Serialize(VArchive& ar)
{
  IVObjectComponent::Serialize(ar);

  if (ar.IsLoading())
  {
    unsigned int iVersion = 0;
    ar >> iVersion;
    VASSERT_MSG(iVersion <= PATHRENDERINGMETADATA_SERIALIZATION_VERSION_CURRENT, "Invalid version of serialized data!");

    ar >> Diameter;
    ar >> LinkLength;
    ar >> LinkGap;
  }
  else
  {
    ar << (unsigned int)PATHRENDERINGMETADATA_SERIALIZATION_VERSION_CURRENT;
    ar << Diameter;
    ar << LinkLength;
    ar << LinkGap;
  }
}

// ----------------------------------------------------------------------------
START_VAR_TABLE(VPathRenderingMetaData, IVObjectComponent, "Meta data component that can be attached to VisPath_cl instances to provide additional properties for path rendering like diameter, link length and link gap", VVARIABLELIST_FLAGS_NONE, "Path Rendering Meta Data")
  DEFINE_VAR_FLOAT(VPathRenderingMetaData, Diameter, "Diameter of the chain/cable", "1.0", 0, NULL);
  DEFINE_VAR_FLOAT(VPathRenderingMetaData, LinkLength, "Length of a single chain link", "5.0", 0, NULL);
  DEFINE_VAR_FLOAT(VPathRenderingMetaData, LinkGap, "Length of the gap between the links of the chain/cable", "0.0", 0, NULL);
END_VAR_TABLE

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
