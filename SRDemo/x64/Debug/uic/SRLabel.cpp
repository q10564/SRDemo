#include "SRLabel.h"



SRLabel::SRLabel()
{
	this->setMouseTracking(true);
}


SRLabel::SRLabel(QWidget *widget)
{
	this->setMouseTracking(true);
}

SRLabel::~SRLabel()
{
}

void SRLabel::mouseMoveEvent(QMouseEvent *event)
{
	//qDebug() << event->x() << "," << event->y();
	emit sendPos(event->pos());
	if (Qt::LeftButton == (event->buttons() & Qt::LeftButton))
	{
		emit sendLeftMovePos(event->pos());
	}
}

void SRLabel::mousePressEvent(QMouseEvent * event)
{
	if (event->button() == Qt::LeftButton)
	{
		emit sendLeftStartPos(event->pos());
	}
}

void SRLabel::mouseReleaseEvent(QMouseEvent  *event)
{
	if (Qt::LeftButton == event->button())
	{
		emit sendLeftEndPos(event->pos());
	}
}

