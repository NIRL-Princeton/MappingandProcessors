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
    //RandHoldTriggerToggle,
    //RandHoldThreshold,
    RandHoldAmp

} RandHoldParams;

typedef struct _tRandAndHoldModule
{
    ModuleHeader header;

    float currRand;
    //float threshold;

    //uint8_t triggerToggle;
    uint8_t noteOn;

    float amp;
    tSlopeRamp ampSmoother;

    tMempool* mempool;

} _tRandAndHoldModule;

typedef _tRandAndHoldModule* tRandAndHoldModule;

//init module
void tRandAndHoldModule_init(void** const randHold, float* const params, float id, LEAF* const leaf);
void tRandAndHoldModule_initToPool(void** const randHold, float* const params, float id, tMempool** const mempool);
void tRandAndHoldModule_free(void** const randHold);
void tRandAndHoldModule_setParameter(tRandAndHoldModule const randHold, RandHoldParams param_type, float input);

// Modulatable setters
void tRandAndHoldModule_onNoteOn(tRandAndHoldModule const randHold, float velocity);

void tRandAndHoldModule_tick (tRandAndHoldModule const randHold);

#endif // ELECTORSYNTH_RANDANDHOLDMODULE_H
