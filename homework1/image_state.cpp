#include <iostream>
#include <string>
#include "opencv2/opencv.hpp"

enum Tool {eyedropper, crop, pencil, paint_bucket, reset};

class ImageState {
    private:
        cv::Mat current_image;
        cv::Mat original_image;
        int image_rows;
        int image_cols;
        Tool current_tool;
        cv::Vec3b eyedropper_color;
        cv::Point initial_crop_location;
        cv::Point final_crop_location;
        cv::Point updated_rectangle_point;
        cv::Rect crop_rectangle;
        bool pencil_active;
        cv::Vec3b anti_color;
    public:
        ImageState(cv::Mat image) : current_image(image.clone()), original_image(image),
            image_rows(image.rows), image_cols(image.cols), 
            current_tool(eyedropper), eyedropper_color(cv::Vec3b(255,255,255)), 
            pencil_active(false), anti_color(cv::Vec3b(0,0,0)) {}
        cv::Mat getCurrentImage();
        cv::Mat getOriginalImage();
        void toggleTool();
        Tool getTool();
        void setEyedropperColor(int x, int y);
        cv::Vec3b getEyedropperColor();
        void cropLeftClicked(int x, int y);
        void executeCrop(int x, int y);
        void setPencilActive(bool active);
        bool getPencilActive();
        void pencilDraw(int x, int y);
        void paintBucketFill(int x, int y);
        void paintBucketFillRecursive(int x, int y);
        void resetImage();
        bool inRange(int x, int y);
};

cv::Mat ImageState::getCurrentImage() {
    return current_image;
}

cv::Mat ImageState::getOriginalImage() {
    return original_image;
}

void ImageState::toggleTool() {
    if (current_tool != reset) {
        int tool_int = current_tool;
        current_tool = static_cast<Tool>(++tool_int);
    } else {
        current_tool = eyedropper;
    }
}

Tool ImageState::getTool() {
    return current_tool;
}

void ImageState::setEyedropperColor(int x, int y) {
    eyedropper_color = current_image.at<cv::Vec3b>(y, x);
}

cv::Vec3b ImageState::getEyedropperColor() {
    return eyedropper_color;
}

void ImageState::cropLeftClicked(int x, int y) {
    std::cout << "New initial crop location set to row: " << y << "col: " << x << std::endl;
    initial_crop_location = cv::Point(x,y);
}

void ImageState::executeCrop(int x, int y) {
    std::cout << "Left Mouse Click Released...  Attempting to execute crop from" << initial_crop_location << " to row: " << y << " col: " << x << std::endl;
    final_crop_location = cv::Point(x,y);
    crop_rectangle = cv::Rect(initial_crop_location, final_crop_location);
    current_image = current_image(crop_rectangle);
    cv::imshow("imageIn", current_image);
}

void ImageState::setPencilActive(bool active) {
    pencil_active = active;
}

bool ImageState::getPencilActive() {
    return pencil_active;
}

void ImageState::pencilDraw(int x, int y) {
    current_image.at<cv::Vec3b>(y,x) = eyedropper_color;
    cv::imshow("imageIn", current_image);
}

bool ImageState::inRange(int x, int y) {
    return (x <= image_rows && x >= 0) && (y <= image_cols && y >= 0);
}

void ImageState::paintBucketFill(int x, int y) {
    anti_color = current_image.at<cv::Vec3b>(y,x);
    std::cout << "Attempting paint bucket fill..." << std::endl;
    paintBucketFillRecursive(x,y);
    cv::imshow("imageIn", current_image);
}

void ImageState::paintBucketFillRecursive(int x, int y) {
    //does will not access out of range values due to short-circuit eval
    if(!inRange(x,y) || current_image.at<cv::Vec3b>(y,x) == anti_color) {
        return;
    }
    
    current_image.at<cv::Vec3b>(y,x) = eyedropper_color;
    paintBucketFillRecursive(x-1,y);
    paintBucketFillRecursive(x,y-1);
    paintBucketFillRecursive(x+1,y);
    paintBucketFillRecursive(x,y+1);
}

void ImageState::resetImage() {
    std::cout << "Attempting reset..." << std::endl;
    current_image = original_image.clone();
    cv::imshow("imageIn", current_image);
}