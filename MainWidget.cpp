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

#include "MyQShortings.h"
#include "MyQFileDir.h"
#include "MyQDialogs.h"

MainWidget::MainWidget(QWidget *parent)
	: QWidget(parent)
{
	leFilter->setDisabled(true);
	textEditDirs->setLineWrapMode(QTextEdit::NoWrap);
	textEditFindRes->setLineWrapMode(QTextEdit::NoWrap);

	checkBoxIncludeSubcats->setChecked(true);

	auto vloMain = new QVBoxLayout(this);
	auto l1_splitter = new QSplitter(Qt::Horizontal);
	auto hlo2 = new QHBoxLayout;

	vloMain->addWidget(l1_splitter);
	vloMain->addLayout(hlo2);

	auto w_in_l1 = new QWidget;
	l1_splitter->addWidget(w_in_l1);
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

	l1_splitter->addWidget(textEditFindRes);

	auto btnScan = new QPushButton("Scan");
	connect(btnScan, &QAbstractButton::clicked, this, &MainWidget::SlotScan);

	auto btnReplace = new QPushButton("Replace");
	connect(btnReplace, &QAbstractButton::clicked, this, &MainWidget::SlotReplace);

	hlo2->addWidget(btnScan);
	hlo2->addWidget(btnReplace);
	hlo2->addStretch();
}

MainWidget::~MainWidget()
{

}

void MainWidget::SlotScan()
{
	textEditFindRes->clear();

	auto rows = GetRows(textEditDirs);

	if(rows.isEmpty()) { QMbError("Empty dirs"); return; }

	QStringList errors;
	for(auto &row:rows)
	{
		if(not QFileInfo(row).isDir()) { errors += row+" is not dir"; continue; }

		auto flags = QDirIterator::NoIteratorFlags;
		if(checkBoxIncludeSubcats->isChecked()) flags = QDirIterator::Subdirectories;
		QDirIterator dirIt(row, QDir::Files | QDir::NoDotAndDotDot, flags);
		while(dirIt.hasNext())
		{
			textEditFindRes->append(dirIt.next());
		}
	}

	if(not errors.isEmpty())
	{
		MyQDialogs::ShowText("Errors while scan:\n"+errors.join('\n'));
	}
}

void MainWidget::SlotReplace()
{
	auto from = leFrom->text();
	auto to = leTo->text();

	if(from.isEmpty()) { QMbError("Empty from value"); return; }

	QRegularExpression fromRe(from);
	bool useFromRe = checkBoxRegExprInFrom->isChecked();

	QStringList errors;
	QStringList logs;

	auto rows = GetRows(textEditFindRes);

	if(rows.isEmpty()) { QMbError("Empty find res"); return; }

	struct Replace { QString from; QString to; };

	std::vector<Replace> replaces;
	for(auto &row:rows)
	{
		QFileInfo fi(row);
		if(not fi.isFile())
		{
			errors += "is not file: " + row;
			logs += "error, is not file: "+row;
			continue;
		}

		int index = -1;
		int length = -1;

		if(not useFromRe)
		{
			index = fi.fileName().indexOf(from);
			length = from.length();
		}
		else
		{
			QRegularExpressionMatch match = fromRe.match(fi.fileName());
			if (match.hasMatch()) {
				index = match.capturedStart(0);
				length = match.capturedLength(0);
			}
		}

		if(index != -1)
		{
			if(length <= 0)
			{
				errors += "replace length <= 0 in "+row;
				logs += "error, replace length <= 0 in "+row;
				continue;
			}

			auto &rep = replaces.emplace_back();
			rep.from = fi.filePath();
			rep.to = fi.path() + "/" + fi.fileName().replace(index, length, to);
		}
		else logs += "doesn't contains from value, will not be renamed: " + row;
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
