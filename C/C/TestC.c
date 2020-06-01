//
//  TestC.c
//  C
//
//  Created by Robert Keeling on 14/11/2019.
//  Copyright © 2019 lisadmin. All rights reserved.
//

#include "TestC.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "/Library/Frameworks/Python.framework/Versions/3.7/include/python3.7m/Python.h"
#include "/Library/Frameworks/Python.framework/Versions/3.7/include/python3.7m/structmember.h"

static PyTypeObject MatrixType;

typedef struct {
    PyObject_HEAD
    PyObject *array;
    Matrix cMatrix;
} PyMatrixObject;

static void Matrix_dealloc(PyMatrixObject *self){
    Py_XDECREF(self->array);
    free(self->cMatrix.matrix);
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static Matrix buildCMatrixFromPyMatrix(PyObject * in){
    Matrix toReturn;
    if(PyTuple_Check(in)){
        in = PyTuple_GetItem(in, 0);
    }
    if(PyObject_IsInstance(in, (PyObject *)&MatrixType)){
        PyMatrixObject * qw = (PyMatrixObject *)in;
        in = qw->array;
    }
    if(PyList_Check(in)){
        long rows = PyList_Size(in);
        long columns;
        PyObject * firstItem = PyList_GetItem(in, 0);
        if(PyList_Check(firstItem)){
            columns = PyList_Size(firstItem);
        }else{
            columns = rows;
            rows = 1;
        }
        Matrix t = {(int)rows, (int)columns, (double*)calloc((int)(rows*columns), sizeof(double))};
        for(int i=0; i<rows; i++){
            PyObject * row = PyList_GetItem(in, i);
            for(int j=0; j<columns; j++){
                double tmp;
                PyObject * col = PyList_GetItem(row, j);
                PyArg_Parse(col, "d", &tmp);
                *(t.matrix+i*columns+j) = tmp;
            }
        }
        toReturn = t;
    }else{
        Matrix t = {0, 0, NULL};
        toReturn = t;
    }
    return toReturn;
}

static PyObject * copyPyMatrixFromCMatrix(PyMatrixObject *pyMatrix, Matrix * cMatrix){
    int rows = cMatrix->rowSize; int cols = cMatrix->columnSize;
    for(int i=0; i<rows; i++){
        PyObject * row = PyList_GetItem(pyMatrix->array, i);
        for(int j=0; j<cols; j++){
            PyList_SetItem(row, j, PyFloat_FromDouble(*(cMatrix->matrix+i*cols+j)));
        }
    }
    Py_RETURN_NONE;
}

static PyObject * buildPyMatrixFromCMatrix(Matrix * cMatrix){
    printf("\nBUILDING\n");
    int rows = cMatrix->rowSize; int cols = cMatrix->columnSize;
    PyObject *nargs = PyTuple_New(1);
    PyObject * array = PyList_New(rows);
    for(int i=0; i<rows; i++){
        PyObject * row = PyList_New(cols);
        for(int j=0; j<cols; j++){
            printf("%i: %f ",i*cols+j, *(cMatrix->matrix+i*cols+j));
            PyList_SetItem(row, j, PyFloat_FromDouble(*(cMatrix->matrix+i*cols+j)));
        }
        PyList_SetItem(array, i, row);
    }
    PyTuple_SetItem(nargs, 0, array);
    PyObject * toReturn = PyObject_CallObject((PyObject *) &MatrixType, nargs);
    Py_DecRef(nargs);
    printf("\nEND BUILDING\n");
    return toReturn;
}

static PyObject * rebuildPyMatrixFromCMatrix(PyMatrixObject *pyMatrix, Matrix *cMatrix){
    int rows = cMatrix->rowSize; int cols = cMatrix->columnSize;
    PySequence_DelSlice(pyMatrix->array, 0, PySequence_Length(pyMatrix->array));
    for(int i=0; i<rows; i++){
        PyObject * row = PyList_New(cols);
        for(int j=0; j<cols; j++){
            PyList_SetItem(row, j, PyFloat_FromDouble(*(cMatrix->matrix+i*cols+j)));
        }
        PyList_Append(pyMatrix->array, row);
    }
    Py_RETURN_NONE;
}


static PyObject * Matrix_long_math(PyMatrixObject *self, long v, Matrix (*fun)(Matrix *, long)){
    Matrix resultcMatrix = fun(&self->cMatrix, v);
    return buildPyMatrixFromCMatrix(&resultcMatrix);
}

static PyObject * Matrix_math(PyMatrixObject *self, PyMatrixObject *other, Matrix (*fun)(Matrix *, Matrix *)){
    Matrix resultcMatrix = fun(&self->cMatrix, &(other->cMatrix));
    return buildPyMatrixFromCMatrix(&resultcMatrix);
}

static PyObject * Matrix_math_decide(PyObject *a, PyObject *b, Matrix(*f1)(Matrix *, Matrix *), Matrix(*f2)(Matrix *, long)){
    if(PyObject_IsInstance(a, (PyObject *) &MatrixType)){
        if(PyNumber_Check(b)){
            long l;
            if(!PyArg_Parse(b, "l", &l)){
                return NULL;
            }
            return Matrix_long_math((PyMatrixObject *)a, l, f2);
        }
        return Matrix_math((PyMatrixObject *)a, (PyMatrixObject *)b, f1);
    }else{
        if(PyNumber_Check(a)){
            long l;
            if(!PyArg_Parse(a, "l", &l)){
                return NULL;
            }
            return Matrix_long_math((PyMatrixObject *)b, l, f2);
        }
        return Matrix_math((PyMatrixObject *)b, (PyMatrixObject *)a, f1);
    }
}

static PyObject * Matrix_subtract(PyObject *self, PyObject *args){
    return Matrix_math_decide(self, args, subtractMatrixCopy, subtractMatrixLongCopy);
}

static PyObject * Matrix_add(PyObject *self, PyObject *args){
    return Matrix_math_decide(self, args, addMatrixCopy, addMatrixLongCopy);
}

static PyObject * Matrix_cross(PyObject *self, PyObject *args){
    return Matrix_math_decide(self, args, crossProductMatrixCopy, crossProductMatrixLongCopy);
}

static PyObject * Matrix_divide(PyObject *self, PyObject *args){
    return Matrix_math_decide(self, args, crossDivdeMatrixCopy, crossDivideMatrixLongCopy);
}

static PyObject * Matrix_dot(PyMatrixObject *self, PyObject *args){
    PyMatrixObject * other = (PyMatrixObject *)args;
//    long i1 = PyList_Size(self->array);
//    long i2 = PyList_Size(PyList_GetItem(self->array, 0));
//    long i3 = PyList_Size(other->array);
//    long i4 = PyList_Size(PyList_GetItem(other->array, 0));
//    long i5 = self->cMatrix.rowSize;
//    long i6 = self->cMatrix.columnSize;
//    long i7 = other->cMatrix.rowSize;
//    long i8 = other->cMatrix.columnSize;
//    printf("FROM C CODE Start\n");
//    printf("%lix%li=%lix%li        %lix%li=%lix%li\n", i1,i2,i5,i6,i3,i4,i7,i8);
//    printf("FROM C CODE END\n");
    PyObject * t = Matrix_math(self, other, dotProductMatrixCopy);
    PyMatrixObject * v = (PyMatrixObject *)t;
    printf("\nDOT\n");
    printMatrix(&v->cMatrix, 0);
    printf("\nresultcMAtrixAbove\nresult PyMAtrix Below\n");
    PyObject * p = PyList_GetItem(v->array, 0);
    for(int i=0;i<PyList_Size(p);i++){
        double y = PyFloat_AsDouble(PyList_GetItem(p, 0));
        printf("%i: %f ", i, y);
    }
    
    printf("\nResult PyMAtrix ABOVE\nEND DOT\n");
    return t;
}

static PyObject * Matrix_sigmoid_in_place(PyMatrixObject *self, PyObject *arg){
    if(!PyNumber_Check(arg)){
        return NULL;
    }
    int deriv = (int)PyLong_AsLong(arg);
    if(deriv!=0&&deriv!=1){
        return NULL;
    }
    Matrix cMatrix = sigmoidMatrixCopy(&self->cMatrix, deriv);
    copyPyMatrixFromCMatrix(self, &cMatrix);
    Py_RETURN_NONE;
}

static PyObject * Matrix_sigmoid(PyMatrixObject *self, PyObject *arg){
    if(!PyNumber_Check(arg)){
        return NULL;
    }
    int deriv = (int)PyLong_AsLong(arg);
    if(deriv!=0&&deriv!=1){
        return NULL;
    }
    Matrix cMatrix = sigmoidMatrixCopy(&self->cMatrix, deriv);
    printf("\nSIGMOID START\n");
    for(int i=0; i<cMatrix.columnSize*cMatrix.rowSize; i++){
        printf("%i: %f",i, *(cMatrix.matrix+i));
    }
    printf("\nSIGGY\n");
    PyObject * toReturn = buildPyMatrixFromCMatrix(&cMatrix);
    printf("\nSIGMOID END\n");
    return toReturn;
}

static PyObject * Matrix_transpose(PyMatrixObject *self){
    Matrix cMatrix = transposeCopy(&self->cMatrix);
    PyObject * toReturn = buildPyMatrixFromCMatrix(&cMatrix);
    return toReturn;
}

static PyObject * Matrix_softmax(PyMatrixObject *self){
    Matrix cMatrix = softmaxCopy(&self->cMatrix);
    PyObject * toReturn = buildPyMatrixFromCMatrix(&cMatrix);
    return toReturn;
}

static PyObject * Matrix_one_hot(PyMatrixObject *self){
    Matrix cMatrix = oneHotCopy(&self->cMatrix);
    PyObject * toReturn = buildPyMatrixFromCMatrix(&cMatrix);
    return toReturn;
}

static PyObject * Matrix_add_weights(PyMatrixObject *self){
    PyObject * a = self->array;
    for(int i=0; i<PyList_Size(a); i++){
        PyObject * b = PyList_GetItem(a, i);
        PyList_Append(b, Py_BuildValue("f",1.0));
    }
    Py_RETURN_NONE;
}

static PyObject * Matrix_remove_weights(PyMatrixObject *self){
    for(int i=0; i<PyList_Size(self->array); i++){
        PyObject * row = PyList_GetItem(self->array, i);
        PyObject * new_row = PyList_GetSlice(row, 0, PyList_Size(row)-1);
        PyList_SetItem(self->array, i, new_row);
    }
    Py_RETURN_NONE;
}

static PyObject * Matrix_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    PyMatrixObject *self;
    self = (PyMatrixObject *) type->tp_alloc(type, 0);
    if (self != NULL) {
        self->array = PyList_New(0);
        if (self->array == NULL) {
            Py_DECREF(self);
            return NULL;
        }
    }
    return (PyObject *) self;
}

