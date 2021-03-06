#include <iostream>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

void gaussianFilterCallback(cv::Mat &originalImage, 
                            cv::Mat &outputImage,
                            int &gaussianFilterSize)
{
    int filterSize = 2*gaussianFilterSize + 1;
    cv::GaussianBlur(originalImage, outputImage, cv::Size(filterSize, filterSize), 0, 0);
}

void tracking(cv::Mat &outputImage,
              cv::Mat &binaryImage,
              int &areaThreshold)
{
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;

    cv::findContours(binaryImage, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));

    std::vector<cv::Moments> m(contours.size());
    for(int i = 0; i < contours.size(); i++)
    {
        m[i] = cv::moments(contours[i]);
    }

    std::vector<cv::Point2f> centroids(contours.size());
    for(int i = 0; i < contours.size(); i++)
    {
        float pos_x = m[i].m10/m[i].m00;
        float pos_y = m[i].m01/m[i].m00;
        centroids[i] = cv::Point2f(pos_x, pos_y);
    }

    for(int i = 0; i < contours.size(); i++)
    {
        if(cv::contourArea(contours[i]) > areaThreshold)
        {
            cv::drawContours(outputImage, contours, i, cv::Scalar(0, 0, 255), 2, 8, hierarchy, 0, cv::Point());
            cv::circle(outputImage, centroids[i], 5, cv::Scalar(0, 255, 0), -1);
        }
    }
}

int main(int argc, char *argv[])
{
    cv::VideoCapture webcam(0, cv::CAP_V4L2);
    cv::namedWindow("originalVideo", cv::WINDOW_AUTOSIZE);
    cv::namedWindow("gaussianFrame", cv::WINDOW_AUTOSIZE);
    cv::namedWindow("hsvVideo", cv::WINDOW_AUTOSIZE);
    cv::namedWindow("control", cv::WINDOW_NORMAL);
    cv::namedWindow("mask", cv::WINDOW_AUTOSIZE);

    if(!webcam.isOpened())
    {
        std::cout << "Erro em abrir o vídeo!" << std::endl;
        return -1;
    }

    double fps;
    fps = webcam.get(cv::CAP_PROP_FPS);
    std::cout << "FPS = " << fps << std::endl;

    int hLow = 0, sLow = 92, vLow = 135;
    int hHigh = 50, sHigh = 255, vHigh = 255;
    cv::createTrackbar("hLow", "control", &hLow, 255);
    cv::createTrackbar("hHigh", "control", &hHigh, 255);
    cv::createTrackbar("sLow", "control", &sLow, 255);
    cv::createTrackbar("sHigh", "control", &sHigh, 255);
    cv::createTrackbar("vLow", "control", &vLow, 255);
    cv::createTrackbar("vHigh", "control", &vHigh, 255);

    int gaussianFilterSize = 28;
    cv::createTrackbar("gaussianFilterSize", "control", &gaussianFilterSize, 200);

    int erodeSize = 28, dilateSize = 28;
    cv::createTrackbar("erodeSize", "control", &erodeSize, 200);
    cv::createTrackbar("dilateSize", "control", &dilateSize, 200);

    int areaThreshold = 500;
    cv::createTrackbar("areaThreshold", "control", &areaThreshold, 1000);

    while(true)
    {
        cv::Mat frame, gaussianFrame, hsvFrame, thresholdFrame;

        webcam.read(frame);

        gaussianFilterCallback(frame, gaussianFrame, gaussianFilterSize);

        cv::cvtColor(gaussianFrame,
                     hsvFrame,
                     cv::COLOR_BGR2HSV);

        cv::inRange(hsvFrame,
                    cv::Scalar(hLow, sLow, vLow),
                    cv::Scalar(hHigh, sHigh, vHigh),
                    thresholdFrame);

        cv::erode(thresholdFrame,
                  thresholdFrame,
                  cv::getStructuringElement(cv::MORPH_ELLIPSE, 
                                            cv::Size(erodeSize*2 + 1, erodeSize*2 + 1)));

        cv::dilate(thresholdFrame,
                   thresholdFrame,
                   cv::getStructuringElement(cv::MORPH_ELLIPSE, 
                                             cv::Size(dilateSize*2 + 1, dilateSize*2 + 1)));

        cv:tracking(frame, thresholdFrame, areaThreshold);

        cv::imshow("originalVideo", frame);
        cv::imshow("gaussianFrame", gaussianFrame);
        cv::imshow("hsvVideo", hsvFrame);
        cv::imshow("mask", thresholdFrame);

        if(cv::waitKey(10) == 'q')
        {
            std::cout << "Vídeo finalizado!" << std::endl;
            break;
        }
    }

    cv::destroyAllWindows();
    
    return 0;
}