/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

/// \brief A wrapper around a platform-specific native dialog box.
///
/// This message box doesn't use any Vision resources, and can be used before the
/// Engine is initialized.
class IVNativeMobileDialog
{
public:
  /// \brief The current state of the dialog box.
  enum State
  {
    CANCELLED = -2, ///< The dialog was cancelled by some external means, e.g. by pressing the back button on the device.
    WAITING = -1,   ///< There was no user input yet.
    BUTTON0 = 0,    ///< The button with index 0 was pressed.
    BUTTON1 = 1,    ///< The button with index 1 was pressed.
    BUTTON2 = 2,    ///< The button with index 2 was pressed.
  };

  /// \brief Destructor.
  virtual ~IVNativeMobileDialog() {}

  /// \brief Creates an instance of the dialog.
  static IVNativeMobileDialog* CreateInstance();

  /// \brief Sets the title of the dialog.
  virtual void SetTitle(const char* pTitle) = 0;

  /// \brief Sets the text message of the dialog.
  ///
  /// Changing the text after calling Show() works on all platforms.
  virtual void SetText(const char* pText) = 0;

  /// \brief Adds a button, and returns its index.
  virtual int AddButton(const char* pButtonText) = 0;

  /// \brief Shows the dialog and returns immediately.
  ///
  /// Depending on the platform, title and buttons can't be changed anymore after this call.
  virtual void Show() = 0;

  /// \brief Closes the dialog.
  virtual void Close() = 0;

  /// \brief Retrieves the current state of the dialog.
  ///
  /// \sa IVNativeMobileDialog::State
  virtual State GetState() = 0;
};

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
