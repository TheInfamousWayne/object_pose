#pragma once
#include <thread>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <filesystem>
#include <image.hpp>

using namespace cv;
using namespace std;
namespace fs = __fs::filesystem;

class Pose {
    vector<Image> image_objects;
    Mat K; // camera matrix
    Mat D; // distortion coeffs
    Mat R; // rotation matrix
    Mat T; // translation matrix
    Point3f position_lower_bound;
    Point3f position_upper_bound;
    Point3f orientation_lower_bound;
    Point3f orientation_upper_bound;
    Point3f mean_position;
    Point3f mean_orientation;
    Point3f prev_orientation;
    Point3f prev_position;
    Point3f var_position;
    Point3f var_orientation;

public:
    Pose(vector<Image>);
    void cem();
    void cost_function();
    void find_pose();
};



