//
// Created by Jeffrey Snyder on 7/31/24.
//
//
// Created by Joshua Warner on 6/13/24.
//

#include "FilterModule.h"
#include <cstdio>


#include <assert.h>

void tFiltModule_init(void** const filt, float* params, float id, LEAF* const leaf) {
	if(leaf->resTable == NULL)
		{
		tLookupTable_create(&leaf->mempool,&leaf->resTable);
		tLookupTable_init(leaf, leaf->resTable,  0.0f, 10.0f, 0.5f, 2048);
	}
    tFiltModule_initToPool(filt, params, id, &leaf->mempool, leaf->resTable);
}

float dbToATableLookupFunction(float const in, float const sizeMinusOne, float* const tableAddress)
{
    uint32_t inDBIndex = (uint32_t) in;
    uint32_t inDBIndexPlusOne = inDBIndex + 1;
    if (inDBIndexPlusOne > sizeMinusOne)
    {
        inDBIndexPlusOne = sizeMinusOne;
    }
    float alpha = in - (float)inDBIndex;
    return ((tableAddress[inDBIndex] * (1.0f - alpha)) + (tableAddress[inDBIndexPlusOne] * alpha));
}


float resTableLookupFunction (float input, float* resTableAddress, float resTableSizeMinusOne)
{
	input *= resTableSizeMinusOne;
	int inputInt = (int)input;
	float inputFloat = input - (float)inputInt;
	int nextPos = LEAF_clip(0.0f, inputInt + 1.0f, resTableSizeMinusOne);
	return LEAF_clip(0.1f, (resTableAddress[inputInt] * (1.0f - inputFloat)) + (resTableAddress[nextPos] * inputFloat), 10.0f);
}


void tFiltModule_setGain(tFiltModule const filt, float const gain)
{
    // I think there should be two separate gain parameters. One master, and one for the filters with internal gain parameters -Matt
    filt->gain = gain; // Also unsure if it's a good thing to have the master gain and internal gain to be set simultaneously -Matt
    const float eqGain = powf(10.0f, ((gain * 50.0f) - 25.0f) / 20.0f);
	switch (filt->filtType)
	{
	    case FiltTypePeak:
	        tVZFilterBell_setGain(&filt->bellFilter, eqGain);
	        break;
	    case FiltTypeHighShelf:
	        tVZFilterHS_setGain(&filt->highShelfFilter, eqGain);
	        break;
	    case FiltTypeLowShelf:
	        tVZFilterLS_setGain(&filt->lowShelfFilter, eqGain);
	        break;
	    default:
	        break;
	}

 //    filt->amp = gain;
 //    //float floatIndex = LEAF_clip (0, ((gain * 24.0f) - 12.0f * filt->dbTableScalar) -  filt->dbTableOffset, filt->dbTableSizeMinusOne);
	// switch(filt->filtType)
	//  {
	//  case FiltTypePeak:
	// 	 //floatIndex = LEAF_clip (0, ((gain * 50.f) - 25.f * filt->dbTableScalar) -  filt->dbTableOffset, filt->dbTableSizeMinusOne);
	// 	 tVZFilterBell_setGain((tVZFilterBell*)filt->theFilt, gain);
	// 	 break;
	//  case FiltTypeHighShelf:
	// 	 //floatIndex = LEAF_clip (0, ((gain * 50.f) - 25.f  * filt->dbTableScalar) -  filt->dbTableOffset, filt->dbTableSizeMinusOne);
	// 	 tVZFilterHS_setGain((tVZFilterHS*)filt->theFilt, gain);
	// 	 break;
	//  case FiltTypeLowShelf:
	// 	 //floatIndex = LEAF_clip (0, ((gain * 50.f) - 25.f  * filt->dbTableScalar) -  filt->dbTableOffset, filt->dbTableSizeMinusOne);
	// 	 tVZFilterLS_setGain((tVZFilterLS*)filt->theFilt, gain);
	// 	 break;
	//  default:
	// 	 break;
	//  }
}

