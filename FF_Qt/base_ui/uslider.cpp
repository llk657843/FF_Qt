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
	//�����ֹslider�Դ��Ķ�������¼�����Ӧ
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

	//��ȡ�������λ��
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