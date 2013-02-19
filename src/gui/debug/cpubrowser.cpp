//
// cpubrowser.cpp - Jaguar CPU browser
//
// by James Hammons
// (C) 2012 Underground Software
//
// JLH = James Hammons <jlhamm@acm.org>
//
// Who  When        What
// ---  ----------  -------------------------------------------------------------
// JLH  08/14/2012  Created this file
//

// STILL TO DO:
//

#include "cpubrowser.h"
//#include "memory.h"
#include "m68000/m68kinterface.h"
#include "dsp.h"
#include "gpu.h"


CPUBrowserWindow::CPUBrowserWindow(QWidget * parent/*= 0*/): QWidget(parent, Qt::Dialog),
//	layout(new QVBoxLayout), text(new QTextBrowser),
	layout(new QVBoxLayout), text(new QLabel),
	refresh(new QPushButton(tr("Refresh"))),
	memBase(0)
{
	setWindowTitle(tr("CPU Browser"));

	// Need to set the size as well...
//	resize(560, 480);

//	QFont fixedFont("Lucida Console", 8, QFont::Normal);
	QFont fixedFont("", 8, QFont::Normal);
	fixedFont.setStyleHint(QFont::TypeWriter);
	text->setFont(fixedFont);
////	layout->setSizeConstraint(QLayout::SetFixedSize);
	setLayout(layout);

	layout->addWidget(text);
	layout->addWidget(refresh);

	connect(refresh, SIGNAL(clicked()), this, SLOT(RefreshContents()));
}


