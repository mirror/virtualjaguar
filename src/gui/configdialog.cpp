//
// configdialog.cpp - Configuration dialog
//
// by James L. Hammons
// (C) 2010 Underground Software
//
// JLH = James L. Hammons <jlhamm@acm.org>
//
// Who  When        What
// ---  ----------  -------------------------------------------------------------
// JLH  01/29/2010  Created this file
// JLH  06/23/2011  Added initial implementation
//

#include "configdialog.h"

#include "generaltab.h"
#include "controllertab.h"
#include "settings.h"


ConfigDialog::ConfigDialog(QWidget * parent/*= 0*/): QDialog(parent)
{
	tabWidget = new QTabWidget;
	generalTab = new GeneralTab(this);
	controllerTab = new ControllerTab(this);
	tabWidget->addTab(generalTab, tr("General"));
	tabWidget->addTab(controllerTab, tr("Controller"));

	buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

	connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

	QVBoxLayout * mainLayout = new QVBoxLayout;
	mainLayout->addWidget(tabWidget);
	mainLayout->addWidget(buttonBox);
	setLayout(mainLayout);

	setWindowTitle(tr("Virtual Jaguar Settings"));

	LoadDialogFromSettings();
}

ConfigDialog::~ConfigDialog()
{
}

void ConfigDialog::LoadDialogFromSettings(void)
{
	generalTab->edit1->setText(vjs.jagBootPath);
	generalTab->edit2->setText(vjs.CDBootPath);
	generalTab->edit3->setText(vjs.EEPROMPath);
	generalTab->edit4->setText(vjs.ROMPath);
}

void ConfigDialog::UpdateVJSettings(void)
{
	strcpy(vjs.jagBootPath, generalTab->edit1->text().toAscii().data());
	strcpy(vjs.CDBootPath,  generalTab->edit2->text().toAscii().data());
	strcpy(vjs.EEPROMPath,  generalTab->edit3->text().toAscii().data());
	strcpy(vjs.ROMPath,     generalTab->edit4->text().toAscii().data());
}
