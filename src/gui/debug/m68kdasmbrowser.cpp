//
// m68kdasmbrowser.cpp - Jaguar M68K disassembly browser
//
// by James Hammons
// (C) 2012 Underground Software
//
// JLH = James Hammons <jlhamm@acm.org>
//
// Who  When        What
// ---  ----------  -------------------------------------------------------------
// JLH  12/01/2012  Created this file
//

// STILL TO DO:
//

#include "m68kdasmbrowser.h"
//#include "memory.h"
#include "m68000/m68kinterface.h"
#include "dsp.h"
#include "gpu.h"


M68KDasmBrowserWindow::M68KDasmBrowserWindow(QWidget * parent/*= 0*/): QWidget(parent, Qt::Dialog),
//	layout(new QVBoxLayout), text(new QTextBrowser),
	layout(new QVBoxLayout), text(new QLabel),
	refresh(new QPushButton(tr("Refresh"))),
	memBase(0x4000)
{
	setWindowTitle(tr("M68K Disassembly Browser"));

	// Need to set the size as well...
//	resize(560, 480);

	QFont fixedFont("Lucida Console", 10, QFont::Normal);
	text->setFont(fixedFont);
////	layout->setSizeConstraint(QLayout::SetFixedSize);
	setLayout(layout);

	layout->addWidget(text);
	layout->addWidget(refresh);

	connect(refresh, SIGNAL(clicked()), this, SLOT(RefreshContents()));
}


void M68KDasmBrowserWindow::RefreshContents(void)
{
	char string[1024];//, buf[64];
	QString s;

	char buffer[2048];
	int pc = memBase, oldpc;

	for(uint32_t i=0; i<32; i++)
	{
		oldpc = pc;
		pc += m68k_disassemble(buffer, pc, 0);
//		WriteLog("%06X: %s\n", oldpc, buffer);
		sprintf(string, "%06X: %s<br>", oldpc, buffer);

		buffer[0] = 0;	// Clear string
		char singleCharString[2] = { 0, 0 };

		for(int j=0; j<strlen(string); j++)
		{
			if (string[j] == 32)
				strcat(buffer, "&nbsp;");
			else
			{
				singleCharString[0] = string[j];
				strcat(buffer, singleCharString);
			}
		}

//		s += QString(string);
		s += QString(buffer);
	}

	text->clear();
	text->setText(s);
}


void M68KDasmBrowserWindow::keyPressEvent(QKeyEvent * e)
{
	if (e->key() == Qt::Key_Escape || e->key() == Qt::Key_Return)
		hide();
#if 1
	else if (e->key() == Qt::Key_PageUp)
	{
		memBase -= 480;

		if (memBase < 0)
			memBase = 0;

		RefreshContents();
	}
	else if (e->key() == Qt::Key_PageDown)
	{
		memBase += 480;

		if (memBase > (0x200000 - 480))
			memBase = 0x200000 - 480;

		RefreshContents();
	}
	else if (e->key() == Qt::Key_Up || e->key() == Qt::Key_Minus)
	{
		memBase -= 16;

		if (memBase < 0)
			memBase = 0;

		RefreshContents();
	}
	else if (e->key() == Qt::Key_Down || e->key() == Qt::Key_Equal)
	{
		memBase += 16;

		if (memBase > (0x200000 - 480))
			memBase = 0x200000 - 480;

		RefreshContents();
	}
#endif
}
