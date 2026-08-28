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
        case SampHoldThreshold:
            sampHold->threshold = input;
            break;
        case SampHoldFrequency:
            sampHold->frequency = input;
            break;
        default:
            break;
    }
}

void tSampleAndHoldModule_initToPool(void** const sampHold, float* const param, float id, tMempool** const mempool)
{
    tMempool* m = *mempool;
    _tSampleAndHoldModule* SampleAndHoldModule = (_tSampleAndHoldModule*) (*sampHold = (_tSampleAndHoldModule*) mpool_alloc (sizeof (_tSampleAndHoldModule), m));

    #ifndef __cplusplus
    memcpy(SampleAndHoldModule->params, param, SampleAndHoldNumParams*sizeof(float));
    #endif __cplusplus
    SampleAndHoldModule->header.uniqueID = id;

    SampleAndHoldModule->mempool = m;

    SampleAndHoldModule->header.moduleType = ModuleTypeSampleAndHoldModule;

    #ifndef __cplusplus
    for (int i = 0; i < PerlNoiseNumParams; i++)
    {
        tPerlNoiseModule_setParameter(PerlNoiseModule, i, PerlNoiseModule->params[i]);

    }
    #endif

}

void tSampleAndHoldModule_free(void** const sampHold)
{
    _tSampleAndHoldModule* SampleAndHoldModule = (_tSampleAndHoldModule*) (*sampHold);

    mpool_free((char*)SampleAndHoldModule, SampleAndHoldModule->mempool);
}

// tick function
void tSampleAndHoldModule_tick (tSampleAndHoldModule const sampHold, float* buffer)
{
    sampHold->mix = tSlopeRamp_tick(&sampHold->mixSmoother);
    sampHold->gain = tSlopeRamp_tick(&sampHold->gainSmoother);

    float output = 0.f;
    buffer[0] = sampHold->header.outputs[0] = buffer[0] * (1.f - sampHold->mix) + sampHold->mix * output;
}

