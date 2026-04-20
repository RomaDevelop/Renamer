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
	void UpdatePreviewFont();
	void UpdateRunButtonState();
	void SetAllRowsChecked(Qt::CheckState state);
	void InvertRowsChecked();

	std::vector<Replace> &replaces;

	QTableWidget *table;
	QSlider *sliderFontSize;
	QPushButton *btnRun;
};

#endif // DIALOGCONFIRMREPLACE_H
