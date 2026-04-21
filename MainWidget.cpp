#include "MainWidget.h"

#include "DialogConfirmReplace.h"

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

MainWidget::MainWidget(QWidget *parent)
	: QWidget(parent)
{
	textEditDirs->setLineWrapMode(QTextEdit::NoWrap);
	textEditFindRes->setLineWrapMode(QTextEdit::NoWrap);
	comboFindResView->addItem("Показывать только изменяемые строки");
	comboFindResView->addItem("Показывать все строки");

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
	vloLeft->addWidget(checkBoxRegExprInFrom);
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
	vloRight->addWidget(comboFindResView);
	vloRight->addWidget(textEditFindRes);

	CreateBottomRow(vloMain);

	connect(radioReplaceFirst, &QAbstractButton::toggled, this, [this](){ RefreshFindResView(); });
	connect(radioReplaceAll, &QAbstractButton::toggled, this, [this](){ RefreshFindResView(); });
	connect(checkBoxRegExprInFrom, &QAbstractButton::toggled, this, [this](){ RefreshFindResView(); });
	connect(leFrom, &QLineEdit::textChanged, this, [this](){ RefreshFindResView(); });
	connect(comboFindResView, qOverload<int>(&QComboBox::currentIndexChanged), this, [this](){ RefreshFindResView(); });

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

	hlo2->addStretch();
}

void MainWidget::SaveSettings()
{
	replaceAllEntries = radioReplaceAll->isChecked();
	QSettings qsettings(settingsFile, QSettings::IniFormat);
	auto settings = GetSettings();
	for(auto &setting:settings)
	{
		qsettings.setValue(setting.name, setting.VarToStr());
	}
}

void MainWidget::LoadSettings()
{
	QSettings qsettings(settingsFile, QSettings::IniFormat);
	auto settings = GetSettings();
	for(auto &setting:settings)
	{
		setting.VarFromStr(qsettings.value(setting.name).toString());
	}
	radioReplaceAll->setChecked(replaceAllEntries);
	radioReplaceFirst->setChecked(not replaceAllEntries);
}

std::vector<setting> MainWidget::GetSettings()
{
	std::vector<setting> settings;
	settings.emplace_back(setting{"notesContent", &notesContent});
	settings.emplace_back(setting{"replaceAllEntries", &replaceAllEntries});
	settings.emplace_back(setting{"geometry", QWidgetGeometry(this)});
	settings.emplace_back(setting{"splitterState", QSplitterState(splitter)});
	return settings;
}

void MainWidget::ClearFindResHighlight()
{
	QTextCursor cursor(textEditFindRes->document());
	cursor.select(QTextCursor::Document);
	QTextCharFormat format;
	format.setBackground(Qt::transparent);
	cursor.setCharFormat(format);
}

std::vector<std::pair<int, int>> MainWidget::GetHighlightRanges(const QStringList & rows, const ReplaceSettings & replaceSettings, bool * showInfoForAdd)
{
	if(showInfoForAdd) *showInfoForAdd = false;

	std::vector<std::pair<int, int>> ranges;
	if(replaceSettings.from.isEmpty() or not replaceSettings.error.isEmpty()) return ranges;

	int rowStart = 0;
	for(const auto &row: rows)
	{
		auto replace = PrepareReplaceForRow(row, replaceSettings);
		if(replace.error.isEmpty())
		{
			for(const auto &match: replace.matches)
			{
				int add = match.indexInSrc.length == 0 ? 1 : 0;
				if(showInfoForAdd and add != 0) *showInfoForAdd = true;
				ranges.emplace_back(rowStart + match.indexInSrc.startIndexInNameWithPath,
									rowStart + match.indexInSrc.startIndexInNameWithPath + match.indexInSrc.length + add);
			}
		}
		rowStart += row.size() + 1;
	}

	return ranges;
}

QStringList MainWidget::GetDisplayedFindResRows() const
{
	if(comboFindResView->currentIndex() != 0) return findResRowsAll;

	auto settings = ReplaceSettingsGet();
	QStringList rowsToShow;
	for(const auto &row : findResRowsAll)
	{
		auto replace = PrepareReplaceForRow(row, settings);
		if(replace.error.isEmpty() and replace.HasMatches())
			rowsToShow += row;
	}

	return rowsToShow;
}

