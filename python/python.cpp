
#include "stdafx.h"
#include "util/fixes.h"
#include "util/addresses.h"
#include "dependencies/python-2.2/Python.h"
#include "temple_functions.h"
#include "python_header.h"
#include "testhelper.h"
#include "python_debug.h"
#include "party.h"
#include "common.h"
#include "spell.h"
#include "d20.h"
#include "action_sequence.h"
#include <ui/ui_picker.h>
#include <location.h>
#include <turn_based.h>

static struct PythonInternal : AddressTable {

	/*
		The python dictionary that contains all global Python objects.
	*/
	PyObject **globals;

	/*
		Initialization function for Python.
	*/
	int (__cdecl *InitPython)();

	PythonInternal() {
		rebase(globals, 0x10BCA764);
		rebase(InitPython, 0x100ADA30);
	}

} pythonInternal;

#pragma region Helper Functions

static bool isTypeCritter(int nObjType){
	if (nObjType == obj_t_pc || nObjType == obj_t_npc){
		return 1;
	}
	return 0;
};

static bool isTypeContainer(int nObjType){
	if (nObjType == obj_t_container || nObjType == obj_t_bag ){
		return 1;
	}
	return 0;
};

static _fieldIdx objGetInventoryListField(TemplePyObjHandle* obj){
	int objType = objects.GetType(obj->objHandle);  //templeFuncs.Obj_Get_Field_32bit(obj->objHandle, obj_f_type);
	if (isTypeCritter(objType)){
		return obj_f_critter_inventory_list_idx;
	}
	else if (isTypeContainer(objType)){
		return obj_f_container_inventory_list_idx;
	};
	return 0;
}

#pragma endregion

#pragma region Python Obj Field Get/Set Function Implementation

static PyObject * pyObjHandleType_Get_Field_64bit(TemplePyObjHandle* obj, PyObject * pyTupleIn){
	_fieldIdx nFieldIdx = 0;
	uint64_t n64 = 0;
	if (!PyArg_ParseTuple(pyTupleIn, "i", &nFieldIdx)) {
		return nullptr;
	};
	return PyLong_FromLongLong( templeFuncs.Obj_Get_Field_64bit(obj->objHandle, nFieldIdx) );
};

static PyObject * pyObjHandleType_Set_Field_64bit(TemplePyObjHandle* obj, PyObject * pyTupleIn){
	_fieldIdx nFieldIdx = 0;
	uint64_t n64 = 0;
	if (!PyArg_ParseTuple(pyTupleIn, "iL", &nFieldIdx, &n64)) {
		return PyInt_FromLong(0);
	};
	templeFuncs.Obj_Set_Field_64bit(obj->objHandle, nFieldIdx, n64);
	return PyInt_FromLong(1);
};

static PyObject * pyObjHandleType_Get_Field_ObjHandle(TemplePyObjHandle* obj, PyObject * pyTupleIn){
	_fieldIdx nFieldIdx = 0;
	objHndl ObjHnd = 0;
	if (!PyArg_ParseTuple(pyTupleIn, "i", &nFieldIdx)) {
		return nullptr;
	};
	return PyLong_FromLongLong(templeFuncs.Obj_Get_Field_ObjHnd__fastout(obj->objHandle, nFieldIdx));
};

static PyObject * pyObjHandleType_Set_Field_ObjHandle(TemplePyObjHandle* obj, PyObject * pyTupleIn){
	_fieldIdx nFieldIdx = 0;
	objHndl ObjHnd = 0;
	if (!PyArg_ParseTuple(pyTupleIn, "iL", &nFieldIdx, &ObjHnd)) {
		return PyInt_FromLong(0);
	};
	templeFuncs.Obj_Set_Field_ObjHnd(obj->objHandle, nFieldIdx, ObjHnd);
	return PyInt_FromLong(1);
};

static PyObject * pyObjHandleType_Get_IdxField_32bit(TemplePyObjHandle* obj, PyObject * pyTupleIn){
	_fieldIdx nFieldIdx = 0;
	_fieldSubIdx nFieldSubIdx = 0;
	if (!PyArg_ParseTuple(pyTupleIn, "ii", &nFieldIdx, &nFieldSubIdx)) {
		return nullptr;
	};
	uint32_t n32 = templeFuncs.Obj_Get_IdxField_32bit(obj->objHandle, nFieldIdx, nFieldSubIdx);
	return PyInt_FromLong(n32);
};

