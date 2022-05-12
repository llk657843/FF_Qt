#pragma once
#include "QWidget"
#include "QPointer"
#include <QPoint>
#include <QSize>
/*´°¿ÚÔ¤Áô8¸öÏñËØÍâ±ß¿ò*/
class BaseShadowForm : public QWidget
{
public:
	BaseShadowForm(QWidget* wid = 0);
	~BaseShadowForm();
	void SelectedShadow();	//8ÏñËØÍâ±ß¿ò
	void UnselectedShadow();	//8ÏñËØÍâ±ß¿ò
	void SetWidgetObjectName(const QString& obj_name);
	void ShowShadow(bool b_show);

protected:
	void resizeEvent(QResizeEvent*) override;
	void paintEvent(QPaintEvent *event) override;
	void SetWindowSize(const QSize&);
	void SetObjectName(QWidget *wid, const QString& obj_name);

private:
	QPointer<QWidget> wid_shadow_;
	bool b_first_paint_;
};
