#pragma once
#include "QLabel"
class CLabel : public QLabel
{
	Q_OBJECT
public:
	CLabel(QWidget* wid);
	~CLabel();

protected:
	bool eventFilter(QObject*, QEvent*);

private:
	QWidget* parent_;
};