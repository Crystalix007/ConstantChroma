#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <string>
#include <iostream>
#include <array>
#include <fstream>

using namespace cv;
using namespace std;

array<Mat, 3> getHistograms(const Mat &img);
array<Mat, 3> getCumulativeHistograms(const Mat &img);
array<array<uint16_t, 256>, 3> getLUT(const Mat &img, const array<Mat, 3> &refCumHist);
Mat doLUT(const string &filename, const array<Mat, 3> &refCumHist);
string getFilename(const size_t index);

//Edit these values here
const constexpr size_t digitCount    = 5;
const constexpr size_t startFrame    = 1;
const constexpr size_t frameCount    = 20505;
const           string framePrefix   = "Frame";
const           string frameExt      = ".png";

//Path must exist
const           string processedPath = "processed/"; //Must end in '/'

int main()
{
	auto refImg    = imread(getFilename(startFrame), IMREAD_COLOR);
	auto refHistos = getCumulativeHistograms(refImg);

	//Write out the reference image first
	imwrite(processedPath + getFilename(startFrame), refImg);

	for (size_t i = startFrame + 1; i < startFrame + frameCount; i++)
	{
		clog << "Processing frame: " << i << endl;
		string filename = getFilename(i);

		const auto lutted = doLUT(filename, refHistos);
		imwrite(processedPath + filename, lutted);
	}

	return 0;
}

array<Mat, 3> getHistograms(const Mat &img)
{
	array<Mat, 3> compPlanes;
	split(img, compPlanes);

	const int histSize = 256;

	const float range[]{ 0.f, 256.f };
	const float* histRange{ range };
	bool uniform = true, accumulate = false;

	array<Mat, 3> histos;

	for (size_t i = 0u; i < 3; i++)
		calcHist(&compPlanes[i], 1, nullptr, Mat{}, histos[i], 1, &histSize, &histRange, uniform, accumulate);

	for (size_t histI = 0u; histI < histos.size(); histI++)
	{
		double histTotal = 0.0;

		for (size_t i = 0u; i < histos[histI].rows; i++)
			histTotal += histos[histI].at<float>(i);

		for (size_t i = 0u; i < histos[histI].rows; i++)
			histos[histI].at<float>(i) /= histTotal;
	}

	return histos;
}

array<Mat, 3> getCumulativeHistograms(const Mat &img)
{
	auto histos = getHistograms(img);

	for (size_t histI = 0; histI < histos.size(); histI++)
	{
		double cumulativeTotal = 0.0;

		for (size_t i = 0; i < histos[histI].rows; i++)
		{
			cumulativeTotal += histos[histI].at<float>(i);
			histos[histI].at<float>(i) = cumulativeTotal;
		}
	}

	return histos;
}

array<array<uint16_t, 256>, 3> getLUT(const Mat &img, const array<Mat, 3> &refCumHist)
{
	const auto cumulativeHistos = getCumulativeHistograms(img);
	array<array<uint16_t, 256>, 3> luts{};

	auto comp = [](float a, float b){ return (b - a) > 1.0e-6f; };

	for (size_t lut = 0; lut < luts.size(); lut++)
	{
		uint16_t targetBin = 1;

		for (uint16_t bin = 0; bin < luts[lut].size(); bin++)
		{
			while (comp(refCumHist[lut].at<float>(targetBin), cumulativeHistos[lut].at<float>(bin)) && targetBin < luts[lut].size())
				targetBin++;

			luts[lut][bin] = targetBin - 1;
		}
	}

	return luts;
}

Mat doLUT(const string &filename, const array<Mat, 3> &refCumHist)
{
	Mat img{ imread(filename, IMREAD_COLOR) };

	if (!img.data)
	{
		cerr << "Failed to load image \'" << filename << "\'" << endl;
		return {};
	}

	const auto luts = getLUT(img, refCumHist);

	for (size_t x = 0; x < img.rows; x++)
	{
		for (size_t y = 0; y < img.cols; y++)
		{
			for (size_t c = 0; c < 3; c++)
				img.at<Vec3b>(x, y)[c] = luts[c][img.at<Vec3b>(x, y)[c]];
		}
	}

	return img;
}

string getFilename(const size_t index)
{
		string filename{ framePrefix };
		string numeric{ to_string(index) };

		if (numeric.size() > digitCount)
			throw runtime_error{ "Invalid digit count specified" };

		filename += string(digitCount - numeric.size(), '0') + numeric + frameExt;

		return filename;
}