static PyObject * pyObjHandleType_Set_IdxField_32bit(TemplePyObjHandle* obj, PyObject * pyTupleIn){
	_fieldIdx nFieldIdx = 0;
	_fieldSubIdx nFieldSubIdx = 0;
	uint32_t n32 = 0;
	if (!PyArg_ParseTuple(pyTupleIn, "iii", &nFieldIdx, &nFieldSubIdx, &n32)) {
		return PyInt_FromLong(0);
	};
	templeFuncs.Obj_Set_IdxField_byValue(obj->objHandle, nFieldIdx, nFieldSubIdx, n32);
	return PyInt_FromLong(1);
};

static PyObject * pyObjHandleType_Get_IdxField_64bit(TemplePyObjHandle* obj, PyObject * pyTupleIn){
	_fieldIdx nFieldIdx = 0;
	_fieldSubIdx nFieldSubIdx = 0;
	if (!PyArg_ParseTuple(pyTupleIn, "ii", &nFieldIdx, &nFieldSubIdx)) {
		return nullptr;
	};
	uint64_t n64 = templeFuncs.Obj_Get_IdxField_64bit(obj->objHandle, nFieldIdx, nFieldSubIdx);
	return PyLong_FromLongLong(n64);
};

static PyObject * pyObjHandleType_Set_IdxField_64bit(TemplePyObjHandle* obj, PyObject * pyTupleIn){
	_fieldIdx nFieldIdx = 0;
	_fieldSubIdx nFieldSubIdx = 0;
	uint64_t n64 = 0;
	if (!PyArg_ParseTuple(pyTupleIn, "iiL", &nFieldIdx, &nFieldSubIdx, &n64)) {
		return PyInt_FromLong(0);
	};
	templeFuncs.Obj_Set_IdxField_byValue(obj->objHandle, nFieldIdx, nFieldSubIdx, n64);
	return PyInt_FromLong(1);
};


#pragma endregion

#pragma region Python Obj Inventory Manipulation
static PyObject * pyObjHandleType_Inventory(TemplePyObjHandle* obj, PyObject * pyTupleIn){
	int nArgs = PyTuple_Size(pyTupleIn);
	objHndl ObjHnd = obj->objHandle;
	int nModeSelect = 0;
	bool bIncludeBackpack = 1;
	bool bIncludeEquipped = 0;
	bool bRetunProtos = 0;
	int invFieldType = inventory.GetInventoryListField(obj->objHandle);

	int nItems = templeFuncs.Obj_Get_IdxField_NumItems(ObjHnd, invFieldType);

	if (nArgs == 1){ // returns the entire non-worn inventory
		if (!PyArg_ParseTuple(pyTupleIn, "i", &nModeSelect)) {
			return nullptr;
		}
		else if (nModeSelect == 1){
			bIncludeEquipped = 1;
		}
		else if (nModeSelect == 2){
			bIncludeBackpack = 0;
			bIncludeEquipped = 1;
		};
	};

	objHndl ItemObjHnds[8192] = {}; // seems large enough to cover any practical case :P

	int nMax = CRITTER_MAX_ITEMS;
	if (invFieldType == obj_f_container_inventory_list_idx){
		nMax = CONTAINER_MAX_ITEMS;
	};

	int j = 0;
	if (bIncludeBackpack){
		for (int i = 0; (j < nItems) & (i < nMax); i++){
			ItemObjHnds[j] = objects.inventory.GetItemAtInventoryLocation(ObjHnd, i);
			if (ItemObjHnds[j]){ j++; };
		}
	};

	if (bIncludeEquipped){
		for (int i = CRITTER_EQUIPPED_ITEM_OFFSET; (j < nItems) & (i < CRITTER_EQUIPPED_ITEM_OFFSET + CRITTER_EQUIPPED_ITEM_SLOTS); i++){
			ItemObjHnds[j] = objects.inventory.GetItemAtInventoryLocation(ObjHnd, i);
			if (ItemObjHnds[j]){ j++; };
		}
	};

	auto ItemPyTuple = PyTuple_New(j);
	if (!bRetunProtos){
		for (int i = 0; i < j; i++){
			PyTuple_SetItem(ItemPyTuple, i, templeFuncs.PyObjFromObjHnd(ItemObjHnds[i]));
		}
	}
	else {
		//TODO
	}

	return ItemPyTuple;
};

