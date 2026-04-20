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
#include <QRadioButton>
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

using var_setting = std::variant<QString*, QByteArray*, bool*, QWidgetGeometry, QSplitterState>;

struct setting
{
	QString name;
	var_setting var;

	void VarFromStr(const QString &str);
	QString VarToStr();
};

struct ReplaceSettings
{
	QString error;
	QString from;
	QString to;
	bool fromRegExprEnabled;
	bool replaceAllEntries;
	QRegularExpression fromRegExpr;
};

struct ReplaceMatch
{
	int foundIndex = -1;
	int foundIndexInNameWithPath = -1;
	int lengthToReplace = 0;
};

struct Replace
{
	QString error;
	QString from;
	QString to;
	std::vector<ReplaceMatch> matches;

	bool HasMatches() const { return not matches.empty(); }
};

class MainWidget : public QWidget
{
	Q_OBJECT

public:
	MainWidget(QWidget *parent = nullptr);
	~MainWidget();

private:
	void CreateBottomRow(QVBoxLayout *vloMain);
	void UpdateFindResHighlight();
	void ClearFindResHighlight();
	std::vector<std::pair<int, int>> GetHighlightRanges(const QStringList &rows, const ReplaceSettings &replaceSettings, bool *showInfoForAdd = nullptr);

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
	QRadioButton *radioReplaceFirst = new QRadioButton("First occurrence");
	QRadioButton *radioReplaceAll = new QRadioButton("All occurrences");
	QLineEdit *leFrom = new QLineEdit;
	QLineEdit *leTo = new QLineEdit;

	QTextEdit *textEditFindRes = new QTextEdit;
	bool replaceAllEntries = false;

	void SlotScan();
	void SlotReplace();
	ReplaceSettings ReplaceSettingsGet()
	{
		ReplaceSettings settings {
			"",
			leFrom->text(),
			leTo->text(),
			checkBoxRegExprInFrom->isChecked(),
			radioReplaceAll->isChecked(),
			QRegularExpression(leFrom->text())
		};

		if(settings.fromRegExprEnabled and not settings.fromRegExpr.isValid())
			settings.error = settings.fromRegExpr.errorString();

		return settings;
	}
	Replace PrepareReplaceForRow(const QString &row, const ReplaceSettings &replaceSettings);

	static QStringList GetRows(QTextEdit *textEdit);
};

#endif // MAINWINDOW_H
