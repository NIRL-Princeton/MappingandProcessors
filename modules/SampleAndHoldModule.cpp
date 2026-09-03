//
// Created by Matthew McWeeney on 8/28/26.
//

#include "SampleAndHoldModule.h"
#include "leaf-envelopes.h"

#include <cstdio>

void tSampleAndHoldModule_init(void** const sampHold, float* params, float id, LEAF* const leaf)
{
    tSampleAndHoldModule_initToPool(sampHold, params, id, &leaf->mempool);
}

void tSampleAndHoldModule_setParameter(tSampleAndHoldModule const sampHold, const SampHoldParams param_type, float input)
{
    switch (param_type) {
        case SampHoldEventWatchFlag:
            break;
        case SampHoldThreshold:
            input = sampHold->gainAmpTable->table[(int)(input*2047)];
            sampHold->threshold = input;
            break;
        case SampHoldFrequency:
            sampHold->frequency = sampHold->skewFreqTable->table[(int)(input*16383)];
            if (sampHold->frequency < 0.0011)
            {
                sampHold->hold = 1;
            } else
            {
                sampHold->hold = 0;
                tSampleAndHoldModule_setBinLength(sampHold);
            }
            break;
        case SampHoldDurRand:
            sampHold->durRand = input;
            tSampleAndHoldModule_setBinLength(sampHold);
            break;
        case SampHoldGain:
            input = sampHold->gainAmpTable->table[(int)(input*2047)];
            tSlopeRamp_setDest(&sampHold->gainSmoother, input);
            break;
        case SampHoldMix:
            tSlopeRamp_setDest(&sampHold->mixSmoother, input);
            break;
        case SampHoldMidiPitch:
            sampHold->pitch = sampHold->mtofTable->table[(int)(input*16383)];
            tSampleAndHoldModule_setBinLength(sampHold);
            break;
        case SampHoldKeyFollow:
            sampHold->keyFollow = input;
            tSampleAndHoldModule_setBinLength(sampHold);
            break;
        case SampHoldHarmonic:
            input = (input - .5f) * 30.f;

            if (input >= 0.0f) {
                sampHold->harmonicMultiplier = (input + 1.0f);
            } else {
                sampHold->harmonicMultiplier = (1.0f / fabsf((input - 1.0f)));
            }
            tSampleAndHoldModule_setBinLength(sampHold);
            break;
        default:
            break;
    }
}

void tSampleAndHoldModule_initToPool(void** const sampHold, float* const param, float id, tMempool** const mempool)
{
    tMempool* m = *mempool;
    _tSampleAndHoldModule* SampleAndHoldModule = (_tSampleAndHoldModule*) (*sampHold = (_tSampleAndHoldModule*) mpool_alloc (sizeof (_tSampleAndHoldModule), m));

    SampleAndHoldModule->header.uniqueID = id;
    SampleAndHoldModule->mempool = m;
    SampleAndHoldModule->header.moduleType = ModuleTypeSampleAndHoldModule;

    SampleAndHoldModule->frequency = 10.f;
    SampleAndHoldModule->binLength = 1/10.f*44100.f;
    SampleAndHoldModule->counter = 0;
    SampleAndHoldModule->currSample = 0.f;

    tSlopeRamp_init(m->leaf, &SampleAndHoldModule->gainSmoother, SMOOTH_SLOPE_MULTIPLIER * 4.f, 1.f);
    tSlopeRamp_init(m->leaf, &SampleAndHoldModule->mixSmoother, SMOOTH_SLOPE_MULTIPLIER, 1.f);

    SampleAndHoldModule->sampleRate = m->leaf->sampleRate;
    SampleAndHoldModule->invSampleRate = m->leaf->invSampleRate;

    tLookupTable_create(&SampleAndHoldModule->mempool, &SampleAndHoldModule->skewFreqTable);
    tLookupTable_init (SampleAndHoldModule->mempool->leaf, SampleAndHoldModule->skewFreqTable, 0.001f, 20000.f, 20.f, 16384);

    tLookupTable_create(&SampleAndHoldModule->mempool, &SampleAndHoldModule->gainAmpTable);
    tLookupTable_init (SampleAndHoldModule->mempool->leaf, SampleAndHoldModule->gainAmpTable, 0.f, 4.f, 1.f, 2048);

    tLookupTable_create(&SampleAndHoldModule->mempool, &SampleAndHoldModule->mtofTable);
    tLookupTable_init (SampleAndHoldModule->mempool->leaf, SampleAndHoldModule->mtofTable, 0.f, 0.f, 0.f, 16384);
    LEAF_generate_mtof (SampleAndHoldModule->mtofTable->table, 0., 127, 16384);
}

void tSampleAndHoldModule_free(void** const sampHold)
{
    _tSampleAndHoldModule* SampleAndHoldModule = (_tSampleAndHoldModule*) (*sampHold);
    mpool_free((char*)SampleAndHoldModule, SampleAndHoldModule->mempool);
}

void tSampleAndHoldModule_onNoteOn(tSampleAndHoldModule const sampHold, float vel)
{
    if (vel > -1)
    {
        sampHold->counter = sampHold->binLength;
    }
}

void tSampleAndHoldModule_setBinLength(tSampleAndHoldModule const sampHold)
{
    sampHold->finalFreq = LEAF_clip(0.f, (sampHold->frequency + sampHold->pitch * sampHold->keyFollow) * sampHold->harmonicMultiplier, 2.f*sampHold->sampleRate);
    sampHold->binLength = (1.f/sampHold->finalFreq*sampHold->sampleRate) * (1 + 2*(sampHold->mempool->leaf->random() - 0.5)*sampHold->durRand);
    //sampHold->binLength = 1.f/sampHold->frequency*sampHold->sampleRate;
}

// tick function
void tSampleAndHoldModule_tick (tSampleAndHoldModule const sampHold, float* buffer)
{
    sampHold->mix = tSlopeRamp_tick(&sampHold->mixSmoother);
    sampHold->gain = tSlopeRamp_tick(&sampHold->gainSmoother);

    sampHold->counter++;
    //printf("%f\n", buffer[0]);

    if (sampHold->counter > sampHold->binLength && buffer[0] >= sampHold->threshold && sampHold->hold == 0){
        sampHold->counter = 0;
        sampHold->currSample = buffer[0] * sampHold->gain;
        tSampleAndHoldModule_setBinLength(sampHold);
    }
    //sampHold->currSample = tRamp_tick(&sampHold->sampleSmoother) * sampHold->gain;

    buffer[0] = sampHold->header.outputs[0] = buffer[0] * (1.f - sampHold->mix) + sampHold->mix * sampHold->currSample;
    //buffer[0] = sampHold->header.outputs[0] = buffer[0];
    //buffer[0] = sampHold->header.outputs[0] = sampHold->currSample;
}
