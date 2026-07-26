//
// Created by Matthew McWeeney on 7/14/26.
//

#include "SimpleNoiseModule.h"

void tSimpNoiseModule_init(void** const simpNoise, float* params, float id, LEAF* const leaf)
{
    tSimpNoiseModule_initToPool(simpNoise, params, id, &leaf->mempool);
}

void tSimpNoiseModule_setParameter(tSimpNoiseModule const simpNoise, const SimpNosParams param_type, float input)
{
    switch (param_type) {
        case SimpNoiseGain:
            simpNoise->gain = input;
            break;
        default:
            break;
    }
}

void tSimpNoiseModule_initToPool(void** const simpNoise, float* const param, float id, tMempool** const mempool)
{
    tMempool* m = *mempool;
    _tSimpNoiseModule* SimpNoiseModule = (_tSimpNoiseModule*) (*simpNoise = (_tSimpNoiseModule*) mpool_alloc (sizeof (_tSimpNoiseModule), m));

#ifndef __cplusplus
    memcpy(SimpNoiseModule->params, param, SimpNoiseNumParams*sizeof(float));
#endif __cplusplus
    SimpNoiseModule->header.uniqueID = id;

    SimpNoiseModule->mempool = m;

    tNoise_create (&SimpNoiseModule->mempool, (tNoise**)&SimpNoiseModule->theSimpNoise);
    tNoise_init (SimpNoiseModule->mempool->leaf, (tNoise*)SimpNoiseModule->theSimpNoise, WhiteNoise);

    SimpNoiseModule->header.moduleType = ModuleTypeSimpNoiseModule;

#ifndef __cplusplus
    for (int i = 0; i < SimpNoiseNumParams; i++)
    {
    	tSimpNoiseModule_setParameter(SimpNoiseModule, i, SimpNoiseModule->params[i]);

    }
#endif

}

void tSimpNoiseModule_free(void** const simpNoise)
{
    _tSimpNoiseModule* SimpNoiseModule = (_tSimpNoiseModule*) (*simpNoise);
    tNoise_free((tNoise**)&SimpNoiseModule->theSimpNoise);
    mpool_free((char*)SimpNoiseModule, SimpNoiseModule->mempool);
}

// tick function
void tSimpNoiseModule_tick (tSimpNoiseModule const simpNoise)
{
    // const float input = filt->header.externalInputSum[0].exchange(0.0f, std::memory_order_relaxed);
    // buffer[0] += input;

    simpNoise->header.outputs[0] = tNoise_tick((tNoise*)simpNoise->theSimpNoise) * simpNoise->gain;
}