void tFiltModule_setRes(tFiltModule const filt, float const res)
{
	// const float normalizedResonance = LEAF_clip(0.0f, res, 1.0f);
	// filt->resonanceKnob = normalizedResonance;
	// const float q = resTableLookupFunction(normalizedResonance, filt->resTable->table,
	//                                        filt->resTable->tableSize - 1);
	// tSVF_setQ((tSVF*)filt->filters[FiltTypeLowpass], q);
	// tSVF_setQ((tSVF*)filt->filters[FiltTypeHighpass], q);
	// tSVF_setQ((tSVF*)filt->filters[FiltTypeBandpass], q);
	// // The nonlinear ladder APIs do not use the same Q scale as the SVF/EQ
	// // implementations. Feeding them the 0.1..10 lookup result drives both into
	// // sustained self-oscillation at the normal parameter maximum.
	// tDiodeFilter_setQ((tDiodeFilter*)filt->filters[FiltTypeDiodeLowpass],
	//                   0.25f + (normalizedResonance * 3.25f)); // internal r: 0.5..7.0
	// tVZFilterBell_setBandwidth((tVZFilterBell*)filt->filters[FiltTypePeak], q);
	// tVZFilterHS_setResonance((tVZFilterHS*)filt->filters[FiltTypeHighShelf], q);
	// tVZFilterLS_setResonance((tVZFilterLS*)filt->filters[FiltTypeLowShelf], q);
	// tVZFilterBR_setResonance((tVZFilterBR*)filt->filters[FiltTypeNotch], q);
	// tLadderFilter_setQ((tLadderFilter*)filt->filters[FiltTypeLadderLowpass],
	//                    0.2f + (normalizedResonance * 3.6f)); // feedback below 4.0

    const float normalizedResonance = LEAF_clip(0.0f, res, 1.0f);
    filt->resonanceKnob = normalizedResonance;
    const float q = resTableLookupFunction(normalizedResonance, filt->resTable->table,
                                           filt->resTable->tableSize - 1);

    switch (filt->filtType)
    {
        case FiltTypeLowpass:
            tSVF_setQ(&filt->lowPassFilter, q);
            break;
        case FiltTypeHighpass:
            tSVF_setQ(&filt->highPassFilter, q);
            break;
        case FiltTypeBandpass:
            tSVF_setQ(&filt->bandPassFilter, q);
            break;
        case FiltTypeDiodeLowpass:
            // The nonlinear ladder APIs do not use the same Q scale as the SVF/EQ
            // implementations. Feeding them the 0.1..10 lookup result drives both into
            // sustained self-oscillation at the normal parameter maximum.
            tDiodeFilter_setQ(&filt->diodeFilter, 0.25f + (normalizedResonance * 3.25f)); // internal r: 0.5..7.0
            // making sure it doesn't self-oscillate makes sense, but pushing the diode filter to its limits sounds very cool.
            // Also I never got it to self-oscillate in my old version of FilterModule -Matt
            break;
        case FiltTypePeak:
            tVZFilterBell_setBandwidth(&filt->bellFilter, q * 20.f); // i feel like something else should be done here, or maybe nothing -Matt
            break;
        case FiltTypeHighShelf:
            tVZFilterHS_setResonance(&filt->highShelfFilter, q);
            break;
        case FiltTypeLowShelf:
            tVZFilterLS_setResonance(&filt->lowShelfFilter, q);
            break;
        case FiltTypeNotch:
            tVZFilterBR_setResonance(&filt->notchFilter, q);
            break;
        case FiltTypeLadderLowpass:
            tLadderFilter_setQ(&filt->ladderFilter, 0.2f + (normalizedResonance * 3.6f)); // feedback below 4.0
            break;
        default:
            break;
    }
}

