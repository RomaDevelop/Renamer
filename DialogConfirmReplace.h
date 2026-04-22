#ifndef DIALOGCONFIRMREPLACE_H
#define DIALOGCONFIRMREPLACE_H

#include <set>
#include <vector>

#include <QDialog>

#include "MainWidget.h"
#include "RowsUpdater.h"

class QLabel;
class QPushButton;
class QSlider;
class QTableWidget;
class QTimer;

class DialogConfirmReplace : public QDialog
{
	Q_OBJECT

public:
	explicit DialogConfirmReplace(std::vector<ReplaceRow> &replaces, QWidget *parent = nullptr);

	static bool Confirm(std::vector<ReplaceRow> &replaces, QWidget *parent = nullptr);

private:
	void FillTable();
	void UpdatePreviewTexts();
	void UpdatePreviewRow(int row);
	void UpdateRunButtonState();
	void SetAllRowsChecked(Qt::CheckState state);
	void InvertRowsChecked();

	std::vector<ReplaceRow> &replaces;
	std::vector<QLabel*> currentValueLabels;
	std::vector<QLabel*> newValueLabels;
	int trimStartPercentPending = 0;

	QTableWidget *table;
	QSlider *sliderTrimStartPercent;
	QPushButton *btnRun;

	RowsUpdater rowsUpdater;
	void SetRowsUpdaterArgs();
};

#endif // DIALOGCONFIRMREPLACE_H
