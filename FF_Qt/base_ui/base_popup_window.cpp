#include "base_popup_window.h"
#include "qicon.h"
#include "windows.h"
#include <QEvent>
#include <QCloseEvent>
#include <QApplication>


enum WindowEdge {
	CENTER = 22,
	RIGHT = 23,
	BOTTOM = 32,
	BOTTOMRIGHT = 33
};
const int FRAME_SHAPE = 5;
const QMargins DEFAULT_SHADOW = QMargins(12,12,12,12);
BasePopupWindow::BasePopupWindow(QWidget *parent /*= 0*/) : BasePopShadowForm(parent)
{
	if (parent) {
		this->setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
		this->setWindowModality(Qt::WindowModal);
	}
	else {
		this->setWindowFlags(Qt::FramelessWindowHint);
	}
	this->setAttribute(Qt::WA_DeleteOnClose);
	this->setAttribute(Qt::WA_TranslucentBackground);
	this->installEventFilter(this);
	b_max_win_enabled_ = false;
	title_height_ = 60;
	shadow_margin_ = DEFAULT_SHADOW;
	move_flag_ = false;
	resize_flag_ = false;
	b_resize_view_ = true;
	cal_cursor_pos_ = 0;
	minimum_size_ = QSize(9999999, 9999999);
	UpdateWindowTitle("");
	UpdateWindowIcon();
}

BasePopupWindow::~BasePopupWindow()
{
	UnRegisterWnd();
}

void BasePopupWindow::Create(SHADOW_TYPE shadow_type, bool b_resize)
{
	b_resize_view_ = b_resize;
	CheckoutShadowType(shadow_type);
	RegisterWnd();
	InitWindow();
}

void BasePopupWindow::SetMinimumSize(const QSize& size)
{
	minimum_size_ = size;
	this->resize(minimum_size_);
}

void BasePopupWindow::SetMaxWinEnabled(bool b_enabled)
{
	b_max_win_enabled_ = b_enabled;
}

void BasePopupWindow::mousePressEvent(QMouseEvent *event)
{
	if (event->y() < title_height_ && this->windowState() != Qt::WindowMaximized)
	{
		mouse_pt_ = event->globalPos();
		move_flag_ = true;
	}
	else if(b_resize_view_)
	{
		cal_cursor_pos_ = calCursorPos(event->pos(), calCursorCol(event->pos()));
		if (event->button() == Qt::LeftButton)
		{
			if (cal_cursor_pos_ != CENTER)
			{
				resize_flag_ = true;
			}
		}
		pre_geometry_ = geometry();
		view_mouse_pos_ = event->globalPos();
	}
}

void BasePopupWindow::mouseMoveEvent(QMouseEvent *event)
{
	if (event->y() < title_height_ && move_flag_)
	{
		int dx = event->globalX() - mouse_pt_.x();
		int dy = event->globalY() - mouse_pt_.y();
		mouse_pt_ = event->globalPos();
		this->move(this->x() + dx, this->y() + dy);
	}
	else if (resize_flag_)
	{
		if (Qt::WindowMaximized != windowState())
		{
			setCursorShape(calCursorPos(event->pos(), calCursorCol(event->pos())));
		}
		QPoint current_pos = QCursor::pos(); //获取当前的点，这个点是全局的
		QPoint move_size = current_pos - view_mouse_pos_; //计算出移动的位置，当前点 - 鼠标左键按下的点
		QRect temp_geometry = pre_geometry_;
	
		switch (cal_cursor_pos_)
		{
		/*case WindowEdge::TOPLEFT:
			temp_geometry.setTopLeft(pre_geometry_.topLeft() + move_size);
			break;
		case WindowEdge::TOP:
			temp_geometry.setTop(pre_geometry_.top() + move_size.y());
			break;
		case WindowEdge::TOPRIGHT:
			temp_geometry.setTopRight(pre_geometry_.topRight() + move_size);
			break;
		case WindowEdge::LEFT:
			temp_geometry.setLeft(pre_geometry_.left() + move_size.x());
			break;*/
		case WindowEdge::RIGHT:
			temp_geometry.setRight(pre_geometry_.right() + move_size.x());
			break;
			/*case WindowEdge::BOTTOMLEFT:
				temp_geometry.setBottomLeft(pre_geometry_.bottomLeft() + move_size);
				break;*/
		case WindowEdge::BOTTOM:
			temp_geometry.setBottom(pre_geometry_.bottom() + move_size.y());
			break;
		case WindowEdge::BOTTOMRIGHT:
			temp_geometry.setBottomRight(pre_geometry_.bottomRight() + move_size);
			break;
		default:
			break;
		}
		//LOG_ERR("temp_geometry({0}, {1}, {2}, {3})") << temp_geometry.left() << temp_geometry.top() << temp_geometry.right() << temp_geometry.bottom();
		if (temp_geometry.width() < minimum_size_.width())
		{
			temp_geometry.setRight(minimum_size_.width() + temp_geometry.left());
		}
		if (temp_geometry.height() < minimum_size_.height())
		{
			temp_geometry.setBottom(minimum_size_.height() + temp_geometry.top());
		}
		this->setGeometry(temp_geometry);
		
	}
}

