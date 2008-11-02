//
// FileList class
//
// by James L. Hammons
//

#ifndef __FILELIST_H__
#define __FILELIST_H__

#include "window.h"
#include "listbox.h"

class FileList: public Window
{
	public:
		FileList(uint32 x = 0, uint32 y = 0, uint32 w = 0, uint32 h = 0);
		virtual ~FileList() {}
		virtual void HandleKey(SDLKey key);
		virtual void HandleMouseMove(uint32 x, uint32 y);
		virtual void HandleMouseButton(uint32 x, uint32 y, bool mouseDown);
		virtual void Draw(uint32 offsetX = 0, uint32 offsetY = 0);
		virtual void Notify(Element * e);

	protected:
		ListBox * files;
		Button * load;
};

#endif	// __FILELIST_H__
