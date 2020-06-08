#include <image.hpp>
#include <dbscan.hpp>
//#include <bits/stdc++.h>

Image::Image (Mat img) {
	image = img;
    set_color_bounds();
    initialise_object_model();
//	cvtColor(image, image, COLOR_RGB2HSV);
}

void Image::set_color_bounds() {
    // HSV color bounds
    // TODO: bounds not right for new colors
    colors["red"] = {"red", {0, 159, 164}, {12, 255, 208}};
	colors["blue"] = {"blue", {101, 189, 143}, {179, 255, 181}};
	colors["green"] = {"green", {71, 122, 160}, {90, 255, 255}};
	colors["yellow"] = {"yellow", {5, 63, 233}, {12, 115, 255}};
	colors["magenta"] = {"magenta", {114, 40, 169}, {179, 129, 196}};
	colors["cyan"] = {"cyan", {75, 50, 40}, {97, 255, 180}};
}

void Image::initialise_object_model() {
    object_model[make_pair("yellow", "cyan")] = {0,1};
    object_model[make_pair("yellow", "magenta")] = {0,2};
    object_model[make_pair("yellow", "green")] = {1,3};
    object_model[make_pair("yellow", "blue")] = {2,3};
    object_model[make_pair("blue", "green")] = {3,7};
    object_model[make_pair("cyan", "green")] = {1,5};
    object_model[make_pair("cyan", "magenta")] = {0,4};
    object_model[make_pair("blue", "magenta")] = {2,6};
    object_model[make_pair("red", "magenta")] = {4,6};
    object_model[make_pair("red", "cyan")] = {4,5};
    object_model[make_pair("red", "green")] = {5,7};
    object_model[make_pair("red", "blue")] = {6,7};
    for (auto& i : object_model) {
        object_model[make_pair(i.first.second, i.first.first)] = i.second;
    }
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
    find_dominant_colors(3);
	remove_outliers();
    make_valid_combinations();

    vecOfThreads.clear();
    for (auto& i : color_pairs) {
		thread th(&Image::get_line_between_colors, this, i.first, i.second);
		vecOfThreads.push_back(move(th));
	}
	// wait for threads to finish
	for (thread & th : vecOfThreads) {
        // If thread Object is Joinable then Join that thread.
        if (th.joinable())
            th.join();
    }
    cout << "X\n" ;
}

void Image::startSingleThread() {
//	for (auto& i : colors) {
//		pipeline(i.first);
//	}
    std::time_t start, end;
    std::time(&start);
    ios_base::sync_with_stdio(false);
    gmm_mask(); // create_pixel_dataset is called within this
    std::time(&end);
    double time_taken = double(end - start);
    cout << "Time taken by gmm is : " << fixed
         << time_taken << setprecision(5);
    cout << " sec " << endl;
    find_dominant_colors(3);

//	for (auto& r : dominant_colors) {
//	    cout << r.first << " ---- " << r.second << " " << dominant_colors.size() << endl;
//	}

    std::time(&start);
	bool status = denoise();
    std::time(&end);
    time_taken = double(end - start);
    cout << "Time taken by denoise is : " << fixed
         << time_taken << setprecision(5);
    cout << " sec " << endl;

	for (auto& i : dominant_colors) {
	    Mat output, image_bgr;
        cv::cvtColor(image, image_bgr, cv::COLOR_RGB2BGR);
	    cv::bitwise_and(image_bgr, image_bgr, output, masks[i.first]);
	    cv::imshow("Orig", image_bgr);
	    cv::imshow(i.first, output);
	}
	cout << "Esc to exit...\n";
	cv::waitKey(0);
//	remove_outliers();
//    make_valid_combinations();
//    for (auto& i : color_pairs) {
//        get_line_between_colors(i.first, i.second);
//    }
	cout << "X\n" ;
}

void Image::pipeline(const string& color) {
	create_mask(color);
	create_pixel_dataset(color);
}

