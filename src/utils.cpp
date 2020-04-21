#include <utils.hpp>

Mat get_image(const string& path, const int idx) {
    Mat image;
    image = imread(path + "/image_" + to_string(idx) + ".jpg", 1);
    if ( !image.data )
    {
        cout<< (path + "/image_" + to_string(idx) + ".jpg");
        printf("No image data \n");
        exit(0);
    }
     cvtColor(image, image, COLOR_BGR2RGB);
    return image;
}

vector<string> get_directories(const string& s)
{
    vector<string> r;
    for(auto p : fs::recursive_directory_iterator(s))
        if (fs::is_directory(p))
            r.push_back(p.path().string());
    return r;
}
