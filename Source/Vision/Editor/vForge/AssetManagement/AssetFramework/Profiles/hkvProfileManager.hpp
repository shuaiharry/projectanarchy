/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

/// \file hkvProfileManager.hpp

#ifndef VPROFILEMANAGER_HPP_INCLUDED
#define VPROFILEMANAGER_HPP_INCLUDED

#include <Vision/Editor/vForge/AssetManagement/AssetFramework/Base/hkvCallback.hpp>
#include <Vision/Editor/vForge/AssetManagement/AssetFramework/Profiles/hkvProfile.hpp>
#include <Vision/Editor/vForge/AssetManagement/AssetFramework/Profiles/hkvTransformationRuleTemplate.hpp>
#include <vector>
#include <Vision/Runtime/Base/System/IO/System/FileLock.hpp>

class hkvProfileManager;


/// \brief
///   Data object for the OnProfileAdded callback in the hkvProfileManager.
class hkvProfileAddedData : public hkvCallbackData
{
public:
  ASSETFRAMEWORK_IMPEXP hkvProfileAddedData(const hkvCallback* sender) : hkvCallbackData(sender)
  {
    m_profileIndex = HKV_INVALID_INDEX;
  }

  hkUint32 m_profileIndex;
  hkvTargetPlatform m_platform;
  hkStringPtr m_profileName;
  hkStringPtr m_copySettingsFromProfileName;
};


/// \brief
///   Data object for the OnProfileDeleted callback in the hkvProfileManager.
class hkvProfileDeletedData : public hkvCallbackData
{
public:
  ASSETFRAMEWORK_IMPEXP hkvProfileDeletedData(const hkvCallback* sender) : hkvCallbackData(sender)
  {
    m_profileIndex = HKV_INVALID_INDEX;
  }

  hkUint32 m_profileIndex;
  hkvTargetPlatform m_platform;
  hkStringPtr m_profileName;
};


/// \brief
///   Data object for the OnActiveProfileChanged callback in the hkvProfileManager.
class hkvActiveProfileChangedData : public hkvCallbackData
{
public:
  ASSETFRAMEWORK_IMPEXP hkvActiveProfileChangedData(const hkvCallback* sender) : hkvCallbackData(sender)
  {
    m_profileIndex = HKV_INVALID_INDEX;
  }

  hkUint32 m_profileIndex;
  hkStringPtr m_profileName;
  hkBool m_assetPreviewMode;

  hkBool m_profileChanged;
  hkBool m_assetPreviewModeChanged;
};


/// \brief
///   This manager holds all profiles and templates loaded from the profile manifest file.
class hkvProfileManager : public hkvPropertyReader
{
public: // static functions
  /// \brief
  ///   Static init function of the profile manager which creates the global instance.
  ASSETFRAMEWORK_IMPEXP static void initProfileManager();

  /// \brief
  ///   Static de-init function of the profile manager which destroys the global instance.
  ASSETFRAMEWORK_IMPEXP static void deInitProfileManager();

  /// \brief
  ///   Returns the global instance of the profile manager.
  ASSETFRAMEWORK_IMPEXP static hkvProfileManager* getGlobalInstance();

  /// \brief
  ///   Returns whether a given platform is supported in this version of Vision. Anarchy builds will only return true for dx9, android and ios.
  ASSETFRAMEWORK_IMPEXP static bool isTargetPlatformSupported(VTargetDevice_e platform);

public: // public functions
  ASSETFRAMEWORK_IMPEXP hkvProfileManager();
  ASSETFRAMEWORK_IMPEXP ~hkvProfileManager();

  /// \brief
  ///   Resets the profile manager to its initial state. Should be called when a new project is loaded / unloaded.
  ASSETFRAMEWORK_IMPEXP void clearProfilesAndTemplates(bool emitCallbacks = true);

  /// \brief
  ///   Loads profile and template information from the given project directory (either Anarchy.assetprj or Project.assetprj).
  ASSETFRAMEWORK_IMPEXP bool loadProfileManifest(const char* projectDir);

