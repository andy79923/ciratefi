
// ciratefi.cpp : 定義應用程式的類別行為。
//

#include "stdafx.h"
#include "ciratefiApp.h"
#include "ciratefiDlg.h"

using namespace cv;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CciratefiApp

BEGIN_MESSAGE_MAP(CiratefiApp, CWinAppEx)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CciratefiApp 建構

CiratefiApp::CiratefiApp()
{
	// TODO: 在此加入建構程式碼，
	// 將所有重要的初始設定加入 InitInstance 中
}


// 僅有的一個 CciratefiApp 物件

CiratefiApp theApp;


// CciratefiApp 初始設定

BOOL CiratefiApp::InitInstance()
{
	// 假如應用程式資訊清單指定使用 ComCtl32.dll 6 (含) 以後版本，
	// 來啟動視覺化樣式，在 Windows XP 上，則需要 InitCommonControls()。
	// 否則任何視窗的建立都將失敗。
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// 設定要包含所有您想要用於應用程式中的
	// 通用控制項類別。
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinAppEx::InitInstance();

	AfxEnableControlContainer();

	// 標準初始設定
	// 如果您不使用這些功能並且想減少
	// 最後完成的可執行檔大小，您可以
	// 從下列程式碼移除不需要的初始化常式，
	// 變更儲存設定值的登錄機碼
	// TODO: 您應該適度修改此字串
	// (例如，公司名稱或組織名稱)
	SetRegistryKey(_T("本機 AppWizard 所產生的應用程式"));

	CiratefiDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: 在此放置於使用 [確定] 來停止使用對話方塊時
		// 處理的程式碼
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: 在此放置於使用 [取消] 來停止使用對話方塊時
		// 處理的程式碼
	}

	// 因為已經關閉對話方塊，傳回 FALSE，所以我們會結束應用程式，
	// 而非提示開始應用程式的訊息。
	return FALSE;
}

void CiratefiApp::ShowMatOnPicture(Mat& image, CDialog* dlg, int pID)
{

	CRect PictureRect;
	CStatic* PictureControl=(CStatic*)dlg->GetDlgItem(pID);
	PictureControl->GetClientRect(&PictureRect);
	CDC *pDc = PictureControl->GetWindowDC();
	SetStretchBltMode(pDc->m_hDC,STRETCH_HALFTONE);

	Mat resizeImage = image.clone();
	if(resizeImage.rows>PictureRect.Height() || resizeImage.cols>PictureRect.Width())
	{
		double resizeRatio = min((double)PictureRect.Width()/(double)image.cols, (double)PictureRect.Height()/(double)image.rows);
		resize(image, resizeImage, Size(), resizeRatio, resizeRatio);
	}

	CImage outputImage;
	int width = resizeImage.cols;
	int height = resizeImage.rows;
	int channels = resizeImage.channels();
	outputImage.Destroy(); //clear
	outputImage.Create(width, height, 8*channels);

	if(channels==1)
	{
		RGBQUAD* ColorTable;
		int MaxColors=outputImage.GetMaxColorTableEntries();
		ColorTable = new RGBQUAD[MaxColors];
		outputImage.GetColorTable(0, MaxColors, ColorTable);
		for (int i = 0; i < MaxColors; i++)
		{
			ColorTable[i].rgbBlue = (BYTE)i;
			ColorTable[i].rgbGreen = (BYTE)i;
			ColorTable[i].rgbRed = (BYTE)i;
		}
		outputImage.SetColorTable(0, MaxColors, ColorTable);
		delete []ColorTable;
	}


	uchar* ps;
	uchar* pimg = (uchar*)outputImage.GetBits(); //A pointer to the bitmap buffer
	//The pitch is the distance, in bytes. represent the beginning of
	// one bitmap line and the beginning of the next bitmap line
	int step = outputImage.GetPitch();
	for (int i = 0; i < height; ++i)
	{
		ps = (resizeImage.ptr<uchar>(i));
		for ( int j = 0; j < width; j++ )
		{
			if ( channels == 1 ) //gray
			{
				*(pimg + i*step + j) = ps[j];
			}
			else if ( channels == 3 ) //color
			{
				for (int k = 0 ; k < 3; k++ )
				{
					*(pimg + i*step + j*3 + k ) = ps[j*3 + k];
				}
			}
		}
	}
	outputImage.Draw(pDc->m_hDC, CRect(CPoint(PictureRect.TopLeft().x+(PictureRect.Width()-width)/2,PictureRect.TopLeft().y+(PictureRect.Height()-height)/2), CSize(width,height)));
	dlg->ReleaseDC(pDc);
}
