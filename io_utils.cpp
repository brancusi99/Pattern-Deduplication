#include "io_utils.h"

#include <iostream>
#include <filesystem>
#include <cctype>

namespace fs = std::filesystem;

bool parseArguments(int argc, char** argv, std::string& inputFolder, std::string& outputFolder)
{
    if (argc != 3)
    {
        std::cerr << "Utilizare: " << argv[0] << " <input_folder> <output_folder>\n";
        return false;
    }

    inputFolder = argv[1];
    outputFolder = argv[2];

    if (!fs::exists(inputFolder))
    {
        std::cerr << "Eroare: folderul de input nu exista: " << inputFolder << "\n";
        return false;
    }

    if (!fs::is_directory(inputFolder))
    {
        std::cerr << "Eroare: calea de input nu este folder: " << inputFolder << "\n";
        return false;
    }

    if (!fs::exists(outputFolder))
    {
        try
        {
            fs::create_directories(outputFolder);
        }
        catch (const fs::filesystem_error& e)
        {
            std::cerr << "Eroare la crearea folderului de output: " << e.what() << "\n";
            return false;
        }
    }

    return true;
}

bool isImageFile(const std::string& fileName)
{
    std::string ext = fs::path(fileName).extension().string();

    for (char& c : ext)
    {
        c = (char)tolower(c);
    }

    return (ext == ".jpg" || ext == ".jpeg" || ext == ".png" || ext == ".bmp" || ext == ".tiff");
}

bool loadImagesFromFolder(const std::string& folderPath, std::vector<cv::Mat>& images, std::vector<std::string>& imageNames)
{
    images.clear();
    imageNames.clear();

    if (!fs::exists(folderPath))
    {
        std::cerr << "Eroare: folderul nu exista: " << folderPath << "\n";
        return false;
    }

    if (!fs::is_directory(folderPath))
    {
        std::cerr << "Eroare: calea data nu este folder: " << folderPath << "\n";
        return false;
    }

    bool foundAtLeastOneImage = false;

    for (const auto& entry : fs::directory_iterator(folderPath))
    {
        if (!entry.is_regular_file())
        {
            continue;
        }

        std::string filePath = entry.path().string();
        std::string fileName = entry.path().filename().string();

        if (!isImageFile(fileName))
        {
            continue;
        }

        foundAtLeastOneImage = true;

        cv::Mat img = cv::imread(filePath);

        if (img.empty())
        {
            std::cerr << "Avertisment: imaginea nu a putut fi incarcata: " << filePath << "\n";
            continue;
        }

        images.push_back(img);
        imageNames.push_back(fileName);

        std::cout << "Imagine incarcata: " << fileName
                    << " (" << img.cols << "x" << img.rows << ")\n";
    }

    if (!foundAtLeastOneImage)
    {
        std::cerr << "Eroare: nu s-au gasit imagini in folderul: " << folderPath << "\n";
        return false;
    }

    if (images.empty())
    {
        std::cerr << "Eroare: au fost gasite fisiere imagine, dar niciuna nu a putut fi incarcata.\n";
        return false;
    }

    return true;
}

bool saveImage(const std::string& outputPath, const cv::Mat& image)
{
    if (image.empty())
    {
        std::cerr << "Eroare: imaginea care trebuie salvata este goala.\n";
        return false;
    }

    try
    {
        bool ok = cv::imwrite(outputPath, image);

        if (!ok)
        {
            std::cerr << "Eroare: OpenCV nu a putut salva imaginea la: " << outputPath << "\n";
            return false;
        }
    }
    catch (const cv::Exception& e)
    {
        std::cerr << "Eroare OpenCV la salvare: " << e.what() << "\n";
        return false;
    }

    std::cout << "Imagine salvata cu succes la: " << outputPath << "\n";
    return true;
}
