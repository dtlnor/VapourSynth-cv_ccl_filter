#include "shared.h"

VS_EXTERNAL_API(void) VapourSynthPluginInit2(VSPlugin* plugin, const VSPLUGINAPI* vspapi) {
	vspapi->configPlugin("com.dtlnor.cv_ccl", "cv_ccl", "Mask connected components label filtering", VS_MAKE_VERSION(1, 0), VAPOURSYNTH_API_VERSION, 0, plugin);
	vspapi->registerFunction("ExcludeCCLAbove", "mask:vnode;cc_thr:int;connectivity:int:opt;ccl_type:int:opt;cc_stat_type:int:opt;", "clip:vnode;", excludeCCLAboveCreate, nullptr, plugin);
	vspapi->registerFunction("ExcludeCCLUnder", "mask:vnode;cc_thr:int;connectivity:int:opt;ccl_type:int:opt;cc_stat_type:int:opt;", "clip:vnode;", excludeCCLUnderCreate, nullptr, plugin);
}
