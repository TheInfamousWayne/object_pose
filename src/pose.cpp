#include <pose.hpp>

Pose::Pose(vector<Image> obj) {
    image_objects = obj;
    float K[3][3] = {{589.607902244274, 0.0, 366.49661815699994},
                     {0.0, 590.1790734214388, 297.98736394590526},
                     {0.0, 0.0, 1.0}};

    float D[5][1] = {{-0.2489693848298703}, {0.13435384837763378}, {0.0003204379158765275}, {-0.00036140843150739765}, {-0.06579839150308762}};

    float R[3][3] = {{1.0890998989916354, 2.614833277418142, 1.0605210499895859}, // 1st column is r60, 2nd is r180, 3rd is r300
                     {2.5172012639946733, -0.026669213558675804, -2.5351740094307447},
                     {-0.8452495044215251, -0.01769147496035694, 0.9169399036537441}};

    float T[3][3] = {{-0.016788995687316167, -0.001077263937284312, -0.0030656937101139854}, // 1st column is t60, 2nd is t180, 3rd is t300
                     {-0.02269247299737612, -0.015886005467183863, -0.022204248661934205},
                     {0.5454465438837742, 0.5365514015146118, 0.5282896793079157}};

    float reference_center_Point_3d[8][4] = {{0.0326, -0.0326, 0.0652, 1},
                                             {-0.0326, -0.0326, 0.0652, 1},
                                             {0.0326, 0.0326, 0.0652, 1},
                                             {-0.0326, 0.0326, 0.0652, 1},
                                             {0.0326, -0.0326, 0, 1},
                                             {-0.0326, -0.0326, 0, 1},
                                             {0.0326, 0.0326, 0, 1},
                                             {-0.0326, 0.0326, 0, 1}};

    this->K = Mat(3,3, CV_32F, &K);
    this->D = Mat(5,1, CV_32F, &D);
    this->R = Mat(3,3, CV_32F, &R);
    this->T = Mat(3,3, CV_32F, &T);
    this->reference_center_Point_3d = Mat(8,4, CV_32F, &reference_center_Point_3d);

    // Setting the bounds for pose estimation
    position_lower_bound = Point3f(-0.5, -0.5, -0.5);
    position_upper_bound = Point3f(0.5, 0.5, 0.5);
    orientation_lower_bound = Point3f(-3.14159265, -3.14159265, -3.14159265);
    orientation_upper_bound = Point3f(3.14159265, 3.14159265, 3.14159265);

    mean_position = (position_upper_bound + position_lower_bound) / 2.0;
    mean_orientation = (orientation_upper_bound + orientation_lower_bound) / 2.0;

    var_position = power((position_lower_bound - position_upper_bound)/4.0, 2);
    var_orientation = power((orientation_lower_bound - orientation_upper_bound)/4.0, 2);

}

Point3f Pose::power(Point3f p, float n) {
    p.x = pow(p.x, n);
    p.y = pow(p.y, n);
    p.z = pow(p.z, n);
    return p;
}



vector<Point3f> Pose::random_normal(Point3f mean, Point3f var, int rows, int cols) {
    vector<Point3f> data;
    std::default_random_engine gen;
    std::normal_distribution<float> d1{mean.x, var.x};
    std::normal_distribution<float> d2{mean.y, var.y};
    std::normal_distribution<float> d3{mean.z, var.z};
    float x,y,z;
    for (int r=0; r<rows; r++) {
        for (int c=0; c<cols; c++) {
            x = d1(gen);
            y = d2(gen);
            z = d3(gen);
            data.push_back(Point3f(x,y,z));
        }
    }
    return data;
}


vector<float> Pose::cost_function(vector<Point3f> proposed_translation, vector<Point3f> proposed_orientation) {
    int number_of_particles = proposed_translation.size();
    vector<Mat> pose;
    vector<Mat> proposed_new_cube_pts_w;

    for (int i=0; i<number_of_particles; i++) {
        Mat dest;
        cv::Rodrigues(Mat(proposed_orientation[i]), dest);
        cv::hconcat(dest, Mat(proposed_translation[i]), dest);
        vector<float> temp = {0, 0, 0, 1};
        dest.push_back(Mat(1,4, CV_32F, &temp));
        pose.push_back(dest);

        Mat new_pt = dest * reference_center_Point_3d.t();
        proposed_new_cube_pts_w.push_back(new_pt.t());
    }

    for (int i=0; i<3; i++) {
        Mat imgpoints;
        Mat input = Mat(number_of_particles * reference_center_Point_3d.rows, 4, CV_32F, &proposed_new_cube_pts_w);
        input = input.colRange(0,input.cols-1);
        cout << R << endl;
        cout << R.col(i) << endl;
        cout << T.col(i) << endl;
        int a;
        cin >>a;
//        cv::projectPoints(input.t(), R.col(i), T.col(i), K, D, imgpoints);

    }


    vector<float> error(number_of_particles, 0.0);
    return error;
}


void Pose::cem() {
    int max_iterations = 30;
    int number_of_particles = 1000;
    int elites = 100;
    float alpha = 0.2;
    float eps = 5.0;
    float best_cost = FLT_MAX;
    vector<float> costs;

    for (int i =0; i<max_iterations && best_cost>eps; i++) {
        vector<Point3f> sample_p = random_normal(mean_position, power(var_position, 0.5), number_of_particles, 3);
        vector<Point3f> sample_o = random_normal(mean_orientation, power(var_orientation, 0.5), number_of_particles, 3);

        costs = cost_function(sample_p, sample_o);
    }




}

void Pose::find_pose() {
    cout << this->R << endl;
    cout << R.col(0) << endl;
    cout << T.col(0) << endl;
    cem(); // calculates mean_position and mean_orientation

}