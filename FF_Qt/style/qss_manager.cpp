#include "qss_manager.h"
#include <QCoreApplication>
#include <QApplication>
#include <QFile>
#include <QGraphicsEffect>
#include <QDir>

#ifdef _DEBUG
#include <QDebug>
#include <QTime>
#endif



QGraphicsDropShadowEffect* QssManager::graphics_shadow_effect_normal_ = new QGraphicsDropShadowEffect;
QGraphicsDropShadowEffect* QssManager::graphics_shadow_effect_hover_ = new QGraphicsDropShadowEffect;
QGraphicsDropShadowEffect* QssManager::graphics_shadow_effect_none_ = new QGraphicsDropShadowEffect;
QGraphicsDropShadowEffect* QssManager::graphics_shadow_effect_window_ = new QGraphicsDropShadowEffect;
QColor QssManager::color_default_;

QssManager::QssManager()
{

}

QssManager::~QssManager()
{

}

void QssManager::InitGraphicsShadowEffect()
{
	QssManager::SetGraphicsShadowEffectNone();
	QssManager::SetGraphicsShadowEffectNormal(0, 2, 16, QColor("#000000"));
	QssManager::SetGraphicsShadowEffectHover(0, 2, 20, QColor("#000000"));
	QssManager::SetGraphicsShadowEffectWindow(0, 1, 20, QColor("#7B909A"));
}

bool QssManager::SetGlobalStyle(const QString &dir_path)
{
	QStringList str_list;
	if (!ReadStyleFiles(dir_path, str_list))
	{
		return false;
	}

	return SetGlobalStyle(str_list);
}

bool QssManager::SetGlobalStyle(const QStringList &style)
{
#ifdef _DEBUG
	QTime time;
	time.start();
#endif // DEBUG

	QString qss_str;
	for (int i = 0; i < style.size(); ++i)
	{
		QFile qss_file(style[i]);
		qss_file.open(QFile::ReadOnly);
		if (qss_file.isOpen())
		{
			QString str = qss_file.readAll();
			qss_str += str;
			qss_file.close();
		}
	}
	if (!qss_str.isEmpty())
	{
		QApplication* q_app = static_cast<QApplication *>(QCoreApplication::instance());
		q_app->setStyleSheet(qss_str);
#ifdef _DEBUG
		qDebug() << "set qss time" << time.elapsed() << "ms";
#endif // _DEBUG		
		return true;
	}
#ifdef _DEBUG
	qDebug() << "fail name" << time.elapsed() << "ms";
#endif // _DEBUG
	
	return false;
}

bool QssManager::ReadStyleFiles(const QString& dir_path, QStringList& file_list)
{
#ifdef _DEBUG
	QTime time;
	time.start();
#endif // _DEBUG

	QDir dir(dir_path);
	if (!dir.exists())
	{
#ifdef _DEBUG
		qDebug() << "fail dir path not exist." << time.elapsed() << "ms";
#endif // _DEBUG		
		return false;
	}
	QStringList filters;
	filters << "*.qss";
	dir.setFilter(QDir::Files | QDir::NoSymLinks);
	dir.setNameFilters(filters);
	int count = dir.count();
	for (int i = 0; i < count; ++i)
	{
		QString file_path = dir_path + dir[i];
		file_list += file_path;
	}

#ifdef _DEBUG
	qDebug() << "Read file time "<< time.elapsed() << "ms";
#endif	
	return true;
}

QGraphicsDropShadowEffect* QssManager::GetGraphicsShadowEffectNormal(qreal alpha /*= 0.3*/)
{
	QColor color_temp = graphics_shadow_effect_normal_->color();
	color_temp.setAlpha(255 * alpha);
	graphics_shadow_effect_normal_->setColor(color_temp);
	return graphics_shadow_effect_normal_;
}

QGraphicsDropShadowEffect* QssManager::GetGraphicsShadowEffectHover(qreal alpha /*= 0.5*/)
{
	QColor color_temp = graphics_shadow_effect_hover_->color();
	color_temp.setAlpha(255 * alpha);
	graphics_shadow_effect_hover_->setColor(color_temp);
	return graphics_shadow_effect_hover_;
}

void QssManager::SetGraphicsShadowEffectNormal(int x_offset, int y_offset, int blur_radius, const QColor &color)
{
	if (graphics_shadow_effect_normal_ != nullptr)
	{
		graphics_shadow_effect_normal_->setOffset(x_offset, y_offset);
		graphics_shadow_effect_normal_->setBlurRadius(blur_radius);
		graphics_shadow_effect_normal_->setColor(color);
		color_default_ = color;
	}
}

void QssManager::SetGraphicsShadowEffectHover(int x_offset, int y_offset, int blur_radius, const QColor &color)
{
	if (graphics_shadow_effect_hover_ != nullptr)
	{
		graphics_shadow_effect_hover_->setOffset(x_offset, y_offset);
		graphics_shadow_effect_hover_->setBlurRadius(blur_radius);
		graphics_shadow_effect_hover_->setColor(color);
	}
}

QColor QssManager::GetDefaultColor(qreal alpha /*= 0.3*/)
{
	color_default_.setAlpha(255*alpha);
	return color_default_;
}

QGraphicsDropShadowEffect* QssManager::GetGraphicsShadowEffectNone()
{
	return graphics_shadow_effect_none_;
}

void QssManager::SetGraphicsShadowEffectNone()
{
	graphics_shadow_effect_none_->setOffset(0);
	graphics_shadow_effect_none_->setBlurRadius(0);
	graphics_shadow_effect_none_->setColor(QColor(255, 255, 255, 0));
}

QGraphicsDropShadowEffect* QssManager::GetGraphicsShadowEffectWindow(qreal alpha /*= 0.5*/)
{
	QColor color_temp = graphics_shadow_effect_window_->color();
	color_temp.setAlpha(255 * alpha);
	graphics_shadow_effect_window_->setColor(color_temp);
	return graphics_shadow_effect_window_;
}

void QssManager::SetGraphicsShadowEffectWindow(int x_offset, int y_offset, int blur_radius, const QColor &color)
{
	if (graphics_shadow_effect_window_ != nullptr)
	{
		graphics_shadow_effect_window_->setOffset(x_offset, y_offset);
		graphics_shadow_effect_window_->setBlurRadius(blur_radius);
		graphics_shadow_effect_window_->setColor(color);
	}
}
