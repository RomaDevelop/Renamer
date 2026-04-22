#ifndef DIALOGCONFIRMREPLACE_H
#define DIALOGCONFIRMREPLACE_H

#include <vector>

#include <QComboBox>
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

	QComboBox *comboShowAllOrFound = new QComboBox;

private:
	void SetRowTexts(int row);

	/// 0 = uncheked, 1 = checked, 2 = invert, >2 = checked
	void SetAllRowsCheckState(uint val);

	std::vector<ReplaceRow> &replaces;

	QTableWidget *table;
	QSlider *sliderTrimmer;
	QPushButton *btnRun;

	RowsUpdater rowsUpdater;
	void SetRowsUpdaterArgs();
	void SetRowsUpdaterFunction();
};

#endif // DIALOGCONFIRMREPLACE_H
