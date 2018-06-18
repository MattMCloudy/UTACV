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
        bool cropLeftMouseClicked;
        cv::Point initial_crop_location;
        cv::Point final_crop_location;
        cv::Rect crop_rectangle;
    public:
        ImageState(cv::Mat image) : current_image(image), original_image(image), 
            current_tool(eyedropper), eyedropper_color(cv::Vec3b(255,255,255)), cropLeftMouseClicked(FALSE){}
        cv::Mat getCurrentImage();
        cv::Mat getOriginalImage();
        void toggleTool();
        Tool getTool();
        void setEyedropperColor(int x, int y);
        cv::Vec3b getEyedropperColor();
        void cropLeftClicked(int x, int y);
        bool getCropLeftClicked();
        void executeCrop(int x, int y);
        void updateRectangle(int x, int y);
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
    cropLeftMouseClicked = true;
    initial_crop_location = cv::Point(x,y);
    crop_rectangle = cv::Rect(initial_crop_location, initial_crop_location)
    cv::rectangle(current_image, crop_rectangle, cv::Scalar(0, 255, 0));
}

bool ImageState::getCropLeftClicked() {
    return cropLeftMouseClicked;
}

void ImageState::executeCrop(int x, int y) {
    cropLeftMouseClicked = false;
    final_crop_location = cv::Point(x,y);
    crop_rectangle = cv::Rect(initial_crop_location, final_crop_location);
    cv::rectangle(current_image, crop_rectangle, cv::Scalar(0, 255, 0));
    current_image = current_image(crop_rectangle);
    cv::imshow("cropped_image", current_image);
}

void ImageState::updateRectangle(int x, int y) {
    updated_rectangle_point = cv::Point(x,y);
    crop_rectangle = cv::Rect(initial_crop_location, updated_rectangle_point);
    cv::rectangle(current_image, crop_rectangle, cv::Scalar(0, 255, 0));
}