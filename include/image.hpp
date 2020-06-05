#pragma once
#include <thread>
#include <iostream> 
#include <typeinfo>
#include <opencv2/opencv.hpp>
#include <armadillo>

using namespace std;
using namespace cv;

struct ColorBounds {
	string name;
	int lower[3];
	int upper[3];
};

class Image {
	Mat image;
	map<string, ColorBounds> colors;
	map<string, Mat> masks; // individual color segment mask
	map<string, vector<int>> pixel_idx;
	map<string, vector<pair<int, int>>> pixel_dataset; // pixel coordinates of the region of interest
	map<string, int> color_count; // total pixels with a particular color
	struct cmp { // Declaring a set that will store the pairs using the comparator logic
        bool operator()(pair<string, int> elem1, pair<string, int> elem2) {
            return elem1.second > elem2.second;
        }
    };
	set<pair<string, int>, cmp> dominant_colors;
	vector<pair<string,string>> color_pairs;

    struct cmp2 { // Declaring a set that will store the pairs using the comparator logic
        bool operator()(pair<int, int> elem1, pair<int, int> elem2) {
            return elem1.second > elem2.second;
        }
    };
    set<pair<int, int>, cmp2> label_count;



public:
	Image(Mat);
	void set_color_bounds();
	void initialise_object_model();
    void gmm_mask();
    bool denoise();
	void create_mask(const string&);
	void start();
	void startSingleThread();
	void create_pixel_dataset(const string&);
	void pipeline(const string&);
	void find_dominant_colors(const int);
	void show();
	void print_pixels();
	void remove_outliers();
	void make_valid_combinations();
	void get_line_between_colors(const string&, const string&);
    void clean_mask(const string& color);
    int getNthElement(int n);
	map<pair<string, string>, pair<float,float>> lines;
	map<pair<string, string>, pair<int,int>> object_model;

};







