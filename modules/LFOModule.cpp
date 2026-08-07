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

    tSlopeRamp_init(m->leaf, &LFOModule->shapeSmoother, SMOOTH_SLOPE_MULTIPLIER, 0.f);
    // LFOModule->setterFunctions[LFOEventWatchFlag] = (tSetter)(&tLFOModule_blankFunction);
    // LFOModule->setterFunctions[LFOType] = (tSetter)(&tLFOModule_blankFunction);
    // LFOModule->setterFunctions[LFORateParam] = (tSetter)(&tLFOModule_setRate);
    if (LFOModule->lfo_type == LFOTypeSineTri)
    {
        tSineTriLFO_create (mempool, (tSineTriLFO**)&LFOModule->theLFO);
        tSineTriLFO_init   (m->leaf, (tSineTriLFO*)LFOModule->theLFO);

        LFOModule->freqSetter = (tSetter)(&tSineTriLFO_setFreq);
        LFOModule->lfoTicker = (tTickFuncReturningFloat)(&tSineTriLFO_tick);
        LFOModule->phaseSetter = (tSetter)(&tSineTriLFO_setPhase);
        LFOModule->shapeSetter = (tSetter)(&tSineTriLFO_setShape);
        // LFOModule->header.tick          = (tTickFuncReturningFloat)(&tSineTriLFO_tick);
    }
    else if (LFOModule->lfo_type == LFOTypeSawSquare)
    {
        tSawSquareLFO_create (mempool, (tSawSquareLFO**)&LFOModule->theLFO);
        tSawSquareLFO_init   (m->leaf, (tSawSquareLFO*)LFOModule->theLFO);

        LFOModule->freqSetter = (tSetter)(&tSawSquareLFO_setFreq);
        LFOModule->lfoTicker = (tTickFuncReturningFloat)(&tSawSquareLFO_tick);
        LFOModule->phaseSetter = (tSetter)(&tSawSquareLFO_setPhase);
        LFOModule->shapeSetter = (tSetter)(&tSawSquareLFO_setShape);
        // LFOModule->header.tick          = (tTickFuncReturningFloat)(&tSawSquareLFO_tick);
    }
    else if (LFOModule->lfo_type == LFOTypeSine)
    {
        tCycle_create (mempool, (tCycle**)&LFOModule->theLFO);
        tCycle_init   (m->leaf, (tCycle*)LFOModule->theLFO);

        LFOModule->freqSetter = (tSetter)(&tCycle_setFreq);
        LFOModule->lfoTicker = (tTickFuncReturningFloat)(&tCycle_tick);
        LFOModule->phaseSetter = (tSetter)(&tCycle_setPhase);
        LFOModule->shapeSetter = blankSetter;
        // LFOModule->header.tick          = (tTickFuncReturningFloat)(&tCycle_tick);
    }
    else if (LFOModule->lfo_type == LFOTypeTri)
    {
        tTriLFO_create (mempool, (tTriLFO**)&LFOModule->theLFO);
        tTriLFO_init   (m->leaf, (tTriLFO*)LFOModule->theLFO);

        LFOModule->freqSetter = (tSetter)(&tTriLFO_setFreq);
        LFOModule->lfoTicker = (tTickFuncReturningFloat)(&tTriLFO_tick);
        LFOModule->phaseSetter = (tSetter)(&tTriLFO_setPhase);
        LFOModule->shapeSetter = blankSetter;
        // LFOModule->header.tick          = (tTickFuncReturningFloat)(&tTriLFO_tick);
    }
    else if (LFOModule->lfo_type == LFOTypeSaw)
    {
        tIntPhasor_create (mempool, (tIntPhasor**)&LFOModule->theLFO);
        tIntPhasor_init   (m->leaf, (tIntPhasor*)LFOModule->theLFO);

        LFOModule->freqSetter = (tSetter)(&tIntPhasor_setFreq);
        LFOModule->lfoTicker = (tTickFuncReturningFloat)(&tIntPhasor_tickBiPolar);
        LFOModule->phaseSetter = (tSetter)(&tIntPhasor_setPhase);
        LFOModule->shapeSetter = blankSetter;
        // LFOModule->header.tick          = (tTickFuncReturningFloat)(&tIntPhasor_tickBiPolar);
    }
    else if (LFOModule->lfo_type == LFOTypeSquare)
    {
        tSquareLFO_create (mempool, (tSquareLFO**)&LFOModule->theLFO);
        tSquareLFO_init   (m->leaf, (tSquareLFO*)LFOModule->theLFO);

        LFOModule->freqSetter = (tSetter)(&tSquareLFO_setFreq);
        LFOModule->lfoTicker = (tTickFuncReturningFloat)(&tSquareLFO_tick);
        LFOModule->phaseSetter = (tSetter)(&tSquareLFO_setPhase);
        LFOModule->shapeSetter = (tSetter)(&tSquareLFO_setPulseWidth);
        // LFOModule->header.tick          = (tTickFuncReturningFloat)(&tSquareLFO_tick);
    }

    LFOModule->header.moduleType = ModuleTypeLFOModule;
}

