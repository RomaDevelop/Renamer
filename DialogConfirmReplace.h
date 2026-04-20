#ifndef DIALOGCONFIRMREPLACE_H
#define DIALOGCONFIRMREPLACE_H

#include <set>
#include <vector>

#include <QDialog>

#include "MainWidget.h"

class QLabel;
class QPushButton;
class QSlider;
class QTableWidget;
class QTimer;

class DialogConfirmReplace : public QDialog
{
	Q_OBJECT

public:
	explicit DialogConfirmReplace(std::vector<Replace> &replaces, QWidget *parent = nullptr);

	static bool Confirm(std::vector<Replace> &replaces, QWidget *parent = nullptr);

private:
	void FillTable();
	void UpdatePreviewTexts();
	void RebuildPreviewUpdateQueues();
	void ProcessPreviewUpdateChunk();
	void UpdatePreviewRow(int row);
	void UpdatePreviewFont();
	void UpdateRunButtonState();
	void SetAllRowsChecked(Qt::CheckState state);
	void InvertRowsChecked();

	std::vector<Replace> &replaces;
	std::vector<QLabel*> currentValueLabels;
	std::vector<QLabel*> newValueLabels;
	std::set<int> rowsInViewportCached;
	std::vector<int> rowsPendingVisible;
	std::vector<int> rowsPendingOther;
	bool rowsInViewportProcessed = false;
	int trimStartPercentPending = 0;

	QTableWidget *table;
	QSlider *sliderTrimStartPercent;
	QSlider *sliderFontSize;
	QPushButton *btnRun;
	QTimer *timerPreviewUpdate;
};

#endif // DIALOGCONFIRMREPLACE_H
