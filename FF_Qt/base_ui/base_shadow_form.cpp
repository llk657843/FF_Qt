#include "base_shadow_form.h"
#include "qevent.h"

BaseShadowForm::BaseShadowForm(QWidget* wid /*= 0*/) :QWidget(wid), b_first_paint_(true)
{
	wid_shadow_ = new QWidget(this);
	wid_shadow_->move(0, 0);
	wid_shadow_->setObjectName("wid_item_shadow_500");
	wid_shadow_->lower();
}

BaseShadowForm::~BaseShadowForm()
{
}

void BaseShadowForm::SelectedShadow()
{
	SetWidgetObjectName("wid_item_shadow_501");
}

void BaseShadowForm::UnselectedShadow()
{
	SetWidgetObjectName("wid_item_shadow_500");
}

void BaseShadowForm::SetWidgetObjectName(const QString& obj_name)
{
	if (wid_shadow_)
	{
		SetObjectName(wid_shadow_, obj_name);
	}
}

void BaseShadowForm::ShowShadow(bool b_show)
{
	if (wid_shadow_)
	{
		wid_shadow_->setVisible(b_show);
	}
}

void BaseShadowForm::SetWindowSize(const QSize& size)
{
	wid_shadow_->resize(size);
}

void BaseShadowForm::resizeEvent(QResizeEvent* event)
{
	wid_shadow_->resize(event->size());
	QWidget::resizeEvent(event);
}

void BaseShadowForm::paintEvent(QPaintEvent *event)
{
	if (b_first_paint_)
	{
		b_first_paint_ = false;
		wid_shadow_->resize(this->size());
	}
	QWidget::paintEvent(event);
}

void BaseShadowForm::SetObjectName(QWidget *wid, const QString& obj_name)
{
	wid->setObjectName(obj_name);
	wid->setStyle(wid->style());
}