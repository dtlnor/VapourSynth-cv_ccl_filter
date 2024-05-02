# VapourSynth-cv_ccl_filter
Vapoursynth Connected Components Label Filtering from OpenCV.

## Description

Using OpenCV's [`connectedComponentsWithStats`](https://docs.opencv.org/5.x/d3/dc0/group__imgproc__shape.html#ga107a78bf7cd25dec05fb4dfc5c9e765f) to compute the connected components labeled image of a boolean image and also get statistics for each label. Then filter the labeled content based on the statistical results.

Require VS API >= 4.0

## Usage
Exclude the labels **above** `ccl_thr`
```
core.cv_ccl.ExcludeCCLAbove(clip mask, int cc_thr[, int connectivity, int ccl_type, int cc_stat_type])
```
Exclude the labels **under** `ccl_thr`
```
core.cv_ccl.ExcludeCCLUnder(clip mask, int cc_thr[, int connectivity, int ccl_type, int cc_stat_type])
```
Read all the stats from `cv::connectedComponentsWithStats` to FrameProps  
```
core.cv_ccl.GetCCLStats(clip mask[, int connectivity, int ccl_type])
```

For example, if you want to remove all the connected components smaller than 2500 pixels (in area) inside your mask clip, you can simply write:
```python
core.cv_ccl.ExcludeCCLUnder(mask, 2500)
```

<details>
<summary>Sample</summary>

`Original`  
![image](https://github.com/dtlnor/VapourSynth-cv_ccl_filter/assets/21131439/cafc05ca-d759-412c-a791-a15ea13a17f8)

`ExcludeCCLUnder 2500`  
![image](https://github.com/dtlnor/VapourSynth-cv_ccl_filter/assets/21131439/566c7370-b0ec-4930-a11b-2673b3f98f53)

`ExcludeCCLAbove 2500`  
![image](https://github.com/dtlnor/VapourSynth-cv_ccl_filter/assets/21131439/7141aeb2-5915-4600-90b9-fadc4100ffc2)

</details>

If you want to get all the ccl stats data, using `core.cv_ccl.GetCCLStats`, you will get FrameProps like:

```
_CCLStatNumLabels
_CCLStatAreas
_CCLStatLefts
_CCLStatTops
_CCLStatWidths
_CCLStatHeights
_CCLStatCentroids_x
_CCLStatCentroids_y
```

which should contains array (except `_CCLStatNumLabels`) if the input clip isn't a blank clip. Note that the first label is always the background label, so you will like to get the frame prop like this
```py
f.props.get('_CCLStatAreas', None)[1:]
```

***Parameters:***

- **mask**

    Clip to process. Binarize `Gray8` clip only.

- **cc_thr**

    Use to compare with the results of statistics with `cc_stat_type`.

- **connectivity**

    8 or 4 for 8-way or 4-way connectivity respectively. Exact param in `cv::connectedComponentsWithStats`.  
    Default `8`

- **ccl_type**

    [cv::ConnectedComponentsAlgorithmsTypes](https://docs.opencv.org/5.x/d3/dc0/group__imgproc__shape.html#ga5ed7784614678adccb699c70fb841075)  
    connected components algorithm  
    - `0` - SAUF algorithm  
    - `1` - BBDT algorithm  
    - `2` - Spaghetti algorithm

    Default `-1` Auto (Spaghetti algorithm)

- **cc_stat_type**

    [cv::ConnectedComponentsTypes](https://docs.opencv.org/5.x/d3/dc0/group__imgproc__shape.html#gac7099124c0390051c6970a987e7dc5c5)  
    connected components statistics
    - `0` - The leftmost (x) coordinate which is the inclusive start of the bounding box in the horizontal direction.
    - `1` - The topmost (y) coordinate which is the inclusive start of the bounding box in the vertical direction.
    - `2` - The horizontal size of the bounding box.
    - `3` - The vertical size of the bounding box.
    - `4` - The total area (in pixels) of the connected component.

    Default `4` Area.


 


