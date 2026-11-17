//
// Created by Jeffrey Snyder on 7/31/24.
//
//
// Created by Joshua Warner on 6/13/24.
//

#include "VCAModule.h"

#include <assert.h>
#include "string.h"

void tVCAModule_init(void** const VCA, float* params, float id, LEAF* const leaf)
{
    tVCAModule_initToPool(VCA, params, id, &leaf->mempool);
}

void tVCAModule_blankFunction (tVCAModule const VCA, float freq)
{
    ;
}



void tVCAModule_initToPool(void** const VCA, float* const params, float id, tMempool** const mempool)
{
    tMempool* m = *mempool;
    _tVCAModule* VCAModule = (_tVCAModule*) ( *VCA = (_tVCAModule*) mpool_alloc(sizeof(_tVCAModule), m));
#ifndef __cplusplus
    for (int i = 0; i < VCANumParams; i++)
    {
        VCAModule->header.params[i] = params[i];
    }
   // memcpy(VCAModule->header.params, params, VCANumParams*sizeof(float));

#endif
    VCAModule->header.uniqueID = id;
    //CPPDEREF VCAModule->params[VCAAudioInput] = 0.0f;
    //int type = 0.0f;//roundf(VCAModule->params[VCAType]);
    VCAModule->mempool = m;
    VCAModule->amp = 1.0f;
    VCAModule->gain = 1.0f;
    VCAModule->audioInput = 0.0f;
    VCAModule->header.moduleType = ModuleTypeVCAModule;
    VCAModule->external_offset = 0.0f;

    if (m->leaf->gainAmpTable == NULL)
    {
        tLookupTable_create(&m->leaf->mempool, &m->leaf->gainAmpTable);
        tLookupTable_init (m->leaf, m->leaf->gainAmpTable, 0.f, TWELVE_DB_AMPLITUDE, 1.f, 2048);
    }
    VCAModule->gainAmpTable = m->leaf->gainAmpTable;
#ifndef __cplusplus
    for (int i = 0; i < VCANumParams; i++)
    {
        tVCAModule_setParameter(VCAModule, i, VCAModule->header.params[i]);
    }
    #endif
}


void tVCAModule_free(void** const VCA)
{
    _tVCAModule* VCAModule =(_tVCAModule*) *VCA;
    mpool_free((char*)VCAModule, VCAModule->mempool);
}


void tVCAModule_setParameter(tVCAModule const VCA, VCAParams param_type, float input)
{
    switch (param_type)
    {
        case VCAEventWatchFlag:
            break;
        case VCAGain: //exponential
            VCA->gain = LEAF_interpolateTableLookupFunction(input, VCA->gainAmpTable);
            break;
        case VCAAmp: //linear
            VCA->amp = input;
            break;
        case VCAOffset:
            VCA->external_offset = input;
            break;
        case VCAAudioInput:
            VCA->audioInput = input;
            break;
        case VCARouting:
            break;
        default:
            break;
    }

}

// tick function
void tVCAModule_tick (tVCAModule const VCA)
{

    //buffer[0] = VCA->header.outputs[0] = (*buffer   + VCA->external_input)*VCA->amp;
    const float input = VCA->header.inputs[0];
    VCA->header.inputs[0] = 0.0f;

    VCA->header.outputs[0] = (input  + VCA->external_offset + VCA->audioInput) * VCA->amp * VCA->gain;
}

// Modulatable setters

// Non-modulatable setters

// be sure to set the tables before initing the processor