static void tFiltModule_resetNonlinearState(tFiltModule const filt, uint32_t const type)
{
    switch (type)
    {
        case FiltTypeDiodeLowpass:
        {
            //auto* diode = (tDiodeFilter*)filt->filters[type];
            tDiodeFilter diode = filt->diodeFilter;
            diode.zi = 0.0f;
            diode.s0 = diode.s1 = diode.s2 = diode.s3 = 0.0f;
            break;
        }

        case FiltTypeLadderLowpass:
        {
            //auto* ladder = (tLadderFilter*)filt->filters[type];
            for (float& state : filt->ladderFilter.b)
                state = 0.0f;
            break;
        }
        default:
            break;
    }

  //   filt->qValue = res;
	 // switch(filt->filtType)
	 // {
	 // case FiltTypeLowpass:
		//  tSVF_setQ((tSVF*)filt->theFilt,resTableLookupFunction(res, filt->table->table, filt->table->tableSize-1));
		//  break;
	 // case FiltTypeHighpass:
		//  tSVF_setQ((tSVF*)filt->theFilt,resTableLookupFunction(res, filt->table->table, filt->table->tableSize-1));
		//  break;
	 // case FiltTypeBandpass:
		//  tSVF_setQ((tSVF*)filt->theFilt,resTableLookupFunction(res, filt->table->table, filt->table->tableSize-1));
		//  break;
	 // case FiltTypeDiodeLowpass:
		//  tDiodeFilter_setQ((tDiodeFilter*)filt->theFilt, resTableLookupFunction(res, filt->table->table, filt->table->tableSize-1));
		//  break;
	 // case FiltTypePeak:
		//  tVZFilterBell_setBandwidth((tVZFilterBell*)filt->theFilt, resTableLookupFunction(res*20.0f, filt->table->table, filt->table->tableSize-1));
		//  break;
	 // case FiltTypeHighShelf:
		//  tVZFilterHS_setResonance((tVZFilterHS*)filt->theFilt, resTableLookupFunction(res, filt->table->table, filt->table->tableSize-1));
		//  break;
	 // case FiltTypeLowShelf:
		//  tVZFilterLS_setResonance((tVZFilterLS*)filt->theFilt, resTableLookupFunction(res, filt->table->table, filt->table->tableSize-1));
		//  break;
	 // case FiltTypeNotch:
		//  tVZFilterBR_setResonance((tVZFilterBR*)filt->theFilt,resTableLookupFunction(res, filt->table->table, filt->table->tableSize-1));
		//  break;
	 // case FiltTypeLadderLowpass:
		//  tLadderFilter_setQ((tLadderFilter*)filt->theFilt, resTableLookupFunction(res, filt->table->table, filt->table->tableSize-1));
		//  break;
	 // default:
		//  break;
	 // }
}


