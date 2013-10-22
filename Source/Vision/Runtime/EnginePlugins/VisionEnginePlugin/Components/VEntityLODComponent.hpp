/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

/// \file VFollowPathComponent.hpp

#ifndef VENTITYLODCOMPONENT_HPP_INCLUDED
#define VENTITYLODCOMPONENT_HPP_INCLUDED

#include <Vision/Runtime/EnginePlugins/VisionEnginePlugin/Rendering/Effects/EffectsModule.hpp>
#include <Vision/Runtime/Engine/Visibility/VisApiVisibilityLODHysteresis.hpp>

// Versions
#define ENTITYLODCOMPONENT_VERSION_0          0     // Initial version
#define ENTITYLODCOMPONENT_VERSION_1          1     // LOD count incremented by one in order to make code easier to read.
#define ENTITYLODCOMPONENT_VERSION_CURRENT    1     // Current version

///
/// \brief
///   Enum that represents commonly used LOD level indices
enum VEntityLODLevel_e
{
  VLOD_HIGH      = 0,     ///< Highest LOD level, mesh defined in the owner entity
  VLOD_MEDIUM    = 1,     ///< Medium LOD level
  VLOD_LOW       = 2,     ///< Low LOD level
  VLOD_ULTRALOW  = 3,     ///< Ultra low LOD level

  VLOD_NONE      = 4,     ///< No LOD, LOD is disabled, use entity mesh (if available)
  VLOD_AUTO      = 5      ///< Switches LOD by distance
};

///
/// \brief
///   Class that represents information about one LOD level. The VEntityLODComponent has an array of it.
///
class VEntityLODLevelInfo
{
public:
  /// \brief
  ///   Constructor
  VEntityLODLevelInfo()
  {
    m_fMinSwitchDistance = 0.f;
    m_fMaxSwitchDistance = 0.f;
    m_pEntity = NULL;
  }

  /// \brief
  ///   Associates a model file with this Level
  bool SetModelFile(const char *szFilename, bool bAppyMotionDelta = false);

  /// \brief
  ///   Sets a distance at which the level switches to next level. So the LOD level will be visible
  ///   in the range [m_fMinSwitchLevel .. m_fMaxSwitchLevel]
  inline void SetSwitchDistance(float fMinDist, float fMaxDist)
  {
    m_fMinSwitchDistance = fMinDist;
    m_fMaxSwitchDistance = fMaxDist;
  }

public:
  // ----- Functions required for LOD fading feature of simulation package (not used in base SDK)

  /// \brief
  ///   Returns the previously set bounding box.
  inline const hkvAlignedBBox &GetBoundingBox() const 
  {
    VASSERT(m_pEntity != NULL)
    return m_pEntity->GetBoundingBox(); 
  }


public:
  VDynamicMeshPtr m_spMesh;                                   ///< Mesh of this LOD level
  VisAnimConfigPtr m_spAnimConfig;                            ///< Animation Config of this LOD level  
  VisAnimFinalSkeletalResultPtr m_spFinalSkeletalResult;      ///< Final Animation Result of this LOD level
  float m_fMinSwitchDistance;                                 ///< Min switching distance of this LOD level
  float m_fMaxSwitchDistance;                                 ///< Max switching distance of this LOD level
  VisBaseEntity_cl* m_pEntity;                                ///< Entity instance that is owner of the VEntityLODComponent
};


/// \brief
///   Data object that is sent by the static VEntityLODLevelInfo::OnEntityLODDestroyed callback
class VEntityLODDataObject : public IVisCallbackDataObject_cl
{
public:
  inline VEntityLODDataObject(VCallback* pSender, VEntityLODLevelInfo* pObj) : IVisCallbackDataObject_cl(pSender), m_pEntityLODObject(pObj) {}
  VEntityLODLevelInfo* m_pEntityLODObject;
};

class VEntityLODComponentManager;

