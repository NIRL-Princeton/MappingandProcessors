//
// Created by Matthew McWeeney on 7/15/26.
//

#include "PerlinNoiseModule.h"
#include <cstdio>

void tPerlNoiseModule_init(void** const perlNoise, float* params, float id, LEAF* const leaf)
{
    tPerlNoiseModule_initToPool(perlNoise, params, id, &leaf->mempool);
}

void tPerlNoiseModule_setParameter(tPerlNoiseModule const perlNoise, const PerlNosParams param_type, float input)
{
    float temp;
    switch (param_type) {
        case PerlNoiseGain:
            if (perlNoise->gainSmoother.dest != input)
            {
                tSlopeRamp_setDest(&perlNoise->gainSmoother, input);
            }
            break;
        case PerlNoiseRate:
            temp = input * 1999.9f + 0.1f;
            if (perlNoise->rateMs != temp)
            {
                tPerlNoiseModule_setRate (perlNoise, input * 1999.9f + 0.1f);
            }
            break;
        case PerlNoiseEnergy:
            if (perlNoise->energy != input)
            {
                tPerlNoiseModule_setEnergy(perlNoise, input);
            }
            break;
        default:
            break;
    }
}

void tPerlNoiseModule_initToPool(void** const perlNoise, float* const param, float id, tMempool** const mempool)
{
    tMempool* m = *mempool;
    _tPerlNoiseModule* PerlNoiseModule = (_tPerlNoiseModule*) (*perlNoise = (_tPerlNoiseModule*) mpool_alloc (sizeof (_tPerlNoiseModule), m));

    #ifndef __cplusplus
    memcpy(PerlNoiseModule->params, param, PerlNoiseNumParams*sizeof(float));
    #endif __cplusplus
    PerlNoiseModule->header.uniqueID = id;

    PerlNoiseModule->mempool = m;

    tPerlinNoise_create (&PerlNoiseModule->mempool, (tPerlinNoise**)&PerlNoiseModule->thePerlNoise);
    tPerlinNoise_init (PerlNoiseModule->mempool->leaf, (tPerlinNoise*)PerlNoiseModule->thePerlNoise, 4.f, .5f);
    tSlopeRamp_init (PerlNoiseModule->mempool->leaf, &PerlNoiseModule->gainSmoother, SMOOTH_SLOPE_MULTIPLIER, 0.5f);

    PerlNoiseModule->header.moduleType = ModuleTypePerlNoiseModule;

    #ifndef __cplusplus
    for (int i = 0; i < PerlNoiseNumParams; i++)
    {
        tPerlNoiseModule_setParameter(PerlNoiseModule, i, PerlNoiseModule->params[i]);

    }
    #endif

}

void tPerlNoiseModule_free(void** const perlNoise)
{
    _tPerlNoiseModule* PerlNoiseModule = (_tPerlNoiseModule*) (*perlNoise);
    tPerlinNoise_free((tPerlinNoise**)&PerlNoiseModule->thePerlNoise);
    mpool_free((char*)PerlNoiseModule, PerlNoiseModule->mempool);
}

// tick function
void tPerlNoiseModule_tick (tPerlNoiseModule const perlNoise)
{
    tPerlNoiseModule_setGain(perlNoise, tSlopeRamp_tick(&perlNoise->gainSmoother));

    perlNoise->header.outputs[0] = tPerlinNoise_tick((tPerlinNoise*)perlNoise->thePerlNoise) * perlNoise->gain;
    //float output = tPerlinNoise_tick((tPerlinNoise*)perlNoise->thePerlNoise) * perlNoise->gain;
    //perlNoise->header.outputs[0] = output;
    //printf("%f,", output);
}

void tPerlNoiseModule_setRate(tPerlNoiseModule const perlNoise, float rate)
{
    perlNoise->rateMs = rate;
    tPerlinNoise_setRate((tPerlinNoise*)perlNoise->thePerlNoise, rate);
}

void tPerlNoiseModule_setEnergy(tPerlNoiseModule const perlNoise, float energy)
{
    perlNoise->energy = energy;
    tPerlinNoise_setEnergy((tPerlinNoise*)perlNoise->thePerlNoise, energy);
}

void tPerlNoiseModule_setGain (tPerlNoiseModule const perlNoise, float gain)
{
    perlNoise->gain = gain;
}