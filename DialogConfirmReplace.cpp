#include "DialogConfirmReplace.h"

#include <algorithm>

#include <QCheckBox>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSignalBlocker>
#include <QScrollBar>
#include <QSlider>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTextDocument>
#include <QTimer>
#include <QVBoxLayout>

#include "MyQShortings.h"
#include "MyQTableWidget.h"

#include "MainWidget.h"

namespace Cols
{
	const int checkBox = 0;
}

namespace
{
	struct PreviewText
	{
		QString text;
		std::vector<OneMatch> matches;
	};

	PreviewText TrimPreviewText(const QString &text, const std::vector<OneMatch> &matches, int trimStartPercent, bool useResultIndexes)
	{
		PreviewText result;
		int trimCount = text.size() * trimStartPercent / 100;
		result.text = text.mid(trimCount);

		for(const auto &match : matches)
		{
			const Index &index = useResultIndexes ? match.indexInResult : match.indexInSrc;
			int matchStart = index.startIndexInNameWithPath;
			if(index.length == 0)
			{
				if(matchStart < trimCount) continue;

				result.matches.emplace_back(OneMatch(Index(
														 matchStart - trimCount,
														 matchStart - trimCount,
														 0
														 )));
				continue;
			}

			int matchEnd = matchStart + index.length;
			if(matchEnd <= trimCount) continue;

			int visibleStart = std::max(matchStart, trimCount);
			int visibleEnd = std::min(matchEnd, text.size());
			if(visibleEnd <= visibleStart) continue;

			result.matches.emplace_back(OneMatch(Index(
													 visibleStart - trimCount,
													 visibleStart - trimCount,
													 visibleEnd - visibleStart
													 )));
		}

		return result;
	}

	QString HighlightedText(const QString &text, const std::vector<OneMatch> &matches)
	{
		auto toHtmlEscaped = [](const QString &value){ return value.toHtmlEscaped().replace('\n', "<br>"); };

		QString html;
		int currentIndex = 0;
		for(const auto &match : matches)
		{
			const Index &index = match.indexInSrc;
			if(index.startIndex < currentIndex) continue;

			int safeStart = std::clamp(index.startIndex, 0, text.size());
			int safeLength = std::clamp(index.length, 0, text.size() - safeStart);

			html += toHtmlEscaped(text.mid(currentIndex, safeStart - currentIndex));
			html += "<span style=\"background-color:#fff59d;\">";

			if(safeLength == 0)
				html += "&nbsp;";
			else
				html += toHtmlEscaped(text.mid(safeStart, safeLength));

			html += "</span>";
			currentIndex = safeStart + safeLength;
		}

		html += toHtmlEscaped(text.mid(currentIndex));
		return html;
	}

	QLabel *CreatePreviewLabel()
	{
		auto label = new QLabel;
		label->setTextFormat(Qt::RichText);
		label->setTextInteractionFlags(Qt::TextSelectableByMouse);
		label->setWordWrap(false);
		label->setMargin(4);
		return label;
	}
}

