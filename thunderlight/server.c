#include "server.h"
#include "protocol.h"


static int Server_init(Server *self, PyObject *args, PyObject *kwds) {
    PyArg_ParseTuple(args, "OO", &self->app, &self->port);
    Py_INCREF(((Server *)self)->app);
    return 0;
}

static void Server_dealloc(Server *self) {
    Py_DECREF(self->app);
    Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *Server_call(Server *self, PyObject *args, PyObject *kwargs) {
    return (PyObject *)Protocol_native_new(self->app);
}

static PyObject *Server_listen(Server *self) {
    PyObject *uvloop = PyImport_ImportModule("uvloop");
    PyObject *new_event_loop = PyObject_GetAttrString(uvloop, "new_event_loop");
    PyObject *loop = PyObject_CallNoArgs(new_event_loop);
    PyObject *asyncio = PyImport_ImportModule("asyncio");
    PyObject *set_event_loop = PyObject_GetAttrString(asyncio, "set_event_loop");
    PyObject_CallOneArg(set_event_loop, loop);
    PyObject *create_server = PyObject_GetAttrString(loop, "create_server");
    PyObject *args = PyTuple_New(1);
    PyTuple_SetItem(args, 0, (PyObject *)self);
    PyObject *kwargs = PyDict_New();
    PyDict_SetItemString(kwargs, "port", self->port);
    PyObject *server_coro = PyObject_Call(create_server, args, kwargs);
    Py_DECREF(args);
    Py_DECREF(kwargs);
    PyObject *run_until_complete = PyObject_GetAttrString(loop, "run_until_complete");
    PyObject *server = PyObject_CallOneArg(run_until_complete, server_coro);
    PyObject *run_forever = PyObject_GetAttrString(loop, "run_forever");
    PyObject *none = PyObject_CallNoArgs(run_forever);
    Py_RETURN_NONE;
}

static PyMethodDef Server_methods[] = {
    {"listen", (PyCFunction)Server_listen, METH_NOARGS, "Start the server."},
    {NULL}
};

static PyTypeObject ServerType = {
    .tp_name = "server.Server",
    .tp_itemsize = sizeof(Server),
    .tp_alloc = PyType_GenericAlloc,
    .tp_new = PyType_GenericNew,
    .tp_init = (initproc)Server_init,
    .tp_dealloc = (destructor)Server_dealloc,
    .tp_call = (ternaryfunc)Server_call,
    .tp_methods = Server_methods
};

static PyModuleDef Server_module = {
    PyModuleDef_HEAD_INIT,
    "server",
    "server",
    -1,
    NULL, NULL, NULL, NULL, NULL
};

PyMODINIT_FUNC
PyInit_server(void) {
    PyObject* m = NULL;
    if (PyType_Ready(&ServerType) < 0) {
        goto error;
    }
    m = PyModule_Create(&Server_module);
    Py_INCREF(&ServerType);
    PyModule_AddObject(m, "Server", (PyObject *)&ServerType);
    if (!m) {
        goto error;
    }
    goto finally;
error:
    m = NULL;
finally:
    return m;
}
