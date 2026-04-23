#ifndef PREPROCESS_H
#define PREPROCESS_H

#include <opencv2/opencv.hpp>
#include <vector>

/*Pregateste imaginile pentru analiza
 * Redimensioneaza pozele la o scară comuna si elimina culorile/zgomotul
 */

cv::Mat preprocesareImagine(const cv::Mat& input, cv::Size dimensiuneTinta);
std::vector<cv::Mat> preprocesareToate(const std::vector<cv::Mat>& imaginiInput, cv::Size dimensiuneTinta);

#endif