WidgetTable::WidgetTable(std::vector<ReplaceRow> &replacesRef, QWidget *parent)
	: QWidget(parent),
	  replaces(replacesRef),
	  rowsUpdater(this)
{
	//qdbg << 1;
	setWindowTitle("Подтверждение замены");
	resize(1200, 700);

	auto vloMain = new QVBoxLayout(this);

	auto hloTop = new QHBoxLayout;
	vloMain->addLayout(hloTop);

	auto btnSelectAll = new QPushButton("Выбрать все");
	hloTop->addWidget(btnSelectAll);
	connect(btnSelectAll, &QPushButton::clicked, this, [this](){ SetAllRowsChecked(Qt::Checked); });

	auto btnClearSelection = new QPushButton("Снять выбор");
	hloTop->addWidget(btnClearSelection);
	connect(btnClearSelection, &QPushButton::clicked, this, [this](){ SetAllRowsChecked(Qt::Unchecked); });

	auto btnInvertSelection = new QPushButton("Инвертировать");
	hloTop->addWidget(btnInvertSelection);
	connect(btnInvertSelection, &QPushButton::clicked, this, &WidgetTable::InvertRowsChecked);

	hloTop->addSpacing(16);
	hloTop->addWidget(new QLabel("Скрыть начало:"));
	sliderTrimmer = new QSlider(Qt::Horizontal);
	sliderTrimmer->setRange(0, 100);
	sliderTrimmer->setValue(0);
	sliderTrimmer->setFixedWidth(170);
	hloTop->addWidget(sliderTrimmer);
	connect(sliderTrimmer, &QSlider::valueChanged, this, [this](){  SetRowsUpdaterArgs(); });

	hloTop->addStretch();

	table = new QTableWidget;
	table->setColumnCount(3);
	table->setHorizontalHeaderLabels({"Зам.", "Текущий путь", "Новый путь"});
	table->setSelectionBehavior(QAbstractItemView::SelectRows);
	table->setSelectionMode(QAbstractItemView::SingleSelection);
	table->setEditTriggers(QAbstractItemView::NoEditTriggers);
	table->verticalHeader()->setVisible(false);
	table->horizontalHeader()->setStretchLastSection(false);
	table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
	table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
	table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
	table->setColumnWidth(0, 54);
	vloMain->addWidget(table);

	FillTable();

	connect(table->verticalScrollBar(), &QScrollBar::valueChanged, this, [this](){
		SetRowsUpdaterArgs();
	});
	connect(table, &QTableWidget::itemChanged, this, [this](QTableWidgetItem *item) {
		if (item->column() == Cols::checkBox) {
			if(item->row() < 0 or item->row() >= (int)replaces.size())
				qdbg << "QTableWidget::itemChanged invalid row "<<item->row();
			auto &replace = replaces[item->row()];
			replace.enabled = item->checkState() == Qt::Checked;
		}
	});

	auto f = [this](int row)
	{
		SetRowTexts(row);
	};
	rowsUpdater.function = f;
}

void WidgetTable::FillTable()
{
	table->setRowCount(0);
	table->setRowCount(static_cast<int>(replaces.size()));
	currentValueLabels.resize(replaces.size());
	newValueLabels.resize(replaces.size());

	for(int row = 0; row < table->rowCount(); row++)
	{
		auto &replace = replaces[row];

		auto itemEnabled = new QTableWidgetItem;
		itemEnabled->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		itemEnabled->setCheckState(replace.enabled ? Qt::Checked : Qt::Unchecked);
		table->setItem(row, Cols::checkBox, itemEnabled);

		currentValueLabels[row] = CreatePreviewLabel();
		newValueLabels[row] = CreatePreviewLabel();
		table->setCellWidget(row, 1, currentValueLabels[row]);
		table->setCellWidget(row, 2, newValueLabels[row]);
	}

	SetRowsUpdaterArgs();
}

void WidgetTable::SetRowTexts(int row)
{
	const auto &replace = replaces[row];
	auto currentPreview = TrimPreviewText(replace.from, replace.matches, sliderTrimmer->value(), false);
	auto newPreview = TrimPreviewText(replace.to, replace.matches, sliderTrimmer->value(), true);

	currentValueLabels[row]->setText(HighlightedText(currentPreview.text, currentPreview.matches));
	newValueLabels[row]->setText(HighlightedText(newPreview.text, newPreview.matches));
}

void WidgetTable::SetAllRowsChecked(Qt::CheckState state)
{
	QSignalBlocker blocker(table);
	for(int row = 0; row < table->rowCount(); row++)
	{
		if(auto item = table->item(row, 0))
			item->setCheckState(state);
	}
}

void WidgetTable::InvertRowsChecked()
{
	QSignalBlocker blocker(table);
	for(int row = 0; row < table->rowCount(); row++)
	{
		if(auto item = table->item(row, 0))
			item->setCheckState(item->checkState() == Qt::Checked ? Qt::Unchecked : Qt::Checked);
	}
}

void WidgetTable::SetRowsUpdaterArgs()
{
	auto rowsInViewPort = MyQTableWidget::RowsInViewPort(table);

	for(auto row:rowsInViewPort)
		rowsUpdater.args.push(row);

	for(int row=0; row<table->rowCount(); row++)
		if(rowsInViewPort.count(row) == 0) rowsUpdater.args.push(row);
}