/// 
/// \brief
///   Entity LOD component that can be added to entities so they can have multiple LOD levels each represented by a different model.
/// 
#ifdef SUPPORTS_LOD_HYSTERESIS_THRESHOLDING
  class VEntityLODComponent : public IVLODHysteresisComponent
#else
  class VEntityLODComponent : public IVObjectComponent
#endif //SUPPORTS_LOD_HYSTERESIS_THRESHOLDING
{
  friend class VEntityLODComponentManager;

public: 

  ///
  /// @name Constructor
  /// @{
  ///

  ///
  /// \brief
  ///   Component Constructor
  /// 
  /// \param iComponentFlags
  ///   See IVObjectComponent
  /// 
  /// \see
  ///   IVObjectComponent
  /// 
  EFFECTS_IMPEXP VEntityLODComponent(int iComponentFlags=VIS_OBJECTCOMPONENTFLAG_NONE);

  /// 
  /// \brief
  ///   Component Destructor
  /// 
  EFFECTS_IMPEXP virtual ~VEntityLODComponent();

  ///
  /// @}
  ///

  ///
  /// @name Component methods
  /// @{
  ///

  ///
  /// \brief
  ///   Initializes the entity LOD component by setting the initial values
  /// 
  EFFECTS_IMPEXP void CommonInit();

  ///
  /// \brief
  ///   Try to find an anim config that might have been set from outside (animation component, transition state machine)
  ///
  EFFECTS_IMPEXP bool ConnectToExistingAnimConfig();

  ///
  /// \brief
  ///   Accesses the LOD level info of specified index, where the index must be in the allocated range.
  ///
  inline VEntityLODLevelInfo &GetLevelInfo(int iIndex)
  {
    VASSERT(iIndex >= 0 && iIndex <= LOD_LevelCount);
    return m_pLevels[iIndex];
  }

  ///
  /// \brief
  ///   Forwards the new frozen state to all animation configs involved
  ///
  inline void SetFrozen(bool bNewState)
  {
    for (int i=0; i < LOD_LevelCount; i++)
      m_pLevels[i].m_spAnimConfig->SetFrozen(bNewState);
  }

  ///
  /// \brief
  ///   Sets a LOD index. Can be LOD_AUTO to detect according to distance in the Update() function
  ///
  EFFECTS_IMPEXP void SetLODLevel(VEntityLODLevel_e newLevel);

  ///
  /// \brief
  ///   Returns the current LOD level, which is always >= 0
  ///
  inline int GetCurrentLODLevel() const 
  { 
    return m_iCurrentLevel; 
  }

  ///
  /// \brief
  ///   Sets the LOD Level Count.
  ///
  inline void SetLODLevelCount(const int iCount)
  {
    LOD_LevelCount = iCount;
    CommonInit(); // reinitializes component if owner is set.
  }

  ///
  /// \brief
  ///   Returns LOD Level Count.
  ///
  inline int GetLODLevelCount() const
  {
    return LOD_LevelCount;
  }

  ///
  /// \brief
  ///   Sets Medium Level Mesh Filename.
  ///
  inline void SetMediumLevelMesh(const char* szFilename)
  {
    Level_Medium_Mesh = szFilename;
    CommonInit(); // reinitializes component if owner is set.
  }

  ///
  /// \brief
  ///   Returns Medium Level Mesh Filename.
  ///
  inline const char* GetMediumLevelMesh() const
  {
    return Level_Medium_Mesh.AsChar();
  }

  ///
  /// \brief
  ///   Sets Low Level Mesh Filename.
  ///
  inline void SetLowLevelMesh(const char* szFilename)
  {
    Level_Low_Mesh = szFilename;
    CommonInit(); // reinitializes component if owner is set.
  }

  ///
  /// \brief
  ///   Returns Low Level Mesh Filename.
  ///
  inline const char* GetLowLevelMesh() const
  {
    return Level_Low_Mesh.AsChar();
  }

  ///
  /// \brief
  ///   Sets UltraLow Level Mesh Filename.
  ///
  inline void SetUltraLowLevelMesh(const char* szFilename)
  {
    Level_UltraLow_Mesh = szFilename;
    CommonInit(); // reinitializes component if owner is set.
  }

  ///
  /// \brief
  ///   Returns UltraLow Level Mesh Filename.
  ///
  inline const char* GetUltraLowLevelMesh() const
  {
    return Level_UltraLow_Mesh.AsChar();
  }

  ///
  /// \brief
  ///   Sets the Medium Level distance.
  ///
  inline void SetMediumLevelDistance(float fDistance)
  {
    Level_Medium_Distance = fDistance;
    CommonInit(); // reinitializes component if owner is set.
  }

  ///
  /// \brief
  ///  Returns the Medium Level distance.
  ///
  inline float GetMediumLevelDistance() const
  {
    return Level_Medium_Distance;
  }

  ///
  /// \brief
  ///   Sets the Low Level distance.
  ///
  inline void SetLowLevelDistance(float fDistance)
  {
    Level_Low_Distance = fDistance;
    CommonInit(); // reinitializes component if owner is set.
  }

  ///
  /// \brief
  ///  Returns the Low Level distance.
  ///
  inline float GetLowLevelDistance() const
  {
    return Level_Low_Distance;
  }
  ///
  /// \brief
  ///   Sets the UltraLow Level distance.
  ///
  inline void SetUltraLowLevelDistance(float fDistance)
  {
    Level_UltraLow_Distance = fDistance;
    CommonInit(); // reinitializes component if owner is set.
  }

  ///
  /// \brief
  ///  Returns the UltraLow Level distance.
  ///
  inline float GetUltraLowLevelDistance() const
  {
    return Level_UltraLow_Distance;
  }

  ///
  /// \brief
  ///   Evaluates the current distance of the owner's entity to the camera
  ///
  inline float GetDistanceToCamera() const 
  {
    VASSERT(GetOwner() != NULL);
    return (static_cast<VisBaseEntity_cl*>(GetOwner())->GetPosition() - 
      Vision::Camera.GetCurrentCameraPosition()).getLength(); 
  }

  ///
  /// @}
  ///

#ifdef SUPPORTS_LOD_HYSTERESIS_THRESHOLDING
  ///
  /// @name IVLODHysteresisLevelProvider Virtual Overrides
  /// @{
  ///
  EFFECTS_IMPEXP virtual int GetLODLevel() const HKV_OVERRIDE;

  ///
  /// @}
  ///
#endif //SUPPORTS_LOD_HYSTERESIS_THRESHOLDING

  ///
  /// @name VTypedObject Virtual Overrides
  /// @{
  ///

#if defined(WIN32) || defined(_VISION_DOC)

  EFFECTS_IMPEXP virtual void GetVariableAttributes(VisVariable_cl *pVariable, VVariableAttributeInfo &destInfo) HKV_OVERRIDE;

#endif

  EFFECTS_IMPEXP virtual void MessageFunction(int iID, INT_PTR iParamA, INT_PTR iParamB) HKV_OVERRIDE;

  ///
  /// @}
  ///


  ///
  /// @name IVObjectComponent Overrides
  /// @{
  ///

  EFFECTS_IMPEXP virtual void SetOwner(VisTypedEngineObject_cl *pOwner) HKV_OVERRIDE;

  EFFECTS_IMPEXP virtual BOOL CanAttachToObject(VisTypedEngineObject_cl *pObject, VString &sErrorMsgOut) HKV_OVERRIDE;

  EFFECTS_IMPEXP virtual void OnVariableValueChanged(VisVariable_cl *pVar, const char * value) HKV_OVERRIDE;

  ///
  /// @}
  ///

  ///
  /// @name Serialization
  /// @{
  ///

  V_DECLARE_SERIAL_DLLEXP(VEntityLODComponent, EFFECTS_IMPEXP)

  V_DECLARE_VARTABLE(VEntityLODComponent, EFFECTS_IMPEXP)

  EFFECTS_IMPEXP virtual void Serialize(VArchive &ar) HKV_OVERRIDE;

  ///
  /// @}
  ///

protected:
  ///
  /// \brief
  ///   Updates the LOD level of the owner object
  /// 
  /// The PerFrameUpdate() function takes care of evaluating the current distance of the object 
  /// to the camera and updates the LOD level by assigning the respective model.
  ///
  /// \note
  ///   You do not have to call this function manually, since the VEntityLODComponentManager
  ///   class will take care of this.
  ///
  EFFECTS_IMPEXP void PerFrameUpdate();

  ///
  /// \brief
  ///   Updates the LOD level depending on the camera distance
  ///
  EFFECTS_IMPEXP void UpdateLOD();

  ///
  /// \brief
  ///   Updates the passed LOD level
  ///
  EFFECTS_IMPEXP void ApplyLOD(int newLevel);


private:  
  // Private functions
  void InitializeLODLevelInfo(int iLevel, const char* szMeshFilename, float fMinDistance, float fMaxDistance);

  // Set the result generator as the animation root node on the final result of each LOD.
  void SetSkeletalAnimRootNode(IVisAnimResultGenerator_cl *pRoot, bool bAppyMotionDelta);

  // Exposed to vForge:
  int LOD_LevelMode;
  int LOD_LevelCount;
  VString Level_Medium_Mesh;    
  VString Level_Low_Mesh;        
  VString Level_UltraLow_Mesh;
  float Level_Medium_Distance;  
  float Level_Low_Distance;    
  float Level_UltraLow_Distance;

  // Not exposed:
  VString Level_High_Mesh;              ///< High level mesh defined by owner entity mesh
  int m_iCurrentLevel;                  ///< Current LOD level
  VEntityLODLevelInfo *m_pLevels;       ///< Information about LOD levels
};


