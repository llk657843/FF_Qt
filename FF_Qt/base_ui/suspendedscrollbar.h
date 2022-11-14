#ifndef SUSPENDEDSCROLLBAR_H
#define SUSPENDEDSCROLLBAR_H

#include <QWidget>
#include <QScrollBar>
#include <QListWidget>
#include <QScrollArea>

class SuspendedScrollBar : public QScrollBar
{
    Q_OBJECT
public:
    explicit SuspendedScrollBar(Qt::Orientation t , QWidget * parent = 0);
    ~SuspendedScrollBar(){}
public slots:
    void slt_valueChange_scrollBar(int);
    void slt_rangeChanged(int min,int max);
};

class SuspendedScrollBar2 : public QScrollBar
{
	Q_OBJECT
public:
	explicit SuspendedScrollBar2(Qt::Orientation t, QWidget * parent = 0);
	~SuspendedScrollBar2(){}

public slots:
	void slt_valueChange_scrollBar(int);
	void slt_rangeChanged(int min, int max);
};

//QListWidget ��������������
class SuspendedScrollBarListWidget : public QListWidget
{
    Q_OBJECT
public:
    explicit SuspendedScrollBarListWidget(QWidget * parent = 0);
    ~SuspendedScrollBarListWidget(){}
	void SetRollingItems();
private:
    SuspendedScrollBar * m_pVertScrollBar;//����������
public slots:
    void slt_valueChange_widget(int);
protected:
    void resizeEvent(QResizeEvent *e);
    void enterEvent(QEvent *);
    void leaveEvent(QEvent *);
	void wheelEvent(QWheelEvent *e);

private:
	bool b_rolling_items_;
};

//QScrollArea ��������������
class SuspendedScrollBarArea : public QScrollArea
{
    Q_OBJECT
public:
    explicit SuspendedScrollBarArea(QWidget * parent = 0);
    ~SuspendedScrollBarArea(){}
private:
    SuspendedScrollBar * m_pVertScrollBar;//����������
public slots:
    void slt_valueChange_widget(int);
protected:
    void resizeEvent(QResizeEvent *e);
    void enterEvent(QEvent *);
    void leaveEvent(QEvent *);
};


class SuspendedScrollVHBarArea : public QScrollArea
{
	Q_OBJECT
public:
	explicit SuspendedScrollVHBarArea(QWidget * parent = 0);
	~SuspendedScrollVHBarArea(){}
	void SetDragging(bool can_drag);

private:
	SuspendedScrollBar2 * m_pVertScrollBar;//����������
	SuspendedScrollBar2 * m_pHertScrollBar;//����������

public slots:
	void SlotVerticalValueChanged(int);
	void SlotHorizontalValueChanged(int);

protected:
	void resizeEvent(QResizeEvent *e);

	void mousePressEvent(QMouseEvent *e);
	void mouseMoveEvent(QMouseEvent *e);
	void mouseReleaseEvent(QMouseEvent *e);

private:
	bool can_dragging_;
	QPoint pressed_pos_;
	bool b_pressed_;
};



#endif // SUSPENDEDSCROLLBAR_H
