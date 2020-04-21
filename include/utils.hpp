#include <stdio.h>
#include <stdlib.h>
#include <opencv2/opencv.hpp>
#include <boost/filesystem.hpp>
using namespace cv;
using namespace std;

namespace fs = boost::filesystem;

Mat get_image(const string& path, const int idx) ;
vector<string> get_directories(const string& s);
