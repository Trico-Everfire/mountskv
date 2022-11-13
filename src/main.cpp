
#include "MainView.h"

#include <QApplication>
#include <QMessageBox>
#include <stdlib.h>

// AppID which we will use for steam.
// 440000 - P2CE
// 1400890 - P2CE SDK
// 1802710 - Momentum Mod
constexpr int APP_ID = 640;

// Main application

int main( int argc, char **argv )
{
    QApplication app( argc, argv );

    QApplication::setAttribute( Qt::AA_DisableWindowContextHelpButton );

    auto pDialog = new ui::CMainView( nullptr );
	if(!pDialog->m_steamGameProvider->Available()) {
		QMessageBox::critical(nullptr,"Game Fetching Error", "There was an error getting your game library, either your libraryfolders.vdf is corrupt or you have no games installed on your computer.");
		return EXIT_FAILURE;
	}
    pDialog->setWindowTitle( "Mounts.kv Editor" );
    pDialog->show();

    return QApplication::exec();
}

