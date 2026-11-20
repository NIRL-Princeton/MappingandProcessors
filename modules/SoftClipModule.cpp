//
// Created by Jeffrey Snyder on 7/9/24.
//


#include "SoftClipModule.h"

#include <assert.h>
#include "string.h"

void tSoftClipModule_init(void** const c, float* params, float id, LEAF* const leaf)
{
    tSoftClipModule_initToPool(c, params, id, &leaf->mempool);
}


void tSoftClipModule_free(void** const c)
{
    _tSoftClipModule* SoftClipModule = (_tSoftClipModule*) (*c);
    mpool_free((char*)SoftClipModule, SoftClipModule->mempool);
}
//tick function
void tSoftClipModule_tick (tSoftClipModule const c)
{


    const float input = c->header.inputs[0];
    c->header.inputs[0] = 0.0f;

    float sample = input;


    sample = sample * 5.0f * c->inputGain;
    sample = sample + c->offset;

    sample = tanhf(sample);

    /*
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
*/
    sample = (tHighpass_tick(&c->highpass, sample) * c->outputGain * c->mix) + (input * (1.0f-c->mix));
    c->header.outputs[0] = sample;

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

            c->inputGain = LEAF_interpolateTableLookupFunction(input, c->gainAmpTable);
            //c->inputGain = input;

            break;
        }

        case SoftClipOffset:
        {
            //c->offset = input;
            c->offset = input * 2.0f - 1.0f;
            break;
        }

        case SoftClipShape:
        {
            float shape = (input * .99f) + 0.01f;
            //c->shapeDivider = 1.0f / (shape - ((shape*shape*shape) * 0.3333333f));
            c->shapeDivider =  1.0f / (shape - ((shape*shape*shape) * 0.3333333f));
            break;
        }
        case SoftClipOutputGain:
        {
            c->outputGain = LEAF_interpolateTableLookupFunction(input, c->gainAmpTable);
            break;
        }
        case SoftClipMix:
            c->mix = input;
            break;
        default:
            break;
    }
}


void tSoftClipModule_initToPool(void** const c, float* const params, float id, tMempool** const mempool)
{
    tMempool* m = *mempool;


	_tSoftClipModule* SoftClipModule = (_tSoftClipModule*) (*c = (_tSoftClipModule*) mpool_alloc (sizeof (_tSoftClipModule), m));
#ifndef __cplusplus
	memcpy(SoftClipModule->header.params, params, SoftClipNumParams*sizeof(float));
#endif

    SoftClipModule->mempool = m;
    tHighpass_init   (SoftClipModule->mempool->leaf, &SoftClipModule->highpass, 20.0f);
    SoftClipModule->mempool = m;
    SoftClipModule->inputGain = 1.0f;
    SoftClipModule->outputGain = 1.0f;
    SoftClipModule->offset = 0.0f;
    SoftClipModule->shapeDivider = 1.0f;

    SoftClipModule->header.uniqueID = id;

    if (SoftClipModule->mempool->leaf->gainAmpTable == NULL)
    {
   	 tLookupTable_create(&SoftClipModule->mempool, &SoftClipModule->mempool->leaf->gainAmpTable);
   	 tLookupTable_init (SoftClipModule->mempool->leaf, SoftClipModule->mempool->leaf->gainAmpTable, 0.f, TWELVE_DB_AMPLITUDE, 1.f, 2048);
    }
    SoftClipModule->gainAmpTable = SoftClipModule->mempool->leaf->gainAmpTable;
#ifndef __cplusplus
    for (int i = 0; i < SoftClipNumParams; i++)
    {
    	tSoftClipModule_setParameter(SoftClipModule, i, SoftClipModule->header.params[i]);

    }
#endif

    SoftClipModule->header.moduleType = ModuleTypeSoftClipModule;
}

