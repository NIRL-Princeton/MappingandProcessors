//
// Created by Matthew McWeeney on 8/3/26.
//

#include "SineModule.h"
//#include <iostream>
#include "defs.h"

#include <assert.h>
#include "string.h"

void tSineModule_init(void** const osc, float* params, float id, LEAF* const leaf)
{
    tSineModule_initToPool(osc, params, id, &leaf->mempool);
}

void tSineModule_setParameter(tSineModule osc, SineParams param_type, float input)
{
	switch (param_type) {
	    case SineEventWatchFlag:
		    break;
	    case SineHarmonic:
	        //tOscModule_setInputNote (osc, input * 127.f);
	        input = (input - .5f) * 30.f;
	        input = roundf(input);
	        if (input >= 0.0f) {
	            osc->harmonicMultiplier = (input + 1.0f);
	        } else {
	            osc->harmonicMultiplier = (1.0f / fabsf((input - 1.0f)));
	        }
	        break;
	    case SinePitchFine:
	        //tOscModule_setInputNote (osc, input * 127.f);
	        osc->pitchFine = input * 2.0f - 1.0f;
	        break;
	    case SinePitch:
	        //tOscModule_setInputNote (osc, input * 127.f);
	        osc->note = (input * 127.0f) + osc->pitchFine;
	        //input = osc->mtofTable->table[(int)(input*16383)];
	        input = (LEAF_interpolateTableLookupFunction(osc->note * INV_LEAF_MODULE_MAX_MIDI_NOTE, osc->mtofTable));
	        tCycle_setFreq(&osc->theSine, input * osc->harmonicMultiplier);
	        break;
	    case SineAmp:
	        osc->amp = input;
	        break;
	    case SineGain:
	    	osc->gain = LEAF_interpolateTableLookupFunction(input, osc->gainAmpTable);
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
     memcpy(SineModule->header.params, param, SineNumParams*sizeof(float));
#endif __cplusplus
    SineModule->header.uniqueID = id;

    SineModule->mempool = m;
    SineModule->invSr = m->leaf->invSampleRate;
    SineModule->sr = m->leaf->sampleRate;

    //tRamp_init(SineModule->mempool->leaf, (tRamp*)&SineModule->pitchSmooth, 1.0f, 1);

    //tCycle_create (mempool, &SineModule->theSine);
    tCycle_init   (SineModule->mempool->leaf, &SineModule->theSine);

    if (SineModule->mempool->leaf->mtofTable == NULL)
    {
   	 tLookupTable_create(&SineModule->mempool, &SineModule->mempool->leaf->mtofTable);
   	 tLookupTable_init (SineModule->mempool->leaf, SineModule->mempool->leaf->mtofTable, 0.f, 0.f, 0.f, 16384);
   	 LEAF_generate_mtof (SineModule->mempool->leaf->mtofTable->table, 0, LEAF_MODULE_MAX_MIDI_NOTE, 16384);
    }
    SineModule->mtofTable = SineModule->mempool->leaf->mtofTable;

    if (SineModule->mempool->leaf->gainAmpTable == NULL)
    {
   	 tLookupTable_create(&SineModule->mempool, &SineModule->mempool->leaf->gainAmpTable);
 	 tLookupTable_init (SineModule->mempool->leaf, SineModule->mempool->leaf->gainAmpTable, 0.f, TWELVE_DB_AMPLITUDE, 1.f, 2048);
    }

    SineModule->gainAmpTable = SineModule->mempool->leaf->gainAmpTable;


    SineModule->header.moduleType = ModuleTypeSineModule;
#ifndef __cplusplus
    for (int i = 0; i < SineNumParams; i++)
    {
    	tSineModule_setParameter(SineModule, i, SineModule->header.params[i]);
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
void tSineModule_tick (tSineModule const osc)
{

    //const float input = noise->header.summedInput + buffer[0];
    osc->header.inputs[0] = 0.0f;
    //tSineModule_setParameter(osc, SineFreqParam, MIDIpitch)
    //osc->amp = tSlopeRamp_tick(&osc->ampSmoother);

    //printf("%f",osc->amp);
    osc->header.outputs[0] = tCycle_tick(&osc->theSine)* osc->amp * osc->gain;
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
