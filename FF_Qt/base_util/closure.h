#pragma once
#include <functional>
#include <QObject>

class QObjectWithClosureSignal : public QObject
{
	Q_OBJECT

public:
	Q_SIGNAL void SignalClosure(const std::function<void()>&);
};
