//
// Created by MrKahoobadoo on 9/24/26.
//

#ifndef ELECTORSYNTH_RANDANDHOLDMODULE_H
#define ELECTORSYNTH_RANDANDHOLDMODULE_H

#include "leaf.h"
#include "defs.h"
#include "leaf-mempool.h"
#include "leaf-oscillators.h"
#include "leaf-envelopes.h"

typedef enum
{
    RandHoldEventWatchFlag,
    RandHoldThreshold,
    RandHoldFrequency,
    RandHoldDurRand,
    RandHoldGain,
    RandHoldMix,

} RandHoldParams;

typedef struct _tRandAndHoldModule
{
    ModuleHeader header;

    float currRand;

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

    float RandRate;
    float invRandRate;

    tLookupTable* skewFreqTable;
    tLookupTable* gainAmpTable;
    tLookupTable* mtofTable;

    tMempool* mempool;

} _tRandAndHoldModule;

typedef _tRandAndHoldModule* tRandAndHoldModule;

//init module
void tRandAndHoldModule_init(void** const randHold, float* const params, float id, LEAF* const leaf);
void tRandAndHoldModule_initToPool(void** const randHold, float* const params, float id, tMempool** const mempool);
void tRandAndHoldModule_free(void** const randHold);
void tRandAndHoldModule_setParameter(tRandAndHoldModule const randHold, RandHoldParams param_type, float input);

// Modulatable setters
void tRandAndHoldModule_setBinLength(tRandAndHoldModule const randHold);

void tRandAndHoldModule_onNoteOn(tRandAndHoldModule const randHold, float velocity);

void tRandAndHoldModule_tick (tRandAndHoldModule const randHold, float*);

#endif // ELECTORSYNTH_RANDANDHOLDMODULE_H
