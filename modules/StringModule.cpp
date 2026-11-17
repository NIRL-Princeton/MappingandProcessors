//
// Created by Davis Polito on 4/29/25.
//
#include "StringModule.h"
#include "string.h"

void tStringModule_init(void** const module, float* params, float id, LEAF* const leaf)
{
    tStringModule_initToPool(module, params, id, &leaf->mempool);
}

void tStringModule_free(void** const mod)
{
    _tStringModule* module =(tStringModule )(*mod);
    tSimpleLivingString3_free(&module->theString);
    mpool_free((char*)module, module->mempool);
}
void tStringModule_initToPool(void** const s, float* const params, float id, tMempool** const mempool)
{
    tMempool* m = *mempool;
    _tStringModule* module = (_tStringModule *)(*s = (_tStringModule*) mpool_alloc(sizeof(_tStringModule), m));
    #ifndef __cplusplus
    for (int i = 0; i < StringNumParams; i++)
    {
    	module->header.params[i] = params[i];
    }
    #endif
    module->mempool = m;
    module->header.moduleType = ModuleTypeStringModule;
   //can't figure out how to get this module to add itself as a listener of the event emitter... -JS
    tSimpleLivingString3_create (mempool, &module->theString);
    tSimpleLivingString3_init   (m->leaf,
                                 module->theString,
                                 1,
                                 440.f,
                                 440.0,
                                 1.0,
                                 1.0,
                                 1.0,
                                 1.0,
                                 0);
    tExpSmooth_init(m->leaf,&module->barSlideSmoother, 0.0f, 0.001f);
    tExpSmooth_init(m->leaf,&module->barMovementSmoother, 0.0f, 0.01f);
    tEnvelopeFollower_init(m->leaf,&module->barNoiseSmoother,0.000001f, 0.9993f);
    tNoise_init(m->leaf,&module->noise, WhiteNoise);
    tVZFilter_init(m->leaf, &module->noiseFilter, Lowpass, 800.0f, 0.9f);
    tVZFilter_setFreq( &module->noiseFilter, 3332.0f); //based on testing with knob values
    tPickupNonLinearity_init(m->leaf, &module->pu);
    module->slideNoiseAmount = 0.2f;
    module->inputNote = 40.0f;
    module->decayIn = 100.0f;
    module->freqOffset = 0.0f;
    module->lsDecay = 1;
    module->prevSlidePosition = 0.0f;
	#ifndef __cplusplus
	for (int i = 0; i < StringNumParams; i++)
	{
		tStringModule_setParameter(module, i, module->header.params[i]);
	}
	#endif
    //module->header.setterFunctions[StringEventWatchFlag] =(tSetter) &tStringModule_onNoteOn;
    //CPPDEREF module->header.params[StringEventWatchFlag] = 1.0f;
}


void tStringModule_tick(tStringModule const s)
{
    const float input = s->header.inputs[0];
    s->header.inputs[0] = 0.0f;

    float temp = 0.0f;
	float note;


	note = s->inputNote + s->octave;
	//sourceValues[MIDI_KEY_SOURCE_OFFSET][v] = (note[v] - midiKeySubtractor) * midiKeyDivisor;

	if (note < 0.0f)
	{
		note = 0.0f;
	}
	if (note > 127.0f)
	{
		note = 127.0f;
	}
	if (isnan(note))
	{
		note = 64.0f;
	}

	//float finalFreq = mtof(note[i]);
	float dampFreq = 15778.3f;
	float decay = 0.1f;
	if (s->lsDecay)
	{
		decay = (s->decayIn * 800.0f) + 10.0f;
	}

	tSimpleLivingString3_setDecay(s->theString, decay);
	//tSimpleLivingString3_setDampFreq(s->theString, dampFreq);


	tSimpleLivingString3_setFreq(s->theString, mtof(note));

	float currentBarSlide = tExpSmooth_tick(&s->barMovementSmoother);
	s->slideChange = fabsf(currentBarSlide - s->slidePosition);
	s->slidePosition = currentBarSlide;
	float barDelta = s->slideChange;
	if (barDelta > 0.9f)
	{
		barDelta = 0.0f; //to avoid noise on open string glitches
	}
	tExpSmooth_setDest(&s->barSlideSmoother, barDelta);
	barDelta = tExpSmooth_tick(&s->barSlideSmoother);
	barDelta = tEnvelopeFollower_tick(&s->barNoiseSmoother, barDelta);
	barDelta -= 0.00003f;//remove noise
	if (barDelta < 0.0f)
	{
		barDelta = 0.0f;
	}
	//tVZFilter_setFreq(&noiseFilt, faster_mtof(knobScaled[5] * 128.0f));

	float filtNoise = tVZFilter_tickEfficient(&s->noiseFilter, tNoise_tick(&s->noise));
	//filtNoise = tVZFilter_tickEfficient(&noiseFilt2, filtNoise);
	float slideNoise = filtNoise * barDelta * s->slideNoiseAmount * 100.0f;

	temp = tPickupNonLinearity_tick(&s->pu, tSimpleLivingString3_tick(s->theString, LEAF_clip(-1.0f, slideNoise + input, 1.0f)));
    s->header.outputs[0] = temp;
}


void tStringModule_onNoteOn(tStringModule const s, float velocity)
{
    float Vel = velocity;
    if (velocity > 0.0001f)
    {
        s->lsDecay = 1;
        tSimpleLivingString3_pluck(s->theString, Vel, s->pluckPosition);
    }
    else
    {
        s->lsDecay = 0;
    }
}

void tStringModule_setParameter(tStringModule const s, StringModuleParams param, float input)
{
    switch (param)
    {
        case StringEventWatchFlag:
            //CPPDEREF s->header.params[StringEventWatchFlag] = input; // store directly or implement oversample logic
            break;

        case StringOversample:
            //CPPDEREF s->header.params[StringOversample] = input; // store directly or implement oversample logic
            break;


        case StringMIDIPitch:
            s->inputNote = input * 127.0f;
            break;

        case StringFreqOffset:
            s->freqOffset = input * 2000.0f;
            break;

        case StringOctave:
            s->octave = (roundf((input * 8.0f) - 4.0f) * 12.0f);
            break;

        case StringDampFreq:
            tSimpleLivingString3_setDampFreq(s->theString, mtof(input * 136.0f));
            break;

        case StringDecay:
            s->decayIn = (input * 800.0f) + 10.0f;
            break;

        case StringTargetLevel:
            tSimpleLivingString3_setTargetLev(s->theString, input);
            break;

        case StringLevelSmooth:
            tSimpleLivingString3_setLevSmoothFactor(s->theString, input);
            break;

        case StringLevelStrength:
            tSimpleLivingString3_setLevStrength(s->theString, input * 0.0352872f);
            break;

        case StringPickupPoint:
            tSimpleLivingString3_setPickupPoint(s->theString, input);
            break;

        case StringLevelMode:
            tSimpleLivingString3_setLevMode(s->theString, (int)input);
            break;

        case StringRippleGain:
            s->theString->rippleGain = input * -0.03f;
            s->theString->invOnePlusr = 1.0f / (1.0f + s->theString->rippleGain);
            break;

        case StringRippleDelay:
            s->theString->rippleDelay = input;
            break;

        case StringPluckPosition:
            s->pluckPosition = input;
            // s->theString->pluckPosition = input;
            break;

        default:
            // unknown param, do nothing
            break;
    }
}

void tStringModule_setSlidePosition(tStringModule const s, float input)
{
    tExpSmooth_setDest(&s->barMovementSmoother, input);


}
