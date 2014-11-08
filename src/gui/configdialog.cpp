//
// configdialog.cpp - Configuration dialog
//
// by James Hammons
// (C) 2010 Underground Software
//
// JLH = James Hammons <jlhamm@acm.org>
//
// Who  When        What
// ---  ----------  ------------------------------------------------------------
// JLH  01/29/2010  Created this file
// JLH  06/23/2011  Added initial implementation
// JLH  10/14/2011  Fixed possibly missing final slash in paths
//

#include "configdialog.h"

#include "alpinetab.h"
#include "controllertab.h"
#include "controllerwidget.h"
#include "generaltab.h"
#include "settings.h"


ConfigDialog::ConfigDialog(QWidget * parent/*= 0*/): QDialog(parent),
	tabWidget(new QTabWidget),
	generalTab(new GeneralTab(this)),
	controllerTab1(new ControllerTab(this))
{
//	tabWidget = new QTabWidget;
//	generalTab = new GeneralTab(this);
//	controllerTab1 = new ControllerTab(this);
////	controllerTab2 = new ControllerTab(this);

//	if (vjs.hardwareTypeAlpine)
//		alpineTab = new AlpineTab(this);

	tabWidget->addTab(generalTab, tr("General"));
	tabWidget->addTab(controllerTab1, tr("Controllers"));
//	tabWidget->addTab(controllerTab2, tr("Controller #2"));

	if (vjs.hardwareTypeAlpine)
	{
		alpineTab = new AlpineTab(this);
		tabWidget->addTab(alpineTab, tr("Alpine"));
	}

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
//	generalTab->edit1->setText(vjs.jagBootPath);
//	generalTab->edit2->setText(vjs.CDBootPath);
	generalTab->edit3->setText(vjs.EEPROMPath);
	generalTab->edit4->setText(vjs.ROMPath);

	generalTab->useBIOS->setChecked(vjs.useJaguarBIOS);
	generalTab->useGPU->setChecked(vjs.GPUEnabled);
	generalTab->useDSP->setChecked(vjs.DSPEnabled);
	generalTab->useFullScreen->setChecked(vjs.fullscreen);
//	generalTab->useHostAudio->setChecked(vjs.audioEnabled);
	generalTab->useFastBlitter->setChecked(vjs.useFastBlitter);

	if (vjs.hardwareTypeAlpine)
	{
		alpineTab->edit1->setText(vjs.alpineROMPath);
		alpineTab->edit2->setText(vjs.absROMPath);
		alpineTab->writeROM->setChecked(vjs.allowWritesToROM);
	}

#warning "!!! Need to load settings from controller profile !!!"
// We do this now, but not here. Need to fix this...
#if 0
	for(int i=0; i<21; i++)
	{
// We need to find the right profile and load it up here...
		controllerTab1->controllerWidget->keys[i] = vjs.p1KeyBindings[i];
//		controllerTab2->controllerWidget->keys[i] = vjs.p2KeyBindings[i];
	}
#endif
}


void ConfigDialog::UpdateVJSettings(void)
{
//	strcpy(vjs.jagBootPath, generalTab->edit1->text().toAscii().data());
//	strcpy(vjs.CDBootPath,  generalTab->edit2->text().toAscii().data());
	strcpy(vjs.EEPROMPath,  CheckForTrailingSlash(
		generalTab->edit3->text()).toUtf8().data());
	strcpy(vjs.ROMPath,     CheckForTrailingSlash(
		generalTab->edit4->text()).toUtf8().data());

	vjs.useJaguarBIOS  = generalTab->useBIOS->isChecked();
	vjs.GPUEnabled     = generalTab->useGPU->isChecked();
	vjs.DSPEnabled     = generalTab->useDSP->isChecked();
	vjs.fullscreen     = generalTab->useFullScreen->isChecked();
//	vjs.audioEnabled   = generalTab->useHostAudio->isChecked();
	vjs.useFastBlitter = generalTab->useFastBlitter->isChecked();

	if (vjs.hardwareTypeAlpine)
	{
		strcpy(vjs.alpineROMPath, alpineTab->edit1->text().toUtf8().data());
		strcpy(vjs.absROMPath,    alpineTab->edit2->text().toUtf8().data());
		vjs.allowWritesToROM = alpineTab->writeROM->isChecked();
	}

#warning "!!! Need to save settings to controller profile !!!"
// We do this now, but not here. Need to fix this...
#if 0
	for(int i=0; i<21; i++)
	{
// We need to find the right profile and load it up here...
		vjs.p1KeyBindings[i] = controllerTab1->controllerWidget->keys[i];
//		vjs.p2KeyBindings[i] = controllerTab2->controllerWidget->keys[i];
	}
#endif
}


QString ConfigDialog::CheckForTrailingSlash(QString s)
{
	if (!s.endsWith('/') && !s.endsWith('\\'))
		s.append('/');

	return s;
}
