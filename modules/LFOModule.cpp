//
// Created by Joshua Warner on 6/13/24.
//

#include "LFOModule.h"
#include <cstdio>

#include <assert.h>

void tLFOModule_init(void** const lfo, float* params, float id, LEAF* const leaf)
{
    if(leaf->lfoRateTable == NULL)
    {
        tLookupTable_create(&leaf->mempool, &leaf->lfoRateTable);
        tLookupTable_init(leaf,leaf->lfoRateTable, 0.f, 30.f, 2.f,2048);
       // LEAF_generate_table_skew_non_sym(skewTableLFORate, 0.0f, 30.0f, 2.0f, 2048);
    }
    tLFOModule_initToPool(lfo, params, id,  &leaf->mempool, leaf->lfoRateTable);
}

void tLFOModule_initToPool(void** const lfo, float* const params, float id, tMempool** const mempool, tLookupTable* rateTable)
{
    tMempool* m = *mempool;
    _tLFOModule* LFOModule = static_cast<_tLFOModule*>(*lfo = (_tLFOModule*) mpool_alloc(sizeof(_tLFOModule), m));
    int type = 0;
#ifndef __cplusplus
    memcpy(LFOModule->params, params, LFONumParams*sizeof(ATOMIC_FLOAT));
    int type = roundf(CPPDEREF LFOModule->params[LFOType]);
#endif
    LFOModule->header.uniqueID = id;
    LFOModule->table = rateTable;
    //LFOModule->header.params[LFOType] = 0;
    LFOModule->lfo_type = type;

    LFOModule->mempool = m;

    LFOModule->counter = 0;

    //tSlopeRamp_create(mempool, &LFOModule->shapeSmoother);
    tSlopeRamp_init(m->leaf, &LFOModule->shapeSmoother, SMOOTH_SLOPE_MULTIPLIER, 0.f);
    // LFOModule->setterFunctions[LFOEventWatchFlag] = (tSetter)(&tLFOModule_blankFunction);
    // LFOModule->setterFunctions[LFOType] = (tSetter)(&tLFOModule_blankFunction);
    // LFOModule->setterFunctions[LFORateParam] = (tSetter)(&tLFOModule_setRate);

    //tSineTriLFO_create  (mempool, &LFOModule->sineTriLFO);
    tSineTriLFO_init    (m->leaf, &LFOModule->sineTriLFO);
    //tSawSquareLFO_create(mempool, &LFOModule->sawSquareLFO);
    tSawSquareLFO_init  (m->leaf, &LFOModule->sawSquareLFO);
    //tCycle_create       (mempool, &LFOModule->sineLFO);
    tCycle_init         (m->leaf, &LFOModule->sineLFO);
    //tTriLFO_create      (mempool, &LFOModule->triLFO);
    tTriLFO_init        (m->leaf, &LFOModule->triLFO);
    //tIntPhasor_create   (mempool, &LFOModule->sawLFO);
    tIntPhasor_init     (m->leaf, &LFOModule->sawLFO);
    //tSquareLFO_create   (mempool, &LFOModule->squareLFO);
    tSquareLFO_init     (m->leaf, &LFOModule->squareLFO);

    LFOModule->header.moduleType = ModuleTypeLFOModule;
}

