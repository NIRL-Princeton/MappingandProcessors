//
// Created by Joshua Warner on 6/13/24.
//

#include "SimpleOscModule.h"
//#include "../../../source/synthesis/framework/utils.h"
#include "defs.h"
//#include <iostream>

#include <assert.h>
#include "string.h"

void tOscModule_init(void** const osc, float* params, float id, LEAF* const leaf)
{
    tOscModule_initToPool(osc, params, id, &leaf->mempool);
}

void tOscModule_setType (tOscModule const osc, int type)
{
    osc->oscType = type;
}

void tOscModule_setParameter(tOscModule const osc, OscParams param_type,float input)
{
	switch (param_type) {
	    case OscEventWatchFlag:
		    break;
	    case OscMidiPitch:
	        tOscModule_setInputNote (osc, 127.0f * input);
	        //printf("note: %f\n", input);
		    break;
	    case OscHarmonic:
	    {
	        input = (input - .5f) * 30.f;
	        if (osc->hStepped) {
	            input = roundf(input);
	        }

	        if (input >= 0.0f) {
	            osc->harmonicMultiplier = (input + 1.0f);
	        } else {
	            osc->harmonicMultiplier = (1.0f / fabsf((input - 1.0f)));
	        }

	        //printf("Harm: %f\n", input);
	        break;
	    }
	    case OscPitchOffset:
	    {
	        input = (input - .5f) * 24.f;
	        if (osc->pStepped) {
	            input = roundf(input);
	        }
	        osc->pitchOffset = input;
	        break;
	    }
	    case OscPitchFine:
	    {
	        osc->fine = (input - 0.5f) * 2.f;
	        //printf("FINE\n");
	        break;
	    }
	    case OscFreqOffset:
	    {
	        osc->freqOffset = (input * 4000.f - 2000.f);
	        //printf("input: %f.  ", input);
	        //printf("Output: %f\n", osc->freqOffset);
	        break;
	    }
	    case OscShapeParam:
	    {
	        osc->oscShape = input;
	        tOscModule_setShape(osc, input);
	        //tSlopeRamp_setDest(&osc->shapeSmoother, input);
	        break;
	    }
	    case OscGainParam:
	    {
	    	tOscModule_setGain(osc, input);
	    	//tSlopeRamp_setDest(&osc->gainSmoother, input); // * TEN_DB_AMPLITUDE
	        break;
	    }
	    case OscGlide:
	    {
	        osc->inputGlideTime = input;

	        input = LEAF_interpolateTableLookupFunction(input, osc->glideTimeTable);
	        tRamp_setTime(&osc->pitchSmooth, input);
	        /*
	        input = roundf(input * 2047);
	        input = osc->glideTimeTable->table[(uint16_t)input];
	        tRamp_setTime(&osc->pitchSmooth, input);
	        */
	        //printf("glideTime: %f\n", input);
	        break;
	    }
	    case OscSteppedHarmonic:
		    osc->hStepped = roundf(input);
		    break;
	    case OscSteppedPitch:
		    osc->pStepped = roundf(input);
		    break;
	    case OscSyncMode:
	        osc->syncMode = roundf(input);
		    break;
	    case OscSyncIn:
		    break;
	    case OscType:
	    {
	        input = powf(2, (uint8_t)(5.4 * input));
	        if ((uint8_t)input != osc->oscType)
	        {
	            tOscModule_setType(osc, (uint8_t)input);
	        }
	        break;
	    }
        case OscPortaType:
	    {
	        if (osc->portaType != (uint8_t)input)
	        {
	            osc->portaType = (uint8_t)input;
	        }
	        break;
	    }
	    default:
		    break;
	}
}

