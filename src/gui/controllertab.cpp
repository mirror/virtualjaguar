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
	painter.setPen(QColor(0, 128, 255, 255));	// This is R,G,B,A

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
	char buttonName[21][10] = { "Up", "Dn", "Lf", "Rt",
		"-", "7", "4", "1", "0", "8", "5", "2", "=", "9", "6", "3",
		"C", "X", "Z", "'", "Ret" };

#if 0
	painter.drawText(QPoint(buttonPos[BUTTON_U][0], buttonPos[BUTTON_U][1]), QChar(0x2191));
	painter.drawText(QPoint(buttonPos[BUTTON_D][0], buttonPos[BUTTON_D][1]), QChar(0x2193));
#else
	for(int i=BUTTON_FIRST; i<=BUTTON_LAST; i++)
		painter.drawText(QPoint(buttonPos[i][0] - 5, buttonPos[i][1] + 5), QString(buttonName[i]));
#endif

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
}

#if 0
	// Keybindings in order of U, D, L, R, C, B, A, Op, Pa, 0-9, #, *
	vjs.p1KeyBindings[BUTTON_U] = settings.value("p1k_up", Qt::Key_Up).toInt();
	vjs.p1KeyBindings[BUTTON_D] = settings.value("p1k_down", Qt::Key_Down).toInt();
	vjs.p1KeyBindings[BUTTON_L] = settings.value("p1k_left", Qt::Key_Left).toInt();
	vjs.p1KeyBindings[BUTTON_R] = settings.value("p1k_right", Qt::Key_Right).toInt();
	vjs.p1KeyBindings[BUTTON_C] = settings.value("p1k_c", Qt::Key_Z).toInt();
	vjs.p1KeyBindings[BUTTON_B] = settings.value("p1k_b", Qt::Key_X).toInt();
	vjs.p1KeyBindings[BUTTON_A] = settings.value("p1k_a", Qt::Key_C).toInt();
	vjs.p1KeyBindings[BUTTON_OPTION] = settings.value("p1k_option", Qt::Key_Apostrophe).toInt();
	vjs.p1KeyBindings[BUTTON_PAUSE] = settings.value("p1k_pause", Qt::Key_Return).toInt();
	vjs.p1KeyBindings[BUTTON_0] = settings.value("p1k_0", Qt::Key_0).toInt();
	vjs.p1KeyBindings[BUTTON_1] = settings.value("p1k_1", Qt::Key_1).toInt();
	vjs.p1KeyBindings[BUTTON_2] = settings.value("p1k_2", Qt::Key_2).toInt();
	vjs.p1KeyBindings[BUTTON_3] = settings.value("p1k_3", Qt::Key_3).toInt();
	vjs.p1KeyBindings[BUTTON_4] = settings.value("p1k_4", Qt::Key_4).toInt();
	vjs.p1KeyBindings[BUTTON_5] = settings.value("p1k_5", Qt::Key_5).toInt();
	vjs.p1KeyBindings[BUTTON_6] = settings.value("p1k_6", Qt::Key_6).toInt();
	vjs.p1KeyBindings[BUTTON_7] = settings.value("p1k_7", Qt::Key_7).toInt();
	vjs.p1KeyBindings[BUTTON_8] = settings.value("p1k_8", Qt::Key_8).toInt();
	vjs.p1KeyBindings[BUTTON_9] = settings.value("p1k_9", Qt::Key_9).toInt();
	vjs.p1KeyBindings[BUTTON_d] = settings.value("p1k_pound", Qt::Key_Slash).toInt();
	vjs.p1KeyBindings[BUTTON_s] = settings.value("p1k_star", Qt::Key_Asterisk).toInt();

	vjs.p2KeyBindings[BUTTON_U] = settings.value("p2k_up", Qt::Key_Up).toInt();
	vjs.p2KeyBindings[BUTTON_D] = settings.value("p2k_down", Qt::Key_Down).toInt();
	vjs.p2KeyBindings[BUTTON_L] = settings.value("p2k_left", Qt::Key_Left).toInt();
	vjs.p2KeyBindings[BUTTON_R] = settings.value("p2k_right", Qt::Key_Right).toInt();
	vjs.p2KeyBindings[BUTTON_C] = settings.value("p2k_c", Qt::Key_Z).toInt();
	vjs.p2KeyBindings[BUTTON_B] = settings.value("p2k_b", Qt::Key_X).toInt();
	vjs.p2KeyBindings[BUTTON_A] = settings.value("p2k_a", Qt::Key_C).toInt();
	vjs.p2KeyBindings[BUTTON_OPTION] = settings.value("p2k_option", Qt::Key_Apostrophe).toInt();
	vjs.p2KeyBindings[BUTTON_PAUSE] = settings.value("p2k_pause", Qt::Key_Return).toInt();
	vjs.p2KeyBindings[BUTTON_0] = settings.value("p2k_0", Qt::Key_0).toInt();
	vjs.p2KeyBindings[BUTTON_1] = settings.value("p2k_1", Qt::Key_1).toInt();
	vjs.p2KeyBindings[BUTTON_2] = settings.value("p2k_2", Qt::Key_2).toInt();
	vjs.p2KeyBindings[BUTTON_3] = settings.value("p2k_3", Qt::Key_3).toInt();
	vjs.p2KeyBindings[BUTTON_4] = settings.value("p2k_4", Qt::Key_4).toInt();
	vjs.p2KeyBindings[BUTTON_5] = settings.value("p2k_5", Qt::Key_5).toInt();
	vjs.p2KeyBindings[BUTTON_6] = settings.value("p2k_6", Qt::Key_6).toInt();
	vjs.p2KeyBindings[BUTTON_7] = settings.value("p2k_7", Qt::Key_7).toInt();
	vjs.p2KeyBindings[BUTTON_8] = settings.value("p2k_8", Qt::Key_8).toInt();
	vjs.p2KeyBindings[BUTTON_9] = settings.value("p2k_9", Qt::Key_9).toInt();
	vjs.p2KeyBindings[BUTTON_d] = settings.value("p2k_pound", Qt::Key_Slash).toInt();
	vjs.p2KeyBindings[BUTTON_s] = settings.value("p2k_star", Qt::Key_Asterisk).toInt();

