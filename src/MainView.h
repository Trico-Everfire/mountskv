//
// Created by trico on 27-9-22.
//
#pragma once
#ifndef MOUNTSKV_MAINVIEW_H
#define MOUNTSKV_MAINVIEW_H

#include <QDialog>
#include <QScrollArea>
#include <QPushButton>
#include <QTreeWidget>
#include "FilesystemSearchProvider.h"

namespace ui {

    class CMainView : public QDialog {
		Q_OBJECT
		void PathResolver (QTreeWidgetItem* parent, fs::path sPath);
		void dirvpk(QTreeWidgetItem *item, QString str, KeyValue* kv);
		void hasChildHierarchy(QTreeWidgetItem *item, bool& isValid);
	public:
		explicit CMainView(QWidget* pParent);
		QTreeWidget *m_treeWidget;
		CFileSystemSearchProvider *m_steamGameProvider = new CFileSystemSearchProvider();
		QPushButton *m_exportButton;
		QPushButton *m_importButton;
		KeyValueRoot* GenerateMountKV();

		void ImportMounts( QString file );
	};

} // ui

#endif //MOUNTSKV_MAINVIEW_H
