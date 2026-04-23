#include <iostream>
#include <vector>
#include <string>
#include <opencv2/opencv.hpp>

// Includem toate modulele echipei
#include "io_utils.h"
#include "preprocess.h"
#include "duplicate_detection.h"
#include "merge_utils.h"

int main(int argc, char** argv)
{
    //parseaza argumentele venite de la server
    std::string inputFolder;
    std::string outputFolder;

    //daca argumentele nu sunt bune, opreste executia
    if (!parseArguments(argc, argv, inputFolder, outputFolder))
    {
        return 1; 
    }

    //incarca imaginile (io_utils)
    std::vector<cv::Mat> images;
    std::vector<std::string> imageNames;
    
    //opreste executia daca nu a gasit imagini
    if (!loadImagesFromFolder(inputFolder, images, imageNames))
    {
        return 1; 
    }
    std::cout << "S-au incarcat " << images.size() << " imagini.\n";

    //le proceseaza (preprocess) -> alb-negru, blur, normalizare
    cv::Size blockDim(800, 600);
    std::vector<cv::Mat> processedImgs = preprocesareToate(images, blockDim);

    //detecteaza duplicate (duplicate_detection)
    std::vector<cv::Mat> duplicateMasks;
    int blockSize = 32;       //dimensiunea unui patratel
    double threshold = 50.0;  //pragul de toleranta la diferente

    //prima imagine este baza, deci masca ei e complet neagra (0)
    duplicateMasks.push_back(cv::Mat::zeros(processedImgs[0].size(), CV_8UC1));

    //compara restul imaginilor cu baza
    for (size_t i = 1; i < processedImgs.size(); ++i) {
        cv::Mat mask = findDuplicateRegions(processedImgs[0], processedImgs[i], blockSize, threshold);
        duplicateMasks.push_back(mask);
    }

    //asamblarea (merge_utils)
    std::vector<cv::Mat> colorResizedImgs;
    for (const auto& img : images) {
        cv::Mat resizedImg;
        cv::resize(img, resizedImg, cv::Size(800, 600));
        colorResizedImgs.push_back(resizedImg);
    }

    //trimite imaginile color (800x600), impreuna cu mastile
    cv::Mat finalImage = buildImage(colorResizedImgs, duplicateMasks, blockSize);

    //salveaza (io_utils)
    saveImage(outputFolder + "/test_output.png", finalImage);

    return 0; 
}