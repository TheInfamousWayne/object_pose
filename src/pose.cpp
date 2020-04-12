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

    this->K = Mat(3,3, CV_32FC1, &K);
    this->D = Mat(5,1, CV_32FC1, &D);
    this->R = Mat(3,3, CV_32FC1, &R);
    this->T = Mat(3,3, CV_32FC1, &T);

    // Setting the bounds for pose estimation
    position_lower_bound = Point3f(-0.5, -0.5, -0.5);
    position_upper_bound = Point3f(0.5, 0.5, 0.5);
    orientation_lower_bound = Point3f(-3.14159265, -3.14159265, -3.14159265);
    orientation_upper_bound = Point3f(3.14159265, 3.14159265, 3.14159265);

    mean_position = (position_upper_bound + position_lower_bound) / 2.0;
    mean_orientation = (orientation_upper_bound + orientation_lower_bound) / 2.0;

    var_position = (position_lower_bound - position_upper_bound)/4.0;
    var_orientation = (orientation_lower_bound - orientation_upper_bound)/4.0;

    var_position.x *= var_position.x;
    var_position.y *= var_position.y;
    var_position.z *= var_position.z;

    var_orientation.x *= var_orientation.x;
    var_orientation.y *= var_orientation.y;
    var_orientation.z *= var_orientation.z;

}

void Pose::cost_function() {

}


void Pose::cem() {
    int max_iterations = 30;
    int number_of_particles = 1000;
    int elites = 100;
    float alpha = 0.2;
    float eps = 5.0;




}

void Pose::find_pose() {
    cem(); // calculates mean_position and mean_orientation

}