void tLFOModule_free(void** const lfo)
{
    _tLFOModule* LFOModule = static_cast<_tLFOModule*>(*lfo);

    switch (LFOModule->lfo_type)
    {
        case(LFOTypeSineTri):
            tSineTriLFO_free((tSineTriLFO**)&LFOModule->theLFO);
            break;
        case(LFOTypeSawSquare):
            tSawSquareLFO_free((tSawSquareLFO**)&LFOModule->theLFO);
            break;
        case(LFOTypeSine):
            tCycle_free((tCycle**)&LFOModule->theLFO);
            break;
        case(LFOTypeTri):
            tTriLFO_free((tTriLFO**)&LFOModule->theLFO);
            break;
        case(LFOTypeSaw):
            tIntPhasor_free((tIntPhasor**)&LFOModule->theLFO);
            break;
        case(LFOTypeSquare):
            tSquareLFO_free((tSquareLFO**)&LFOModule->theLFO);
            break;
        default:
            break;
    }
    mpool_free((char*)LFOModule, LFOModule->mempool);
}
//tick function
void tLFOModule_tick (tLFOModule const lfo)
{
    lfo->shape = tSlopeRamp_tick(&lfo->shapeSmoother);
    lfo->shapeSetter(lfo->theLFO, lfo->shape);

    lfo->header.outputs[0] = lfo->lfoTicker(lfo->theLFO);
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
        lfo->phaseSetter(lfo->theLFO, lfo->phase);
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

void tLFOModule_setShape(tLFOModule const lfo, float shape)
{
    ;
}

void tLFOModule_setPhase (tLFOModule const lfo, float phase)
{
    lfo->phase = phase;
}

// Non-modulatable setters
//void tLFOModule_setRateTableLocationAndSize (tLFOModule const lfo, float* tableAddress, uint32_t size)
//{
//    lfo->rateTable = tableAddress;
//    lfo->rateTableSize = size-1;
//}

void tLFOModule_setSampleRate (tLFOModule const lfo, float sr)
{
    switch(lfo->lfo_type)
    {
        case(LFOTypeSineTri):
            tSineTriLFO_setSampleRate((tSineTriLFO*)lfo->theLFO, sr);
            break;
        case(LFOTypeSawSquare):
            tSawSquareLFO_setSampleRate((tSawSquareLFO*)lfo->theLFO, sr);
            break;
        case(LFOTypeSine):
            tCycle_setSampleRate((tCycle*)lfo->theLFO, sr);
            break;
        case(LFOTypeTri):
            tTriLFO_setSampleRate((tTriLFO*)lfo->theLFO, sr);
            break;
        case(LFOTypeSaw):
            tIntPhasor_setSampleRate((tIntPhasor*)lfo->theLFO, sr);
            break;
        case(LFOTypeSquare):
            tSquareLFO_setSampleRate((tSquareLFO*)lfo->theLFO, sr);
            break;
        default:
            break;
    }
}

void tLFOModule_setType (tLFOModule const lfo, int type)
{
    switch(lfo->lfo_type)
    {
        case(LFOTypeSineTri):
            tSineTriLFO_free((tSineTriLFO**)&lfo->theLFO);
            break;
        case(LFOTypeSawSquare):
            tSawSquareLFO_free((tSawSquareLFO**)&lfo->theLFO);
            break;
        case(LFOTypeSine):
            tCycle_free((tCycle**)&lfo->theLFO);
            break;
        case(LFOTypeTri):
            tTriLFO_free((tTriLFO**)&lfo->theLFO);
            break;
        case(LFOTypeSaw):
            tIntPhasor_free((tIntPhasor**)&lfo->theLFO);
            break;
        case(LFOTypeSquare):
            tSquareLFO_free((tSquareLFO**)&lfo->theLFO);
            break;
        default:
            break;
    }

    lfo->lfo_type = type;

    switch(lfo->lfo_type)
    {
        case(LFOTypeSineTri):
            tSineTriLFO_create (&lfo->mempool, (tSineTriLFO**)&lfo->theLFO);
            tSineTriLFO_init   (lfo->mempool->leaf, (tSineTriLFO*)lfo->theLFO);

            lfo->freqSetter = (tSetter)(&tSineTriLFO_setFreq);
            lfo->lfoTicker = (tTickFuncReturningFloat)(&tSineTriLFO_tick);
            lfo->phaseSetter = (tSetter)(&tSineTriLFO_setPhase);
            lfo->shapeSetter = (tSetter)(&tSineTriLFO_setShape);
            break;
        case(LFOTypeSawSquare):
            tSawSquareLFO_create (&lfo->mempool, (tSawSquareLFO**)&lfo->theLFO);
            tSawSquareLFO_init   (lfo->mempool->leaf, (tSawSquareLFO*)lfo->theLFO);

            lfo->freqSetter = (tSetter)(&tSawSquareLFO_setFreq);
            lfo->lfoTicker = (tTickFuncReturningFloat)(&tSawSquareLFO_tick);
            lfo->phaseSetter = (tSetter)(&tSawSquareLFO_setPhase);
            lfo->shapeSetter = (tSetter)(&tSawSquareLFO_setShape);
            break;
        case(LFOTypeSine):
            tCycle_create (&lfo->mempool, (tCycle**)&lfo->theLFO);
            tCycle_init   (lfo->mempool->leaf, (tCycle*)lfo->theLFO);

            lfo->freqSetter = (tSetter)(&tCycle_setFreq);
            lfo->lfoTicker = (tTickFuncReturningFloat)(&tCycle_tick);
            lfo->phaseSetter = (tSetter)(&tCycle_setPhase);
            lfo->shapeSetter = blankSetter;
            break;
        case(LFOTypeTri):
            tTriLFO_create (&lfo->mempool, (tTriLFO**)&lfo->theLFO);
            tTriLFO_init   (lfo->mempool->leaf, (tTriLFO*)lfo->theLFO);

            lfo->freqSetter = (tSetter)(&tTriLFO_setFreq);
            lfo->lfoTicker = (tTickFuncReturningFloat)(&tTriLFO_tick);
            lfo->phaseSetter = (tSetter)(&tTriLFO_setPhase);
            lfo->shapeSetter = blankSetter;
            break;
        case(LFOTypeSaw):
            tIntPhasor_create (&lfo->mempool, (tIntPhasor**)&lfo->theLFO);
            tIntPhasor_init   (lfo->mempool->leaf, (tIntPhasor*)lfo->theLFO);

            lfo->freqSetter = (tSetter)(&tIntPhasor_setFreq);
            lfo->lfoTicker = (tTickFuncReturningFloat)(&tIntPhasor_tickBiPolar);
            lfo->phaseSetter = (tSetter)(&tIntPhasor_setPhase);
            lfo->shapeSetter = blankSetter;
            break;
        case(LFOTypeSquare):
            tSquareLFO_create (&lfo->mempool, (tSquareLFO**)&lfo->theLFO);
            tSquareLFO_init   (lfo->mempool->leaf, (tSquareLFO*)lfo->theLFO);

            lfo->freqSetter = (tSetter)(&tSquareLFO_setFreq);
            lfo->lfoTicker = (tTickFuncReturningFloat)(&tSquareLFO_tick);
            lfo->phaseSetter = (tSetter)(&tSquareLFO_setPhase);
            lfo->shapeSetter = (tSetter)(&tSquareLFO_setPulseWidth);
            break;
        default:
            break;
    }
}

void tLFOModule_setParameter(tLFOModule const lfo, LFOParams param_type, float input)
{
    switch (param_type)
    {
        case LFOEventWatchFlag:
            break;

        case LFOType:
        {
            int temp = (int)roundf(5.5f * input);
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
                input *= (float)lfo->table->tableSize;
                int inputInt = (int)input;
                float inputFloat = (float)inputInt - input;
                int nextPos = LEAF_clip(0, inputInt + 1, lfo->table->tableSize);
                float tempRate = (lfo->table->table[inputInt] * (1.0f - inputFloat))
                               + (lfo->table->table[nextPos] * inputFloat);
                lfo->freqSetter(lfo->theLFO, tempRate);
                //printf("%f\n", tempRate);
                break;
            }
        }

        case LFOShapeParam:
            if (input != lfo->shapeSmoother.dest)
            {
                lfo->shapeSmoother.dest = input;
            }
            break;

        case LFOPhaseParam:
            if (input != lfo->phase)
            {
                tLFOModule_setPhase (lfo, input);
            }
            break;

        case LFOSyncNoteOnParam:
            if ((uint8_t)input != lfo->syncNoteOn)
            {
                lfo->syncNoteOn = (uint8_t)input;
                printf("changed sync!!!");
            }
            break;
        default:
            break;
    }
}
