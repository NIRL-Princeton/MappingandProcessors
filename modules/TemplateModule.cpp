//
// Created by Myra Norton on 8/13/26.
//

#include "TemplateModule.h"
#include <assert.h>
#include "string.h"

#if 0
// TODO: Change every instance of "Template" in this file to the capitalized name of your module
// TODO: Change every instance of "template" and "template_" in this file to the name of your module (not capitalized)

/* init() ensures that the resTable has been initialized and calls initToPool() with the mempool
 * associated with the LEAF instance passed into it. No changes needed here. */
void tTemplateModule_init(void** const template_, float* params, float id, LEAF* const leaf) {

    tTemplateModule_initToPool(template_, params, id, &leaf->mempool;
}

/* initToPool() are the functions that initialize your module. init() will call initToPool() with the default
 * values, but a user can call initToPool() directly to specify the mempool */
void tTemplateModule_initToPool(void** const template_, float* const params, float id, tMempool** const mempool)
{
    tMempool* m = *mempool;
    _tTemplateModule* TemplateModule =(_tTemplateModule *) ( *template_ = (_tTemplateModule*) mpool_alloc(sizeof(_tTemplateModule), m));
    #ifndef __cplusplus
        memcpy(TemplateModule->header.params, params, TemplateNumParams*sizeof(float));
    #endif

    // initialize attributes of the module
    TemplateModule->header.uniqueID = id;
    TemplateModule->mempool = m;
    TemplateModule->sr = m->leaf->sampleRate;
    TemplateModule->invSr = m->leaf->invSampleRate;
    // TODO: Add ModuleTypeTemplateModule to the ModuleType enum in defs.h
    TemplateModule->header.moduleType = ModuleTypeTemplateModule;

    // tTemplate_create() and tTemplate_init() are functions that should exist in LEAF for your module
    tTemplate_create(mempool, (tTemplate**)&TemplateModule->theTemplate);
    tTemplate_init(m->leaf,(tTemplate*)TemplateModule->theTemplate, 500.0f, 50000.f);

    #ifndef __cplusplus
        for (int i = 0; i < TemplateNumParams; i++)
        {
    	    tTemplateModule_setParameter(TemplateModule, i, TemplateModule->header.params[i]);

        }
    #endif
}

/* calls tTemplate_free(), which is a function that should exist in LEAF for your module*/
void tTemplateModule_free(void** const template_)
{
    _tTemplateModule* TemplateModule =(_tTemplateModule*) *template_;
    // tTemplate_free() and tTemplate should exist in LEAF for your module
    tTemplate_free((tTemplate**)TemplateModule->theTemplate);
    mpool_free((char*)TemplateModule, TemplateModule->mempool);

    // TODO: uncomment below if your module has multiple types
    //int type = roundf(CPPDEREF TemplateModule->header.params[TemplateType]);
}


/* sets the audio sample to what the value output from the module should be (computed each sample).  */
void tTemplateModule_tick (tTemplateModule const template_)
{
    const float input = template_->header.inputs[0]; //get any audio input coming into the module if you need it
    template_->header.inputs[0] = 0.0f; //set the input in the header to zero to prepare for next input sample to come in

    template_->header.outputs[0] = 0.0f; //tick a LEAF object like a tCycle or whatever and get the output//
}

/* private helper function for setting a parameter */
static void tTemplateModule_setParam1(tTemplateModule const template_, float const time) {
    // this is where you set the internal LEAF objects to have their real values, based on the 0-1 inputs from the parameters array
    //for instance, if it was freq and the module holds a sine wave, call tCycle_setFreq here.
}

/* changes the modules internal parameters when a user changes it */
void tTemplateModule_setParameter(tTemplateModule const template_, TemplateParams param_type,float input)
{
    switch (param_type) {
        //TODO: create a case for each param type. Create a helper function as shown above.

        // case TemplateParam1:
        //     tTemplateModule_setParam1(template_, input);
        //     break;
        // case TemplateParam2:
        //     tTemplateModule_setParam2(template_, input);
        //     break;
        default:
            break;
    }
}

/* calls the relevant LEAF function for setting the sample rate  */
void tTemplateModule_setSampleRate (tTemplateModule const template_, float const sr)
{
    //tCycle_setSampleRate(template_->template_s[0], sr);
}
#endif


//TODO: include your .h file in funcmaps.h
//TODO: include your init function in funcmaps.c
//TODO: make sure your type name is in defs.h
//TODO: for the C++ code in JUCE project, add your parameters to the array in parameterArrays.h
//TODO: for the C++ code in JUCE project, create a TemplateModuleProcessor.cpp and TemplateModuleProcessor.h in either Processors or Modulators and
//fill that info in
//TODO: for the C++ code in JUCE project, add it to ModuleList.cpp (include it's processor.h file and add it to the list where it belongs)
//TODO: for the C++ code in JUCE project, add it to the ModulationModuleSection or SoundModuleSection, so it shows up in the dropdown