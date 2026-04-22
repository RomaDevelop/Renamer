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
#include "WidgetTable.h"

enum class FromMode { undefined = 0, caseSensitive, caseInsensitive, regExpr };

struct ReplaceSettings
{
	QString error;
	QString from;
	QString to;
	FromMode fromMode;
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
	void RefreshView();
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

	QComboBox *comboFromMode = new QComboBox();
	std::map<QString, FromMode> comboFromModeVals {
		{"From case sensitive", FromMode::caseSensitive},
		{"From case insensitive", FromMode::caseInsensitive},
		{"From regular expression", FromMode::regExpr}
	};
	FromMode GetFromMode();

	QRadioButton *radioReplaceFirst = new QRadioButton("First occurrence");
	QRadioButton *radioReplaceAll = new QRadioButton("All occurrences");
	QLineEdit *leFrom = new QLineEdit;
	QLineEdit *leTo = new QLineEdit;

	std::vector<ReplaceRow> preparedReplacesAll;
	WidgetTable *widgetTable;

	thread_box renameThread {"renameThread"};

	QStringList errors;
	void InitErrorsShowing(QHBoxLayout *hlo);

	void SlotScan();
	void SlotReplace();
	ReplaceSettings ReplaceSettingsGet();
	ReplaceRow PrepareReplaceForRow(const QString &row, const ReplaceSettings &replaceSettings);
};

#endif // MAINWINDOW_H
