//
// Created by Matthew McWeeney on 8/28/26.
//

#include "SampleAndHoldModule.h"
#include "leaf-envelopes.h"

#include <cstdio>

void tSampleAndHoldModule_init(void** const sampHold, float* params, float id, LEAF* const leaf)
{
    tSampleAndHoldModule_initToPool(sampHold, params, id, &leaf->mempool);
}

void tSampleAndHoldModule_setParameter(tSampleAndHoldModule const sampHold, const SampHoldParams param_type, float input)
{
    switch (param_type) {
        case SampHoldEventWatchFlag:
            break;
        case SampHoldThreshold:
            sampHold->threshold = input;
            break;
        case SampHoldFrequency:
            sampHold->frequency = input;
            break;
        case SampHoldDurRand:
            sampHold->durRand = input;
            break;
        case SampHoldGain:
            tSlopeRamp_setDest(&sampHold->gainSmoother, input);
            break;
        case SampHoldMix:
            tSlopeRamp_setDest(&sampHold->mixSmoother, input);
            break;
        default:
            break;
    }
}

void tSampleAndHoldModule_initToPool(void** const sampHold, float* const param, float id, tMempool** const mempool)
{
    tMempool* m = *mempool;
    _tSampleAndHoldModule* SampleAndHoldModule = (_tSampleAndHoldModule*) (*sampHold = (_tSampleAndHoldModule*) mpool_alloc (sizeof (_tSampleAndHoldModule), m));

    SampleAndHoldModule->header.uniqueID = id;
    SampleAndHoldModule->mempool = m;
    SampleAndHoldModule->header.moduleType = ModuleTypeSampleAndHoldModule;

    tRamp_init(m->leaf, &SampleAndHoldModule->sampleSmoother, 10.f, 1);
    tSlopeRamp_init(m->leaf, &SampleAndHoldModule->gainSmoother, SMOOTH_SLOPE_MULTIPLIER * 4.f, 1.f);
    tSlopeRamp_init(m->leaf, &SampleAndHoldModule->mixSmoother, SMOOTH_SLOPE_MULTIPLIER, 1.f);

    SampleAndHoldModule->sampleRate = m->leaf->sampleRate;
    SampleAndHoldModule->invSampleRate = m->leaf->invSampleRate;

}

void tSampleAndHoldModule_free(void** const sampHold)
{
    _tSampleAndHoldModule* SampleAndHoldModule = (_tSampleAndHoldModule*) (*sampHold);
    mpool_free((char*)SampleAndHoldModule, SampleAndHoldModule->mempool);
}

void tSampleAndHoldModule_setBinLength(tSampleAndHoldModule const sampHold)
{
    sampHold->binLength = (1.f/sampHold->frequency*sampHold->sampleRate) * (1 + 2*(sampHold->mempool->leaf->random() - 0.5)*sampHold->durRand);
}

// tick function
void tSampleAndHoldModule_tick (tSampleAndHoldModule const sampHold, float* buffer)
{
    sampHold->mix = tSlopeRamp_tick(&sampHold->mixSmoother);
    sampHold->gain = tSlopeRamp_tick(&sampHold->gainSmoother);

    sampHold->counter++;

    if (sampHold->counter >= sampHold->binLength && buffer[0] >= sampHold->threshold) {
        sampHold->counter = 0;
        tRamp_setDest(&sampHold->sampleSmoother, buffer[0]);

        tSampleAndHoldModule_setBinLength(sampHold);
    }
    sampHold->currSample = tRamp_tick(&sampHold->sampleSmoother);

    buffer[0] = sampHold->header.outputs[0] = buffer[0] * (1.f - sampHold->mix) + sampHold->mix * sampHold->currSample;
}

