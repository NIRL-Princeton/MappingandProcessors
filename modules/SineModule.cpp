//
// Created by Matthew McWeeney on 8/3/26.
//

#include "SineModule.h"
#include <iostream>
#include "defs.h"

#include <assert.h>
void tSineModule_init(void** const osc, float* params, float id, LEAF* const leaf)
{
    tSineModule_initToPool(osc, params, id, &leaf->mempool);
}

void tSineModule_setParameter(tSineModule osc, SineParams param_type, float input)
{
	switch (param_type) {
	    case SineEventWatchFlag:
		    break;
	    case SinePitch:
	        //tOscModule_setInputNote (osc, input * 127.f);
	        osc->note = input * 127;
	        input = osc->mtofTable->table[(int)(input*16383)];
	        tCycle_setFreq(&osc->theSine, input);
		    break;
	    case SineGain:
	        tSlopeRamp_setDest(&osc->ampSmoother, input);
		    break;
	    default:
		    break;
	}
}

void tSineModule_initToPool(void** const osc, float* const param, float id, tMempool** const mempool)
{
    tMempool* m = *mempool;
    _tSineModule* SineModule = (_tSineModule*) (*osc = (_tSineModule*) mpool_alloc (sizeof (_tSineModule), m));
#ifndef __cplusplus
    memcpy(SineModule->params, param, SineNumParams*sizeof(float));
    int type = roundf(CPPDEREF SineModule->params[SineType]);
#endif __cplusplus
    SineModule->header.uniqueID = id;

    SineModule->mempool = m;
    SineModule->invSr = m->leaf->invSampleRate;
    SineModule->sr = m->leaf->sampleRate;

    //tRamp_init(SineModule->mempool->leaf, (tRamp*)&SineModule->pitchSmooth, 1.0f, 1);

    tSlopeRamp_init(SineModule->mempool->leaf, &SineModule->ampSmoother, SMOOTH_SLOPE_MULTIPLIER * TEN_DB_AMPLITUDE, 1.f);

    //tCycle_create (mempool, &SineModule->theSine);
    tCycle_init   (SineModule->mempool->leaf, &SineModule->theSine);

    tLookupTable_create(&SineModule->mempool, &SineModule->mtofTable);
    tLookupTable_init (SineModule->mempool->leaf, SineModule->mtofTable, 0.f, 0.f, 0.f, 16384);
    LEAF_generate_mtof (SineModule->mtofTable->table, 0, 127, 16384);

    tLookupTable_create(&SineModule->mempool, &SineModule->gainAmpTable);
    tLookupTable_init (SineModule->mempool->leaf, SineModule->gainAmpTable, 0.f, TWELVE_DB_AMPLITUDE, 1.f, 2048);

    SineModule->header.moduleType = ModuleTypeSineModule;
#ifndef __cplusplus
    for (int i = 0; i < SineNumParams; i++)
    {
    	tSineModule_setParameter(SineModule, i, SineModule->params[i]);

    }
#endif

}


void tSineModule_free(void** const osc)
{
    _tSineModule* SineModule = (_tSineModule*) (*osc);
    //tCycle_free(&SineModule->theSine);
    mpool_free((char*)SineModule, SineModule->mempool);
}

// void tOscModule_setInputNote (tSineModule const osc, float inputNote)
// {
//     if (osc->inputNote != inputNote)
//     {
//         osc->inputNote = inputNote;
//         float freqToSmooth = (osc->inputNote);
//         tRamp_setDest(&osc->pitchSmooth, freqToSmooth);
//         //printf("hello\n");
//     }
// }

// tick function
void tSineModule_tick (tSineModule const osc,float* buffer)
{
    osc->amp = osc->gainAmpTable->table[(int)(tSlopeRamp_tick(&osc->ampSmoother)*2047.f)];

    *buffer = tCycle_tick(&osc->theSine)* osc->amp;


    //printf("%f",osc->amp);
    osc->header.outputs[0] = *buffer;
}

// void tOscModule_setGlideOrigin(tSineModule const osc, float originNote)
// {
//     tRamp_setVal(&osc->pitchSmooth, originNote);
// }

// Non-modulatable setters
// void tOscModule_setMTOFTableLocation (tSineModule const osc, float* const tableAddress)
// {
//     osc->mtofTable = tableAddress;
// }

void tSineModule_setSampleRate (tSineModule const osc, float sr)
{
    osc->sr = sr;
    osc->invSr = 1.0f / sr;
    tCycle_setSampleRate(&osc->theSine, sr);
}