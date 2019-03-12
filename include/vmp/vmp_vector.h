
#ifndef VMP_VECTOR_H
#define VMP_VECTOR_H

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

typedef CVector CVECTOR_FAR * VmpVector;

/**
 * tmVectorAddElement -- add an element to a VmpVector
*/
TM_VECTOR_API int tmVectorAddElement(const VmpVector vectorhandle, const void CVECTOR_FAR * element);

/**
 * tmVectorCapacity -- macro to return the VmpVector capacity
*/
#define tmVectorCapacity(vectorhandle)  (vectorhandle)->capacity

/**
 * tmVectorClear -- clear a generic vector
*/
TM_VECTOR_API int tmVectorClear(const VmpVector vectorhandle);

/**
 * tmVectorCreate -- create a VmpVector
*/
TM_VECTOR_API int tmVectorCreate(VmpVector CVECTOR_FAR * vectorhandle, const int elementsize, const int capacity);

/**
 * tmVectorElementAt -- return the element at the given index as a void pointer without checking and without protection against relocation
*/
#define tmVectorElementAt(vectorhandle,index) ((void CVECTOR_FAR *)(((tmChar *)((vectorhandle)->array))+(index)*(vectorhandle)->elementsize))

/**
 * tmVectorFree -- remove a VmpVector
*/
TM_VECTOR_API int tmVectorFree(VmpVector CVECTOR_FAR * vectorhandle);

/**
 * tmVectorGetCapacity - function to return the VmpVector capacity
*/
TM_VECTOR_API int tmVectorGetCapacity(const VmpVector vectorhandle, int CVECTOR_FAR * capacity);

/**
 * tmVectorGetElement -- get a copy of an element from a VmpVector
*/
TM_VECTOR_API int tmVectorGetElement(const VmpVector vectorhandle, void CVECTOR_FAR * element, const int index);

/**
 * tmVectorGetElementptr -- get a pointer to an element from a VmpVector
*/
TM_VECTOR_API int tmVectorGetElementptr(const VmpVector vectorhandle, void CVECTOR_FAR ** elementptr, const int index);

/**
 * tmVectorGetFlags - function to return the VmpVector flags
*/
TM_VECTOR_API int tmVectorGetFlags(const VmpVector vectorhandle, unsigned int CVECTOR_FAR * flags);

/**
 * tmVectorGetSize - function to return the VmpVector size
*/
TM_VECTOR_API int tmVectorGetSize(const VmpVector vectorhandle, int CVECTOR_FAR * size);

/**
 * tmVectorRemoveElement -- remove an element from a generic vector
*/
TM_VECTOR_API int tmVectorRemoveElement(const VmpVector vectorhandle, const int index);

/**
 * tmVectorSetCapacity - function to set the VmpVector capacity
*/
TM_VECTOR_API int tmVectorSetCapacity(const VmpVector vectorhandle, const int capacity);   

/**
 * tmVectorSetElement -- set a copy of an element into a VmpVector
*/
TM_VECTOR_API int tmVectorSetElement(const VmpVector vectorhandle, const void CVECTOR_FAR * element, const int index);

/**
 * tmVectorSetFags - function to set the VmpVector flags
*/
TM_VECTOR_API int tmVectorSetFlags(const VmpVector vectorhandle, const unsigned int flags);

/**
 * tmVectorSetSize - function to set the VmpVector size
*/
TM_VECTOR_API int tmVectorSetSize(const VmpVector vectorhandle, const int size);

/**
 * tmVectorSize -- macro to return the VmpVector size
*/
#define tmVectorSize(vectorhandle)  (vectorhandle)->size

#ifdef __cplusplus
}
#endif

#endif //VMP_VECTOR_H
