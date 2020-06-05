#include <utils.hpp>
#include <armadillo>
#include <numeric>

Mat get_image(const string& path, const int idx) {
    Mat image;
    image = imread(path + "/image_" + to_string(idx) + ".jpg", 1);
    if ( !image.data )
    {
        cout<< (path + "/image_" + to_string(idx) + ".jpg");
        printf("No image data \n");
        exit(0);
    }
    cv::fastNlMeansDenoisingColored(image, image, 10, 10, 7, 21);
    cv::GaussianBlur(image, image, Size(5,5), 0);
    cv::cvtColor(image, image, COLOR_BGR2RGB);
    return image;
}

vector<Mat> get_images(const string& path, bool get_masks) {
    vector<Mat> frames;
    for (auto p : fs::recursive_directory_iterator(path)) {
        int ascii = (int)p.path().string()[37];

        if (ascii >= 48 && ascii <= 57) {
            if (get_masks) {
                cout << p.path().string() << "\n";
                Mat image = imread(p.path().string(), 1);
                if (!image.data) {
                    cout << (p);
                    printf("No image data \n");
                    exit(0);
                }
                cv::fastNlMeansDenoisingColored(image, image, 10, 10, 7, 21);
                cv::GaussianBlur(image, image, Size(5,5), 0);
                cv::cvtColor(image, image, COLOR_BGR2RGB);
                frames.push_back( image );

                Mat temp = image.reshape(1, image.rows * image.cols);
                temp.convertTo(temp, CV_32F);
                Mat res;
                remove_if(temp, res, is_zero, true);
                cout << res.rows << " " << res.cols << endl;
            }
            else {
                continue;
            }
        }
        if (get_masks) {
            continue;
        }

        cout << path << " " << p << "\n";

        Mat image = imread(p.path().string(), 1);
        if ( !image.data )
        {
            cout<< (p);
            printf("No image data \n");
            exit(0);
        }
        cv::fastNlMeansDenoisingColored(image, image, 10, 10, 7, 21);
        cv::GaussianBlur(image, image, Size(5,5), 0);
        cv::cvtColor(image, image, COLOR_BGR2RGB);
        frames.push_back( image );
    }
    return frames;
}

map<char, string> initialize() {
    map<char, string> mapper = {
            {'r', "red"},
            {'g', "green"},
            {'b', "blue"},
            {'y', "yellow"},
            {'m', "magenta"},
            {'c', "cyan"}
    };
    return mapper;
}


bool is_zero(const Mat &rc)
{
    return (sum(rc)[0]==0);
}

//typedef bool (*remove_predicate)(const Mat &rc);

void remove_if(const Mat &mat, Mat &res, remove_predicate pred, bool removeRows=true)
{
    res.release();
    int n = removeRows ? mat.rows : mat.cols;
    for (int i=0; i<n; i++)
    {
        Mat rc = removeRows ? mat.row(i) : mat.col(i);
        if (pred(rc)) continue; // remove element
        if (res.empty()) res = rc;
        else
        {
            if (removeRows)
                vconcat(res,rc,res);
            else
                hconcat(res,rc,res);
        }
    }
}

map<string, Mat> get_color_masks(const string& path, map<string, Mat> dataset) {
    auto mapper = initialize();
    for (auto p : fs::recursive_directory_iterator(path)) {
        int ascii = (int) p.path().string()[37];
        if (ascii >= 48 && ascii <= 57) {
            cout << p.path().string() << "\n";
            string color = mapper[p.path().string()[36]];
            Mat image = imread(p.path().string(), 1);
            if (!image.data) {
                cout << (p);
                printf("No image data \n");
                exit(0);
            }
            Mat image_hsv, concatenated_data, cleaned_data;
            cv::cvtColor(image, image_hsv, cv::COLOR_BGR2HSV);
            Mat data1 = image.reshape(1, image.rows * image.cols);
            Mat data2 = image_hsv.reshape(1, image_hsv.rows * image_hsv.cols);
            data1.convertTo(data1, CV_32F);
            data2.convertTo(data2, CV_32F);
            cv::hconcat(data1, data2, concatenated_data);
            remove_if(concatenated_data, cleaned_data, is_zero, true); // removing all zero rows
            bool print = false;
            if (print) {
                cout << cleaned_data.rows << " " << cleaned_data.cols << endl;
            }
            if (dataset.find(color) == dataset.end()) { // key not found
                dataset[color] = cleaned_data;
            }
            else { // key found
                cv::vconcat(dataset[color], cleaned_data, dataset[color]);
            }
        }
    }
    return dataset;
}

vector<string> get_directories(const string& s)
{
    vector<string> r;
    for(auto p : fs::recursive_directory_iterator(s))
        if (fs::is_directory(p)) {
            r.push_back(p.path().string());
        }
    return r;
}



void train_gmm(map<string, Mat> dataset) {

    map<string, int> modes_ = {
            {"red", 4},
            {"green", 4},
            {"blue", 6},
            {"yellow", 4},
            {"magenta", 4},
            {"cyan", 6}
    };

    for(auto& color_data : dataset) {
        string color = color_data.first;
        Mat data = color_data.second.t();
        arma::mat input_data(data.rows, data.cols, arma::fill::zeros);

        for (int i=0; i<data.cols; i++) {
            vector<double> vec;
            data.col(i).copyTo(vec);
            input_data.col(i) = arma::vec(vec);
        }

        arma::gmm_full model;
        bool status = model.learn(input_data, modes_[color], arma::maha_dist, arma::random_subset, 100, 1000, 1e-6, true);
        if(status == false)
        {
            cout << "learning failed" << endl;
        }
        model.means.print("means:");
        string name = "../data/" + color + ".gmm";
        model.save(name);
        cout << color << " done\n";
    }

}