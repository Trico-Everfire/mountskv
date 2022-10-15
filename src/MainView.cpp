//
// Created by trico on 27-9-22.
//
#include "MainView.h"

#include "widgets/DraggableTreeWidget.h"

#include <QDebug>
#include <QFileDialog>
#include <QGridLayout>
#include <QMessageBox>

namespace ui
{
	CMainView::CMainView( QWidget *pParent ) :
		QDialog( pParent )
	{
		setMinimumSize( 100, 100 );
		resize( 400, 500 );
		auto layout = new QGridLayout(this);

		m_treeWidget = new QTreeWidget( this );
		layout->addWidget(m_treeWidget,0,0,1,2);
		m_treeWidget->resize( 400, 500 );
		m_treeWidget->headerItem()->setText( 0, "Games:" );

		m_treeWidget->setSelectionMode( QAbstractItemView::SingleSelection );
		m_treeWidget->setDragEnabled( true );
		m_treeWidget->viewport()->setAcceptDrops( true );
		m_treeWidget->setDropIndicatorShown( true );
		m_treeWidget->setDragDropMode( QAbstractItemView::InternalMove );

		auto appids = m_steamGameProvider->GetInstalledAppsEX();
		QStringList installDirs;
		for ( int i = 0; i < m_steamGameProvider->GetNumInstalledApps() + 1; i++ )
		{
			if ( m_steamGameProvider->BIsSourceGame( appids[i] ) )
			{
				auto game = m_steamGameProvider->GetAppInstallDirEX( appids[i] );

				if ( installDirs.contains( game->installDir ) )
					continue;
				installDirs.append( game->installDir );

				QTreeWidgetItem *item = new QTreeWidgetItem();
				item->setCheckState( 0, Qt::Unchecked );
				item->setData( 0, Qt::UserRole, game->appid );

				auto flags = item->flags().setFlag( Qt::ItemFlag::ItemIsDropEnabled, false );
				item->setFlags( flags );
				item->setText( 0, game->gameName );
				fs::path paths( game->library );
				paths.append( "common" );
				paths.append( game->installDir );

				QString pathFolder = "null";
				fs::path vPath = paths;
				QStringList pathList;

				PathResolver( item, paths );
				item->sortChildren( 0, Qt::SortOrder::AscendingOrder );


				QPixmap im( strdup( game->icon ) );
				item->setIcon( 0, QIcon( im ) );
				m_treeWidget->addTopLevelItem( item );
				delete game;
			}
		}
		delete[] appids;
		m_treeWidget->sortItems( 0, Qt::SortOrder::AscendingOrder );

		m_exportButton = new QPushButton(this);
		m_exportButton->setText(tr("Export Mounts.kv"));
		layout->addWidget(m_exportButton,1,0);

		m_importButton = new QPushButton(this);
		m_importButton->setText(tr("Import Mounts.kv"));
		m_importButton->setDisabled(true);


		connect(m_exportButton, &QPushButton::pressed, [this](){

					 QFileDialog* fd = new QFileDialog(this, "Save Mount.kv", "./","*.kv");
					 fd->exec();
					 if(fd->selectedFiles().isEmpty()) return;
					 QString savePath = fd->selectedFiles()[0];
					 auto mountskv = GenerateMountKV();
					 auto file = QFile( savePath );
					 file.open( QFile::WriteOnly );
					 file.write( mountskv->ToString() );
					 file.close();
		});
		layout->addWidget(m_importButton,1,1);

	}

	KeyValueRoot* CMainView::GenerateMountKV()
	{
		auto mountskv = new KeyValueRoot();
		auto mounts = mountskv->AddNode( "Mounts" );
		for ( int i = 0; i < m_treeWidget->topLevelItemCount(); ++i )
		{
			QTreeWidgetItem *item = m_treeWidget->topLevelItem( i );
			if(item->checkState(0) != Qt::Checked) continue;
			auto kv = mounts->AddNode( item->data( 0, Qt::UserRole ).toString().toStdString().c_str() );
			for(int j = 0; j < item->childCount(); j++){
				auto child = item->child(j);
				bool isValid = false;
				hasChildHierarchy(child,isValid);
				if(isValid == true || child->checkState(0) == Qt::Checked)
				{
					auto node = kv->AddNode( child->data( 0, Qt::UserRole ).toString().toStdString().substr( 1 ).c_str() );
					QString str;
					dirvpk( child, str, node );
				}
			}

		}

		return mountskv;
	}

	void CMainView::PathResolver( QTreeWidgetItem *parent, fs::path sPath )
	{
		for ( auto &dir : fs::directory_iterator( sPath, fs::directory_options::skip_permission_denied ) )
		{
			auto currentPath = dir.path().string().substr( sPath.string().length() );
			if ( dir.is_directory() )
			{
				QTreeWidgetItem *item = new QTreeWidgetItem();
				item->setText( 0, currentPath.substr( 1 ).c_str() );
				item->setData( 0, Qt::UserRole, QString( currentPath.c_str() ) );
				item->setCheckState( 0, Qt::CheckState::Unchecked );
				parent->addChild( item );
				sPath.string().append( currentPath );
				PathResolver( item, fs::path( sPath.string().append( currentPath ) ) );
				item->sortChildren( 0, Qt::SortOrder::AscendingOrder );
			}
			if ( currentPath.ends_with( "_dir.vpk" ) )
			{
				QTreeWidgetItem *item = new QTreeWidgetItem();
				item->setIcon( 0, QIcon( ":/resource/VPK.jpg" ) );
				item->setCheckState( 0, Qt::CheckState::Unchecked );
				item->setData( 0, Qt::UserRole, QString( currentPath.c_str() ) );
				item->setText( 0, currentPath.substr( 1 ).c_str() );
				parent->addChild( item );
			}
		}
	}

	void CMainView::dirvpk(QTreeWidgetItem *item, QString str, KeyValue* kv){
		for(int j = 0; j < item->childCount(); j++)
		{
			auto child = item->child(j);
			auto data = child->data(0,Qt::UserRole).toString();

			if(data.endsWith("_dir.vpk")){
				if(child->checkState(0) != Qt::Checked) continue;
				QString aStr(str);
				aStr.append(data);
				kv->Add("vpk",aStr.toStdString().substr(1,aStr.length() - 5).c_str());
				continue;
			}

			QString aStr(str);
			aStr.append(data);
			if(child->checkState(0) == Qt::Checked)
				kv->Add("dir",aStr.toStdString().substr(1).c_str());
			dirvpk(item->child(j), aStr, kv);
		}
	}

	void CMainView::hasChildHierarchy(QTreeWidgetItem *item, bool& isValid){
		for(int j = 0; j < item->childCount(); j++)
		{
			auto child = item->child(j);
			if(child->checkState(0) == Qt::Checked)
				isValid =  true;
		}
	}

} // namespace ui