void tFiltModule_initToPool(void** const filt, float* const params, float id, tMempool** const mempool, tLookupTable* resTable)
{
    tMempool* m = *mempool;
    _tFiltModule* FiltModule =(_tFiltModule *) ( *filt = (_tFiltModule*) mpool_alloc(sizeof(_tFiltModule), m));
#ifndef __cplusplus
    memcpy(FiltModule->params, params, FiltNumParams*sizeof(float));

#endif
    FiltModule->header.uniqueID = id;
    //CPPDEREF FiltModule->params[FiltAudioInput] = 0.0f;
    FiltModule->mempool = m;
    FiltModule->gain = 1.0f;
    FiltModule->gainKnob = 1.0f;
    FiltModule->resonanceKnob = 0.5f;
    FiltModule->cutoffKnob = 60.0f;
    FiltModule->keyFollow = 0.0f;
    FiltModule->inputNote = 0.0f;
    FiltModule->invSr = m->leaf->invSampleRate;
    FiltModule->sr = m->leaf->sampleRate;
	FiltModule->resTable = resTable;

    // for (int type = 0; type < FiltNumTypes; ++type)
    //     FiltModule->filters[type] = NULL;

    tSlopeRamp_init(m->leaf, &FiltModule->gainSmoother, SMOOTH_SLOPE_MULTIPLIER, 1.f);
    tSlopeRamp_init(m->leaf, &FiltModule->cutoffSmoother, SMOOTH_SLOPE_MULTIPLIER * 125.0f, 10000.0f);
    tSlopeRamp_init(m->leaf, &FiltModule->keyFollowSmoother, SMOOTH_SLOPE_MULTIPLIER, 0.f);
    tSlopeRamp_init(m->leaf, &FiltModule->qSmoother, SMOOTH_SLOPE_MULTIPLIER, 0.5f);
    tSlopeRamp_init(m->leaf, &FiltModule->mixSmoother, SMOOTH_SLOPE_MULTIPLIER, 1.0f);

    // tSVF_create(mempool, (tSVF**)&FiltModule->filters[FiltTypeLowpass]);
    // tSVF_init(m->leaf, (tSVF*)FiltModule->filters[FiltTypeLowpass], SVFTypeLowpass, 10000.0f, 0.5f);
    // tSVF_create(mempool, (tSVF**)&FiltModule->filters[FiltTypeHighpass]);
    // tSVF_init(m->leaf, (tSVF*)FiltModule->filters[FiltTypeHighpass], SVFTypeHighpass, 100.0f, 0.5f);
    // tSVF_create(mempool, (tSVF**)&FiltModule->filters[FiltTypeBandpass]);
    // tSVF_init(m->leaf, (tSVF*)FiltModule->filters[FiltTypeBandpass], SVFTypeBandpass, 100.0f, 0.5f);
    // tDiodeFilter_create(mempool, (tDiodeFilter**)&FiltModule->filters[FiltTypeDiodeLowpass]);
    // tDiodeFilter_init(m->leaf, (tDiodeFilter*)FiltModule->filters[FiltTypeDiodeLowpass], 10000.0f, 0.5f);
    // tFiltModule_resetNonlinearState(FiltModule, FiltTypeDiodeLowpass);
    // tVZFilterBell_create(mempool, (tVZFilterBell**)&FiltModule->filters[FiltTypePeak]);
    // tVZFilterBell_init(m->leaf, (tVZFilterBell*)FiltModule->filters[FiltTypePeak], 100.0f, 0.5f, 1.0f);
    // tVZFilterHS_create(mempool, (tVZFilterHS**)&FiltModule->filters[FiltTypeHighShelf]);
    // tVZFilterHS_init(m->leaf, (tVZFilterHS*)FiltModule->filters[FiltTypeHighShelf], 100.0f, 0.5f, 1.0f);
    // tVZFilterLS_create(mempool, (tVZFilterLS**)&FiltModule->filters[FiltTypeLowShelf]);
    // tVZFilterLS_init(m->leaf, (tVZFilterLS*)FiltModule->filters[FiltTypeLowShelf], 100.0f, 0.5f, 1.0f);
    // tVZFilterBR_create(mempool, (tVZFilterBR**)&FiltModule->filters[FiltTypeNotch]);
    // tVZFilterBR_init(m->leaf, (tVZFilterBR*)FiltModule->filters[FiltTypeNotch], 100.0f, 0.5f);
    // tLadderFilter_create(mempool, (tLadderFilter**)&FiltModule->filters[FiltTypeLadderLowpass]);
    // tLadderFilter_init(m->leaf, (tLadderFilter*)FiltModule->filters[FiltTypeLadderLowpass], 100.0f, 0.5f);
    // tFiltModule_resetNonlinearState(FiltModule, FiltTypeLadderLowpass);

    tSVF_init(m->leaf, &FiltModule->lowPassFilter, SVFTypeLowpass, 10000.0f, 0.5f);
    tSVF_init(m->leaf, &FiltModule->highPassFilter, SVFTypeHighpass, 100.0f, 0.5f);
    tSVF_init(m->leaf, &FiltModule->bandPassFilter, SVFTypeBandpass, 100.0f, 0.5f);
    tDiodeFilter_init(m->leaf, &FiltModule->diodeFilter, 10000.0f, 0.5f);
    tFiltModule_resetNonlinearState(FiltModule, FiltTypeDiodeLowpass);

    tVZFilterBell_init(m->leaf, &FiltModule->bellFilter, 100.0f, 0.5f, 1.0f);
    tVZFilterHS_init(m->leaf, &FiltModule->highShelfFilter, 100.0f, 0.5f, 1.0f);
    tVZFilterLS_init(m->leaf, &FiltModule->lowShelfFilter, 100.0f, 0.5f, 1.0f);
    tVZFilterBR_init(m->leaf, &FiltModule->notchFilter, 100.0f, 0.5f);
    tLadderFilter_init(m->leaf, &FiltModule->ladderFilter, 100.0f, 0.5f);
    tFiltModule_resetNonlinearState(FiltModule, FiltTypeLadderLowpass);

    tLookupTable_create(&FiltModule->mempool, &FiltModule->mtofTable);
    tLookupTable_init (FiltModule->mempool->leaf, FiltModule->mtofTable, 0.f, 0.f, 0.f, 16384);
    LEAF_generate_mtof (FiltModule->mtofTable->table, 0., 127, 16384);

    tLookupTable_create(&FiltModule->mempool, &FiltModule->skewFreqTable);
    tLookupTable_init (FiltModule->mempool->leaf, FiltModule->skewFreqTable, 0.1f, 20000.f, 1000.f, 16384);

    FiltModule->filtType = FiltTypeLowpass;

    FiltModule->header.moduleType = ModuleTypeFilterModule;

#ifndef __cplusplus
    for (int i = 0; i < FiltNumParams; i++)
    {
    	tFiltModule_setParameter(FiltModule, i, FiltModule->params[i]);

    }
#endif

}

