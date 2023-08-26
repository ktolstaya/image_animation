#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include "core.hpp"
#include "highgui.hpp"
#include "imgproc.hpp"
#include "opencv.hpp"

struct Star
{
	int x;
	int y;
	int radius;
	double speed;
	Star(int ix, int iy, int ir, double is)
	{
		x = ix;
		y = iy;
		radius = ir;
		speed = is;
	}
};

std::vector<Star> stars{ Star(489, 55, 39, -2.), Star(285, 337, 56, 1.) };
int N_FRAMES = 17;

cv::Mat rotate_image(cv::Mat& image, double angle)
{
	auto msize = image.size();
	cv::Point2f image_center(msize.width / 2.f, msize.height / 2.f);
	auto rot_mat = cv::getRotationMatrix2D(image_center, angle, 1.0);
	cv::Mat result;
	cv::warpAffine(image, result, rot_mat, image.size(), cv::INTER_CUBIC);
	return result;
}

void alphaBlend(cv::Mat& foreground, cv::Mat& background, cv::Mat& alpha, cv::Mat& out)
{
	out = background.clone();
	int numberOfPixels = foreground.rows * foreground.cols * foreground.channels();
	unsigned char* fptr = reinterpret_cast<unsigned char*>(foreground.data);
	unsigned char* bptr = reinterpret_cast<unsigned char*>(background.data);
	float* aptr = reinterpret_cast<float*>(alpha.data);
	unsigned char* outImagePtr = reinterpret_cast<unsigned char*>(out.data);
	for (auto i = 0; i < numberOfPixels; i++, outImagePtr++, fptr++, aptr++, bptr++)
		*outImagePtr = static_cast<unsigned char>((*aptr) * (*fptr) + (*bptr) * (1 - *aptr));
}

int main(int argc, char** argv)
{
	if (argc != 5)
	{
		std::cout << "Usage: animate.exe path_to_image    path_to_angles    path_to_star_mask   n_frames\n";
		return 0;
	}

	char* path_to_image = argv[1];
	char* path_to_angles = argv[2];
	char* path_to_star_mask = argv[3];
	int total_frames = std::atoi(argv[4]);
		
	cv::Mat angles;
	cv::FileStorage fs(path_to_angles, cv::FileStorage::READ);
	if (!fs.isOpened()) 
	{
		fs.release();
		std::cout << "Can not open angles file " << path_to_angles << std::endl;
		return -1;
	}
	fs["angles"] >> angles;
	fs.release();

	cv::Mat img, star_mask;
	try
	{
		img = cv::imread(path_to_image);
	}
	catch (const std::exception& e)
	{
		std::cout << "Error reading image: " << e.what() << std::endl;
	}
	try
	{
		star_mask = cv::imread(path_to_star_mask);
	}
	catch (const std::exception& e)
	{
		std::cout << "Error reading star mask: " << e.what() << std::endl;
	}

	// get Cartesian coordinates of motion vectors using angles
	cv::Mat magnitude = cv::Mat::ones(angles.size(), angles.type());
	magnitude.setTo(cv::Scalar(0.0), cv::abs(angles) < std::numeric_limits<float>::epsilon());
	cv::Mat vx, vy;
	cv::polarToCart(magnitude*0.5, angles, vx, vy);
	for (auto j = 0; j < vx.rows; j++)
	{
		auto* vx_row = vx.ptr<float>(j);
		auto* vy_row = vy.ptr<float>(j);
		for (auto i = 0; i < vx.cols; i++) {
			vx_row[i] += j;
			vy_row[i] += i;
		}
	}

	// prepare images with motion
	cv::Mat img0;
	img.copyTo(img0);
	cv::Mat mask;
	cv::cvtColor(magnitude, mask, cv::COLOR_GRAY2BGR);
	auto disocclusion_mask = mask.clone();
	std::vector<cv::Mat> imgs;
	for (auto k = 0; k < N_FRAMES; k++)
	{
		cv::remap(img, img, vy, vx, cv::INTER_CUBIC);
		cv::remap(disocclusion_mask, disocclusion_mask, vy, vx, cv::INTER_LINEAR);
		img0.copyTo(img, mask - disocclusion_mask > 0);
		imgs.push_back(img.clone());

	}

	// blend images for infinite motion and attach stars
	star_mask.convertTo(star_mask, CV_32FC3, 1.0 / 255);
	for (auto f = 0; f < total_frames; f++)
	{

		double alpha;
		if (f % N_FRAMES <= N_FRAMES / 2)
			alpha = 2. * (f % N_FRAMES) / (N_FRAMES - 1);
		else
			alpha = 1. - 2. * (f % N_FRAMES - N_FRAMES / 2) / (N_FRAMES - 1);

		cv::Mat blended;
		cv::addWeighted(imgs[f % N_FRAMES], alpha, imgs[(f % N_FRAMES + N_FRAMES / 2) % N_FRAMES], 1. - alpha, 0., blended);

		for (auto& star : stars)
		{
			cv::Mat star_mask_resized;
			cv::resize(star_mask, star_mask_resized, cv::Size(2 * star.radius + 1, 2 * star.radius + 1));
			cv::Rect star_rect(star.x - star.radius, star.y - star.radius, 2 * star.radius + 1, 2 * star.radius + 1);
			auto star_img = blended(star_rect).clone();
			auto star_rotated = rotate_image(star_img, f * star.speed);

			cv::Mat res;
			alphaBlend(star_rotated, star_img, star_mask_resized, res);
			res.copyTo(blended(star_rect));
		}

		char s[100];
		std::sprintf(s, "frame%d.jpg", f);
		cv::imwrite(s, blended);
	}

	return 0;

}