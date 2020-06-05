#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <opencv2/opencv.hpp>
#include <boost/filesystem.hpp>
using namespace cv;
using namespace std;

namespace fs = boost::filesystem;

Mat get_image(const string& path, const int idx);
vector<Mat> get_images(const string& path, bool get_masks=false);
vector<string> get_directories(const string& s);
map<char, string> initialize();
map<string, Mat> get_color_masks(const string& path, map<string, Mat> dataset);
void train_gmm(map<string, Mat> dataset);
bool is_zero(const Mat &rc);
typedef bool (*remove_predicate)(const Mat &rc);
void remove_if(const Mat &mat, Mat &res, remove_predicate pred, bool removeRows);
