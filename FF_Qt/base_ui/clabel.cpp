#include "clabel.h"
#include "QEvent"
CLabel::CLabel(QWidget* wid) : QLabel(wid)
{
	parent_ = wid;
	this->installEventFilter(this);
}

CLabel::~CLabel()
{
}

bool CLabel::eventFilter(QObject* obj, QEvent* evt)
{
	if (evt->type() == QEvent::MouseButtonDblClick) 
	{
		if(this->isFullScreen())
		{
			/*this->setParent(parent_);
			this->showNormal();*/
		}
		else 
		{
			this->setParent(NULL);
			this->showFullScreen();
		}
	}
	return false;
}
