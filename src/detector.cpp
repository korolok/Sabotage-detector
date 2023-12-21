#include <iostream>
#include <opencv2/opencv.hpp>
#include <unistd.h>
#include <chrono>

int closeCam(cv::Mat other_frame, int best_cntr);
int shiftCam(cv::Mat best_frame, cv::Mat frame);
void getBest(cv::VideoCapture camera, cv::Mat &best, int &best_cntr);

int main(int argc, char**argv)
{
    cv::Mat best;
    cv::Mat frame;
    cv::Mat other_Frame;
    cv::VideoCapture camera;
    bool flag = false;
    
    std::string cam_url = "rtsp://admin:Admin1234@10.24.72.84:554/ch01.264?dev=1";
    std::string message_err;

    int timestamp;
    int best_cntr;

    if (!camera.open(cam_url))
    {
        std::cout << "not work" << std::endl;
        return -1;
    }

    getBest(camera, best, best_cntr);


    while (1)
    {
        auto t1 = std::chrono::steady_clock::now();
        
        camera >> frame;

        cv::resize(frame, frame, cv::Size(800, 600));

        cv::putText(frame, message_err, cv::Point(10, 20), cv::FONT_HERSHEY_SIMPLEX, 0.75, cv::Scalar(0,0,255),2);

        if (timestamp >= 1000)
        {
            timestamp = 0;

            cv::cvtColor(frame, other_Frame, cv::COLOR_RGB2GRAY);

            if (closeCam(other_Frame, best_cntr))
            {
                if (flag == true)
                {
                    message_err = "SABOTAGE Close or Defocus Cam";
                    std::cout << "SABOTAGE Close or Defocus Cam" << std::endl;
                    flag = false;
                }
                else
                {
                    flag = true;
                }
            }

            else if (shiftCam(best, other_Frame))
            {
                message_err = "SABOTAGE Shift Cam";
                std::cout << "SABOTAGE Shift Cam" << std::endl;
            }

            else
            {
                flag = false;
                message_err = "";
            }
        }

        cv::imshow("Output Window",  frame);

        if(cv::waitKey(1) >= 0) 
            break;

        auto t2 = std::chrono::steady_clock::now();
        timestamp += std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
    }

    cv::destroyAllWindows();

    return 0;
}

int closeCam(cv::Mat other_frame, int best_cntr)
{
    std::vector<std::vector<cv::Point>> cnts;

    cv::GaussianBlur(other_frame, other_frame, cv::Size(7, 7), 0);
    cv::adaptiveThreshold(other_frame, other_frame, 200, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY_INV, 9, 2);

    cv::findContours(other_frame, cnts, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    if (cnts.size() < best_cntr)
    {   
        return 1;
    }
    
    return 0;
}

int shiftCam(cv::Mat best_frame, cv::Mat frame)
{
    cv::Mat hann, prev64f, curr64f;

    createHanningWindow(hann, frame.size(), CV_64F);

    best_frame.convertTo(prev64f, CV_64F);
    frame.convertTo(curr64f, CV_64F);

    cv::Point2d shift = phaseCorrelate(prev64f, curr64f, hann);
    double radius = std::sqrt(shift.x*shift.x + shift.y*shift.y);

    if(radius > 20)
    {
        return 1;
    }

    return 0;
}

void getBest(cv::VideoCapture camera, cv::Mat &best, int &best_cntr)
{
    std::vector<std::vector<cv::Point>> cont;
    camera >> best;

    cv::resize(best, best, cv::Size(800, 600));
    cv::cvtColor(best, best, cv::COLOR_RGB2GRAY);

    cv::GaussianBlur(best, best, cv::Size(7, 7), 0);
    cv::adaptiveThreshold(best, best, 200, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY_INV, 9, 2);

    cv::findContours(best, cont, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    
    best.convertTo(best, CV_64F);
    best_cntr = cont.size() / 1.5;
}