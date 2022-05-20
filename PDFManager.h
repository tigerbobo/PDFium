#pragma once

#include "stdstring\stdstring.h"

#include "fpdfsdk/include/fpdf_ext.h"
#include "fpdfsdk/include/fpdftext.h"
#include "fpdfsdk/include/fpdfview.h"
#include "fpdfsdk/include/fpdfformfill.h"
#include "fpdfsdk/include/fpdf_dataavail.h"

class CPDFManager
{
public:
	CPDFManager();
	virtual ~CPDFManager();
public:
	static void initialize();
	static void uninitialize();
public:
	static void pdf2image(stdstring pdf_path_name, int page_index, int multiple, stdstring image_path_name);
private:
	static void write_bmp(stdstring path_name, const void* buffer, int stride, int width, int height);
};

