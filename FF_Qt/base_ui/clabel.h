#pragma once
#include "QWidget"
#include "QPointer"
class QLabel;
class QHBoxLayout;
class CLabel : public QWidget
{
	Q_OBJECT
public:
	CLabel(QWidget* wid);
	~CLabel();
	void SetPixmap(const QPixmap& pixmap);
	void ShowNormal();
	void ShowFullScreen();
	
protected:
	bool eventFilter(QObject*, QEvent*);

private:
	void OnModifyUI();

private:
	QPointer<QLabel> lb_img_;
	QPointer<QHBoxLayout> hbox_;
};