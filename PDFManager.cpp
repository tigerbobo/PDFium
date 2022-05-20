#include "stdafx.h"
#include "PDFManager.h"

#include "exception\exceptionplus.h"

#pragma comment(lib, "fdrm.lib")
#pragma comment(lib, "formfiller.lib")
#pragma comment(lib, "fpdfapi.lib")
#pragma comment(lib, "fpdfdoc.lib")
#pragma comment(lib, "fpdftext.lib")
#pragma comment(lib, "fxcodec.lib")
#pragma comment(lib, "fxcrt.lib")
#pragma comment(lib, "fxedit.lib")
#pragma comment(lib, "fxge.lib")
#pragma comment(lib, "pdfium.lib")
#pragma comment(lib, "pdfwindow.lib")


CPDFManager::CPDFManager()
{
}

CPDFManager::~CPDFManager()
{
}

void CPDFManager::initialize()
{
	FPDF_InitLibrary(NULL);
}

void CPDFManager::uninitialize()
{
	FPDF_DestroyLibrary();
}

void CPDFManager::pdf2image(stdstring pdf_path_name, int page_index, int multiple, stdstring image_path_name)
{
	int page_count;
	FPDF_DOCUMENT pdf_doc = FPDF_LoadDocument(unicode2asc(pdf_path_name).c_str(), NULL);

	if (NULL == pdf_doc)
		EXCEPTIONEX(_T("PDF"), _T("load document"), pdf_path_name);

	page_count = FPDF_GetPageCount(pdf_doc);
	if (page_count < page_index)
	{
		FPDF_CloseDocument(pdf_doc);

		EXCEPTION(_T("PDF"), _T("get page count"), _T("< page index"));
	}

	FPDF_PAGE page = FPDF_LoadPage(pdf_doc, page_index);

	int width = static_cast<int>(FPDF_GetPageWidth(page)) * multiple;
	int height = static_cast<int>(FPDF_GetPageHeight(page)) * multiple;

	FPDF_BITMAP bitmap = FPDFBitmap_Create(width, height, 0);
	FPDFBitmap_FillRect(bitmap, 0, 0, width, height, 0xFFFFFFFF);

	FPDF_RenderPageBitmap(bitmap, page, 0, 0, width, height, 0, 0);

	int stride = FPDFBitmap_GetStride(bitmap);
	const char* buffer = reinterpret_cast<const char*>(FPDFBitmap_GetBuffer(bitmap));

	try
	{
		write_bmp(image_path_name, buffer, stride, width, height);
	}
	catch (CExceptionPlus &e)
	{
		throw e;
	}
}

void CPDFManager::write_bmp(stdstring path_name, const void* buffer, int stride, int width, int height)
{
	if (stride < 0 || width < 0 || height < 0)
		EXCEPTIONEX(_T("PDF"), _T("write bmp"), _T("stride < 0 || width < 0 || height < 0"));

	if (height > 0 && width > INT_MAX / height)
		EXCEPTIONEX(_T("PDF"), _T("write bmp"), _T("width > INT_MAX / height"));

	int out_len = stride * height;
	if (out_len > INT_MAX / 3)
		EXCEPTIONEX(_T("PDF"), _T("write bmp"), _T("stride * height > INT_MAX / 3"));

	FILE* fp;

	if (_tfopen_s(&fp, path_name.c_str(), _T("wb")))
		EXCEPTIONEX(_T("PDF"), _T("write bmp"), _T("open write file"));

	BITMAPINFO bmi = { 0 };
	bmi.bmiHeader.biSize = sizeof(bmi) - sizeof(RGBQUAD);
	bmi.bmiHeader.biWidth = width;
	bmi.bmiHeader.biHeight = -height;  // top-down image
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biSizeImage = 0;

	BITMAPFILEHEADER file_header = { 0 };
	file_header.bfType = 0x4d42;
	file_header.bfSize = sizeof(file_header) + bmi.bmiHeader.biSize + out_len;
	file_header.bfOffBits = file_header.bfSize - out_len;

	fwrite(&file_header, sizeof(file_header), 1, fp);
	fwrite(&bmi, bmi.bmiHeader.biSize, 1, fp);
	fwrite(buffer, out_len, 1, fp);

	fclose(fp);
}