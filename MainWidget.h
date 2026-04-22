#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <vector>

#include <QRegularExpression>
#include <QLineEdit>
#include <QTextEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QSplitter>
#include <QStringList>

#include "MyQDifferent.h"
#include "thread_box.h"

#include "Settings.h"
#include "DialogConfirmReplace.h"



struct ReplaceSettings
{
	QString error;
	QString from;
	QString to;
	bool fromRegExprEnabled;
	bool replaceAllEntries;
	QRegularExpression fromRegExpr;
};

struct Index
{
	int startIndex = -1;
	int startIndexInNameWithPath = -1;
	int length = 0;

	Index() = default;
	Index(int startIndex, int startIndexInNameWithPath, int length)
		: startIndex{startIndex},
		  startIndexInNameWithPath{startIndexInNameWithPath},
		  length{length}
	{}
};

struct OneMatch
{
	Index indexInSrc;
	Index indexInResult;

	OneMatch() = default;
	explicit OneMatch(const Index &indexInSrc)
		: indexInSrc{indexInSrc}
	{}
	OneMatch(const Index &indexInSrc, const Index &indexInResult)
		: indexInSrc{indexInSrc},
		  indexInResult{indexInResult}
	{}
};

struct ReplaceRow
{
	QString error;
	QString from;
	QString to;
	std::vector<OneMatch> matches;
	bool enabled = true;

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
	void UpdatePreparedReplaces(bool *showInfoForAdd = nullptr);
	void RefreshFindResView(bool *showInfoForAdd = nullptr);
	std::vector<int> GetDisplayedReplaceIndexes() const;
	QString BuildLogsText(const QStringList &logs, const QStringList &errors) const;
	QString LogsDirPath() const;
	void SaveLogs(const QStringList &logs, const QStringList &errors) const;
	void OpenLogsDir();

	void SaveSettings();
	void LoadSettings();
	std::vector<setting> GetSettingsObjectsList();

	QString settingsFile = MyQDifferent::ExePath()+"/files/settings.ini";
	QString notesContent;

	QSplitter *splitter;
	QTextEdit *textEditDirs = new QTextEdit;
	QCheckBox *checkBoxIncludeSubcats = new QCheckBox("Include subcats");
	QCheckBox *checkBoxRegExprInFrom = new QCheckBox("Reg. expr. in from");
	QRadioButton *radioReplaceFirst = new QRadioButton("First occurrence");
	QRadioButton *radioReplaceAll = new QRadioButton("All occurrences");
	QLineEdit *leFrom = new QLineEdit;
	QLineEdit *leTo = new QLineEdit;

	QComboBox *comboFindResView = new QComboBox;
	WidgetTable *widgetTable;
	QStringList findResRowsAll;
	std::vector<ReplaceRow> preparedReplacesAll;
	thread_box renameThread {"renameThread"};

	void SlotScan();
	void SlotReplace();
	ReplaceSettings ReplaceSettingsGet() const
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
	ReplaceRow PrepareReplaceForRow(const QString &row, const ReplaceSettings &replaceSettings) const;
};

#endif // MAINWINDOW_H