static int Matrix_init(PyMatrixObject *self, PyObject *args){
    PyObject *array;
    PyObject * tmp;
    if(!PyArg_Parse(args, "O", &array)){
        return -1;
    }
    if (array) {
        if(PyTuple_Check(array)){
            if(PyTuple_Size(array)==2){
                PyObject * tmp1 = PyTuple_GetItem(array, 0);
                PyObject * tmp2 = PyTuple_GetItem(array, 1);
                if(PyNumber_Check(tmp1)&&PyNumber_Check(tmp2)){
                    int x1 = (int)PyLong_AS_LONG(tmp1);
                    int x2 = (int)PyLong_AS_LONG(tmp2);
                    Matrix cMatrix = createRandomMatrix(x1, x2);
                    printf("\nOn Create\n%ix%i\n", cMatrix.columnSize, cMatrix.rowSize);
                    rebuildPyMatrixFromCMatrix(self, &cMatrix);
                    self->cMatrix = cMatrix;
                    return 0;
                }else{
                    return 0;
                }
            }
            array = PyTuple_GetItem(array, 0);
        }
        tmp = self->array;
        Py_INCREF(array);
        self->array = array;
        self->cMatrix = buildCMatrixFromPyMatrix(array);
        Py_XDECREF(tmp);
    }
    return 0;
};

