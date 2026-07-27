//
// Created by Matthew McWeeney on 7/15/26.
//

#include "PerlinNoiseModule.h"

void tPerlNoiseModule_init(void** const perlNoise, float* params, float id, LEAF* const leaf)
{
    tPerlNoiseModule_initToPool(perlNoise, params, id, &leaf->mempool);
}

void tPerlNoiseModule_setParameter(tPerlNoiseModule const perlNoise, const PerlNosParams param_type, float input)
{
    switch (param_type) {
        case PerlNoiseGain:
            perlNoise->gain = input;
            break;

        case PerlNoiseRate:
            tPerlNoiseModule_setRate (perlNoise->mempool->leaf, perlNoise, input * 199.99f + 0.01f);
            break;

        case PerlNoiseEnergy:
            tPerlNoiseModule_setEnergy (perlNoise->mempool->leaf, perlNoise, input);
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
    const float input = perlNoise->header.externalInputSum[0].exchange(0.0f, std::memory_order_relaxed);
    // buffer[0] += input;
    perlNoise->header.outputs[0] = tPerlinNoise_tick((tPerlinNoise*)perlNoise->thePerlNoise) * perlNoise->gain;
    //perlNoise->header.outputs[0] = 0.0f;
}

void tPerlNoiseModule_setRate(LEAF* const leaf, tPerlNoiseModule const perlNoise, float rate)
{
    tPerlinNoise_setRate(leaf, (tPerlinNoise*)perlNoise->thePerlNoise, rate);
}

void tPerlNoiseModule_setEnergy(LEAF* const leaf, tPerlNoiseModule const perlNoise, float energy)
{
    tPerlinNoise_setEnergy(leaf, (tPerlinNoise*)perlNoise->thePerlNoise, energy);
}