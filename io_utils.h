#ifndef IO_UTILS_H
#define IO_UTILS_H

#include <string>
#include <vector>
#include <opencv2/opencv.hpp>

bool parseArguments(int argc, char** argv, std::string& inputFolder, std::string& outputFolder);
bool loadImagesFromFolder(const std::string& folderPath, std::vector<cv::Mat>& images, std::vector<std::string>& imageNames);
bool saveImage(const std::string& outputPath, const cv::Mat& image);
bool isImageFile(const std::string& fileName);

#endif
