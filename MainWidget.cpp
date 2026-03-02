#include "MainWidget.h"

#include <set>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDirIterator>
#include <QMetaObject>
#include <QDebug>
#include <QMessageBox>
#include <QElapsedTimer>
#include <QFile>
#include <QLabel>
#include <QSplitter>
#include <QSettings>

#include "MyQShortings.h"
#include "MyQFileDir.h"
#include "MyQDialogs.h"
#include "MyQTextEdit.h"

MainWidget::MainWidget(QWidget *parent)
	: QWidget(parent)
{
	leFilter->setDisabled(true);
	textEditDirs->setLineWrapMode(QTextEdit::NoWrap);
	textEditFindRes->setLineWrapMode(QTextEdit::NoWrap);

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
	vloLeft->addWidget(new QLabel("Filter:"));
	vloLeft->addWidget(leFilter);
	vloLeft->addWidget(checkBoxIncludeSubcats);
	vloLeft->addWidget(checkBoxRegExprInFrom);
	vloLeft->addWidget(new QLabel("Replace from:"));
	vloLeft->addWidget(leFrom);
	vloLeft->addWidget(new QLabel("Replace to:"));
	vloLeft->addWidget(leTo);
	vloLeft->addStretch();

	splitter->addWidget(textEditFindRes);

	CreateBottomRow(vloMain);

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

	hlo2->addStretch();
}

void MainWidget::SaveSettings()
{
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
}

std::vector<setting> MainWidget::GetSettings()
{
	std::vector<setting> settings;
	settings.emplace_back(setting{"notesContent", &notesContent});
	settings.emplace_back(setting{"geometry", QWidgetGeometry(this)});
	settings.emplace_back(setting{"splitterState", QSplitterState(splitter)});
	return settings;
}

void MainWidget::SlotScan()
{
	textEditFindRes->clear();

	auto rows = GetRows(textEditDirs);

	if(rows.isEmpty()) { QMbError("Empty dirs"); return; }

	auto settings = ReplaceSettingsGet();

	std::vector<std::pair<int, int>> fromToForColorize;

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
			int countBeforeAdd = textEditFindRes->document()->characterCount();
			if(countBeforeAdd == 1) countBeforeAdd = 0; // пустой документ выдаёт 1
			textEditFindRes->append(row);

			if(not settings.from.isEmpty())
			{
				auto replace = PrepareReplaceForRow(row, settings);
				if(replace.foundIndex >= 0)
				{
					int index = countBeforeAdd + replace.foundIndexInNameWithPath;
					int add = replace.lengthToReplace == 0 ? 1 : 0;
					if(add != 0) showInfoForAdd = true;
					fromToForColorize.emplace_back(std::pair{index, index+replace.lengthToReplace+add});
				}
			}
		}
	}

	MyQTextEdit::ColorizeBackground(textEditFindRes, fromToForColorize, Qt::yellow);

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

	if(regStgs.from.isEmpty()) { QMbError("Empty from value"); return; }

	QStringList errors;
	QStringList logs;

	auto rows = GetRows(textEditFindRes);

	if(rows.isEmpty()) { QMbError("Empty find res"); return; }

	std::vector<Replace> replaces;
	for(auto &row:rows)
	{
		Replace replace = PrepareReplaceForRow(row, regStgs);
		if(replace.error.isEmpty())
		{
			if(replace.foundIndex >= 0)
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

	QStringList text {"Replaces will be (you can confirm or abort on next step):"};
	for(auto &rep:replaces)
	{
		text += rep.from + " -> " + rep.to;
	}
	MyQDialogs::ShowText(text);
	auto answ = QMessageBox::question({}, "Confirm replace", "Confirm replace?");
	if(answ == QMessageBox::No) return;

	for(auto &rep:replaces)
	{
		auto renameRes = QFile::rename(rep.from, rep.to);
		if(renameRes)
		{
			logs += "success, was renamed: " + rep.from + " -> " + rep.to;
		}
		else
		{
			errors += "error, was not renamed: " + rep.from + " -> " + rep.to;
		}
	}

	answ = QMessageBox::question({}, "Rename finished", "Show log?");
	if(answ == QMessageBox::Yes)
	{
		MyQDialogs::ShowText(logs);
	}
}

Replace MainWidget::PrepareReplaceForRow(const QString & row, const ReplaceSettings & replaceSettings)
{
	Replace replace;
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
		replace.foundIndex = fileNameNoPath.indexOf(replaceSettings.from);
		replace.foundIndexInNameWithPath = path.size() + 1 + replace.foundIndex;
		replace.lengthToReplace = replaceSettings.from.length();
	}
	else
	{
		QRegularExpressionMatch match = replaceSettings.fromRegExpr.match(fileNameNoPath);
		if (match.hasMatch()) {
			replace.foundIndex = match.capturedStart(0);
			replace.foundIndexInNameWithPath = path.size() + 1 + replace.foundIndex;
			replace.lengthToReplace = match.capturedLength(0);
		}
	}

	if(replace.foundIndex != -1)
	{
		if((not replaceSettings.fromRegExprEnabled and replace.lengthToReplace <= 0)
				or (replaceSettings.fromRegExprEnabled and replace.lengthToReplace < 0))
		{
			replace.error = "replace length = "+QSn(replace.lengthToReplace);
			return replace;
		}

		replace.from = fi.filePath();
		replace.to = fi.path() + "/" + fileNameNoPath.replace(replace.foundIndex, replace.lengthToReplace, replaceSettings.to);
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
		QString operator()(QWidgetGeometry wGeo) { return ByteArrToStr(wGeo.widget->saveGeometry()); }
		QString operator()(QSplitterState splState) { return ByteArrToStr(splState.splitter->saveState()); }
	};

	return std::visit(var_to_str{}, var);
}
