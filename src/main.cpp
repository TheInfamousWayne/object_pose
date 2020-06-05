#include <utils.hpp>
#include <image.hpp>
#include <pose.hpp>
#include <thread>


int main(int argc, char** argv ) {

    vector<string> camera_data = get_directories("../data/cube_dataset_real_cube");

    bool generate_dataset = false;

    if (generate_dataset) {
        map<string, Mat> training_dataset_for_gmm;
        for (string folder_path : camera_data) {
            training_dataset_for_gmm = get_color_masks(folder_path, training_dataset_for_gmm);
        }
        train_gmm(training_dataset_for_gmm);
    }

    if (!generate_dataset) {
        for (string folder_path : camera_data) {
            // getting frames from three cameras
            vector <Mat> frames = get_images(folder_path);

//            int ii = 0;
//            for (auto& img : frames) {
//                cv::imshow("0"+ii, img);
//                cv::waitKey(0);
//            }

            // sending frames color segmentation
            vector <Image> images;
            int thread_type = 0; // 1 = multi-threaded

            if (thread_type == 0) {
                for (auto &image : frames) {
                    Image obj(image);
                    obj.startSingleThread();
                    images.push_back(obj);
                }
            } else {
                vector <thread> vecOfThreads;
                int ii = 0;
                for (auto &image : frames) {
                    Image obj(image);
                    images.push_back(obj);
                    thread th(&Image::startSingleThread, images.back());
                    vecOfThreads.push_back(move(th));
                }

                // waiting for threads to finish
                for (thread &th : vecOfThreads) {
                    if (th.joinable())
                        th.join();
                }
            }


            cout << "Segmentation complete" << "\n";

//            // Pose Detection from below
//            Pose pose(images);
//            pose.find_pose();
//
//            cout << "Pose detected\n";
//
//            for (int i = 0; i < images.size(); i++) {
//                vector <Point2f> imgpoints = pose.projected_points[i];
//                for (auto &it : images[i].object_model) {
//
//                ax.plot([imgpoints[it.second.first].x, imgpoints[it.second.second].y] , [imgpoints[it.second.first].y, imgpoints[it.second.second].x])
//                cvtColor(plotdata, plotdata, COLOR_RGB2BGR);
//                }
//            }

        }
    }
    return 0;
}
