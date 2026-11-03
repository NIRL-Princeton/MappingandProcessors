//
// Created by Myra Norton on 8/13/26.
//

#ifndef ELECTORSYNTH_TEMPLATEMODULE_H
#define ELECTORSYNTH_TEMPLATEMODULE_H

// TODO: Change every instance of "Template" in this file to the capitalized name of your module
// TODO: Change every instance of "template" and "template_" in this file to the name of your module (not capitalized)

#include "leaf.h"
#include "defs.h"
#include "leaf-mempool.h"

//TODO: include the relevant LEAF header file
//#include "leaf-template.h"

/**********************************************************************************************************************/

/* These are the parameters for your module. It must start with the EventWatchFlag and end with the NumParams.
 * Include type if there are different implementations within your module. Make sure that each parameter starts with
 * the name of your module (e.g. DelayFeedback) */
typedef enum {
    TemplateEventWatchFlag,
    // TODO: Add module-specific parameters here
    // TemplateType,
    TemplateNumParams
} TemplateParams;

/**********************************************************************************************************************/

/* These are the different implementations/types of your module. If your module doesn't have different types, you can
 * delete this section. Otherwise, change the numbers below with the names of your module types. Make sure the enum
 * ends with the NumParams. */
typedef enum {
    //TODO: Rename Template Types or delete this section
    TemplateType1,
    TemplateType2,
    TemplateNumTypes
} TemplateTypes;

/**********************************************************************************************************************/

/* This is the struct associated with your module. Add any variables/pointers that might need to be accessed throughout
 * the codebase. */
typedef struct _tTemplateModule {
    ModuleHeader header;
    void* theTemplate;
    uint32_t templateType; //TODO: delete this if your module does NOT have different types

    // most modules will need amplitude, sample rate, and inverse sample rate
    float amp;
    float sr;
    float invSr;

    // TODO: add any other variables needed here

    // most modules will need tables for the decibels
    float* dbTableAddress;
    uint32_t dbTableScalar;
    float dbTableOffset;
    float dbTableSizeMinusOne;

    // all modules need a pointer to a lookup table for mapping parameter values and a pointer to the memory pool
    tLookupTable* table;
    tMempool* mempool;
} _tTemplateModule;

/**********************************************************************************************************************/

// This is simply for convenience - it allows us to abstract away the fact that we’re dealing with a pointer.
typedef _tTemplateModule* tTemplateModule;

/**********************************************************************************************************************/

/* These are the functions every module needs. Feel free to add any necessary helper functions. */

/* init() and initToPool() are the functions that initialize your module. init() will call initToPool() with the default
 * values, but a user can call initToPool() directly to specify the mempool and resTable. */
void tTemplateModule_init(void** const template_, float* const params, float id, LEAF* const leaf);
void tTemplateModule_initToPool(void** const template_, float* const params, float id, tMempool** const mempool, tLookupTable* resTable);

/* free() removes the module from the memory pool when it is deleted */
void tTemplateModule_free(void** const template_);

/* tick() advances to the next audio sample. This is the function that gets called in the processBlock() function */
void tTemplateModule_tick (tTemplateModule const template_, float*);

/* setParameter() changes the modules internal parameters when a user changes it */
void tTemplateModule_setParameter(tTemplateModule const template_, TemplateParams param_type,float input);

/* These are setter functions for non-modulatable variables */
void tTemplateModule_setDBtoATableLocation (tTemplateModule const template_, float* tableAddress, uint32_t tableSize);
void tTemplateModule_setSampleRate (tTemplateModule const template_, float sr);

//TODO: Add more relevant functions

#endif //ELECTORSYNTH_TEMPLATEMODULE_H
