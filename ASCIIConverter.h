#pragma once
#ifndef _ASCII_CONVERTER_H_
#define _ASCII_CONVERTER_H_

#include <string>
#include <vector>
#include <opencv2/opencv.hpp>
#include <cmath>

class ASCIIConverter
{
private:
	const std::string asciiChar = " .,:+*?%S#@";
	const int asciiCoeff = int(255 / (asciiChar.length() - 1));
	int charsWidth, charsHeight, fps, length;
	double width, height, widthStep, heightStep;
	std::string path;
	cv::VideoCapture capture;

private:
	cv::Mat getFrame();
	cv::Mat getTransposedFrame();
	cv::Mat getGrayImage();

public:
	struct Pixel
	{
		int x, y;
		char ch;

		virtual ~Pixel() = default;
	};

	typedef std::vector<ASCIIConverter::Pixel> pixel_vector;

public:
	ASCIIConverter(const std::string& path, int charsWidth, int charsHeight);

	void init();
	pixel_vector* getImage();
	const int& getFPS() const { return fps; };
	const int& getFrameCount() const { return length; };
	virtual ~ASCIIConverter() = default;
};

#endif