  /// \brief
  ///   Saves profile and template information to the same file it was loaded from unless a different path is given.
  ///
  /// \param fileName
  ///   Filename of the manifest. If set to NULL, the file the settings were loaded from is used.
  ///
  /// \param force
  ///   Whether the changes should even be changed if the lock couldn't be acquired. If true, manual merging by the user
  ///   in the RCS will be required.
  ASSETFRAMEWORK_IMPEXP bool saveProfileManifest(const char* fileName = NULL, bool force = false);

  /// \brief
  ///   Returns whether the profile manifest is already loaded.
  ASSETFRAMEWORK_IMPEXP bool isProfileManifestLoaded() const;

  /// \brief
  ///   Returns the absolute path to the manifest file used (either Anarchy.assetprj or Project.assetprj).
  ASSETFRAMEWORK_IMPEXP const char* getProfileManifestFile() const {return m_manifestFile.cString();};

  /// \brief
  ///   Returns whether changes have been made to any template or profile.
  ASSETFRAMEWORK_IMPEXP inline bool isDirty() const {return m_dirty;}

  /// \brief
  ///   Sets whether changes have been made to any template or profile.
  ASSETFRAMEWORK_IMPEXP void setDirty();
  
  /// \brief
  ///   Returns the number of profiles that are present in this project.
  ASSETFRAMEWORK_IMPEXP hkUint32 getNumberOfProfiles() const;

  /// \brief
  ///   Returns the profile by the given index. Invalid indices return NULL.
  ASSETFRAMEWORK_IMPEXP hkvProfile::RefCPtr getProfileByIndex(hkUint32 index) const;

  /// \brief
  ///   Returns the profile by the given name. Invalid names return NULL.
  ASSETFRAMEWORK_IMPEXP hkvProfile::RefCPtr getProfileByName(const char* name) const;

  /// \brief
  ///   Returns an array of all profiles.
  ASSETFRAMEWORK_IMPEXP const std::vector<hkvProfile::RefPtr>& getProfiles() const;

  /// \brief
  ///   Returns the index of the given profile. If the name could not be resolved, HKV_INVALID_INDEX is returned.
  ASSETFRAMEWORK_IMPEXP hkUint32 getProfileIndexByName(const char* name) const;

  /// \brief
  ///   Adds a profile with the given name and platform, copies all settings from another profile if available and returns the new profile's index.
  ASSETFRAMEWORK_IMPEXP hkUint32 addProfile(const char* name, hkvTargetPlatform platform, hkUint32 copySettingsFromProfileIndex = HKV_INVALID_INDEX);

  /// \brief
  ///   Deletes the profile with the given index. The last and active profile can't be deleted in which case false is returned.
  ASSETFRAMEWORK_IMPEXP hkBool deleteProfile(hkUint32 index);

  /// \brief
  ///   Returns the index of the currently active profile.
  ASSETFRAMEWORK_IMPEXP hkUint32 getActiveProfileIndex() const;

  /// \brief
  ///   Returns the currently active profile.
  ASSETFRAMEWORK_IMPEXP hkvProfile::RefCPtr getActiveProfile() const;

  /// \brief
  ///   Sets the currently active profile by index. If the index is invalid, false is returned.
  ASSETFRAMEWORK_IMPEXP hkBool setActiveProfile(hkUint32 index);

  /// \brief
  ///   Returns whether the asset preview mode is enabled.
  ASSETFRAMEWORK_IMPEXP hkBool isAssetPreviewModeEnabled() const;

  /// \brief
  ///   Sets the state of the asset preview mode.
  ASSETFRAMEWORK_IMPEXP void setAssetPreviewMode(bool enableAssetPreviewMode);

  // RCS
  ASSETFRAMEWORK_IMPEXP bool getLockStat(VRCSFileStatus& out_status) const;
  ASSETFRAMEWORK_IMPEXP bool isUpToDate() const;
  ASSETFRAMEWORK_IMPEXP bool getLock();
  ASSETFRAMEWORK_IMPEXP bool releaseLock();
  ASSETFRAMEWORK_IMPEXP bool ownsLock() const;

