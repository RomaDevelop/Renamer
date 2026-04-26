#include "MainWidget.h"

#include "WidgetTable.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDirIterator>
#include <QMetaObject>
#include <QMessageBox>
#include <QEventLoop>
#include <QLabel>
#include <QProgressDialog>
#include <QSplitter>
#include <QSettings>
#include <QDateTime>
#include <QDir>
#include <QTimer>

#include "MyQShortings.h"
#include "MyQFileDir.h"
#include "MyQDialogs.h"
#include "MyQExecute.h"
#include "MyQTextEdit.h"
#include "MyCppDifferent.h"

MainWidget::MainWidget(QWidget *parent)
	: QWidget(parent),
	  widgetTable { new WidgetTable(preparedReplacesAll, this) }
{
	textEditDirs->setLineWrapMode(QTextEdit::NoWrap);

	checkBoxIncludeSubcats->setChecked(true);

	auto vloMain = new QVBoxLayout(this);
	splitter = new QSplitter(Qt::Horizontal);

	vloMain->addWidget(splitter);

	auto w_in_l1 = new QWidget;
	splitter->addWidget(w_in_l1);
	auto vloLeft = new QVBoxLayout(w_in_l1);
	vloLeft->setContentsMargins(0,0,0,0);

	vloLeft->addWidget(new QLabel("Dirs:"));
	vloLeft->addWidget(textEditDirs);
	vloLeft->addWidget(checkBoxIncludeSubcats);
	vloLeft->addWidget(comboFromMode);
	for(auto &node:comboFromModeVals) comboFromMode->addItem(node.first);
	radioReplaceFirst->setChecked(true);
	vloLeft->addWidget(radioReplaceFirst);
	vloLeft->addWidget(radioReplaceAll);
	vloLeft->addWidget(new QLabel("Replace from:"));
	vloLeft->addWidget(leFrom);
	vloLeft->addWidget(new QLabel("Replace to:"));
	vloLeft->addWidget(leTo);
	vloLeft->addStretch();

	auto w_in_r1 = new QWidget;
	splitter->addWidget(w_in_r1);
	auto vloRight = new QVBoxLayout(w_in_r1);
	vloRight->setContentsMargins(0,0,0,0);
	vloRight->addWidget(widgetTable);

	CreateBottomRow(vloMain);

	connect(radioReplaceFirst, &QAbstractButton::toggled, this, [this](){ RefreshView(); });
	connect(radioReplaceAll, &QAbstractButton::toggled, this, [this](){ RefreshView(); });
	connect(comboFromMode, qOverload<int>(&QComboBox::currentIndexChanged), this, [this](){ RefreshView(); });
	connect(leFrom, &QLineEdit::textChanged, this, [this](){ RefreshView(); });
	connect(leTo, &QLineEdit::textChanged, this, [this](){ RefreshView(); });
	connect(widgetTable->comboShowAllOrFound, qOverload<int>(&QComboBox::currentIndexChanged), this, [this](){ RefreshView(); });

	QTimer::singleShot(0, [this](){ LoadSettings(); });
}

MainWidget::~MainWidget()
{
	SaveSettings();
}

void MainWidget::CreateBottomRow(QVBoxLayout * vloMain)
{
	auto hlo2 = new QHBoxLayout;
	vloMain->addLayout(hlo2);

	auto btnScan = new QPushButton("Scan");
	hlo2->addWidget(btnScan);
	connect(btnScan, &QAbstractButton::clicked, this, &MainWidget::SlotScan);

	auto btnReplace = new QPushButton("Replace");
	hlo2->addWidget(btnReplace);
	connect(btnReplace, &QAbstractButton::clicked, this, &MainWidget::SlotReplace);

	auto btnNotes = new QPushButton("Notes");
	hlo2->addWidget(btnNotes);
	connect(btnNotes, &QAbstractButton::clicked, this, [this](){
		auto res = MyQDialogs::InputText("Notes", notesContent);
		if(res.acceptedAndChanged) notesContent = res.text;
	});

	auto btnLogs = new QPushButton("Logs");
	hlo2->addWidget(btnLogs);
	connect(btnLogs, &QAbstractButton::clicked, this, &MainWidget::OpenLogsDir);

	InitErrorsShowing(hlo2);

	hlo2->addStretch();
}

