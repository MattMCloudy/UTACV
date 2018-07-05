//
//    Copyright 2018 Christopher D. McMurrough
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

/*******************************************************************************************************************//**
 * @file cv_ellipse.cpp
 * @brief C++ example of Canny edge detection and ellipse model fitting in OpenCV
 * @author Christopher D. McMurrough
 **********************************************************************************************************************/

// include necessary dependencies
#include <iostream>
#include <string>
#include "opencv2/opencv.hpp"

// configuration parameters
#define NUM_COMNMAND_LINE_ARGUMENTS 1

/*******************************************************************************************************************//**
 * @brief program entry point
 * @param[in] argc number of command line arguments
 * @param[in] argv string array of command line arguments
 * @return return code (0 for normal termination)
 * @author Christoper D. McMurrough
 **********************************************************************************************************************/
int main(int argc, char **argv)
{
    cv::Mat imageIn;

    // validate and parse the command line arguments
    if(argc != NUM_COMNMAND_LINE_ARGUMENTS + 1)
    {
        std::printf("USAGE: %s <image_path> \n", argv[0]);
        return 0;
    }
    else
    {
        imageIn = cv::imread(argv[1], CV_LOAD_IMAGE_COLOR);

        // check for file error
        if(!imageIn.data)
        {
            std::cout << "Error while opening file " << argv[1] << std::endl;
            return 0;
        }
    }

    // get the image size
    std::cout << "image width: " << imageIn.size().width << std::endl;
    std::cout << "image height: " << imageIn.size().height << std::endl;
    std::cout << "image channels: " << imageIn.channels() << std::endl;

    // convert the image to grayscale
    cv::Mat imageGray;
    cv::cvtColor(imageIn, imageGray, cv::COLOR_BGR2GRAY);

    // normalize image
    cv::Mat imageNormalized;
    cv::normalize(imageGray, imageNormalized, 0, 255);

    // equalize image
    cv::Mat imageEqualized;
    cv::equalizeHist(imageNormalized, imageEqualized);

    // find the image edges
    cv::Mat imageEdges;
    const double cannyThreshold1 = 100;
    const double cannyThreshold2 = 200;
    const int cannyAperture = 3;
    cv::Canny(imageGray, imageEdges, cannyThreshold1, cannyThreshold2, cannyAperture);

    // locate the image contours (after applying a threshold or canny)
    std::vector<std::vector<cv::Point> > contours;
    cv::findContours(imageEdges, contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));

    // draw the contours
    cv::Mat imageContours = cv::Mat::zeros(imageEdges.size(), CV_8UC3);
    cv::RNG rand(12345);
    for(int i = 0; i < contours.size(); i++)
    {
        cv::Scalar color = cv::Scalar(rand.uniform(0, 256), rand.uniform(0,256), rand.uniform(0,256));
        cv::drawContours(imageContours, contours, i, color);
    }

    // fit ellipses to contours containing sufficient inliers
    std::vector<cv::RotatedRect> fittedEllipses(contours.size());
    for(int i = 0; i < contours.size(); i++)
    {
        // compute an ellipse only if the contour has more than 5 points (the minimum for ellipse fitting)
        if(contours.at(i).size() > 5)
        {
            fittedEllipses[i] = cv::fitEllipse(contours[i]);
        }
    }

    // eliminate ellipses that are too small
    std::vector<cv::RotatedRect> largeEllipses;
    const int minEllipseInliers = 500;
    for(int i = 0; i < contours.size(); i++) {
        if(contours.at(i).size() > minEllipseInliers) {
            std::cout << "Ellipse found with size: " << fittedEllipses[i].size << std::endl;
            largeEllipses.push_back(fittedEllipses[i]);
        }
    }

    // eliminate ellipses that are contained within other ellipses
    std::vector<cv::RotatedRect> coinEllipses;
    for(int i = 0; i < largeEllipses.size(); i++) {
        bool isInsideOtherEllipse = false;
        cv::Point2f center = largeEllipses[i].center;
        for(int j = 0; j < largeEllipses.size(); j++) {
            if (i == j) continue;

            // points are stored bottomLeft, topLeft, topRight, bottomRight
            cv::Point2f pts[4];
            largeEllipses[j].points(pts);
            if((center.x > pts[0].x && center.y > pts[0].y) 
                && (center.x < pts[2].x && center.y < pts[2].y))
            {
                isInsideOtherEllipse = true;
            }
        }

        if(!isInsideOtherEllipse) coinEllipses.push_back(largeEllipses[i]);
    }

    // draw the ellipses
    cv::Mat imageEllipse = cv::Mat::zeros(imageEdges.size(), CV_8UC3);
    for(int i = 0; i < coinEllipses.size(); i++)
    {
        cv::Scalar color = cv::Scalar(rand.uniform(0, 256), rand.uniform(0,256), rand.uniform(0,256));
        cv::ellipse(imageEllipse, coinEllipses[i], color, 2);
    }

    // display the images
    cv::imshow("imageIn", imageIn);
    cv::imshow("imageGray", imageGray);
    cv::imshow("imageEdges", imageEdges);
    cv::imshow("imageContours", imageContours);
    cv::imshow("imageEllipse", imageEllipse);
    cv::waitKey();
}
