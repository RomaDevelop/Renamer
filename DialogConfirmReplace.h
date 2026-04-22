#ifndef DIALOGCONFIRMREPLACE_H
#define DIALOGCONFIRMREPLACE_H

#include <vector>

#include <QWidget>

#include "RowsUpdater.h"

class QLabel;
class QPushButton;
class QSlider;
class QTableWidget;
class QTimer;

struct ReplaceRow;

class WidgetTable : public QWidget
{
	Q_OBJECT

public:
	explicit WidgetTable(std::vector<ReplaceRow> &replaces, QWidget *parent = nullptr);

	void FillTable();

private:
	void SetRowTexts(int row);

	void SetAllRowsChecked(Qt::CheckState state);
	void InvertRowsChecked();

	std::vector<ReplaceRow> &replaces;
	std::vector<QLabel*> currentValueLabels;
	std::vector<QLabel*> newValueLabels;

	QTableWidget *table;
	QSlider *sliderTrimmer;
	QPushButton *btnRun;

	RowsUpdater rowsUpdater;
	void SetRowsUpdaterArgs();
};

#endif // DIALOGCONFIRMREPLACE_H
