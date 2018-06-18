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
    public:
        ImageState(cv::Mat image) : current_image(image), original_image(image), 
            current_tool(eyedropper), eyedropper_color((u_char*) [255, 255, 255]) {}
        cv::Mat getCurrentImage();
        cv::Mat getOriginalImage();
        void toggleTool();
        Tool getTool();
        void setEyedropperColor(int x, int y);
        cv::Vec3b getEyedropperColor();
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