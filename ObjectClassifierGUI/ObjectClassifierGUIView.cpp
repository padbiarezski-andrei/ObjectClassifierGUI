
// ObjectClassifierGUIView.cpp : implementation of the CObjectClassifierGUIView class
//

#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "ObjectClassifierGUI.h"
#endif

#include "ObjectClassifierGUIDoc.h"
#include "ObjectClassifierGUIView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CObjectClassifierGUIView

IMPLEMENT_DYNCREATE(CObjectClassifierGUIView, CView)

BEGIN_MESSAGE_MAP(CObjectClassifierGUIView, CView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CObjectClassifierGUIView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_COMMAND(ID_OBJECTCLASSIFIER_OPENIMAGE, &CObjectClassifierGUIView::OnObjectclassifierOpenimage)
	ON_COMMAND(ID_OBJECTCLASSIFIER_SAVEIMAGE, &CObjectClassifierGUIView::OnObjectClassifierSaveImage)
	ON_COMMAND(ID_OBJECTCLASSIFIER_PROCESSIMAGE, &CObjectClassifierGUIView::OnObjectClassifierProcessImage)
END_MESSAGE_MAP()

// CObjectClassifierGUIView construction/destruction

CObjectClassifierGUIView::CObjectClassifierGUIView() noexcept
{
	// TODO: add construction code here

}

CObjectClassifierGUIView::~CObjectClassifierGUIView()
{
	if (m_hBitmap)
		DeleteObject(m_hBitmap);
}

BOOL CObjectClassifierGUIView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CObjectClassifierGUIView drawing

void CObjectClassifierGUIView::OnDraw(CDC* pDC)
{
	CObjectClassifierGUIDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;
	// TODO: add draw code for native data here

	if (!img.empty())
	{
		drawImageToHDC(pDC);
	}
}


// CObjectClassifierGUIView printing


void CObjectClassifierGUIView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CObjectClassifierGUIView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CObjectClassifierGUIView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CObjectClassifierGUIView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

void CObjectClassifierGUIView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CObjectClassifierGUIView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CObjectClassifierGUIView diagnostics

#ifdef _DEBUG
void CObjectClassifierGUIView::AssertValid() const
{
	CView::AssertValid();
}

void CObjectClassifierGUIView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CObjectClassifierGUIDoc* CObjectClassifierGUIView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CObjectClassifierGUIDoc)));
	return (CObjectClassifierGUIDoc*)m_pDocument;
}
#endif //_DEBUG


// CObjectClassifierGUIView message handlers


void CObjectClassifierGUIView::OnObjectclassifierOpenimage()
{
	// TODO: Add your command handler code here
	const BOOL openFileDialog = TRUE;
	const LPCTSTR defaultFileNameExt = NULL;
	const LPCTSTR initialName = L"*.jpg; *.png";

	CFileDialog fileDialog(openFileDialog, defaultFileNameExt, initialName);
	int result = fileDialog.DoModal();
	if (result == IDOK) {
		CString filepath = fileDialog.GetPathName();
		CT2CA pszConvertedAnsiString(filepath);
		std::string strStd(pszConvertedAnsiString);
		img = cv::imread(strStd, cv::IMREAD_UNCHANGED);
		RedrawWindow();
	}
}

void CObjectClassifierGUIView::OnObjectClassifierSaveImage()
{
	// TODO: Add your command handler code here
	const BOOL saveFileDialog = FALSE;
	const LPCTSTR defaultFileNameExt = L"jpg";
	const LPCTSTR initialName = L"processedImage";
	const DWORD flags = NULL;
	const LPCTSTR filters = L"jpg (*.jpg)|*.jpg| png (*.png)|*.png|";

	CFileDialog fileDialog(saveFileDialog, defaultFileNameExt, initialName, flags, filters);
	int result = fileDialog.DoModal();
	if (result == IDOK) {
		CString filepath = fileDialog.GetPathName();
		CT2CA pszConvertedAnsiString(filepath);
		std::string strStd(pszConvertedAnsiString);
		cv::imwrite(strStd, img);
	}
}

void CObjectClassifierGUIView::OnObjectClassifierProcessImage()
{
	// TODO: Add your command handler code here

	oc::ObjectClassifier obc;
	std::vector<Object> objects = obc.classify(img);
	for (auto& object : objects) {
		object.drawObjectFrame(img);
	}
	RedrawWindow();
}

void CObjectClassifierGUIView::createBitmap(HDC hdc)
{
	unsigned char* lpBitmapBits;

	if (m_hBitmap)
		DeleteObject(m_hBitmap);

	BITMAPINFO bi;
	ZeroMemory(&bi, sizeof(BITMAPINFO));
	bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bi.bmiHeader.biWidth = img.cols;
	bi.bmiHeader.biHeight = -img.rows;
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = (img.elemSize() << 3);
	bi.bmiHeader.biCompression = BI_RGB;

	m_hBitmap = CreateDIBSection(hdc, &bi, DIB_RGB_COLORS, (VOID**)&lpBitmapBits, NULL, 0);

#define ALIGN(x,a)              __ALIGN_MASK(x, a-1)
#define __ALIGN_MASK(x,mask)    (((x)+(mask))&~(mask))
	int width = img.cols * img.elemSize();
	int pitch = ALIGN(width, 4);
#undef __ALIGN_MASK
#undef ALIGN

	for (int i = 0; i < img.rows; i++)
	{
		unsigned char* data = img.ptr<unsigned char>(i);
		memcpy(lpBitmapBits, data, width);
		lpBitmapBits += pitch;
	}
}

void CObjectClassifierGUIView::drawImageToHDC(CDC* pDC)
{
	HDC hDCMem = CreateCompatibleDC(*pDC);
	createBitmap(hDCMem);

	SelectObject(hDCMem, m_hBitmap);

	BitBlt(*pDC, 0, 0, img.cols, img.rows, hDCMem, 0, 0, SRCCOPY);

	DeleteDC(hDCMem);
}
