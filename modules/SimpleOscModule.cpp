//
// Created by Joshua Warner on 6/13/24.
//

#include "SimpleOscModule.h"
#include <iostream>
#include "defs.h"

#include <assert.h>
void tOscModule_init(void** const osc, float* params, float id, LEAF* const leaf)
{
    tOscModule_initToPool(osc, params, id, &leaf->mempool);
}

void tOscModule_setType (tOscModule const osc, int type)
{
//destroy current oscillator object
    //int type = osc->osctype;
 //    switch(osc->osctype)
 //    {
	// 	case OscTypeSawSquare:
	// 		tPBSawSquare_free((tPBSawSquare**)&osc->theOsc);
	// 		break;
	// 	case OscTypeSineTri:
	// 		tPBSineTriangle_free((tPBSineTriangle**)&osc->theOsc);
	// 		break;
	// 	case OscTypeSaw:
	// 		tPBSaw_free((tPBSaw**)&osc->theOsc);
	// 		break;
	// 	case OscTypePulse:
	// 		tPBPulse_free((tPBPulse**)&osc->theOsc);
	// 		break;
	// 	case OscTypeSine:
	// 		//tCycle_free((tCycle**)&osc->theOsc);
 //            tPBSineTriangle_free((tPBSineTriangle**)&osc->theOsc);
	// 		break;
	// 	case OscTypeTri:
	// 		tPBTriangle_free((tPBTriangle**)&osc->theOsc);
	// 		break;
	// 	default:
	// 		break;
 //    }
 //
 //    //set new oscillator type
 //    //type = round(typefloat * (float)OscNumTypes);
 //    //create new oscillator object
	// switch (type)
	// {
	// 	case OscTypeSawSquare:
	// 		tPBSawSquare_create (&osc->mempool, (tPBSawSquare**)&osc->theOsc);
	// 		tPBSawSquare_init   (osc->mempool->leaf, (tPBSawSquare*)osc->theOsc);
	// 		break;
 //
	// 	case OscTypeSineTri:
	// 		tPBSineTriangle_create (&osc->mempool, (tPBSineTriangle**)&osc->theOsc);
	// 		tPBSineTriangle_init   (osc->mempool->leaf, (tPBSineTriangle*)osc->theOsc);
	// 		break;
 //
	// 	case OscTypeSaw:
	// 		tPBSaw_create (&osc->mempool, (tPBSaw**)&osc->theOsc);
	// 		tPBSaw_init   (osc->mempool->leaf, (tPBSaw*)osc->theOsc);
	// 		break;
 //
	// 	case OscTypePulse:
	// 		tPBPulse_create (&osc->mempool, (tPBPulse**)&osc->theOsc);
	// 		tPBPulse_init   (osc->mempool->leaf, (tPBPulse*)osc->theOsc);
	// 		break;
 //
	// 	case OscTypeSine:
	// 		// tCycle_create (&osc->mempool, (tCycle**)&osc->theOsc);
	// 		// tCycle_init   (osc->mempool->leaf, (tCycle*)osc->theOsc);
	//         tPBSineTriangle_create (&osc->mempool, (tPBSineTriangle**)&osc->theOsc);
	//         tPBSineTriangle_init   (osc->mempool->leaf, (tPBSineTriangle*)osc->theOsc);
	// 		break;
 //
	// 	case OscTypeTri:
	// 		tPBTriangle_create (&osc->mempool, (tPBTriangle**)&osc->theOsc);
	// 		tPBTriangle_init   (osc->mempool->leaf, (tPBTriangle*)osc->theOsc);
	// 		break;
 //
	// 	default:
	// 		break;
	// }
    osc->oscType = type;
}
void tOscModule_setParameter(tOscModule const osc, OscParams param_type,float input)
{
	float factor;
	switch (param_type) {
	    case OscEventWatchFlag:
		    break;
	    case OscMidiPitch:
	        tOscModule_setInputNote (osc, input * 127.f);
		    //osc->inputNote = input * 127.0f;
		    break;
	    case OscHarmonic:
		    input -= 0.5f;
		    input *= 2.f;
		    input *= 15.0f;
		    if (osc->hStepped) {
			    input = roundf(input);
		    }

		    if (input >= 0.0f) {
			    osc->harmonicMultiplier = (input + 1.0f);
		    } else {
			    osc->harmonicMultiplier = (1.0f / fabsf((input - 1.0f)));
		    }
		    break;
	    case OscPitchOffset:
		    input -= 0.5f;
		    input *= 24.0f;
		    if (osc->pStepped) {
			    input = roundf(input);
		    }
		    osc->pitchOffset = input;
		    break;
	    case OscPitchFine:
		    osc->fine = (input - 0.5f) * 2.f;
		    break;
	    case OscFreqOffset:
		    osc->freqOffset = (input * 4000.0f) - 2000.f;
		    break;
	    case OscShapeParam:
            //tRamp_setDest(&osc->shapeSmooth, input);
	        tSlopeRamp_setDest(&osc->shapeSmoother, input);
		    break;
	    case OscAmpParam:
		    //tRamp_setDest(&osc->ampSmooth, input);
	        tSlopeRamp_setDest(&osc->ampSmoother, input);
		    break;
	    case OscGlide:
	        tRamp_setTime(&osc->pitchSmooth, input);
		    break;
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
	        uint8_t inp = (uint8_t)(input * (OscNumTypes - 1));
	        if (inp != osc->oscType)
	        {
	            tOscModule_setType(osc, inp);
	        }
	        break;
	    }
        case OscPortaType:
	        osc->portaType = input;
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

    int type = OscTypeSawSquare;
    OscModule->oscType = OscTypeSawSquare;
    OscModule->mempool = m;
    OscModule->invSr = m->leaf->invSampleRate;
    OscModule->sr = m->leaf->sampleRate;

	//float val = 64.f;
	//float factor = 0.05f;
    //tRamp_create(mempool, (tRamp**)&OscModule->theOsc);
    //OscModule->pitchSmooth.curr = val;
    //OscModule->pitchSmooth.dest = val;
	//if (factor < 0.0f) factor = 0.0f;
	//if (factor > 1.0f) factor = 1.0f;
	//smooth->baseFactor = factor;
    //OscModule->pitchSmoother.factor = factor;
	//OscModule->pitchSmoother.oneminusfactor = 1.0f - factor;

    //tRamp_create(mempool, &OscModule->pitchSmooth);
    tRamp_init(OscModule->mempool->leaf, &OscModule->pitchSmooth, 1.0f, 1);
    //tSlopeRamp_create(mempool, &OscModule->ampSmoother);
    tSlopeRamp_init(OscModule->mempool->leaf, &OscModule->ampSmoother, SMOOTH_SLOPE_MULTIPLIER, 0.5f);
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

    // tPBSawSquare_free(&OscModule->sawSquareOsc);
    // tPBSineTriangle_free(&OscModule->sineTriangleOsc);
    // tPBSaw_free(&OscModule->sawOsc);
    // tPBPulse_free(&OscModule->squareOsc);
    // tCycle_free(&OscModule->sineOsc);
    // tPBTriangle_free(&OscModule->triOsc);

    // tRamp_free(&OscModule->pitchSmooth);
    // tSlopeRamp_free(&OscModule->ampSmoother);
    // tSlopeRamp_free(&OscModule->shapeSmoother);

    mpool_free((char*)OscModule, OscModule->mempool);
}

