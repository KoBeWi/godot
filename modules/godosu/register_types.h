/* register_types.h */

#ifndef GODOSU_REGISTER_TYPES_H
#define GODOSU_REGISTER_TYPES_H

#include "modules/register_module_types.h"

void initialize_godosu_module(ModuleInitializationLevel p_level);
void uninitialize_godosu_module(ModuleInitializationLevel p_level);
/* yes, the word in the middle must be the same as the module folder name */

#endif
