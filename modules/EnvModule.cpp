//
// Created by Jeffrey Snyder on 7/9/24.
//

#include "EnvModule.h"
//#include <cstdio>

#include <assert.h>
//#include <iostream>
#include "string.h"
void tEnvModule_init (void** const env, float* params, float id, LEAF* const leaf)
{
    tEnvModule_initToPool (env, params, id, &leaf->mempool);
}

void tEnvModule_free (void** const env)
{
    _tEnvModule* EnvModule = (_tEnvModule*) (*env);
    mpool_free ((char*) EnvModule, EnvModule->mempool);
}
//tick function
void tEnvModule_tick (tEnvModule const env)
{
    //const float input = noise->header.summedInput + buffer[0];
    env->header.inputs[0] = 0.0f;
    CPPDEREF env->header.params[EnvVelocitySense] = tSlopeRamp_tick(&env->velSenseSmoother);
    env->header.outputs[0] = tADSRT_tick (&env->theEnv);
}

//special noteOnFunction
void tEnvModule_onNoteOn (tEnvModule const env, float velocity)
{
    float envVel = velocity;
    if (velocity > 0.005f)
    {
        //velocity sensitivity from mystran on KVR forum
        // this would be a parameter from 0 to some maximum
        // I just tried 20 and it feels nice
        float velSense = CPPDEREF env->header.params[EnvVelocitySense];
        float velocity_dB_scale = 40.0f * velSense;

        // map midi velocities [0, 127] to [0, 1] doubles
        // MIDI velocity 0 has already been converted to note off
        // but it doesn't really make any huge difference

        // velo_level is the constant to scale the note with
        float velo_level = powf(10.0f, velocity_dB_scale*(velocity - 0.7f) * 0.025f); //divide by 20 equals  * 0.05f

        tADSRT_on (&env->theEnv, velo_level);
    }
    else
    {
        tADSRT_off (&env->theEnv);
    }
}

// Non-modulatable setters
void tEnvModule_setExpTableLocation (tEnvModule const env, const float* tableAddress, uint32_t const tableSize)
{
    env->theEnv.exp_buff = tableAddress;
    env->theEnv.buff_size = tableSize;
    env->theEnv.buff_sizeMinusOne = tableSize - 1;
    env->theEnv.bufferSizeDividedBySampleRateInMs = env->theEnv.buff_size / (env->theEnv.sampleRate * 0.001f);
}



void tEnvModule_setSampleRate (tEnvModule const env, float sr)
{
    //how to handle this? if then cases for different types?
}
void tEnvModule_setParameter (tEnvModule const env, int parameter_id, float input)
{
    switch (parameter_id)
    {
        case EnvEventWatchFlag:
        {
            // handled by onnoteon listener
            break;
        }

        case EnvAttack:
        {
        	float const theValue = LEAF_interpolateTableLookupFunction(input, env->envTimeTable);
            tADSRT_setAttack (&env->theEnv, theValue);
            break;
        }

        case EnvDecay:
        {
        	 float const theValue = LEAF_interpolateTableLookupFunction(input, env->envTimeTable);
             tADSRT_setDecay (&env->theEnv, theValue);
            //tADSRT_setDecay (&env->theEnv, input);
            break;
        }

        case EnvSustain:
        {
            tADSRT_setSustain (&env->theEnv, input);
            break;
        }

        case EnvRelease:
        {
             //std::cout << "input: " << input;
        	 float const theValue = LEAF_interpolateTableLookupFunction(input, env->envTimeTable);
             //input *= env->envTimeTableSizeMinusOne;
             //int const inputInt = (int) input;
             //float const inputFloat = input - (float) inputInt;
             //int const nextPos = LEAF_clip (0.0f, inputInt + 1.0f, env->envTimeTableSizeMinusOne);
             //float const theValue = LEAF_clip (0.01f, (env->envTimeTableAddress[inputInt] * (1.0f - inputFloat)) + (env->envTimeTableAddress[nextPos] * inputFloat), 20000.0f);
           	 tADSRT_setRelease (&env->theEnv, theValue);
             //std::cout << " release milliseconds: " << std::to_string(theValue + 0.001f) << std::endl;
            //tADSRT_setRelease (&env->theEnv, input);
            break;
        }

        case EnvLeak:
        {
            tADSRT_setLeakFactor (&env->theEnv, 0.99995f + 0.00005f * (1.f - input));
            break;
        }

        case EnvShape:
        {
            tADSRT_setShape (&env->theEnv, input);
            break;
        }

        case EnvVelocitySense:
        {
            tSlopeRamp_setDest (&env->velSenseSmoother, input);
            break;
        }

        default:
            break;
    }
}

void tEnvModule_initToPool (void** const env, float* const params, float id, tMempool** const mempool)
{
    tMempool* m = *mempool;
    _tEnvModule* EnvModule = (_tEnvModule*) (*env = (_tEnvModule*) mpool_alloc (sizeof (_tEnvModule), m));
#ifndef __cplusplus
    memcpy (EnvModule->header.params, params, EnvNumParams * sizeof(float));
#endif __cplusplus
    EnvModule->mempool = m;

    EnvModule->header.uniqueID = id;


    if ((*mempool)->leaf->decayExpBuffer == NULL)
    {
		// exponential decay buffer falling from 1 to
    	tLookupTable_create (&m, &(*mempool)->leaf->decayExpBuffer);
    	tLookupTable_init ((*mempool)->leaf, (*mempool)->leaf->decayExpBuffer, 0.0001f, 20000.f, 4000.f, DECAY_EXP_BUFFER_SIZE);
		LEAF_generate_exp ((*mempool)->leaf->decayExpBuffer->table, 0.001f, 0.0f, 1.0f, -0.0008f, DECAY_EXP_BUFFER_SIZE);
    }
    tADSRT_set (&EnvModule->theEnv, 1.0f, 1000.0f, 1.0f, 1000.0f, (*mempool)->leaf->decayExpBuffer->table, DECAY_EXP_BUFFER_SIZE, (*mempool)->leaf);
	tADSRT_setSampleRate (&EnvModule->theEnv, m->leaf->sampleRate);
	//EnvModule->header.setterFunctions[EnvEventWatchFlag] = (tSetter) &tEnvModule_onNoteOn;
    if ((*mempool)->leaf->envTimeTable == NULL)
    {
        tLookupTable_create (&m, &(*mempool)->leaf->envTimeTable);
        tLookupTable_init ((*mempool)->leaf, (*mempool)->leaf->envTimeTable, 0.0001f, 20000.f, 4000.f, 2048);
    }
    EnvModule->envTimeTable = (*mempool)->leaf->envTimeTable;
    //tEnvModule_setTimeScalingTableLocation (EnvModule, (*mempool)->leaf->envTimeTable->table, 2048);

    tSlopeRamp_init(m->leaf, &EnvModule->velSenseSmoother, SMOOTH_SLOPE_MULTIPLIER, 1.f);
    EnvModule->header.moduleType = ModuleTypeEnvModule;
#ifndef __cplusplus
    for (int i = 0; i < EnvNumParams; i++)
    {
    	tEnvModule_setParameter(EnvModule, i, EnvModule->header.params[i]);

    }
#endif
}

