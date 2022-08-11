
#include "pch.h"
#include "ObjectClassifier.h"

std::vector<cv::Scalar> Object::colors{};

void Object::drawObjectFrame(cv::Mat& img)
{
	if (colors.size() <= id)
	{
		static cv::RNG rng(0xFFFFFFFB);

		for (size_t i = colors.size(); i <= id; ++i) {
			int icolor = (unsigned)rng;
			cv::Scalar color(icolor & 255, (icolor >> 8) & 255, (icolor >> 16) & 255);
			colors.push_back(color);
		}
	}

	for (size_t k = 0; k < 4; ++k) {
		const int lineThickness = 3;
		line(img, vtx[k], vtx[(k + 1) % 4], colors[id], lineThickness, cv::LINE_AA);
	}
	const double fontScale = 0.75;
	const cv::Scalar innerOuretBorderColor(0, 0, 255);
	const cv::Scalar textColor(0, 0, 0);
	const int outerTextBorderThickness = 3;
	const int textThickness = 2;
	const int innerTextBorderThickness = 1;
	putText(img, cv::format("%d", id), vtx[0], cv::FONT_HERSHEY_DUPLEX, fontScale, innerOuretBorderColor, outerTextBorderThickness);
	putText(img, cv::format("%d", id), vtx[0], cv::FONT_HERSHEY_DUPLEX, fontScale, textColor, textThickness);
	putText(img, cv::format("%d", id), vtx[0], cv::FONT_HERSHEY_DUPLEX, fontScale, innerOuretBorderColor, innerTextBorderThickness);
}

namespace oc {
	std::vector<Object> ObjectClassifier::classify(const cv::Mat& img)
	{
		std::vector<Object> classifiedObjects;
		auto allObjects = getAllObjects(img);

		size_t id = 0;
		for (size_t i = 0; i < allObjects.size(); ++i) {
			if (allObjects[i].marked == true) {
				continue;
			}
			allObjects[i].marked = true;
			classifiedObjects.emplace_back(allObjects[i].minAreaBox, id);

			for (size_t j = i + 1; j < allObjects.size(); ++j) {
				if (allObjects[j].marked == false && isSameObject(allObjects[i], allObjects[j])) {
					allObjects[j].marked = true;

					classifiedObjects.emplace_back(allObjects[j].minAreaBox, id);
				}
			}
			++id;
		}
		return classifiedObjects;
	}