void tLFOModule_free(void** const lfo)
{
    _tLFOModule* LFOModule = static_cast<_tLFOModule*>(*lfo);

    // tSineTriLFO_free    (&LFOModule->sineTriLFO);
    // tSawSquareLFO_free  (&LFOModule->sawSquareLFO);
    // tCycle_free         (&LFOModule->sineLFO);
    // tTriLFO_free        (&LFOModule->triLFO);
    // tIntPhasor_free     (&LFOModule->sawLFO);
    // tSquareLFO_free     (&LFOModule->squareLFO);
    // tSlopeRamp_free     (&LFOModule->shapeSmoother);

    mpool_free((char*)LFOModule, LFOModule->mempool);
}
//tick function
void tLFOModule_tick (tLFOModule const lfo)
{
    //const float input = noise->header.summedInput + buffer[0];
    lfo->header.summedInput = 0.0f;

    lfo->shape = tSlopeRamp_tick(&lfo->shapeSmoother);

    switch (lfo->lfo_type)
    {
        case(LFOTypeSineTri):
            tSineTriLFO_setShape(&lfo->sineTriLFO, lfo->shape);

            lfo->header.outputs[0] =tSineTriLFO_tick(&lfo->sineTriLFO);
            break;
        case(LFOTypeSawSquare):
            tSawSquareLFO_setShape(&lfo->sawSquareLFO, lfo->shape);

            lfo->header.outputs[0] = tSawSquareLFO_tick(&lfo->sawSquareLFO);
            break;
        case(LFOTypeSine):

            lfo->header.outputs[0] = tCycle_tick(&lfo->sineLFO);
            break;
        case(LFOTypeTri):

            lfo->header.outputs[0] = tTriLFO_tick(&lfo->triLFO);
            break;
        case(LFOTypeSaw):

            lfo->header.outputs[0] = tIntPhasor_tick(&lfo->sawLFO);
            break;
        case(LFOTypeSquare):
            tSquareLFO_setPulseWidth(&lfo->squareLFO, lfo->shape);

            lfo->header.outputs[0] = tSquareLFO_tick(&lfo->squareLFO);
            break;
        default:
            break;
    }

    //lfo->header.outputs[0] = lfo->lfoTicker(lfo->theLFO);
    //printf("Ticking!!");
}

//special noteOnFunction
void tLFOModule_onNoteOn(tLFOModule const lfo)
{
    //printf("Note on!!");
    // lfo->setterFunctions[LFOPhaseParam](lfo->theLFO, CPPDEREF lfo->params[LFOPhaseParam]); //call actual function

    if (lfo->counter == 0 && lfo->syncNoteOn == 1)
    {
        lfo->counter = 1; // counter is a band-aid fix for issue in SoundEngine, where noteOn is called for both the actual noteOn and noteOff events
        //printf("really on\n");
        tLFOModule_setPhase(lfo, lfo->phase);
        //printf("Cuz note offff!!!!\n");
    } else
    {
        lfo->counter = 0;
        //printf("really off\n");
    }
}

// Modulatable setters
// void tLFOModule_setRate (tLFOModule const lfo, float rate)
// {
//     rate *= (float)lfo->table->tableSize;
//     int inputInt = (int)rate;
//     float inputFloat = (float)inputInt - rate;
//     int nextPos = LEAF_clip(0, inputInt + 1, lfo->table->tableSize);
//     float tempRate = (lfo->table->table[inputInt] * (1.0f - inputFloat)) + (lfo->table->table[nextPos] * inputFloat);
//     //lfo->freq_set_func (lfo->theLFO, tempRate);
//     lfo->freqSetter (lfo->theLFO, tempRate);
//     printf("%f\n", tempRate);
// }

void tLFOModule_setRate (tLFOModule const lfo, float rate)
{
    lfo->rate = rate;
    switch(lfo->lfo_type)
    {
        case(LFOTypeSineTri):
            tSineTriLFO_setFreq(&lfo->sineTriLFO, rate);
            break;
        case(LFOTypeSawSquare):
            tSawSquareLFO_setFreq(&lfo->sawSquareLFO, rate);
            break;
        case(LFOTypeSine):
            tCycle_setFreq(&lfo->sineLFO, rate);
            break;
        case(LFOTypeTri):
            tTriLFO_setFreq(&lfo->triLFO, rate);
            break;
        case(LFOTypeSaw):
            tIntPhasor_setFreq(&lfo->sawLFO, rate);
            break;
        case(LFOTypeSquare):
            tSquareLFO_setFreq(&lfo->squareLFO, rate);
            break;
        default:
            break;
    }
}


void tLFOModule_setShape(tLFOModule const lfo, float shape)
{
    tSlopeRamp_setDest(&lfo->shapeSmoother, shape);

    switch (lfo->lfo_type)
    {
        case(LFOTypeSineTri):
            tSineTriLFO_setShape(&lfo->sineTriLFO, shape);
            break;
        case(LFOTypeSawSquare):
            tSawSquareLFO_setShape(&lfo->sawSquareLFO, shape);
            break;
        case(LFOTypeSquare):
            tSquareLFO_setPulseWidth(&lfo->squareLFO, shape);
            break;
        default:
            break;
    }
}

