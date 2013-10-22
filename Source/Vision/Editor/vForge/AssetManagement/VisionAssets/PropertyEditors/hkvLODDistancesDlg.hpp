/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

#ifndef HKV_LOD_DISTANCES_DLG_HPP
#define HKV_LOD_DISTANCES_DLG_HPP

#include <Vision/Runtime/Base/VBase.hpp> // Needed for the moc_* file to compile properly
#include <Vision/Editor/vForge/AssetManagement/AssetFramework/hkvIProperties.hpp>

ANALYSIS_IGNORE_ALL_START
#include <QDialogButtonBox>
#include <Vision/Editor/vForge/AssetManagement/VisionAssets/GeneratedFiles/ui_hkvLODDistancesDlg.h>
ANALYSIS_IGNORE_ALL_END


class hkvLODDistancesDlg : public QWidget, public Ui_hkvLODDistancesDlg
{
  Q_OBJECT

private:
  class TableModel;

public:
  hkvLODDistancesDlg(hkvProperty::VariantValueList& distances, QWidget* parent = 0, Qt::WindowFlags f = 0);
  virtual ~hkvLODDistancesDlg();

signals:
  void AvailableButtonsChanged(QDialogButtonBox::StandardButtons buttons);
  void ButtonStateChanged(QDialogButtonBox::StandardButton button, bool enabled, QString text);
  void RequestDialogClose(int returnCode);

public slots:
  void OnDialogButtonClicked(QDialogButtonBox::StandardButton button);
  void OnWidgetEmbedded(QWidget* widget);

private:
  hkvProperty::VariantValueList& m_distances;
};


class hkvLODDistancesDlg::TableModel: public QAbstractTableModel
{
public:
  TableModel(hkvLODDistancesDlg& owner);

  virtual int rowCount(const QModelIndex& parent = QModelIndex()) const HKV_OVERRIDE;
  virtual int columnCount(const QModelIndex& parent = QModelIndex()) const HKV_OVERRIDE;
  virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const HKV_OVERRIDE;
  virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) HKV_OVERRIDE;
  virtual Qt::ItemFlags flags(const QModelIndex& index) const HKV_OVERRIDE;
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const HKV_OVERRIDE;

private:
  hkvLODDistancesDlg& m_owner;
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
