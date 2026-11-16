//
// Created by Jeff Snyder on 11/15/26.
//

#include "ControlModule.h"
//#include <cstdio>

#include <assert.h>
//#include <iostream>
#include "string.h"
void tControlModule_init (void** const ctl, float* params, float id, LEAF* const leaf)
{
    tControlModule_initToPool (ctl, params, id, &leaf->mempool);
}

void tControlModule_free (void** const ctl)
{
    _tControlModule* CtlModule = (_tControlModule*) (*ctl);
    mpool_free ((char*) CtlModule, CtlModule->mempool);
}
//tick function
void tControlModule_tick (tControlModule const ctl)
{
    //const float input = noise->header.summedInput + buffer[0];
    ctl->header.inputs[0] = 0.0f;
    float temp = tExpSmooth_tick(&ctl->smooth);
    //CPPDEREF ctl->header.params[EnvVelocitySense] = tSlopeRamp_tick(&env->velSenseSmoother);
    ctl->header.outputs[0] = temp;//set the output
}

void tControlModule_setSampleRate (tControlModule const env, float sr)
{
    //how to handle this? if then cases for different types?
}

uint8_t tControlModule_getCCNum (tControlModule const ctl)
{
	return ctl->ccNum;
}

void tControlModule_setParameter (tControlModule const ctl, int parameter_id, float input)
{
    switch (parameter_id)
    {
        case ControlEventWatchFlag:
        {
            // handled by onnoteon listener
            break;
        }

        case ControlCCParam:
        {
            ctl->ccNum = (uint8_t)(lround(input * 16.0f));
            break;
        }

        case ControlCCValue:
        {
            ctl->ccValue = input;
            tExpSmooth_setDest(&ctl->smooth, input);
            break;
        }


        default:
            break;
    }
}

void tControlModule_initToPool (void** const ctl, float* const params, float id, tMempool** const mempool)
{
    tMempool* m = *mempool;
    _tControlModule* CtlModule = (_tControlModule*) (*ctl = (_tControlModule*) mpool_alloc (sizeof (_tControlModule), m));
#ifndef __cplusplus
    memcpy (CtlModule->header.params, params, ControlNumParams * sizeof(float));
#endif __cplusplus
    CtlModule->mempool = m;

    CtlModule->header.uniqueID = id;
    tExpSmooth_init(m->leaf, &CtlModule->smooth, 0.0f, 0.003f);
    CtlModule->header.moduleType = ModuleTypeControlModule;
#ifndef __cplusplus
    for (int i = 0; i < ControlNumParams; i++)
    {
    	tControlModule_setParameter(CtlModule, i, CtlModule->header.params[i]);
    }
#endif
}