void MainWidget::SaveSettings()
{
	QSettings qsettings(settingsFile, QSettings::IniFormat);
	auto settings = GetSettingsObjectsList();
	for(auto &setting:settings)
	{
		qsettings.setValue(setting.name, setting.VarToStr());
	}
}

void MainWidget::LoadSettings()
{
	QSettings qsettings(settingsFile, QSettings::IniFormat);
	auto settings = GetSettingsObjectsList();
	for(auto &setting:settings)
	{
		if(qsettings.contains(setting.name))
			setting.VarFromStr(qsettings.value(setting.name).toString());
	}
}

std::vector<setting> MainWidget::GetSettingsObjectsList()
{
	std::vector<setting> settings;
	settings.emplace_back(setting{"textEditDirsContent", QTextEditState(textEditDirs)});
	settings.emplace_back(setting{"replaceAllEntries", QRadioGroupState({radioReplaceAll, radioReplaceFirst})});
	settings.emplace_back(setting{"geometry", QWidgetGeometry(this)});
	settings.emplace_back(setting{"splitterState", QSplitterState(splitter)});
	settings.emplace_back(setting{"includeSubcats", QCheckBoxState(checkBoxIncludeSubcats)});
	settings.emplace_back(setting{"from", QLineEditState(leFrom)});
	settings.emplace_back(setting{"to", QLineEditState(leTo)});
	settings.emplace_back(setting{"comboFromMode", QComboBoxState(comboFromMode)});
	settings.emplace_back(setting{"comboShowAllOrFound", QComboBoxState(widgetTable->comboShowAllOrFound)});
	settings.emplace_back(setting{"notesContent", &notesContent});
	return settings;
}

FromMode MainWidget::GetFromMode()
{
	FromMode mode = comboFromModeVals[comboFromMode->currentText()];
	if(mode == FromMode::undefined)
	{
		qdbg << "MainWidget::GetFromMode undefined from mode; set other";
		mode = FromMode::caseSensitive;
	}
	return mode;
}

void MainWidget::InitErrorsShowing(QHBoxLayout *hlo)
{
	static bool inited = false;
	if(not inited)
	{
		QPushButton *btn = new QPushButton();
		hlo->addWidget(btn);
		connect(btn, &QPushButton::clicked, this, [this](){
			auto dumpErrors = errors;
			errors.clear();
			MyQDialogs::ShowText(dumpErrors);
		});

		QTimer *timer = new QTimer(this);
		timer->start(100);
		connect(timer, &QTimer::timeout, this, [this, btn](){
			//static int i=0;
			//errors += QSn(i++);

			if(errors.isEmpty())
			{
				btn->setEnabled(false);
				btn->setText("No errors");
				btn->hide();
				return;
			}

			btn->setEnabled(true);
			btn->setText(" Show "+QSn(errors.size())+" errors ");
			btn->show();
		});
	}
}

void MainWidget::RefreshView()
{
	auto settings = ReplaceSettingsGet();
	for(auto &rep : preparedReplacesAll)
	{
		ReplaceRow &replace = rep;
		replace = PrepareReplaceForRow(replace.from, settings);
	}
	widgetTable->FillTable();
}

void MainWidget::SlotScan()
{
	preparedReplacesAll.clear();

	auto rows = textEditDirs->toPlainText().split('\n');
	for(auto &row : rows)
	{
		if(row.endsWith('\r')) row.chop(1);
		while(row.endsWith(' ')) row.chop(1);
		while(row.startsWith(' ')) row.remove(0, 1);
	}
	auto removeRes = std::remove_if(rows.begin(), rows.end(), [](const QString &row){ return row.isEmpty(); });
	rows.erase(removeRes, rows.end());

	if(rows.isEmpty()) { QMbError("Empty dirs"); return; }

	auto settings = ReplaceSettingsGet();
	if(not settings.error.isEmpty()) { QMbError(settings.error); return; }

	QStringList errors;
	for(auto &row:rows)
	{
		if(not QFileInfo(row).isDir()) { errors += row+" is not dir"; continue; }

		auto flags = QDirIterator::NoIteratorFlags;
		if(checkBoxIncludeSubcats->isChecked()) flags = QDirIterator::Subdirectories;
		QDirIterator dirIt(row, QDir::Files | QDir::NoDotAndDotDot, flags);
		while(dirIt.hasNext())
		{
			QString row = dirIt.next();
			auto &newRep = preparedReplacesAll.emplace_back();
			newRep.from = row;
		}
	}

	RefreshView();

	if(not errors.isEmpty())
	{
		MyQDialogs::ShowText("Errors while scan:\n"+errors.join('\n'));
	}
}

