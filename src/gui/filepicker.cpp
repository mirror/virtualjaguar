//
// filepicker.cpp - A ROM chooser
//
// by James L. Hammons
// (C) 2010 Underground Software
//
// JLH = James L. Hammons <jlhamm@acm.org>
//
// Who  When        What
// ---  ----------  -------------------------------------------------------------
// JLH  01/22/2010  Created this file
// JLH  02/06/2010  Modified to use Qt model/view framework
//

#include "filepicker.h"

#include "filedb.h"
#include "filelistmodel.h"
#include "filethread.h"
#include "imagedelegate.h"
//#include "settings.h"
//#include "types.h"

/*
Our strategy here is like so:
Look at the files in the directory pointed to by ROMPath.
For each file in the directory, take the CRC32 of it and compare it to the CRC
in the romList[]. If there's a match, put it in a list and note it's index value
in romList for future reference.

When constructing the list, use the index to pull up an image of the cart and
put that in the list. User picks from a graphical image of the cart.

Ideally, the label will go into the archive along with the ROM image, but that's
for the future...
Maybe box art, screenshots will go as well...

I'm thinking compatibility info should be displayed as well... Just to stop the
inevitable stupid questions from people too lazy to do basic research for themselves.
*/

//could use Window as well...
//FilePickerWindow::FilePickerWindow(QWidget * parent/*= 0*/): QWidget(parent, Qt::Dialog)
FilePickerWindow::FilePickerWindow(QWidget * parent/*= 0*/): QWidget(parent, Qt::Window)
{
	setWindowTitle("Insert Cartridge...");

//is there any reason why this must be cast as a QAbstractListModel? No
//Also, need to think about data structure for the model...
	model = new FileListModel;
	fileList = new QListView;
	fileList->setModel(model);
//	fileList->setItemDelegate(new ImageDelegate(this));
	fileList->setItemDelegate(new ImageDelegate());

//	QVBoxLayout * layout = new QVBoxLayout;
	QHBoxLayout * layout = new QHBoxLayout;
	setLayout(layout);
	layout->addWidget(fileList);

	// Weird note: This layout has to be added *before* putting anything into it...
	QVBoxLayout * vLayout = new QVBoxLayout;
	layout->addLayout(vLayout);

	QLabel * image = new QLabel;
//	image->setAlignment(Qt::AlignRight);
//	image->setPixmap(QPixmap(":/res/cart-blank.png"));
	QImage cartImg(":/res/cart-blank.png");
	QPainter painter(&cartImg);
	painter.drawPixmap(23, 87, 373, 172, QPixmap(":/res/label-blank.png"));
	painter.end();
	image->setPixmap(QPixmap::fromImage(cartImg));
	image->setMargin(4);
	vLayout->addWidget(image);

	QLabel * text1 = new QLabel(QString(tr(
		"<h2>Attack of the Mutant Penguins (World)</h2>"
	)));
	text1->setMargin(6);
	text1->setAlignment(Qt::AlignCenter);
	vLayout->addWidget(text1);

	QLabel * text2 = new QLabel(QString(tr(
		"<table>"
		"<tr><td align='right'><b>Type: </b></td><td>4MB Cartridge</td></tr>"
		"<tr><td align='right'><b>CRC32: </b></td><td>FEDCBA98</td></tr>"
		"<tr><td align='right'><b>Compatibility: </b></td><td>DOES NOT WORK</td></tr>"
		"<tr><td align='right'><b>Notes: </b></td><td>Universal Header detected; Requires DSP</td></tr>"
		"</table>"
	)));
	vLayout->addWidget(text2);

	fileThread = new FileThread(this);
//	connect(fileThread, SIGNAL(FoundAFile(unsigned long)), this, SLOT(AddFileToList(unsigned long)));
	connect(fileThread, SIGNAL(FoundAFile2(unsigned long, QString, QImage *)), this, SLOT(AddFileToList2(unsigned long, QString, QImage *)));
	fileThread->Go();
/*
New sizes: 373x172 (label), 420x340 (cart)
*/
}

//
// This slot gets called by the FileThread's run() function when it finds a
// match in the filesystem to a ROM on our CRC list.
//
void FilePickerWindow::AddFileToList(unsigned long index)
{
printf("FilePickerWindow: Found match [%s]...\n", romList[index].name);
	// NOTE: The model *ignores* what you send it, so this is crap. !!! FIX !!! [DONE, somewhat]
//	model->AddData(QIcon(":/res/generic.png"));
	model->AddData(index);
}

void FilePickerWindow::AddFileToList2(unsigned long index, QString str, QImage * img)
{
printf("FilePickerWindow(2): Found match [%s]...\n", romList[index].name);
	if (img)
		model->AddData(index, str, *img);
	else
		model->AddData(index, str, QImage());
}

/*
    Super Duper Awesome Guy (World)

         Type: 4MB Cartridge
        CRC32: FEDCBA98
Compatibility: DOES NOT WORK
        Notes: Universal Header detected; Requires DSP


    Stupid Homebrew Game That Sux

         Type: ABS/COF Executable (43853 bytes)
        CRC32: 76543210
Compatibility: Unknown
        Notes: $4000 Load, $4000 Run


    Action Hopscotch Plus (Prototype)

         Type: 2MB Alpine ROM
        CRC32: 44889921
Compatibility: 80% (or ****)
        Notes: EEPROM available


*/


