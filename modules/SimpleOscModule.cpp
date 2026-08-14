//
// Created by Joshua Warner on 6/13/24.
//

#include "SimpleOscModule.h"
//#include "../../../source/synthesis/framework/utils.h"
#include "defs.h"
#include <iostream>

#include <assert.h>
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
	        tOscModule_setInputNote (osc, 127*input);
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
	        if (osc->pitchOffset != input)
	        {
	            if (osc->pStepped) {
	                input = roundf(input);
	            }
	            osc->pitchOffset = input;
	        }
	        break;
	    }
	    case OscPitchFine:
	    {
	        input = (input - 0.5f) * 2.f;
	        osc->fine = input;
	        //printf("FINE\n");
	        break;
	    }
	    case OscFreqOffset:
	    {
	        input = (input * 4000.0f) - 2000.f;
	        osc->freqOffset = input;
	        //printf("OFFSET\n");
	        break;
	    }
	    case OscShapeParam:
	    {
	        if (osc->shapeSmoother.dest != input)
	        {
	            tSlopeRamp_setDest(&osc->shapeSmoother, input);
	        }
	        break;
	    }
	    case OscAmpParam:
	    {
	        tSlopeRamp_setDest(&osc->ampSmoother, input * TEN_DB_AMPLITUDE);
	        break;
	    }
	    case OscGlide:
	    {
	        if (osc->inputGlideTime != input)
	        {
	            osc->inputGlideTime = input;
	            input = roundf(input * 2047);
	            input = osc->glideTimeTable->table[(uint16_t)input];
	            tRamp_setTime(&osc->pitchSmooth, input);
	            //printf("glideTime: %f\n", input);
	        }
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
    memcpy(OscModule->params, param, OscNumParams*sizeof(float));
    int type = roundf(CPPDEREF OscModule->params[OscType]);
#endif __cplusplus
    OscModule->header.uniqueID = id;

    OscModule->oscType = OscTypeSawSquare;
    OscModule->harmonicMultiplier = 0;
    OscModule->oscShape = 0;
    OscModule->fine = 0;
    OscModule->freqOffset = 0;
    OscModule->inputGlideTime = 0;
    OscModule->amp = 1.f;
    OscModule->pitchOffset = 0;

    OscModule->mempool = m;
    OscModule->invSr = m->leaf->invSampleRate;
    OscModule->sr = m->leaf->sampleRate;
    OscModule->counter = 0;

    //tRamp_create(mempool, &OscModule->pitchSmooth);
    tRamp_init(OscModule->mempool->leaf, &OscModule->pitchSmooth, 1.0f, 1);
    //tSlopeRamp_create(mempool, &OscModule->ampSmoother);
    tSlopeRamp_init(OscModule->mempool->leaf, &OscModule->ampSmoother, SMOOTH_SLOPE_MULTIPLIER * TEN_DB_AMPLITUDE, 1.f);
    //tSlopeRamp_create(mempool, &OscModule->shapeSmoother);
    tSlopeRamp_init(OscModule->mempool->leaf, &OscModule->shapeSmoother, SMOOTH_SLOPE_MULTIPLIER, 0.f);

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

    // if (dbtoaTable == NULL)
    // {
    //     tLookupTable_create(&OscModule->mempool, &dbtoaTable);
    //     tLookupTable_init (OscModule->mempool->leaf, dbtoaTable, 0.f, 0.f, 0.f, 2048);
    //     LEAF_generate_dbtoa (dbtoaTable->table, 2048, -80.f, 9.542f);
    // }

    // if (atodbTable == NULL)
    // {
    //     tLookupTable_create(&OscModule->mempool, &atodbTable);
    //     tLookupTable_init (OscModule->mempool->leaf, atodbTable, 0.f, 0.f, 0.f, 2048);
    //     LEAF_generate_atodb (atodbTable->table, 2048, 0.f, 3.f);
    // }

    // if (mtofTable == NULL)
    // {
    //     tLookupTable_create(&OscModule->mempool, &mtofTable);
    //     tLookupTable_init (OscModule->mempool->leaf, mtofTable, 0.f, 0.f, 0.f, 32768);
    //     LEAF_generate_mtof (mtofTable->table, -160, 135.5, 32768);
    // }
    //
    // if (ftomTable == NULL)
    // {
    //     tLookupTable_create(&OscModule->mempool, &ftomTable);
    //     tLookupTable_init (OscModule->mempool->leaf, ftomTable, 0.f, 0.f, 0.f, 32768);
    //     LEAF_generate_ftom (ftomTable->table, 0.02535f, 20500.f, 32768);
    // }

    // if (glideTimeTable == NULL)
    // {
    //     tLookupTable_create(&OscModule->mempool, &glideTimeTable);
    //     tLookupTable_init (OscModule->mempool->leaf, glideTimeTable, 0.f, 8000.f, 500.f, 2048);
    // }

    tLookupTable_create(&OscModule->mempool, &OscModule->glideTimeTable);
    tLookupTable_init (OscModule->mempool->leaf, OscModule->glideTimeTable, 0.f, 8000.f, 500.f, 2048);

    tLookupTable_create(&OscModule->mempool, &OscModule->mtofTable);
    tLookupTable_init (OscModule->mempool->leaf, OscModule->mtofTable, 0.f, 0.f, 0.f, 16384);
    LEAF_generate_mtof (OscModule->mtofTable->table, 0, 127, 16384);

#ifndef __cplusplus
    for (int i = 0; i < OscNumParams; i++)
    {
    	tOscModule_setParameter(OscModule, i, OscModule->params[i]);

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

// tick function
void tOscModule_tick (tOscModule const osc, float* buffer)
{
    if (osc->oscShape != osc->shapeSmoother.dest)
    {
        //tOscModule_setShape(osc, tSlopeRamp_tick(&osc->shapeSmoother));
        osc->oscShape = tSlopeRamp_tick(&osc->shapeSmoother);
    }

    if (osc->amp != osc->ampSmoother.dest)
    {
        osc->amp = tSlopeRamp_tick(&osc->ampSmoother);
    }

    float tempMIDI;
    if (osc->pitchSmooth.curr != osc->inputNote)
    {
        tempMIDI = tRamp_tick(&osc->pitchSmooth) + osc->pitchOffset + osc->octaveOffset + osc->fine;
    } else
    {
        tempMIDI = osc->inputNote + osc->pitchOffset + osc->octaveOffset + osc->fine;
    }

	//    float tempIndexgit F = ((LEAF_clip(-163.0f, tempMIDI, 163.0f) * 100.0f) + 16384.0f);
	//    int tempIndexI = (int)tempIndexF;
	//    tempIndexF = tempIndexF -tempIndexI;
	//    float freqToSmooth1 = osc->mtofTable[tempIndexI & 32767];
	//    float freqToSmooth2 = osc->mtofTable[(tempIndexI + 1) & 32767];
	    //float nowFreq = tempMIDI;// ((freqToSmooth1 * (1.0f - tempIndexF)) + (freqToSmooth2 * tempIndexF));

    // float finalFreq = mtof(tempMIDI) * osc->harmonicMultiplier + osc->freqOffset;
    float finalFreq = osc->mtofTable->table[(int)((tempMIDI)/(127) * 16383)] * osc->harmonicMultiplier + osc->freqOffset;
    //printf("freq: %f\n", finalFreq);
	switch (osc->oscType) {
	    case OscTypeSawSquare: {
		    tPBSawSquare_setFreq(&osc->sawSquareOsc,finalFreq);
	        tPBSawSquare_setShape(&osc->sawSquareOsc,osc->oscShape);
		    *buffer = tPBSawSquare_tick(&osc->sawSquareOsc)* osc->amp;
		    break;
	    }
	    case OscTypeSineTri: {
		    tPBSineTriangle_setFreq(&osc->sineTriangleOsc,finalFreq);
	        tPBSineTriangle_setShape(&osc->sineTriangleOsc,osc->oscShape);
		    *buffer = tPBSineTriangle_tick(&osc->sineTriangleOsc)* osc->amp;
		    break;
	    }
	    case OscTypeSaw: {
		    tPBSaw_setFreq(&osc->sawOsc,finalFreq);
		    *buffer = tPBSaw_tick(&osc->sawOsc)* osc->amp;
		    break;
	    }
	    case OscTypePulse: {
		    tPBPulse_setFreq(&osc->squareOsc,finalFreq);
	        tPBPulse_setWidth(&osc->squareOsc,osc->oscShape);
		    *buffer = tPBPulse_tick(&osc->squareOsc)* osc->amp;
		    break;
	    }
	    case OscTypeSine: {
		    tCycle_setFreq(&osc->sineOsc,finalFreq);
		    *buffer = tCycle_tick(&osc->sineOsc)* osc->amp;
		    break;
	    }
	    case OscTypeTri: {
		    tPBTriangle_setFreq(&osc->triOsc,finalFreq);
	        tPBTriangle_setSkew(&osc->triOsc,osc->oscShape);
		    *buffer = tPBTriangle_tick(&osc->triOsc)* osc->amp;
		    break;
	    }
	}
    osc->header.outputs[0] = *buffer;
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

void tOscModule_onNoteOn (tOscModule const osc)
{
    if (osc->counter == 0)
    {
        osc->counter = 1; // counter is a band-aid fix for issue in SoundEngine, where noteOn is called for both the actual noteOn and noteOff events

        osc->ampSmoother.curr = osc->ampSmoother.dest;
        osc->amp = osc->ampSmoother.dest;

        osc->shapeSmoother.curr = osc->shapeSmoother.dest;
        osc->oscShape = osc->shapeSmoother.dest;
    } else
    {
        osc->counter = 0;
    }
}

