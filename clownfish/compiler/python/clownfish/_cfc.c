
#include "Python.h"
#include "CFC.h"

typedef struct {
    PyObject_HEAD
    void *cfc_obj;
} CFCPyWrapper;

static CFCHierarchy*
S_extract_hierarchy(PyObject *wrapper) {
    return (CFCHierarchy*)((CFCPyWrapper*)wrapper)->cfc_obj;
}

static CFCPyWrapper*
S_CFCHierarchy_new(PyTypeObject *type, PyObject *args,
                   PyObject *keyword_args) {
    char *dest = NULL;
    char *keywords[] = {"dest", NULL};
    int result = PyArg_ParseTupleAndKeywords(args, keyword_args, "|s",
                                             keywords, &dest);
    if (!result) { return NULL; }
    if (!dest) {
        PyErr_SetString(PyExc_TypeError, "Missing required arg 'dest'");
        return NULL;
    }
    CFCPyWrapper *wrapper = (CFCPyWrapper*)type->tp_alloc(type, 0);
    if (wrapper) {
        wrapper->cfc_obj = CFCHierarchy_new(dest);
    }
    return wrapper;
}

static void
S_CFCHierarchy_dealloc(CFCPyWrapper *wrapper) {
    CFCBase *temp = (CFCBase*)wrapper->cfc_obj;
    wrapper->cfc_obj = NULL;
    CFCBase_decref(temp);
    Py_TYPE(wrapper)->tp_free(wrapper);
}

static PyObject*
S_CFCHierarchy_add_include_dir(PyObject *wrapper, PyObject *dir) {
    CFCHierarchy *wrapped  = S_extract_hierarchy(wrapper);
    CFCHierarchy_add_include_dir(S_extract_hierarchy(wrapper),
                                 PyUnicode_AsUTF8(dir));
    Py_RETURN_NONE;
}

static PyModuleDef cfc_module_def = {
    PyModuleDef_HEAD_INIT,
    "clownfish.cfc",
    "CFC: Clownfish compiler",
    -1,
    NULL, NULL, NULL, NULL, NULL
};

static PyModuleDef cfc_model_module_def = {
    PyModuleDef_HEAD_INIT,
    "clownfish.cfc.model",
    "CFC classes which model language constructs",
    -1,
    NULL, NULL, NULL, NULL, NULL
};

static PyMethodDef hierarchy_methods[] = {
    {"add_include_dir", (PyCFunction)S_CFCHierarchy_add_include_dir, METH_O,
     NULL},
    {NULL}
};

static PyTypeObject CFCHierarchy_pytype = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "clownfish.cfc.model.Hierarchy",    // tp_name
    sizeof(CFCPyWrapper),               // tp_basicsize
    0,                                  // tp_itemsize
    (destructor)S_CFCHierarchy_dealloc, // tp_dealloc
    0,                                  // tp_print
    0,                                  // tp_getattr
    0,                                  // tp_setattr
    0,                                  // tp_reserved
    0,                                  // tp_repr
    0,                                  // tp_as_number
    0,                                  // tp_as_sequence
    0,                                  // tp_as_mapping
    0,                                  // tp_hash
    0,                                  // tp_call
    0,                                  // tp_str
    0,                                  // tp_getattro
    0,                                  // tp_setattro
    0,                                  // tp_as_buffer
    Py_TPFLAGS_DEFAULT,                 // tp_flags
    "CFCHierarchy",                     // tp_doc
    0,                                  // tp_traverse
    0,                                  // tp_clear
    0,                                  // tp_richcompare
    0,                                  // tp_weaklistoffset
    0,                                  // tp_iter
    0,                                  // tp_iternext
    hierarchy_methods,                  // tp_methods
    0,                                  // tp_members
    0,                                  // tp_getset
    0,                                  // tp_base
    0,                                  // tp_dict
    0,                                  // tp_descr_get
    0,                                  // tp_descr_set
    0,                                  // tp_dictoffset
    0,                                  // tp_init
    0,                                  // tp_allow
    (newfunc)S_CFCHierarchy_new         // tp_new
};

PyMODINIT_FUNC
PyInit__cfc(void) {
    if (PyType_Ready(&CFCHierarchy_pytype) < 0) {
        return NULL;
    }
    PyObject *cfc_module = PyModule_Create(&cfc_module_def);
    PyObject *cfc_model_module = PyModule_Create(&cfc_model_module_def);
    PyModule_AddObject(cfc_module, "model", (PyObject*)cfc_model_module);
    Py_INCREF(&CFCHierarchy_pytype);
    PyModule_AddObject(cfc_model_module, "Hierarchy",
                       (PyObject*)&CFCHierarchy_pytype);

    return cfc_module;
}


