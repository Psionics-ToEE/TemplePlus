
#include "stdafx.h"
#include "fixes.h"
#include "addresses.h"
#include "dependencies/python-2.2/Python.h"

/*
	Define all type objects used by ToEE.
*/
static GlobalStruct<PyTypeObject, 0x102CF3B8> pyObjHandleType;
static getattrfunc pyObjHandleTypeGetAttr; // Original getattr of pyObjHandleType

PyObject* __cdecl  pyObjHandleType_getAttrNew(PyObject *obj, char *name) {
	LOG(info) << "Tried getting property: " << name;
	if (!strcmp(name, "co8rocks")) {
		return PyString_FromString("IT SURE DOES!");
	}

	return pyObjHandleTypeGetAttr(obj, name);
}

class PythonExtensions : public TempleFix {
public:
	const char* name() override {
		return "Python Script Extensions";
	}
	void apply() override;
} pythonExtension;

void PythonExtensions::apply() {

	// Hook the getattr function of obj handles
	pyObjHandleTypeGetAttr = pyObjHandleType->tp_getattr;
	pyObjHandleType->tp_getattr = pyObjHandleType_getAttrNew;


}
