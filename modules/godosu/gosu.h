#ifndef GOSU_H
#define GOSU_H

#include "scene/gui/control.h"

class Gosu : public Control {
	GDCLASS(Gosu, Control);

protected:
	void _notification(int p_what);
};

#endif // GOSU_H
