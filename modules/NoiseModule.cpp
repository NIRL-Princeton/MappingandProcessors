//
// Created by Matthew McWeeney on 7/9/26.
//

#include "NoiseModule.h"
#include <iostream>

void tNoiseModule_init(void** const noise, float* params, float id, LEAF* const leaf)
{
    if(leaf->resTable == NULL)
    {
        tLookupTable_create(&leaf->mempool,&leaf->resTable);
        tLookupTable_init(leaf, leaf->resTable,  0.0f, 10.0f, 0.5f, 2048);
    }
    tNoiseModule_initToPool(noise, params, id, &leaf->mempool, leaf->resTable);
    //noise->theNoise
}

void tNoiseModule_setParameter(tNoiseModule const noise, const NosParams param_type, float input)
{
    switch (param_type) {
        case NoiseGain:
            noise->gain = input;
            break;

        case NoiseTilt:
            input = (20.f * input) - 10.f;
            noise->tilt = input;
            tTiltFilter_setTilt((tTiltFilter*)&noise->theTilter, input);
            break;

        case NoisePeakGain:
            noise->peakGain = input;
            tVZFilterBell_setGain((tVZFilterBell*)&noise->theBellter, input * 9.f + 1.f); // < 1 causes a dip at the target frequency - might add later
            break;

        case NoisePeakFreq:
            noise->peakFreq = input;
            tVZFilterBell_setFreq((tVZFilterBell*)&noise->theBellter, input * 19880.f + 20.f);
            break;

        case NoisePeakBandwidth:
            noise->peakBandwidth = input;
            tVZFilterBell_setBandwidth((tVZFilterBell*)&noise->theBellter, input * 5.f);

        default:
            break;
    }
}

void tNoiseModule_initToPool(void** const noise, float* const param, float id, tMempool** const mempool, tLookupTable* resTable)
{
    tMempool* m = *mempool;
    _tNoiseModule* NoiseModule = (_tNoiseModule*) (*noise = (_tNoiseModule*) mpool_alloc (sizeof (_tNoiseModule), m));

#ifndef __cplusplus
    memcpy(NoiseModule->params, param, NoiseNumParams*sizeof(float));
#endif __cplusplus
    NoiseModule->header.uniqueID = id;

    NoiseModule->mempool = m;

    tNoise_create (&NoiseModule->mempool, (tNoise**)&NoiseModule->theNoise);
    tNoise_init (NoiseModule->mempool->leaf, (tNoise*)NoiseModule->theNoise, WhiteNoise);

    NoiseModule->header.moduleType = ModuleTypeNoiseModule;

    tTiltFilter_create(&NoiseModule->mempool, (tTiltFilter**)&NoiseModule->theTilter);
    tTiltFilter_init(NoiseModule->mempool->leaf, (tTiltFilter*)&NoiseModule->theTilter, 1000.f);
    //tTiltFilter_setSampleRate((tTiltFilter*)&NoiseModule->theTilter, NoiseModule->mempool->leaf->sampleRate * 2.f);

    tVZFilterBell_create(&NoiseModule->mempool, (tVZFilterBell**)&NoiseModule->theBellter);
    tVZFilterBell_init(NoiseModule->mempool->leaf, (tVZFilterBell*)&NoiseModule->theBellter, 1000.f, 50.f, 1.f);
    //tVZFilterBell_setSampleRate((tVZFilterBell*)&NoiseModule->theBellter, NoiseModule->mempool->leaf->sampleRate * 2.f);
#ifndef __cplusplus
    for (int i = 0; i < NoiseNumParams; i++)
    {
    	tNoiseModule_setParameter(NoiseModule, i, NoiseModule->params[i]);

    }
#endif

}

void tNoiseModule_free(void** const noise)
{
    _tNoiseModule* NoiseModule = (_tNoiseModule*) (*noise);

    tTiltFilter_free((tTiltFilter**)&NoiseModule->theTilter);
    tVZFilterBell_free((tVZFilterBell**)&NoiseModule->theBellter);
    tNoise_free((tNoise**)&NoiseModule->theNoise);

    mpool_free((char*)NoiseModule, NoiseModule->mempool);
}

// tick function
void tNoiseModule_tick (tNoiseModule const noise,float* buffer)
{
    // const float input = filt->header.externalInputSum[0].exchange(0.0f, std::memory_order_relaxed);
    // buffer[0] += input;

    //*buffer = (noise->mempool->leaf->random()*2.f - 1.f);
    *buffer = tNoise_tick((tNoise*)noise->theNoise);
    *buffer = tTiltFilter_tick((tTiltFilter*)&noise->theTilter, *buffer);
    *buffer = tVZFilterBell_tick((tVZFilterBell*)&noise->theBellter, *buffer) * noise->gain;
    //*buffer = (*buffer * noise->gain)*2.f-1.0f;

    noise->header.outputs[0] = *buffer;
}

// Non-modulatable setters
// void tNoiseModule_setDBtoATableLocation (tNoiseModule const noise, float* const tableAddress, uint32_t const tableSize)
// {
//     noise->dbTableAddress = tableAddress;
//     noise->dbTableSizeMinusOne = (float)(tableSize - 1);
//     noise->dbTableScalar = noise->dbTableSizeMinusOne/(4.0f-0.00001f);
//     noise->dbTableOffset = 0.00001f * noise->dbTableScalar;
// }

// void tNoiseModule_setMTOFTableLocation (tNoiseModule const noise, float* const tableAddress)
// {
//     noise->mtofTable = tableAddress;
// }
//
// float dbToATableLookupFunctionNos(float const in, float const sizeMinusOne, float* const tableAddress)
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

