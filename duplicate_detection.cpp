#include "duplicate_detection.h"

// MSE 
double compareBlocks(const Mat& img1, const Mat& img2, int x, int y, int blockSize) {
    double mse = 0.0;

    for (int j = 0; j < blockSize; j++) {
        for (int i = 0; i < blockSize; i++) {

            int px = x + i;
            int py = y + j;

            if (px >= img1.cols || py >= img1.rows) continue;

            int val1 = img1.at<uchar>(py, px);
            int val2 = img2.at<uchar>(py, px);

            int diff = val1 - val2;
            mse += diff * diff;
        }
    }

    return mse / (blockSize * blockSize);
}

// Gaseste Duplicate 
Mat findDuplicateRegions(const Mat& img1, const Mat& img2, int blockSize, double threshold) {

    Mat mask = Mat::zeros(img1.size(), CV_8UC1);

    for (int y = 0; y < img1.rows; y += blockSize) {
        for (int x = 0; x < img1.cols; x += blockSize) {

            double mse = compareBlocks(img1, img2, x, y, blockSize);

            if (mse < threshold) {
                for (int j = 0; j < blockSize; j++) {
                    for (int i = 0; i < blockSize; i++) {

                        int px = x + i;
                        int py = y + j;

                        if (px < img1.cols && py < img1.rows) {
                            mask.at<uchar>(py, px) = 255;
                        }
                    }
                }
            }
        }
    }

    return mask;
}
