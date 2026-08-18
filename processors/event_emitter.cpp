//
// Created by Davis Polito on 9/1/25.
//
#include "event_emitter.h"
#include "defs.h"
#include "EnvModule.h"
#include "StringModule.h"
#include "LFOModule.h"
#include "SimpleOscModule.h"

void callNoteOn(ModuleHeader* const module,  float velocity)
{

        if (!module) return;

        // Cast to base module type to access moduleType
        uint32_t type = module->moduleType; // assuming moduleType is the first member of all modules
        switch ((ModuleType)type)
        {
            case ModuleTypeOscModule:
            {
                // tOscModule* osc = (tOscModule*)module;
                // call osc-specific function, e.g.,
                tOscModule_onNoteOn((tOscModule)module, velocity);
                break;
            }
            case ModuleTypeLFOModule:
            {
                // tLFOModule* lfo = (tLFOModule*)module;
                tLFOModule_onNoteOn((tLFOModule)module, velocity);
                break;
            }
            case ModuleTypeEnvModule:
            {

                // call the function pointer stored in setterFunctions
                tEnvModule_onNoteOn((tEnvModule)module,velocity);
                break;
            }
            case ModuleTypeFilterModule:
            {
                // tFilterModule* filter = (tFilterModule*)module;
                // tFilterModule_onNoteOn(filter, velocity);
                break;
            }
            case ModuleTypeStringModule:
            {
                // tStringModule* str = (tStringModule*)module;
                 tStringModule_onNoteOn((tStringModule)module, velocity);
                break;
            }
            case ModuleTypeVCAModule:
            {
                // tVCAModule* vca = (tVCAModule*)module;
                // tVCAModule_onNoteOn(vca, velocity);
                break;
            }
            default:
                // unknown module type
                break;
        }
    }

void callNoteOff(ModuleHeader* const module,  float velocity)
{

    if (!module) return;

    // Cast to base module type to access moduleType
    uint32_t type = module->moduleType; // assuming moduleType is the first member of all modules
    switch ((ModuleType)type)
    {
        case ModuleTypeOscModule:
        {
            // tOscModule* osc = (tOscModule*)module;
            // call osc-specific function, e.g.,
            // tOscModule_onNoteOff(osc, velocity);
            break;
        }
        case ModuleTypeLFOModule:
        {
            // tLFOModule* lfo = (tLFOModule*)module;
            //tLFOModule_onNoteOnff((tLFOModule)module);
            break;
        }
        case ModuleTypeEnvModule:
        {

            // call the function pointer stored in setterFunctions
            //tEnvModule_onNoteOn((tEnvModule)module,velocity);
            break;
        }
        case ModuleTypeFilterModule:
        {
            // tFilterModule* filter = (tFilterModule*)module;
            // tFilterModule_onNoteOn(filter, velocity);
            break;
        }
        case ModuleTypeStringModule:
        {
            // tStringModule* str = (tStringModule*)module;
            //tStringModule_onNoteOn((tStringModule)module, velocity);
            break;
        }
        case ModuleTypeVCAModule:
        {
            // tVCAModule* vca = (tVCAModule*)module;
            // tVCAModule_onNoteOn(vca, velocity);
            break;
        }
        default:
            // unknown module type
            break;
    }
}