-------------------------------------------------------------------------------

Qt::Key_Escape	0x01000000	 
Qt::Key_Tab	0x01000001	 
Qt::Key_Backtab	0x01000002	 
Qt::Key_Backspace	0x01000003	 
Qt::Key_Return	0x01000004	 
Qt::Key_Enter	0x01000005	Typically located on the keypad.
Qt::Key_Insert	0x01000006	 
Qt::Key_Delete	0x01000007	 
Qt::Key_Pause	0x01000008	The Pause/Break key (Note: Not anything to do with pausing media)
Qt::Key_Print	0x01000009	 
Qt::Key_SysReq	0x0100000a	 
Qt::Key_Clear	0x0100000b	 
Qt::Key_Home	0x01000010	 
Qt::Key_End	0x01000011	 
Qt::Key_Left	0x01000012	 
Qt::Key_Up	0x01000013	 
Qt::Key_Right	0x01000014	 
Qt::Key_Down	0x01000015	 
Qt::Key_PageUp	0x01000016	 
Qt::Key_PageDown	0x01000017	 
Qt::Key_Shift	0x01000020	 
Qt::Key_Control	0x01000021	On Mac OS X, this corresponds to the Command keys.
Qt::Key_Meta	0x01000022	On Mac OS X, this corresponds to the Control keys. On Windows keyboards, this key is mapped to the Windows key.
Qt::Key_Alt	0x01000023	 
Qt::Key_AltGr	0x01001103	On Windows, when the KeyDown event for this key is sent, the Ctrl+Alt modifiers are also set.
Qt::Key_CapsLock	0x01000024	 
Qt::Key_NumLock	0x01000025	 
Qt::Key_ScrollLock	0x01000026	 
Qt::Key_F1	0x01000030	 
Qt::Key_F2	0x01000031	 
Qt::Key_F3	0x01000032	 
Qt::Key_F4	0x01000033	 
Qt::Key_F5	0x01000034	 
Qt::Key_F6	0x01000035	 
Qt::Key_F7	0x01000036	 
Qt::Key_F8	0x01000037	 
Qt::Key_F9	0x01000038	 
Qt::Key_F10	0x01000039	 
Qt::Key_F11	0x0100003a	 
Qt::Key_F12	0x0100003b	 
Qt::Key_F13	0x0100003c	 
Qt::Key_F14	0x0100003d	 
Qt::Key_F15	0x0100003e	 
Qt::Key_F16	0x0100003f	 
Qt::Key_F17	0x01000040	 
Qt::Key_F18	0x01000041	 
Qt::Key_F19	0x01000042	 
Qt::Key_F20	0x01000043	 
Qt::Key_F21	0x01000044	 
Qt::Key_F22	0x01000045	 
Qt::Key_F23	0x01000046	 
Qt::Key_F24	0x01000047	 
Qt::Key_F25	0x01000048	 
Qt::Key_F26	0x01000049	 
Qt::Key_F27	0x0100004a	 
Qt::Key_F28	0x0100004b	 
Qt::Key_F29	0x0100004c	 
Qt::Key_F30	0x0100004d	 
Qt::Key_F31	0x0100004e	 
Qt::Key_F32	0x0100004f	 
Qt::Key_F33	0x01000050	 
Qt::Key_F34	0x01000051	 
Qt::Key_F35	0x01000052	 
Qt::Key_Super_L	0x01000053	 
Qt::Key_Super_R	0x01000054	 
Qt::Key_Menu	0x01000055	 
Qt::Key_Hyper_L	0x01000056	 
Qt::Key_Hyper_R	0x01000057	 
Qt::Key_Help	0x01000058	 
Qt::Key_Direction_L	0x01000059	 
Qt::Key_Direction_R	0x01000060	 
Qt::Key_Space	0x20	 
Qt::Key_Any	Key_Space	 
Qt::Key_Exclam	0x21	 
Qt::Key_QuoteDbl	0x22	 
Qt::Key_NumberSign	0x23	 
Qt::Key_Dollar	0x24	 
Qt::Key_Percent	0x25	 
Qt::Key_Ampersand	0x26	 
Qt::Key_Apostrophe	0x27	 
Qt::Key_ParenLeft	0x28	 
Qt::Key_ParenRight	0x29	 
Qt::Key_Asterisk	0x2a	 
Qt::Key_Plus	0x2b	 
Qt::Key_Comma	0x2c	 
Qt::Key_Minus	0x2d	 
Qt::Key_Period	0x2e	 
Qt::Key_Slash	0x2f	 
Qt::Key_0	0x30	 
Qt::Key_1	0x31	 
Qt::Key_2	0x32	 
Qt::Key_3	0x33	 
Qt::Key_4	0x34	 
Qt::Key_5	0x35	 
Qt::Key_6	0x36	 
Qt::Key_7	0x37	 
Qt::Key_8	0x38	 
Qt::Key_9	0x39	 
Qt::Key_Colon	0x3a	 
Qt::Key_Semicolon	0x3b	 
Qt::Key_Less	0x3c	 
Qt::Key_Equal	0x3d	 
Qt::Key_Greater	0x3e	 
Qt::Key_Question	0x3f	 
Qt::Key_At	0x40	 
Qt::Key_A	0x41	 
Qt::Key_B	0x42	 
Qt::Key_C	0x43	 
Qt::Key_D	0x44	 
Qt::Key_E	0x45	 
Qt::Key_F	0x46	 
Qt::Key_G	0x47	 
Qt::Key_H	0x48	 
Qt::Key_I	0x49	 
Qt::Key_J	0x4a	 
Qt::Key_K	0x4b	 
Qt::Key_L	0x4c	 
Qt::Key_M	0x4d	 
Qt::Key_N	0x4e	 
Qt::Key_O	0x4f	 
Qt::Key_P	0x50	 
Qt::Key_Q	0x51	 
Qt::Key_R	0x52	 
Qt::Key_S	0x53	 
Qt::Key_T	0x54	 
Qt::Key_U	0x55	 
Qt::Key_V	0x56	 
Qt::Key_W	0x57	 
Qt::Key_X	0x58	 
Qt::Key_Y	0x59	 
Qt::Key_Z	0x5a	 
Qt::Key_BracketLeft	0x5b	 
Qt::Key_Backslash	0x5c	 
Qt::Key_BracketRight	0x5d	 
Qt::Key_AsciiCircum	0x5e	 
Qt::Key_Underscore	0x5f	 
Qt::Key_QuoteLeft	0x60	 
Qt::Key_BraceLeft	0x7b	 
Qt::Key_Bar	0x7c	 
Qt::Key_BraceRight	0x7d	 
Qt::Key_AsciiTilde	0x7e	

#endif
