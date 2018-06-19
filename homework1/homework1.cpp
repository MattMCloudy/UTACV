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
 * @file cv_clickable.cpp
 * @brief C++ example of basic interaction with an image in OpenCV
 * @author Christopher D. McMurrough
 **********************************************************************************************************************/

// include necessary dependencies
#include <iostream>
#include <string>
#include "opencv2/opencv.hpp"
#include "image_state.cpp"

// function prototypes
static void clickCallback(int event, int x, int y, int flags, void* userdata);
static void eyedropperTool(int event, int x, int y, int flags, void* userdata);
static void cropTool(int event, int x, int y, int flags, void* userdata);
static void pencilTool(int event, int x, int y, int flags, void* userdata);
static void paintBucketTool(int event, int x, int y, int flags, void* userdata);
static void resetTool(int event, int x, int y, int flags, void* userdata);

static void eyedropperTool(int event, int x, int y, int flags, void* userdata) {
    ImageState* imageStateRef = (ImageState*) userdata;

    if(event == cv::EVENT_LBUTTONDOWN)
    {
        std::cout << "Setting eyedropper color..." << std::endl;
        imageStateRef->setEyedropperColor(x,y);
        std::cout << "Eyedropper color set to " << imageStateRef->getEyedropperColor() << std::endl;
    }
    else if(event == cv::EVENT_RBUTTONDOWN)
    {
        imageStateRef->toggleTool();
        std::cout << "Tool is now set to " << imageStateRef->getTool() << std::endl;
    }
    else if(event == cv::EVENT_MBUTTONDOWN)
    {
    }
    else if(event == cv::EVENT_MOUSEMOVE)
    {
    }
}

static void cropTool(int event, int x, int y, int flags, void* userdata) {
    ImageState* imageStateRef = (ImageState*) userdata;

    if(event == cv::EVENT_LBUTTONDOWN)
    {
        imageStateRef->cropLeftClicked(x,y);
    }
    else if (event == cv::EVENT_LBUTTONUP)
    {
        imageStateRef->executeCrop(x,y);
    }
}


static void pencilTool(int event, int x, int y, int flags, void* userdata) {
    ImageState* imageStateRef = (ImageState*) userdata;

    if(event == cv::EVENT_LBUTTONDOWN)
    {
        std::cout << "LEFT CLICK (" << x << ", " << y << ")" << std::endl;
    }
    else if(event == cv::EVENT_RBUTTONDOWN)
    {
        imageStateRef->toggleTool();
        std::cout << "Tool is now set to " << imageStateRef->getTool() << std::endl;
    }
    else if(event == cv::EVENT_MBUTTONDOWN)
    {
        std::cout << "MIDDLE CLICK (" << x << ", " << y << ")" << std::endl;
    }
    else if(event == cv::EVENT_MOUSEMOVE)
    {
        std::cout << "MOUSE OVER (" << x << ", " << y << ")" << std::endl;
    }
}


static void paintBucketTool(int event, int x, int y, int flags, void* userdata) {
    ImageState* imageStateRef = (ImageState*) userdata;

    if(event == cv::EVENT_LBUTTONDOWN)
    {
        std::cout << "LEFT CLICK (" << x << ", " << y << ")" << std::endl;
    }
    else if(event == cv::EVENT_RBUTTONDOWN)
    {
        imageStateRef->toggleTool();
        std::cout << "Tool is now set to " << imageStateRef->getTool() << std::endl;
    }
    else if(event == cv::EVENT_MBUTTONDOWN)
    {
        std::cout << "MIDDLE CLICK (" << x << ", " << y << ")" << std::endl;
    }
    else if(event == cv::EVENT_MOUSEMOVE)
    {
        std::cout << "MOUSE OVER (" << x << ", " << y << ")" << std::endl;
    }
}


static void resetTool(int event, int x, int y, int flags, void* userdata) {
    ImageState* imageStateRef = (ImageState*) userdata;

    if(event == cv::EVENT_LBUTTONDOWN)
    {
        std::cout << "LEFT CLICK (" << x << ", " << y << ")" << std::endl;
    }
    else if(event == cv::EVENT_RBUTTONDOWN)
    {
        imageStateRef->toggleTool();
        std::cout << "Tool is now set to " << imageStateRef->getTool() << std::endl;
    }
    else if(event == cv::EVENT_MBUTTONDOWN)
    {
        std::cout << "MIDDLE CLICK (" << x << ", " << y << ")" << std::endl;
    }
    else if(event == cv::EVENT_MOUSEMOVE)
    {
        std::cout << "MOUSE OVER (" << x << ", " << y << ")" << std::endl;
    }
}


/*******************************************************************************************************************//**
 * @brief handler for image click callbacks
 * @param[in] event number of command line arguments
 * @param[in] x string array of command line arguments
 * @param[in] y string array of command line arguments
 * @param[in] flags string array of command line arguments
 * @param[in] userdata string array of command line arguments
 * @return return code (0 for normal termination)
 * @author Christoper D. McMurrough
 **********************************************************************************************************************/
static void clickCallback(int event, int x, int y, int flags, void* userdata)
{
    ImageState* imageStateRef = (ImageState*) userdata;

    switch (imageStateRef->getTool()) {
        case eyedropper:
            eyedropperTool(event, x, y, flags, userdata);
            break;
        case crop:
            cropTool(event, x, y, flags, userdata);
            break;
        case pencil:
            pencilTool(event, x, y, flags, userdata);
            break;
        case paint_bucket:
            paintBucketTool(event, x, y, flags, userdata);
            break;
        case reset:
            resetTool(event, x, y, flags, userdata);
            break;
        default:
            std::cout << "Tool State Unknown, please try again" << std::endl;
            break;
    }
}

/*******************************************************************************************************************//**
 * @brief program entry point
 * @param[in] argc number of command line arguments
 * @param[in] argv string array of command line arguments
 * @return return code (0 for normal termination)
 * @author Christoper D. McMurrough
 **********************************************************************************************************************/
int main(int argc, char **argv)
{
    // open the input image
    std::string inputFileName = "test.png";
	cv::Mat imageIn;
	imageIn = cv::imread(inputFileName, CV_LOAD_IMAGE_COLOR);

	// check for file error
	if(!imageIn.data)
	{
		std::cout << "Error while opening file " << inputFileName << std::endl;
		return 0;
	}

    ImageState* imageState = new ImageState(imageIn);

	
    // display the input image
	cv::imshow("imageIn", imageState->getCurrentImage());
	cv::setMouseCallback("imageIn", clickCallback, imageState);
	cv::waitKey();
}

