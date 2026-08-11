//
// Created by Matthew McWeeney on 7/9/26.
//

#include "NoiseModule.h"
#include "leaf-envelopes.h"
#include <iostream>

void tNoiseModule_init(void** const noise, float* params, float id, LEAF* const leaf)
{
    if(leaf->resTable == NULL)
    {
        tLookupTable_create(&leaf->mempool,&leaf->resTable);
        tLookupTable_init(leaf, leaf->resTable,  0.0f, 10.0f, 0.5f, 2048);
    }
    tNoiseModule_initToPool(noise, params, id, &leaf->mempool, leaf->resTable);
}

void tNoiseModule_setParameter(tNoiseModule const noise, const NosParams param_type, float input)
{
    switch (param_type) {
        case NoiseGain:
            if (noise->gain != input)
            {
                noise->gain = input;
            }
            break;
        case NoiseTilt:
            input = (20.f * input) - 10.f;
            if (noise->tilt != input)
            {
                noise->tilt = input;
                tTiltFilter_setTilt(&noise->theTilter, input);
            }
            break;
        case NoisePeakGain:
            if (noise->peakGain != input)
            {
                noise->peakGain = input;
                tVZFilterBell_setGain(&noise->theBellter, input * 90.f + 1.f); // < 1 causes a dip at the target frequency - might add later
            }
            break;
        case NoiseFreqKnob:
            input = input * 137.5f;
            if (noise->freqKnob != input)
            {
                noise->freqKnob = input;
            }
            break;
        case NoisePeakBandwidth:
            if (noise->peakBandwidth != input)
            {
                noise->peakBandwidth = input;
                tVZFilterBell_setBandwidth(&noise->theBellter, input * 5.f);
            }
            break;
        case NoiseKeyFollow:
            if (noise->keyFollow != input)
            {
                noise->keyFollow = input;
            }
            break;
        case NoiseGlide:
            if (noise->glide != input)
            {
                noise->glide = input;
                tRamp_setTime(&noise->pitchSmoother, input);
            }
            break;
        case NoisePortaType:
            if (noise->portaType != input)
            {
                noise->portaType = input;
            }
            break;
        case NoiseMIDIPitch:
            //noise->inputMIDINote = input * 127.0;
            //tNoiseModule_setInputNote(noise, input * 127.0);
            if (noise->inputMIDINote != input * 127.0)
            {
                noise->inputMIDINote = input * 127.0;
                tRamp_setDest(&noise->pitchSmoother, noise->inputMIDINote);
            }
            break;
        default:
            break;
    }
}

void tNoiseModule_initToPool(void** const noise, float* const param, float id, tMempool** const mempool, tLookupTable* resTable)
{
    tMempool* m = *mempool;
    _tNoiseModule* NoiseModule = (_tNoiseModule*) (*noise = (_tNoiseModule*) mpool_alloc (sizeof (_tNoiseModule), m));

#ifndef __cplusplus
    memcpy(NoiseModule->params, param, NoiseNumParams*sizeof(float));
#endif __cplusplus
    NoiseModule->header.uniqueID = id;
    NoiseModule->mempool = m;
    NoiseModule->header.moduleType = ModuleTypeNoiseModule;

    tNoise_init(NoiseModule->mempool->leaf, &NoiseModule->theNoise, WhiteNoise);

    tTiltFilter_init(NoiseModule->mempool->leaf, &NoiseModule->theTilter, 1000.f);
    //tTiltFilter_setSampleRate((tTiltFilter*)&NoiseModule->theTilter, NoiseModule->mempool->leaf->sampleRate * 2.f);
    tVZFilterBell_init(NoiseModule->mempool->leaf, &NoiseModule->theBellter, 1000.f, 50.f, 1.f);
    //tVZFilterBell_setSampleRate((tVZFilterBell*)&NoiseModule->theBellter, NoiseModule->mempool->leaf->sampleRate * 2.f);
    tRamp_init (NoiseModule->mempool->leaf, &NoiseModule->pitchSmoother, 1.0f, 1);

#ifndef __cplusplus
    for (int i = 0; i < NoiseNumParams; i++)
    {
    	tNoiseModule_setParameter(NoiseModule, i, NoiseModule->params[i]);
    }
#endif
}

void tNoiseModule_free(void** const noise)
{
    _tNoiseModule* NoiseModule = (_tNoiseModule*) (*noise);

    // tTiltFilter_free((tTiltFilter**)&NoiseModule->theTilter);
    // tVZFilterBell_free((tVZFilterBell**)&NoiseModule->theBellter);
    // tNoise_free((tNoise**)&NoiseModule->theNoise);

    mpool_free((char*)NoiseModule, NoiseModule->mempool);
}

void tNoiseModule_setGlideOrigin (tNoiseModule const noise, float originNote)
{
    tRamp_setVal(&noise->pitchSmoother, originNote);
}

// void tNoiseModule_setInputNote (tNoiseModule const noise, float inputNote)
// {
//     if (noise->inputMIDINote != inputNote)
//     {
//         noise->inputMIDINote = inputNote;
//         tRamp_setDest(&noise->pitchSmoother, noise->inputMIDINote);
//     }
// }

void tNoiseModule_setPeakFreq (tNoiseModule const noise, float inputFreq)
{
    noise->peakFreq = inputFreq;
    tVZFilterBell_setFreqFast(&noise->theBellter, noise->peakFreq);
}

// tick function
void tNoiseModule_tick (tNoiseModule const noise,float* buffer)
{
    tNoiseModule_setPeakFreq(noise, tRamp_tick(&noise->pitchSmoother) * noise->keyFollow + noise->freqKnob);

    *buffer = tNoise_tick(&noise->theNoise);
    *buffer = tTiltFilter_tick(&noise->theTilter, *buffer);
    *buffer = tVZFilterBell_tick(&noise->theBellter, *buffer) * noise->gain;

    noise->header.outputs[0] = *buffer;
}

// Non-modulatable setters
// void tNoiseModule_setDBtoATableLocation (tNoiseModule const noise, float* const tableAddress, uint32_t const tableSize)
// {
//     noise->dbTableAddress = tableAddress;
//     noise->dbTableSizeMinusOne = (float)(tableSize - 1);
//     noise->dbTableScalar = noise->dbTableSizeMinusOne/(4.0f-0.00001f);
//     noise->dbTableOffset = 0.00001f * noise->dbTableScalar;
// }

// void tNoiseModule_setMTOFTableLocation (tNoiseModule const noise, float* const tableAddress)
// {
//     noise->mtofTable = tableAddress;
// }
//
// float dbToATableLookupFunctionNos(float const in, float const sizeMinusOne, float* const tableAddress)
// {
//     uint32_t inDBIndex = (uint32_t) in;
//     uint32_t inDBIndexPlusOne = inDBIndex + 1;
//     if (inDBIndexPlusOne > sizeMinusOne)
//     {
//         inDBIndexPlusOne = sizeMinusOne;
//     }
//     float alpha = in - (float)inDBIndex;
//     return ((tableAddress[inDBIndex] * (1.0f - alpha)) + (tableAddress[inDBIndexPlusOne] * alpha));
// }

