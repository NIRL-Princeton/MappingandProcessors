//
// Created by Matthew McWeeney on 7/15/26.
//

#include "PerlinNoiseModule.h"
#include <cstdio>

void tPerlNoiseModule_init(void** const perlNoise, float* params, float id, LEAF* const leaf)
{
    if(leaf->lfoRateTable == NULL)
    {
        tLookupTable_create(&leaf->mempool, &leaf->lfoRateTable);
        tLookupTable_init(leaf,leaf->lfoRateTable, 0.f, 30.f, 2.f,2048);
    }
    tPerlNoiseModule_initToPool(perlNoise, params, id, &leaf->mempool, leaf->lfoRateTable);
}

void tPerlNoiseModule_setParameter(tPerlNoiseModule const perlNoise, const PerlNosParams param_type, float input)
{
    switch (param_type) {
        case PerlNoiseGain:
            if (perlNoise->gainSmoother.dest != input)
            {
                tSlopeRamp_setDest(&perlNoise->gainSmoother, input);
            }
            break;
        case PerlNoiseRate:
            if (perlNoise->inputRateHz != input)
            {
                perlNoise->inputRateHz = input;

                tPerlNoiseModule_setRate (perlNoise, perlNoise->table->table[(int)roundf(input * 2047)]);
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

void tPerlNoiseModule_initToPool(void** const perlNoise, float* const param, float id, tMempool** const mempool, tLookupTable* rateTable)
{
    tMempool* m = *mempool;
    _tPerlNoiseModule* PerlNoiseModule = (_tPerlNoiseModule*) (*perlNoise = (_tPerlNoiseModule*) mpool_alloc (sizeof (_tPerlNoiseModule), m));

    #ifndef __cplusplus
    memcpy(PerlNoiseModule->params, param, PerlNoiseNumParams*sizeof(float));
    #endif __cplusplus
    PerlNoiseModule->header.uniqueID = id;
    PerlNoiseModule->table = rateTable;

    PerlNoiseModule->mempool = m;

    //tPerlinNoise_create (&PerlNoiseModule->mempool, &PerlNoiseModule->thePerlNoise);
    tPerlinNoise_init (PerlNoiseModule->mempool->leaf, &PerlNoiseModule->thePerlNoise, 4.f, .5f);
    //tSlopeRamp_create(&PerlNoiseModule->mempool, &PerlNoiseModule->gainSmoother);
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
    // tSlopeRamp_free(&PerlNoiseModule->gainSmoother);
    // tPerlinNoise_free(&PerlNoiseModule->thePerlNoise);
    mpool_free((char*)PerlNoiseModule, PerlNoiseModule->mempool);
}

// tick function
void tPerlNoiseModule_tick (tPerlNoiseModule const perlNoise)
{
    tPerlNoiseModule_setGain(perlNoise, tSlopeRamp_tick(&perlNoise->gainSmoother));

    perlNoise->header.outputs[0] = tPerlinNoise_tick(&perlNoise->thePerlNoise) * perlNoise->gain;
    //float output = tPerlinNoise_tick(perlNoise->thePerlNoise) * perlNoise->gain;
    //perlNoise->header.outputs[0] = output;
    //printf("%f,", output);
}

void tPerlNoiseModule_setRate(tPerlNoiseModule const perlNoise, float rate)
{
    perlNoise->rateHz = rate;
    tPerlinNoise_setRate(&perlNoise->thePerlNoise, 1000.f/rate);
}

void tPerlNoiseModule_setEnergy(tPerlNoiseModule const perlNoise, float energy)
{
    perlNoise->energy = energy;
    tPerlinNoise_setEnergy(&perlNoise->thePerlNoise, energy);
}

void tPerlNoiseModule_setGain (tPerlNoiseModule const perlNoise, float gain)
{
    perlNoise->gain = gain;
}