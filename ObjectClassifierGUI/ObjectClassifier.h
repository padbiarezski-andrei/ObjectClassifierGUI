#pragma once

#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <vector>

class Object
{
public:
	cv::Point2f vtx[4];
	size_t id;

	Object(cv::RotatedRect box, size_t _id) : id(_id)
	{
		box.points(vtx);
	}

	void drawObjectFrame(cv::Mat&);

private:
	static std::vector<cv::Scalar> colors;
};

namespace oc {
	struct ObjectWithParameters {
		ObjectWithParameters(const cv::Mat& img_, const cv::RotatedRect& minAreaBox_, double perimeter_) :
			minAreaBox(minAreaBox_), perimeter(perimeter_) {
			img_.copyTo(img);
			x = std::max(minAreaBox.size.width, minAreaBox.size.height);
			marked = false;
		}

		cv::RotatedRect minAreaBox;
		cv::Mat img;
		double perimeter;
		double x;

		bool marked;
	};

	class ObjectClassifier
	{
	public:
		std::vector<Object> classify(const cv::Mat&);

	private:
		// порог близости контура/формы
		const double SHAPE_THRESHOLD = 0.85;
		// порог результата функции matchTemplate
		const double TEMPLATE_MATCH_THRESHOLD = 0.7;
		// какую часть от всего изображения составляет контур
		const double CONTOUR_AREA_THRESHOLD = 1. / 200.;

		double maxPerimeter;
		double maxArea;
		double maxX;

		std::vector<ObjectWithParameters> getAllObjects(const cv::Mat&);
		void updateMaxObjectParam(double, const cv::RotatedRect&);

		void preProcessing(const cv::Mat&, cv::Mat&);
		void removeShadowsnEdges(const cv::Mat&, cv::Mat&);
		void cropImageOfObject(const cv::Mat&, cv::Mat&, const cv::RotatedRect&);

		bool isSameObject(const ObjectWithParameters&, const ObjectWithParameters&);
		bool isSameImageOfObject(const cv::Mat&, const cv::Mat&);
		void makeSameSize(const cv::Mat&, cv::Mat&);

		void rotateWithoutCrop(const cv::Mat&, cv::Mat&, const double angle);
	};
}