void CPUBrowserWindow::RefreshContents(void)
{
	char string[2048];
	QString s;

	// 68K
	uint32_t m68kPC = m68k_get_reg(NULL, M68K_REG_PC);
	uint32_t m68kSR = m68k_get_reg(NULL, M68K_REG_SR);
	sprintf(string, "PC: %06X&nbsp;&nbsp;SR: %04X<br><br>", m68kPC, m68kSR);
	s += QString(string);

	uint32_t m68kA0 = m68k_get_reg(NULL, M68K_REG_A0);
	uint32_t m68kA1 = m68k_get_reg(NULL, M68K_REG_A1);
	uint32_t m68kA2 = m68k_get_reg(NULL, M68K_REG_A2);
	uint32_t m68kA3 = m68k_get_reg(NULL, M68K_REG_A3);
	sprintf(string, "A0: %08X&nbsp;&nbsp;A1: %08X&nbsp;&nbsp;A2: %08X&nbsp;&nbsp;A3: %08X<br>", m68kA0, m68kA1, m68kA2, m68kA3);
	s += QString(string);

	uint32_t m68kA4 = m68k_get_reg(NULL, M68K_REG_A4);
	uint32_t m68kA5 = m68k_get_reg(NULL, M68K_REG_A5);
	uint32_t m68kA6 = m68k_get_reg(NULL, M68K_REG_A6);
	uint32_t m68kA7 = m68k_get_reg(NULL, M68K_REG_A7);
	sprintf(string, "A4: %08X&nbsp;&nbsp;A5: %08X&nbsp;&nbsp;A6: %08X&nbsp;&nbsp;A7: %08X<br><br>", m68kA4, m68kA5, m68kA6, m68kA7);
	s += QString(string);

	uint32_t m68kD0 = m68k_get_reg(NULL, M68K_REG_D0);
	uint32_t m68kD1 = m68k_get_reg(NULL, M68K_REG_D1);
	uint32_t m68kD2 = m68k_get_reg(NULL, M68K_REG_D2);
	uint32_t m68kD3 = m68k_get_reg(NULL, M68K_REG_D3);
	sprintf(string, "D0: %08X&nbsp;&nbsp;D1: %08X&nbsp;&nbsp;D2: %08X&nbsp;&nbsp;D3: %08X<br>", m68kD0, m68kD1, m68kD2, m68kD3);
	s += QString(string);

	uint32_t m68kD4 = m68k_get_reg(NULL, M68K_REG_D4);
	uint32_t m68kD5 = m68k_get_reg(NULL, M68K_REG_D5);
	uint32_t m68kD6 = m68k_get_reg(NULL, M68K_REG_D6);
	uint32_t m68kD7 = m68k_get_reg(NULL, M68K_REG_D7);
	sprintf(string, "D4: %08X&nbsp;&nbsp;D5: %08X&nbsp;&nbsp;D6: %08X&nbsp;&nbsp;D7: %08X<br><br>", m68kD4, m68kD5, m68kD6, m68kD7);
	s += QString(string);

	// GPU
	sprintf(string, "GPU PC: %06X&nbsp;&nbsp;FLAGS: %08X<br><br>", GPUReadLong(0xF02010), GPUReadLong(0xF02000));
	s += QString(string);

	sprintf(string, "Bank 0:<br>"
		"R00: %08X&nbsp;&nbsp;R01: %08X&nbsp;&nbsp;R02: %08X&nbsp;&nbsp;R03: %08X<br>"
		"R04: %08X&nbsp;&nbsp;R05: %08X&nbsp;&nbsp;R06: %08X&nbsp;&nbsp;R07: %08X<br>"
		"R08: %08X&nbsp;&nbsp;R09: %08X&nbsp;&nbsp;R10: %08X&nbsp;&nbsp;R11: %08X<br>"
		"R12: %08X&nbsp;&nbsp;R13: %08X&nbsp;&nbsp;R14: %08X&nbsp;&nbsp;R15: %08X<br>"
		"R16: %08X&nbsp;&nbsp;R17: %08X&nbsp;&nbsp;R18: %08X&nbsp;&nbsp;R19: %08X<br>"
		"R20: %08X&nbsp;&nbsp;R21: %08X&nbsp;&nbsp;R22: %08X&nbsp;&nbsp;R23: %08X<br>"
		"R24: %08X&nbsp;&nbsp;R25: %08X&nbsp;&nbsp;R26: %08X&nbsp;&nbsp;R27: %08X<br>"
		"R28: %08X&nbsp;&nbsp;R29: %08X&nbsp;&nbsp;R30: %08X&nbsp;&nbsp;R31: %08X<br><br>",
		gpu_reg_bank_0[0], gpu_reg_bank_0[1], gpu_reg_bank_0[2], gpu_reg_bank_0[3],
		gpu_reg_bank_0[4], gpu_reg_bank_0[5], gpu_reg_bank_0[6], gpu_reg_bank_0[7],
		gpu_reg_bank_0[8], gpu_reg_bank_0[9], gpu_reg_bank_0[10], gpu_reg_bank_0[11],
		gpu_reg_bank_0[12], gpu_reg_bank_0[13], gpu_reg_bank_0[14], gpu_reg_bank_0[15],
		gpu_reg_bank_0[16], gpu_reg_bank_0[17], gpu_reg_bank_0[18], gpu_reg_bank_0[19],
		gpu_reg_bank_0[20], gpu_reg_bank_0[21], gpu_reg_bank_0[22], gpu_reg_bank_0[23],
		gpu_reg_bank_0[24], gpu_reg_bank_0[25], gpu_reg_bank_0[26], gpu_reg_bank_0[27],
		gpu_reg_bank_0[28], gpu_reg_bank_0[29], gpu_reg_bank_0[30], gpu_reg_bank_0[31]);
	s += QString(string);

	sprintf(string, "Bank 1:<br>"
		"R00: %08X&nbsp;&nbsp;R01: %08X&nbsp;&nbsp;R02: %08X&nbsp;&nbsp;R03: %08X<br>"
		"R04: %08X&nbsp;&nbsp;R05: %08X&nbsp;&nbsp;R06: %08X&nbsp;&nbsp;R07: %08X<br>"
		"R08: %08X&nbsp;&nbsp;R09: %08X&nbsp;&nbsp;R10: %08X&nbsp;&nbsp;R11: %08X<br>"
		"R12: %08X&nbsp;&nbsp;R13: %08X&nbsp;&nbsp;R14: %08X&nbsp;&nbsp;R15: %08X<br>"
		"R16: %08X&nbsp;&nbsp;R17: %08X&nbsp;&nbsp;R18: %08X&nbsp;&nbsp;R19: %08X<br>"
		"R20: %08X&nbsp;&nbsp;R21: %08X&nbsp;&nbsp;R22: %08X&nbsp;&nbsp;R23: %08X<br>"
		"R24: %08X&nbsp;&nbsp;R25: %08X&nbsp;&nbsp;R26: %08X&nbsp;&nbsp;R27: %08X<br>"
		"R28: %08X&nbsp;&nbsp;R29: %08X&nbsp;&nbsp;R30: %08X&nbsp;&nbsp;R31: %08X<br><br>",
		gpu_reg_bank_1[0], gpu_reg_bank_1[1], gpu_reg_bank_1[2], gpu_reg_bank_1[3],
		gpu_reg_bank_1[4], gpu_reg_bank_1[5], gpu_reg_bank_1[6], gpu_reg_bank_1[7],
		gpu_reg_bank_1[8], gpu_reg_bank_1[9], gpu_reg_bank_1[10], gpu_reg_bank_1[11],
		gpu_reg_bank_1[12], gpu_reg_bank_1[13], gpu_reg_bank_1[14], gpu_reg_bank_1[15],
		gpu_reg_bank_1[16], gpu_reg_bank_1[17], gpu_reg_bank_1[18], gpu_reg_bank_1[19],
		gpu_reg_bank_1[20], gpu_reg_bank_1[21], gpu_reg_bank_1[22], gpu_reg_bank_1[23],
		gpu_reg_bank_1[24], gpu_reg_bank_1[25], gpu_reg_bank_1[26], gpu_reg_bank_1[27],
		gpu_reg_bank_1[28], gpu_reg_bank_1[29], gpu_reg_bank_1[30], gpu_reg_bank_1[31]);
	s += QString(string);

	// DSP
	sprintf(string, "DSP PC: %06X&nbsp;&nbsp;FLAGS: %08X<br><br>", DSPReadLong(0xF1A110), DSPReadLong(0xF1A100));
	s += QString(string);

	sprintf(string, "Bank 0:<br>"
		"R00: %08X&nbsp;&nbsp;R01: %08X&nbsp;&nbsp;R02: %08X&nbsp;&nbsp;R03: %08X<br>"
		"R04: %08X&nbsp;&nbsp;R05: %08X&nbsp;&nbsp;R06: %08X&nbsp;&nbsp;R07: %08X<br>"
		"R08: %08X&nbsp;&nbsp;R09: %08X&nbsp;&nbsp;R10: %08X&nbsp;&nbsp;R11: %08X<br>"
		"R12: %08X&nbsp;&nbsp;R13: %08X&nbsp;&nbsp;R14: %08X&nbsp;&nbsp;R15: %08X<br>"
		"R16: %08X&nbsp;&nbsp;R17: %08X&nbsp;&nbsp;R18: %08X&nbsp;&nbsp;R19: %08X<br>"
		"R20: %08X&nbsp;&nbsp;R21: %08X&nbsp;&nbsp;R22: %08X&nbsp;&nbsp;R23: %08X<br>"
		"R24: %08X&nbsp;&nbsp;R25: %08X&nbsp;&nbsp;R26: %08X&nbsp;&nbsp;R27: %08X<br>"
		"R28: %08X&nbsp;&nbsp;R29: %08X&nbsp;&nbsp;R30: %08X&nbsp;&nbsp;R31: %08X<br><br>",
		dsp_reg_bank_0[0], dsp_reg_bank_0[1], dsp_reg_bank_0[2], dsp_reg_bank_0[3],
		dsp_reg_bank_0[4], dsp_reg_bank_0[5], dsp_reg_bank_0[6], dsp_reg_bank_0[7],
		dsp_reg_bank_0[8], dsp_reg_bank_0[9], dsp_reg_bank_0[10], dsp_reg_bank_0[11],
		dsp_reg_bank_0[12], dsp_reg_bank_0[13], dsp_reg_bank_0[14], dsp_reg_bank_0[15],
		dsp_reg_bank_0[16], dsp_reg_bank_0[17], dsp_reg_bank_0[18], dsp_reg_bank_0[19],
		dsp_reg_bank_0[20], dsp_reg_bank_0[21], dsp_reg_bank_0[22], dsp_reg_bank_0[23],
		dsp_reg_bank_0[24], dsp_reg_bank_0[25], dsp_reg_bank_0[26], dsp_reg_bank_0[27],
		dsp_reg_bank_0[28], dsp_reg_bank_0[29], dsp_reg_bank_0[30], dsp_reg_bank_0[31]);
	s += QString(string);

	sprintf(string, "Bank 1:<br>"
		"R00: %08X&nbsp;&nbsp;R01: %08X&nbsp;&nbsp;R02: %08X&nbsp;&nbsp;R03: %08X<br>"
		"R04: %08X&nbsp;&nbsp;R05: %08X&nbsp;&nbsp;R06: %08X&nbsp;&nbsp;R07: %08X<br>"
		"R08: %08X&nbsp;&nbsp;R09: %08X&nbsp;&nbsp;R10: %08X&nbsp;&nbsp;R11: %08X<br>"
		"R12: %08X&nbsp;&nbsp;R13: %08X&nbsp;&nbsp;R14: %08X&nbsp;&nbsp;R15: %08X<br>"
		"R16: %08X&nbsp;&nbsp;R17: %08X&nbsp;&nbsp;R18: %08X&nbsp;&nbsp;R19: %08X<br>"
		"R20: %08X&nbsp;&nbsp;R21: %08X&nbsp;&nbsp;R22: %08X&nbsp;&nbsp;R23: %08X<br>"
		"R24: %08X&nbsp;&nbsp;R25: %08X&nbsp;&nbsp;R26: %08X&nbsp;&nbsp;R27: %08X<br>"
		"R28: %08X&nbsp;&nbsp;R29: %08X&nbsp;&nbsp;R30: %08X&nbsp;&nbsp;R31: %08X<br>",
		dsp_reg_bank_1[0], dsp_reg_bank_1[1], dsp_reg_bank_1[2], dsp_reg_bank_1[3],
		dsp_reg_bank_1[4], dsp_reg_bank_1[5], dsp_reg_bank_1[6], dsp_reg_bank_1[7],
		dsp_reg_bank_1[8], dsp_reg_bank_1[9], dsp_reg_bank_1[10], dsp_reg_bank_1[11],
		dsp_reg_bank_1[12], dsp_reg_bank_1[13], dsp_reg_bank_1[14], dsp_reg_bank_1[15],
		dsp_reg_bank_1[16], dsp_reg_bank_1[17], dsp_reg_bank_1[18], dsp_reg_bank_1[19],
		dsp_reg_bank_1[20], dsp_reg_bank_1[21], dsp_reg_bank_1[22], dsp_reg_bank_1[23],
		dsp_reg_bank_1[24], dsp_reg_bank_1[25], dsp_reg_bank_1[26], dsp_reg_bank_1[27],
		dsp_reg_bank_1[28], dsp_reg_bank_1[29], dsp_reg_bank_1[30], dsp_reg_bank_1[31]);
	s += QString(string);

	text->clear();
	text->setText(s);
}


void CPUBrowserWindow::keyPressEvent(QKeyEvent * e)
{
	if (e->key() == Qt::Key_Escape)
		hide();
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
}
