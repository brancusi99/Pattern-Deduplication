#ifndef MERGE_UTILS_H
#define MERGE_UTILS_H

#include <opencv2/opencv.hpp>
#include <vector>

cv::Mat buildImage(
    const std::vector<cv::Mat>& originals,
    const std::vector<cv::Mat>& duplicates,
    int blockDim
);

#endif