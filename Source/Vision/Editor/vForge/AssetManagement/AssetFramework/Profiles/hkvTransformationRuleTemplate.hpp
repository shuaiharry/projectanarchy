/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

/// \file hkvTransformationRuleTemplate.hpp

#ifndef HKVTRANSFORMATIONRULETEMPLATE_HPP_INCLUDED
#define HKVTRANSFORMATIONRULETEMPLATE_HPP_INCLUDED

#include <Vision/Editor/vForge/AssetManagement/AssetFramework/TransformationRules/hkvTransformationRule.hpp>
#include <Common/Base/Container/StringMap/hkStorageStringMap.h>

class hkvProfileManager;

/// \brief
///   This class stores for each profile a transformation rule for a specific asset type.
///
/// Each template is uniquely identified by the asset type it represents and its name.
/// An asset of matching type can set its transformation template to this instance in order to reuse
/// the transformation rules defined within.
/// There is always a 'Default' template that cannot be deleted. Templates are managed by the hkvProfileManager and can only be created by it.
///
/// \sa hkvProfileManager
class hkvTransformRuleTemplate : public hkvIProperties
{
  friend class hkvProfileManager;
public:
  /// \brief
  ///   Returns the unique name of the template. This name is used to map an asset to a template.
  ASSETFRAMEWORK_IMPEXP const char* getName() const;

  /// \brief
  ///   Returns the unique string identifier of the asset type for which this template provides transformation rules.
  ASSETFRAMEWORK_IMPEXP const char* getAssetType() const;

  /// \brief
  ///   Returns the transformation rule for the given profile name. If the profile does not exist, NULL is returned.
  ASSETFRAMEWORK_IMPEXP hkvTransformationRule* getTransformRule(const char* profileName) const;

public: // hkvIProperties interface
  ASSETFRAMEWORK_IMPEXP virtual const char* getTypeName() const HKV_OVERRIDE;
  ASSETFRAMEWORK_IMPEXP virtual void getProperties(hkvPropertyList& properties, hkvProperty::Purpose purpose) const HKV_OVERRIDE;
  ASSETFRAMEWORK_IMPEXP virtual void setProperty(const hkvProperty& prop, const hkArray<hkStringPtr>& path, hkUint32 stackIndex, hkvProperty::Purpose purpose) HKV_OVERRIDE;

private:
  /// \brief
  ///   Constructor for the template.
  ///
  /// \param assetType
  ///   The unique string identifier of the asset type for which this template should provide transformation rules.
  ///
  /// \param templateName
  ///   The unique string identifier of this template.
  ASSETFRAMEWORK_IMPEXP hkvTransformRuleTemplate(const char* assetType, const char* templateName);

  /// \brief
  ///   Destructor.
  ASSETFRAMEWORK_IMPEXP ~hkvTransformRuleTemplate();

  /// \brief
  ///   Creates a clone of this template with the given name. This function cannot fail.
  ASSETFRAMEWORK_IMPEXP hkvTransformRuleTemplate* clone(const char* cloneName) const;

  /// \brief
  ///   Returns the transformation rule for the given profile name.
  ///
  /// \param profileName
  ///   The unique string identifier of the profile for which the transformation rule is requested.
  ///
  /// \param transformRuleType
  ///   The unique string identifier of the transformation rule type which should be created if no entry exists yet for the given profile.
  ///
  /// \return
  ///   A pointer to the hkvTransformationRule matching the requested profile. If the profile does not exist, NULL is returned.
  ASSETFRAMEWORK_IMPEXP hkvTransformationRule* getTransformRule(const char* profileName, const char* transformRuleType) const;

private:
  hkStringPtr m_templateName;
  hkStringPtr m_assetType;
  mutable hkStorageStringMap<hkvTransformationRule*> m_profileRules; // Maps profiles to hkvTransformationRule
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