static PyMemberDef Matrix_members[] = {
    {"array", T_OBJECT_EX, offsetof(PyMatrixObject, array), 0, "array"},
    {NULL}
};

static PyMethodDef Matrix_methods[] = {
    {"dot", (PyCFunction) Matrix_dot, METH_O, "dot product of matrices"},
    {"sigmoid_in_place", (PyCFunction) Matrix_sigmoid_in_place, METH_O, "sigmoid function in place"},
    {"sigmoid", (PyCFunction) Matrix_sigmoid, METH_O, "sigmoid function copy"},
    {"transpose", (PyCFunction) Matrix_transpose, METH_NOARGS, "transpose function copy"},
    {"add_weights", (PyCFunction) Matrix_add_weights, METH_NOARGS, "add weights to matrix"},
    {"softmax", (PyCFunction) Matrix_softmax, METH_NOARGS, "softmax function copy"},
    {"one_hot", (PyCFunction) Matrix_one_hot, METH_NOARGS, "oen hot function copy"},
    {"remove_weights", (PyCFunction) Matrix_remove_weights, METH_NOARGS, "remove weights from matrix"},
    {NULL}
};

static PyNumberMethods Matrix_number_methods = {
    .nb_add = (PyCFunction) Matrix_add,
    .nb_subtract = (PyCFunction) Matrix_subtract,
    .nb_multiply = (PyCFunction) Matrix_cross,
    .nb_true_divide = (PyCFunction) Matrix_divide,
};

static PyTypeObject MatrixType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "NNAPI.Matrix",
    .tp_doc = "My C matrix class",
    .tp_basicsize = sizeof(PyMatrixObject),
    .tp_itemsize = 2*sizeof(double),
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = Matrix_new,
    .tp_init = (initproc) Matrix_init,
    .tp_dealloc = (destructor) Matrix_dealloc,
    .tp_members = Matrix_members,
    .tp_as_number = &Matrix_number_methods,
    .tp_methods = Matrix_methods,
};

static PyModuleDef matrixModule = {
    PyModuleDef_HEAD_INIT,
    .m_name = "NNAPI",
    .m_doc = "Python extention module for Matrix class",
    .m_size = -1
};


PyMODINIT_FUNC PyInit_NNAPI(void){
    PyObject *m;
    if (PyType_Ready(&MatrixType) < 0)
        return NULL;

    m = PyModule_Create(&matrixModule);
    if (m == NULL)
        return NULL;

    Py_INCREF(&MatrixType);
    if (PyModule_AddObject(m, "Matrix", (PyObject *) &MatrixType) < 0) {
        Py_DECREF(&MatrixType);
        Py_DECREF(m);
        return NULL;
    }
    return m;
}
