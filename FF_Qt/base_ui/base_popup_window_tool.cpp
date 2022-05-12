#include "base_popup_window_tool.h"
#include "QApplication"
#include "QDesktopWidget"

namespace base_window
{
	int GetPopupWinHeight()
	{
		QDesktopWidget* desktop_widget = QApplication::desktop();
		QRect desk_rect = desktop_widget->availableGeometry();

		int wid_height = desk_rect.height() * 0.9 ;
		if (wid_height < 680)
		{
			wid_height = 680;
		}
		return wid_height;
	}

	int GetWebPopupWinHeight()
	{
		QDesktopWidget* desktop_widget = QApplication::desktop();
		QRect desk_rect = desktop_widget->availableGeometry();

		int wid_height = desk_rect.height() * 0.85;
		if (desk_rect.height() < 680)
		{
			wid_height = 680;
		}
		return wid_height;
	}

	void ShowCenterWindow(QWidget* wid, QWidget* parent_wid)
	{
		if (wid)
		{
			QDesktopWidget* desktop_widget = QApplication::desktop();
			QRect desk_rect = desktop_widget->availableGeometry();
			int x = (desk_rect.width() - wid->width()) / 2;
			int y = (desk_rect.height() - wid->height()) / 2;
			if (parent_wid) {
				QPoint pos = parent_wid->mapToGlobal(QPoint(0, 0));
				//if (wid->width() + 50 < parent_wid->width()) 
				{
					x = pos.x() + (parent_wid->width() - wid->width()) / 2;
				}
				if (wid->height() + 50 < parent_wid->height()) 
				{
					y = pos.y() + (parent_wid->height() - wid->height()) / 2;
				}
			}
			wid->move(x, y);
			wid->activateWindow();
			wid->show();
		}
	}

}