void tFiltModule_free(void** const filt)
{
    _tFiltModule* FiltModule =(_tFiltModule*) *filt;

    // tSVF_free((tSVF**)&FiltModule->filters[FiltTypeLowpass]);
    // tSVF_free((tSVF**)&FiltModule->filters[FiltTypeHighpass]);
    // tSVF_free((tSVF**)&FiltModule->filters[FiltTypeBandpass]);
    // tDiodeFilter_free((tDiodeFilter**)&FiltModule->filters[FiltTypeDiodeLowpass]);
    // tVZFilterBell_free((tVZFilterBell**)&FiltModule->filters[FiltTypePeak]);
    // tVZFilterHS_free((tVZFilterHS**)&FiltModule->filters[FiltTypeHighShelf]);
    // tVZFilterLS_free((tVZFilterLS**)&FiltModule->filters[FiltTypeLowShelf]);
    // tVZFilterBR_free((tVZFilterBR**)&FiltModule->filters[FiltTypeNotch]);
    // tLadderFilter_free((tLadderFilter**)&FiltModule->filters[FiltTypeLadderLowpass]);

    mpool_free((char*)FiltModule, FiltModule->mempool);
}


// static float tFiltModule_tickType(tFiltModule const filt, uint32_t const type,
//                                   float const input)
// {
//     // the below should probably go down in the tick, for clarity
//     tFiltModule_setGain(filt, tSlopeRamp_tick(&filt->gainSmoother));
//     tFiltModule_setRes(filt, tSlopeRamp_tick(&filt->qSmoother));
//
//     filt->cutoffKnob = tSlopeRamp_tick(&filt->cutoffSmoother);
//     filt->keyFollow = tSlopeRamp_tick(&filt->keyFollowSmoother);
//     tFiltModule_setFreq(filt, filt->cutoffKnob + filt->keyFollow * filt->inputNote);
//     //printf("Freq: %f", filt->currFreq);
//
//     void* const filter = filt->filters[type];
//     switch (type)
//     {
//     case FiltTypeLowpass:
//     case FiltTypeHighpass:
//     case FiltTypeBandpass:
//         tSVF_setFreqFast((tSVF*)filter, filt->currFreq);
//         return tSVF_tick((tSVF*)filter, input) * filt->gain;
//     case FiltTypeDiodeLowpass:
//         tDiodeFilter_setFreqFast((tDiodeFilter*)filter, filt->currFreq);
//         return tDiodeFilter_tickEfficient((tDiodeFilter*)filter, input) * filt->gain;
//     case FiltTypePeak:
//         tVZFilterBell_setFreqFast((tVZFilterBell*)filter, filt->currFreq);
//         return tVZFilterBell_tick((tVZFilterBell*)filter, input);
//     case FiltTypeHighShelf:
//         tVZFilterHS_setFreqFast((tVZFilterHS*)filter, filt->currFreq);
//         return tVZFilterHS_tick((tVZFilterHS*)filter, input);
//     case FiltTypeLowShelf:
//         tVZFilterLS_setFreqFast((tVZFilterLS*)filter, filt->currFreq);
//         return tVZFilterLS_tick((tVZFilterLS*)filter, input);
//     case FiltTypeNotch:
//         tVZFilterBR_setFreqFast((tVZFilterBR*)filter, filt->currFreq);
//         return tVZFilterBR_tick((tVZFilterBR*)filter, input) * filt->gain;
//     case FiltTypeLadderLowpass:
//         tLadderFilter_setFreqFast((tLadderFilter*)filter, filt->currFreq);
//         // LEAF's ladder adds a fixed 0.015 bias internally. Cancel it here so a
//         // filter module cannot become an audio source when its input is silent.
//         return tLadderFilter_tick((tLadderFilter*)filter, input - 0.015f) * filt->gain;
//     default:
//         return input;
//     }
// }

