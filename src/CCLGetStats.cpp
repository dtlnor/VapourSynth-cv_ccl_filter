#include "shared.h"

static void process_ccl_stats(const VSFrame* src, VSFrame* dst, const MaskData* const VS_RESTRICT d, const VSAPI* vsapi) {
	const int w = vsapi->getFrameWidth(src, 0);
	const int h = vsapi->getFrameHeight(src, 0);
	ptrdiff_t stride = vsapi->getStride(src, 0);
	const uint8_t* maskp = vsapi->getReadPtr(src, 0);
	uint8_t* dstp = vsapi->getWritePtr(dst, 0);
	VSMap* props = vsapi->getFramePropertiesRW(dst);

	cv::Mat maskImg(h, w, CV_8UC1, (void*)maskp);
	cv::Mat labels, stats, centroids;

	auto num_labels = cv::connectedComponentsWithStats(maskImg, labels, stats, centroids, d->connectivity, CV_16U, d->ccl_type);
	
	// becareful with the number of labels, if there is only one label, it is the background label
	vsapi->mapSetInt(props, "_CCLStatNumLabels", num_labels, maReplace);

	// get the stats for each label, if there is only one label, it is the background label, and its data type will be a int/float.
	for (int label = 0; label < num_labels; label++) {

		auto area = stats.at<int>(label, cv::CC_STAT_AREA);
		auto left = stats.at<int>(label, cv::CC_STAT_LEFT);
		auto top = stats.at<int>(label, cv::CC_STAT_TOP);
		auto width = stats.at<int>(label, cv::CC_STAT_WIDTH);
		auto height = stats.at<int>(label, cv::CC_STAT_HEIGHT);
		// get the centroid at label
		auto centroidx = centroids.at<cv::Point2d>(label).x;
		auto centroidy = centroids.at<cv::Point2d>(label).y;

		vsapi->mapSetInt(props, "_CCLStatAreas", area, maAppend);
		vsapi->mapSetInt(props, "_CCLStatLefts", left, maAppend);
		vsapi->mapSetInt(props, "_CCLStatTops", top, maAppend);
		vsapi->mapSetInt(props, "_CCLStatWidths", width, maAppend);
		vsapi->mapSetInt(props, "_CCLStatHeights", height, maAppend);
		vsapi->mapSetFloat(props, "_CCLStatCentroids_x", centroidx, maAppend);
		vsapi->mapSetFloat(props, "_CCLStatCentroids_y", centroidy, maAppend);
	}

	for (int y = 0; y < h; y++) {
		memcpy(dstp, maskImg.ptr(y), w);
		dstp += stride;
	}
}

static const VSFrame* VS_CC getCCLStatsGetFrame(int n, int activationReason, void* instanceData, void** frameData, VSFrameContext* frameCtx, VSCore* core, const VSAPI* vsapi) {
	auto d{ static_cast<MaskData*>(instanceData) };

	if (activationReason == arInitial) {
		vsapi->requestFrameFilter(n, d->node, frameCtx);
	}
	else if (activationReason == arAllFramesReady) {
		const VSFrame* src = vsapi->getFrameFilter(n, d->node, frameCtx);

		const VSVideoFormat* fi = vsapi->getVideoFrameFormat(src);
		int height = vsapi->getFrameHeight(src, 0);
		int width = vsapi->getFrameWidth(src, 0);
		VSFrame* dst = vsapi->newVideoFrame(fi, width, height, src, core);

		if (d->vi->format.colorFamily == cfGray) {
			process_ccl_stats(src, dst, d, vsapi);
		}

		vsapi->freeFrame(src);
		return dst;
	}
	return nullptr;
}

static void VS_CC getCCLStatsMapFree(void* instanceData, VSCore* core, const VSAPI* vsapi) {
	auto d{ static_cast<MaskData*>(instanceData) };
	vsapi->freeNode(d->node);
	delete d;
}

void VS_CC getCCLStatsCreate(const VSMap* in, VSMap* out, void* userData, VSCore* core, const VSAPI* vsapi) {
	auto d{ std::make_unique<MaskData>() };
	int err{ 0 };

	d->node = vsapi->mapGetNode(in, "mask", 0, nullptr);
	d->vi = vsapi->getVideoInfo(d->node);

	auto ccl_type = vsapi->mapGetIntSaturated(in, "ccl_type", 0, &err);
	if (err)
		d->ccl_type = cv::CCL_DEFAULT;
	else {
		if (ccl_type < -1 || ccl_type > 5) {
			vsapi->mapSetError(out, "GetCCLStats: ccl_type must be between -1 and 6.");
			vsapi->freeNode(d->node);
			return;
		}
		d->ccl_type = ccl_type;
	}

	auto connectivity = vsapi->mapGetIntSaturated(in, "connectivity", 0, &err);
	if (err)
		d->connectivity = 8;
	else {
		if (connectivity != 4 && connectivity != 8) {
			vsapi->mapSetError(out, "GetCCLStats: connectivity must be 4 or 8.");
			vsapi->freeNode(d->node);
			return;
		}
		d->connectivity = connectivity;
	}

	if (d->vi->format.bytesPerSample != 1 || (d->vi->format.colorFamily != cfGray)) {
		vsapi->mapSetError(out, "GetCCLStats: only Gray8 formats supported.");
		vsapi->freeNode(d->node);
		return;
	}

	VSFilterDependency deps[]{ {d->node, rpGeneral} };
	vsapi->createVideoFilter(out, "GetCCLStats", d->vi, getCCLStatsGetFrame, getCCLStatsMapFree, fmParallel, deps, 1, d.get(), core);
	d.release();
}
