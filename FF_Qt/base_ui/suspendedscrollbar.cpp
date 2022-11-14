#include "suspendedscrollbar.h"
#include <QScrollBar>
#include <QFile>
#include <QWheelEvent>

const QString scrollbar_style = "QScrollBar:vertical{ width:6px; background:transparent; margin:0px, 0px, 0px, 0px; padding-top:0px; padding-bottom:0px; }"
"QScrollBar::handle:vertical { background:#E1E1E1; border-radius:3px; min-height:25px; }"
"QScrollBar::add-line:vertical { height:0px; subcontrol-position:bottom; }"
"QScrollBar::sub-line:vertical { height:0px; subcontrol-position:top; }"
"QScrollBar::handle:vertical:hover { background:#C4C4C4; }";

const QString scrollbar_style2 = "QScrollBar:horizontal{ width:6px; background:transparent; margin:0px, 0px, 0px, 0px; padding-left:0px; padding-right:0px; }"
"QScrollBar::handle:horizontal { background:#E1E1E1; border-radius:3px; min-width:25px; }"
"QScrollBar::add-line:horizontal { height:0px; subcontrol-position:right; }"
"QScrollBar::sub-line:horizontal { height:0px; subcontrol-position:left; }"
"QScrollBar::handle:horizontal:hover { background:#C4C4C4; }";


const int scrollbar_width = 6;

SuspendedScrollBar::SuspendedScrollBar(Qt::Orientation t ,  QWidget *parent) : QScrollBar(parent){

    this->setOrientation(t);
    this->setRange(0 , 0);
    this->hide();
}

void SuspendedScrollBar::slt_rangeChanged(int min,int max){
    this->setMinimum(min);
    this->setRange(0 , max);
	if (this->orientation() == Qt::Vertical)
	{
		this->setPageStep(0.75 * (this->height() + max));
	}
	else if (this->orientation() == Qt::Horizontal)
	{
		this->setPageStep(0.75 * (this->width() + max));
	}

	if (max <= 0)
	{
		this->hide();
	}
}

void SuspendedScrollBar::slt_valueChange_scrollBar(int value){
    this->setValue(value);
}

SuspendedScrollBar2::SuspendedScrollBar2(Qt::Orientation t, QWidget * parent /*= 0*/) : QScrollBar(parent)
{
	this->setOrientation(t);
	this->setRange(0, 0);
	this->hide();
}

void SuspendedScrollBar2::slt_valueChange_scrollBar(int value)
{
	this->setValue(value);
}

void SuspendedScrollBar2::slt_rangeChanged(int min, int max)
{
	this->setMinimum(min);
	this->setRange(0, max);
	if (this->orientation() == Qt::Vertical)
	{
		this->setPageStep(0.75 * (this->height() + max));
	}
	else if (this->orientation() == Qt::Horizontal)
	{
		this->setPageStep(0.75 * (this->width() + max));
	}

	if (max > 0)
	{
		this->raise();
		this->show();
	}
	else
	{
		this->hide();
	}
}


SuspendedScrollBarListWidget::SuspendedScrollBarListWidget(QWidget * parent)
	: QListWidget(parent), b_rolling_items_(false)
{
    this->setVerticalScrollMode(ScrollPerPixel);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_pVertScrollBar = new SuspendedScrollBar(Qt::Vertical, this);
	m_pVertScrollBar->setContextMenuPolicy(Qt::NoContextMenu);
	this->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
	this->setContextMenuPolicy(Qt::NoContextMenu);

	m_pVertScrollBar->setStyleSheet(scrollbar_style);
	m_pVertScrollBar->setStyle(m_pVertScrollBar->style());
    connect(this->verticalScrollBar(),SIGNAL(valueChanged(int)),m_pVertScrollBar,SLOT(slt_valueChange_scrollBar(int)));
    connect(m_pVertScrollBar,SIGNAL(valueChanged(int)),this,SLOT(slt_valueChange_widget(int)));
    connect(this->verticalScrollBar(),SIGNAL(rangeChanged(int,int)),m_pVertScrollBar,SLOT(slt_rangeChanged(int,int)));

}

void SuspendedScrollBarListWidget::SetRollingItems()
{
	b_rolling_items_ = true;
}

