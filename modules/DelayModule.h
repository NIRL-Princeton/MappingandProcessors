//
// Created by Myra Norton on 1/22/26.
//

#ifndef ELECTORSYNTH_DELAYMODULE_H
#define ELECTORSYNTH_DELAYMODULE_H


#include "leaf.h"
#include "defs.h"
#include "leaf-mempool.h"
#include "leaf-delay.h"

typedef enum {
    DelayEventWatchFlag,
    DelayTime,
	DelayHPCutoff,
	DelayLPCutoff,
	DelayFeedback,
    DelayGain,
    DelayNumParams
} DelParams;



//the actual frequency setter function
// typedef void (*tDelayInternalParamSetFunc)(void*, float);

typedef struct _tDelayModule {
    ModuleHeader header;
    tTapeDelay* theDelay;
    tSVF lp;
    tSVF hp;
    tHighpass shaperHp;
    tFeedbackLeveler feedbackControl;
    float fbSample;
    float fbAmount;
    float cutoff1;
    float cutoff2;
    float amp;
    float sr;
    float invSr;


    tLookupTable* gainAmpTable;
    tMempool* mempool;
} _tDelayModule;

typedef _tDelayModule* tDelayModule;

//init module
void tDelayModule_init(void** const delay, float* const params, float id, LEAF* const leaf);
void tDelayModule_initToPool(void** const delay, float* const params, float id, tMempool** const mempool);

void tDelayModule_free(void** const delay);

// tick
void tDelayModule_tick (tDelayModule const delay);

void tDelayModule_setParameter(tDelayModule const delay, DelParams param_type,float input);


void tDelayModule_setSampleRate (tDelayModule const delay, float sr);

#endif // ELECTORSYNTH_DELAYMODULE_H
