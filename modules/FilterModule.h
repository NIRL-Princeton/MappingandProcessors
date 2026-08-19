//
// Created by Joshua Warner on 6/13/24.
//
#ifndef FILTERMODULE_H
#define FILTERMODULE_H

#include "leaf.h"
#include "defs.h"
#include "leaf-mempool.h"
#include "leaf-filters.h"
#include "leaf-envelopes.h"


typedef enum {
    FiltEventWatchFlag,
    FiltMidiPitch,
    FiltCutoff,
    FiltGain,
    FiltResonance,
    FiltKeyfollow,
    FiltType,
    FiltMix,
    FiltNumParams
} FiltParams;

typedef enum FlagFiltTypes{
    FiltTypeLowpass = 1,
    FiltTypeHighpass = 2,
    FiltTypeBandpass = 4,
    FiltTypeDiodeLowpass = 8,
    FiltTypePeak = 16,
    FiltTypeHighShelf = 32,
    FiltTypeLowShelf = 64,
    FiltTypeNotch = 128,
    FiltTypeLadderLowpass = 256
};

//the actual frequency setter function
//typedef void (*tFiltInternalParamSetFunc)(void*, float);

typedef struct _tFiltModule {
    ModuleHeader header;
    //void* theFilt;
    //void* filters[FiltNumTypes];

    tSVF lowPassFilter;
    tSVF highPassFilter;
    tSVF bandPassFilter;
    tDiodeFilter diodeFilter;
    tVZFilterBell bellFilter;
    tVZFilterHS highShelfFilter;
    tVZFilterLS lowShelfFilter;
    tVZFilterBR notchFilter;
    tLadderFilter ladderFilter;

    float* dbTableAddress;
    uint32_t dbTableScalar;
    float dbTableOffset;
    float dbTableSizeMinusOne;
    float* resTableAddress;
    float resTableSizeMinusOne;
    uint16_t filtType;

    float gainKnob; // from Gabe
    float gain;
    tSlopeRamp gainSmoother;

    float keyFollow;
    tSlopeRamp keyFollowSmoother;

    float cutoffKnob; // from Gabe
    tSlopeRamp cutoffSmoother;
    float inputNote; // raw midi input
    float currFreq;

    float qValue;
    float resonanceKnob; // from Gave
    tSlopeRamp qSmoother;

    float mix;
    tSlopeRamp mixSmoother;

    float sr;
    float invSr;

    tMempool* mempool;
    tLookupTable* resTable;
    tLookupTable* mtofTable;
    tLookupTable* skewFreqTable;
    tLookupTable* gainAmpTable;
} _tFiltModule;

typedef _tFiltModule* tFiltModule;

//init module
void tFiltModule_init(void** const filt, float* const params, float id, LEAF* const leaf);
void tFiltModule_initToPool(void** const filt, float* const params, float id, tMempool** const mempool, tLookupTable* resTable);

void tFiltModule_free(void** const filt);
float dbToATableLookupFunction(float const in, float const sizeMinusOne, float* const tableAddress);

// tick
void tFiltModule_tick (tFiltModule const filt, float*);

void tFiltModule_setParameter(tFiltModule const filt, FiltParams param_type,float input);
void tFiltModule_setType(tFiltModule const filt, uint16_t type);

//Modulatable setters
//void tFiltModule_setMIDIPitch (tFiltModule const filt, float const input);
void tFiltModule_setFreq(tFiltModule const filt, float freqInput);
void tFiltModule_setGain(tFiltModule const filt, float gain);
void tFiltModule_setKeyFollow(tFiltModule const filt, float keyFollow);
void tFiltModule_setMix(tFiltModule const filt, float mix);

// Non-modulatable setters

//void tFiltModule_setDBtoATableLocation (tFiltModule const filt, float* tableAddress, uint32_t tableSize);
void tFiltModule_setResTableLocation (tFiltModule const filt, float* tableAddress, uint32_t tableSize);
void tFiltModule_setSampleRate (tFiltModule const filt, float sr);




#endif //FILTERMODULE_H