void SuspendedScrollBarListWidget::slt_valueChange_widget(int value){
    this->verticalScrollBar()->setValue(value);
}

void SuspendedScrollBarListWidget::resizeEvent(QResizeEvent *e){
	int x = this->width() - scrollbar_width - 2;
	m_pVertScrollBar->setGeometry(x, 1, scrollbar_width, this->height() - 2);
	return QListWidget::resizeEvent(e);
}

void SuspendedScrollBarListWidget::enterEvent(QEvent * e){
    if(m_pVertScrollBar->maximum() > 0)
        m_pVertScrollBar->show();
    return QListWidget::enterEvent(e);
}
void SuspendedScrollBarListWidget::leaveEvent(QEvent * e){
    m_pVertScrollBar->hide();
    return QListWidget::leaveEvent(e);
}

void SuspendedScrollBarListWidget::wheelEvent(QWheelEvent *e)
{
	if (b_rolling_items_)
	{
		int diff = 0;
		int number_degrees = e->delta() / 8;
		int number_steps = number_degrees / 15;
		if (e->orientation() == Qt::Vertical && number_steps > 0)
		{
			diff = -1;
		}
		else if (e->orientation() == Qt::Vertical && number_steps < 0)
		{
			diff = 1;
		}

		int next_row = this->currentRow() + diff;

		if (diff != 0 && 0 <= next_row && next_row < this->count())
		{
			this->setCurrentRow(next_row, QItemSelectionModel::SelectCurrent);
		}
	}
	else
	{
		QListWidget::wheelEvent(e);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
SuspendedScrollBarArea::SuspendedScrollBarArea(QWidget * parent)
    : QScrollArea(parent)
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setFrameShape(QFrame::NoFrame);

    m_pVertScrollBar = new SuspendedScrollBar(Qt::Vertical , this);
	m_pVertScrollBar->setContextMenuPolicy(Qt::NoContextMenu);
	this->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
	this->setContextMenuPolicy(Qt::NoContextMenu);

	m_pVertScrollBar->setStyleSheet(scrollbar_style);
	m_pVertScrollBar->setStyle(m_pVertScrollBar->style());

    connect(this->verticalScrollBar(),SIGNAL(valueChanged(int)),m_pVertScrollBar,SLOT(slt_valueChange_scrollBar(int)));
    connect(m_pVertScrollBar,SIGNAL(valueChanged(int)),this,SLOT(slt_valueChange_widget(int)));
    connect(this->verticalScrollBar(),SIGNAL(rangeChanged(int,int)),m_pVertScrollBar,SLOT(slt_rangeChanged(int,int)));
}

void SuspendedScrollBarArea::slt_valueChange_widget(int value){
	if (m_pVertScrollBar->maximum() > 0 && !m_pVertScrollBar->isVisible()) {
		m_pVertScrollBar->show();
	}
    this->verticalScrollBar()->setValue(value);
}

void SuspendedScrollBarArea::resizeEvent(QResizeEvent *e){
	int x = this->width() - scrollbar_width - 2;
	m_pVertScrollBar->setGeometry(x, 1, scrollbar_width, this->height() - 2);
    return QScrollArea::resizeEvent(e);
}

void SuspendedScrollBarArea::enterEvent(QEvent * e){
	if (m_pVertScrollBar->maximum() > 0)
		m_pVertScrollBar->show();
    return QScrollArea::enterEvent(e);
}
void SuspendedScrollBarArea::leaveEvent(QEvent * e){
    m_pVertScrollBar->hide();
    return QScrollArea::leaveEvent(e);
}

SuspendedScrollVHBarArea::SuspendedScrollVHBarArea(QWidget * parent /*= 0*/): can_dragging_(false)
{
	b_pressed_ = false;
	pressed_pos_ = QPoint(0, 0);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setFrameShape(QFrame::NoFrame);

	m_pVertScrollBar = new SuspendedScrollBar2(Qt::Vertical, this);
	m_pVertScrollBar->setContextMenuPolicy(Qt::NoContextMenu);

	m_pHertScrollBar = new SuspendedScrollBar2(Qt::Horizontal, this);
	m_pHertScrollBar->setContextMenuPolicy(Qt::NoContextMenu);

	this->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
	this->setContextMenuPolicy(Qt::NoContextMenu);

	m_pVertScrollBar->setStyleSheet(scrollbar_style);
	m_pVertScrollBar->setStyle(m_pVertScrollBar->style());

	m_pHertScrollBar->setStyleSheet(scrollbar_style2);
	m_pHertScrollBar->setStyle(m_pHertScrollBar->style());

	connect(this->verticalScrollBar(), SIGNAL(valueChanged(int)), m_pVertScrollBar, SLOT(slt_valueChange_scrollBar(int)));
	connect(this->horizontalScrollBar(), SIGNAL(valueChanged(int)), m_pHertScrollBar, SLOT(slt_valueChange_scrollBar(int)));
	connect(m_pVertScrollBar, SIGNAL(valueChanged(int)), this, SLOT(SlotVerticalValueChanged(int)));
	connect(m_pHertScrollBar, SIGNAL(valueChanged(int)), this, SLOT(SlotHorizontalValueChanged(int)));
	connect(this->verticalScrollBar(), SIGNAL(rangeChanged(int, int)), m_pVertScrollBar, SLOT(slt_rangeChanged(int, int)));
	connect(this->horizontalScrollBar(), SIGNAL(rangeChanged(int, int)), m_pHertScrollBar, SLOT(slt_rangeChanged(int, int)));
}

void SuspendedScrollVHBarArea::SetDragging(bool can_drag)
{
	can_dragging_ = can_drag;
}

void SuspendedScrollVHBarArea::resizeEvent(QResizeEvent *e)
{
	int x = this->width() - scrollbar_width;
	m_pVertScrollBar->setGeometry(x, 0, scrollbar_width, this->height());

	int y = this->height() - scrollbar_width;
	m_pHertScrollBar->setGeometry(0, y, x, scrollbar_width);

	return QScrollArea::resizeEvent(e);
}

void SuspendedScrollVHBarArea::SlotHorizontalValueChanged(int value)
{
	if (m_pHertScrollBar->maximum() > 0 && !m_pHertScrollBar->isVisible()) {
		m_pHertScrollBar->raise();
		m_pHertScrollBar->show();
	}
	this->horizontalScrollBar()->setValue(value);
}

void SuspendedScrollVHBarArea::SlotVerticalValueChanged(int value)
{
	if (m_pVertScrollBar->maximum() > 0 && !m_pVertScrollBar->isVisible()) {
		m_pVertScrollBar->raise();
		m_pVertScrollBar->show();
	}
	this->verticalScrollBar()->setValue(value);
}

void SuspendedScrollVHBarArea::mousePressEvent(QMouseEvent *e)
{
	if (can_dragging_)
	{
		b_pressed_ = true;
		pressed_pos_ = e->pos();
	}
	QWidget::mousePressEvent(e);
}

void SuspendedScrollVHBarArea::mouseMoveEvent(QMouseEvent *e)
{
	if (b_pressed_)
	{
		QPoint cur_pos = e->pos();
		QPoint dist = pressed_pos_ - cur_pos;

		int h_val = m_pHertScrollBar->value() + dist.x();
		int v_val = m_pVertScrollBar->value() + dist.y();

		if (m_pHertScrollBar->maximum() > 0 && m_pHertScrollBar->maximum() >= h_val)
		{
			m_pHertScrollBar->setValue(h_val);
		}
		if (m_pVertScrollBar->maximum() > 0 && m_pVertScrollBar->maximum() >= v_val)
		{
			m_pVertScrollBar->setValue(v_val);
		}
	}

	if (can_dragging_)
	{
		bool b_bar_show = false;
		if (m_pHertScrollBar->maximum() > 0 || m_pVertScrollBar->maximum() > 0)
		{
			b_bar_show = true;
		}
		this->setCursor(b_bar_show ? Qt::OpenHandCursor : Qt::ArrowCursor);
	}

	QWidget::mouseMoveEvent(e);
}

void SuspendedScrollVHBarArea::mouseReleaseEvent(QMouseEvent *e)
{
	if (can_dragging_)
	{
		b_pressed_ = false;
		pressed_pos_ = QPoint(0, 0);
	}
	QWidget::mouseReleaseEvent(e);
}
