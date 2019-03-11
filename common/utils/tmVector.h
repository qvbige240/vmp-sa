/*********************************************************************************
  *Copyright(C),2016-2017,timanetworks.com
  *FileName:  tmVector.h
  *Author:  wison.wei
  *Description:  c vector
**********************************************************************************/

#ifndef CVECTOR_H_INCLUDED
#define CVECTOR_H_INCLUDED

#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* CVector flags */
#define CVECTOR_FLAGS_NO_RELOCATION		(1)
#define CVECTOR_FLAGS_NO_RESIZE			(2)


/* CVector error return values */
#define CVECTOR_MALLOC_FAILED			(-1)
#define CVECTOR_BAD_ARGUMENT			(-2)
#define CVECTOR_NOT_FOUND				(-4)
#define CVECTOR_NO_RELOCATION			(-8)
#define CVECTOR_NO_RESIZE				(-16)


#define CVECTOR_FAR
#define TM_VECTOR_API

typedef struct {
	int size;				/* size of the vector      */
	int capacity;			/* capacity of the vector  */
	int elementsize;		/* size of an element      */
	void CVECTOR_FAR * array;	/* the array of elements   */
	unsigned int flags;				/* flags                   */
} CVector;

typedef CVector CVECTOR_FAR * tmVector;

/**
 * tmVectorAddElement -- add an element to a tmVector
*/
TM_VECTOR_API int tmVectorAddElement(const tmVector vectorhandle, const void CVECTOR_FAR * element);

/**
 * tmVectorCapacity -- macro to return the tmVector capacity
*/
#define tmVectorCapacity(vectorhandle)  (vectorhandle)->capacity

/**
 * tmVectorClear -- clear a generic vector
*/
TM_VECTOR_API int tmVectorClear(const tmVector vectorhandle);

/**
 * tmVectorCreate -- create a tmVector
*/
TM_VECTOR_API int tmVectorCreate(tmVector CVECTOR_FAR * vectorhandle, const int elementsize, const int capacity);

/**
 * tmVectorElementAt -- return the element at the given index as a void pointer without checking and without protection against relocation
*/
#define tmVectorElementAt(vectorhandle,index) ((void CVECTOR_FAR *)(((tmChar *)((vectorhandle)->array))+(index)*(vectorhandle)->elementsize))

/**
 * tmVectorFree -- remove a tmVector
*/
TM_VECTOR_API int tmVectorFree(tmVector CVECTOR_FAR * vectorhandle);

/**
 * tmVectorGetCapacity - function to return the tmVector capacity
*/
TM_VECTOR_API int tmVectorGetCapacity(const tmVector vectorhandle, int CVECTOR_FAR * capacity);

/**
 * tmVectorGetElement -- get a copy of an element from a tmVector
*/
TM_VECTOR_API int tmVectorGetElement(const tmVector vectorhandle, void CVECTOR_FAR * element, const int index);

/**
 * tmVectorGetElementptr -- get a pointer to an element from a tmVector
*/
TM_VECTOR_API int tmVectorGetElementptr(const tmVector vectorhandle, void CVECTOR_FAR ** elementptr, const int index);

/**
 * tmVectorGetFlags - function to return the tmVector flags
*/
TM_VECTOR_API int tmVectorGetFlags(const tmVector vectorhandle, unsigned int CVECTOR_FAR * flags);

/**
 * tmVectorGetSize - function to return the tmVector size
*/
TM_VECTOR_API int tmVectorGetSize(const tmVector vectorhandle, int CVECTOR_FAR * size);

/**
 * tmVectorRemoveElement -- remove an element from a generic vector
*/
TM_VECTOR_API int tmVectorRemoveElement(const tmVector vectorhandle, const int index);

/**
 * tmVectorSetCapacity - function to set the tmVector capacity
*/
TM_VECTOR_API int tmVectorSetCapacity(const tmVector vectorhandle, const int capacity);   

/**
 * tmVectorSetElement -- set a copy of an element into a tmVector
*/
TM_VECTOR_API int tmVectorSetElement(const tmVector vectorhandle, const void CVECTOR_FAR * element, const int index);

/**
 * tmVectorSetFags - function to set the tmVector flags
*/
TM_VECTOR_API int tmVectorSetFlags(const tmVector vectorhandle, const unsigned int flags);

/**
 * tmVectorSetSize - function to set the tmVector size
*/
TM_VECTOR_API int tmVectorSetSize(const tmVector vectorhandle, const int size);

/**
 * tmVectorSize -- macro to return the tmVector size
*/
#define tmVectorSize(vectorhandle)  (vectorhandle)->size

#ifdef __cplusplus
}
#endif

#endif
