#include <header.h>
#include <image.hpp>
#include <thread>

int main(int argc, char** argv )
{
    if ( argc != 2 )
    {
        printf("usage: DisplayImage.out <Image_Path>\n");
        return -1;
    }

    // cout << "Current path is " << fs::current_path() << "\n";
    vector<string> camera_data = get_directories("object_pose/data/camera");




    for (size_t i = 0; i < 3; ++i) {
        // getting frames from three cameras
        vector<Mat> frames;
        for (string path : camera_data) {
            frames.push_back( get_image(path, i+1) );
        }

        // for (auto& image : frames) {
        //     Image* img = new Image(image);
        //     img->start();
        // }


        // send this frame set for color segmentation
        vector<thread> vecOfThreads;
        for (auto& image : frames) {
            thread th(&Image::start, Image(image));
            vecOfThreads.push_back(move(th));
        }
        for (thread& th : vecOfThreads) {
            if (th.joinable())
                th.join();
        }

        
    }

    return 0;
}