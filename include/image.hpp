#include <thread> 
#include <iostream> 
#include <typeinfo>
#include <opencv2/opencv.hpp>

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
	map<string, vector<int, int>> pixel_dataset; // pixel coordinates of the region of interest
	maps<string, int> color_count; // total pixels with a particular color

public:
	Image(Mat);
	void create_mask(const string&);
	void start();
	void startSingleThread();
	void create_pixel_dataset(const string&);
	void pipeline(const string&);
	void find_dominant_colors(const int);
	void sort_by_count();
	void show();
};

void Image::create_pixel_dataset(const string& color) {
	vector<int, int> poi; //pixels_of_interest
	for (int i=0; i < masks[color].rows; ++i){
		for (int j=0; j < masks[color].cols; ++j){
			if ((int)masks[color].at<uchar>(i, j) == 255)
				poi.push_back(i,j);
		}
	}
	pixel_dataset[color] = poi;
	color_count[color] = poi.size();
}

Image::Image (Mat img) {
	image = img;
	// HSV color bounds
	colors["red"] = {"red", {0, 159, 164}, {12, 255, 208}};
	colors["blue"] = {"blue", {101, 189, 143}, {179, 255, 181}};
	colors["green"] = {"green", {71, 122, 160}, {90, 255, 255}};
	colors["pink"] = {"pink", {5, 63, 233}, {12, 115, 255}};
	colors["dark_pink"] = {"dark_pink", {114, 40, 169}, {179, 129, 196}};
	colors["bullshit"] = {"bullshit", {0, 0, 0}, {0, 0, 0}};
	cvtColor(image, image, COLOR_RGB2HSV);
}

void Image::create_mask(const string& color) {
	cout<<colors[color].name<<endl;
	Mat mask;
	auto lower = colors[color].lower;
	auto upper = colors[color].upper;
	inRange(image, Scalar(lower[0], lower[1], lower[2]), Scalar(upper[0], upper[1], upper[2]), mask);
	masks[color] = {mask};
	// cout << "Mask : " << mask.size() << endl;
	// cout << "Image: " << image.size() << endl;
	string var = to_string(rand()%1000);
	namedWindow(var, WINDOW_AUTOSIZE);
	imshow(var, image);
	waitKey(0);
	// imshow("sd", mask);
}

void Image::sort_by_count() {
	// Defining a lambda function to compare two pairs. It will compare two pairs using second field
	typedef function<bool(pair<string, int>, pair<string, int>)> Comparator;
	Comparator compFunctor = 
			[](pair<string, int> elem1 ,pair<tring, int> elem2){
				return elem1.second > elem2.second; // descending order
			};
	// Declaring a set that will store the pairs using above comparision logic
	set<pair<string, int>, Comparator> dominent_colors(
			color_count.begin(), color_count.end(), compFunctor);
 	
	set<pair<string, int>::iterator it = dominent_colors.begin();



 	return dominent_colors
}

void Image::find_dominant_colors(const int N) { // N dominent colors 
	sort_by_count();
}

void Image::pipeline(const string& color) {
	create_mask(color);
	create_pixel_dataset(color);
	find_dominant_colors(3);
}

void Image::start() {
	vector<thread> vecOfThreads;
	// create color masks
	for (auto& i : colors) {
		thread th(&Image::pipeline, this, i.first);
		vecOfThreads.push_back(move(th));
	}
	// wait for threads to finish
	for (thread & th : vecOfThreads) {
        // If thread Object is Joinable then Join that thread.
        if (th.joinable())
            th.join();
    }
    // display masks 
    show();

}


void Image::startSingleThread() {
	for (auto& i : colors) {
		create_mask(i.first);
	}
}


void Image::show() {
	for (auto& m : masks) {
		
        namedWindow(m.first, WINDOW_AUTOSIZE );
        cout<<"Waiting to display"<<endl;
        imshow(m.first, image);
        waitKey(0);
    }
}