// tick function
void tFiltModule_tick (tFiltModule const filt, float* buffer)
{
    tFiltModule_setMix(filt, tSlopeRamp_tick(&filt->mixSmoother));
    tFiltModule_setGain(filt, tSlopeRamp_tick(&filt->gainSmoother));
    tFiltModule_setRes(filt, tSlopeRamp_tick(&filt->qSmoother));

    filt->keyFollow = tSlopeRamp_tick(&filt->keyFollowSmoother);
    filt->cutoffKnob = tSlopeRamp_tick(&filt->cutoffSmoother);
    tFiltModule_setFreq(filt, filt->cutoffKnob + filt->keyFollow * filt->inputNote);
    //printf("Freq: %f", filt->currFreq);

    float input = buffer[0];
    float output;
    switch (filt->filtType)
    {
        case FiltTypeLowpass:
            tSVF_setFreq(&filt->lowPassFilter, filt->currFreq);
            output = tSVF_tick(&filt->lowPassFilter, input) * filt->gain;
            break;
        case FiltTypeHighpass:
            tSVF_setFreq(&filt->highPassFilter, filt->currFreq);
            output = tSVF_tick(&filt->highPassFilter, input) * filt->gain;
            break;
        case FiltTypeBandpass:
            tSVF_setFreq(&filt->bandPassFilter, filt->currFreq);
            output = tSVF_tick(&filt->bandPassFilter, input) * filt->gain;
            break;
        case FiltTypeDiodeLowpass:
            tDiodeFilter_setFreq(&filt->diodeFilter, filt->currFreq);
            output = tDiodeFilter_tickEfficient(&filt->diodeFilter, input) * filt->gain;
            break;
        case FiltTypePeak:
            tVZFilterBell_setFreq(&filt->bellFilter, filt->currFreq);
            output = tVZFilterBell_tick(&filt->bellFilter, input);
            break;
        case FiltTypeHighShelf:
            tVZFilterHS_setFreq(&filt->highShelfFilter, filt->currFreq);
            output = tVZFilterHS_tick(&filt->highShelfFilter, input);
            break;
        case FiltTypeLowShelf:
            tVZFilterLS_setFreq(&filt->lowShelfFilter, filt->currFreq);
            output = tVZFilterLS_tick(&filt->lowShelfFilter, input);
            break;
        case FiltTypeNotch:
            tVZFilterBR_setFreq(&filt->notchFilter, filt->currFreq);
            output = tVZFilterBR_tick(&filt->notchFilter, input) * filt->gain;
            break;
        case FiltTypeLadderLowpass:
            tLadderFilter_setFreq(&filt->ladderFilter, filt->currFreq);
            // LEAF's ladder adds a fixed 0.015 bias internally. Cancel it here so a
            // filter module cannot become an audio source when its input is silent.
            output = tLadderFilter_tick(&filt->ladderFilter, input - 0.015f) * filt->gain;
            break;
        default:
            output = input;
            break;
    }
    buffer[0] = filt->header.outputs[0] = input * (1.f - filt->mix) + filt->mix * output;
}

// Modulatable setters


void tFiltModule_setParameter(tFiltModule const filt, FiltParams param_type, float input)
{
	switch (param_type) {
	    case FiltEventWatchFlag:
	        break;
	    case FiltMidiPitch:
		    input = input * 127.f;
	        if (filt->inputNote != input)
		    {
                filt->inputNote = input;
	            tFiltModule_setFreq(filt, filt->mtofTable->table[(int)((input/127)*16383)]);
		    }
		    break;
	    case FiltCutoff:
	        input = input * 20000.f; // 135.5 to allow for about 21000hz
	        if (filt->cutoffKnob!= input)
	        {
	            filt->cutoffKnob = input;
	            //tSlopeRamp_setDest(&filt->freqSmoother, filt->inputNote * filt->keyFollow + filt->cutoffKnob);
	            tSlopeRamp_setDest(&filt->cutoffSmoother, filt->cutoffKnob);
	        }
		    break;
	    case FiltGain:
		    if (filt->gainKnob != input)
		    {
		        filt->gainKnob = input;
		        tSlopeRamp_setDest(&filt->gainSmoother, input);
		    }
		    break;
	    case FiltResonance:
		    if (filt->qSmoother.dest != input)
		    {
		        tSlopeRamp_setDest(&filt->qSmoother, input);
		    }
		    break;
	    case FiltKeyfollow:
		    if (filt->keyFollowSmoother.dest != input)
		    {
		        tSlopeRamp_setDest(&filt->keyFollowSmoother, input);
		    }
		    break;
	    case FiltType:
	        input = roundf(input * 8.f);
	        if (filt->filtType != (int)input)
	        {
	            tFiltModule_setType(filt, (int)input);
	        }
		    break;
	    case FiltMix:
	        if (filt->mixSmoother.dest != input)
	        {
	            filt->mixSmoother.dest = input;
	        }
	    default:
		    break;
	}
}

