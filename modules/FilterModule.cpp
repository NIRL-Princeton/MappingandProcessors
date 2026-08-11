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
	filt->gainKnob = gain;
	filt->amp = gain;
	const float eqGain = powf(10.0f, ((gain * 50.0f) - 25.0f) / 20.0f);
	tVZFilterBell_setGain((tVZFilterBell*)filt->filters[FiltTypePeak], eqGain);
	tVZFilterHS_setGain((tVZFilterHS*)filt->filters[FiltTypeHighShelf], eqGain);
	tVZFilterLS_setGain((tVZFilterLS*)filt->filters[FiltTypeLowShelf], eqGain);
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
	const float normalizedResonance = LEAF_clip(0.0f, res, 1.0f);
	filt->resonanceKnob = normalizedResonance;
	const float q = resTableLookupFunction(normalizedResonance, filt->table->table,
	                                       filt->table->tableSize - 1);
	tSVF_setQ((tSVF*)filt->filters[FiltTypeLowpass], q);
	tSVF_setQ((tSVF*)filt->filters[FiltTypeHighpass], q);
	tSVF_setQ((tSVF*)filt->filters[FiltTypeBandpass], q);
	// The nonlinear ladder APIs do not use the same Q scale as the SVF/EQ
	// implementations. Feeding them the 0.1..10 lookup result drives both into
	// sustained self-oscillation at the normal parameter maximum.
	tDiodeFilter_setQ((tDiodeFilter*)filt->filters[FiltTypeDiodeLowpass],
	                  0.25f + (normalizedResonance * 3.25f)); // internal r: 0.5..7.0
	tVZFilterBell_setBandwidth((tVZFilterBell*)filt->filters[FiltTypePeak], q);
	tVZFilterHS_setResonance((tVZFilterHS*)filt->filters[FiltTypeHighShelf], q);
	tVZFilterLS_setResonance((tVZFilterLS*)filt->filters[FiltTypeLowShelf], q);
	tVZFilterBR_setResonance((tVZFilterBR*)filt->filters[FiltTypeNotch], q);
	tLadderFilter_setQ((tLadderFilter*)filt->filters[FiltTypeLadderLowpass],
	                   0.2f + (normalizedResonance * 3.6f)); // feedback below 4.0
}

