#include "shared.h"

static void process_ccl_above(const VSFrame* src, VSFrame* dst, const MaskData* const VS_RESTRICT d, const VSAPI* vsapi) {
	const int w = vsapi->getFrameWidth(src, 0);
	const int h = vsapi->getFrameHeight(src, 0);
	ptrdiff_t stride = vsapi->getStride(src, 0);
	const uint8_t* maskp = vsapi->getReadPtr(src, 0);
	uint8_t* dstp = vsapi->getWritePtr(dst, 0);

	cv::Mat maskImg(h, w, CV_8UC1, (void*)maskp);
	cv::Mat dstImg{ maskImg.clone() };
	cv::Mat labels, stats, centroids;

	auto num_labels = cv::connectedComponentsWithStats(maskImg, labels, stats, centroids, d->connectivity, CV_16U, d->ccl_type);

	for (int label = 1; label < num_labels; label++) {
		auto area = stats.at<int>(label, d->cc_stat_type);
		if (area > d->cc_thr) {
			cv::Mat mask = labels == label;
			dstImg.setTo(0, mask);
		}
	}

	for (int y = 0; y < h; y++) {
		memcpy(dstp, dstImg.ptr(y), w);
		dstp += stride;
	}
}

static const VSFrame* VS_CC excludeCCLAboveGetFrame(int n, int activationReason, void* instanceData, void** frameData, VSFrameContext* frameCtx, VSCore* core, const VSAPI* vsapi) {
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
			process_ccl_above(src, dst, d, vsapi);
		}

		vsapi->freeFrame(src);
		return dst;
	}
	return nullptr;
}

static void VS_CC excludeCCLAboveMapFree(void* instanceData, VSCore* core, const VSAPI* vsapi) {
	auto d{ static_cast<MaskData*>(instanceData) };
	vsapi->freeNode(d->node);
	delete d;
}

void VS_CC excludeCCLAboveCreate(const VSMap* in, VSMap* out, void* userData, VSCore* core, const VSAPI* vsapi) {
	auto d{ std::make_unique<MaskData>() };
	int err{ 0 };

	d->node = vsapi->mapGetNode(in, "mask", 0, nullptr);
	d->vi = vsapi->getVideoInfo(d->node);
	d->cc_thr = vsapi->mapGetIntSaturated(in, "cc_thr", 0, &err);
	if (err) {
		vsapi->mapSetError(out, "ExcludeCCLAbove: cc_thr must be specified.");
		vsapi->freeNode(d->node);
		return;
	}

	auto ccl_type = vsapi->mapGetIntSaturated(in, "ccl_type", 0, &err);
	if (err)
		d->ccl_type = cv::CCL_DEFAULT;
	else {
		if (ccl_type < -1 || ccl_type > 5) {
			vsapi->mapSetError(out, "ExcludeCCLAbove: ccl_type must be between -1 and 5.");
			vsapi->freeNode(d->node);
			return;
		}
		d->ccl_type = ccl_type;
	}

	auto cc_stat_type = vsapi->mapGetIntSaturated(in, "cc_stat_type", 0, &err);
	if (err)
		d->cc_stat_type = cv::CC_STAT_AREA;
	else {
		if (cc_stat_type < 0 || cc_stat_type > 4) {
			vsapi->mapSetError(out, "ExcludeCCLAbove: cc_stat_type must be between 0 and 4.");
			vsapi->freeNode(d->node);
			return;
		}
		d->cc_stat_type = cc_stat_type;
	}

	auto connectivity = vsapi->mapGetIntSaturated(in, "connectivity", 0, &err);
	if (err)
		d->connectivity = 8;
	else {
		if (connectivity != 4 && connectivity != 8) {
			vsapi->mapSetError(out, "ExcludeCCLAbove: connectivity must be 4 or 8.");
			vsapi->freeNode(d->node);
			return;
		}
		d->connectivity = connectivity;
	}

	if (d->vi->format.bytesPerSample != 1 || (d->vi->format.colorFamily != cfGray)) {
		vsapi->mapSetError(out, "ExcludeCCLAbove: only Gray8 formats supported.");
		vsapi->freeNode(d->node);
		return;
	}

	VSFilterDependency deps[]{ {d->node, rpGeneral} };
	vsapi->createVideoFilter(out, "ExcludeCCLAbove", d->vi, excludeCCLAboveGetFrame, excludeCCLAboveMapFree, fmParallel, deps, 1, d.get(), core);
	d.release();
}
