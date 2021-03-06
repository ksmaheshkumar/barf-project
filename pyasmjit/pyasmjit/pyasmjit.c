#include <stdio.h>
#include <string.h>
#include <sys/mman.h>

#include "Python.h"

typedef struct {
    unsigned long rax;
    unsigned long rbx;
    unsigned long rcx;
    unsigned long rdx;
    unsigned long rdi;
    unsigned long rsi;
    unsigned long rbp;
    unsigned long rsp;
    unsigned long rip;
    unsigned long r8;
    unsigned long r9;
    unsigned long r10;
    unsigned long r11;
    unsigned long r12;
    unsigned long r13;
    unsigned long r14;
    unsigned long r15;
    unsigned long rflags;
} context_t;

void
print_context(context_t *ctx)
{
    printf("ctx @ %p\n", ctx);

    printf("   rax : 0x%016lx @ %p\n",    ctx->rax,    &ctx->rax);
    printf("   rbx : 0x%016lx @ %p\n",    ctx->rbx,    &ctx->rbx);
    printf("   rcx : 0x%016lx @ %p\n",    ctx->rcx,    &ctx->rcx);
    printf("   rdx : 0x%016lx @ %p\n",    ctx->rdx,    &ctx->rdx);
    printf("   rdi : 0x%016lx @ %p\n",    ctx->rdi,    &ctx->rdi);
    printf("   rsi : 0x%016lx @ %p\n",    ctx->rsi,    &ctx->rsi);
    printf("   rbp : 0x%016lx @ %p\n",    ctx->rbp,    &ctx->rbp);
    printf("   rsp : 0x%016lx @ %p\n",    ctx->rsp,    &ctx->rsp);
    printf("   rip : 0x%016lx @ %p\n",    ctx->rip,    &ctx->rip);
    printf("   r8  : 0x%016lx @ %p\n",     ctx->r8,     &ctx->r8);
    printf("   r9  : 0x%016lx @ %p\n",     ctx->r9,     &ctx->r9);
    printf("   r10 : 0x%016lx @ %p\n",    ctx->r10,    &ctx->r10);
    printf("   r11 : 0x%016lx @ %p\n",    ctx->r11,    &ctx->r11);
    printf("   r12 : 0x%016lx @ %p\n",    ctx->r12,    &ctx->r12);
    printf("   r13 : 0x%016lx @ %p\n",    ctx->r13,    &ctx->r13);
    printf("   r14 : 0x%016lx @ %p\n",    ctx->r14,    &ctx->r14);
    printf("   r15 : 0x%016lx @ %p\n",    ctx->r15,    &ctx->r15);
    printf("rflags : 0x%016lx @ %p\n", ctx->rflags, &ctx->rflags);
}

unsigned long
load_register_from_dict(PyObject *dict, const char *reg, unsigned long _default)
{
    unsigned long rv = _default;

    if (PyDict_Contains(dict, Py_BuildValue("s", reg)) == 1) {
        rv = PyLong_AsUnsignedLong(PyDict_GetItemString(dict, reg));
    }

    return rv;
}

void
load_context_from_dict(PyObject *dict, context_t *ctx)
{
    ctx->rax    = load_register_from_dict(dict,    "rax",     0);
    ctx->rbx    = load_register_from_dict(dict,    "rbx",     0);
    ctx->rcx    = load_register_from_dict(dict,    "rcx",     0);
    ctx->rdx    = load_register_from_dict(dict,    "rdx",     0);
    ctx->rdi    = load_register_from_dict(dict,    "rdi",     0);
    ctx->rsi    = load_register_from_dict(dict,    "rsi",     0);
    ctx->rbp    = load_register_from_dict(dict,    "rbp",     0);
    ctx->rsp    = load_register_from_dict(dict,    "rsp",     0);
    ctx->rip    = load_register_from_dict(dict,    "rip",     0);
    ctx->r8     = load_register_from_dict(dict,     "r8",     0);
    ctx->r9     = load_register_from_dict(dict,     "r9",     0);
    ctx->r10    = load_register_from_dict(dict,    "r10",     0);
    ctx->r11    = load_register_from_dict(dict,    "r11",     0);
    ctx->r12    = load_register_from_dict(dict,    "r12",     0);
    ctx->r13    = load_register_from_dict(dict,    "r13",     0);
    ctx->r14    = load_register_from_dict(dict,    "r14",     0);
    ctx->r15    = load_register_from_dict(dict,    "r15",     0);
    ctx->rflags = load_register_from_dict(dict, "rflags", 0x202);
}

