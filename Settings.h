#ifndef SETTINGS_H
#define SETTINGS_H

#include <variant>

#include <QDebug>
#include <QByteArray>
#include <QCheckBox>
#include <QSplitter>
#include <QString>
#include <QRadioButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>

static QByteArray ByteArrFromStr(const QString &str) { return QByteArray::fromBase64(str.toLatin1()); }
static QString ByteArrToStr(const QByteArray &byteArr) {  return QString::fromLatin1(byteArr.toBase64()); }

struct QWidgetGeometry
{
	QWidget *obj;
	QWidgetGeometry(QWidget *obj): obj{obj} {}
	QString Save() { return ByteArrToStr(obj->saveGeometry()); }
	void Load(const QString &str) { obj->restoreGeometry(ByteArrFromStr(str)); }
};

struct QSplitterState
{
	QSplitter *obj;
	QSplitterState(QSplitter *obj): obj{obj} {}
	QString Save() { return ByteArrToStr(obj->saveState()); }
	void Load(const QString &str) { obj->restoreState(ByteArrFromStr(str)); }
};

struct QCheckBoxState
{
	QCheckBox *obj;
	QCheckBoxState(QCheckBox *obj): obj{obj} {}
	QString Save() { return obj->isChecked() ? "1" : "0"; }
	void Load(const QString &str) {
		if(str == "1") obj->setChecked(true);
		else if(str == "0") obj->setChecked(false);
		else qDebug() << "QCheckBoxState: invalid state ["+str+"]";
	}
};

struct QRadioGroupState
		// можно еще сделать пару и хранить её одно значение
		// и заготовки по кол-вам чтобы без вектора
{
	std::vector<QRadioButton *> objects;
	QRadioGroupState(std::vector<QRadioButton *> rbtns): objects{std::move(rbtns)} {}
	QString Save() {
		QString res;
		for(auto rbtn:objects) res.append(rbtn->isChecked() ? "1" : "0").append(',');
		res.chop(1);
		return res;
	}
	void Load(const QString &str) {
		auto list = str.split(',');
		if(list.size() == (int)objects.size())
		{
			for(int i=0; i<list.size(); i++)
				if(list[i] == "1") objects[i]->setChecked(true);
				else if(list[i] == "0") objects[i]->setChecked(false);
				else qDebug() << "QCheckBoxState: invalid state ["+list[i]+"]";
		}
		else qDebug() << "QRadioGroupState: invalid state ["+str+"]";
	}
};

struct QLineEditState
{
	QLineEdit *obj;
	QLineEditState(QLineEdit *obj): obj{obj} {}
	QString Save() { return obj->text(); }
	void Load(const QString &str) { obj->setText(str); }
};

struct QTextEditState
{
	QTextEdit *obj;
	QTextEditState(QTextEdit *obj): obj{obj} {}
	QString Save() { return obj->toPlainText(); }
	void Load(const QString &str) { obj->setPlainText(str); }
};

struct QComboBoxState
{
	QComboBox *obj;
	QComboBoxState(QComboBox *obj): obj{obj} {}
	QString Save() { return QString::number(obj->currentIndex()); }
	void Load(const QString &str) { obj->setCurrentIndex(str.toInt()); }
};

using var_setting = std::variant<QString*, QByteArray*, bool*,
		QWidgetGeometry, QSplitterState, QCheckBoxState, QRadioGroupState, QLineEditState, QTextEditState, QComboBoxState>;

struct var_from_str {
	var_from_str(const QString &str): str{str} {}
	void operator()(QString *strPtr) { *strPtr = str; }
	void operator()(QByteArray *byteArr) { *byteArr = ByteArrFromStr(str); }
	void operator()(bool *boolPtr) { *boolPtr = (str == "true" or str == "1"); }
	template<class SettingState>
	void operator()(SettingState state) { state.Load(str); }
	QString str;
};

struct var_to_str {
	template<typename T>
	QString operator()(T arg) const {
		if constexpr (std::is_same_v<T, QString*>) return *arg;
		else if constexpr (std::is_same_v<T, QByteArray*>) return ByteArrToStr(*arg);
		else if constexpr (std::is_same_v<T, bool*>) return *arg ? "true" : "false";
		else return arg.Save();
	}
};

struct setting
{
	QString name;
	var_setting var;

	void VarFromStr(const QString &str)
	{
		std::visit(var_from_str{str}, var);
	}

	QString VarToStr()
	{
		return std::visit(var_to_str{}, var);
	}
};

#endif // SETTINGS_H
