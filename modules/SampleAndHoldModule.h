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
    SampHoldMidiPitch,
    SampHoldKeyFollow,
    SampHoldHarmonic,

} SampHoldParams;

typedef struct _tSampleAndHoldModule
{
    ModuleHeader header;

    float currSample;

    float threshold;
    float durRand;
    int counter;

    int binLength;
    uint8_t hold;

    float gain;
    tSlopeRamp gainSmoother;
    float mix;
    tSlopeRamp mixSmoother;

    uint8_t noteOn;

    float frequency;
    float pitch;
    float keyFollow;
    float harmonicMultiplier;
    float finalFreq;

    float sampleRate;
    float invSampleRate;

    tLookupTable* skewFreqTable;
    tLookupTable* gainAmpTable;
    tLookupTable* mtofTable;

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

void tSampleAndHoldModule_onNoteOn(tSampleAndHoldModule const sampHold, float velocity);

void tSampleAndHoldModule_tick (tSampleAndHoldModule const sampHold, float*);

#endif // ELECTORSYNTH_SAMPLEANDHOLDMODULE_H
