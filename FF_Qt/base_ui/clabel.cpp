#include "clabel.h"
#include "QEvent"
#include "QLabel"
#include <qlayout.h>
#include "../../player_controller/player_controller.h"
#include "windows.h"
CLabel::CLabel(QWidget* wid) : QWidget(wid)
{
	hbox_ = nullptr;
	lb_img_ = new QLabel(this);
	lb_img_->installEventFilter(this);
	OnModifyUI();
}

CLabel::~CLabel()
{
	if (lb_img_) 
	{
		delete lb_img_;
	}
}

void CLabel::SetPixmap(const QPixmap& pixmap)
{
	if (lb_img_) 
	{
		lb_img_->setPixmap(pixmap);
	}
}

void CLabel::ShowNormal()
{
	if (lb_img_->isFullScreen())
	{
		lb_img_->showNormal();
		lb_img_->setParent(this);
		hbox_->addWidget(lb_img_);
		PlayerController::GetInstance()->SetImageSize(this->width(), this->height());
	}
}

void CLabel::ShowFullScreen()
{
	if (!lb_img_->isFullScreen()) 
	{
		hbox_->removeWidget(lb_img_);
		lb_img_->setParent(NULL);
		lb_img_->showFullScreen();
		int width = GetSystemMetrics(SM_CXSCREEN);
		int height = GetSystemMetrics(SM_CYSCREEN);
		PlayerController::GetInstance()->SetImageSize(width, height);
	}
}

bool CLabel::eventFilter(QObject* obj, QEvent* evt)
{
	if (evt->type() == QEvent::MouseButtonDblClick && obj == lb_img_) 
	{
		if (lb_img_->isFullScreen()) 
		{
			ShowNormal();
		}
		else 
		{
			ShowFullScreen();
		}
	}
	return false;
}

void CLabel::OnModifyUI()
{
	hbox_ = new QHBoxLayout;
	hbox_->setMargin(0);
	hbox_->setSpacing(0);
	hbox_->addWidget(lb_img_);
	this->setLayout(hbox_);
	
}