///
/// \brief
///   Collection for handling entity LOD components
///
class VEntityLODComponentCollection : public VRefCountedCollection<VEntityLODComponent>
{
};


///
/// \brief
///   Manager for all VPlayableCharacterComponent instance
///
/// This manager class has a list of all available VEntityLODComponent instances
/// and takes care of calling their VEntityLODComponent::PerFrameUpdate function
/// each frame.
///
class VEntityLODComponentManager : public IVisCallbackHandler_cl
{
public:
  ///
  /// \brief
  ///   Gets the singleton of the manager
  ///
  static VEntityLODComponentManager &GlobalManager()
  {
    return g_GlobalManager;
  }

  ///
  /// \brief
  ///   Should be called at plugin initialization time
  ///
  void OneTimeInit()
  {
    // Register
    Vision::Callbacks.OnUpdateSceneFinished += this; 
    Vision::Callbacks.OnAfterSceneLoaded += this;
  }

  ///
  /// \brief
  ///   Should be called at plugin de-initialization time
  ///
  void OneTimeDeInit()
  {
    // De-register
    Vision::Callbacks.OnUpdateSceneFinished -= this;
    Vision::Callbacks.OnAfterSceneLoaded -= this;
  }

  ///
  /// \brief
  ///   Gets all VEntityLODComponent instances this manager holds
  /// 
  inline VEntityLODComponentCollection &Instances() { return m_Components; }

  /// 
  /// \brief
  ///   Callback method that takes care of updating the managed instances each frame
  ///  
  virtual void OnHandleCallback(IVisCallbackDataObject_cl *pData);

protected:

  /// Holds the collection of all instances of VEntityLODComponent
  VEntityLODComponentCollection m_Components;

  /// One global instance of our manager
  static VEntityLODComponentManager g_GlobalManager;

};

#endif // VENTITYLODCOMPONENT_HPP_INCLUDED

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
