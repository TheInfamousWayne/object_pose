#include <utils.hpp>
#include <image.hpp>
#include <pose.hpp>
#include <thread>

int main(int argc, char** argv ) {
    cout << "Current path is " << fs::current_path() << "\n";
    vector<string> camera_data = get_directories("../data/camera");

    for (size_t i = 0; i < 3; ++i) {
        // getting frames from three cameras
        vector<Mat> frames;
        for (string path : camera_data) {
            cout << path << "\n";
            frames.push_back( get_image(path, i+1) );
        }
        // sending frames color segmentation
        vector<Image> images;
        int thread_type = 0; // 1 = multi-threaded

        if (thread_type == 0) {
            for (auto& image : frames) {
                Image obj(image);
                obj.startSingleThread();
                images.push_back(obj);
            }
        }
        else {
            vector<thread> vecOfThreads;
            for (auto& image : frames) {
                Image obj(image);
                images.push_back(obj);
                thread th(&Image::startSingleThread, images.back());
                vecOfThreads.push_back(move(th));
            }

            // waiting for threads to finish
            for (thread& th : vecOfThreads) {
                if (th.joinable())
                    th.join();
            }
        }

        // waiting for random input
        int a;
        cin >> a;

        // using stored objects to call class functions
//        for (auto obj : images) {
//            obj.print_pixels();
//        }

        cout << "end" << "\n";

//        Pose Detection from below
        Pose pose(images);
        pose.find_pose();




    }
    return 0;
}