void tLFOModule_setPhase (tLFOModule const lfo, float phase)
{
    lfo->phase = phase;

    switch(lfo->lfo_type)
    {
        case(LFOTypeSineTri):
            tSineTriLFO_setPhase(&lfo->sineTriLFO, phase);
            break;
        case(LFOTypeSawSquare):
            tSawSquareLFO_setPhase(&lfo->sawSquareLFO, phase);
            break;
        case(LFOTypeSine):
            tCycle_setPhase(&lfo->sineLFO, phase);
            break;
        case(LFOTypeTri):
            tTriLFO_setPhase(&lfo->triLFO, phase);
            break;
        case(LFOTypeSaw):
            tIntPhasor_setPhase(&lfo->sawLFO, phase);
            break;
        case(LFOTypeSquare):
            tSquareLFO_setPhase(&lfo->squareLFO, phase);
            break;
        default:
            break;
    }

}

// Non-modulatable setters
//void tLFOModule_setRateTableLocationAndSize (tLFOModule const lfo, float* tableAddress, uint32_t size)
//{
//    lfo->rateTable = tableAddress;
//    lfo->rateTableSize = size-1;
//}

void tLFOModule_setSampleRate (tLFOModule const lfo, float sr)
{
    tSineTriLFO_setSampleRate(&lfo->sineTriLFO, sr);
    tSawSquareLFO_setSampleRate(&lfo->sawSquareLFO, sr);
    tCycle_setSampleRate(&lfo->sineLFO, sr);
    tTriLFO_setSampleRate(&lfo->triLFO, sr);
    tIntPhasor_setSampleRate(&lfo->sawLFO, sr);
    tSquareLFO_setSampleRate(&lfo->squareLFO, sr);
}

void tLFOModule_setType (tLFOModule const lfo, int type)
{
    lfo->lfo_type = type;
    //tLFOModule_setParameter(lfo, LFORateParam, lfo->inputRate);
    tLFOModule_setRate(lfo, lfo->rate);
    tLFOModule_setPhase(lfo, lfo->phase);
}

void tLFOModule_setParameter(tLFOModule const lfo, LFOParams param_type, float input)
{
    switch (param_type)
    {
        case LFOEventWatchFlag:
            break;

        case LFOType:
        {
            int temp = (int)roundf(5.4f * input);
            if (temp != lfo->lfo_type)
            {
                tLFOModule_setType(lfo, temp);
            }
            break;
        }

        case LFORateParam:
        {
            if (lfo->inputRate != input)
            {
                lfo->inputRate = input;
                // Interpolate lookup table and set frequency
                // input *= (float)lfo->table->tableSize;
                // int inputInt = (int)input;
                // float inputFloat = (float)inputInt - input;
                // int nextPos = LEAF_clip(0, inputInt + 1, lfo->table->tableSize);
                // float tempRate = (lfo->table->table[inputInt] * (1.0f - inputFloat))
                //                + (lfo->table->table[nextPos] * inputFloat);

                tLFOModule_setRate (lfo, lfo->table->table[(int)roundf(input * 2047)]);
                //printf("%f\n", tempRate);
                break;
            }
        }

        case LFOShapeParam:
            if (input != lfo->shapeSmoother.dest)
            {
                tSlopeRamp_setDest(&lfo->shapeSmoother, input);
                //printf("Always?!?!?\n");
            }
            break;

        case LFOPhaseParam:
            if (input != lfo->phase)
            {
                lfo->phase = input;
                //tLFOModule_setPhase (lfo, input);
                //printf("Normal param!!!\n");
            }
            break;

        case LFOSyncNoteOnParam:
            if ((uint8_t)input != lfo->syncNoteOn)
            {
                lfo->syncNoteOn = (uint8_t)input;
                //printf("changed sync!!!");
            }
            break;
        default:
            break;
    }
}
