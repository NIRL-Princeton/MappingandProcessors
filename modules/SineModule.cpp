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
	        tCycle_setFreq(osc->theSine, mtof(input * 127.f));
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

    tSlopeRamp_init(SineModule->mempool->leaf, (tSlopeRamp*)&SineModule->ampSmoother, SMOOTH_SLOPE_MULTIPLIER, 0.5f);

    tCycle_create (mempool, &SineModule->theSine);
    tCycle_init   (SineModule->mempool->leaf, SineModule->theSine);

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
    tCycle_free(&SineModule->theSine);
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
    osc->amp = tSlopeRamp_tick(&osc->ampSmoother);

    //float tempMIDI = tRamp_tick(&osc->pitchSmooth) + osc->pitchOffset + osc->octaveOffset + osc->fine;

    //tCycle_setFreq((tCycle*)osc->theOsc,osc->note);
    *buffer = tCycle_tick(osc->theSine)* osc->amp;

    //float finalFreq = mtof(tempMIDI) * osc->harmonicMultiplier + osc->freqOffset;
    //printf("%f",osc->amp);
    osc->header.outputs[0] = *buffer;
}

// void tOscModule_setGlideOrigin(tSineModule const osc, float originNote)
// {
//     tRamp_setVal(&osc->pitchSmooth, originNote);
// }

// Non-modulatable setters
void tOscModule_setMTOFTableLocation (tSineModule const osc, float* const tableAddress)
{
    osc->mtofTable = tableAddress;
}