void BasePopupWindow::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->y() < title_height_ && move_flag_)
	{
		mouse_pt_ = event->globalPos();
	}
	else if(resize_flag_)
	{
		setCursorShape(0);
	}
	
	resize_flag_ = false;
	move_flag_ = false;
}

void BasePopupWindow::mouseDoubleClickEvent(QMouseEvent *event)
{
	if(!b_max_win_enabled_)
	{
		return;
	}
	if (event->button() == Qt::LeftButton && b_resize_view_)       //鼠标双击最大化/正常
	{
		if (event->y() < title_height_)
		{
			if (windowState() != Qt::WindowMaximized)
			{
				this->showMaximized();
			}
			else
			{
				this->showNormal();
			}
		}
	}
}

void BasePopupWindow::setCursorShape(int pos)
{
	Qt::CursorShape cursor;
	switch (pos)
	{
	//case WindowEdge::TOPLEFT:
	case WindowEdge::BOTTOMRIGHT:
		cursor = Qt::SizeFDiagCursor;
		break;
		/*case WindowEdge::TOPRIGHT:
		case WindowEdge::BOTTOMLEFT:
			cursor = Qt::SizeBDiagCursor;
			break;*/
	//case WindowEdge::TOP:
	case WindowEdge::BOTTOM:
		cursor = Qt::SizeVerCursor;
		break;
	//case WindowEdge::LEFT:
	case WindowEdge::RIGHT:
		cursor = Qt::SizeHorCursor;
		break;
	default:
		cursor = Qt::ArrowCursor;
		break;
	}
	this->setCursor(cursor);
}

int BasePopupWindow::calCursorCol(QPoint pt)
{
	int left_pos = pt.x() - shadow_margin_.left();
	int right_pos = this->width() - shadow_margin_.right();
	bool b_left = (left_pos >= 0 && left_pos <= FRAME_SHAPE);
	bool b_right = (pt.x() >= right_pos - FRAME_SHAPE && pt.x() <= right_pos);
	return (b_left ? 1 : (b_right ? 3 : 2));
}

int BasePopupWindow::calCursorPos(QPoint pt, int col_pos)
{
	int top_pos = pt.y() - shadow_margin_.top();
	int bootom_pos = this->height() - shadow_margin_.bottom();
	bool b_top = (top_pos >= 0 && top_pos <= FRAME_SHAPE);
	bool b_bottom = (pt.y() >= bootom_pos - FRAME_SHAPE && pt.y() <= bootom_pos);
	return ((b_top ? 10 : (b_bottom ? 30 : 20)) + col_pos);
}

void BasePopupWindow::SetTitleHeight(int height)
{
	title_height_ = height;
}

void BasePopupWindow::SetShadowMargin(QMargins shadow_margin)
{
	shadow_margin_ = shadow_margin;
}

void BasePopupWindow::UnRegisterWnd()
{
	if (!wnd_id_.empty())
	{
		//WindowsManager::GetInstance()->UnRegisterWindow(wnd_id_);
		wnd_id_ = L"";
	}
}

void BasePopupWindow::UpdateWindowTitle(const QString& title)
{
	if (title.isEmpty())
	{
		setWindowTitle(QString::fromLocal8Bit("会员时代"));
	}
	else
	{
		setWindowTitle(title);
	}
}

void BasePopupWindow::UpdateWindowIcon()
{
	QIcon icon = QIcon(":/hysd/images/icon/hysd-logo.ico");
	setWindowIcon(icon);
}

bool BasePopupWindow::RegisterWnd()
{
	wnd_id_ = GetWindowId();
	/*if (!WindowsManager::GetInstance()->RegisterWindow(wnd_id_, this))
	{
		return false;
	}*/
	return true;
}

