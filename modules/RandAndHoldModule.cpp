//
// Created by MrKahoobadoo on 9/24/26.
//

#include "RandAndHoldModule.h"
#include "leaf-envelopes.h"

#include <cstdio>

void tRandAndHoldModule_init(void** const randHold, float* params, float id, LEAF* const leaf)
{
    tRandAndHoldModule_initToPool(randHold, params, id, &leaf->mempool);
}

void tRandAndHoldModule_setParameter(tRandAndHoldModule const randHold, const RandHoldParams param_type, float input)
{
    switch (param_type) {
        case RandHoldEventWatchFlag:
            break;
        // case RandHoldTriggerToggle:
        //     randHold->triggerToggle = input;
        //     break;
        // case RandHoldThreshold:
        //     randHold->threshold = input;
        //     break;
        case RandHoldAmp:
            tSlopeRamp_setDest(&randHold->ampSmoother, input * 2.f);
            break;
        default:
            break;
    }
}

void tRandAndHoldModule_initToPool(void** const RandHold, float* const param, float id, tMempool** const mempool)
{
    tMempool* m = *mempool;
    _tRandAndHoldModule* RandAndHoldModule = (_tRandAndHoldModule*) (*RandHold = (_tRandAndHoldModule*) mpool_alloc (sizeof (_tRandAndHoldModule), m));

    RandAndHoldModule->header.uniqueID = id;
    RandAndHoldModule->mempool = m;
    RandAndHoldModule->header.moduleType = ModuleTypeRandAndHoldModule;

    RandAndHoldModule->currRand = 0.f;

    tSlopeRamp_init(m->leaf, &RandAndHoldModule->ampSmoother, SMOOTH_SLOPE_MULTIPLIER * 2.f, 1.f);
}

void tRandAndHoldModule_free(void** const RandHold)
{
    _tRandAndHoldModule* RandAndHoldModule = (_tRandAndHoldModule*) (*RandHold);
    mpool_free((char*)RandAndHoldModule, RandAndHoldModule->mempool);
}

void tRandAndHoldModule_onNoteOn(tRandAndHoldModule const randHold, float vel)
{
    if (vel > 0)
    {
        randHold->noteOn = 1;
        randHold->currRand = randHold->mempool->leaf->random();
        //printf("Rand: %f", randHold->currRand);
    }
}

// tick function
void tRandAndHoldModule_tick (tRandAndHoldModule const randHold)
{
    randHold->amp = tSlopeRamp_tick(&randHold->ampSmoother);

    //float currVal = CPPDEREF buffer;

    // if (randHold->noteOn == 1 || (randHold->triggerToggle && currVal > randHold->threshold))
    // {
    //     randHold->noteOn = 0;
    //     randHold->currRand = currVal * randHold->amp;
    // }

    randHold->header.outputs[0] = randHold->currRand * randHold->amp;
}
