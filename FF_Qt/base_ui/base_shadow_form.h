#pragma once
#include "QWidget"
#include "QPointer"
#include <QPoint>
#include <QSize>
/*����Ԥ��8��������߿�*/
class BaseShadowForm : public QWidget
{
public:
	BaseShadowForm(QWidget* wid = 0);
	~BaseShadowForm();
	void SelectedShadow();	//8������߿�
	void UnselectedShadow();	//8������߿�
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