void
save_context_to_dict(PyObject *dict, context_t *ctx)
{
    PyDict_SetItemString(dict,    "rax", Py_BuildValue("I",    ctx->rax));
    PyDict_SetItemString(dict,    "rbx", Py_BuildValue("I",    ctx->rbx));
    PyDict_SetItemString(dict,    "rcx", Py_BuildValue("I",    ctx->rcx));
    PyDict_SetItemString(dict,    "rdx", Py_BuildValue("I",    ctx->rdx));
    PyDict_SetItemString(dict,    "rdi", Py_BuildValue("I",    ctx->rdi));
    PyDict_SetItemString(dict,    "rsi", Py_BuildValue("I",    ctx->rsi));
    PyDict_SetItemString(dict,    "rbp", Py_BuildValue("I",    ctx->rbp));
    PyDict_SetItemString(dict,    "rsp", Py_BuildValue("I",    ctx->rsp));
    PyDict_SetItemString(dict,    "rip", Py_BuildValue("I",    ctx->rip));
    PyDict_SetItemString(dict,     "r8", Py_BuildValue("I",     ctx->r8));
    PyDict_SetItemString(dict,     "r9", Py_BuildValue("I",     ctx->r9));
    PyDict_SetItemString(dict,    "r10", Py_BuildValue("I",    ctx->r10));
    PyDict_SetItemString(dict,    "r11", Py_BuildValue("I",    ctx->r11));
    PyDict_SetItemString(dict,    "r12", Py_BuildValue("I",    ctx->r12));
    PyDict_SetItemString(dict,    "r13", Py_BuildValue("I",    ctx->r13));
    PyDict_SetItemString(dict,    "r14", Py_BuildValue("I",    ctx->r14));
    PyDict_SetItemString(dict,    "r15", Py_BuildValue("I",    ctx->r15));
    PyDict_SetItemString(dict, "rflags", Py_BuildValue("I", ctx->rflags));
}

unsigned long
run(unsigned char *data, unsigned int size, context_t *ctx) {
    /* Allocate executable memory */
    void *mem = mmap(
        NULL,
        size,
        PROT_WRITE | PROT_EXEC,
        MAP_ANONYMOUS | MAP_PRIVATE | MAP_32BIT,
        -1,
        0
    );

    /* Return on error */
    if (mem == MAP_FAILED) {
        return -1;
    }

    /* Copy binary code into allocated memory */
    memcpy(mem, data, size);

    /* Typecast allocated memory to a function pointer */
    void (*func) () = mem;

    /* Run code */
    func(ctx);

    /* Free up allocated memory */
    munmap(mem, size);

    return 0;
}

/*
 * Function to be called from Python
 */
PyObject *
py_jit(PyObject * self, PyObject * args)
{
    unsigned char   *data;
    unsigned int     size;
    unsigned int     rv;
    PyObject        *dict_in;
    PyObject        *dict_out = PyDict_New();
    context_t        ctx;

    /* Check newly created dict is not null */
    if (dict_out == NULL)
        return Py_BuildValue("I{}", -1);

    /* Parse input arguments */
    PyArg_ParseTuple(args, "s#O!", &data, &size, &PyDict_Type, &dict_in);

    /* Load context from input dictionary */
    load_context_from_dict(dict_in, &ctx);

    /* Run input code */
    rv = run(data, size, &ctx);

    /* Save context to output dictionary */
    save_context_to_dict(dict_out, &ctx);

    /* Build return value and return */
    return Py_BuildValue("IO", rv, dict_out);
}

/*
 * Bind Python function names to our C functions
 */
static PyMethodDef pyasmjit_methods[] = {
    {"jit", py_jit, METH_VARARGS},
    {NULL, NULL}
};

/*
 * Python calls this to let us initialize our module
 */
void
initpyasmjit(void)
{
    (void) Py_InitModule("pyasmjit", pyasmjit_methods);
}
