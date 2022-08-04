
// ObjectClassifierGUIView.h : interface of the CObjectClassifierGUIView class
//

#pragma once

#include "ObjectClassifier.h"

class CObjectClassifierGUIView : public CView
{
protected: // create from serialization only
	CObjectClassifierGUIView() noexcept;
	DECLARE_DYNCREATE(CObjectClassifierGUIView)

	// Attributes
public:
	CObjectClassifierGUIDoc* GetDocument() const;

	// Operations
public:

	// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

	// Implementation
public:
	virtual ~CObjectClassifierGUIView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

	// Generated message map functions
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
	////////////////
public:
	afx_msg void OnObjectclassifierOpenimage();
	afx_msg void OnObjectClassifierSaveImage();
	afx_msg void OnObjectClassifierProcessImage();

private:
	cv::Mat img;
	HBITMAP m_hBitmap;

	void createBitmap(HDC pDC);
	void drawImageToHDC(CDC* pDC);
};

#ifndef _DEBUG  // debug version in ObjectClassifierGUIView.cpp
inline CObjectClassifierGUIDoc* CObjectClassifierGUIView::GetDocument() const
{
	return reinterpret_cast<CObjectClassifierGUIDoc*>(m_pDocument);
}
#endif

