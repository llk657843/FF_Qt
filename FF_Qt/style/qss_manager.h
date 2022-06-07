#ifndef QSS_MANAGER_H
#define QSS_MANAGER_H

#include <QString>
#include <QWidget>
#include <QPushButton>

class QGraphicsDropShadowEffect;
class QssManager
{
public:
	QssManager();
	~QssManager();

	static bool SetGlobalStyle(const QString &dir_path);

	static void InitGraphicsShadowEffect();

	static QGraphicsDropShadowEffect* GetGraphicsShadowEffectNormal(qreal alpha = 0.12);			//透明度百分比
	static void SetGraphicsShadowEffectNormal(int x_offset, int y_offset, int blur_radius, const QColor &color);

	static QGraphicsDropShadowEffect* GetGraphicsShadowEffectHover(qreal alpha = 0.12);			//透明度百分比
	static void SetGraphicsShadowEffectHover(int x_offset, int y_offset, int blur_radius, const QColor &color);

	static QColor GetDefaultColor(qreal alpha = 0.3);											//透明度百分比

	static QGraphicsDropShadowEffect* GetGraphicsShadowEffectNone();
	static void SetGraphicsShadowEffectNone();

	static QGraphicsDropShadowEffect* GetGraphicsShadowEffectWindow(qreal alpha = 0.5);
	static void SetGraphicsShadowEffectWindow(int x_offset, int y_offset, int blur_radius, const QColor &color);

private:
	static bool SetGlobalStyle(const QStringList&);
	static bool ReadStyleFiles(const QString& dir_path, QStringList& file_list);

private:
	static QGraphicsDropShadowEffect *graphics_shadow_effect_normal_;
	static QGraphicsDropShadowEffect *graphics_shadow_effect_hover_;
	static QGraphicsDropShadowEffect *graphics_shadow_effect_none_;
	static QGraphicsDropShadowEffect *graphics_shadow_effect_window_;
	static QColor color_default_;
};

#endif	// QSS_MANAGER_H