#include "preprocess.h"

cv::Mat preprocesareImagine(const cv::Mat& input, cv::Size dimensiuneTinta) {
    if (input.empty()) return cv::Mat();            // ignoră dacă imaginea e goală

    cv::Mat res;

    cv::resize(input, res, dimensiuneTinta);        // standardizează dimensiunile

    if (res.channels() == 3) {                      // dacă imaginea este color
        cv::cvtColor(res, res, cv::COLOR_BGR2GRAY); // convertește în grayscale
    }

    cv::GaussianBlur(res, res, cv::Size(3, 3), 0);  // elimină zgomotul pixelilor

    // balansează contrastul pentru iluminare similară între imagini
    cv::normalize(res, res, 0, 255, cv::NORM_MINMAX); 

    return res;
}

std::vector<cv::Mat> preprocesareToate(const std::vector<cv::Mat>& imaginiInput, cv::Size dimensiuneTinta) {
    std::vector<cv::Mat> listaRezultate;
    
    for (int i = 0; i < (int)imaginiInput.size(); i++) {
        cv::Mat p = preprocesareImagine(imaginiInput[i], dimensiuneTinta);
        listaRezultate.push_back(p);                // salvează imaginea procesată
    }
    
    return listaRezultate;
}