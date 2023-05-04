#include "SRTools.h"
void writeCalib(vector<Point2f> world, vector<Point2f> pix, calibResult calib)
{
	QDomDocument doc;  //QDomDocument��
	QDomProcessingInstruction instruction = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
	doc.appendChild(instruction);

	QDomElement root = doc.createElement("calibration");//�������ڵ�  QDomElemetԪ��
	doc.appendChild(root);//��Ӹ��ڵ�  

	QDomElement strMac = doc.createElement("world");//����Ԫ�ؽڵ�  
	root.appendChild(strMac);//���Ԫ�ؽڵ㵽���ڵ�  
	QString value;
	value = ("(" + QString::number(world[0].x, 10, 3) + "," + QString::number(world[0].y, 10, 3) + ")@");
	value += ("(" + QString::number(world[1].x, 10, 3) + "," + QString::number(world[1].y, 10, 3) + ")@");
	value += ("(" + QString::number(world[2].x, 10, 3) + "," + QString::number(world[2].y, 10, 3) + ")@");
	value += ("(" + QString::number(world[3].x, 10, 3) + "," + QString::number(world[3].y, 10, 3) + ")@");
	value += ("(" + QString::number(world[4].x, 10, 3) + "," + QString::number(world[4].y, 10, 3) + ")@");
	value += ("(" + QString::number(world[5].x, 10, 3) + "," + QString::number(world[5].y, 10, 3) + ")@");
	value += ("(" + QString::number(world[6].x, 10, 3) + "," + QString::number(world[6].y, 10, 3) + ")@");
	value += ("(" + QString::number(world[7].x, 10, 3) + "," + QString::number(world[7].y, 10, 3) + ")@");
	value += ("(" + QString::number(world[8].x, 10, 3) + "," + QString::number(world[8].y, 10, 3) + ")");
	QDomText strMacNodeText = doc.createTextNode(value);//����Ԫ���ı� 
	strMac.appendChild(strMacNodeText);//���Ԫ���ı���Ԫ�ؽڵ�  

	strMac = doc.createElement("pix");//����Ԫ�ؽڵ�  
	root.appendChild(strMac);//���Ԫ�ؽڵ㵽���ڵ�  
	value;
	value = ("(" + QString::number(pix[0].x, 10, 3) + "," + QString::number(pix[0].y, 10, 3) + ")@");
	value += ("(" + QString::number(pix[1].x, 10, 3) + "," + QString::number(pix[1].y, 10, 3) + ")@");
	value += ("(" + QString::number(pix[2].x, 10, 3) + "," + QString::number(pix[2].y, 10, 3) + ")@");
	value += ("(" + QString::number(pix[3].x, 10, 3) + "," + QString::number(pix[3].y, 10, 3) + ")@");
	value += ("(" + QString::number(pix[4].x, 10, 3) + "," + QString::number(pix[4].y, 10, 3) + ")@");
	value += ("(" + QString::number(pix[5].x, 10, 3) + "," + QString::number(pix[5].y, 10, 3) + ")@");
	value += ("(" + QString::number(pix[6].x, 10, 3) + "," + QString::number(pix[6].y, 10, 3) + ")@");
	value += ("(" + QString::number(pix[7].x, 10, 3) + "," + QString::number(pix[7].y, 10, 3) + ")@");
	value += ("(" + QString::number(pix[8].x, 10, 3) + "," + QString::number(pix[8].y, 10, 3) + ")");
	strMacNodeText = doc.createTextNode(value);//����Ԫ���ı� 
	strMac.appendChild(strMacNodeText);//���Ԫ���ı���Ԫ�ؽڵ�  

	strMac = doc.createElement("calib");//����Ԫ�ؽڵ�  
	root.appendChild(strMac);//���Ԫ�ؽڵ㵽���ڵ�  
	value = (QString::number(calib.A, 10, 6) + ";");
	value += (QString::number(calib.B, 10, 6) + ";");
	value += (QString::number(calib.C, 10, 6) + ";");
	value += (QString::number(calib.D, 10, 6) + ";");
	value += (QString::number(calib.E, 10, 6) + ";");
	value += QString::number(calib.F, 10, 6);
	strMacNodeText = doc.createTextNode(value);//����Ԫ���ı� 
	strMac.appendChild(strMacNodeText);//���Ԫ���ı���Ԫ�ؽڵ�  

	QDateTime curDateTime = QDateTime::currentDateTime();
	QString data = curDateTime.toString("yyyyMMdd-hh-mm-ss");
	QFile file("..//calibration//" + data + ".xml");
	if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
		return;
	QTextStream out(&file);
	out.setCodec("UTF-8");
	doc.save(out, 4, QDomNode::EncodingFromTextStream);
	file.close();
}

void readCalib(QString path,calibResult &calib)
{
	QFile file(path);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		qDebug() << "Open file hello.xml failure";
		return;
	}

	// ��������
	QXmlStreamReader *xml = new QXmlStreamReader(&file);
	if (xml->readNextStartElement()) 
	{
		QString strName = xml->name().toString();
		if (strName == "calibration")
		{  // ��ȡ��Ԫ��
			while (xml->readNextStartElement()) {
				if (xml->name().toString() == "calib")
				{
					QString value = xml->readElementText();
					qDebug() << "calib is :" << value << endl;
					QStringList list = value.split(";");
					if (list.length() != 6)
					{
						file.close();
						return;
					}			
					else
					{
						calib.A = list[0].toDouble();
						calib.B = list[1].toDouble();
						calib.C = list[2].toDouble();
						calib.D = list[3].toDouble();
						calib.E = list[4].toDouble();
						calib.F = list[5].toDouble();
					}
				}
				else
					xml->skipCurrentElement();  // ������ǰԪ��
			}
		}
		else {
			xml->raiseError("XML file format error.");
		}
	}
	
	file.close();
	xml->clear();
	delete xml;
	xml = NULL;
}