void tFiltModule_setFreq (tFiltModule const filt, float const freqInput)
{
    filt->currFreq = freqInput;
    // switch(filt->filtType)
    // {
    //     case FiltTypeLowpass:
    //         tSVF_setFreqFast((tSVF*)filt->theFilt, freqInput);
    //         break;
    //     case FiltTypeHighpass:
    //         tSVF_setFreqFast((tSVF*)filt->theFilt, freqInput);
    //         break;
    //     case FiltTypeBandpass:
    //         tSVF_setFreqFast((tSVF*)filt->theFilt, freqInput);
    //         break;
    //     case FiltTypeDiodeLowpass:
    //         tDiodeFilter_setFreqFast((tDiodeFilter*)filt->theFilt, freqInput);
    //         break;
    //     case FiltTypePeak:
    //         tVZFilterBell_setFreqFast((tVZFilterBell*)filt->theFilt, freqInput);
    //         break;
    //     case FiltTypeHighShelf:
    //         tVZFilterHS_setFreqFast((tVZFilterHS*)filt->theFilt, freqInput);
    //         break;
    //     case FiltTypeLowShelf:
    //         tVZFilterLS_setFreqFast((tVZFilterLS*)filt->theFilt, freqInput);
    //         break;
    //     case FiltTypeNotch:
    //         tVZFilterBR_setFreqFast((tVZFilterBR*)filt->theFilt, freqInput);
    //         break;
    //     case FiltTypeLadderLowpass:
    //         tLadderFilter_setFreqFast((tLadderFilter*)filt->theFilt, freqInput);
    //         break;
    //     default:
    //         break;
    // }
}

void tFiltModule_setMix (tFiltModule const filt, float mix)
{
    filt->mix = mix;
}

