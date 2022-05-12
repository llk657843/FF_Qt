#pragma once
#include "QWidget"
#include "QPointer"
/*����Ԥ��12��������߿�*/
enum SHADOW_TYPE 
{
	SHADOW_TYPE_DEFAULT,//Ԥ��12������Ӱ,12pxԲ��
	SHADOW_TYPE_LOGIN,	//Ԥ��30������Ӱ,18����Բ��
	SHADOW_TYPE_MAIN_FORM,	//Ԥ��30������Ӱ,7����Բ��
	SHADOW_NO_STYLE,//û����ʽ
};
class BasePopShadowForm : public QWidget
{
public:
	BasePopShadowForm(QWidget* wid = 0);
	~BasePopShadowForm();
	
	void CheckoutShadowType(SHADOW_TYPE shadow_type);

protected:
	void resizeEvent(QResizeEvent*);
	void paintEvent(QPaintEvent *event) override;
	void SetWindowSize(const QSize&);

protected:
	QPointer<QWidget> wid_shadow_;

private:
	bool b_first_paint_;
};
