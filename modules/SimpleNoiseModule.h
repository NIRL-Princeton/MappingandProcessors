//
// Created by Matthew McWeeney on 7/14/26.
//

#ifndef ELECTORSYNTH_SIMPLENOISEMODULE_H
#define ELECTORSYNTH_SIMPLENOISEMODULE_H
#include "leaf.h"
#include "defs.h"
#include "leaf-mempool.h"
#include "leaf-oscillators.h"

typedef enum
{
    SimpNoiseEventWatchFlag,
    SimpNoiseGain,

} SimpNosParams;

typedef struct _tSimpNoiseModule
{
    ModuleHeader header;

    void* theSimpNoise;

    float gain;

    tMempool* mempool;

} _tSimpNoiseModule;

typedef _tSimpNoiseModule* tSimpNoiseModule;

//init module
void tSimpNoiseModule_init(void** const simpNoise, float* const params, float id, LEAF* const leaf);
void tSimpNoiseModule_initToPool(void** const simpNoise, float* const params, float id, tMempool** const mempool);
void tSimpNoiseModule_free(void** const simpNoise);
void tSimpNoiseModule_setParameter(tSimpNoiseModule const simpNoise, SimpNosParams param_type,float input);
// Modulatable setters
void tSimpNoiseModule_tick (tSimpNoiseModule const simpNoise);

#endif // ELECTORSYNTH_SIMPLENOISEMODULE_H
