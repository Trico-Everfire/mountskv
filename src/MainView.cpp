#include "MainView.h"

#include "widgets/DraggableTreeWidget.h"

#include <QDebug>
#include <QFileDialog>
#include <QGridLayout>
#include <QMessageBox>

// Development of the Import button.
// currently under development and does not
// function.
// #define IMPORT_MOUNT

namespace ui
{
	CMainView::CMainView( QWidget *pParent ) :
		QDialog( pParent )
	{
		setMinimumSize( 100, 100 );
		resize( 400, 500 );
		auto layout = new QGridLayout( this );

		m_treeWidget = new QTreeWidget( this );
		layout->addWidget( m_treeWidget, 0, 0, 1, 2 );
		m_treeWidget->resize( 400, 500 );
		m_treeWidget->headerItem()->setText( 0, "Games:" );

		m_treeWidget->setSelectionMode( QAbstractItemView::SingleSelection );
		m_treeWidget->setDragEnabled( true );
		m_treeWidget->viewport()->setAcceptDrops( true );
		m_treeWidget->setDropIndicatorShown( true );
		m_treeWidget->setDragDropMode( QAbstractItemView::InternalMove );

		auto appids = m_steamGameProvider->GetInstalledAppsEX();
		qInfo() << &appids;
		QStringList installDirs;
		for ( int i = 0; i < m_steamGameProvider->GetNumInstalledApps(); i++ )
		{
			if ( m_steamGameProvider->BIsSourceGame( appids[i] ) )
			{
				auto game = m_steamGameProvider->GetAppInstallDirEX( appids[i] );

				if ( installDirs.contains( game->installDir ) )
					continue;
				installDirs.append( game->installDir );

				auto *item = new QTreeWidgetItem();
				item->setCheckState( 0, Qt::Unchecked );
				item->setData( 0, Qt::UserRole, game->appid );

				auto flags = item->flags().setFlag( Qt::ItemFlag::ItemIsDropEnabled, false );
				item->setFlags( flags );
				item->setText( 0, game->gameName );
				fs::path paths( game->library );
				paths.append( "common" );
				paths.append( game->installDir );

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

		m_exportButton = new QPushButton( this );
		m_exportButton->setText( tr( "Export Mounts.kv" ) );
		layout->addWidget( m_exportButton, 1, 0 );

		m_importButton = new QPushButton( this );
		m_importButton->setText( tr( "Import Mounts.kv" ) );
#ifndef IMPORT_MOUNT
		m_importButton->setDisabled( true );
#endif

		connect( m_exportButton, &QPushButton::pressed, [this]()
				 {
					 QString savePath = QFileDialog::getSaveFileName( this, "Save Mount.kv", "./", "*.kv" );
					 if(savePath.isEmpty())
						 return;
					 auto mountskv = GenerateMountKV();
					 auto file = QFile( savePath );
					 file.open( QFile::WriteOnly );
					 file.write( mountskv->ToString() );
					 file.close();
				 } );
#ifdef IMPORT_MOUNT
		connect( m_importButton, &QPushButton::pressed, [this]()
				 {
					 ImportMounts( "./mounts.kv" );
				 } );
#endif
		layout->addWidget( m_importButton, 1, 1 );
	}

	KeyValueRoot *CMainView::GenerateMountKV() const
	{
		auto mountskv = new KeyValueRoot();
		auto mounts = mountskv->AddNode( "Mounts" );
		for ( int i = 0; i < m_treeWidget->topLevelItemCount(); ++i )
		{
			QTreeWidgetItem *item = m_treeWidget->topLevelItem( i );
			if ( item->checkState( 0 ) != Qt::Checked )
				continue;
			auto kv = mounts->AddNode( item->data( 0, Qt::UserRole ).toString().toStdString().c_str() );
			for ( int j = 0; j < item->childCount(); j++ )
			{
				auto child = item->child( j );
				bool isValid = false;
				hasChildHierarchy( child, isValid );
				if ( isValid || child->checkState( 0 ) == Qt::Checked )
				{
					auto node = kv->AddNode( child->data( 0, Qt::UserRole ).toString().toStdString().substr( 1 ).c_str() );
					QString str;
					dirvpk( child, str, node );
				}
			}
		}

		return mountskv;
	}

	void CMainView::PathResolver( QTreeWidgetItem *parent, const fs::path& sPath )
	{
		for ( auto &dir : fs::directory_iterator( sPath, fs::directory_options::skip_permission_denied ) )
		{
			auto currentPath = dir.path().u16string().substr( sPath.u16string().length() );
			if ( dir.is_directory() )
			{
				auto *item = new QTreeWidgetItem();
				item->setText( 0, QString::fromStdU16String( currentPath.substr( 1 ).c_str() ) );
				item->setData( 0, Qt::UserRole, QString::fromStdU16String( currentPath.c_str() ) );
				item->setCheckState( 0, Qt::CheckState::Unchecked );
				parent->addChild( item );
				sPath.u16string().append( currentPath );
				PathResolver( item, fs::path( sPath.u16string().append( currentPath ) ) );
				item->sortChildren( 0, Qt::SortOrder::AscendingOrder );
			}
			if ( currentPath.ends_with( u"_dir.vpk" ) )
			{
				auto *item = new QTreeWidgetItem();
				item->setIcon( 0, QIcon( ":/resource/VPK.jpg" ) );
				item->setCheckState( 0, Qt::CheckState::Unchecked );
				item->setData( 0, Qt::UserRole, QString::fromStdU16String( currentPath.c_str() ) );
				item->setText( 0, QString::fromStdU16String( currentPath.substr(1).c_str() ) );
				parent->addChild( item );
			}
		}
	}
#ifdef IMPORT_MOUNT
	void CMainView::ImportMounts( QString file )
	{
		QFile fileData( file );
		fileData.open( QFile::ReadOnly );
		auto keyFile = KeyValueRoot::Create( fileData.readAll() );
		keyFile->Solidify();
		fileData.close();
		auto &mounts = keyFile->Get( "Mounts" );
		if ( !mounts.IsValid() )
		{
			QMessageBox::warning( this, "Invalid Mounts.kv", "The mounts.kv file you selected is invalid and cannot be parsed." );
			return;
		}
		for ( int i = 0; i < mounts.ChildCount(); i++ )
		{
			auto &game = mounts[i];
			auto appid = std::stoi( game.Key().string );

			if ( m_steamGameProvider->BIsAppInstalled( appid ) )
			{
				auto gameObject = m_steamGameProvider->GetAppInstallDirEX( appid );
				//				auto gameContents = game;
				for ( int j = 0; j < game.ChildCount(); j++ )
				{
					auto &gameProvider = game[j];
					for ( int k = 0; k < gameProvider.ChildCount(); k++ )
					{
						auto &gameContents = gameProvider[k];
						QRegExp rx( R"(\b(\/|\\)\b)" );
						auto sGameValue = QString( gameContents.Value().string );
						auto stringList = sGameValue.split( rx );
						for ( int l = 0; l < stringList.length(); l++ )
						{
							auto str = stringList[l];
							// qInfo() << m_treeWidget->findItems()
						}

						qInfo() << stringList;
					}
				}
				delete gameObject;
			}
		}
	}
#endif

	void CMainView::dirvpk( QTreeWidgetItem *item, const QString& str, KeyValue *kv )
	{
		for ( int j = 0; j < item->childCount(); j++ )
		{
			auto child = item->child( j );
			auto data = child->data( 0, Qt::UserRole ).toString();

			if ( data.endsWith( "_dir.vpk" ) )
			{
				if ( child->checkState( 0 ) != Qt::Checked )
					continue;
				QString aStr( str );
				aStr.append( data );
				kv->Add( "vpk", aStr.toStdString().substr( 1, aStr.length() - 5 ).c_str() );
				continue;
			}

			QString aStr( str );
			aStr.append( data );
			if ( child->checkState( 0 ) == Qt::Checked )
				kv->Add( "dir", aStr.toStdString().substr( 1 ).c_str() );
			dirvpk( item->child( j ), aStr, kv );
		}
	}

	void CMainView::hasChildHierarchy( QTreeWidgetItem *item, bool &isValid )
	{
		for ( int j = 0; j < item->childCount(); j++ )
		{
			auto child = item->child( j );
			if ( child->checkState( 0 ) == Qt::Checked )
				isValid = true;
		}
	}

} // namespace ui
