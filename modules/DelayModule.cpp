//
// Created by Myra Norton on 1/22/26.
//

#include "DelayModule.h"
#include <assert.h>
#include "string.h"
// lagrange and tape delay

void tDelayModule_init(void** const delay, float* params, float id, LEAF* const leaf) {
    tDelayModule_initToPool(delay, params, id, &leaf->mempool);
}

// float dbToATableLookupFunction(float const in, float const sizeMinusOne, float* const tableAddress)
// {
//     uint32_t inDBIndex = (uint32_t) in;
//     uint32_t inDBIndexPlusOne = inDBIndex + 1;
//     if (inDBIndexPlusOne > sizeMinusOne)
//     {
//         inDBIndexPlusOne = sizeMinusOne;
//     }
//     float alpha = in - (float)inDBIndex;
//     return ((tableAddress[inDBIndex] * (1.0f - alpha)) + (tableAddress[inDBIndexPlusOne] * alpha));
// }

void tDelayModule_setDelay(tDelayModule const delay, float const time)
{
    tDelay_setDelay((tDelay*)delay->theDelay, time*delay->sr);

}

void tDelayModule_setGain(tDelayModule const delay, float const gain)
{
	float floatIndex = 0.0f;
    delay->amp = gain;
    tDelay_setGain((tDelay*)delay->theDelay, gain);

}

void tDelayModule_initToPool(void** const delay, float* const params, float id, tMempool** const mempool)
{
    tMempool* m = *mempool;
    _tDelayModule* DelayModule =(_tDelayModule *) ( *delay = (_tDelayModule*) mpool_alloc(sizeof(_tDelayModule), m));
#ifndef __cplusplus
    memcpy(DelayModule->header.params, params, DelayNumParams*sizeof(float));

#endif
    DelayModule->header.uniqueID = id;

    DelayModule->mempool = m;
    DelayModule->invSr = m->leaf->invSampleRate;
    DelayModule->sr = m->leaf->sampleRate;

    tDelay_create(mempool, (tDelay**)&DelayModule->theDelay);
    tDelay_init(m->leaf,(tDelay*)DelayModule->theDelay, 0.0f, 100000.f);

    // switch(type)
    // {
    //     case DelayTypeDelay:
    //         tDelay_create(mempool, (tDelay**)&DelayModule->theDelay);
    //         tDelay_init(m->leaf,(tDelay*)DelayModule->theDelay, 10000.0f, 0.5f);
    //     case DelayTypeLinear:
    //         tLinearDelay_create(mempool, (tLinearDelay**)&DelayModule->theDelay);
    //         tLinearDelay_init(m->leaf,(tLinearDelay*)DelayModule->theDelay, 10000.0f, 0.5f);
    //     case DelayTypeHermite:
    //         tHermiteDelay_create(mempool, (tHermiteDelay**)&DelayModule->theDelay);
    //         tHermiteDelay_init(m->leaf,(tHermiteDelay*)DelayModule->theDelay, 10000.0f, 0.5f);
    //     case DelayTypeLagrange:
    //         tLagrangeDelay_create(mempool, (tLagrangeDelay**)&DelayModule->theDelay);
    //         tLagrangeDelay_init(m->leaf,(tLagrangeDelay*)DelayModule->theDelay, 10000.0f, 0.5f);
    //     case DelayTypeAllpass:
    //         tAllpassDelay_create(mempool, (tAllpassDelay**)&DelayModule->theDelay);
    //         tAllpassDelay_init(m->leaf,(tAllpassDelay*)DelayModule->theDelay, 10000.0f, 0.5f);
    //     case DelayTypeTape:
    //         tTapeDelay_create(mempool, (tTapeDelay**)&DelayModule->theDelay);
    //         tTapeDelay_init(m->leaf,(tTapeDelay*)DelayModule->theDelay, 10000.0f, 0.5f);
    //     default:
    //         break;
    // }
    DelayModule->header.moduleType = ModuleTypeDelayModule;
    /*
    if(m->leaf->delayTimeTable == NULL)
    {
        tLookupTable_create(&m->leaf->mempool, &m->leaf->delayTimeTable);
        tLookupTable_init(m->leaf, m->leaf->delayTimeTable,  0.0f, 2250.0f, 200.0f, 2048);
    }
    */
#ifndef __cplusplus
    for (int i = 0; i < DelayNumParams; i++)
    {
    	tDelayModule_setParameter(DelayModule, i, DelayModule->header.params[i]);

    }
#endif

}

void tDelayModule_free(void** const delay)
{

    _tDelayModule* DelayModule = (_tDelayModule*) (*delay);
    mpool_free ((char*) DelayModule, DelayModule->mempool);
}

// tick function
void tDelayModule_tick (tDelayModule const delay)
{
    const float input = delay->header.inputs[0];
    delay->header.inputs[0] = 0.0f;
    delay->header.outputs[0] = tDelay_tick((tDelay*)delay->theDelay,  input) * delay->amp;

}

// Modulatable setters

void tDelayModule_setParameter(tDelayModule const delay, DelParams param_type,float input)
{
    switch (param_type) {
        case DelayTime:
            tDelayModule_setDelay(delay, input);
            break;
        case DelayGain:
            tDelayModule_setGain(delay, input);
            break;

        default:
            break;
    }
}


void tDelayModule_setSampleRate (tDelayModule const delay, float const sr)
{
    //tDelay_setSampleRate(delay, sr);
}