void tOscModule_initToPool(void** const osc, float* const param, float id, tMempool** const mempool)
{
    tMempool* m = *mempool;
    _tOscModule* OscModule = (_tOscModule*) (*osc = (_tOscModule*) mpool_alloc (sizeof (_tOscModule), m));
#ifndef __cplusplus
    memcpy(OscModule->header.params, param, OscNumParams*sizeof(float));
#endif __cplusplus
    OscModule->header.uniqueID = id;

    OscModule->oscType = OscTypeSawSquare;
    OscModule->harmonicMultiplier = 0;
    OscModule->oscShape = 0;
    OscModule->fine = 0;
    OscModule->freqOffset = 0;
    OscModule->inputGlideTime = 0;
    OscModule->gain = 1.f;
    OscModule->pitchOffset = 0;

    OscModule->mempool = m;
    OscModule->invSr = m->leaf->invSampleRate;
    OscModule->sr = m->leaf->sampleRate;
    OscModule->counter = 0;

    //tRamp_create(mempool, &OscModule->pitchSmooth);
    tRamp_init(OscModule->mempool->leaf, &OscModule->pitchSmooth, 1.0f, 1);

    //tPBSawSquare_create (&OscModule->mempool, &OscModule->sawSquareOsc);
    tPBSawSquare_init   (OscModule->mempool->leaf, &OscModule->sawSquareOsc);
    //tPBSineTriangle_create (&OscModule->mempool, &OscModule->sineTriangleOsc);
    tPBSineTriangle_init   (OscModule->mempool->leaf, &OscModule->sineTriangleOsc);
    //tPBSaw_create (&OscModule->mempool, &OscModule->sawOsc);
    tPBSaw_init   (OscModule->mempool->leaf, &OscModule->sawOsc);
    //tPBPulse_create (&OscModule->mempool, &OscModule->squareOsc);
    tPBPulse_init   (OscModule->mempool->leaf, &OscModule->squareOsc);
    //tCycle_create (&OscModule->mempool, &OscModule->sineOsc);
    tCycle_init   (OscModule->mempool->leaf, &OscModule->sineOsc);
    //tPBTriangle_create (&OscModule->mempool, &OscModule->triOsc);
    tPBTriangle_init   (OscModule->mempool->leaf, &OscModule->triOsc);

    OscModule->header.moduleType = ModuleTypeOscModule;

     if (OscModule->mempool->leaf->glideTimeTable == NULL)
     {
         tLookupTable_create(&OscModule->mempool, &OscModule->mempool->leaf->glideTimeTable);
         tLookupTable_init (OscModule->mempool->leaf, OscModule->mempool->leaf->glideTimeTable, 0.f, 8000.f, 500.f, 2048);
     }
     OscModule->glideTimeTable = OscModule->mempool->leaf->glideTimeTable;

     if (OscModule->mempool->leaf->gainAmpTable == NULL)
     {
    	 tLookupTable_create(&OscModule->mempool, &OscModule->mempool->leaf->gainAmpTable);
    	 tLookupTable_init (OscModule->mempool->leaf, OscModule->mempool->leaf->gainAmpTable, 0.f, TWELVE_DB_AMPLITUDE, 1.f, 2048);
     }
     OscModule->gainAmpTable = OscModule->mempool->leaf->gainAmpTable;



     if (OscModule->mempool->leaf->mtofTable == NULL)
     {
    	 tLookupTable_create(&OscModule->mempool, &OscModule->mempool->leaf->mtofTable);
    	 tLookupTable_init (OscModule->mempool->leaf, OscModule->mempool->leaf->mtofTable, 0.f, 0.f, 0.f, 16384);
    	 LEAF_generate_mtof (OscModule->mempool->leaf->mtofTable->table, 0, LEAF_MODULE_MAX_MIDI_NOTE, 16384);
     }
     OscModule->mtofTable = OscModule->mempool->leaf->mtofTable;



#ifndef __cplusplus
    for (int i = 0; i < OscNumParams; i++)
    {
    	tOscModule_setParameter(OscModule, i, OscModule->header.params[i]);

    }
#endif

}

void tOscModule_free(void** const osc)
{
    _tOscModule* OscModule = (_tOscModule*) (*osc);
    mpool_free((char*)OscModule, OscModule->mempool);
}

void tOscModule_setInputNote (tOscModule const osc, float inputNote)
{
    osc->inputNote = inputNote;
    tRamp_setDest(&osc->pitchSmooth, osc->inputNote);
}

void tOscModule_setGain (tOscModule const osc, float inputGain)
{
    //osc->gain = osc->gainAmpTable->table[(int)(inputGain * 2047)];
	osc->gain = LEAF_interpolateTableLookupFunction(inputGain, osc->gainAmpTable);
    //printf("Set gain %f\n", osc->gain);
}

