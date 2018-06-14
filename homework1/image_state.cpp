#include <iostream>
#include <string>
#include "opencv2/opencv.hpp"

enum Tool {eyedropper, crop, pencil, paint_bucket, reset};

class ImageState {
    private:
        cv::Mat current_image;
        cv::Mat original_image;
        Tool current_tool;
    public:
        ImageState(cv::Mat image) : current_image(image), original_image(image), current_tool(eyedropper) {}
        cv::Mat getCurrentImage();
        cv::Mat getOriginalImage();
        void toggleTool();
        Tool getTool();
};

cv::Mat ImageState::getCurrentImage() {
    return current_image;
}

cv::Mat ImageState::getOriginalImage() {
    return original_image;
}

void ImageState::toggleTool() {
    if (current_tool != reset) {
        current_tool++;
    } else {
        current_tool = eyedropper;
    }
}

Tool ImageState::getTool() {
    return current_tool;
}