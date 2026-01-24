#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <atomic>
#include <memory>
#include <thread>

#include <QMainWindow>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QCheckBox>

class MainWidget : public QWidget
{
	Q_OBJECT

public:
	MainWidget(QWidget *parent = nullptr);
	~MainWidget();

private:
	QTextEdit *textEditDirs = new QTextEdit;
	QLineEdit *leFilter = new QLineEdit;
	QCheckBox *checkBoxIncludeSubcats = new QCheckBox("Include subcats");
	QCheckBox *checkBoxRegExprInFrom = new QCheckBox("Reg. expr. in from");
	QLineEdit *leFrom = new QLineEdit;
	QLineEdit *leTo = new QLineEdit;

	QTextEdit *textEditFindRes = new QTextEdit;

	void SlotScan();
	void SlotReplace();

	static QStringList GetRows(QTextEdit *textEdit);
};

#endif // MAINWINDOW_H
