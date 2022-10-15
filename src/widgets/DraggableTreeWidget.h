//
// Created by trico on 12-10-22.
//

#ifndef MOUNTSKV_DRAGGABLETREEWIDGET_H
#define MOUNTSKV_DRAGGABLETREEWIDGET_H

#include <QApplication>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QDropEvent>

class TreeView: public QTreeWidget
{
public:
	TreeView(QWidget* pParent) : QTreeWidget(pParent)
	{
		resize(200, 300);

		setSelectionMode(QAbstractItemView::SingleSelection);
		setDragEnabled(true);
		viewport()->setAcceptDrops(true);
		setDropIndicatorShown(true);
		setDragDropMode(QAbstractItemView::InternalMove);

	}

private:
	virtual void  dropEvent(QDropEvent * event)
	{
		QModelIndex droppedIndex = indexAt( event->pos() );

		if( !droppedIndex.isValid() )
			return;

		QTreeWidget::dropEvent(event);
	}
};

#endif // MOUNTSKV_DRAGGABLETREEWIDGET_H
