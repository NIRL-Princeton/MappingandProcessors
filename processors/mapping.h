/**********************************************************************/
/*                           Mapping header                           */
/*               Authors: Davis Polito and Joshua Warner              */
/**********************************************************************/

#ifndef MAPPING_HEADER
#define MAPPING_HEADER


#include <stdint.h>
#include "leaf-mempool.h"

#include "defs.h"
    #ifdef __cplusplus
    #include <cstdio>
namespace leaf
{
    #endif

    typedef struct AudioRouting {
        ATOMIC_FLOAT* audio;
    } tAudioRouting;


    typedef struct Mapping
    {
        // ModuleType dest_type;
        // tSetter setter; // Setter function for the parameter of the mapping
        uint8_t uuid; //ID for the mapping's parameter
        uint8_t index;

        // destination (output)
        void* destObject; // OUT destination for the mapping
        uint8_t paramID;
        uint8_t destinationProcessorUniqueID; // ID for destination processors that param is in
        ATOMIC_FLOAT* initialVal; // The mapping's initial value

        // source (input)
        uint8_t numUsedSources; // Number of active sources for the mapping
        uint8_t inUUIDS[MAX_NUM_SOURCES];
        ATOMIC_FLOAT* inSources[MAX_NUM_SOURCES]; // IN sources from processors
        ATOMIC_FLOAT CPPDEREF scalingValues[MAX_NUM_SOURCES]; // Scaling for the IN sources
        ATOMIC_FLOAT bipolarOffset[MAX_NUM_SOURCES];

        tMempool *mempool;
    } tMapping;


    typedef struct MappingPreset
    {
        uint8_t index;
        uint8_t uuid; // ID for this processors
        uint8_t destinationUUID;
        uint8_t destParamID;
        uint8_t inUUID;
        float bipolarOffset;
        float scalingValues;

    } tMappingPreset;

    typedef union {
        tMappingPreset data;
        uint8_t bytes[sizeof (tMappingPreset)];
    } tMappingPresetUnion;


    typedef struct tModuleSmall
    {
        uint8_t active;
        uint8_t type;
        uint16_t uuid;
        uint8_t terminal;
        uint8_t chainEnd;
        float params[MAX_NUM_PARAMS];
    } tModuleSmall;

    typedef struct tMappingSmall
    {
        uint8_t active;
        uint16_t mapUuid;
        uint16_t sUuid;
        uint16_t dUuid;
        uint8_t dParamId;
        float scalar;
        float bipolarOffset;
    }tMappingSmall;

    typedef struct tAudioConnectionSmall
    {
        uint8_t active;
        uint16_t sUuid;
        uint16_t dUuid;
        float amount;
    } tAudioConnectionSmall;


    typedef struct tAudioConnectionHardware
    {
        float* sourcePointer;
        float* destinationPointer;
        float amount;
        uint8_t chainExit;
        uint8_t terminal;
    } tAudioConnectionHardware;


//representation of full state of preset
    typedef struct tFullPresetState {
        //modules
        tModuleSmall master;
        tModuleSmall modules[MAX_NUM_MODULES];
        //how many modules total
            //for each
                //module type
                //module id
                //params array
        //mappings
        tMappingSmall mappings[MAX_NUM_MAPPINGS];

        //audioConnections
        tAudioConnectionSmall routing[MAX_NUM_AUDIO_CONNECTIONS];

        //chainExitModulesList
            //
        //terminalAudio
            //

    } tFullPresetState;

    typedef union {
        tFullPresetState data;
        uint8_t bytes[sizeof (tFullPresetState)];
    } tFullPresetStateUnion;

    typedef struct tFullPresetStateWithNameAndNumber {
        //modules
        uint8_t presetVersionNumberMajor;
        uint8_t presetVersionNumberMinor;
        uint8_t presetNumber;
        uint8_t name[14];
        tFullPresetState fullstate;
    } tFullPresetStateWithNameAndNumber;

    typedef union tFullPresetStateWithNameAndNumberUnion {
        //modules
        tFullPresetStateWithNameAndNumber data;
        uint8_t bytes[sizeof (tFullPresetStateWithNameAndNumber)];
    } tFullPresetStateWithNameAndNumberUnion;


// 7-Bit representation of tMappingPreset
    typedef struct tMappingPreset7Bit {
        uint8_t index[2];                        // 2 chunks of 7 bits
        uint8_t uuid[2];                         // 2 chunks of 7 bits
        uint8_t destinationUUID[2];              // 2 chunks of 7 bits
        uint8_t destParamID[2];                  // 2 chunks of 7 bits
        uint8_t numUsedSources[2];               // 2 chunks of 7 bits
        uint8_t inUUIDs[MAX_NUM_SOURCES][2];     // Each UUID -> 2 chunks of 7 bits
        uint8_t bipolarOffset[MAX_NUM_SOURCES][5]; // Each float -> 5 chunks of 7 bits
        uint8_t scalingValues[MAX_NUM_SOURCES][5]; // Each float -> 5 chunks of 7 bits
    } tMappingPreset7Bit;


    typedef struct  _tMappingReceiver{
       uint8_t receivedData[sizeof(tMappingPreset7Bit)]; //
        size_t receivedDataSize;
    }tMappingReceiver;

    void splitMappingPreset(const tMappingPreset* preset, tMappingPreset7Bit* preset7Bit);
    void unsplitMappingPreset(const tMappingPreset7Bit* preset7Bit, tMappingPreset* preset);
    void mapping_to_preset(tMapping *mapping, tMappingPresetUnion * preset);
    
    void preset_to_mapping(tMappingPresetUnion preset, tMapping *mapping);
    void preset_to_mapping_(const tMappingPreset *preset, tMapping *mapping);
    void processMapping (tMapping* mapping);

    void tMapping_init (tMapping** const mapping, LEAF* const leaf);
    void tMapping_free (tMapping** const mapping);

    void tMapping_setParameter(void* module, int paramID, float val);
    void tMapping_initToPool (tMapping** const mapping, tMempool** const mp);
    // void tMappingAdd (tMapping* mapping, tProcessor* outputProcessor, tProcessor* destProcessor, uint8_t destParam,
    //     uint8_t source, LEAF *leaf, ATOMIC_FLOAT CPPDEREF scalingptr);
    // void tMappingUpdateDest(tMapping* mapping, uint8_t source,
    //      tProcessor *newDestProcessor, uint8_t destParam,
    //     ATOMIC_FLOAT CPPDEREF scalingValue);
    void tMappingAdd_(tMapping *mapping, ATOMIC_FLOAT* insource, uint8_t insource_uuid,
        ATOMIC_FLOAT* dest_param, uint8_t dest_uuid, uint8_t dest_param_index,
        void* obj, LEAF* leaf,
    ATOMIC_FLOAT CPPDEREF scalingValue);

#ifdef __cplusplus

}
#endif




#endif
