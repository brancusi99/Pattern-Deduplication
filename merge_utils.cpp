#include "merge_utils.h"
#include <iostream>

cv::Mat buildImage(
    const std::vector<cv::Mat>& original,
    const std::vector<cv::Mat>& duplicates,
    int blockDim
) {
    if (original.empty()) return cv::Mat();

    cv::Mat result= original[0].clone();


    for (size_t i = 1; i < original.size(); ++i) {
        const cv::Mat& currentImg = original[i];
        const cv::Mat& mask = duplicates[i];

        for (int y = 0; y < currentImg.rows; y += blockDim) {
            for (int x = 0; x < currentImg.cols; x += blockDim) {

                cv::Rect region(x, y, blockDim, blockDim);

                if (x + blockDim > currentImg.cols) region.width = currentImg.cols - x;
                if (y + blockDim > currentImg.rows) region.height = currentImg.rows - y;

                cv::Scalar mediaMask = cv::mean(mask(region));

                if (mediaMask[0] < 127) {
                    currentImg(region).copyTo(result(region));
                }
            }
        }
    }
    return result;
}