void Image::gmm_mask() {

    std::time_t start, end;
    double time_taken;


    map<string, float> threshold_ = {
            {"red", -29.0},
            {"green", -29.0},
            {"blue", -29.0},
            {"yellow", -29.0},
            {"magenta", -29.0},
            {"cyan", -29.0}
    };

    Mat image_bgr, image_hsv, concatenated_data;
    cv::cvtColor(image, image_hsv, cv::COLOR_RGB2HSV);
    cv::cvtColor(image, image_bgr, cv::COLOR_RGB2BGR);
    Mat data1 = image_bgr.reshape(1, image_bgr.rows * image_bgr.cols);
    Mat data2 = image_hsv.reshape(1, image_hsv.rows * image_hsv.cols);
    data1.convertTo(data1, CV_32F);
    data2.convertTo(data2, CV_32F);
    cv::hconcat(data1, data2, concatenated_data);
    concatenated_data = concatenated_data.t();

    arma::mat input_data(concatenated_data.rows, concatenated_data.cols, arma::fill::zeros);
    for (int i=0; i<concatenated_data.cols; i++) {
        vector<double> vec;
        concatenated_data.col(i).copyTo(vec);
        input_data.col(i) = arma::vec(vec);
    }

    Mat scores_(cv::Size(6, image.rows * image.cols), CV_64FC1, cv::Scalar(0));

    int color_idx = 0;
    map<int, string> idx2color;
    for (auto& color : colors) {
        arma::gmm_full model;
        string saved_model_path = "../data/" + color.first + ".gmm";
        bool status = model.load(saved_model_path);
        if (status == false) {
            cout << "loading failed for " << color.first << endl;
        }

        std::time(&start);
        auto result = model.log_p(input_data);
        std::time(&end);
        time_taken = double(end - start);
        cout << "Time taken by model prediction is : " << fixed
             << time_taken << setprecision(5);
        cout << " sec. " << color.first << endl;

        idx2color[color_idx] = color.first;
        for (int row_idx = 0; row_idx < result.size(); row_idx++) {
            scores_.at<double>(row_idx, color_idx) = (float) result[row_idx];
        }
        color_idx++;

        if (masks.find(color.first) == masks.end()) {
            Mat m(cv::Size(1, result.size()), CV_64FC1, cv::Scalar(0));
            masks[color.first] = m;
        }

        if (pixel_idx.find(color.first) == pixel_idx.end()) {
            vector<int> idx;
            pixel_idx[color.first] = idx;
        }
    }


    for (int row_idx=0; row_idx<scores_.rows; row_idx++) {
        cv::Point maxLoc;
        Mat row = scores_.row(row_idx);
        cv::minMaxLoc(row, NULL, NULL, NULL, &maxLoc);

        string color = idx2color[maxLoc.x];
        if (scores_.at<double>(row_idx, maxLoc.x) > threshold_[color]) {
            pixel_idx[color].push_back(row_idx);
        }
    }

    for (auto& color : colors) {
        clean_mask(color.first);
        for (auto& idx : pixel_idx[color.first]) {
            masks[color.first].at<double>(idx,0) = 255.0;
        }

//        cout << color.first << " " << pixel_idx[color.first].size() << endl;

        masks[color.first] = masks[color.first].reshape(1, image.rows);
        masks[color.first].convertTo(masks[color.first], CV_8U);

        std::time(&start);
        create_pixel_dataset(color.first);
        std::time(&end);
        time_taken = double(end - start);
        cout << "Time taken by pixel dataset is : " << fixed
             << time_taken << setprecision(5);
        cout << " sec " << endl;
    }
}

void Image::clean_mask(const string& color) {
    if (color == "cyan") {
        Mat image_hsv, mask;
        cv::cvtColor(image, image_hsv, cv::COLOR_RGB2HSV);
        vector<int> merged(pixel_idx[color]);
//        merged.insert(merged.end(), pixel_idx["blue"].begin(), pixel_idx["blue"].end());
        auto lower = colors[color].lower;
        auto upper = colors[color].upper;
        int n = image.rows * image.cols;
        cv::inRange(image_hsv, Scalar(lower[0], lower[1], lower[2]), Scalar(upper[0], upper[1], upper[2]), mask);
        mask = mask.reshape(1, n);
        mask.convertTo(mask, CV_64FC1);
        vector<int> idx2;
        for (int i=0; i<n; i++) {
            if ((float)mask.at<double>(i,0) == 255) { idx2.push_back(i); }
        }
        std::sort(merged.begin(), merged.end());
        std::sort(idx2.begin(), idx2.end());
        vector<int> final_idx(merged.size() + idx2.size());
        std::vector<int>::iterator it = std::set_intersection(merged.begin(), merged.end(), idx2.begin(), idx2.end(), final_idx.begin());
        final_idx.resize(it - final_idx.begin());
        pixel_idx[color] = final_idx;
    }

    if (color == "green") {
        Mat image_hsv, mask;
        cv::cvtColor(image, image_hsv, cv::COLOR_RGB2HSV);
        vector<int> merged(pixel_idx[color]);
        auto lower = colors[color].lower;
        auto upper = colors[color].upper;
        int n = image.rows * image.cols;
        cv::inRange(image_hsv, Scalar(0, 0, 0), Scalar(73, 255, 95), mask);
        mask = mask.reshape(1, n);
        mask.convertTo(mask, CV_64FC1);
        vector<int> idx2;
        for (int i=0; i<n; i++) {
            if ((float)mask.at<double>(i,0) == 0) { idx2.push_back(i); }
        }
        std::sort(merged.begin(), merged.end());
        std::sort(idx2.begin(), idx2.end());
        vector<int> final_idx(merged.size() + idx2.size());
        std::vector<int>::iterator it = std::set_intersection(merged.begin(), merged.end(), idx2.begin(), idx2.end(), final_idx.begin());
        final_idx.resize(it - final_idx.begin());
        pixel_idx[color] = final_idx;
    }

    if (color == "yellow") {
        Mat image_hsv, mask;
        cv::cvtColor(image, image_hsv, cv::COLOR_RGB2HSV);
        vector<int> merged(pixel_idx[color]);
        auto lower = colors[color].lower;
        auto upper = colors[color].upper;
        int n = image.rows * image.cols;
        cv::inRange(image_hsv, Scalar(0, 0, 0), Scalar(73, 255, 95), mask);
        mask = mask.reshape(1, n);
        mask.convertTo(mask, CV_64FC1);
        vector<int> idx2;
        for (int i=0; i<n; i++) {
            if ((float)mask.at<double>(i,0) == 0) { idx2.push_back(i); }
        }
        std::sort(merged.begin(), merged.end());
        std::sort(idx2.begin(), idx2.end());
        vector<int> final_idx(merged.size() + idx2.size());
        std::vector<int>::iterator it = std::set_intersection(merged.begin(), merged.end(), idx2.begin(), idx2.end(), final_idx.begin());
        final_idx.resize(it - final_idx.begin());
        pixel_idx[color] = final_idx;
    }
}

