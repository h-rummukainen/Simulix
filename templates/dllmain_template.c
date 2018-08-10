// Enables support to read resources folder in the FMU
#ifdef WIN32
# define FMUHASRESOURCES
#endif
#define FMUHASENTERINITIALIZATIONMODE

#include "{modelName}.h"
#include "rtwtypes.h"
#include <stdint.h>


// include fmu header files, typedefs and macros
#include "fmuTemplate.h"

//! System DLL base address
size_t dllBaseAddress_U32 = 0;
//! System DLL image size in bytes
size_t dllImageSize_U32 = 0;

static fmi2Real         current_time        = 0;
static uint32_t         {modelName}_time = 0;
static const fmi2Real   step_size           = {stepSize};
static fmi2Integer      simulation_ticks    = 0;

// define class name and unique id
#define MODEL_IDENTIFIER "{modelName}"
#define MODEL_GUID "{GUID}"

// define model size
#define NUMBER_OF_REALS                  {numReal}
#define NUMBER_OF_INTEGERS               {numInt}
#define NUMBER_OF_BOOLEANS               {numBoolean}
#define NUMBER_OF_STRINGS                0
#define NUMBER_OF_REALS_IN_MODEL         {numReal}
#define NUMBER_OF_INTEGERS_IN_MODEL      {numInt}
#define NUMBER_OF_BOOLEANS_IN_MODEL      {numBoolean}
#define NUMBER_OF_STRINGS_IN_MODEL       0
#define NUMBER_OF_STATES                 0
#define NUMBER_OF_EVENT_INDICATORS       0
#define STATES                           {{0}}


typedef struct {{
    enum type {{F32 ,F64 , S32 , S , B}} dataType;
    union {{
        void          * xxx;
        float         * F32;
        double        * F64;
        int32_t       * S32;
        fmi2Integer   * S;
        boolean_T     * B;
    }} ptr;
}} ptr_elem;

//data pointers for reals
ptr_elem    dataReal[ NUMBER_OF_REALS_IN_MODEL + 1 ]       = {{
    {realString}
    {{0, NULL}}
}};

//data pointers for integers
 
ptr_elem    dataInteger[ NUMBER_OF_INTEGERS_IN_MODEL + 1 ] = {{
    {{S, (void *)&simulation_ticks}},
	{intString}
    {{0, NULL}}
}};

//data pointers for booleans 
ptr_elem    dataBoolean[ NUMBER_OF_BOOLEANS_IN_MODEL + 1] = {{
    {booleanString}
    {{0, NULL}}
}};

// called by fmi2Instantiate
// Set values for all variables that define a start value
// Settings used unless changed by fmi2SetX before fmi2EnterInitializationMode
static void setStartValues(ModelInstance *comp) {{
    
   {modelName}_initialize();


}}

// called by fmi2EnterInitializationMode(), (which is called after fmi2Instantiate(), and after any initial fmi2SetX())
void enterInitializationMode(ModelInstance *comp) {{

}}


// called by fmi2GetReal, fmi2GetInteger, fmi2GetBoolean, fmi2GetString, fmi2ExitInitialization
// if setStartValues or environment set new values through fmi2SetXXX.
// Lazy set values for all variable that are computed from other variables.
static void calculateValues(ModelInstance *comp) {{
    if (comp->state == modelInitializationMode) {{
        // set first time event
        comp->eventInfo.nextEventTimeDefined = fmi2True;
        comp->eventInfo.nextEventTime        = step_size + comp->time;
    }}
}}

// called by fmi2GetReal, fmi2GetContinuousStates and fmi2GetDerivatives
static fmi2Real getReal(ModelInstance *comp, fmi2ValueReference vr)
{{
    if(vr < NUMBER_OF_REALS_IN_MODEL)
    {{
        if (dataReal[vr].dataType == F32) {{
        return (fmi2Real)*(dataReal[vr].ptr.F32);
        }}
        else {{
            return (fmi2Real)*(dataReal[vr].ptr.F64);
        }}
    }}
    else
    {{
        return comp->r[vr];
    }}    
}}

// called by fmi2SetReal
static void setReal(ModelInstance *comp, fmi2ValueReference vr , fmi2Real value)
{{
    if(vr < NUMBER_OF_REALS_IN_MODEL)
    {{
        if (dataReal[vr].dataType == F32) {{
            *(dataReal[vr].ptr.F32) = (float)value;
        }}
        else {{
            *(dataReal[vr].ptr.F64) = (double)value;
        }}
    }}
    else
    {{
        comp->r[vr] = value;
    }}
}}

// called by fmi2GetInteger
static fmi2Integer getInteger(ModelInstance *comp, fmi2ValueReference vr)
{{
    if(vr < NUMBER_OF_INTEGERS_IN_MODEL)
    {{
        if (dataReal[vr].dataType == S32) {{
            return (fmi2Integer)*(dataInteger[vr].ptr.S32);
        }}
        else {{
            return (fmi2Integer)*(dataInteger[vr].ptr.S);
        }}
    }}
    else
    {{
        return comp->i[vr];
    }}
}}