static PyObject * pyObjHandleType_Inventory_Item(TemplePyObjHandle* obj, PyObject * pyTupleIn){
	int nArgs = PyTuple_Size(pyTupleIn);
	objHndl ObjHnd = obj->objHandle;
	bool bRetunProtos = 0;
	int invFieldType = inventory.GetInventoryListField(obj->objHandle);
	int n = 0;
	if (PyArg_ParseTuple(pyTupleIn, "i", &n)){
		int nMax = CRITTER_MAX_ITEMS;
		if (invFieldType == obj_f_container_inventory_list_idx){
			nMax = CONTAINER_MAX_ITEMS;
		};
		if (n < nMax){
			return templeFuncs.PyObjFromObjHnd(objects.inventory.GetItemAtInventoryLocation(ObjHnd, n));
		};
		
	};
	return templeFuncs.PyObjFromObjHnd(0);

};

#pragma endregion

static PyObject * pyObjHandleType_Remove_From_All_Groups(TemplePyObjHandle* obj, PyObject * pyTupleIn){
	party.ObjRemoveFromAllGroupArrays(obj->objHandle);
	return PyInt_FromLong(1);
};

static PyObject * pyObjHandleType_PC_Add(TemplePyObjHandle* obj, PyObject * pyTupleIn){
	// TODO  add fluidity to number of PCs / NPCs
	party.ObjAddToPCGroup(obj->objHandle);
	return PyInt_FromLong(1);
};


UiPickerType operator&(const UiPickerType& lhs, const UiPickerType& rhs){
	return (UiPickerType)((uint64_t)lhs & (uint64_t)rhs);
};

static PyObject * pyObjHandleType_ObjFeatAdd(TemplePyObjHandle* obj, PyObject * pyTupleIn){
	feat_enums nFeatCode;
	if (!PyArg_ParseTuple(pyTupleIn, "i", &nFeatCode)) {
		return nullptr;
	};

	if (nFeatCode == 0){
		return PyInt_FromLong(0);
	}

	objects.feats.FeatAdd(obj->objHandle, nFeatCode);
	objects.d20.d20Status->D20StatusInit(obj->objHandle);

	return PyInt_FromLong(1);
};


static PyObject * pyObjHandleType_MakeWizard(TemplePyObjHandle* obj, PyObject * pyTupleIn){
	uint32_t level;
	if (!PyArg_ParseTuple(pyTupleIn, "i", &level)) {
		return nullptr;
	};

	if (level <= 0 || level > 20){
		return PyInt_FromLong(0);
	}
	for (uint32_t i = 0; i < level; i++)
	{
		templeFuncs.Obj_Set_IdxField_byValue(obj->objHandle, obj_f_critter_level_idx, i, stat_level_wizard);
	}

	objects.d20.d20Status->D20StatusInit(obj->objHandle);
	

	return PyInt_FromLong(1);
};

/*
static PyObject * pyObjHandleType_Set_IdxField_byValue(TemplePyObjHandle* obj, PyObject * pyTupleIn){
_fieldIdx nFieldIdx = 0;
_fieldSubIdx nFieldSubIdx = 0;
va_list dataIn; 
if (!PyArg_ParseTuple(pyTupleIn, "iiL", &nFieldIdx, &nFieldSubIdx, &dataIn)) {
return PyInt_FromLong(0);
};
templeFuncs.Obj_Set_IdxField_byValue(obj->objHandle, nFieldIdx, nFieldSubIdx, dataIn);
return PyInt_FromLong(1);
}; */


