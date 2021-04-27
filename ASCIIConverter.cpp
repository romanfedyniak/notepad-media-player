#include "ASCIIConverter.h"


ASCIIConverter::ASCIIConverter(const std::string& path, int charsWidth, int charsHeight)
	:
	path(path),
	charsWidth(charsWidth),
	charsHeight(charsHeight) {}

void ASCIIConverter::init()
{
	capture = cv::VideoCapture(path);
	fps = (int)capture.get(cv::CAP_PROP_FPS);
	length = (int)capture.get(cv::CAP_PROP_FRAME_COUNT);
	width = capture.get(cv::CAP_PROP_FRAME_WIDTH);
	height = capture.get(cv::CAP_PROP_FRAME_HEIGHT);

	widthStep = width / (double)charsWidth;
	heightStep = height / (double)charsHeight;
}

cv::Mat ASCIIConverter::getFrame()
{
	cv::Mat frame;
	capture.read(frame);
	return frame;
}

cv::Mat ASCIIConverter::getTransposedFrame()
{
	cv::Mat transposed_frame;
	cv::transpose(getFrame(), transposed_frame);
	return transposed_frame;
}

cv::Mat ASCIIConverter::getGrayImage()
{
	cv::Mat gray_scale;
	cv::cvtColor(getTransposedFrame(), gray_scale, cv::COLOR_BGR2GRAY);
	return gray_scale;
}

ASCIIConverter::pixel_vector* ASCIIConverter::getImage()
{
	cv::Mat gray_scale = getGrayImage();
	ASCIIConverter::pixel_vector* image = new ASCIIConverter::pixel_vector;
	for (double x = 0; x < width; x += widthStep)
	{
		for (double y = 0; y < height; y += heightStep)
		{
			uchar char_index = gray_scale.at<uchar>((int)x, (int)y) / asciiCoeff;
			if (char_index)
			{
				Pixel pixel;
				pixel.x = std::round(x / widthStep);
				pixel.y = std::round(y / heightStep);
				pixel.ch = asciiChar[char_index];
				image->push_back(pixel);
			}
		}
	}
	return image;
}