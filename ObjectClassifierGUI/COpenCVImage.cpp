#include "pch.h"
#include "COpenCVImage.h"

COpenCVImage::COpenCVImage(LPCTSTR lpszFileName) : m_hBitmap(NULL), m_bInit(false)
{
	if (!PathFileExists(lpszFileName))
	{
		MessageBox(NULL, _T("Cannot open the image file"), _T("Error"), MB_OK);
		return;
	}

	char* pFileName;
	UINT nLen;

	nLen = WideCharToMultiByte(CP_UTF8, 0, lpszFileName, sizeof(TCHAR) * (_tcslen(lpszFileName) + 1), NULL, 0, NULL, NULL);
	pFileName = new char[nLen];
	WideCharToMultiByte(CP_UTF8, 0, lpszFileName, sizeof(TCHAR) * (_tcslen(lpszFileName) + 1), pFileName, nLen, NULL, NULL);

	m_matImage = cv::imread(pFileName, cv::IMREAD_COLOR);
	delete[] pFileName;

	if (m_matImage.empty())
	{
		MessageBox(NULL, _T("Cannot read in the image file"), _T("Error"), MB_OK);
		return;
	}

	m_bInit = true;
}

COpenCVImage::~COpenCVImage(void)
{
	if (m_hBitmap)
		DeleteObject(m_hBitmap);
}


BOOL COpenCVImage::DrawBitmap(HDC hdc, LPRECT lpRect)
{
	if (!m_bInit)
		return FALSE;

	HDC hDCMem = CreateCompatibleDC(hdc);

	if (!m_hBitmap)
	{
		createBitmap(hDCMem, m_matImage);
	}

	SelectObject(hDCMem, m_hBitmap);

	BitBlt(hdc, lpRect->left, lpRect->top, lpRect->right - lpRect->left,
		lpRect->bottom - lpRect->top, hDCMem, 0, 0, SRCCOPY);

	DeleteDC(hDCMem);

	return TRUE;
}

//
//BOOL COpenCVImage::GetSize(LPRECT lpRect)
//{
//	if (!m_bInit)
//	{
//		memset(lpRect, 0, sizeof(RECT));
//		return FALSE;
//	}
//
//	lpRect->left = 0;
//	lpRect->top = 0;
//	lpRect->right = m_matImage.cols;
//	lpRect->bottom = m_matImage.rows;
//
//	return TRUE;
//}

bool COpenCVImage::createBitmap(HDC hdc, cv::Mat mat)
{
	unsigned char* lpBitmapBits;

	if (m_hBitmap)
		DeleteObject(m_hBitmap);

	BITMAPINFO bi;
	ZeroMemory(&bi, sizeof(BITMAPINFO));
	bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bi.bmiHeader.biWidth = m_matImage.cols;
	bi.bmiHeader.biHeight = -m_matImage.rows;
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = (m_matImage.elemSize() << 3);
	bi.bmiHeader.biCompression = BI_RGB;

	m_hBitmap = CreateDIBSection(hdc, &bi, DIB_RGB_COLORS, (VOID**)&lpBitmapBits, NULL, 0);

#define ALIGN(x,a)              __ALIGN_MASK(x, a-1)
#define __ALIGN_MASK(x,mask)    (((x)+(mask))&~(mask))
	int width = m_matImage.cols * m_matImage.elemSize();
	int pitch = ALIGN(width, 4);
#undef __ALIGN_MASK
#undef ALIGN

	for (int i = 0; i < m_matImage.rows; i++)
	{
		unsigned char* data = mat.ptr<unsigned char>(i);
		memcpy(lpBitmapBits, data, width);
		lpBitmapBits += pitch;
	}

	return true;
}