static PyMethodDef pyObjHandleMethods_New[] = {
	"inventory", (PyCFunction)pyObjHandleType_Inventory, METH_VARARGS, "Fetches a tuple of the object's inventory (items are Python Objects). Optional argument int nModeSelect : 0 - backpack only (excludes equipped items); 1 - backpack + equipped; 2 - equipped only",
	"inventory_item", (PyCFunction)pyObjHandleType_Inventory_Item, METH_VARARGS, "Fetches an inventory item of index n",
	"obj_get_field_64bit", (PyCFunction)pyObjHandleType_Get_Field_64bit, METH_VARARGS, "Gets 64 bit field",
	"obj_set_field_64bit", (PyCFunction)pyObjHandleType_Set_Field_64bit, METH_VARARGS, "Sets 64 bit field",
	"obj_get_field_objHndl", (PyCFunction)pyObjHandleType_Get_Field_ObjHandle, METH_VARARGS, "Gets objHndl field",
	"obj_set_field_objHndl", (PyCFunction)pyObjHandleType_Set_Field_ObjHandle, METH_VARARGS, "Sets objHndl field",
	"obj_get_idxfield_32bit", (PyCFunction)pyObjHandleType_Get_IdxField_32bit, METH_VARARGS, "Gets 32 bit index field",
	"obj_set_idxfield_32bit", (PyCFunction)pyObjHandleType_Set_IdxField_32bit, METH_VARARGS, "Sets 32 bit index field",
	"obj_get_idxfield_64bit", (PyCFunction)pyObjHandleType_Get_IdxField_64bit, METH_VARARGS, "Gets 64 bit index field",
	"obj_set_idxfield_64bit", (PyCFunction)pyObjHandleType_Set_IdxField_64bit, METH_VARARGS, "Sets 64 bit index field",
	//"obj_set_idxfield_byvalue", (PyCFunction)pyObjHandleType_Set_IdxField_byValue, METH_VARARGS, "Sets index field - general (depending on nFieldIndex which the game looks up and fetches nFieldType to determine data size)",
	"obj_remove_from_all_groups", (PyCFunction)pyObjHandleType_Remove_From_All_Groups, METH_VARARGS, "Removes the object from all the groups (GroupList, PCs, NPCs, AI controlled followers, Currently Selected",
	"pc_add", (PyCFunction)pyObjHandleType_PC_Add, METH_VARARGS, "Adds object as a PC party member.",
	"objfeatadd", (PyCFunction)pyObjHandleType_ObjFeatAdd, METH_VARARGS, "Adds a feat. Feats can be added multiple times (at least some of them? like Toughness)",
	"makewiz", (PyCFunction)pyObjHandleType_MakeWizard, METH_VARARGS, "Makes character a wizard of level n",
	//
	0, 0, 0, 0
};



