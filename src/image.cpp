#include <image.hpp>
#include <dbscan.hpp>


Image::Image (Mat img) {
	image = img;
    set_color_bounds();
    initialise_object_model();
	cvtColor(image, image, COLOR_RGB2HSV);
}

void Image::set_color_bounds() {
    // HSV color bounds
    colors["red"] = {"red", {0, 159, 164}, {12, 255, 208}};
	colors["blue"] = {"blue", {101, 189, 143}, {179, 255, 181}};
	colors["green"] = {"green", {71, 122, 160}, {90, 255, 255}};
	colors["pink"] = {"pink", {5, 63, 233}, {12, 115, 255}};
	colors["dark_pink"] = {"dark_pink", {114, 40, 169}, {179, 129, 196}};
	colors["bullshit"] = {"bullshit", {0, 0, 0}, {0, 0, 0}};
}

void Image::initialise_object_model() {
    object_model[make_pair("green", "dark_pink")] = {0,1};
    object_model[make_pair("green", "pink")] = {0,2};
    object_model[make_pair("green", "blue")] = {1,3};
    object_model[make_pair("green", "red")] = {2,3};
    object_model[make_pair("red", "blue")] = {3,7};
    object_model[make_pair("dark_pink", "blue")] = {1,5};
    object_model[make_pair("dark_pink", "pink")] = {0,4};
    object_model[make_pair("red", "pink")] = {2,6};
    object_model[make_pair("bullshit", "pink")] = {4,6};
    object_model[make_pair("bullshit", "dark_pink")] = {4,5};
    object_model[make_pair("bullshit", "blue")] = {5,7};
    object_model[make_pair("bullshit", "red")] = {6,7};
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
	for (auto& i : colors) {
		pipeline(i.first);
	}
	find_dominant_colors(3);
	remove_outliers();
    make_valid_combinations();
    for (auto& i : color_pairs) {
        get_line_between_colors(i.first, i.second);
    }
	cout << "X\n" ;
}

void Image::pipeline(const string& color) {
	create_mask(color);
	create_pixel_dataset(color);
}

void Image::create_mask(const string& color) {
	Mat mask;
	auto lower = colors[color].lower;
	auto upper = colors[color].upper;
	inRange(image, Scalar(lower[0], lower[1], lower[2]), Scalar(upper[0], upper[1], upper[2]), mask);
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

void Image::find_dominant_colors(const int N) { // N dominant colors
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




