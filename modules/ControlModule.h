//
// Created by Jeff Snyder on 11/15/26.
//

#ifndef CONTROLMODULE_H
#define CONTROLMODULE_H

#include "leaf.h"
#include "defs.h"
// #include "processor.h"
#include "leaf-mempool.h"

//typedef void (*tFreqSetFunc)(void*, float);
typedef enum {
    ControlEventWatchFlag,
    ControlCCParam,
    ControlCCValue,
    ControlNumParams
} ControlParams;


typedef struct _tControlModule {
    ModuleHeader header;
    uint8_t ccNum;
    float ccValue;
    tExpSmooth smooth;
   tMempool* mempool;
} _tControlModule;

typedef _tControlModule* tControlModule;

//init module
void tControlModule_init(void** const ctl, float* const params, float id, LEAF* const leaf);
void tControlModule_initToPool(void** const ctl, float* const params, float id, tMempool** const mempool);
void tControlModule_free(void** const ctl);


// Modulatable setters
void tControlModule_setCCNum (tControlModule const ctl, float ccNum);

void tControlModule_setParameter(tControlModule const ctl, int param_type, float input);

// Non-modulatable setters
//void tLFOModule_setRateTableLocationAndSize (tLFOModule const lfo, float* tableAddress, uint32_t size);
void tControlModule_setSampleRate (tControlModule const ctl, float sr);

void tControlModule_tick (tControlModule const ctl);




#endif //CONTROLMODULE_H
