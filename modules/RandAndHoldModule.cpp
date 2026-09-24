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
        case RandHoldThreshold:
            input = randHold->gainAmpTable->table[(int)(input*2047)];
            randHold->threshold = input;
            break;
        case RandHoldFrequency:
            randHold->frequency = randHold->skewFreqTable->table[(int)(input*16383)];
            if (randHold->frequency < 0.0011)
            {
                randHold->hold = 1;
            } else
            {
                randHold->hold = 0;
                tRandAndHoldModule_setBinLength(randHold);
            }
            break;
        case RandHoldDurRand:
            randHold->durRand = input;
            tRandAndHoldModule_setBinLength(randHold);
            break;
        case RandHoldGain:
            input = randHold->gainAmpTable->table[(int)(input*2047)];
            tSlopeRamp_setDest(&randHold->gainSmoother, input);
            break;
        case RandHoldMix:
            tSlopeRamp_setDest(&randHold->mixSmoother, input);

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

    RandAndHoldModule->frequency = 10.f;
    RandAndHoldModule->binLength = 1/10.f*44100.f;
    RandAndHoldModule->counter = 0;
    RandAndHoldModule->currRand = 0.f;

    tSlopeRamp_init(m->leaf, &RandAndHoldModule->gainSmoother, SMOOTH_SLOPE_MULTIPLIER * 4.f, 1.f);
    tSlopeRamp_init(m->leaf, &RandAndHoldModule->mixSmoother, SMOOTH_SLOPE_MULTIPLIER, 1.f);

    RandAndHoldModule->RandRate = m->leaf->sampleRate;
    RandAndHoldModule->invRandRate = m->leaf->invSampleRate;

    tLookupTable_create(&RandAndHoldModule->mempool, &RandAndHoldModule->skewFreqTable);
    tLookupTable_init (RandAndHoldModule->mempool->leaf, RandAndHoldModule->skewFreqTable, 0.001f, 20000.f, 20.f, 16384);

    tLookupTable_create(&RandAndHoldModule->mempool, &RandAndHoldModule->gainAmpTable);
    tLookupTable_init (RandAndHoldModule->mempool->leaf, RandAndHoldModule->gainAmpTable, 0.f, 4.f, 1.f, 2048);

    tLookupTable_create(&RandAndHoldModule->mempool, &RandAndHoldModule->mtofTable);
    tLookupTable_init (RandAndHoldModule->mempool->leaf, RandAndHoldModule->mtofTable, 0.f, 0.f, 0.f, 16384);
    LEAF_generate_mtof (RandAndHoldModule->mtofTable->table, 0., 127, 16384);
}

void tRandAndHoldModule_free(void** const RandHold)
{
    _tRandAndHoldModule* RandAndHoldModule = (_tRandAndHoldModule*) (*RandHold);
    mpool_free((char*)RandAndHoldModule, RandAndHoldModule->mempool);
}

void tRandAndHoldModule_onNoteOn(tRandAndHoldModule const randHold, float vel)
{
    if (vel > -1)
    {
        randHold->counter = randHold->binLength;
    }
}

void tRandAndHoldModule_setBinLength(tRandAndHoldModule const randHold)
{
    randHold->finalFreq = LEAF_clip(0.f, (randHold->frequency + randHold->pitch * randHold->keyFollow) * randHold->harmonicMultiplier, 2.f*randHold->RandRate);
    randHold->binLength = (1.f/randHold->finalFreq*randHold->RandRate) * (1 + 2*(randHold->mempool->leaf->random() - 0.5)*randHold->durRand);
    //randHold->binLength = 1.f/randHold->frequency*randHold->RandRate;
}

// tick function
void tRandAndHoldModule_tick (tRandAndHoldModule const randHold, float* buffer)
{
    randHold->mix = tSlopeRamp_tick(&randHold->mixSmoother);
    randHold->gain = tSlopeRamp_tick(&randHold->gainSmoother);

    randHold->counter++;
    //printf("%f\n", buffer[0]);

    if (randHold->counter > randHold->binLength && buffer[0] >= randHold->threshold && randHold->hold == 0){
        randHold->counter = 0;
        randHold->currRand = buffer[0] * randHold->gain;
        tRandAndHoldModule_setBinLength(randHold);
    }
    //randHold->currRand = tRamp_tick(&randHold->RandSmoother) * randHold->gain;

    buffer[0] = randHold->header.outputs[0] = buffer[0] * (1.f - randHold->mix) + randHold->mix * randHold->currRand;
    //buffer[0] = randHold->header.outputs[0] = buffer[0];
    //buffer[0] = randHold->header.outputs[0] = randHold->currRand;
}