static void tFiltModule_resetNonlinearState(tFiltModule const filt, uint32_t const type)
{
	if (type == FiltTypeDiodeLowpass)
	{
		auto* diode = (tDiodeFilter*)filt->filters[type];
		diode->zi = 0.0f;
		diode->s0 = diode->s1 = diode->s2 = diode->s3 = 0.0f;
	}
	else if (type == FiltTypeLadderLowpass)
	{
		auto* ladder = (tLadderFilter*)filt->filters[type];
		for (float& state : ladder->b)
			state = 0.0f;
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
    FiltModule->amp = 1.0f;
    FiltModule->gainKnob = 1.0f;
    FiltModule->resonanceKnob = 0.5f;
    FiltModule->cutoffKnob = 60.0f;
    FiltModule->keyFollow = 0.0f;
    FiltModule->inputNote = 0.0f;
    FiltModule->invSr = m->leaf->invSampleRate;
    FiltModule->sr = m->leaf->sampleRate;
	FiltModule->table = resTable;

    for (int type = 0; type < FiltNumTypes; ++type)
        FiltModule->filters[type] = NULL;

    tSlopeRamp_init(m->leaf, &FiltModule->ampSmoother, SMOOTH_SLOPE_MULTIPLIER, 1.f);
    tSlopeRamp_init(m->leaf, &FiltModule->cutoffSmoother, SMOOTH_SLOPE_MULTIPLIER * 125.0f, 10000.0f);
    tSlopeRamp_init(m->leaf, &FiltModule->keyFollowSmoother, SMOOTH_SLOPE_MULTIPLIER, 0.f);
    tSlopeRamp_init(m->leaf, &FiltModule->qSmoother, SMOOTH_SLOPE_MULTIPLIER, 0.5f);

    tSVF_create(mempool, (tSVF**)&FiltModule->filters[FiltTypeLowpass]);
    tSVF_init(m->leaf, (tSVF*)FiltModule->filters[FiltTypeLowpass], SVFTypeLowpass, 10000.0f, 0.5f);
    tSVF_create(mempool, (tSVF**)&FiltModule->filters[FiltTypeHighpass]);
    tSVF_init(m->leaf, (tSVF*)FiltModule->filters[FiltTypeHighpass], SVFTypeHighpass, 100.0f, 0.5f);
    tSVF_create(mempool, (tSVF**)&FiltModule->filters[FiltTypeBandpass]);
    tSVF_init(m->leaf, (tSVF*)FiltModule->filters[FiltTypeBandpass], SVFTypeBandpass, 100.0f, 0.5f);
    tDiodeFilter_create(mempool, (tDiodeFilter**)&FiltModule->filters[FiltTypeDiodeLowpass]);
    tDiodeFilter_init(m->leaf, (tDiodeFilter*)FiltModule->filters[FiltTypeDiodeLowpass], 10000.0f, 0.5f);
    tFiltModule_resetNonlinearState(FiltModule, FiltTypeDiodeLowpass);
    tVZFilterBell_create(mempool, (tVZFilterBell**)&FiltModule->filters[FiltTypePeak]);
    tVZFilterBell_init(m->leaf, (tVZFilterBell*)FiltModule->filters[FiltTypePeak], 100.0f, 0.5f, 1.0f);
    tVZFilterHS_create(mempool, (tVZFilterHS**)&FiltModule->filters[FiltTypeHighShelf]);
    tVZFilterHS_init(m->leaf, (tVZFilterHS*)FiltModule->filters[FiltTypeHighShelf], 100.0f, 0.5f, 1.0f);
    tVZFilterLS_create(mempool, (tVZFilterLS**)&FiltModule->filters[FiltTypeLowShelf]);
    tVZFilterLS_init(m->leaf, (tVZFilterLS*)FiltModule->filters[FiltTypeLowShelf], 100.0f, 0.5f, 1.0f);
    tVZFilterBR_create(mempool, (tVZFilterBR**)&FiltModule->filters[FiltTypeNotch]);
    tVZFilterBR_init(m->leaf, (tVZFilterBR*)FiltModule->filters[FiltTypeNotch], 100.0f, 0.5f);
    tLadderFilter_create(mempool, (tLadderFilter**)&FiltModule->filters[FiltTypeLadderLowpass]);
    tLadderFilter_init(m->leaf, (tLadderFilter*)FiltModule->filters[FiltTypeLadderLowpass], 100.0f, 0.5f);
    tFiltModule_resetNonlinearState(FiltModule, FiltTypeLadderLowpass);

    FiltModule->filtType = FiltTypeLowpass;
    //FiltModule->theFilt = FiltModule->filters[FiltTypeLowpass];


    // if (type == FiltTypeLowpass) {
    //     tSVF_create(mempool, (tSVF**)&FiltModule->theFilt);
    // 	tSVF_init(m->leaf,(tSVF*)FiltModule->theFilt, SVFTypeLowpass,10000.0f, 0.5f);
    // }
    // else if (type == FiltTypeHighpass)
    // {
    // 	tSVF_create (mempool, (tSVF**)&FiltModule->theFilt);
    // 	tSVF_init   (m->leaf, (tSVF*)FiltModule->theFilt,
				// 	 SVFTypeHighpass, 100.0f, 0.5f);
    // }
    // else if (type == FiltTypeBandpass)
    // {
    // 	tSVF_create (mempool, (tSVF**)&FiltModule->theFilt);
    // 	tSVF_init   (m->leaf, (tSVF*)FiltModule->theFilt,
				// 	 SVFTypeBandpass, 100.0f, 0.5f);
    // }
    // else if (type == FiltTypeDiodeLowpass)
    // {
    // 	tDiodeFilter_create (mempool, (tDiodeFilter**)&FiltModule->theFilt);
    // 	tDiodeFilter_init   (m->leaf, (tDiodeFilter*)FiltModule->theFilt,
				// 			 10000.0f, 0.5f);
    // }
    // else if (type == FiltTypePeak)
    // {
    // 	tVZFilterBell_create (mempool, (tVZFilterBell**)&FiltModule->theFilt);
    // 	tVZFilterBell_init   (m->leaf, (tVZFilterBell*)FiltModule->theFilt,
				// 			  100.0f, 0.5f, 1.0f);
    // }
    // else if (type == FiltTypeHighShelf)
    // {
    // 	tVZFilterHS_create (mempool, (tVZFilterHS**)&FiltModule->theFilt);
    // 	tVZFilterHS_init   (m->leaf, (tVZFilterHS*)FiltModule->theFilt,
				// 			100.0f, 0.5f, 1.0f);
    // }
    // else if (type == FiltTypeLowShelf)
    // {
    // 	tVZFilterLS_create (mempool, (tVZFilterLS**)&FiltModule->theFilt);
    // 	tVZFilterLS_init   (m->leaf, (tVZFilterLS*)FiltModule->theFilt,
				// 			100.0f, 0.5f, 1.0f);
    // }
    // else if (type == FiltTypeNotch)
    // {
    // 	tVZFilterBR_create (mempool, (tVZFilterBR**)&FiltModule->theFilt);
    // 	tVZFilterBR_init   (m->leaf, (tVZFilterBR*)FiltModule->theFilt,
				// 			100.0f, 0.5f);
    // }
    // else if (type == FiltTypeLadderLowpass)
    // {
    // 	tLadderFilter_create (mempool, (tLadderFilter**)&FiltModule->theFilt);
    // 	tLadderFilter_init   (m->leaf, (tLadderFilter*)FiltModule->theFilt,
				// 			  100.0f, 0.5f);
    // }
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

    tSVF_free((tSVF**)&FiltModule->filters[FiltTypeLowpass]);
    tSVF_free((tSVF**)&FiltModule->filters[FiltTypeHighpass]);
    tSVF_free((tSVF**)&FiltModule->filters[FiltTypeBandpass]);
    tDiodeFilter_free((tDiodeFilter**)&FiltModule->filters[FiltTypeDiodeLowpass]);
    tVZFilterBell_free((tVZFilterBell**)&FiltModule->filters[FiltTypePeak]);
    tVZFilterHS_free((tVZFilterHS**)&FiltModule->filters[FiltTypeHighShelf]);
    tVZFilterLS_free((tVZFilterLS**)&FiltModule->filters[FiltTypeLowShelf]);
    tVZFilterBR_free((tVZFilterBR**)&FiltModule->filters[FiltTypeNotch]);
    tLadderFilter_free((tLadderFilter**)&FiltModule->filters[FiltTypeLadderLowpass]);

    mpool_free((char*)FiltModule, FiltModule->mempool);
}


static float tFiltModule_tickType(tFiltModule const filt, uint32_t const type,
                                  float const input)
{
    // the below should probably go down in the tick, for clarity
    tFiltModule_setGain(filt, tSlopeRamp_tick(&filt->ampSmoother));
    tFiltModule_setRes(filt, tSlopeRamp_tick(&filt->qSmoother));

    filt->cutoffKnob = tSlopeRamp_tick(&filt->cutoffSmoother);
    filt->keyFollow = tSlopeRamp_tick(&filt->keyFollowSmoother);
    tFiltModule_setFreq(filt, filt->cutoffKnob + filt->keyFollow * filt->inputNote);
    //printf("Freq: %f", filt->currFreq);

    void* const filter = filt->filters[type];
    switch (type)
    {
    case FiltTypeLowpass:
    case FiltTypeHighpass:
    case FiltTypeBandpass:
        tSVF_setFreqFast((tSVF*)filter, filt->currFreq);
        return tSVF_tick((tSVF*)filter, input) * filt->amp;
    case FiltTypeDiodeLowpass:
        tDiodeFilter_setFreqFast((tDiodeFilter*)filter, filt->currFreq);
        return tDiodeFilter_tickEfficient((tDiodeFilter*)filter, input) * filt->amp;
    case FiltTypePeak:
        tVZFilterBell_setFreqFast((tVZFilterBell*)filter, filt->currFreq);
        return tVZFilterBell_tick((tVZFilterBell*)filter, input);
    case FiltTypeHighShelf:
        tVZFilterHS_setFreqFast((tVZFilterHS*)filter, filt->currFreq);
        return tVZFilterHS_tick((tVZFilterHS*)filter, input);
    case FiltTypeLowShelf:
        tVZFilterLS_setFreqFast((tVZFilterLS*)filter, filt->currFreq);
        return tVZFilterLS_tick((tVZFilterLS*)filter, input);
    case FiltTypeNotch:
        tVZFilterBR_setFreqFast((tVZFilterBR*)filter, filt->currFreq);
        return tVZFilterBR_tick((tVZFilterBR*)filter, input) * filt->amp;
    case FiltTypeLadderLowpass:
        tLadderFilter_setFreqFast((tLadderFilter*)filter, filt->currFreq);
        // LEAF's ladder adds a fixed 0.015 bias internally. Cancel it here so a
        // filter module cannot become an audio source when its input is silent.
        return tLadderFilter_tick((tLadderFilter*)filter, input - 0.015f) * filt->amp;
    default:
        return input;
    }
}

// tick function
void tFiltModule_tick (tFiltModule const filt, float* buffer)
{
    // Removing keyfollow since it is not set up to a parameter right now.
    //const float cutoff = filt->cutoffKnob + (filt->inputNote * filt->keyFollow);
    const float input = buffer[0];
    float output = tFiltModule_tickType(filt, filt->filtType, input);
    buffer[0] = filt->header.outputs[0] = output;
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
		    }
		    break;
	    case FiltCutoff:
	        input = input * 135.5f; // 135.5 to allow for about 21000hz
	        if (filt->cutoffKnob!= input)
	        {
	            filt->cutoffKnob = input;
	            //tSlopeRamp_setDest(&filt->freqSmoother, filt->inputNote * filt->keyFollow + filt->cutoffKnob);
	            tSlopeRamp_setDest(&filt->cutoffSmoother, filt->cutoffKnob);
	        }
		    break;
	    case FiltGain:
		    if (filt->ampSmoother.curr != input)
		    {
		        tSlopeRamp_setDest(&filt->ampSmoother, input);
		    }
		    break;
	    case FiltResonance:
		    if (filt->qSmoother.dest != input)
		    {
		        tSlopeRamp_setDest(&filt->qSmoother, input);
		    }
		    break;
	    case FiltKeyfollow:
		    if (filt->keyFollow != input)
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

void tFiltModule_setType(tFiltModule const filt, int const input)
{
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

    filt->filtType = input;
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
    tSVF_setSampleRate((tSVF*)filt->filters[FiltTypeLowpass], sr);
    tSVF_setSampleRate((tSVF*)filt->filters[FiltTypeHighpass], sr);
    tSVF_setSampleRate((tSVF*)filt->filters[FiltTypeBandpass], sr);
    tDiodeFilter_setSampleRate((tDiodeFilter*)filt->filters[FiltTypeDiodeLowpass], sr);
    tVZFilterBell_setSampleRate((tVZFilterBell*)filt->filters[FiltTypePeak], sr);
    tVZFilterHS_setSampleRate((tVZFilterHS*)filt->filters[FiltTypeHighShelf], sr);
    tVZFilterLS_setSampleRate((tVZFilterLS*)filt->filters[FiltTypeLowShelf], sr);
    tVZFilterBR_setSampleRate((tVZFilterBR*)filt->filters[FiltTypeNotch], sr);
    tLadderFilter_setSampleRate((tLadderFilter*)filt->filters[FiltTypeLadderLowpass], sr);
}
