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
	"Esc", "Tab", "BTab", "BS", "Ret", "Ent", "Ins", "Del", "Pau", "Prt", "SRq", "Clr",
	"$C", "$D", "$E", "$F", "Hm", "End", "Lf", "Up", "Rt", "Dn", "PgU", "PgD", "$18",
	"$19", "$1A", "$1B", "$1C", "$1D", "$1E", "$1F", "Shf", "Ctl", "Mta", "Alt",
	"Cap", "Num", "ScL", "$27", "$28", "$29", "$2A", "$2B", "$2C", "$2D", "$2E", "$2F",
	"F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12", "F13",
	"F14", "F15", "F16"
};

ControllerTab::ControllerTab(QWidget * parent/*= 0*/): QWidget(parent)
{
	QLabel * img = new QLabel;
//	img->setPixmap(QPixmap(":/res/controller.png"));

/*
	QImage cartImg(":/res/cart-blank.png");
	QPainter painter(&cartImg);
	painter.drawPixmap(23, 87, QPixmap(":/res/label-blank.png"));
	painter.end();
	cartSmall = cartImg.scaled(488/4, 395/4, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
*/
	QImage controller(":/res/controller.png");
	QPainter painter(&controller);
//	painter.draw();
//	painter->drawText(QRect(option.rect.x()+10, option.rect.y()+36, 196/2, 70/2),
//		Qt::TextWordWrap | Qt::AlignHCenter, nameToDraw);

	// Bump up the size of the default font...
	QFont font = painter.font();
	font.setPixelSize(14);
	font.setBold(true);
	painter.setFont(font);
	painter.setPen(QColor(48, 255, 255, 255));	// This is R,G,B,A

	// This is hard-coded crap. It's crap-tastic!
/*
Bitmap Locations:

Up: 87,64
Down: 87,94
Left: 73,78
Right: 105,77
C: 209,104
B: 225,80
A: 242,60
Pause: 141,109
Option: 165,107
1: 125,153
2: 160,153
3: 196,153
4: 125,177
5: 
6: 
7: 125,199
8: 
9: 
*: 125,223
0: 
#: 
enum { BUTTON_FIRST = 0, BUTTON_U = 0,
BUTTON_D = 1,
BUTTON_L = 2,
BUTTON_R = 3,

BUTTON_s = 4,
BUTTON_7 = 5,
BUTTON_4 = 6,
BUTTON_1 = 7,
BUTTON_0 = 8,
BUTTON_8 = 9,
BUTTON_5 = 10,
BUTTON_2 = 11,
BUTTON_d = 12,
BUTTON_9 = 13,
BUTTON_6 = 14,
BUTTON_3 = 15,

BUTTON_A = 16,
BUTTON_B = 17,
BUTTON_C = 18,
BUTTON_OPTION = 19,
BUTTON_PAUSE = 20, BUTTON_LAST = 20 };
*/
	int buttonPos[21][2] = { { 87, 64 }, { 87, 94 }, { 73, 78 }, { 105, 77 },
		{ 125, 223 }, { 125, 199 }, { 125, 177 }, { 125, 153 },
		{ 160, 223 }, { 160, 199 }, { 160, 177 }, { 160, 153 },
		{ 196, 223 }, { 196, 199 }, { 196, 177 }, { 196, 153 },
		{ 242, 60 }, { 225, 80 }, { 209, 104 }, { 165, 107 }, { 141, 109 }
	};

	for(int i=BUTTON_FIRST; i<=BUTTON_LAST; i++)
//		painter.drawText(QPoint(buttonPos[i][0] - 5, buttonPos[i][1] + 5), QString(buttonName[i]));
	{
// Need to convert this back to p1Keys, but will require some restructuring...
//		if (p1Keys[i] < 0x80)
		if (vjs.p1KeyBindings[i] < 0x80)
//			painter.drawText(QPoint(buttonPos[i][0] - 5, buttonPos[i][1] + 5), QString(keyName1[p1Keys[i] - 0x20]));
//			painter.drawText(QPoint(buttonPos[i][0] - 5, buttonPos[i][1] + 5), QString(keyName1[vjs.p1KeyBindings[i] - 0x20]));
			DrawBorderedText(painter, buttonPos[i][0] - 5, buttonPos[i][1] + 5, QString(keyName1[vjs.p1KeyBindings[i] - 0x20]));
		else if ((vjs.p1KeyBindings[i] & 0xFFFFFF00) == 0x01000000)
		{
//			painter.drawText(QPoint(buttonPos[i][0] - 5, buttonPos[i][1] + 5), QString(keyName2[vjs.p1KeyBindings[i] & 0x3F]));
			DrawBorderedText(painter, buttonPos[i][0] - 5, buttonPos[i][1] + 5, QString(keyName2[vjs.p1KeyBindings[i] & 0x3F]));
		}
		else
//			painter.drawText(QPoint(buttonPos[i][0] - 5, buttonPos[i][1] + 5), QString("???"));
			DrawBorderedText(painter, buttonPos[i][0] - 5, buttonPos[i][1] + 5, QString("???"));
	}

	painter.end();
	img->setPixmap(QPixmap::fromImage(controller));

	redefineAll = new QPushButton(tr("Redefine All Keys"));

	QVBoxLayout * layout = new QVBoxLayout;
	layout->addWidget(img);
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

		// Otherwise, populate the appropriate spot in the settings...
		p1Keys[orderToDefine[i]] = key;
	}
}

void ControllerTab::DrawBorderedText(QPainter & painter, int x, int y, QString text)
{
	painter.setPen(QColor(0, 0, 0, 255));		// This is R,G,B,A

	for(int i=-1; i<=1; i++)
	{
		for(int j=-1; j<=1; j++)
		{
			painter.drawText(QPoint(x + i, y + j), text);
		}
	}

	painter.setPen(QColor(48, 255, 255, 255));	// This is R,G,B,A
	painter.drawText(QPoint(x, y), text);
}
