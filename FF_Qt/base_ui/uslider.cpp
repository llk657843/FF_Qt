#include "uslider.h"
#include <QMouseEvent>

USlider::USlider(QWidget* parent):QSlider(Qt::Horizontal, parent)
{

}

USlider::~USlider()
{

}

void USlider::mousePressEvent(QMouseEvent *ev)
{
	//必须禁止slider自带的对鼠标点击事件的响应
//	QSlider::mousePressEvent(ev);
//	setValue(value_);
	emit sliderIsPressed();
}

void USlider::mouseMoveEvent(QMouseEvent *ev)
{
	int value = ev->x()*(maximum() - minimum()) / width();
	value += minimum();
	if (value < minimum())
	{
		value = minimum();
	}
	if (value > maximum())
	{
		value = maximum();
	}
	setValue(value);
	emit sliderIsMoved(value);
}

void USlider::keyPressEvent(QKeyEvent *ev)
{

}

void USlider::wheelEvent(QWheelEvent *ev)
{
	QWidget::wheelEvent(ev);
}

void USlider::mouseReleaseEvent(QMouseEvent *ev)
{
	QSlider::mousePressEvent(ev);

	//获取鼠标点击的位置
	int value = ev->x()*(maximum() - minimum()) / width();
	value += minimum();

	if (value < minimum())
	{
		value = minimum();
	}
	if (value > maximum())
	{
		value = maximum();
	}
	setValue(value);
	emit valueChangedByMouse(value);
}