//
// filepicker.h - A ROM chooser
//

#include <QtGui>

// Forward declarations
class QListWidget;

class FilePickerWindow: public QWidget
{
	public:
		FilePickerWindow(QWidget * parent = 0);

	protected:
		void PopulateList(void);

	private:
		QListWidget * fileList;
};
