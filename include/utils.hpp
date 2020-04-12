#include <stdio.h>
#include <stdlib.h>
#include <opencv2/opencv.hpp>
#include <filesystem>
using namespace cv;
using namespace std;

namespace fs = __fs::filesystem;

Mat get_image(const string& path, const int idx) ;
vector<string> get_directories(const string& s);
