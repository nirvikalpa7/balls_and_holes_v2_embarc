//------------------------------------------------------------------------------
// This is the most fast solution of the task about balls and holes.
// So... I would not say that's the best solution.
//------------------------------------------------------------------------------
//       Used IDE: Embarcadero RAD Studio 10.1 Berlin Architect
// Were also used: Boost, STL, Win API, FireMonkey
//------------------------------------------------------------------------------
// (c) Dmitry Sidelnikov, Nov 2016, nirvikalpa@ya.ru
//------------------------------------------------------------------------------

#include <fmx.h>
#include <mem.h>
#include <math.h>
#include <vector>
#include <queue>

#pragma hdrstop

#include "MainFormUnit.h"

#include "BasicDataTypesUnit.h"
#include "CSceneUnit.h"
#include "CCalcBaseUnit.h"

#pragma package(smart_init)
#pragma resource "*.fmx"

//------------------------------------------------------------------------------

TMainForm* MainForm;

////////////////////////////////////////////////////////////////////////////////
// BEGIN OF THE PROGRAME
////////////////////////////////////////////////////////////////////////////////

__fastcall TMainForm::TMainForm(TComponent* Owner) : TForm(Owner)
{
	SceneImage->Bitmap->SetSize(SceneImage->Width, SceneImage->Height);

	pStartScene = NULL;  // one current state
	pCalcBase = NULL; // all calc processes
	nAvalibleNumCPUs = 0;
	nUsedNumCPUs = 1;
}

//------------------------------------------------------------------------------

void __fastcall TMainForm::FormCreate(TObject *Sender)
{
	pStartScene = new CScene;
	pCalcBase = new CCalcBase;
	pStartScene->SetMainPointers(Application, MainForm, pCalcBase);
	pCalcBase->SetMainPointers(Application, MainForm);

	// get amount of avalible logical CPUs
	SYSTEM_INFO SysInfo;
	GetSystemInfo(&SysInfo);
	nAvalibleNumCPUs = SysInfo.dwNumberOfProcessors;
	// add supported concurent threads into ComboBox
	UnicodeString usCPUNum [] = { "1", "2", "4" };
	for (int i = 0; i < 3; i++) // array! not a Unicode string
	{
		if (StrToInt(usCPUNum[i]) <= nAvalibleNumCPUs)
		{
			CPU_ComboBox->Items->Add(usCPUNum[i]);
		}
	}
	CPU_ComboBox->ItemIndex = CPU_ComboBox->Items->Count - 1; // selct the last one

	// outer border on image
	int nW = SceneImage->Width;
	int nH = SceneImage->Height;
	SceneImage->Bitmap->Canvas->BeginScene();
	SceneImage->Bitmap->Canvas->Clear(claWhite);
	SceneImage->Bitmap->Canvas->Stroke->Color = 0xFF555555;
	SceneImage->Bitmap->Canvas->Stroke->Thickness = 2;
	SceneImage->Bitmap->Canvas->DrawRect(TRectF(0, 0, nW, nH), 0, 0, AllCorners, 1);
	SceneImage->Bitmap->Canvas->EndScene();
}

//------------------------------------------------------------------------------

void __fastcall TMainForm::FormClose(TObject *Sender, TCloseAction &Action)
{
	if (pStartScene != NULL)
	{
		pStartScene->DeleteScene();
		delete pStartScene;
		pStartScene = NULL;
	}

	if (pCalcBase != NULL)
	{
		pCalcBase->DeleteBase();
		delete pCalcBase;
		pCalcBase = NULL;
	}
}
//------------------------------------------------------------------------------

void __fastcall TMainForm::LoadInputDataButtonClick(TObject *Sender)
{
	if (pStartScene->IsCreated()) // delete prev. Scene
	{   
		pStartScene->DeleteScene();
	}
	if (pCalcBase->IsCreated()) // delete prev. CalcBase
	{
		pCalcBase->DeleteBase();
	}
	   // getting input string
	UnicodeString sInputString;
	if (InputComboBox->ItemIndex == 0)
	{
		sInputString = InputEdit->Text;
	}
	else
	{
		sInputString = InputComboBox->Items->Strings[InputComboBox->ItemIndex];
	}

	// try to parse the string
	if (pStartScene->InputDataParser(sInputString, pCalcBase))
	{
		// clear output memo
		OutputMemo->Lines->Clear();
		// print arrays into output memo
		pStartScene->PrintArraysToMemo(OutputMemo);
		// draw scene on an image
		pStartScene->DrawSceneOnImage(SceneImage);
	}
	else
	{
		MessageBox(0, L"Some errors were found in your input data!", L"Error!", MB_OK | MB_ICONERROR);
	}
}

//------------------------------------------------------------------------------

void __fastcall TMainForm::InputComboBoxChange(TObject *Sender)
{
	if (InputComboBox->ItemIndex == 0)
	{
		InputEdit->ReadOnly = false;
	}
	else
	{
		InputEdit->ReadOnly = true;
	}
}