// called by fmi2SetInteger
static void setInteger(ModelInstance *comp, fmi2ValueReference vr , fmi2Integer value) 
{{
    if(vr < NUMBER_OF_INTEGERS_IN_MODEL)
    {{
        if (dataReal[vr].dataType == S32) {{
            *(dataInteger[vr].ptr.S32) = (int32_t)value; 
        }}
        else {{
            *(dataInteger[vr].ptr.S) = (int)value;
        }}
    }}
    else
    {{
        comp->i[vr] = value;
    }}
}}

// called by fmi2GetBoolean
static fmi2Boolean getBoolean(ModelInstance *comp, fmi2ValueReference vr)
{{
    if(vr < NUMBER_OF_BOOLEANS_IN_MODEL)
    {{
        return (fmi2Boolean)*dataBoolean[vr].ptr.B;
    }}
    else
    {{
        return comp->b[vr];
    }}
}}

// called by fmi2SetBoolean
static void setBoolean(ModelInstance *comp, fmi2ValueReference vr , fmi2Boolean value)
{{
    if(vr < NUMBER_OF_BOOLEANS_IN_MODEL)
    {{
        *dataBoolean[vr].ptr.B = (boolean_T)value;
    }}
    else
    {{
        comp->b[vr] = value;
    }}
}}

// used to set the next time event, if any.
static void eventUpdate(ModelInstance *comp, fmi2EventInfo *eventInfo, int isTimeEvent) {{
    static fmi2Boolean overrunFlag = fmi2False;
    fmi2Boolean error = fmi2False;
    int i;
    unsigned int log_address;
    
    if (isTimeEvent) {{
        while (current_time < (comp->time - 0.1 * step_size) )
        {{   
            {modelName}_step();
            current_time        += step_size;
            {modelName}_time += 1;         // Only for Vision/Testlogger that cannot handle double-presicion floats
            simulation_ticks    += 1;
        }}

        /* for (i = 0; i < number_of_log_variables; i++)
        {{
            double converted_value;
            int last_value;
            
            log_address = variables_to_log[i].address + (unsigned int)dllBaseAddress_U32;
        
            last_value = (number_of_log_variables - 1 == i) ? 1 : 0;
            ReadAndConvert(variables_to_log[i], log_address, &converted_value);
            logInternalVariable((double)current_time, converted_value, last_value);
        }} */
        eventInfo->nextEventTimeDefined = fmi2True;
        eventInfo->nextEventTime        = step_size + comp->time;
    }}
}}

// Terminated
static void Terminated()
{{
    {modelName}_terminate();
}}

#if defined(_WIN32) || defined(WIN32)

#ifndef _WIN64
//! Get the Dll image size in bytes
static size_t GetDllImageSize(void);
/*!
 *****************************************************************************
 * Get the Dll image size in bytes
 *
 * @returns Size in bytes
 ****************************************************************************/
static size_t GetDllImageSize(void)
{{
    IMAGE_DOS_HEADER *image_dos_header;
    IMAGE_NT_HEADERS *image_nt_headers;

    image_dos_header = (IMAGE_DOS_HEADER *)dllBaseAddress_U32;
    image_nt_headers = (IMAGE_NT_HEADERS *)(dllBaseAddress_U32 + (size_t)image_dos_header->e_lfanew);

    // Verify that we have loaded the correct headers
    if (image_dos_header->e_magic != IMAGE_DOS_SIGNATURE)
    {{
        return 0;
    }}
    if (image_nt_headers->Signature != IMAGE_NT_SIGNATURE)
    {{
        return 0;
    }}

    return (size_t)image_nt_headers->OptionalHeader.SizeOfImage;
}}
#endif

/*!
 *****************************************************************************
 * Main loop of the EcuDll
 *
 * Input data:
 *  @param hModule
 *  @param ul_reason_for_call
 *  @param lpReserved
 *
 * Output data:
 *  @returns TRUE on successful attach/detach
 ****************************************************************************/
BOOL WINAPI DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved )
{{
// The use of dllBaseAddress is only used for ccp/a2l and is not nesseccary when building 64bit fmu's
#ifndef _WIN64
    switch (ul_reason_for_call)
    {{
        case DLL_PROCESS_ATTACH:
            dllBaseAddress_U32 = (size_t)hModule;
            dllImageSize_U32 = GetDllImageSize();
            if (dllImageSize_U32 == 0)
            {{
                return FALSE;
            }}
            break;
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }}
#endif
    return TRUE;
}}

#endif

// include code that implements the FMI based on the above definitions
#include "fmuTemplate.c"
