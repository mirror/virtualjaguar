//
// controllertab.cpp: "Controller" tab on the config dialog
//
// Part of the Virtual Jaguar Project
// (C) 2011 Underground Software
// See the README and GPLv3 files for licensing and warranty information
//
// JLH = James L. Hammons <jlhamm@acm.org>
//
// WHO  WHEN        WHAT
// ---  ----------  ------------------------------------------------------------
// JLH  06/23/2011  Created this file
// JLH  07/20/2011  Fixed a bunch of stuff
//

#include "controllertab.h"

#include "joystick.h"
#include "keygrabber.h"
#include "settings.h"

// These tables are used to convert Qt keycodes into human readable form. Note that
// a lot of these are just filler.
char ControllerTab::keyName1[96][16] = {
	"Space",
	"!", "\"", "#", "$", "%", "&", "'", "(", ")", "*", "+", ",", "-", ".", "/",
	"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", ":", ";", "<", "=", ">", "?",
	"@", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N",
	"O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z",
	"[", "]", "\\", "^", "_", "`",
	"$61", "$62", "$63", "$64", "$65", "$66", "$67", "$68", "$69", "$6A", "$6B", "$6C", "$6D", 
	"$6E", "$6F", "$70", "$71", "$72", "$73", "$74", "$75", "$76", "$77", "$78", "$79", "$7A", 
	"{", "|", "}", "~"
};

char ControllerTab::keyName2[64][16] = {
	"Esc", "Tab", "BTab", "BS", "Ret", "Ent", "Ins", "Del", "Pause", "Prt", "SRq", "Clr",
	"$C", "$D", "$E", "$F", "Hm", "End", "Lf", "Up", "Rt", "Dn", "PgU", "PgD", "$18",
	"$19", "$1A", "$1B", "$1C", "$1D", "$1E", "$1F", "Shf", "Ctl", "Mta", "Alt",
	"Cap", "Num", "ScL", "$27", "$28", "$29", "$2A", "$2B", "$2C", "$2D", "$2E", "$2F",
	"F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12", "F13",
	"F14", "F15", "F16"
};

ControllerTab::ControllerTab(QWidget * parent/*= 0*/): QWidget(parent)
{
	controllerPic = new QLabel;
	QImage controller(":/res/controller.png");
	controllerPic->setPixmap(QPixmap::fromImage(controller));

	redefineAll = new QPushButton(tr("Redefine All Keys"));

	QVBoxLayout * layout = new QVBoxLayout;
	layout->addWidget(controllerPic);
	layout->addWidget(redefineAll);
	setLayout(layout);

	connect(redefineAll, SIGNAL(clicked()), this, SLOT(DefineAllKeys()));
}

ControllerTab::~ControllerTab()
{
}

void ControllerTab::DefineAllKeys(void)
{
	char jagButtonName[21][10] = { "Up", "Down", "Left", "Right",
		"*", "7", "4", "1", "0", "8", "5", "2", "#", "9", "6", "3",
		"A", "B", "C", "Option", "Pause" };
	int orderToDefine[21] = { 0, 1, 2, 3, 18, 17, 16, 20, 19, 7, 11, 15, 6, 10, 14, 5, 9, 13, 8, 4, 12 };
	KeyGrabber keyGrab(this);

	for(int i=BUTTON_FIRST; i<=BUTTON_LAST; i++)
	{
		keyGrab.SetText(jagButtonName[orderToDefine[i]]);
		keyGrab.exec();
		int key = keyGrab.key;

		if (key == Qt::Key_Escape)
			break;

		// Otherwise, populate the appropriate spot in the settings & update screen...
		p1Keys[orderToDefine[i]] = key;
		UpdateLabel();
	}
}

void ControllerTab::UpdateLabel(void)
{
	QImage controller(":/res/controller.png");
	QPainter painter(&controller);
	painter.setRenderHint(QPainter::Antialiasing);

	// Bump up the size of the default font...
	QFont font = painter.font();
	font.setPixelSize(15);
	font.setBold(true);
	painter.setFont(font);
//	painter.setPen(QColor(48, 255, 255, 255));	// This is R,G,B,A
	painter.setPen(QColor(0, 0, 0, 255));		// This is R,G,B,A
	painter.setBrush(QBrush(QColor(48, 255, 255, 255)));

	// This is hard-coded crap. It's crap-tastic!
	int buttonPos[21][2] = { { 87-1, 64-5 }, { 87-1, 94 }, { 73-5, 78-2 }, { 105+3, 77-1 },
		{ 125, 223 }, { 125, 200 }, { 125, 177 }, { 125, 153 },
		{ 160, 223 }, { 160, 200 }, { 160, 177 }, { 160, 153 },
		{ 196, 223 }, { 196, 200 }, { 196, 177 }, { 196, 153 },
		{ 242, 60 }, { 225-1, 80 }, { 209-2, 104 }, { 162+2, 108-7}, { 141, 108+13 }
	};

	// First, draw black oversize line, then dot, then colored line
	QPen blackPen(QColor(0, 0, 0, 255));
	blackPen.setWidth(4);
	QPen colorPen(QColor(48, 255, 255, 255));
	colorPen.setWidth(2);
	QLine line(QPoint(141, 100), QPoint(141, 108+5));

	painter.setPen(blackPen);
	painter.drawLine(line);//QPoint(141, 100), QPoint(141, 108+5));
	blackPen.setWidth(1);
	painter.setPen(blackPen);
	painter.drawEllipse(QPoint(141, 100), 4, 4);
	painter.setPen(colorPen);
	painter.drawLine(line);//QPoint(141, 100), QPoint(141, 108+5));

	for(int i=BUTTON_FIRST; i<=BUTTON_LAST; i++)
	{
		if (p1Keys[i] < 0x80)
			DrawBorderedText(painter, buttonPos[i][0] /*- 5*/, buttonPos[i][1] /*+ 5*/,
				QString(keyName1[p1Keys[i] - 0x20]));
		else if ((p1Keys[i] & 0xFFFFFF00) == 0x01000000)
		{
			DrawBorderedText(painter, buttonPos[i][0] /*- 5*/, buttonPos[i][1] /*+ 5*/,
				QString(keyName2[p1Keys[i] & 0x3F]));
		}
		else
			DrawBorderedText(painter, buttonPos[i][0] - 5, buttonPos[i][1] + 5, QString("???"));
	}

	painter.end();
	controllerPic->setPixmap(QPixmap::fromImage(controller));
}

void ControllerTab::DrawBorderedText(QPainter & painter, int x, int y, QString text)
{
	// Text is drawn centered at (x, y) as well, using a bounding rect for the purpose.
	QRect rect(0, 0, 60, 30);
	painter.setPen(QColor(0, 0, 0, 255));		// This is R,G,B,A

	for(int i=-1; i<=1; i++)
	{
		for(int j=-1; j<=1; j++)
		{
//			painter.drawText(QPoint(x + i, y + j), text);
			rect.moveCenter(QPoint(x + i, y + j));
			painter.drawText(rect, Qt::AlignCenter, text);
		}
	}

	painter.setPen(QColor(48, 255, 255, 255));	// This is R,G,B,A
//	painter.drawText(QPoint(x, y), text);
	rect.moveCenter(QPoint(x, y));
	painter.drawText(rect, Qt::AlignCenter, text);
}
