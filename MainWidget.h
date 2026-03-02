#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <atomic>
#include <memory>
#include <thread>
#include <variant>

#include <QRegularExpression>
#include <QMainWindow>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QSettings>
#include <QSplitter>

#include "MyQDifferent.h"

struct QWidgetGeometry
{
	QWidget *widget;
	QWidgetGeometry(QWidget *widget): widget{widget} {}
};

struct QSplitterState
{
	QSplitter *splitter;
	QSplitterState(QSplitter *splitter): splitter{splitter} {}
};

using var_setting = std::variant<QString*, QByteArray*, QWidgetGeometry, QSplitterState>;

struct setting
{
	QString name;
	var_setting var;

	void VarFromStr(const QString &str);
	QString VarToStr();
};

struct ReplaceSettings
{
	QString from;
	QString to;
	bool fromRegExprEnabled;
	QRegularExpression fromRegExpr;
};

struct Replace
{
	QString error;
	QString from;
	QString to;
	int foundIndex = -1;
	int foundIndexInNameWithPath = -1;
	int lengthToReplace = 0;
};

class MainWidget : public QWidget
{
	Q_OBJECT

public:
	MainWidget(QWidget *parent = nullptr);
	~MainWidget();

private:
	void CreateBottomRow(QVBoxLayout *vloMain);

	void SaveSettings();
	void LoadSettings();
	std::vector<setting> GetSettings();

	QString settingsFile = MyQDifferent::ExePath()+"/files/settings.ini";
	QString notesContent;

	QSplitter *splitter;
	QTextEdit *textEditDirs = new QTextEdit;
	QLineEdit *leFilter = new QLineEdit;
	QCheckBox *checkBoxIncludeSubcats = new QCheckBox("Include subcats");
	QCheckBox *checkBoxRegExprInFrom = new QCheckBox("Reg. expr. in from");
	QLineEdit *leFrom = new QLineEdit;
	QLineEdit *leTo = new QLineEdit;

	QTextEdit *textEditFindRes = new QTextEdit;

	void SlotScan();
	void SlotReplace();
	ReplaceSettings ReplaceSettingsGet()
	{
		return { leFrom->text(),
				 leTo->text(),
				 checkBoxRegExprInFrom->isChecked(),
				 QRegularExpression(leFrom->text())
				};
	}
	Replace PrepareReplaceForRow(const QString &row, const ReplaceSettings &replaceSettings);

	static QStringList GetRows(QTextEdit *textEdit);
};

#endif // MAINWINDOW_H
