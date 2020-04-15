#include <pose.hpp>

Pose::Pose (vector<Image> obj) {
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

    this->K = Mat(3,3, CV_32F, &K).clone();
    this->D = Mat(5,1, CV_32F, &D).clone();
    this->R = Mat(3,3, CV_32F, &R).clone();
    this->T = Mat(3,3, CV_32F, &T).clone();
    this->reference_center_Point_3d = Mat(8,4, CV_32F, &reference_center_Point_3d).clone();

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


Point3f Pose::power (Point3f p, float n) {
    p.x = pow(p.x, n);
    p.y = pow(p.y, n);
    p.z = pow(p.z, n);
    return p;
}



vector<Point3f> Pose::random_normal (Point3f mean, Point3f var, int rows, int cols) {
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

Mat Pose::getPoseMatrix(Point3f orientation, Point3f position) {
    Mat rotation_matrix;
    cv::Rodrigues(Mat(orientation), rotation_matrix);
    Mat translation = (Mat_<float>(3,1) << position.x, position.y, position.z );
    cv::hconcat(rotation_matrix, translation, rotation_matrix);
    float temp[4] = {0, 0, 0, 1.0};
    rotation_matrix.push_back(Mat(1,4, CV_32F, &temp));
    return rotation_matrix;
}

vector<float> Pose::cost_function (vector<Point3f> proposed_translation, vector<Point3f> proposed_orientation) {
    int number_of_particles = proposed_translation.size();
    vector<Mat> pose;
    Mat proposed_new_cube_pts_w;

    for (int i=0; i<number_of_particles; i++) { // initialization of pose
        Mat rotation_matrix = getPoseMatrix(proposed_orientation[i], proposed_translation[i]);
        pose.push_back(rotation_matrix);
        Mat new_pt = rotation_matrix * reference_center_Point_3d.t();
        new_pt = new_pt.t(); // 8x4
        proposed_new_cube_pts_w.push_back(new_pt.colRange(0, new_pt.cols - 1)); // 8x3

    }

//    vector<vector<Mat>> projected_points;
    vector<float> error(number_of_particles, 0.0);
    for (int i=0; i<R.cols; i++) { // range (r_vecs)
        vector<Point2f> imgpoints;
        cv::projectPoints(proposed_new_cube_pts_w.t(), R.col(i), T.col(i), K, D, imgpoints);
        Mat _8Nx2 = Mat(imgpoints);
        vector<Mat> _Nx8x2;
        for (int i=0; i<_8Nx2.rows; i+=8) {
            _Nx8x2.push_back(_8Nx2.rowRange(i, i+8));
        }
//        projected_points.append(_Nx8x2);
        auto lines = image_objects[i].lines;
        for (auto& it : lines) {
            auto points_lying = image_objects[i].object_model[it.first];
            pair<int,int> a_b = it.second;
            Mat points_on_edge; // 8Nx2
            for (auto& it2 : _Nx8x2) {
                points_on_edge.push_back(it2.row(points_lying.first));
                points_on_edge.push_back(it2.row(points_lying.second));
            }
            Mat distance;
            absdiff( a_b.first * points_on_edge.col(1), points_on_edge.col(0) - a_b.second, distance);
            error.push_back(sum(distance.col(0))[0]);
        }
    }
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
        vector<float> sorted_costs = costs;
        sort(sorted_costs.begin(), sorted_costs.end());
        sorted_costs.erase(sorted_costs.begin() + elites, sorted_costs.end());
        vector<Point3f> elites_p;
        vector<Point3f> elites_o;
        for (auto& it : sorted_costs) {
            elites_p.push_back( sample_p[ find(costs.begin(), costs.end(), it) - costs.begin() ] );
            elites_o.push_back( sample_o[ find(costs.begin(), costs.end(), it) - costs.begin() ] );
        }
        Point3f new_mean_position = mean(elites_p);
        Point3f new_mean_orientation = mean(elites_o);
        mean_position = (alpha * mean_position) + ((1 - alpha) * new_mean_position);
        mean_orientation = (alpha * mean_orientation) + ((1 - alpha) * new_mean_orientation);

        Point3f new_var_position = var(elites_p);
        Point3f new_var_orientation = var(elites_o);
        var_position = (alpha * var_position) + ((1 - alpha) * new_var_position);
        var_orientation = (alpha * var_orientation) + ((1 - alpha) * new_var_orientation);
    }
}

Point3f Pose::mean(vector<Point3f> points) {
    Point3f p(0.,0.,0.);
    for (auto& it : points)
        p += it;
    float N = 1 / points.size();
    p = p * N;
    return p;
}

Point3f Pose::var(vector<Point3f> points) {
    Point3f m = mean(points);
    Point3f p(0.,0.,0.);
    for (auto& it : points)
        p += (it - m);
    float N = 1 / (points.size() - 1);
    p = p * N;
    return p;
}


void Pose::find_pose() {
    cem(); // calculates mean_position and mean_orientation
    Mat pose = getPoseMatrix(mean_orientation, mean_position);
    Mat proposed_new_cube_pts_w = pose * reference_center_Point_3d.t();
    proposed_new_cube_pts_w = proposed_new_cube_pts_w.t(); // 8x4
    proposed_new_cube_pts_w = proposed_new_cube_pts_w.colRange(0, proposed_new_cube_pts_w.cols - 1); // 8x3

    for (int i=0; i<R.cols; i++) { // range (r_vecs)
        vector<Point2f> imgpoints;
        cv::projectPoints(proposed_new_cube_pts_w.t(), R.col(i), T.col(i), K, D, imgpoints);
        projected_points.push_back(imgpoints);// ? projected_points.append(imgpoints[:, 0, :])
    }
}