void MainWidget::SlotReplace()
{
	ReplaceSettings regStgs = ReplaceSettingsGet();
	if(not regStgs.error.isEmpty()) { QMbError(regStgs.error); return; }

	if(regStgs.from.isEmpty()) { QMbError("Empty from value"); return; }

	std::vector<ReplaceRow*> replacesToWork;
	for(auto &rep:preparedReplacesAll)
	{
		if(rep.HasMatches() and rep.enabled) replacesToWork.push_back(&rep);
	}
	if(replacesToWork.empty()) { QMbInfo("Nothing to replace"); return; }

	QProgressDialog progressDialog("Renaming files...", QString(), 0, 100, this);
	progressDialog.setWindowModality(Qt::ApplicationModal);
	progressDialog.setMinimumDuration(0);
	progressDialog.setAutoClose(false);
	progressDialog.setAutoReset(false);
	progressDialog.setCancelButton(nullptr);
	progressDialog.setValue(0);
	progressDialog.show();

	QStringList errors;
	QStringList logs;

	QEventLoop waitLoop;
	QStringList workerErrors;
	QStringList workerLogs;

	renameThread.stopper = false;
	bool started = renameThread.start([this, &replacesToWork, &workerErrors, &workerLogs, &progressDialog, &waitLoop]() mutable {
		any_guard::functions_caller waitLoopQuitGuard(
			{},
			[this, &waitLoop](){
				QMetaObject::invokeMethod(this, [&waitLoop](){
					waitLoop.quit();
				}, Qt::QueuedConnection);
			}
		);

		int done = 0;
		int lastSentPercent = -1;
		const int total = replacesToWork.size();

		for(auto &repPtr : replacesToWork)
		{
			auto &rep = *repPtr;
			auto renameError = MyQFileDir::Rename(rep.from, rep.to, true);
			if(renameError.isEmpty())
				workerLogs += "success, was renamed: " + rep.from + " -> " + rep.to;
			else
			{
				QString errorText = "error, was not renamed: " + rep.from + " -> " + rep.to + "\n" + renameError;
				workerErrors += errorText;
				workerLogs += errorText;
			}

			done++;
			int percent = done * 100 / total;
			if(percent != lastSentPercent)
			{
				lastSentPercent = percent;
				QMetaObject::invokeMethod(&progressDialog, [&progressDialog, percent](){
					progressDialog.setValue(percent);
				}, Qt::QueuedConnection);
			}
		}
	});

	if(not started)
	{
		QMbError("Rename thread was not started");
		return;
	}
	else
	{
		waitLoop.exec();
		renameThread.finish(10);
	}

	progressDialog.setValue(100);

	if(!workerErrors.isEmpty()) workerErrors.prepend("-------------------\nerrors in thread:");
	if(!workerLogs.isEmpty())    workerLogs.prepend("-------------------\nlogs in thread:");
	errors += workerErrors;
	logs += workerLogs;

	SaveLogs(logs, errors);

	if(not errors.isEmpty())
	{
		auto answ = QMessageBox::question({}, "Rename finished", "Show errors log?");
		if(answ == QMessageBox::Yes)
		{
			QStringList textToShow;
			textToShow += BuildLogsText(logs, errors);
			MyQDialogs::ShowText(textToShow.join("\n\n"));
		}
	}
}

ReplaceSettings MainWidget::ReplaceSettingsGet()
{
	ReplaceSettings settings {
		"",
		leFrom->text(),
				leTo->text(),
				GetFromMode(),
				radioReplaceAll->isChecked(),
				QRegularExpression(leFrom->text())
	};

	if(settings.fromMode == FromMode::regExpr and not settings.fromRegExpr.isValid())
		settings.error = settings.fromRegExpr.errorString();

	return settings;
}