	std::vector<ObjectWithParameters> ObjectClassifier::getAllObjects(const cv::Mat& img)
	{
		maxPerimeter = 0;
		maxArea = 0;
		maxX = 0;

		std::vector<ObjectWithParameters> allObjects;
		cv::Mat working;
		preProcessing(img, working);

		std::vector<std::vector<cv::Point> > contours;
		cv::findContours(working, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

		for (size_t i = 0; i < contours.size(); i++) {

			const double areaTreshold = img.size().area() * CONTOUR_AREA_THRESHOLD;
			if (contourArea(contours[i]) > areaTreshold) {
				std::vector<cv::Point> conP;
				const double epsilon = 1.;
				approxPolyDP(contours[i], conP, epsilon, true);
				double peri = arcLength(conP, true);

				cv::RotatedRect box = minAreaRect(conP);

				cv::Mat obj;
				cropImageOfObject(img, obj, box);

				allObjects.emplace_back(obj, box, peri);

				updateMaxObjectParam(peri, box);
			}
		}
		return allObjects;
	}

	void ObjectClassifier::updateMaxObjectParam(double perimeter, const cv::RotatedRect& box)
	{
		if (maxPerimeter < perimeter) {
			maxPerimeter = perimeter;
		}
		if (maxArea < box.size.area()) {
			maxArea = box.size.area();
		}
		float x = std::max(box.size.width, box.size.height);
		if (maxX < x) {
			maxX = x;
		}
	}
	//
	void ObjectClassifier::preProcessing(const cv::Mat& img, cv::Mat& out)
	{
		removeShadowsnEdges(img, out);

		cvtColor(out, out, cv::COLOR_BGR2GRAY);

		const cv::Size GaussianBlurKernel(21, 21);
		const double  sigmaX = 75, sigmaY = 75;
		GaussianBlur(out, out, GaussianBlurKernel, sigmaX, sigmaY);

		const double thresh = 15, maxval = 75;
		threshold(out, out, thresh, maxval, cv::THRESH_BINARY);

		const double thresh1 = 25, thresh2 = 50;
		Canny(out, out, thresh1, thresh2);

		const cv::Size dilateKernelSize(3, 3);
		cv::Mat dilateKernel = cv::getStructuringElement(cv::MORPH_RECT, dilateKernelSize);
		dilate(out, out, dilateKernel);
	}

	void ObjectClassifier::removeShadowsnEdges(const cv::Mat& img, cv::Mat& out)
	{
		cv::Mat1s bgr[4];
		std::vector<cv::Mat> channels_norm;
		cv::split(img, bgr);

		const cv::Size dilateKernelSize(7, 7);
		const cv::Mat dilatekernel = cv::getStructuringElement(cv::MORPH_RECT, dilateKernelSize);

		const int medianBlurkSize = 21;

		const double normalizeAlpha = 0, normalizeBeta = 255;
		for (size_t i = 0; i < 3; i++) {
			cv::Mat plane;
			bgr[i].copyTo(plane);

			dilate(plane, plane, dilatekernel);

			medianBlur(plane, plane, medianBlurkSize);

			cv::Mat diff_img;
			cv::absdiff(plane, bgr[i], diff_img);

			cv::Mat norm_img;
			normalize(diff_img, norm_img, normalizeAlpha, normalizeBeta, cv::NORM_MINMAX);

			channels_norm.emplace_back(norm_img);
		}
		cv::merge(channels_norm, out);
	}

	void ObjectClassifier::cropImageOfObject(const cv::Mat& img, cv::Mat& out, const cv::RotatedRect& box)
	{
		cv::Mat rotated;
		const double scale = 1.;
		cv::Mat rot_mat = cv::getRotationMatrix2D(box.center, box.angle, scale);
		cv::Rect2f bbox = cv::RotatedRect(box.center, img.size(), box.angle).boundingRect2f();
		cv::warpAffine(img, rotated, rot_mat, bbox.size(), cv::INTER_LINEAR, cv::BORDER_TRANSPARENT);

		getRectSubPix(rotated, box.size, box.center, out);
	}

	bool ObjectClassifier::isSameObject(const ObjectWithParameters& a, const ObjectWithParameters& b)
	{
		// разность двух параметров нормированная на максимальный, чем ближе к нулю, тем ближе форма объектов
		double areaWeight = 1 - abs(a.minAreaBox.size.area() - b.minAreaBox.size.area()) / maxArea;
		double perimeterWeight = 1 - abs(a.perimeter - b.perimeter) / maxPerimeter;
		double xWeight = 1 - abs(a.x - b.x) / maxX;

		if (areaWeight < SHAPE_THRESHOLD ||
			perimeterWeight < SHAPE_THRESHOLD ||
			xWeight < SHAPE_THRESHOLD) {
			return false;
		}
		return isSameImageOfObject(a.img, b.img);
	}

	bool ObjectClassifier::isSameImageOfObject(const cv::Mat& a, const cv::Mat& b)
	{
		cv::Mat1f result;
		cv::Mat tmp;
		for (double angle = 0; angle < 360; angle += 90) {
			b.copyTo(tmp);
			if (angle != 0) {
				rotateWithoutCrop(b, tmp, angle);
			}

			makeSameSize(a, tmp);

			matchTemplate(tmp, a, result, cv::TM_CCOEFF_NORMED);

			const double tresholdMaxVal = 1.;
			threshold(result, result, TEMPLATE_MATCH_THRESHOLD, tresholdMaxVal, cv::THRESH_BINARY);

			if (countNonZero(result) >= 1) {
				return true;
			}
		}
		return false;
	}

	void ObjectClassifier::makeSameSize(const cv::Mat& a, cv::Mat& b)//
	{
		int top = 0, bottom, left = 0, right;
		if (b.rows - a.rows < 0) {
			top = a.rows - b.rows;
		}
		if (b.cols - a.cols < 0) {
			left = a.cols - b.cols;
		}

		bottom = top; right = left;
		const  cv::Scalar BORDER_COLOR(255, 255, 255, 255);

		copyMakeBorder(b, b, top, bottom, left, right, cv::BORDER_CONSTANT, BORDER_COLOR);
	}

	void ObjectClassifier::rotateWithoutCrop(const cv::Mat& img, cv::Mat& out, const double angle)
	{
		const cv::Point2f center((img.cols - 1) / 2.0, (img.rows - 1) / 2.0);
		const double scale = 1;
		cv::Mat rot_mat = cv::getRotationMatrix2D(center, angle, scale);

		cv::Rect2f bbox = cv::RotatedRect(cv::Point2f(), img.size(), angle).boundingRect2f();

		const cv::Point2f newCenter(bbox.width / 2.0, bbox.height / 2.0);
		rot_mat.at<double>(0, 2) += newCenter.x - center.x;
		rot_mat.at<double>(1, 2) += newCenter.y - center.y;

		cv::warpAffine(img, out, rot_mat, bbox.size(), 1, cv::BORDER_TRANSPARENT);
	}
}