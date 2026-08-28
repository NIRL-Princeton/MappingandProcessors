//
// Created by Matthew McWeeney on 8/28/26.
//

#ifndef ELECTORSYNTH_SAMPLEANDHOLDMODULE_H
#define ELECTORSYNTH_SAMPLEANDHOLDMODULE_H

#include "leaf.h"
#include "defs.h"
#include "leaf-mempool.h"
#include "leaf-oscillators.h"

typedef enum
{
    SampHoldEventWatchFlag,
    SampHoldThreshold,
    SampHoldFrequency,

} SampHoldParams;

typedef struct _tSampleAndHoldModule
{
    ModuleHeader header;

    float inputSample;
    float threshold;
    float frequency;

    tMempool* mempool;

} _tSampleAndHoldModule;

typedef _tSampleAndHoldModule* tSampleAndHoldModule;

//init module
void tSampleAndHoldModule_init(void** const sampHold, float* const params, float id, LEAF* const leaf);
void tSampleAndHoldModule_initToPool(void** const sampHold, float* const params, float id, tMempool** const mempool);
void tSampleAndHoldModule_free(void** const sampHold);
void tSampleAndHoldModule_setParameter(tSampleAndHoldModule const sampHold, SampHoldParams param_type, float input);
// Modulatable setters
void tSampleAndHoldModule_tick (tSampleAndHoldModule const sampHold);

#endif // ELECTORSYNTH_SAMPLEANDHOLDMODULE_H