  ASSETFRAMEWORK_IMPEXP const char* getEditorProfileName() const;

  // transform rule template functions
  ASSETFRAMEWORK_IMPEXP hkUint32 getNumberOfTransformRuleTemplates(const char* assetType) const;
  ASSETFRAMEWORK_IMPEXP hkvTransformRuleTemplate* getTransformRuleTemplateByIndex(const char* assetType, hkUint32 templateIndex);
  ASSETFRAMEWORK_IMPEXP const hkvTransformRuleTemplate* getTransformRuleTemplateByIndex(const char* assetType, hkUint32 templateIndex) const;
  ASSETFRAMEWORK_IMPEXP hkUint32 getTransformRuleTemplateIndexByName(const char* assetType, const char* name) const;
  ASSETFRAMEWORK_IMPEXP hkUint32 addTransformRuleTemplate(const char* assetType, const char* name);
  ASSETFRAMEWORK_IMPEXP hkUint32 duplicateTransformRuleTemplate(const char* assetType, hkUint32 templateIndex, const char* name);
  ASSETFRAMEWORK_IMPEXP hkBool deleteTransformRuleTemplate(const char* assetType, hkUint32 templateIndex);
  ASSETFRAMEWORK_IMPEXP const char* getTransformRuleTemplateMapping(const char* assetType) const;

  /// \brief
  ///   Templates are not reference counted internally so in order to know how many times a template is used you can call this functions
  ///   which iterates over all assets to build up current reference counts.
  ///
  /// \sa getTemplateReferenceCount
  ASSETFRAMEWORK_IMPEXP void updateTemplateReferenceCounts() const;

  /// \brief
  ///   Returns the computed reference count for the given template.
  ///
  /// \sa updateTemplateReferenceCounts
  ASSETFRAMEWORK_IMPEXP hkUint32 getTemplateReferenceCount(const char* assetType, hkUint32 templateIndex) const;

  virtual hkvJsonStreamReadHandler::ParsingResult ParseMapKey(const unsigned char* value, size_t len);

private:
  ASSETFRAMEWORK_IMPEXP void callOnActiveProfileChanged(bool profileChanged, bool assetPreviewModeChanged) const;
  ASSETFRAMEWORK_IMPEXP void callOnProfileAdded(hkUint32 addedProfileIndex, hkUint32 copySettingsFromProfileIndex) const;

public: // Data
  hkvCallback OnActiveProfileChanged;
  hkvCallback OnProfileAdded;
  hkvCallback OnProfileDeleted;
  hkvCallback OnProfilesLoaded;
  hkvCallback OnProfilesUnloaded;
  hkvCallback OnPropertiesChanged;

private:
  void updateTransformRuleTemplateMapping();
  void resetAssetTypeToTemplatesMap();

  void updateLockTimeStamp();
  hkvProfileManager(const hkvProfileManager& profile);
  void operator=(const hkvProfileManager& rhs);

private:
  hkStringPtr m_manifestFile;
  std::vector<hkvProfile::RefPtr> m_profiles;

  class TransformTemplateContainer;
  TransformTemplateContainer* getTransformTemplateContainer(const char* assetType) const;
  TransformTemplateContainer* getTransformTemplateContainer(const char* assetType, bool createNew);

  hkStorageStringMap<TransformTemplateContainer*> m_assetTypeToTemplates;
  typedef hkStorageStringMap<TransformTemplateContainer*>::Iterator TemplateIterator;
  
  hkUint32 m_activeProfile;
  hkBool m_assetPreviewMode;
  hkBool m_ownsLock;
  hkBool m_dirty;

  hkStringPtr m_editorProfileName;

  // Locking
  VFileLock m_Lock;
  VFileTime m_FileTime;


public:
  ASSETFRAMEWORK_IMPEXP static const char* const s_manifestFileName;
  ASSETFRAMEWORK_IMPEXP static const char* const s_customTemplateName;
  ASSETFRAMEWORK_IMPEXP static const char* const s_defaultTemplateName;

private:
  static hkvProfileManager* s_globalInstance;
};

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
