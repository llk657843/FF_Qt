#include "base_pop_shadow.h"
#include "qevent.h"

BasePopShadowForm::BasePopShadowForm(QWidget* wid /*= 0*/) :QWidget(wid), b_first_paint_(true)
{
	wid_shadow_ = new QWidget(this);
	wid_shadow_->setObjectName("wid_pop_shadow_501");
	wid_shadow_->lower();
}

BasePopShadowForm::~BasePopShadowForm()
{

}

void BasePopShadowForm::SetWindowSize(const QSize& size)
{
	wid_shadow_->resize(size);
}

void BasePopShadowForm::CheckoutShadowType(SHADOW_TYPE shadow_type)
{
	switch (shadow_type)
	{
	case SHADOW_TYPE_DEFAULT:
		wid_shadow_->setObjectName("wid_pop_shadow_501");
		break;
	case SHADOW_TYPE_LOGIN:
		wid_shadow_->setObjectName("wid_login_shadow_500");
		break;
	case SHADOW_TYPE_MAIN_FORM:
		wid_shadow_->setObjectName("wid_main_form_shadow_500");
		break;

	case SHADOW_NO_STYLE:
		wid_shadow_->setObjectName("");
		break;;
	default:
		break;
	}
	wid_shadow_->setStyle(wid_shadow_->style());
}

void BasePopShadowForm::resizeEvent(QResizeEvent* event)
{
	wid_shadow_->resize(event->size());
	QWidget::resizeEvent(event);
}

void BasePopShadowForm::paintEvent(QPaintEvent *event)
{
	if (b_first_paint_)
	{
		b_first_paint_ = false;
		wid_shadow_->resize(this->size());
	}
	QWidget::paintEvent(event);
}

