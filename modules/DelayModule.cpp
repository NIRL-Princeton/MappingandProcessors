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
	tTapeDelay_setDelay(delay->theDelay, time*30000.0f);

}

void tDelayModule_setGain(tDelayModule const delay, float const gain)
{
    delay->amp = LEAF_interpolateTableLookupFunction(gain, delay->gainAmpTable);;
    //tLagrangeDelay_setGain((tDelay*)delay->theDelay, gain);

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

    tTapeDelay_create(mempool,&DelayModule->theDelay);
    tTapeDelay_init(m->leaf,DelayModule->theDelay,2000, 30000);
    tSVF_init(m->leaf,&DelayModule->lp, SVFTypeLowpass, 16000.f, .7f);
    tSVF_init(m->leaf,&DelayModule->hp, SVFTypeHighpass, 20.f, .7f);
    tHighpass_init(m->leaf,&DelayModule->shaperHp, 20.0f);
    tFeedbackLeveler_init(m->leaf,&DelayModule->feedbackControl, .99f, 0.01f, 0.125f, 0);
    //tLagrangeDelay_create(mempool,DelayModule->theDelay (tLagrangeDelay**)&DelayModule->theDelay);
    //tLagrangeDelay_init(m->leaf,(tLagrangeDelay*)DelayModule->theDelay, 0.0f, 100000.f);

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

    if (DelayModule->mempool->leaf->gainAmpTable == NULL)
    {
   	 tLookupTable_create(&DelayModule->mempool, &DelayModule->mempool->leaf->gainAmpTable);
   	 tLookupTable_init (DelayModule->mempool->leaf, DelayModule->mempool->leaf->gainAmpTable, 0.f, TWELVE_DB_AMPLITUDE, 1.f, 2048);
    }
    DelayModule->gainAmpTable = DelayModule->mempool->leaf->gainAmpTable;

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
    tTapeDelay_free(&DelayModule->theDelay);
    mpool_free ((char*) DelayModule, DelayModule->mempool);
}

// tick function
void tDelayModule_tick (tDelayModule const delay)
{
    const float input = delay->header.inputs[0];
    delay->header.inputs[0] = 0.0f;

    float input1 = tFeedbackLeveler_tick(&delay->feedbackControl, tanhf(input + (delay->fbSample * delay->fbAmount)));

    delay->fbSample = tTapeDelay_tick(delay->theDelay, input1);
    delay->fbSample = tSVF_tick(&delay->lp, delay->fbSample);
    delay->fbSample = tanhf(tSVF_tick(&delay->hp, delay->fbSample));

    delay->header.outputs[0] = ((delay->fbSample * delay->amp)*delay->mix) + ((1.0f-delay->mix)*input);

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
        case DelayHPCutoff:
            delay->cutoff1 = LEAF_clip(10.0f,faster_mtof((input * 133.0f) + 3.0f),
                                                  20000.0f);
            tSVF_setFreq(&delay->hp, delay->cutoff1);

            break;
        case DelayLPCutoff:
            delay->cutoff2 = LEAF_clip(10.0f,faster_mtof((input * 133.0f) + 3.0f),
                                                  20000.0f);
            tSVF_setFreq(&delay->lp, delay->cutoff2);
            break;
        case DelayFeedback:
        	delay->fbAmount = input* 1.1f;
            break;
        case DelayMix:
            delay->mix = input;
            break;
        default:
            break;
    }
}


void tDelayModule_setSampleRate (tDelayModule const delay, float const sr)
{
    //tDelay_setSampleRate(delay, sr);
}
