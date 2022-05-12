#pragma once
#include "base_pop_shadow.h"

class BasePopupWindow : public BasePopShadowForm
{
public:
	BasePopupWindow(QWidget *parent = 0);
	virtual ~BasePopupWindow();

	void Create(SHADOW_TYPE shadow_type = SHADOW_TYPE_DEFAULT, bool b_resize = false);
	void SetMinimumSize(const QSize& size);
	virtual std::wstring GetWindowId(void) const = 0;
	virtual void InitWindow() = 0;

protected:
	void SetMaxWinEnabled(bool);
	virtual void mousePressEvent(QMouseEvent *event) override;
	virtual void mouseMoveEvent(QMouseEvent *event) override;
	virtual void mouseReleaseEvent(QMouseEvent *event) override;
	virtual void mouseDoubleClickEvent(QMouseEvent *event) override;
	virtual void setCursorShape(int pos);
	virtual int calCursorCol(QPoint pt);
	virtual int calCursorPos(QPoint pt, int col_pos);

protected:
	void SetTitleHeight(int height);
	void SetShadowMargin(QMargins shadow_margin);
	void UnRegisterWnd();
	void UpdateWindowTitle(const QString& title);
	void UpdateWindowIcon();

private:
	bool RegisterWnd();

private:
	std::wstring wnd_id_;
	int title_height_;
	bool b_resize_view_;
	QMargins shadow_margin_;

	QPoint mouse_pt_;            //用于存储鼠标位置
	bool move_flag_;        //窗口移动标志位
	bool resize_flag_;      //窗口大小重置标志
	int	cal_cursor_pos_;
	QRect pre_geometry_;
	QPoint view_mouse_pos_;
	QSize minimum_size_;
	bool b_max_win_enabled_;
};