QString MainWidget::BuildLogsText(const QStringList &logs, const QStringList &errors) const
{
	QStringList lines;
	lines += "DateTime: " + QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss.zzz");
	lines += "Logs:";
	if(logs.isEmpty()) lines += "<empty>";
	else lines += logs;
	lines += "";
	lines += "Errors:";
	if(errors.isEmpty()) lines += "<empty>";
	else lines += errors;
	return lines.join('\n');
}

QString MainWidget::LogsDirPath() const
{
	return MyQDifferent::PathToExe() + "/files/logs";
}

void MainWidget::SaveLogs(const QStringList &logs, const QStringList &errors) const
{
	QString logsDir = LogsDirPath();
	if(not QDir().mkpath(logsDir)) { QMbError("SaveLogs can't create dir "+logsDir); return; }

	auto removeOldFilesError = MyQFileDir::RemoveOldFiles(logsDir, 29);
	if(not removeOldFilesError.isEmpty()) QMbError("SaveLogs RemoveOldFiles error: " + removeOldFilesError);

	QString logFile = logsDir + "/" + QDateTime::currentDateTime().toString("yyyy.MM.dd_hh-mm-ss.zzz") + ".txt";
	if(not MyQFileDir::WriteFile(logFile, BuildLogsText(logs, errors)))
		QMbError("SaveLogs WriteFile error in " + logFile);
}

void MainWidget::OpenLogsDir()
{
	QString logsDir = LogsDirPath();
	if(not QDir().mkpath(logsDir))
	{
		QMbError("Can't create logs dir:\n" + logsDir);
		return;
	}

	if(not MyQExecute::OpenDir(logsDir))
	{
		QMbError("Can't open logs dir:\n" + logsDir);
	}
}

ReplaceRow MainWidget::PrepareReplaceForRow(const QString & row, const ReplaceSettings & replaceSettings)
{
	QFileInfo fi(row);
	ReplaceRow replace;
	replace.from = fi.filePath();
	QString fileNameNoPath = fi.fileName();
	QString path = fi.path();
	if(not fi.isFile())
	{
		errors.append("is not file");
		return replace;
	}

	if(replaceSettings.from.isEmpty())
	{
		replace.to = replace.from;
		return replace;
	}

	if(replaceSettings.fromMode != FromMode::regExpr)
	{
		auto caseMode = replaceSettings.fromMode == FromMode::caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive;
		int foundIndex = fileNameNoPath.indexOf(replaceSettings.from, 0, caseMode);

		while (foundIndex != -1) {
			replace.matches.emplace_back(OneMatch(Index(
				foundIndex,
				path.size() + 1 + foundIndex,
				replaceSettings.from.length()
			)));

			if(not replaceSettings.replaceAllEntries) break;
			// если включен replaceAllEntries выходим после первого совпадения

			foundIndex = fileNameNoPath.indexOf(replaceSettings.from, foundIndex + replaceSettings.from.length(), caseMode);
		}
	}
	else
	{
		auto matchIt = replaceSettings.fromRegExpr.globalMatch(fileNameNoPath);
		while(matchIt.hasNext())
		{
			QRegularExpressionMatch match = matchIt.next();
			if(match.hasMatch())
			{
				replace.matches.emplace_back(OneMatch(Index(
					match.capturedStart(0),
					path.size() + 1 + match.capturedStart(0),
					match.capturedLength(0)
				)));
			}
			if(not replaceSettings.replaceAllEntries)
				break;
		}
	}

	if(replace.HasMatches())
	{
		for(const auto &match: replace.matches)
		{
			bool regExprMode = replaceSettings.fromMode == FromMode::regExpr;
			if((not regExprMode and match.indexInSrc.length <= 0)
					or (regExprMode and match.indexInSrc.length < 0))
			{
				errors += "replace length = "+QSn(match.indexInSrc.length);
				return replace;
			}
		}

		QString newFileName = fileNameNoPath;
		int lenDiff = 0;
		for(auto &match : replace.matches)
		{
			int foundIndexInResult = match.indexInSrc.startIndex + lenDiff;
			match.indexInResult = Index(
				foundIndexInResult,
				path.size() + 1 + foundIndexInResult,
				replaceSettings.to.size()
			);
			newFileName.replace(foundIndexInResult, match.indexInSrc.length, replaceSettings.to);
			lenDiff += replaceSettings.to.size() - match.indexInSrc.length;
		}
		replace.to = fi.path() + "/" + newFileName;
	}

	return replace;
}




