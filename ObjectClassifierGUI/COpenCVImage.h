#pragma once

#include <opencv2/core.hpp>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

class COpenCVImage
{
public:
	COpenCVImage(LPCTSTR lpszFileName);
	~COpenCVImage(void);

	BOOL DrawBitmap(HDC hdc, LPRECT lpRect);

private:
	cv::Mat m_matImage;
	HBITMAP m_hBitmap;

	bool m_bInit;

	bool createBitmap(HDC hdc, cv::Mat mat);
};