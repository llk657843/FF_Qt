#ifndef PLAYBACKSLIDER_H
#define PLAYBACKSLIDER_H

#include <QSlider>

/*	自定义水平slider，滑块可跳转到鼠标点击的位置*/
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