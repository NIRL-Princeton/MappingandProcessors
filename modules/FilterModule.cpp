//
// Created by Jeffrey Snyder on 7/31/24.
//
//
// Created by Joshua Warner on 6/13/24.
//

#include "FilterModule.h"

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
    FiltModule->previousFiltType = FiltTypeLowpass;
    FiltModule->transitionSamplesRemaining = 0;
    FiltModule->transitionSamplesTotal = 0;
    FiltModule->theFilt = FiltModule->filters[FiltTypeLowpass];
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
                                  float const input, float const cutoff)
{
    void* const filter = filt->filters[type];
    switch (type)
    {
    case FiltTypeLowpass:
    case FiltTypeHighpass:
    case FiltTypeBandpass:
        tSVF_setFreqFast((tSVF*)filter, cutoff);
        return tSVF_tick((tSVF*)filter, input) * filt->amp;
    case FiltTypeDiodeLowpass:
        tDiodeFilter_setFreqFast((tDiodeFilter*)filter, cutoff);
        return tDiodeFilter_tickEfficient((tDiodeFilter*)filter, input) * filt->amp;
    case FiltTypePeak:
        tVZFilterBell_setFreqFast((tVZFilterBell*)filter, cutoff);
        return tVZFilterBell_tick((tVZFilterBell*)filter, input);
    case FiltTypeHighShelf:
        tVZFilterHS_setFreqFast((tVZFilterHS*)filter, cutoff);
        return tVZFilterHS_tick((tVZFilterHS*)filter, input);
    case FiltTypeLowShelf:
        tVZFilterLS_setFreqFast((tVZFilterLS*)filter, cutoff);
        return tVZFilterLS_tick((tVZFilterLS*)filter, input);
    case FiltTypeNotch:
        tVZFilterBR_setFreqFast((tVZFilterBR*)filter, cutoff);
        return tVZFilterBR_tick((tVZFilterBR*)filter, input) * filt->amp;
    case FiltTypeLadderLowpass:
        tLadderFilter_setFreqFast((tLadderFilter*)filter, cutoff);
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
    const float cutoff = filt->cutoffKnob + (filt->inputNote * filt->keyFollow);
    const float input = buffer[0];
    float output = tFiltModule_tickType(filt, filt->filtType, input, cutoff);

    if (filt->transitionSamplesRemaining > 0 && filt->previousFiltType != filt->filtType)
    {
        const float previous = tFiltModule_tickType(filt, filt->previousFiltType, input, cutoff);
        const float progress = 1.0f - ((float)filt->transitionSamplesRemaining
                                      / (float)filt->transitionSamplesTotal);
        output = previous + ((output - previous) * progress);
        --filt->transitionSamplesRemaining;
    }

    buffer[0] = filt->header.outputs[0] = output;
}

// Modulatable setters


void tFiltModule_setParameter(tFiltModule const filt, FiltParams param_type,float input)
{
	switch (param_type) {
	case FiltEventWatchFlag:

		break;
	case FiltMidiPitch:
		filt->inputNote = input * 127.0f;
		break;
	case FiltCutoff:
		filt->cutoffKnob = input * 127.0f;
		break;
	case FiltGain:
		tFiltModule_setGain(filt, input);
		break;
	case FiltResonance:
		tFiltModule_setRes(filt, input);
		break;
	case FiltKeyfollow:
		filt->keyFollow = input;
		break;
	case FiltType:
		tFiltModule_setType(filt, (uint32_t)roundf(LEAF_clip(0.0f, input, 1.0f)
		                                              * (float)(FiltNumTypes - 1)), 0);
		break;

	default:
		break;
	}
}

void tFiltModule_setType(tFiltModule const filt, uint32_t type, uint32_t transitionSamples)
{
    if (type >= FiltNumTypes || type == filt->filtType)
        return;

    filt->previousFiltType = filt->filtType;
    filt->filtType = type;
    filt->theFilt = filt->filters[type];
    tFiltModule_resetNonlinearState(filt, type);
    filt->transitionSamplesTotal = transitionSamples;
    filt->transitionSamplesRemaining = transitionSamples;
    if (transitionSamples == 0)
        filt->previousFiltType = type;
}

void tFiltModule_setMIDIPitch (tFiltModule const filt, float const input)
{
    filt->inputNote = input * 127.0f;
}

void tFiltModule_setCutoff (tFiltModule const filt, float const input)
{
    filt->cutoffKnob = input * 127.0f;
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
