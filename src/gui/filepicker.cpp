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
	fileList->setItemDelegate(new ImageDelegate(this));

	QVBoxLayout * layout = new QVBoxLayout;
	setLayout(layout);
	layout->addWidget(fileList);

	fileThread = new FileThread(this);
	connect(fileThread, SIGNAL(FoundAFile(unsigned long)), this, SLOT(AddFileToList(unsigned long)));
	fileThread->Go();
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
