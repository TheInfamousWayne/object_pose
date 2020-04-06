#include <header.hpp>
#include <image.hpp>
#include <thread>

int main(int argc, char** argv ) {
    cout << "Current path is " << fs::current_path() << "\n";
    vector<string> camera_data = get_directories("../data/camera");

    for (size_t i = 0; i < 3; ++i) {
        // getting frames from three cameras
        vector<Mat> frames;
        for (string path : camera_data) {
            frames.push_back( get_image(path, i+1) );
        }
        // sending frames color segmentation
        vector<Image> images;
        vector<thread> vecOfThreads;
        for (auto& image : frames) {
            images.push_back(Image(image));
            thread th(&Image::startSingleThread, images.back());
            vecOfThreads.push_back(move(th));
        }
        // waiting for threads to finish
        for (thread& th : vecOfThreads) {
            if (th.joinable())
                th.join();
        }
        cout << "end" << "\n";
    }
    return 0;
}