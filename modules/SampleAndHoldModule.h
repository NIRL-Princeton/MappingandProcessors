//
// Created by Matthew McWeeney on 8/28/26.
//

#ifndef ELECTORSYNTH_SAMPLEANDHOLDMODULE_H
#define ELECTORSYNTH_SAMPLEANDHOLDMODULE_H

#include "leaf.h"
#include "defs.h"
#include "leaf-mempool.h"
#include "leaf-oscillators.h"
#include "leaf-envelopes.h"

typedef enum
{
    SampHoldEventWatchFlag,
    SampHoldThreshold,
    SampHoldFrequency,
    SampHoldDurRand,
    SampHoldGain,
    SampHoldMix,

} SampHoldParams;

typedef struct _tSampleAndHoldModule
{
    ModuleHeader header;

    float currSample;
    tRamp sampleSmoother;

    float threshold;
    float frequency;
    float durRand;
    float counter;

    float binLength;

    float gain;
    tSlopeRamp gainSmoother;
    float mix;
    tSlopeRamp mixSmoother;

    float sampleRate;
    float invSampleRate;

    tMempool* mempool;

} _tSampleAndHoldModule;

typedef _tSampleAndHoldModule* tSampleAndHoldModule;

//init module
void tSampleAndHoldModule_init(void** const sampHold, float* const params, float id, LEAF* const leaf);
void tSampleAndHoldModule_initToPool(void** const sampHold, float* const params, float id, tMempool** const mempool);
void tSampleAndHoldModule_free(void** const sampHold);
void tSampleAndHoldModule_setParameter(tSampleAndHoldModule const sampHold, SampHoldParams param_type, float input);

// Modulatable setters
void tSampleAndHoldModule_setBinLength(tSampleAndHoldModule const sampHold);

void tSampleAndHoldModule_onNoteOn(tSampleAndHoldModule const sampHold, float note, float velocity);

void tSampleAndHoldModule_tick (tSampleAndHoldModule const sampHold, float*);

#endif // ELECTORSYNTH_SAMPLEANDHOLDMODULE_H
