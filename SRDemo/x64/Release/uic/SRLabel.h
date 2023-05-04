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
	void mouseMoveEvent(QMouseEvent *event);//�����ƶ�ʱִ��
	void mousePressEvent(QMouseEvent *event);//��������ʱִ��
	void mouseReleaseEvent(QMouseEvent *event);//�����ɿ�ʱִ��

signals:
	void sendPos(QPoint);
	void sendLeftStartPos(QPoint);
	void sendLeftMovePos(QPoint);
	void sendLeftEndPos(QPoint);
};

