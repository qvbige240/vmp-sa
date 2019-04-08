
#include "vmp_vector.h"

#define CVECTOR_MALLOC	malloc
#define CVECTOR_FREE	free
#define CVECTOR_MEMSET	memset
#define CVECTOR_MEMMOVE	memmove

/* tmVectorCreate -- create a generic vector */

int tmVectorCreate(VmpVector CVECTOR_FAR * vectorhandle, const int elementsize, const int capacity) 
{
    size_t cap = capacity;
    
    if ((vectorhandle==NULL)) { return CVECTOR_BAD_ARGUMENT; }
    
    *vectorhandle = (VmpVector)CVECTOR_MALLOC(sizeof(CVector));
    
    if ((*vectorhandle==NULL)) {
        return CVECTOR_MALLOC_FAILED;
    }
    
    (*vectorhandle)->size = 0;
    (*vectorhandle)->flags = 0;
    (*vectorhandle)->capacity = 0;
    (*vectorhandle)->elementsize = elementsize;
    if (!cap) { cap = 10; }
    (*vectorhandle)->array = (void CVECTOR_FAR *)CVECTOR_MALLOC(cap*elementsize);
    if ((*vectorhandle)->array) {
        (*vectorhandle)->capacity = cap;
        return 0;
    }
    CVECTOR_FREE(*vectorhandle);
    *vectorhandle = NULL;
    return CVECTOR_MALLOC_FAILED;
}


/*  tmVectorAddElement -- add an element to a generic vector 
 equivalent to vector::push_back  */


int tmVectorAddElement(const VmpVector vectorhandle, const void CVECTOR_FAR * element) {
    
    size_t newcap;
    
    int errorcode;
    
    if ((vectorhandle==NULL)) { return CVECTOR_BAD_ARGUMENT; }
    
    if ( (vectorhandle->flags&CVECTOR_FLAGS_NO_RESIZE) ) { return CVECTOR_NO_RESIZE; }
    
    if (vectorhandle->size >= vectorhandle->capacity) {
        
        newcap = vectorhandle->capacity*2;
        
        if (newcap < 1) { newcap = 1; }
        
        errorcode = tmVectorSetCapacity (vectorhandle, newcap);
        
        if (errorcode != 0) {
            
            newcap = vectorhandle->capacity+(size_t)(vectorhandle->capacity>>2);
            
            if (newcap < 1) { newcap = 1; }
            
            errorcode = tmVectorSetCapacity (vectorhandle, newcap);
            
            if (errorcode != 0) { return errorcode; }
        }
        
    }
    
    CVECTOR_MEMMOVE(((char CVECTOR_FAR *)(vectorhandle->array))+vectorhandle->size*vectorhandle->elementsize,
            (const char CVECTOR_FAR *)element, vectorhandle->elementsize);
    vectorhandle->size ++;
    return 0;
    
}

/* tmVectorGetElement -- get a copy of an element from a generic vector */

int tmVectorGetElement(const VmpVector vectorhandle, void CVECTOR_FAR * element, const int index) {
    
    if ((vectorhandle==NULL)) { return CVECTOR_BAD_ARGUMENT; }
    
    if (index >= 0 && index < vectorhandle->size) {
        
        CVECTOR_MEMMOVE((char *)element,((char *)(vectorhandle->array))+index*vectorhandle->elementsize,
                vectorhandle->elementsize);
        
        return 0;
        
    } else {
        
        return CVECTOR_NOT_FOUND;
    }
    
}


/* tmVectorGetElementptr -- get a pointer to an element from a generic vector */

int tmVectorGetElementptr(const VmpVector vectorhandle, void CVECTOR_FAR ** elementptr, const int index) {
    
    if ((vectorhandle==NULL)) { return CVECTOR_BAD_ARGUMENT; }
    
    if (index >= 0 && index < vectorhandle->size) {
        
        *elementptr = (void CVECTOR_FAR*)(((char *)(vectorhandle->array))+index*vectorhandle->elementsize);
        
        vectorhandle->flags |= CVECTOR_FLAGS_NO_RELOCATION;
        
        return 0;
        
    } else {
        
        return CVECTOR_NOT_FOUND;
    }
    
}


