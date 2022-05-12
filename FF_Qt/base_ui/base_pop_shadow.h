#pragma once
#include "QWidget"
#include "QPointer"
/*´°¿ÚÔ¤Áô12¸öÏñËØÍâ±ß¿ò*/
enum SHADOW_TYPE 
{
	SHADOW_TYPE_DEFAULT,//Ô¤Áô12ÏñËØÒõÓ°,12pxÔ²½Ç
	SHADOW_TYPE_LOGIN,	//Ô¤Áô30ÏñËØÒõÓ°,18ÏñËØÔ²½Ç
	SHADOW_TYPE_MAIN_FORM,	//Ô¤Áô30ÏñËØÒõÓ°,7ÏñËØÔ²½Ç
	SHADOW_NO_STYLE,//Ã»ÓĞÑùÊ½
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
