//
//    Copyright 2018 Christopher D. McMurrough
#include <iostream>
#include <string>
#include "opencv2/opencv.hpp"

#define NUM_COMNMAND_LINE_ARGUMENTS 1

int main(int argc, char **argv)
{
    cv::Mat imageIn;
    double model[5];

    if(argc != NUM_COMNMAND_LINE_ARGUMENTS + 2)
    {
        std::printf("USAGE: %s <image_path> <model>\n", argv[0]);
        return 0;
    }
    else
    {
        imageIn = cv::imread(argv[1], CV_LOAD_IMAGE_COLOR);

        if(!imageIn.data)
        {
            std::cout << "Error while opening file " << argv[1] << std::endl;
            return 0;
        }

        std::ifstream in(argv[2]);

        if(!in) {
            std::cout << "Error opening model file " << argv[2] << std::endl;
            return 0;
        }
        
        std::string str;
        int iter = 0;
        while(std::getline(in, str)) {
            model[iter] = stod(str);
            iter++;
        }
    }

    std::cout << "image width: " << imageIn.size().width << std::endl;
    std::cout << "image height: " << imageIn.size().height << std::endl;
    std::cout << "image channels: " << imageIn.channels() << std::endl;

    cv::Mat imageGray;
    cv::cvtColor(imageIn, imageGray, cv::COLOR_BGR2GRAY);

    cv::Mat imageNormalized;
    cv::normalize(imageGray, imageNormalized, 0, 255);

    cv::Mat imageEqualized;
    cv::equalizeHist(imageNormalized, imageEqualized);

    cv::Mat imageEdges;
    const double cannyThreshold1 = 100;
    const double cannyThreshold2 = 200;
    const int cannyAperture = 3;
    cv::Canny(imageGray, imageEdges, cannyThreshold1, cannyThreshold2, cannyAperture);

    std::vector<std::vector<cv::Point> > contours;
    cv::findContours(imageEdges, contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));

    cv::Mat imageContours = cv::Mat::zeros(imageEdges.size(), CV_8UC3);
    cv::RNG rand(12345);
    for(int i = 0; i < contours.size(); i++)
    {
        cv::Scalar color = cv::Scalar(rand.uniform(0, 256), rand.uniform(0,256), rand.uniform(0,256));
        cv::drawContours(imageContours, contours, i, color);
    }

    std::vector<cv::RotatedRect> minAreaRectangles(contours.size());
    for(int i = 0; i < contours.size(); i++)
    {
        minAreaRectangles[i] = cv::minAreaRect(contours[i]);
    }
    
    cv::Mat imageRectangles = cv::Mat::zeros(imageEdges.size(), CV_8UC3);
    for(int i = 0; i < contours.size(); i++)
    {
        cv::Scalar color = cv::Scalar(rand.uniform(0, 256), rand.uniform(0,256), rand.uniform(0,256));
        cv::Point2f rectanglePoints[4];
        minAreaRectangles[i].points(rectanglePoints);
        for(int j = 0; j < 4; j++)
        {
            cv::line(imageRectangles, rectanglePoints[j], rectanglePoints[(j+1) % 4], color);
        }
    }

    std::vector<cv::RotatedRect> fittedEllipses(contours.size());
    for(int i = 0; i < contours.size(); i++)
    {
        if(contours.at(i).size() > 500)
        {
            fittedEllipses[i] = cv::fitEllipse(contours[i]);
        }
    }

    std::vector<cv::RotatedRect> normalEllipses;
    for(int i = 0; i < fittedEllipses.size(); i++) {
        if(fittedEllipses[i].size.height < 1000 && fittedEllipses[i].size.width < 1000) {
            std::cout << "Ellipse found with size: " << fittedEllipses[i].size << std::endl;
            normalEllipses.push_back(fittedEllipses[i]);
        }
    }  

    std::vector<cv::RotatedRect> coinEllipses;
    for(int i = 0; i < normalEllipses.size(); i++) {
        bool isInsideOtherEllipse = false;
        cv::Point2f center = normalEllipses[i].center;
        for(int j = 0; j < normalEllipses.size(); j++) {
            if (i == j) continue;

            cv::Point2f pts[4];
            normalEllipses[j].points(pts);
            if(((center.x > pts[0].x && center.y < pts[0].y) 
                && (center.x < pts[2].x && center.y > pts[2].y))
                || ((center.x > pts[1].x && center.y > pts[1].y)
                && (center.x < pts[4].x && center.y < pts[4].y)))
            {
                std::cout << "Eliminated contained ellipse" << std::endl;
                isInsideOtherEllipse = true;
            }
        }

        if(!isInsideOtherEllipse) coinEllipses.push_back(normalEllipses[i]);
    }

    std::vector<double> ellipseDiameters;
    for(int i = 0; i < coinEllipses.size(); i++) {
        cv::Point2f pts[4];
        coinEllipses[i].points(pts);
        double euclideanDistance = sqrt( pow((pts[2].x - pts[0].x), 2) + pow((pts[0].y - pts[2].y), 2) );
        std::cout << "Ellipse Diameter: " << euclideanDistance << std::endl;
        ellipseDiameters.push_back(euclideanDistance);
    }

    enum CoinType {penny, nickel, dime, quarter};
    int coinCount[4] = {0,0,0,0};
    int ellipseAssignments[ellipseDiameters.size()];
    for(int i = 0; i < ellipseDiameters.size(); i++) {
        double currentDiameter = ellipseDiameters[i];
        std::vector<double> sumOfSquaresError(4);
        for(int coinInt = penny; coinInt != quarter+1; coinInt++) {
            sumOfSquaresError.at(coinInt) = pow(model[coinInt] - currentDiameter, 2);
            std::cout << "Error from " << coinInt << " is " << sumOfSquaresError.at(coinInt) << std::endl;
        }
        ellipseAssignments[i] = std::distance( sumOfSquaresError.begin(), 
                                    std::min_element(sumOfSquaresError.begin(), sumOfSquaresError.end()));
        std::cout << "Ellipse assigned to coin: " << ellipseAssignments[i] << std::endl;
        coinCount[ellipseAssignments[i]]++;
    }

    double total = 0;
    for(int coinInt = penny; coinInt != quarter+1; coinInt++) {
        switch(static_cast<CoinType>(coinInt)) {
            case penny:
                std::cout << "There are " << coinCount[coinInt] << " pennies" << std::endl;
                total += 0.01 * coinCount[coinInt];
                break;
            case nickel:
                std::cout << "There are " << coinCount[coinInt] << " nickels" << std::endl;
                total += 0.05 * coinCount[coinInt];
                break;
            case dime:
                std::cout << "There are " << coinCount[coinInt] << " dimes" << std::endl;
                total += 0.1 * coinCount[coinInt];
                break;
            case quarter:
                std::cout << "There are " << coinCount[coinInt] << " quarters" << std::endl;
                total += 0.25 * coinCount[coinInt];
                break;
            default:
                break;      
        }
    }
    std::cout << "There is $" << total << " shown in the image!" << std::endl;

    cv::Mat imageEllipse = cv::Mat::zeros(imageEdges.size(), CV_8UC3);
    for(int i = 0; i < coinEllipses.size(); i++)
    {
        cv::Scalar color;
        switch(static_cast<CoinType>(ellipseAssignments[i])) {
            case penny:
                color = cv::Scalar(0,0,256);
                cv::ellipse(imageEllipse, coinEllipses[i], color, 2);
                break;
            case nickel:
                color = cv::Scalar(0,256,256);
                cv::ellipse(imageEllipse, coinEllipses[i], color, 2);
                break;
            case dime:
                color = cv::Scalar(256,0,0);
                cv::ellipse(imageEllipse, coinEllipses[i], color, 2);
                break;
            case quarter:
                color = cv::Scalar(0,256,0);
                cv::ellipse(imageEllipse, coinEllipses[i], color, 2);
                break;
            default:
                break;
        }
    }

    cv::imshow("imageIn", imageIn);
    cv::imshow("imageGray", imageGray);
    cv::imshow("imageEdges", imageEdges);
    cv::imshow("imageContours", imageContours);
    cv::imshow("imageRectangles", imageRectangles);
    cv::imshow("imageEllipse", imageEllipse);
    cv::waitKey();
}
