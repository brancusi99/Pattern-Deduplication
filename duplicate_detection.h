#ifndef DUPLICATE_DETECTION_H
#define DUPLICATE_DETECTION_H

#include <opencv2/opencv.hpp>

using namespace cv;

// Diferente in doua blocuri (MSE)
double compareBlocks(const Mat& img1, const Mat& img2, int x, int y, int blockSize);

// Gaseste zone duplicate
Mat findDuplicateRegions(const Mat& img1, const Mat& img2, int blockSize, double threshold);

#endif
