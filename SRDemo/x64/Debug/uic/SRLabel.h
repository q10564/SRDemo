#pragma once
#include <qlabel.h>
#include <QDebug>
#include <QMouseEvent>
class SRLabel :
	public QLabel
{
	Q_OBJECT
public:
	SRLabel();
	SRLabel(QWidget*);
	~SRLabel();
protected:
	void mouseMoveEvent(QMouseEvent *event);//按键移动时执行
	void mousePressEvent(QMouseEvent *event);//按键按下时执行
	void mouseReleaseEvent(QMouseEvent *event);//按键松开时执行

signals:
	void sendPos(QPoint);
	void sendLeftStartPos(QPoint);
	void sendLeftMovePos(QPoint);
	void sendLeftEndPos(QPoint);
};

