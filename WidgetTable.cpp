#include "WidgetTable.h"

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

	PreviewText TrimPreviewText(const QString &text, const std::vector<OneMatch> &matches, int trimStartPercent,
								bool useResultIndexes)
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
	setWindowTitle("Подтверждение замены");
	resize(1200, 700);

	auto vloMain = new QVBoxLayout(this);
	vloMain->setContentsMargins(0,0,0,0);

	auto hloTop = new QHBoxLayout;
	vloMain->addLayout(hloTop);

	auto btnSelectAll = new QPushButton("All");
	hloTop->addWidget(btnSelectAll);
	connect(btnSelectAll, &QPushButton::clicked, this, [this](){ SetAllRowsCheckState(1); });

	auto btnClearSelection = new QPushButton("Clear");
	hloTop->addWidget(btnClearSelection);
	connect(btnClearSelection, &QPushButton::clicked, this, [this](){ SetAllRowsCheckState(0); });

	auto btnInvertSelection = new QPushButton("Invert");
	hloTop->addWidget(btnInvertSelection);
	connect(btnInvertSelection, &QPushButton::clicked, this, [this](){ SetAllRowsCheckState(2); });

	hloTop->addStretch();

	hloTop->addWidget(new QLabel("Hide beginning:"));
	sliderTrimmer = new QSlider(Qt::Horizontal);
	sliderTrimmer->setRange(0, 100);
	sliderTrimmer->setValue(0);
	sliderTrimmer->setFixedWidth(170);
	hloTop->addWidget(sliderTrimmer);
	connect(sliderTrimmer, &QSlider::valueChanged, this, [this](){  SetRowsUpdaterArgs(); });

	hloTop->addStretch();

	hloTop->addWidget(comboShowAllOrFound);
	comboShowAllOrFound->addItem("Show only changable");
	comboShowAllOrFound->addItem("Show all");

	table = new QTableWidget;
	table->setColumnCount(3);
	table->setHorizontalHeaderLabels({"On", "Current value", "Value will be after replace"});
	table->setSelectionBehavior(QAbstractItemView::SelectRows);
	table->setSelectionMode(QAbstractItemView::SingleSelection);
	table->setEditTriggers(QAbstractItemView::NoEditTriggers);
	table->verticalHeader()->setVisible(false);
	table->horizontalHeader()->setStretchLastSection(false);
	table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
	table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
	table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
	table->setColumnWidth(0, 38);
	vloMain->addWidget(table);

	FillTable();

	SetRowsUpdaterFunction();
}

struct Data
{
	ReplaceRow *replace;
	QCheckBox* checkbox;
	QLabel *labelFrom;
	QLabel *labelTo;
};
Q_DECLARE_METATYPE(Data)

void WidgetTable::FillTable()
{
	table->setRowCount(0);

	int row = 0;
	for(auto &replace:replaces)
	{
		if(comboShowAllOrFound->currentIndex() == 0)
			if(not replace.HasMatches()) continue;

		table->setRowCount(row+1);

//		auto itemEnabled = new QTableWidgetItem;
//		itemEnabled->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
//		itemEnabled->setCheckState(replace.enabled ? Qt::Checked : Qt::Unchecked);
//		itemEnabled->setTextAlignment(Qt::AlignCenter);
//		table->setItem(row, Cols::checkBox, itemEnabled);

		QCheckBox* checkbox = new QCheckBox();
		checkbox->setChecked(replace.enabled);

		QWidget* container = new QWidget();
		QHBoxLayout* hbox = new QHBoxLayout(container);
		hbox->setContentsMargins(0, 0, 0, 0);
		hbox->addStretch();
		hbox->addWidget(checkbox);
		hbox->addStretch();

		table->setCellWidget(row, Cols::checkBox, container);
		connect(checkbox, &QCheckBox::stateChanged, checkbox, [&replace, checkbox](){
			replace.enabled = checkbox->isChecked();
		});

		auto labelFrom = CreatePreviewLabel();
		auto labelTo = CreatePreviewLabel();
		labelFrom->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
		labelTo->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
		table->setCellWidget(row, 1, labelFrom);
		table->setCellWidget(row, 2, labelTo);

		auto itemEnabled = new QTableWidgetItem;
		table->setItem(row, Cols::checkBox, itemEnabled);
		itemEnabled->setData(Qt::UserRole, QVariant::fromValue(Data{ &replace, checkbox, labelFrom, labelTo }));

		row++;
	}

	SetRowsUpdaterArgs();
}

void WidgetTable::SetRowTexts(int row)
{
	Data data = table->item(row, Cols::checkBox)->data(Qt::UserRole).value<Data>();
	const auto &replace = *data.replace;
	auto fromValue = TrimPreviewText(replace.from, replace.matches, sliderTrimmer->value(), false);
	auto toValue = TrimPreviewText(replace.to, replace.matches, sliderTrimmer->value(), true);

	data.labelFrom->setText(HighlightedText(fromValue.text, fromValue.matches));
	data.labelTo->setText(HighlightedText(toValue.text, toValue.matches));
}

void WidgetTable::SetAllRowsCheckState(uint val)
{
	for(int row = 0; row < table->rowCount(); row++)
	{
		if(auto item = table->item(row, 0))
		{
			auto data = item->data(Qt::UserRole).value<Data>();
			if(val == 2)
				data.checkbox->setChecked(!data.checkbox->isChecked());
			else data.checkbox->setChecked(val);
		}
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

void WidgetTable::SetRowsUpdaterFunction()
{
	auto f = [this](int row)
	{
		SetRowTexts(row);
	};
	rowsUpdater.function = f;
}
