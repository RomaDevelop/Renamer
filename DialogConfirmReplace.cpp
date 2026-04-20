#include "DialogConfirmReplace.h"

#include "MainWidget.h"

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

namespace
{
struct PreviewText
{
	QString text;
	std::vector<ReplaceMatch> matches;
};

PreviewText TrimPreviewText(const QString &text, const std::vector<ReplaceMatch> &matches, int trimStartPercent)
{
	PreviewText result;
	int trimCount = text.size() * trimStartPercent / 100;
	result.text = text.mid(trimCount);

	for(const auto &match : matches)
	{
		int matchStart = match.foundIndexInNameWithPath;
		if(match.lengthToReplace == 0)
		{
			if(matchStart < trimCount) continue;

			result.matches.emplace_back(ReplaceMatch{
				matchStart - trimCount,
				matchStart - trimCount,
				0
			});
			continue;
		}

		int matchEnd = matchStart + match.lengthToReplace;
		if(matchEnd <= trimCount) continue;

		int visibleStart = std::max(matchStart, trimCount);
		int visibleEnd = std::min(matchEnd, text.size());
		if(visibleEnd <= visibleStart) continue;

		result.matches.emplace_back(ReplaceMatch{
			visibleStart - trimCount,
			visibleStart - trimCount,
			visibleEnd - visibleStart
		});
	}

	return result;
}

QString HighlightedText(const QString &text, const std::vector<ReplaceMatch> &matches)
{
	auto toHtmlEscaped = [](const QString &value){ return value.toHtmlEscaped().replace('\n', "<br>"); };

	QString html;
	int currentIndex = 0;
	for(const auto &match : matches)
	{
		if(match.foundIndex < currentIndex) continue;

		int safeStart = std::clamp(match.foundIndex, 0, text.size());
		int safeLength = std::clamp(match.lengthToReplace, 0, text.size() - safeStart);

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

DialogConfirmReplace::DialogConfirmReplace(std::vector<Replace> &replacesRef, QWidget *parent)
	: QDialog(parent),
	  replaces(replacesRef),
	  timerPreviewUpdate(new QTimer(this))
{
	setWindowTitle("Подтверждение замены");
	resize(1200, 700);
	timerPreviewUpdate->setInterval(15);

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
	connect(btnInvertSelection, &QPushButton::clicked, this, &DialogConfirmReplace::InvertRowsChecked);

	hloTop->addSpacing(16);
	hloTop->addWidget(new QLabel("Скрыть начало:"));
	sliderTrimStartPercent = new QSlider(Qt::Horizontal);
	sliderTrimStartPercent->setRange(0, 100);
	sliderTrimStartPercent->setValue(0);
	sliderTrimStartPercent->setFixedWidth(170);
	hloTop->addWidget(sliderTrimStartPercent);
	connect(sliderTrimStartPercent, &QSlider::valueChanged, this, [this](){ UpdatePreviewTexts(); });

	hloTop->addStretch();

	hloTop->addWidget(new QLabel("Размер предпросмотра:"));
	sliderFontSize = new QSlider(Qt::Horizontal);
	sliderFontSize->setRange(8, 20);
	sliderFontSize->setValue(font().pointSize() > 0 ? font().pointSize() : 10);
	sliderFontSize->setFixedWidth(160);
	hloTop->addWidget(sliderFontSize);
	connect(sliderFontSize, &QSlider::valueChanged, this, [this](){ UpdatePreviewFont(); });

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

	auto hloBottom = new QHBoxLayout;
	vloMain->addLayout(hloBottom);
	hloBottom->addStretch();

	btnRun = new QPushButton("Запустить замену");
	btnRun->setDefault(true);
	hloBottom->addWidget(btnRun);
	connect(btnRun, &QPushButton::clicked, this, [this](){
		for(int row = 0; row < table->rowCount(); row++)
		{
			auto item = table->item(row, 0);
			replaces[row].enabled = item and item->checkState() == Qt::Checked;
		}
		accept();
	});

	auto btnCancel = new QPushButton("Отмена");
	hloBottom->addWidget(btnCancel);
	connect(btnCancel, &QPushButton::clicked, this, &DialogConfirmReplace::reject);

	FillTable();
	UpdatePreviewTexts();
	UpdatePreviewFont();
	UpdateRunButtonState();

	connect(timerPreviewUpdate, &QTimer::timeout, this, &DialogConfirmReplace::ProcessPreviewUpdateChunk);
	connect(table->verticalScrollBar(), &QScrollBar::valueChanged, this, [this](){
		if(timerPreviewUpdate->isActive())
			RebuildPreviewUpdateQueues();
	});
}

bool DialogConfirmReplace::Confirm(std::vector<Replace> &replaces, QWidget *parent)
{
	DialogConfirmReplace dialog(replaces, parent);
	return dialog.exec() == QDialog::Accepted;
}

void DialogConfirmReplace::FillTable()
{
	table->setRowCount(static_cast<int>(replaces.size()));
	currentValueLabels.resize(replaces.size());
	newValueLabels.resize(replaces.size());

	for(int row = 0; row < table->rowCount(); row++)
	{
		auto &replace = replaces[row];

		auto itemEnabled = new QTableWidgetItem;
		itemEnabled->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		itemEnabled->setCheckState(replace.enabled ? Qt::Checked : Qt::Unchecked);
		table->setItem(row, 0, itemEnabled);

		currentValueLabels[row] = CreatePreviewLabel();
		newValueLabels[row] = CreatePreviewLabel();
		table->setCellWidget(row, 1, currentValueLabels[row]);
		table->setCellWidget(row, 2, newValueLabels[row]);
	}

	connect(table, &QTableWidget::itemChanged, this, [this](QTableWidgetItem *item){
		if(item and item->column() == 0)
			UpdateRunButtonState();
	});
}

void DialogConfirmReplace::UpdatePreviewTexts()
{
	trimStartPercentPending = sliderTrimStartPercent->value();
	RebuildPreviewUpdateQueues();
	if(not timerPreviewUpdate->isActive())
		timerPreviewUpdate->start();
}

void DialogConfirmReplace::RebuildPreviewUpdateQueues()
{
	rowsInViewportCached = MyQTableWidget::RowsInViewPort(table);
	rowsPendingVisible.assign(rowsInViewportCached.begin(), rowsInViewportCached.end());
	rowsPendingOther.clear();
	rowsPendingOther.reserve(table->rowCount());

	for(int row = 0; row < table->rowCount(); row++)
	{
		if(rowsInViewportCached.count(row) == 0)
			rowsPendingOther.emplace_back(row);
	}

	rowsInViewportProcessed = rowsPendingVisible.empty();
}

void DialogConfirmReplace::ProcessPreviewUpdateChunk()
{
	const int rowsPerTick = 2;

	for(int processed = 0; processed < rowsPerTick; processed++)
	{
		if(not rowsInViewportProcessed)
		{
			if(rowsPendingVisible.empty())
			{
				rowsInViewportProcessed = true;
				continue;
			}

			UpdatePreviewRow(rowsPendingVisible.front());
			rowsPendingVisible.erase(rowsPendingVisible.begin());
			if(rowsPendingVisible.empty())
				rowsInViewportProcessed = true;
			continue;
		}

		if(rowsPendingOther.empty())
		{
			timerPreviewUpdate->stop();
			return;
		}

		UpdatePreviewRow(rowsPendingOther.front());
		rowsPendingOther.erase(rowsPendingOther.begin());
	}
}

void DialogConfirmReplace::UpdatePreviewRow(int row)
{
	const auto &replace = replaces[row];
	auto currentPreview = TrimPreviewText(replace.from, replace.matches, trimStartPercentPending);
	auto newPreview = TrimPreviewText(replace.to, replace.matchesInResult, trimStartPercentPending);

	currentValueLabels[row]->setText(HighlightedText(currentPreview.text, currentPreview.matches));
	newValueLabels[row]->setText(HighlightedText(newPreview.text, newPreview.matches));
}

void DialogConfirmReplace::UpdatePreviewFont()
{
	QFont previewFont = table->font();
	previewFont.setPointSize(sliderFontSize->value());

	int rowHeight = std::max(26, sliderFontSize->value() * 2 + 8);
	for(int row = 0; row < table->rowCount(); row++)
	{
		table->setRowHeight(row, rowHeight);

		for(int col = 1; col <= 2; col++)
		{
			if(auto label = qobject_cast<QLabel*>(table->cellWidget(row, col)))
				label->setFont(previewFont);
		}
	}
}

void DialogConfirmReplace::UpdateRunButtonState()
{
	for(int row = 0; row < table->rowCount(); row++)
	{
		if(auto item = table->item(row, 0); item and item->checkState() == Qt::Checked)
		{
			btnRun->setEnabled(true);
			return;
		}
	}

	btnRun->setEnabled(false);
}

void DialogConfirmReplace::SetAllRowsChecked(Qt::CheckState state)
{
	QSignalBlocker blocker(table);
	for(int row = 0; row < table->rowCount(); row++)
	{
		if(auto item = table->item(row, 0))
			item->setCheckState(state);
	}
	UpdateRunButtonState();
}

void DialogConfirmReplace::InvertRowsChecked()
{
	QSignalBlocker blocker(table);
	for(int row = 0; row < table->rowCount(); row++)
	{
		if(auto item = table->item(row, 0))
			item->setCheckState(item->checkState() == Qt::Checked ? Qt::Unchecked : Qt::Checked);
	}
	UpdateRunButtonState();
}