void MainWidget::RefreshFindResView(bool *showInfoForAdd)
{
	textEditFindRes->clear();
	textEditFindRes->setCurrentCharFormat(QTextCharFormat());
	textEditFindRes->setPlainText(GetDisplayedFindResRows().join('\n'));

	bool localShowInfoForAdd = false;
	if(not showInfoForAdd) showInfoForAdd = &localShowInfoForAdd;

	ClearFindResHighlight();

	auto settings = ReplaceSettingsGet();
	auto rows = GetRows(textEditFindRes);
	auto ranges = GetHighlightRanges(rows, settings, showInfoForAdd);
	if(not ranges.empty())
		MyQTextEdit::ColorizeBackground(textEditFindRes, ranges, Qt::yellow);
}

void MainWidget::SlotScan()
{
	findResRowsAll.clear();

	auto rows = GetRows(textEditDirs);

	if(rows.isEmpty()) { QMbError("Empty dirs"); return; }

	auto settings = ReplaceSettingsGet();
	if(not settings.error.isEmpty()) { QMbError(settings.error); return; }

	bool showInfoForAdd = false;
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
			findResRowsAll += row;
		}
	}

	RefreshFindResView(&showInfoForAdd);

	if(not errors.isEmpty())
	{
		MyQDialogs::ShowText("Errors while scan:\n"+errors.join('\n'));
	}

	if(showInfoForAdd)
	{
		QMbInfo("Длина заменяемого текста равна нулю. "
				"В строках выделена буква, перед котрой будет вставлен текст, сама буква заменена не будет!");
	}
}