/* tmVectorSetElement -- set a copy of an element into a generic vector */

int tmVectorSetElement(const VmpVector vectorhandle, const void CVECTOR_FAR * element, const int index) {
    
    int newcap;
    
    int errorcode;
    
    if ((vectorhandle==NULL) ) { return CVECTOR_BAD_ARGUMENT; }
    
    if (index >= vectorhandle->capacity) {
        
        newcap = index+vectorhandle->capacity+1;
        
        errorcode = tmVectorSetCapacity(vectorhandle, newcap);
        
        if (errorcode != 0 ) {
            
            newcap = index*1.2;
            if (newcap < index+128) { newcap = index+128; }
            
            errorcode = tmVectorSetCapacity(vectorhandle, newcap);
            
            if (errorcode != 0) { return errorcode; }
            
        }
    }
    
    
    if (index >= 0 && index < vectorhandle->capacity) {
        
        CVECTOR_MEMMOVE(((char *)(vectorhandle->array))+index*vectorhandle->elementsize,(char *)element,
                vectorhandle->elementsize);
        
        if (index >= vectorhandle->size) { vectorhandle->size = index+1; }
        return 0;
        
    } else {
        
        return CVECTOR_NOT_FOUND;
    }
    
}

/* tmVectorRemoveElement -- remove an element from a generic vector */

/* keeps elements 0 -- index-1, discards element index
 moves elements index+1 through vectorhandle->size-1
 into element index through vectorhandle->size-2
 
 i.e. moves characters (index+1)*(vectorhandle->elementsize)
 through (vectorhandle->size)*(vectorhandle->elementsize)-1
 to index*(vectorhandle->elementsize)
 */

int tmVectorRemoveElement(const VmpVector vectorhandle, const int index) {
    
    if ((vectorhandle==NULL)) { return CVECTOR_BAD_ARGUMENT; }
    
    if ((vectorhandle->flags&CVECTOR_FLAGS_NO_RELOCATION)) { return CVECTOR_NO_RELOCATION; }
    
    if (index >= vectorhandle->size || index < 0 ) { return CVECTOR_NOT_FOUND; }
    
    if (index == vectorhandle->size-1) {
        vectorhandle->size--;
        return 0;
    }
    
    CVECTOR_MEMMOVE((char *)vectorhandle->array+index*(vectorhandle->elementsize),
            (char *)vectorhandle->array+(index+1)*(vectorhandle->elementsize),
            (vectorhandle->size-1-index)*(vectorhandle->elementsize));
    
    vectorhandle->size--;
    return 0;
}

/* tmVectorClear -- clear a generic vector */

int tmVectorClear(const VmpVector vectorhandle) {
    
    if ((vectorhandle==NULL)) { return CVECTOR_BAD_ARGUMENT; }
    
    if (vectorhandle->flags & CVECTOR_FLAGS_NO_RESIZE) { return CVECTOR_NO_RESIZE; }
    
    vectorhandle->size = 0;
    
    return 0;
    
}

/* tmVectorFree -- remove a generic vector */

int tmVectorFree(VmpVector CVECTOR_FAR * vectorhandle) {
    
    if ((vectorhandle==NULL)) { return CVECTOR_BAD_ARGUMENT; }
    
    if (*vectorhandle) {
        
        if ((*vectorhandle)->flags & CVECTOR_FLAGS_NO_RESIZE) { return CVECTOR_NO_RESIZE; }
        
        if ((*vectorhandle)->flags & CVECTOR_FLAGS_NO_RELOCATION) { return CVECTOR_NO_RELOCATION; }
        
        if ((*vectorhandle)->array) {
            
            CVECTOR_FREE((*vectorhandle)->array);
            
        }
        
        CVECTOR_FREE(*vectorhandle);
        
    }
    
    *vectorhandle = 0;
    
    return 0;
    
}

