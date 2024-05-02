#pragma once

#include <memory>
#include <vector>
#include <opencv2/opencv.hpp>
#include "VapourSynth4.h"
#include "VSHelper4.h"

struct MaskData final {
	VSNode* node;
	const VSVideoInfo* vi;
	int cc_thr;
	int connectivity;
	int ccl_type;
	int cc_stat_type;
};

extern void VS_CC excludeCCLAboveCreate(const VSMap* in, VSMap* out, void* userData, VSCore* core, const VSAPI* vsapi);
extern void VS_CC excludeCCLUnderCreate(const VSMap* in, VSMap* out, void* userData, VSCore* core, const VSAPI* vsapi);
extern void VS_CC getCCLStatsCreate(const VSMap* in, VSMap* out, void* userData, VSCore* core, const VSAPI* vsapi);
