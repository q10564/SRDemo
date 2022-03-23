#pragma once
#include <QFile>     
#include <QtXml\QtXml>  
#include <QtXml\QDomDocument> 
#include "SRVision.h"
void writeCalib(vector<Point2f> world, vector<Point2f> pix, calibResult calib);
void readCalib(QString path, calibResult &calib);

