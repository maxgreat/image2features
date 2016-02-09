#include "bagofword.h"



std::vector<std::vector<float> > bow::Image2SIFT(cv::Mat& image)
{
    std::vector<std::vector<float> > dl;
    std::vector<cv::KeyPoint> lkp; //Used to store the detected keypoint
    cv::Mat desc; //Used to store SIFT descriptor
    cv::Ptr<cv::xfeatures2d::SIFT> sift = cv::xfeatures2d::SIFT::create();//Create SIFT detector
    sift->detectAndCompute(image,cv::noArray(),lkp,desc); //Detect keypoint and Compute sift

    //Store the descriptor (in a cv::Mat) into the vector descList
    for(int i = 0; i < desc.rows; i++){
        std::vector<float> v;
        for(int j = 0; j < desc.cols; j++){
            v.push_back(desc.at<float>(i,j));
        }
        dl.push_back(v);
    }
    return dl;
}


float bow::bowDistance(std::vector<int> bow1, std::vector<int> bow2)
{
    if(bow1.size() != bow2.size())
    {
        fprintf(stderr,"EROOR computing distance : vector have to have the same size\n");
        return -1;
    }
    int d = 0;
    for(size_t i = 0; i < bow1.size(); i++)
    {
        d += (bow1[i]+bow2[i])*(bow1[i]+bow2[i]);
    }
    return sqrt(d);
}

float bow::descriptorDistance(std::vector<float> bow1, std::vector<float> bow2)
{
    if(bow1.size() != bow2.size())
    {
        fprintf(stderr,"EROOR computing distance : vector have to have the same size\n");
        return -1;
    }
    float d = 0;
    for(size_t i = 0; i < bow1.size(); i++)
    {
        d += (bow1[i]-bow2[i])*(bow1[i]-bow2[i]);
    }
    return sqrt(d);
}





void BagOfWord::AddImage(cv::Mat& image)
{
    if(!clusters.empty())
    {
        fprintf(stderr, "ERROR: BagOfWord : cannot add an image, the cluster are already computed\n");
        return;
    }
    std::vector<std::vector<float> > desc = bow::Image2SIFT(image);
    for(auto& i : desc)
    {
        descList.push_back(i);
    }
}


void BagOfWord::saveDesc(const std::string& filename, int jump /* = 1 */)
{
    std::ofstream file;
    file.open(filename);
    for(size_t i = 0; i < (descList.size()/jump); i++)
    {
        for(size_t j = 0; j < descList[i*jump].size(); j++)
        {
            file << descList[i*jump][j] << ' ';
        }
        file << std::endl;
    }
    file.close();
}

void BagOfWord::loadCluster(const std::string& filename)
{
    std::ifstream file(filename);
    std::cout << "Read clusters in file " << filename << std::endl << std::flush;
    int i = 0;
    while(!file.eof())
    {
        std::vector<float> vec;
        vec.resize(128);
        for(int i = 0; i < 128; i++)
            file >> vec[i];
        clusters.push_back(vec);
        i++;
    }
    std::cout << "Read " << i << "vector" << std::endl;
    file.close();
}

std::vector<int> BagOfWord::ImageToBOW(cv::Mat& image)
{
    std::vector<int> vec;
    if(clusters.empty())
    {
        fprintf(stderr, "ERROR: BagOfWord : clusters not computed, cannot compute Bag Of Word\n");
        return vec;
    }
    vec.resize(clusters.size());
    for(size_t i = 0; i < vec.size(); i++)
        vec[i] = 0;

    std::vector<std::vector<float> > desc = bow::Image2SIFT(image);
    for(auto& d : desc)
    {
        unsigned int closest = 0;
        float dist = bow::descriptorDistance(clusters[0], d);
        for(size_t i = 1; i < clusters.size(); i++)
        {
            if(bow::descriptorDistance(clusters[i], d) < dist)
                closest = i;
        }
        vec[closest]++;
    }

    return vec;
}
