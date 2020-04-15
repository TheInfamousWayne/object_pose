#pragma once
#include <thread>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <filesystem>
#include <image.hpp>
#include <math.h>
#include <float.h>
#include <random>

using namespace cv;
using namespace std;

class Pose {
    vector<Image> image_objects;
    Mat K; // camera matrix
    Mat D; // distortion coeffs
    Mat R; // rotation matrix
    Mat T; // translation matrix
    Mat reference_center_Point_3d;
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
    vector<int> xyz;

public:
    Pose(vector<Image>);
    void cem();
    vector<float> cost_function(vector<Point3f>, vector<Point3f>);
    void find_pose();
    vector<Point3f> random_normal(Point3f, Point3f, int rows, int cols);
    Point3f power(Point3f, float);
    Point3f mean(vector<Point3f>);
    Point3f var(vector<Point3f>);
    Mat getPoseMatrix(Point3f, Point3f);
};