//------------------------------------------------------------------------------
// THE MAIN CYCLE OF CALCULATION
//------------------------------------------------------------------------------
void __fastcall TMainForm::FindSolutionButtonClick(TObject *Sender)
{
	if (FindSolutionButton->Text == "Stop") // user wanna stop calculation?
	{
		pCalcBase->bCalcContinue = false;
		pStartScene->bCalcContinue = false;
		FindSolutionButton->Text = "Find a solution!";
		return;
	}

	if (pStartScene->IsCreated() && pCalcBase->IsCreated())
	{
		pCalcBase->bCalcContinue = true; // flags for stoping calculation
		pStartScene->bCalcContinue = true;    //

		FindSolutionButton->Text = "Stop";

		// start loading of arrays
		pCalcBase->FirstLoad(pStartScene);
		ProgrBar->Min = 0;
		ProgrBar->Value = 0;
		bool bSuccessfulResult = false;
		// Amount of threads to be used
		UnicodeString sNumOgThreads = CPU_ComboBox->Items->Strings[CPU_ComboBox->ItemIndex];
		unsigned short int nNumOfThreads = sNumOgThreads.ToInt();
		pCalcBase->ResetAllSolvedVariants();
		MainForm->DisplayUsedMemory();       // Memory
		pCalcBase->SetStartTime();           // Time
		do {
			// CALCULATE CURRENT SCENE LIST
			bSuccessfulResult = pCalcBase->ThreadGlobalCalc(nNumOfThreads);

			if (!bSuccessfulResult) // we must continue calculation
			{
				if (!pCalcBase->GenNewVariantsToFutureList())   // GENERATE FUTURE LIST OF SCENES TO CALC
				{
					// ALL variants are failed
					pCalcBase->bCalcContinue = false;
					OutputMemo->Lines->Add(" ");
					OutputMemo->Lines->Add("All variants are failed. Can not create future tasks for calculation.");
					FindSolutionButton->Text = "Find a solution!";
					break;
				}
				pCalcBase->LoadFirstList();       // LOAD FUTURE LIST TO CURRENT
				MainForm->DisplayUsedMemory();    // Memory
			}

			Application->ProcessMessages();

		} while(!bSuccessfulResult && pCalcBase->bCalcContinue);

		if (bSuccessfulResult)
		{
			pCalcBase->PrintSuccessfulResultsInMemo(OutputMemo);
			OutputMemo->UpdateContentSize();
			FindSolutionButton->Text = "Find a solution!";
		}
	}
	else
	{
		MessageBox(0, L"Input data was not loaded!", L"Error!", MB_OK | MB_ICONERROR);
	}
}

//------------------------------------------------------------------------------

void __fastcall TMainForm::DisplayProgressBar(int nCurSceneAmount, int nResolvedSceneAmount)
{
	TaskAmountAndSolvedLabel->Text = "Cur. variants: " + IntToStr(nCurSceneAmount) + " (solved: " + IntToStr(nResolvedSceneAmount) + ")";
	ProgrBar->Max = nCurSceneAmount;
	ProgrBar->Value = nResolvedSceneAmount;
}

//------------------------------------------------------------------------------

void __fastcall TMainForm::DisplayElapsedTime(const __int64 nElapsedTime)
{
	if (nElapsedTime > 0)
	{
		long double t = nElapsedTime/(long double)1000.0;
		ElapsedTimeLabel->Text = "Elapsed time: " + FloatToStr(t) + " s";
	}
}

//------------------------------------------------------------------------------

void __fastcall TMainForm::DisplayMovmentsCounter(const unsigned int nMovments)
{
	MovmentsLabel->Text = "Movments counter: " + IntToStr(int(nMovments));
}

//------------------------------------------------------------------------------

void __fastcall TMainForm::DisplayAllSolvedVariants(const __int64 nAllSolvedVar)
{
	AllSolvedVariantsLabel->Text = "All solved variants: " + IntToStr(__int64(nAllSolvedVar));
}

//------------------------------------------------------------------------------

void __fastcall TMainForm::DisplayUsedMemory()
{
	int nSceneAmount = pCalcBase->GetSceneAmount();
	int nBallAmount = pCalcBase->GetBallAmount();
	int nBoardSize = pCalcBase->GetBoardSize();

	int nCharTypeSize = sizeof(char);

	int nPresentMatrixSize = nCharTypeSize * sqr(nBoardSize) * 2;
	int nCoordArraySize = sizeof(CBallCoord) * nBallAmount;
	int nLineAndMoveArraysSize = nCharTypeSize * nBoardSize + nCharTypeSize * nMaxMovements;
	int nOneSceneSize = sizeof(CScene);

	__int64 nResultSize = __int64(nOneSceneSize) + nPresentMatrixSize + nCoordArraySize + nLineAndMoveArraysSize;

	nResultSize *= nSceneAmount;

	if (nResultSize < 1024 * 100) // less 100 Kb
	{
		UsedMemoryLabel->Text = "Memory: " + FloatToStr((long double)nResultSize / 1024.0) + " Kb";
	}
	else
	{
		UsedMemoryLabel->Text = "Memory: " + FloatToStr((long double)nResultSize / sqr(1024)) + " Mb";
	}
}

//------------------------------------------------------------------------------