void Image::create_mask(const string& color) {
	Mat mask;
	auto lower = colors[color].lower;
	auto upper = colors[color].upper;
	cv::inRange(image, Scalar(lower[0], lower[1], lower[2]), Scalar(upper[0], upper[1], upper[2]), mask);
	masks[color] = {mask};
}

void Image::create_pixel_dataset(const string& color) {
	vector<pair<int, int>> poi; //pixels_of_interest
	for (int i=0; i < masks[color].rows; ++i) {
		for (int j=0; j < masks[color].cols; ++j) {
			if ((int)masks[color].at<uchar>(i, j) == 255)
				poi.push_back({i,j});
		}
	}
	pixel_dataset[color] = {poi};
	color_count[color] = poi.size();
}

void Image::find_dominant_colors(const int N=3) { // N dominant colors
    for (auto& i : color_count) {
        pair<string, int> c = make_pair(i.first, i.second);
        dominant_colors.insert(c);
    }

    // Removing unwanted entries from the last
    set<pair<string, int>>::iterator it;
    while (dominant_colors.size() > N) {
        dominant_colors.erase(prev(dominant_colors.end()));
    }
}


bool Image::denoise() {
    if (dominant_colors.size() == 0) { return true; }
    int erosion_size = 4;
    Mat kernel = cv::getStructuringElement(cv::MORPH_RECT,
                                            cv::Size( 2*erosion_size + 1, 2*erosion_size+1 ),
                                            cv::Point( erosion_size, erosion_size ) );
    Mat merged_mask(cv::Size(1, image.rows * image.cols), CV_8U, cv::Scalar(0));
    vector<int> merged_idx;

    cout<<"Denoising\n";
    for (auto& color : dominant_colors) {
        cout<< color.first << endl;
        Mat mask = masks[color.first];
        mask = mask.reshape(1, image.rows * image.cols);
        for (int i=0; i<mask.rows; i++) {
            if (mask.at<uchar>(i,0) == 255) {
                merged_idx.push_back(i);
                merged_mask.at<uchar>(i,0) = 255;
            }
        }
    }
    merged_mask = merged_mask.reshape(1, image.rows);
    Mat dilated_mask, labels_im;
    cv::dilate(merged_mask, dilated_mask, kernel, Point(-1,-1), 4);

    int num_labels = cv::connectedComponents(dilated_mask, labels_im);
    labels_im = labels_im.reshape(1, image.rows * image.cols);
    vector<int> unique_;
    for(int i=0; i<labels_im.rows; i++) {
        unique_.push_back(labels_im.at<uchar>(i,0));
    }
    std::sort(unique_.begin(), unique_.end());
    int uniqueCount = std::unique(unique_.begin(), unique_.end()) - unique_.begin();
    label_count.clear();
    for(int i=0; i<uniqueCount; i++) {
        label_count.insert(make_pair(i, std::count(unique_.begin(), unique_.end(), i)));
    }
    int element_number = 1; // 1 because 0 is for background
    vector<int> idx;
    float percentage_overlap = 40.0;
    do {
        int max_count_label = getNthElement(1);
        idx.clear();
        for (int i = 0; i < labels_im.rows; i++) {
            if (labels_im.at<uchar>(i, 0) == max_count_label) { idx.push_back(i); }
        }
        vector<int> final_idx(merged_idx.size() + idx.size());
        std::vector<int>::iterator it = std::set_intersection(merged_idx.begin(), merged_idx.end(), idx.begin(),
                                                              idx.end(), final_idx.begin());
        final_idx.resize(it - final_idx.begin());
        percentage_overlap = final_idx.size() / (merged_idx.size() + 1e-9) * 100;
        element_number++;
    } while (percentage_overlap < 25.0);

    for (auto& color : dominant_colors) {
        Mat mask = masks[color.first];
        mask = mask.reshape(1, image.rows * image.cols);
        for (int i = 0; i < mask.rows; i++) {
            if (mask.at<uchar>(i, 0) == 255) {
                auto it = std::find(idx.begin(), idx.end(), i);
                if (it == idx.end()) { mask.at<uchar>(i, 0) = 0; }
            }
        }
        masks[color.first] = mask.reshape(1, image.rows);
        create_pixel_dataset(color.first);
    }
    find_dominant_colors();
    return true;
}