void MainWidget::SlotReplace()
{
	ReplaceSettings regStgs = ReplaceSettingsGet();
	if(not regStgs.error.isEmpty()) { QMbError(regStgs.error); return; }

	if(regStgs.from.isEmpty()) { QMbError("Empty from value"); return; }

	QStringList errors;
	QStringList logs;

	auto rows = GetRows(textEditFindRes);

	if(rows.isEmpty()) { QMbError("Empty find res"); return; }

	std::vector<ReplaceRow> replaces;
	for(auto &row:rows)
	{
		ReplaceRow replace = PrepareReplaceForRow(row, regStgs);
		if(replace.error.isEmpty())
		{
			if(replace.HasMatches())
				replaces.emplace_back(std::move(replace));
			else logs += "doesn't contains from value, will not be renamed: " + row;
		}
		else
		{
			errors += replace.error + " in " + row;
			logs += "error, " + replace.error + " in " + row;
		}
	}

	if(not errors.isEmpty())
	{
		MyQDialogs::ShowText("Errors:\n"+errors.join('\n'));
		return;
	}

	if(replaces.empty()) { QMbInfo("Nothing to replace"); return; }

	if(not DialogConfirmReplace::Confirm(replaces, this)) return;

	std::vector<ReplaceRow> enabledReplaces;
	enabledReplaces.reserve(replaces.size());
	for(auto &rep:replaces)
	{
		if(not rep.enabled)
		{
			logs += "skipped by user: " + rep.from + " -> " + rep.to;
			continue;
		}
	}
	for(auto &rep:replaces)
		if(rep.enabled)
			enabledReplaces.emplace_back(rep);

	if(enabledReplaces.empty()) { QMbInfo("Nothing selected to replace"); return; }

	QProgressDialog progressDialog("Renaming files...", QString(), 0, 100, this);
	progressDialog.setWindowModality(Qt::ApplicationModal);
	progressDialog.setMinimumDuration(0);
	progressDialog.setAutoClose(false);
	progressDialog.setAutoReset(false);
	progressDialog.setCancelButton(nullptr);
	progressDialog.setValue(0);
	progressDialog.show();

	QEventLoop waitLoop;
	QStringList workerErrors;
	QStringList workerLogs;

	renameThread.stopper = false;
	bool started = renameThread.start([this, enabledReplaces, &workerErrors, &workerLogs, &progressDialog, &waitLoop]() mutable {
		int done = 0;
		int lastSentPercent = -1;
		const int total = enabledReplaces.size();

		for(auto &rep : enabledReplaces)
		{
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

		QMetaObject::invokeMethod(this, [&waitLoop](){
			waitLoop.quit();
		}, Qt::QueuedConnection);
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

ReplaceRow MainWidget::PrepareReplaceForRow(const QString & row, const ReplaceSettings & replaceSettings) const
{
	ReplaceRow replace;
	QFileInfo fi(row);
	QString fileNameNoPath = fi.fileName();
	QString path = fi.path();
	if(not fi.isFile())
	{
		replace.error = "is not file";
		return replace;
	}

	if(not replaceSettings.fromRegExprEnabled)
	{
		int foundIndex = fileNameNoPath.indexOf(replaceSettings.from);
		if(foundIndex != -1)
		{
			replace.matches.emplace_back(OneMatch(Index(
				foundIndex,
				path.size() + 1 + foundIndex,
				replaceSettings.from.length()
			)));
		}

		if(replaceSettings.replaceAllEntries and not replaceSettings.from.isEmpty())
		{
			foundIndex = fileNameNoPath.indexOf(replaceSettings.from, foundIndex + replaceSettings.from.length());
			while(foundIndex != -1)
			{
				replace.matches.emplace_back(OneMatch(Index(
					foundIndex,
					path.size() + 1 + foundIndex,
					replaceSettings.from.length()
				)));
				foundIndex = fileNameNoPath.indexOf(replaceSettings.from, foundIndex + replaceSettings.from.length());
			}
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
			if((not replaceSettings.fromRegExprEnabled and match.indexInSrc.length <= 0)
					or (replaceSettings.fromRegExprEnabled and match.indexInSrc.length < 0))
			{
				replace.error = "replace length = "+QSn(match.indexInSrc.length);
				return replace;
			}
		}

		replace.from = fi.filePath();
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

QStringList MainWidget::GetRows(QTextEdit * textEdit)
{
	auto text = textEdit->toPlainText();
	auto rows = text.split('\n');

	for(auto &row:rows)
	{
		if(row.endsWith('\r')) row.chop(1);
		while(row.endsWith(' ')) row.chop(1);
		while(row.startsWith(' ')) row.remove(0,1);
	}

	auto removeRes = std::remove_if(rows.begin(), rows.end(), [](const QString &row){ return row.isEmpty(); });
	rows.erase(removeRes, rows.end());

	return rows;
}

QByteArray ByteArrFromStr(const QString &str) { return QByteArray::fromBase64(str.toLatin1()); }
QString ByteArrToStr(const QByteArray &byteArr) {  return QString::fromLatin1(byteArr.toBase64()); }

void setting::VarFromStr(const QString &str)
{
	struct var_from_str {
		var_from_str(const QString &str): str{str} {}
		void operator()(QString *strPtr) { *strPtr = str; }
		void operator()(QByteArray *byteArr) { *byteArr = ByteArrFromStr(str); }
		void operator()(bool *boolPtr) { *boolPtr = (str == "true" or str == "1"); }
		void operator()(QWidgetGeometry wGeo) { wGeo.widget->restoreGeometry(ByteArrFromStr(str)); }
		void operator()(QSplitterState splState) { splState.splitter->restoreState(ByteArrFromStr(str)); }
		QString str;
	};

	std::visit(var_from_str{str}, var);
}

QString setting::VarToStr()
{
	struct var_to_str {
		QString operator()(QString *strPtr) const { return *strPtr; }
		QString operator()(QByteArray *byteArr) { return ByteArrToStr(*byteArr); }
		QString operator()(bool *boolPtr) { return *boolPtr ? "true" : "false"; }
		QString operator()(QWidgetGeometry wGeo) { return ByteArrToStr(wGeo.widget->saveGeometry()); }
		QString operator()(QSplitterState splState) { return ByteArrToStr(splState.splitter->saveState()); }
	};

	return std::visit(var_to_str{}, var);
}
