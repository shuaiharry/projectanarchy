/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

/// \file hkvAssetPluginManager.hpp

#ifndef HKV_ASSET_PLUGIN_MANAGER_HPP_INCLUDED
#define HKV_ASSET_PLUGIN_MANAGER_HPP_INCLUDED

#include <vector>

class hkvPropertyEditingProvider;

/// \brief
///   Interface to be implemented within each asset plugin.
///
/// The initialization function of the asset plugin must return an instance of the plugin-specific
/// implementation of this interface.
class hkvAssetPluginInterface
{
public:
  hkvAssetPluginInterface() {}
private:
  hkvAssetPluginInterface(const hkvAssetPluginInterface&);
  hkvAssetPluginInterface& operator=(const hkvAssetPluginInterface&);
public:
  virtual ~hkvAssetPluginInterface() {}

public:
  /// \brief
  ///   Shuts down the asset plugin.
  virtual void shutDown() = 0;

  /// \brief
  ///   Returns an instance of a class providing support for editing custom properties.
  /// \return
  ///   an instance of hkvPropertyEditingProvider, or \c NULL, if this asset plugin does
  ///   not implement any custom property editing support.
  virtual hkvPropertyEditingProvider* getPropertyEditingProvider() { return NULL; }
};


/// \brief
///   Manages loading and unloading of asset plugins, and provides access to the loaded plugins.
class hkvAssetPluginManager
{
private:
  struct PluginInfo
  {
    hkStringPtr m_directory;
    HMODULE m_moduleHandle;
    bool m_unloadWithAssetLibrary;
    hkvAssetPluginInterface* m_interface;
  };

private:
  typedef std::vector<PluginInfo> PluginList;
  typedef hkvAssetPluginInterface* (*hkvInitAssetPluginFunction)();

public:
  hkvAssetPluginManager() {}
private:
  hkvAssetPluginManager(const hkvAssetPluginManager&);
  hkvAssetPluginManager& operator=(const hkvAssetPluginManager&);
public:
  /// \brief
  ///   Destructor; unloads all asset plugins that are still loaded.
  ~hkvAssetPluginManager();

public:
  /// \brief
  ///   Loads asset plugins from the specified directory or from the standard binaries
  ///   directory.
  ///
  /// Plugins loaded from a specific directory (which usually is an asset library) are flagged
  /// as library-specific and are automatically unloaded when the project is closed in vForge.
  ///
  /// \param dir
  ///   the directory to load asset plugins from. If \c NULL, plugins are loaded from
  ///   the standard binaries directory.
  ASSETFRAMEWORK_IMPEXP void loadPlugins(const char* dir = NULL);

  /// \brief
  ///   Unloads asset plugins.
  /// \param dir
  ///   the directory for which to unload asset plugins. If \c NULL, all loaded asset plugins
  ///   are unloaded.
  ASSETFRAMEWORK_IMPEXP void unloadPlugins(const char* dir = NULL);

  /// \brief
  ///   Unloads only those asset plugins that have been loaded from a library directory.
  ASSETFRAMEWORK_IMPEXP void unloadLibrarySpecificPlugins();

  /// \brief
  ///   Returns the number of loaded asset plugins.
  /// \return
  ///   the number of loaded asset plugins
  ASSETFRAMEWORK_IMPEXP size_t getPluginCount() const;

  /// \brief
  ///   Returns the interface to access a loaded asset plugin.
  /// \param index
  ///   the index of the asset plugin
  /// \return
  ///   pointer to the interface for accessing the asset plugin
  ASSETFRAMEWORK_IMPEXP hkvAssetPluginInterface* getPlugin(size_t index) const;

private:
  PluginList m_plugins;
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
