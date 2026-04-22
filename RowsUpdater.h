#ifndef ROWSUPDATER_H
#define ROWSUPDATER_H

#include <queue>
#include <functional>

#include <QTimer>

struct RowsUpdater
{
	QTimer *timerPreviewUpdate;

	std::function<void(int row)> function;
	std::queue<int> args;

	RowsUpdater(QObject *parent);
};

#endif
