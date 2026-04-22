#include "RowsUpdater.h"

#include <QDebug>

#include <MyQShortings.h>

RowsUpdater::RowsUpdater(QObject *parent)
{
	timerPreviewUpdate = new QTimer(parent);
	QObject::connect(timerPreviewUpdate, &QTimer::timeout, [this](){
		if(not function) {
			qdbg << "RowsUpdater error: null function";
			return;
		}

		for(int i=0; i<3; i++)
		{
			if(args.empty()) return;

			auto arg = args.front();
			args.pop();

			function(arg);
		}
	});
	timerPreviewUpdate->start(10);
}