PyObject* __cdecl  pyObjHandleType_getAttrNew(TemplePyObjHandle *obj, char *name) {
	#pragma region PyObjHandle Members - FINISHED
	if (!_strcmpi(name, "ObjHandle")) {
		return  PyLong_FromLongLong(obj->objHandle); 
	}
	else if (!_strcmpi(name, "proto"))
	{
		return  PyLong_FromLongLong(objects.ObjGetProtoNum(obj->objHandle));
	}
	else if (!_strcmpi(name, "description")){
		return  PyString_FromString(objects.description._getDisplayName(obj->objHandle,obj->objHandle));
	}

	if (!_strcmpi(name, "factions")) {
		objHndl ObjHnd = obj->objHandle;
		int a[50] = {};
		int n = 0;

		for (int i = 0; i < 50; i ++){
			int fac = templeFuncs.Obj_Get_IdxField_32bit(ObjHnd, obj_f_npc_faction, i);
			if (fac == 0) break;
			a[i] = fac;
			n++;
		};

		auto outTup = PyTuple_New(n);
		for (int i = 0; i < n ; i++){
			PyTuple_SetItem(outTup, i, PyInt_FromLong(a[i]));
		};

		
		return  outTup; 
	}
	
	if (!_strcmpi(name, "faction_has")) {
		return Py_FindMethod(pyObjHandleMethods_New, obj, "faction_has");
	}
	else if (!_strcmpi(name, "faction_add"))
	{
		return Py_FindMethod(pyObjHandleMethods_New, obj, "faction_add");
	} 
	else if (!_strcmpi(name, "substitute_inventory"))
	{
		objHndl ObjSubsInv = objects.inventory.GetSubstituteInventory(obj->objHandle);
		return templeFuncs.PyObjFromObjHnd(ObjSubsInv);
	}
	else if (!_strcmpi(name, "feat_add")) {
		return Py_FindMethod(pyObjHandleMethods_New, obj, "objfeatadd");
		
	};

	#pragma endregion


	#pragma region TODO
	if (!_strcmpi(name, "inventory")) {
		return Py_FindMethod(pyObjHandleMethods_New, obj, "inventory");
	}
	else if (!_strcmpi(name, "inventory_item")){
		return Py_FindMethod(pyObjHandleMethods_New, obj, "inventory_item");
	}
	else if (!_strcmpi(name, "inventory_room_left")){
		return Py_FindMethod(pyObjHandleMethods_New, obj, "inventory_room_left");
	};


	if (!_strcmpi(name, "pc_stay_behind")){
		return Py_FindMethod(pyObjHandleMethods_New, obj, "obj_remove_from_all_groups");
	}
	else if (!_strcmpi(name, "pc_add")){
		return Py_FindMethod(pyObjHandleMethods_New, obj, "pc_add");
	};

	#pragma endregion


	#pragma region Generic Get/Set Funcs
	if (!_strcmpi(name, "obj_get_field_64bit")) {
		return Py_FindMethod(pyObjHandleMethods_New, obj, "obj_get_field_64bit");
	}
	else if (!_strcmpi(name, "obj_get_field_objHndl")){
		return Py_FindMethod(pyObjHandleMethods_New, obj, "obj_get_field_objHndl");
	}
	else if (!_strcmpi(name, "obj_get_idxfield_numitems")){
		return Py_FindMethod(pyObjHandleMethods_New, obj, "obj_get_idxfield_numitems");
	}
	else if (!_strcmpi(name, "obj_get_idxfield_32bit")){
		return Py_FindMethod(pyObjHandleMethods_New, obj, "obj_get_idxfield_32bit");
	}
	else if (!_strcmpi(name, "obj_get_idxfield_64bit")){
		return Py_FindMethod(pyObjHandleMethods_New, obj, "obj_get_idxfield_64bit");
	}
	else if (!_strcmpi(name, "obj_get_idxfield_objHndl")){
		return Py_FindMethod(pyObjHandleMethods_New, obj, "obj_get_idxfield_objhandle");
	}
	else if (!_strcmpi(name, "obj_get_idxfield_256bit")){
		return Py_FindMethod(pyObjHandleMethods_New, obj, "obj_get_idxfield_256bit");
	}
	else if (!_strcmpi(name, "obj_set_field_64bit")) {
		return Py_FindMethod(pyObjHandleMethods_New, obj, "obj_set_field_64bit");
	}
	else if (!_strcmpi(name, "obj_set_field_objHndl")) {
		return Py_FindMethod(pyObjHandleMethods_New, obj, "obj_set_field_objHndl");
	}
	else if (!_strcmpi(name, "obj_set_idxfield_32bit")){
		return Py_FindMethod(pyObjHandleMethods_New, obj, "obj_set_idxfield_32bit");
	}
	else if (!_strcmpi(name, "obj_set_idxfield_64bit")){
		return Py_FindMethod(pyObjHandleMethods_New, obj, "obj_set_idxfield_64bit");
	};
	#pragma endregion

	if (!_strcmpi(name, "makewiz")) {
		return Py_FindMethod(pyObjHandleMethods_New, obj, "makewiz");
		
	};

	if (!_strcmpi(name, "cast_spell")) {
	//	return Py_FindMethod(pyObjHandleMethods_New, obj, "cast_spell");

	};
	

	return pyObjHandleTypeGetAttr(obj, name);
}

int __cdecl  pyObjHandleType_setAttrNew(TemplePyObjHandle *obj, char *name, TemplePyObjHandle *obj2) {
	logger->info("Tried setting property: {}", name);
	if (!strcmp(name, "co8rocks")) {
		return 0;
	}

	if (!strcmp(name, "substitute_inventory")) {

		if (obj2 != nullptr)  {
			if (obj->ob_type == obj2->ob_type){
				templeFuncs.Obj_Set_Field_ObjHnd(obj->objHandle, obj_f_npc_substitute_inventory, obj2->objHandle);
			}
		}
		return 0;
	}


	return pyObjHandleTypeSetAttr(obj, name, obj2);
}