void tFiltModule_setType(tFiltModule const filt, int const input)
{
    filt->filtType = input;

    //tFiltModule_free(&filt->theFilt);
    //int type = FiltModule->filtType;
    // if (filt->filtType == FiltTypeLowpass) {
    //     tSVF_free((tSVF**)&filt->theFilt);
    // }
    // else if (filt->filtType == FiltTypeHighpass) {
    //     tSVF_free((tSVF**)&filt->theFilt);
    // }
    // else if (filt->filtType == FiltTypeBandpass) {
    //     tSVF_free((tSVF**)&filt->theFilt);
    // }
    // else if (filt->filtType == FiltTypeDiodeLowpass) {
    //     tDiodeFilter_free((tDiodeFilter**)&filt->theFilt);
    // }
    // else if (filt->filtType == FiltTypePeak) {
    //     tVZFilterBell_free((tVZFilterBell**)&filt->theFilt);
    // }
    // else if (filt->filtType == FiltTypeLowShelf) {
    //     tVZFilterLS_free((tVZFilterLS**)&filt->theFilt);
    // }
    // else if (filt->filtType == FiltTypeHighShelf) {
    //     tVZFilterHS_free((tVZFilterHS**)&filt->theFilt);
    // }
    // else if (filt->filtType == FiltTypeNotch) {
    //     tVZFilterBR_free((tVZFilterBR**)&filt->theFilt);
    // }
    // else if (filt->filtType == FiltTypeLadderLowpass) {
    //     tLadderFilter_free((tLadderFilter**)&filt->theFilt);
    // }

    // filt->filtType = input;
    // switch(filt->filtType)
    // {
    //     case FiltTypeLowpass:
    //         tSVF_create(&filt->mempool, (tSVF**)filt->theFilt);
    //         tSVF_init(filt->mempool->leaf, (tSVF*)filt->theFilt, SVFTypeLowpass,10000.0f, 0.5f);
    //         break;
    //
    //     case FiltTypeHighpass:
    //         tSVF_create(&filt->mempool, (tSVF**)filt->theFilt);
    //         tSVF_init(filt->mempool->leaf, (tSVF*)filt->theFilt, SVFTypeHighpass,100.0f, 0.5f);
    //         break;
    //
    //     case FiltTypeBandpass:
    //         tSVF_create(&filt->mempool, (tSVF**)filt->theFilt);
    //         tSVF_init(filt->mempool->leaf, (tSVF*)filt->theFilt, SVFTypeBandpass,100.0f, 0.5f);
    //         break;
    //
    //     case FiltTypeDiodeLowpass:
    //         tDiodeFilter_create (&filt->mempool, (tDiodeFilter**)filt->theFilt);
    //         tDiodeFilter_init   (filt->mempool->leaf, (tDiodeFilter*)filt->theFilt, 10000.0f, 0.5f);
    //         break;
    //
    //     case FiltTypePeak:
    //         tVZFilterBell_create (&filt->mempool, (tVZFilterBell**)filt->theFilt);
    //         tVZFilterBell_init   (filt->mempool->leaf, (tVZFilterBell*)filt->theFilt, 100.0f, 0.5f, 1.0f);
    //         break;
    //
    //     case FiltTypeHighShelf:
    //         tVZFilterHS_create (&filt->mempool, (tVZFilterHS**)filt->theFilt);
    //         tVZFilterHS_init   (filt->mempool->leaf, (tVZFilterHS*)filt->theFilt, 100.0f, 0.5f, 1.0f);
    //         break;
    //
    //     case FiltTypeLowShelf:
    //         tVZFilterLS_create (&filt->mempool, (tVZFilterLS**)filt->theFilt);
    //         tVZFilterLS_init   (filt->mempool->leaf, (tVZFilterLS*)filt->theFilt, 100.0f, 0.5f, 1.0f);
    //         break;
    //
    //     case FiltTypeNotch:
    //         tVZFilterBR_create (&filt->mempool, (tVZFilterBR**)filt->theFilt);
    //         tVZFilterBR_init   (filt->mempool->leaf, (tVZFilterBR*)filt->theFilt, 100.0f, 0.5f);
    //         break;
    //
    //     case FiltTypeLadderLowpass:
    //         tLadderFilter_create (&filt->mempool, (tLadderFilter**)filt->theFilt);
    //         tLadderFilter_init   (filt->mempool->leaf, (tLadderFilter*)filt->theFilt, 100.0f, 0.5f);
    //         break;
    //     default:
    //         break;
    // }
}

// Non-modulatable setters

void tFiltModule_setDBtoATableLocation (tFiltModule const filt, float* const tableAddress, uint32_t const tableSize)
{
    filt->dbTableAddress = tableAddress;
    filt->dbTableSizeMinusOne = (float)(tableSize - 1);
    filt->dbTableScalar = filt->dbTableSizeMinusOne/(4.0f-0.00001f);
    filt->dbTableOffset = 0.00001f * filt->dbTableScalar;
}



void tFiltModule_setSampleRate (tFiltModule const filt, float const sr)
{
    filt->sr = sr;
    filt->invSr = 1.0f / sr;
    tSVF_setSampleRate(&filt->lowPassFilter, sr);
    tSVF_setSampleRate(&filt->highPassFilter, sr);
    tSVF_setSampleRate(&filt->bandPassFilter, sr);
    tDiodeFilter_setSampleRate(&filt->diodeFilter, sr);
    tVZFilterBell_setSampleRate(&filt->bellFilter, sr);
    tVZFilterHS_setSampleRate(&filt->highShelfFilter, sr);
    tVZFilterLS_setSampleRate(&filt->lowShelfFilter, sr);
    tVZFilterBR_setSampleRate(&filt->notchFilter, sr);
    tLadderFilter_setSampleRate(&filt->ladderFilter, sr);
}
