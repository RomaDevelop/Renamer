#ifndef DIALOGCONFIRMREPLACE_H
#define DIALOGCONFIRMREPLACE_H

#include <vector>

#include <QDialog>

#include "MainWidget.h"

class QLabel;
class QPushButton;
class QSlider;
class QTableWidget;

class DialogConfirmReplace : public QDialog
{
	Q_OBJECT

public:
	explicit DialogConfirmReplace(std::vector<Replace> &replaces, QWidget *parent = nullptr);

	static bool Confirm(std::vector<Replace> &replaces, QWidget *parent = nullptr);

private:
	void FillTable();
	void UpdatePreviewTexts();
	void UpdatePreviewFont();
	void UpdateRunButtonState();
	void SetAllRowsChecked(Qt::CheckState state);
	void InvertRowsChecked();

	std::vector<Replace> &replaces;
	std::vector<QLabel*> currentValueLabels;
	std::vector<QLabel*> newValueLabels;

	QTableWidget *table;
	QSlider *sliderTrimStartPercent;
	QSlider *sliderFontSize;
	QPushButton *btnRun;
};

#endif // DIALOGCONFIRMREPLACE_H