void tOscModule_setInputNote (tOscModule const osc, float inputNote)
{
    if (osc->inputNote != inputNote)
    {
        osc->inputNote = inputNote;
        float freqToSmooth = (osc->inputNote);
        tRamp_setDest(&osc->pitchSmooth, freqToSmooth);
        //printf("hello\n");
    }
}

// tick function
void tOscModule_tick (tOscModule const osc,float* buffer)
{
    //tOscModule_setShape(osc, tRamp_tick(&osc->shapeSmooth));
    tOscModule_setShape(osc, tSlopeRamp_tick(&osc->shapeSmoother));
    //osc->amp = tRamp_tick(&osc->ampSmooth);
    osc->amp = tSlopeRamp_tick(&osc->ampSmoother);
	//float freqToSmooth = (osc->inputNote + (osc->fine));
	    //tExpSmooth_setDest(&osc->pitchSmoother, mtof(freqToSmooth));
        //tRamp_setDest(&osc->pitchSmooth, freqToSmooth);

	    //float nowFreq =  mtof(ftom(tExpSmooth_tick(&osc->pitchSmoother)) + osc->pitchOffset + osc->octaveOffset);
    float tempMIDI = tRamp_tick(&osc->pitchSmooth) + osc->pitchOffset + osc->octaveOffset + osc->fine;
    //std::cout << freqToSmooth << std::endl;

	//    float tempIndexgit F = ((LEAF_clip(-163.0f, tempMIDI, 163.0f) * 100.0f) + 16384.0f);
	//    int tempIndexI = (int)tempIndexF;
	//    tempIndexF = tempIndexF -tempIndexI;
	//    float freqToSmooth1 = osc->mtofTable[tempIndexI & 32767];
	//    float freqToSmooth2 = osc->mtofTable[(tempIndexI + 1) & 32767];
	    //float nowFreq = tempMIDI;// ((freqToSmooth1 * (1.0f - tempIndexF)) + (freqToSmooth2 * tempIndexF));

    float finalFreq = mtof(tempMIDI) * osc->harmonicMultiplier + osc->freqOffset;
        //printf("HM: %f", osc->harmonicMultiplier);
	switch (osc->oscType) {
	    case OscTypeSawSquare: {
		    tPBSawSquare_setFreq(&osc->sawSquareOsc,finalFreq);
		    *buffer = tPBSawSquare_tick(&osc->sawSquareOsc)* osc->amp;
		    break;
	    }
	    case OscTypeSineTri: {
		    tPBSineTriangle_setFreq(&osc->sineTriangleOsc,finalFreq);
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
void tOscModule_setMTOFTableLocation (tOscModule const osc, float* const tableAddress)
{
    osc->mtofTable = tableAddress;
}

