//
// Created by Davis Polito on 4/29/25.
//
#include "StringBModule.h"


void tStringBModule_init(void** const module, float* params, float id, LEAF* const leaf)
{
    tStringBModule_initToPool(module, params, id, &leaf->mempool);
}

void tStringBModule_free(void** const mod)
{
    _tStringBModule* module =(tStringBModule )(*mod);
    mpool_free((char*)module, module->mempool);
}
void tStringBModule_initToPool(void** const s, float* const params, float id, tMempool** const mempool)
{
    tMempool* m = *mempool;
    _tStringBModule* module = (_tStringBModule *)(*s = (_tStringBModule*) mpool_alloc(sizeof(_tStringBModule), m));
    #ifndef __cplusplus
    memcpy(module->header.params, params, StringNumParams);
    #endif
    module->mempool = m;
    module->header.moduleType = ModuleTypeStringBModule;
   //can't figure out how to get this module to add itself as a listener of the event emitter... -JS
    tSimpleLivingString3_create (mempool, &module->theString);
    tSimpleLivingString3_init   (m->leaf,
                                 module->theString,
                                 4,
                                 440.f,
                                 440.0,
                                 1.0,
                                 1.0,
                                 1.0,
                                 1.0,
                                 0);
    //module->header.setterFunctions[StringEventWatchFlag] =(tSetter) &tStringBModule_onNoteOn;
    //CPPDEREF module->header.params[StringEventWatchFlag] = 1.0f;
}


void tStringBModule_tick(tStringBModule const s)
{
    const float input = s->header.inputs[0];
    s->header.inputs[0] = 0.0f;
    float inputFreq = mtof(s->inputNote);
    tSimpleLivingString3_setFreq(s->theString, inputFreq);
    s->header.outputs[0] = tSimpleLivingString3_tick(s->theString, input);
}


void tStringBModule_onNoteOn(tStringBModule const s, float velocity)
{
    float Vel = velocity;
    if (velocity > 0.0001f)
    {

        tSimpleLivingString3_pluck(s->theString, Vel, s->pluckPosition);
    }
    else
    {
        ;
    }
}
void tStringBModule_setParameter(tStringBModule const s, StringModelParams param, float input)
{
    switch (param)
    {
        case StringBEventWatchFlag:
            //CPPDEREF s->header.params[StringEventWatchFlag] = input; // store directly or implement oversample logic
            break;

        case StringBOversample:
            //CPPDEREF s->header.params[StringOversample] = input; // store directly or implement oversample logic
            break;


        case StringBMIDIPitch:
            s->inputNote = input * 127.0f;
            break;

        case StringBFreq:

            break;

        case StringBWaveLength:
            //tSimpleLivingString3_setWaveLength(s->theString, input * 95.f + 5.f);

            break;

        case StringBDampFreq:
            tSimpleLivingString3_setDampFreq(s->theString, mtof(input * 127.0f));
            break;

        case StringBDecay:
            tSimpleLivingString3_setDecay(s->theString, input * 400.0f);
            break;

        case StringBTargetLevel:
            tSimpleLivingString3_setTargetLev(s->theString, input);
            break;

        case StringBLevelSmooth:
            tSimpleLivingString3_setLevSmoothFactor(s->theString, input);
            break;

        case StringBLevelStrength:
            tSimpleLivingString3_setLevStrength(s->theString, input);
            break;

        case StringBPickupPoint:
            tSimpleLivingString3_setPickupPoint(s->theString, input);
            break;

        case StringBLevelMode:
            tSimpleLivingString3_setLevMode(s->theString, (int)input);
            break;

        case StringBRippleGain:
            s->theString->rippleGain = input * -0.03f;
            s->theString->invOnePlusr = 1.0f / (1.0f + s->theString->rippleGain);
            break;

        case StringBRippleDelay:
            s->theString->rippleDelay = input;
            break;

        case StringBPluckPosition:
            s->pluckPosition = input;
            // s->theString->pluckPosition = input;
            break;

        default:
            // unknown param, do nothing
            break;
    }
}