// tick function
void tOscModule_tick (tOscModule const osc)
{
    //const float input = noise->header.summedInput + buffer[0];
    osc->header.inputs[0] = 0.0f;

    /*
    if (osc->oscShape != osc->shapeSmoother.dest)
    {
        //tOscModule_setShape(osc, tSlopeRamp_tick(&osc->shapeSmoother));
        osc->oscShape = tSlopeRamp_tick(&osc->shapeSmoother);
    }
*/
    //tOscModule_setGain(osc, tSlopeRamp_tick(&osc->gainSmoother));

    float tempMIDI;

    tempMIDI = tRamp_tick(&osc->pitchSmooth) + osc->pitchOffset + osc->octaveOffset + osc->fine;


	//    float tempIndexgit F = ((LEAF_clip(-163.0f, tempMIDI, 163.0f) * 100.0f) + 16384.0f);
	//    int tempIndexI = (int)tempIndexF;
	//    tempIndexF = tempIndexF -tempIndexI;
	//    float freqToSmooth1 = osc->mtofTable[tempIndexI & 32767];
	//    float freqToSmooth2 = osc->mtofTable[(tempIndexI + 1) & 32767];
	    //float nowFreq = tempMIDI;// ((freqToSmooth1 * (1.0f - tempIndexF)) + (freqToSmooth2 * tempIndexF));

    // float finalFreq = mtof(tempMIDI) * osc->harmonicMultiplier + osc->freqOffset;
    //float finalFreq = LEAF_clip(0.f, osc->mtofTable->table[(int)LEAF_clip(0, ((tempMIDI)/(127) * 16383), 16383)] * osc->harmonicMultiplier + osc->freqOffset, 20500.f);
    float finalFreq = (LEAF_interpolateTableLookupFunction(tempMIDI * INV_LEAF_MODULE_MAX_MIDI_NOTE, osc->mtofTable) * osc->harmonicMultiplier + osc->freqOffset);
    //printf("freq: %f\n", finalFreq);
    float out = 0.0f;
	switch (osc->oscType) {
	    case OscTypeSawSquare: {
		    tPBSawSquare_setFreq(&osc->sawSquareOsc,finalFreq);
	        //tPBSawSquare_setShape(&osc->sawSquareOsc,osc->oscShape);
		    out = tPBSawSquare_tick(&osc->sawSquareOsc)* osc->gain;
		    break;
	    }
	    case OscTypeSineTri: {
		    tPBSineTriangle_setFreq(&osc->sineTriangleOsc,finalFreq);
	        //tPBSineTriangle_setShape(&osc->sineTriangleOsc,osc->oscShape);
		    out = tPBSineTriangle_tick(&osc->sineTriangleOsc)* osc->gain;
		    break;
	    }
	    case OscTypeSaw: {
		    tPBSaw_setFreq(&osc->sawOsc,finalFreq);
		    out = tPBSaw_tick(&osc->sawOsc)* osc->gain;
		    break;
	    }
	    case OscTypePulse: {
		    tPBPulse_setFreq(&osc->squareOsc,finalFreq);
	        //tPBPulse_setWidth(&osc->squareOsc,osc->oscShape);
		    out = tPBPulse_tick(&osc->squareOsc)* osc->gain;
		    break;
	    }
	    case OscTypeSine: {
		    tCycle_setFreq(&osc->sineOsc,finalFreq);
		    out = tCycle_tick(&osc->sineOsc)* osc->gain;
		    break;
	    }
	    case OscTypeTri: {
		    tPBTriangle_setFreq(&osc->triOsc,finalFreq);
	        //tPBTriangle_setSkew(&osc->triOsc,osc->oscShape);
		    out = tPBTriangle_tick(&osc->triOsc)* osc->gain;
		    break;
	    }
	    default:
	        break;
	}
    osc->header.outputs[0] = out;
}

void tOscModule_setShape(tOscModule const osc, float shape)
{
    switch (osc->oscType)
    {
        case OscTypeSawSquare:
            tPBSawSquare_setShape(&osc->sawSquareOsc,shape);
            break;
        case OscTypeSineTri:
            tPBSineTriangle_setShape(&osc->sineTriangleOsc,shape);
            break;
        case OscTypePulse:
            tPBPulse_setWidth(&osc->squareOsc,shape);
            break;
        case OscTypeTri:
            tPBTriangle_setSkew(&osc->triOsc,shape);
            break;
        default:
            break;
    }
}

void tOscModule_setGlideOrigin(tOscModule const osc, float originNote)
{
    //tExpSmooth_setVal(&osc->pitchSmoother, originNote);
    tRamp_setVal(&osc->pitchSmooth, originNote);
}

void tOscModule_setOctave (tOscModule const osc, float const oct)
{
	 osc->octaveOffset = (roundf(((oct - 0.5f) * 6.0f))) * 12.0f;
}

// Non-modulatable setters
// void tOscModule_setMTOFTableLocation (tOscModule const osc, float* const tableAddress)
// {
//     //osc->mtofTable = tableAddress;
// }

void tOscModule_onNoteOn (tOscModule const osc, float vel)
{
    //if (vel > 0.0001f)
    {
        //osc->gainSmoother.curr = osc->gainSmoother.dest;
        //osc->gain = osc->gainSmoother.dest;

        //osc->shapeSmoother.curr = osc->shapeSmoother.dest;
        //osc->oscShape = osc->shapeSmoother.dest;
    }
}

