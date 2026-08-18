//
// Created by Joshua Warner on 6/13/24.
//

#ifndef SIMPLEOSCMODULE_H
#define SIMPLEOSCMODULE_H
#include "leaf.h"
#include "defs.h"
// #include "processor.h"
#include "leaf-mempool.h"
#include "leaf-oscillators.h"
#include "leaf-envelopes.h"

typedef enum {
    OscEventWatchFlag,
    OscMidiPitch,
    OscHarmonic,
    OscPitchOffset,
    OscPitchFine,
    OscFreqOffset,
    OscShapeParam,
    OscAmpParam,
    OscGlide,
    OscSteppedHarmonic,
    OscSteppedPitch,
    OscSyncMode,
    OscSyncIn,
    OscType,
    OscPortaType,
    OscNumParams
} OscParams;

// typedef enum {
//     OscTypeSawSquare,
//     OscTypeSineTri,
//     OscTypeSaw,
//     OscTypePulse,
//     OscTypeSine,
//     OscTypeTri,
//     OscNumTypes
// } OscTypes;

typedef enum FlagOscTypes{
    OscTypeSawSquare = 1,
    OscTypeSineTri = 2,
    OscTypeSaw = 4,
    OscTypePulse = 8,
    OscTypeSine = 16,
    OscTypeTri = 32
};

typedef struct _tOscModule {
    ModuleHeader header;

    tPBSawSquare sawSquareOsc;
    tPBSineTriangle sineTriangleOsc;
    tPBSaw sawOsc;
    tPBPulse squareOsc;
    tCycle sineOsc;
    tPBTriangle triOsc;
    uint8_t oscType;

    float fine;
    float harmonicMultiplier;
    float pitchOffset;
    float freqOffset;
    tRamp pitchSmooth;
    float inputGlideTime;
    float octaveOffset;
    float inputNote;
    float finalFreq;
    float amp;
    tSlopeRamp ampSmoother;
    float sr;
    float invSr;
    int hStepped;
    int pStepped;
    int syncMode;
    float oscShape;
    tSlopeRamp shapeSmoother;
    uint8_t portaType;
    uint8_t counter;

    tMempool* mempool;

    tLookupTable* mtofTable;
    //tLookupTable* ftomTable;
    //tLookupTable* dbtoaTable;
    //tLookupTable* atodbTable;

    tLookupTable* glideTimeTable;
} _tOscModule;

typedef _tOscModule* tOscModule;

//init module
void tOscModule_init(void** const osc, float* const params, float id, LEAF* const leaf);
void tOscModule_initToPool(void** const osc, float* const params, float id, tMempool** const mempool);
void tOscModule_free(void** const osc);
void tOscModule_setParameter(tOscModule const osc, OscParams param_type, float input);
// Modulatable setters
void tOscModule_setInputNote (tOscModule const osc, float inputNote);
void tOscModule_tick (tOscModule const osc, float*);


// Non-modulatable setters
void tOscModule_setMTOFTableLocation (tOscModule const osc, float* tableAddress);
void tOscModule_setSampleRate (tOscModule const osc, float sr);
void tOscModule_setType (tOscModule const osc, int type);
void tOscModule_setShape (tOscModule const osc, float shape);
void tOscModule_setGlideOrigin (tOscModule const osc, float originNote);
void tOscModule_onNoteOn (tOscModule const osc, float vel);



#endif //SIMPLEOSCMODULE_H
