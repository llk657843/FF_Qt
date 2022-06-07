#ifndef PLAYBACKSLIDER_H
#define PLAYBACKSLIDER_H

#include <QSlider>

/*	�Զ���ˮƽslider���������ת���������λ��*/
class USlider :public QSlider
{
	Q_OBJECT
public:
	explicit USlider(QWidget* parent);
	~USlider();

signals:
	void sliderIsPressed();
	void valueChangedByMouse(int);
	void sliderIsMoved(int);

protected:
	void mousePressEvent(QMouseEvent *ev);
	void mouseReleaseEvent(QMouseEvent *ev);
	void mouseMoveEvent(QMouseEvent *ev);
	void keyPressEvent(QKeyEvent *ev);
	void wheelEvent(QWheelEvent *ev);
};

#endif // PLAYBACKSLIDER_H