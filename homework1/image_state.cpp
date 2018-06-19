#include <iostream>
#include <string>
#include "opencv2/opencv.hpp"

enum Tool {eyedropper, crop, pencil, paint_bucket, reset};

class ImageState {
    private:
        cv::Mat current_image;
        cv::Mat original_image;
        Tool current_tool;
        cv::Vec3b eyedropper_color;
        cv::Point initial_crop_location;
        cv::Point final_crop_location;
        cv::Point updated_rectangle_point;
        cv::Rect crop_rectangle;
    public:
        ImageState(cv::Mat image) : current_image(image), original_image(image), 
            current_tool(eyedropper), eyedropper_color(cv::Vec3b(255,255,255)) {}
        cv::Mat getCurrentImage();
        cv::Mat getOriginalImage();
        void toggleTool();
        Tool getTool();
        void setEyedropperColor(int x, int y);
        cv::Vec3b getEyedropperColor();
        void cropLeftClicked(int x, int y);
        void executeCrop(int x, int y);
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