/* tmVectorGetCapacity - function to return the CVector capacity */

int tmVectorGetCapacity(const VmpVector vectorhandle, int CVECTOR_FAR * capacity) {
    
    if ((vectorhandle==NULL)||!(capacity)) { return CVECTOR_BAD_ARGUMENT; }
    
    *capacity = vectorhandle->capacity;
    
    return 0;
}

/* tmVectorGetSize - function to return the VmpVector size */

int tmVectorGetSize(const VmpVector vectorhandle, int CVECTOR_FAR * size) {
    
    if ((vectorhandle==NULL)||!(size)) { return CVECTOR_BAD_ARGUMENT; }
    
    *size = vectorhandle->size;
    
    return 0;
}

/* tmVectorGetFlags - function to return the VmpVector flags */

int tmVectorGetFlags(const VmpVector vectorhandle, unsigned int CVECTOR_FAR * flags) {
    
    if ((vectorhandle==NULL)||!(flags)) { return CVECTOR_BAD_ARGUMENT; }
    
    *flags = vectorhandle->flags;
    
    return 0;
}

/* tmVectorSetCapacity - function to set the VmpVector capacity */

int tmVectorSetCapacity(const VmpVector vectorhandle, const int capacity) {
    
    void CVECTOR_FAR * temparray;
    
    if ((vectorhandle==NULL) || capacity < vectorhandle->size) { return CVECTOR_BAD_ARGUMENT; }
    
    if (capacity == vectorhandle->capacity) { return 0; }
    
    if (vectorhandle->flags&CVECTOR_FLAGS_NO_RELOCATION) { return CVECTOR_NO_RELOCATION; }
    
    if (capacity) {
        
        temparray = CVECTOR_MALLOC(capacity*vectorhandle->elementsize);
        if (!temparray)   { return CVECTOR_MALLOC_FAILED; }
        
        if (vectorhandle->size) {   
          CVECTOR_MEMMOVE((char *)temparray, (char *)vectorhandle->array, vectorhandle->size*vectorhandle->elementsize); 
        }
        CVECTOR_FREE(vectorhandle->array);
        
    } else {
        temparray = NULL;
    }
    
    vectorhandle->array = temparray;
    vectorhandle->capacity = capacity;
    
    return 0;
}

/* tmVectorSetSize - function to set the VmpVector size */

int tmVectorSetSize(const VmpVector vectorhandle, const int size) {
    
    int errorcode;
    
    if ((vectorhandle==NULL) ) { return CVECTOR_BAD_ARGUMENT; }
    
    if (size == vectorhandle->size) { return 0; }
    
    if ((vectorhandle->flags & CVECTOR_FLAGS_NO_RESIZE)) { return CVECTOR_NO_RESIZE; }
    
    if ( size > vectorhandle->capacity ) {
        
        if ((vectorhandle->flags & CVECTOR_FLAGS_NO_RELOCATION) ) { return CVECTOR_NO_RELOCATION; }
        
        errorcode = tmVectorSetCapacity(vectorhandle,size);
        
        if (errorcode != 0) { return errorcode; }
        
    }
    
    if ( size <= vectorhandle->capacity ) {
        
        if (size > vectorhandle->size) {
            
            CVECTOR_MEMSET(((char *)vectorhandle->array)+(vectorhandle->size)*(vectorhandle->elementsize),
                   0, (vectorhandle->size-size)*(vectorhandle->elementsize));
        }
        
        vectorhandle->size = size;
        
    }
    
    
    return 0;
}


/* tmVectorSetFags - function to set the VmpVector flags */

int tmVectorSetFlags(const VmpVector vectorhandle, const unsigned int flags) {
    
    if ((vectorhandle==NULL) ) { return CVECTOR_BAD_ARGUMENT; }
    
    vectorhandle->flags = flags;
    
    return 0;
    
}
