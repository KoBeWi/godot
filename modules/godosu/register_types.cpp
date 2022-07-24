/* register_types.cpp */

#include "register_types.h"

#include "core/object/class_db.h"

#include "gosu.h"

void initialize_godosu_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}

	ClassDB::register_class<Gosu>();
}

void uninitialize_godosu_module(ModuleInitializationLevel p_level) {
}
