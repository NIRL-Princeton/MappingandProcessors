//
// Created by Jeffrey Snyder on 7/9/24.
//


#include "SoftClipModule.h"

#include <assert.h>


void tSoftClipModule_init(void** const c, float* params, float id, LEAF* const leaf)
{
    tSoftClipModule_initToPool(c, params, id, &leaf->mempool);
}


void tSoftClipModule_free(void** const c)
{
    _tSoftClipModule* SoftClipModule = static_cast<_tSoftClipModule *>(*c);

    mpool_free((char*)SoftClipModule, SoftClipModule->mempool);
}
//tick function
void tSoftClipModule_tick (tSoftClipModule const c, float* buffer)
{
    const float input = c->header.summedInput + buffer[0];
    c->header.summedInput = 0.0f;

    float sample = input;
    c->inputGain = tSlopeRamp_tick(&c->inputGainSmoother);
    c->outputGain = tSlopeRamp_tick(&c->outputGainSmoother);
    c->shapeDivider = tSlopeRamp_tick(&c->shapeDividerSmoother);
    c->offset = tSlopeRamp_tick(&c->offsetSmoother);
    c->mix = tSlopeRamp_tick(&c->mixSmoother);

    sample = sample * c->inputGain * 5.0f;
    sample = sample + (c->offset * 2.0f) - 1.0f;

    if (sample <= -1.0f)
    {
        sample = -1.0f;
    } else if (sample >= 1.0f)
    {
        sample = 1.0f;
    }
    {
        sample = 1.5f * (sample) - (((sample * sample * sample))* 0.3333333f);
        sample = sample * c->shapeDivider;
    }

    sample = tHighpass_tick(&c->highpass, sample) * c->outputGain * c->mix + input * (1-c->mix);
    buffer[0] = c->header.outputs[0] = sample;

}

void tSoftClipModule_setParameter(tSoftClipModule const  c, int parameter_id, float input)
{
    switch (parameter_id)
    {
        case SoftClipEventWatchFlag:
        {
            // handled by onnoteon listener
            break;
        }

        case SoftClipInputGain:
        {
            //c->inputGain = input;
            tSlopeRamp_setDest(&c->inputGainSmoother, input * TEN_DB_AMPLITUDE);
            break;
        }

        case SoftClipOffset:
        {
            //c->offset = input;
            tSlopeRamp_setDest(&c->offsetSmoother, input);
            break;
        }

        case SoftClipShape:
        {
            float shape = (input * .99f) + 0.01f;
            //c->shapeDivider = 1.0f / (shape - ((shape*shape*shape) * 0.3333333f));
            tSlopeRamp_setDest(&c->shapeDividerSmoother, 1.0f / (shape - ((shape*shape*shape) * 0.3333333f)));
            break;
        }
        case SoftClipOutputGain:
        {
            //c->outputGain = input;
            tSlopeRamp_setDest(&c->outputGainSmoother, input * TEN_DB_AMPLITUDE);
            break;
        }
        case SoftClipMix:
            tSlopeRamp_setDest(&c->mixSmoother, input);
            break;
        default:
            break;
    }
}


void tSoftClipModule_initToPool(void** const c, float* const params, float id, tMempool** const mempool)
{
    tMempool* m = *mempool;
    _tSoftClipModule* SoftClipModule = static_cast<_tSoftClipModule *>(*c = (_tSoftClipModule*) mpool_alloc(sizeof(_tSoftClipModule), m));
#ifndef __cplusplus
    memcpy(SoftClipModule->params, params, SoftClipNumParams);
#endif __cplusplus
    SoftClipModule->mempool = m;
    tHighpass_init   (SoftClipModule->mempool->leaf, &SoftClipModule->highpass, 20.0f);
    SoftClipModule->mempool = m;
    SoftClipModule->inputGain = 1.0f;
    SoftClipModule->outputGain = 1.0f;
    SoftClipModule->offset = 0.0f;
    SoftClipModule->shapeDivider = 1.0f;

    SoftClipModule->header.uniqueID = id;

    tSlopeRamp_init(m->leaf, &SoftClipModule->inputGainSmoother, SMOOTH_SLOPE_MULTIPLIER * TEN_DB_AMPLITUDE, 1.f);
    tSlopeRamp_init(m->leaf, &SoftClipModule->outputGainSmoother, SMOOTH_SLOPE_MULTIPLIER * TEN_DB_AMPLITUDE, 1.f);
    tSlopeRamp_init(m->leaf, &SoftClipModule->shapeDividerSmoother, SMOOTH_SLOPE_MULTIPLIER * 100.f, 1.f);
    tSlopeRamp_init(m->leaf, &SoftClipModule->offsetSmoother, SMOOTH_SLOPE_MULTIPLIER, 0.f);
    tSlopeRamp_init(m->leaf, &SoftClipModule->mixSmoother, SMOOTH_SLOPE_MULTIPLIER, 1.f);

    SoftClipModule->header.moduleType = ModuleTypeSoftClipModule;
}


