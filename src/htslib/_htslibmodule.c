/*
Copyright (C) 2026 Leiden University Medical Center
This file is part of python-htslib

Python-htslib is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the Free
Software Foundation, either version 3 of the License, or (at your option) any
later version.

Python-htslib is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License along with
python-htslib. If not, see <https://www.gnu.org/licenses/>.
*/

/* Py_LIMITED_API set to 3.11 */
#define Py_LIMITED_API 0x030B0000
#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <htslib/hts.h>


struct HtslibModuleState {
    PyTypeObject *PythonArray_Type;  // array.array
};

/*************************
 * MODULE INITIALIZATION *
 *************************/

/* A C implementation of from module_name import class_name*/
static PyTypeObject *
ImportClassFromModule(const char *module_name, const char *class_name)
{
    PyObject *module = PyImport_ImportModule(module_name);
    if (module == NULL) {
        return NULL;
    }
    PyObject *type_object = PyObject_GetAttrString(module, class_name);
    Py_DECREF(module);
    if (type_object == NULL) {
        return NULL;
    }
    if (!PyType_CheckExact(type_object)) {
        PyErr_Format(PyExc_RuntimeError, "%s.%s is not a type class but, %R",
                     module_name, class_name, Py_TYPE((PyObject *)type_object));
        return NULL;
    }
    return (PyTypeObject *)type_object;
}

/**
 * @brief Add a new type to the module initiated from spec. Return NULL on
 * failure. Returns the type on success.
 */
static PyTypeObject *
python_module_add_type_spec(PyObject *module, PyType_Spec *spec)
{
    const char *class_name = strchr(spec->name, '.');
    if (class_name == NULL) {
        return NULL;
    }
    class_name += 1;  // Use the part after the dot.

    PyTypeObject *type =
        (PyTypeObject *)PyType_FromModuleAndSpec(module, spec, NULL);
    if (type == NULL) {
        return NULL;
    }

    if (PyModule_AddObjectRef(module, class_name, (PyObject *)type) != 0) {
        Py_DECREF(type);
        return NULL;
    }
    return type;
}

struct AddressAndSpec {
    PyTypeObject **address;
    PyType_Spec *spec;
};

static int
_htslib_exec(PyObject *module)
{
    struct HtslibModuleState *state = PyModule_GetState(module);

    PyTypeObject *PythonArray = ImportClassFromModule("array", "array");
    if (PythonArray == NULL) {
        return -1;
    }
    state->PythonArray_Type = PythonArray;
    return 0;
}

static int
_htslib_traverse(PyObject *mod, visitproc visit, void *arg)
{
    PyTypeObject **mod_state_types = PyModule_GetState(mod);
    if (mod_state_types == NULL) {
        return -1;
    }
    size_t number_of_types =
        sizeof(struct HtslibModuleState) / sizeof(PyTypeObject *);
    for (size_t i = 0; i < number_of_types; i++) {
        PyTypeObject *tp = mod_state_types[i];
        Py_VISIT(tp);
    }
    return 0;
}

static int
_htslib_clear(PyObject *mod)
{
    PyTypeObject **mod_state_types = PyModule_GetState(mod);
    if (mod_state_types == NULL) {
        return -1;
    }
    size_t number_of_types =
        sizeof(struct HtslibModuleState) / sizeof(PyTypeObject *);
    for (size_t i = 0; i < number_of_types; i++) {
        Py_XDECREF((PyObject *)mod_state_types[i]);
        mod_state_types[i] = NULL;
    }
    return 0;
}

static void
_htslib_free(void *mod)
{
    _htslib_clear((PyObject *)mod);
}

static PyModuleDef_Slot _htslib_module_slots[] = {
    {Py_mod_exec, _htslib_exec},
    {0, NULL},
};

static struct PyModuleDef _htslib_module = {
    PyModuleDef_HEAD_INIT,
    .m_name = "_htslib",
    .m_doc = NULL,
    .m_size = sizeof(struct HtslibModuleState),
    .m_methods = NULL,
    .m_slots = _htslib_module_slots,
    .m_traverse = _htslib_traverse,
    .m_clear = _htslib_clear,
    .m_free = _htslib_free,
};

PyMODINIT_FUNC
PyInit__htslib(void)
{
    return PyModuleDef_Init(&_htslib_module);
}