int Image::getNthElement(int n) {
    if (n >= label_count.size()) {
        cout << "Not enough elements\n";
        return -1;
    }
    pair<int, int> elem = *std::next(label_count.begin(), n);
    return elem.first;
}

void Image::show() {
	for (auto& m : dominant_colors) {
        namedWindow(m.first, WINDOW_AUTOSIZE);
        cout << "Waiting to display" << endl;
        imshow("Mask", masks[m.first]);
        waitKey(0);
    }
}

void Image::print_pixels() {
    for (auto& i : pixel_dataset) {
        cout << i.first << "\n";
        for (auto p : i.second) {
            cout << p.first << ", " << p.second << " ";
        }
    }
}


void Image::remove_outliers() {
    // Making the classifier input data.
    struct vec2f {
        int data[2];
        int operator[](int idx) const { return data[idx]; }
    };
    vector<vec2f> input_data;
    for (auto& i : dominant_colors) {
        for (auto& d : pixel_dataset[i.first]) {
            input_data.push_back(vec2f{d.first, d.second});
        }
    }

    // Running DBSCAN for outlier detection
    auto dbscan = DBSCAN<vec2f, int>();
    dbscan.Run(&input_data, 2, 10, 50);
    auto noise = dbscan.Noise;
    auto clusters = dbscan.Clusters;

    // Removing noise pixels from pixel dataset
    for (auto& i : dominant_colors) {
        for (auto it = pixel_dataset[i.first].begin(); it != pixel_dataset[i.first].end(); it++) {
            for (auto& k : noise) {
                auto noisy_pixel = make_pair(input_data[k][0], input_data[k][1]);
                if (*it == noisy_pixel) {
                    pixel_dataset[i.first].erase(it--);
                    break;
                }
            }
        }
    }
}


void Image::make_valid_combinations() {
    vector<string> color_keys;
    transform(dominant_colors.begin(), dominant_colors.end(), back_inserter(color_keys), [](pair<string, int> elem){ return elem.first; });
    for (auto it = color_keys.begin(); it!=color_keys.end(); it++) {
        for (auto it2 = it+1; it2!=color_keys.end(); it2++) {
            auto p = make_pair(*it, *it2);
            auto idx = object_model.find(p);
            if (idx != object_model.end())
                color_pairs.push_back(p);
        }
    }
}


void Image::get_line_between_colors(const string& c1, const string& c2) {

    vector<Point2f> classifier_input_data;
    vector<float> classifier_output_data;



    if (pixel_dataset[c1].size() > 0 && pixel_dataset[c2].size() > 0) {

    for (auto& i : pixel_dataset[c1]) {
        classifier_input_data.push_back(Point2f(i.first, i.second));
        classifier_output_data.push_back(0.0);
    }
    for (auto& i : pixel_dataset[c2]) {
        classifier_input_data.push_back(Point2f(i.first, i.second));
        classifier_output_data.push_back(1.0);
    }

    Mat ip = Mat(classifier_input_data.size(), 2, CV_32F, classifier_input_data.data());
    Mat op = Mat(classifier_output_data.size(), 1, CV_32F, classifier_output_data.data());

    Ptr<cv::ml::LogisticRegression> lr = cv::ml::LogisticRegression::create();
    lr->setLearningRate(0.001);
    lr->setTrainMethod(cv::ml::LogisticRegression::MINI_BATCH);
    lr->setMiniBatchSize(2);
    lr->setIterations(100);
    lr->train(ip, cv::ml::ROW_SAMPLE, op);

    auto thetas = lr->get_learnt_thetas();
//    cout << thetas << " " << thetas.at<float>(0,0) << " " << thetas.at<float>(0,1) << " " << thetas.at<float>(0,2) << endl;

    float a = -thetas.at<float>(0,0)/thetas.at<float>(0,1);
    float b = -thetas.at<float>(0,2)/thetas.at<float>(0,1);

    lines[make_pair(c1,c2)] = {a,b};

    }
}




