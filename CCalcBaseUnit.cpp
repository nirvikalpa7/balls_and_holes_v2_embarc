//------------------------------------------------------------------------------
// (c) Dmitry Sidelnikov, Nov 2016, nirvikalpa@ya.ru
//------------------------------------------------------------------------------

//#define BOOST_THREAD_USE_LIB

#include <boost/thread.hpp>
#include <boost/chrono.hpp>

#include "CCalcBaseUnit.h"

//==============================================================================
// IMPLEMENTATION of CCalcBase class
//==============================================================================

TApplication* CCalcBase::pApp = NULL;
TMainForm* CCalcBase::pMForm = NULL;

// used for delay in boost::threads
const unsigned int nThreadDelayMilisec = 10;

//------------------------------------------------------------------------------

CCalcBase::CCalcBase()
{
	nUnsolvedSceneAmount = 0;
	nAllSolvedVariants = 0;
	nSceneAmount = 0;
	nStartTickCount = 0;
	nElapsedTicks = 0;
	nBorderAmount = 0; // amount of borders on a board
	nBoardSize = 0;
	bBaseCreated = false;
	nSuccessfulResultAmount = 0;   // which results are successful?
	ppcBorders = NULL;          // border array
	pBorderCoord = NULL;            // Borders coordinates (to find real borders from unreal)
	cThreadsMode = 0;

	for (char i = 0; i < 4; i++)
	{
		bThreadFinished[i] = true; // from 0 ... 3
	}
}

//------------------------------------------------------------------------------

CCalcBase::~CCalcBase()
{
	DeleteBase();
}

//------------------------------------------------------------------------------

bool __fastcall CCalcBase::CreateBase(unsigned int nNewBoardSize, unsigned int nNewBorderAmount)
{
	if (bBaseCreated)
	{
		return false;
	}

	// if CalcQueue not empty - clear it

	ClearCalcVectorFull(CurrentTaskVector);
	ClearCalcVectorFull(FutureTaskVector);

	nUnsolvedSceneAmount = 0;
	nAllSolvedVariants = 0;
	nSuccessfulResultAmount = 0;

	nStartTickCount = nElapsedTicks = 0;

	nBorderAmount = nNewBorderAmount; // amount of borders on a board
	nBoardSize = nNewBoardSize;

	// border array
	ppcBorders = new char * [nBoardSize];
	if (ppcBorders == NULL)
	{
		return false;
	}
	for (unsigned int i = 0; i < nBoardSize; i++)
	{
		ppcBorders[i] = new char [nBoardSize];
		if (ppcBorders[i] == NULL)
		{
			return false;
		}
		setmem(ppcBorders[i], sizeof(char)*nBoardSize, 0);
	}

	// borders coordinates
	pBorderCoord = new CBorder [nBorderAmount];
	if (pBorderCoord == NULL)
	{
		return false;
	}
	bBaseCreated = true;
	return true;
}

//------------------------------------------------------------------------------

bool CCalcBase::DeleteBase()
{
	if (!bBaseCreated)
	{
		return false;
	}

	// removing border present matrix
	unsigned int i;
	if (ppcBorders != NULL)
	{
		for (i = 0; i < nBoardSize; i++)
		{
			if (ppcBorders[i] != NULL)
			{
				delete [] ppcBorders[i];
			}
		}
		delete [] ppcBorders;
		ppcBorders = NULL;
	}

	// borders coordinates
	if (pBorderCoord != NULL)
	{
		delete [] pBorderCoord;
		pBorderCoord = NULL;
	}

	// if CalcQueue not empty - clear it
	ClearCalcVectorFull(CurrentTaskVector);
	ClearCalcVectorFull(FutureTaskVector);

	nUnsolvedSceneAmount = 0;
	nSuccessfulResultAmount = 0;
	nStartTickCount = 0;
	nElapsedTicks = 0;
	nBorderAmount = 0;
	nBoardSize = 0;

	bBaseCreated = false;
	return true;
}

//------------------------------------------------------------------------------

bool CCalcBase::IsCreated()
{
	return bBaseCreated;
}

//------------------------------------------------------------------------------

int CCalcBase::GetElapsedTicks()
{
	return nElapsedTicks;
}

//------------------------------------------------------------------------------

int CCalcBase::GetUnsolvedSceneAmount()
{
	return nUnsolvedSceneAmount;
}

//------------------------------------------------------------------------------

int CCalcBase::GetResolvedSceneAmount()
{
	return nSceneAmount - nUnsolvedSceneAmount;
}

//------------------------------------------------------------------------------

int CCalcBase::GetSceneAmount()
{
	return nSceneAmount;
};

//------------------------------------------------------------------------------

void __fastcall CCalcBase::ClearCalcVectorFull(std::vector<CScene *> & Vector)
{
	int nSize = Vector.size();
	CScene* pScene;
	for (int i = 0; i < nSize; i++)
	{
		pScene = NULL;
		pScene = Vector[i++];
		if (pScene != NULL)
		{
			pScene->DeleteScene();
			delete pScene;
		}
	}
	Vector.clear();
}

//------------------------------------------------------------------------------

bool __fastcall CCalcBase::IsItRealBorder(const CBorder & Border)
{
	if (pBorderCoord == NULL)
	{
		return false;
	}

	CBorder EquivalentBorder; // any border can be describer in 2 variants of first point
	EquivalentBorder.p1 = Border.p2;
	EquivalentBorder.p2 = Border.p1;
	for (unsigned int i = 0; i < nBorderAmount; i++)
	{
		if (pBorderCoord[i] == Border || pBorderCoord[i] == EquivalentBorder)
		{
			return true;
		}
	}

	return false;
}

//------------------------------------------------------------------------------

void __fastcall CCalcBase::SetMainPointers(TApplication* pNewApp, TMainForm* pNewMForm)
{
	if (pNewApp != NULL)
	{
		pApp = pNewApp;
	}

	if (pNewMForm != NULL)
	{
		pMForm = pNewMForm;
	}
}

//------------------------------------------------------------------------------

void __fastcall CCalcBase::FirstLoad(const CScene* pStartScene)
{
	// if not empty - clear them
	ClearCalcVectorFull(CurrentTaskVector);
	ClearCalcVectorFull(FutureTaskVector);

	if (pStartScene != NULL)
	{
		CScene * pNewScene = NULL;

		pNewScene = new CScene(pStartScene);
		pNewScene->AddMovment('U');
		CurrentTaskVector.push_back(pNewScene);

		pNewScene = new CScene(pStartScene);
		pNewScene->AddMovment('R');
		CurrentTaskVector.push_back(pNewScene);

		pNewScene = new CScene(pStartScene);
		pNewScene->AddMovment('D');
		CurrentTaskVector.push_back(pNewScene);

		pNewScene = new CScene(pStartScene);
		pNewScene->AddMovment('L');
		CurrentTaskVector.push_back(pNewScene);

		nSceneAmount = 4;
		nUnsolvedSceneAmount = 4;
	}
}

//------------------------------------------------------------------------------

void CCalcBase::CalcCurrentList1() // 1st thread   cMode - 1, 2, 4
{
	unsigned int nListSize = CurrentTaskVector.size();
	unsigned int nMovmentAmount;
	char cDirection;
	CScene* pCurrScene = NULL;

	// for all TASKS in CUR LIST
	for (unsigned int i = 0; (i < nListSize) && bCalcContinue; i++)
	{
//		pApp->ProcessMessages(); // app should not hang up

		if (pMForm->CalcDelayCheckBox->IsChecked) // dalay for debug aim
		{
			// for delay in boost::thread we should use boost::chrono
			boost::this_thread::sleep_for(boost::chrono::milliseconds{nThreadDelayMilisec}); // delay
		}

		pCurrScene = CurrentTaskVector[i]; // get first scene

		// move
		nMovmentAmount = pCurrScene->GetMovementCounter();
		cDirection = pCurrScene->GetMovement(nMovmentAmount-1);
		if (cThreadsMode == 4) // four threads
		{
			if (cDirection == 'U')
			{
				pCurrScene->MoveScene(cDirection);
			}
			else
			{
				continue;
			}
		} else
		if (cThreadsMode == 2) // two threads
		{
			if (cDirection == 'U' || cDirection == 'D')
			{
				pCurrScene->MoveScene(cDirection);
			}
			else
			{
				continue;
			}
		} else
		if (cThreadsMode == 1) // one threads
		{
			pCurrScene->MoveScene(cDirection);
		}

		nThreadSolvedVariants[0]++; // solved variants
	}

	bThreadFinished[0] = true;
}

//------------------------------------------------------------------------------

void CCalcBase::CalcCurrentList2() // 2nd threads  cMode - 2, 4
{
	unsigned int nListSize = CurrentTaskVector.size();
	unsigned int nMovmentAmount;
	char cDirection;
	CScene* pCurrScene = NULL;

	// for all TASKS in CUR LIST
	for (unsigned int i = 0; (i < nListSize) && bCalcContinue; i++)
	{
//		pApp->ProcessMessages(); // app should not hang up

		if (pMForm->CalcDelayCheckBox->IsChecked) // dalay for debug aim
		{
			// for delay in boost::thread we should use boost::chrono
			boost::this_thread::sleep_for(boost::chrono::milliseconds{nThreadDelayMilisec}); // delay
		}

		pCurrScene = CurrentTaskVector[i]; // get first scene

		// move
		nMovmentAmount = pCurrScene->GetMovementCounter();
		cDirection = pCurrScene->GetMovement(nMovmentAmount-1);
		if (cThreadsMode == 4) // four threads
		{
			if (cDirection == 'R')
			{
				pCurrScene->MoveScene(cDirection);
			}
			else
			{
				continue;
			}
		} else
		if (cThreadsMode == 2) // two threads
		{
			if (cDirection == 'L' || cDirection == 'R')
			{
				pCurrScene->MoveScene(cDirection);
			}
			else
			{
				continue;
			}
		}

		nThreadSolvedVariants[1]++; // solved variants
	}

	bThreadFinished[1] = true;
}

//------------------------------------------------------------------------------

void CCalcBase::CalcCurrentList3() // 3th threads  cMode - 4
{
	unsigned int nListSize = CurrentTaskVector.size();
	unsigned int nMovmentAmount;
	char cDirection;
	CScene* pCurrScene = NULL;

	// for all TASKS in CUR LIST
	for (unsigned int i = 0; (i < nListSize) && bCalcContinue; i++)
	{
//		pApp->ProcessMessages(); // app should not hang up

		if (pMForm->CalcDelayCheckBox->IsChecked) // dalay for debug aim
		{
			// for delay in boost::thread we should use boost::chrono
			boost::this_thread::sleep_for(boost::chrono::milliseconds{nThreadDelayMilisec}); // delay
		}

		pCurrScene = CurrentTaskVector[i]; // get first scene

		// move
		nMovmentAmount = pCurrScene->GetMovementCounter();
		cDirection = pCurrScene->GetMovement(nMovmentAmount-1);
		if (cThreadsMode == 4) // four threads
		{
			if (cDirection == 'D')
			{
				pCurrScene->MoveScene(cDirection);
			}
			else
			{
				continue;
			}
		}

		nThreadSolvedVariants[2]++; // solved variants
	}

	bThreadFinished[2] = true;
}

//------------------------------------------------------------------------------

void CCalcBase::CalcCurrentList4() // 4th threads  cMode - 4
{
	unsigned int nListSize = CurrentTaskVector.size();
	unsigned int nMovmentAmount;
	char cDirection;
	CScene* pCurrScene = NULL;
	char cCurTaskResult;

	// for all TASKS in CUR LIST
	for (unsigned int i = 0; (i < nListSize) && bCalcContinue; i++)
	{
//		pApp->ProcessMessages(); // app should not hang up

		if (pMForm->CalcDelayCheckBox->IsChecked) // dalay for debug aim
		{
			// for delay in boost::thread we should use boost::chrono
			boost::this_thread::sleep_for(boost::chrono::milliseconds{nThreadDelayMilisec}); // delay
		}

		pCurrScene = CurrentTaskVector[i]; // get first scene

		// move
		nMovmentAmount = pCurrScene->GetMovementCounter();
		cDirection = pCurrScene->GetMovement(nMovmentAmount-1);
		if (cThreadsMode == 4) // four threads
		{
			if (cDirection == 'L')
			{
				pCurrScene->MoveScene(cDirection);
			}
			else
			{
				continue;
			}
		}

		nThreadSolvedVariants[3]++; // solved variants
	}

	bThreadFinished[3] = true;
}

//------------------------------------------------------------------------------
// Main function for multi threads calc
//------------------------------------------------------------------------------

bool __fastcall CCalcBase::ThreadGlobalCalc(const char cMode)
{
	if (pApp == NULL)
	{
		return false;
	}

	bool bGameIsWon = false;
	cThreadsMode = cMode; // pass this parametr to treads

	// default filling (reset)
	for (int i = 0; i < cThreadsMode; i++)
	{
		bThreadFinished[i] = false;
		nThreadSolvedVariants[i] = 0; // solved variants
	}

	//--------------------------------------------------------------------------
	// 1. Run from 1 to 4 threads
	//--------------------------------------------------------------------------
	boost::thread* pThread1 = new boost::thread(&CalcCurrentList1); // = 1 =
	boost::thread* pThread2 = NULL; // = 2 =
	boost::thread* pThread3 = NULL; // = 3 =
	boost::thread* pThread4 = NULL; // = 4 =
	pThread1->join();
	switch (cThreadsMode)
	{
		case 4:
		{
			pThread2 = new boost::thread(&CalcCurrentList2);
			pThread3 = new boost::thread(&CalcCurrentList3);
			pThread4 = new boost::thread(&CalcCurrentList4);
			pThread2->join();
			pThread3->join();
			pThread4->join();
			break;
		}
		case 2:
		{
			pThread2 = new boost::thread(&CalcCurrentList2);
			pThread2->join();
			break;
		}
		default: { break; }
	}

	//--------------------------------------------------------------------------
	// 2. Waiting for finishing of all threads calculation
	//--------------------------------------------------------------------------
	bool bShouldStop = false;
	__int64 nCurSolved;
	__int64 nCurSceneAmount = 0;
	// Update MovementCounter in UI (only one time)
	CScene* pCurrScene = CurrentTaskVector[0]; // get first scene
	unsigned int nMovmentAmount = pCurrScene->GetMovementCounter();
	pMForm->DisplayMovmentsCounter(nMovmentAmount);
	do { // waiting cycle

		//----------------------------------------------------------------------
		// 3. Get currently sloved variants
		//----------------------------------------------------------------------
		nCurSolved = 0;
		for (int i = 0; i < cThreadsMode; i++)
		{
			nCurSolved += nThreadSolvedVariants[i]; // solved variants
		}

		//----------------------------------------------------------------------
		// 4. Update labels with info in UI
		//----------------------------------------------------------------------
		nElapsedTicks = GetTickCount() - nStartTickCount; // calc elapsed time via Win API fun
		pMForm->DisplayElapsedTime(nElapsedTicks);
		pMForm->DisplayAllSolvedVariants(nAllSolvedVariants + nCurSolved);
		nCurSceneAmount = CurrentTaskVector.size();
		nUnsolvedSceneAmount = nCurSceneAmount - nCurSolved;
		pMForm->DisplayProgressBar(nCurSceneAmount, nCurSolved);

		pMForm->OutputMemo->Lines->Add("nCurSceneAmount = " + IntToStr(nCurSceneAmount) + ", nCurSolved = " + IntToStr(nCurSolved));

		pApp->ProcessMessages(); // app should not hang up

		if (!bCalcContinue) // got an exit from UI?
		{
			break;
		}

		switch (cMode)
		{
			case 4: bShouldStop = bThreadFinished[0] && bThreadFinished[1] && bThreadFinished[2] && bThreadFinished[2]; break;
			case 2: bShouldStop = bThreadFinished[0] && bThreadFinished[1]; break;
			case 1: bShouldStop = bThreadFinished[0]; break;
		}

	} while(!bShouldStop);

	//--------------------------------------------------------------------------
	// 5. Looking for successful results
	//--------------------------------------------------------------------------
	char cCurTaskResult;
	for (unsigned int i = 0; (i < nCurSceneAmount) && bCalcContinue; i++)
	{
		pCurrScene = CurrentTaskVector[i]; // get first scene

		// 0 - can't move, 1 - successful move (some ball could win), 2 - win, 3 - game failed
		cCurTaskResult = pCurrScene->GetMovementResult();
		if (cCurTaskResult == 2)
		{
			bGameIsWon = true; // win is got but continue calculation to get other solution
			break;
		}
	}

	//--------------------------------------------------------------------------
	// 6. Destroy thread objects & Update nAllSolvedVariants
	//--------------------------------------------------------------------------
	// destroy thread objects
	if (pThread1 != NULL)
	{
		delete pThread1;
	}
	switch (cThreadsMode)
	{
		case 4:
		{
			delete pThread2;
			delete pThread3;
			delete pThread4;
			break;
		}
		case 2:
		{
			delete pThread2;
			break;
		}
		default: { break; }
	}
    // update
	nAllSolvedVariants += nCurSceneAmount;

	if (bGameIsWon)
	{
		return true;
	}

	return false; // 0,1
}

//------------------------------------------------------------------------------

void CCalcBase::ResetAllSolvedVariants()
{
	nAllSolvedVariants = 0;
	for (int i = 0; i < 4; i++)
	{
		nThreadSolvedVariants[i] = 0; // solved variants
	}
};

//------------------------------------------------------------------------------

bool CCalcBase::GenNewVariantsToFutureList()
{
	ClearCalcVectorFull(FutureTaskVector);
	nUnsolvedSceneAmount = 0;
	CScene * pScene[3];
	char cLastMov;
	UnicodeString usMovDirections = "URDL";
	short unsigned int j, k;
	bool bAllVariantFailed = true;
	for (int i = 0; i < nSceneAmount; i++)
	{
		// this is failed game
		if (CurrentTaskVector[i]->GetMovementResult() == 3)
		{
			continue; // skeep this scene
		}
		else
		{
			bAllVariantFailed = false;
		}

		pScene[0] = CurrentTaskVector[i]; // we can use old scene! =)
		pScene[1] = new CScene(pScene[0]);
		pScene[2] = new CScene(pScene[0]);
		cLastMov = pScene[0]->GetTheLastMovement();

		for (j = 0, k = 0; j < 4; j++)
		{
			if (cLastMov != usMovDirections[j+1])
			{
				pScene[k]->AddMovment(usMovDirections[j+1]);
				pScene[k]->ResetMovementResult();
				k++;
			}
		}

		FutureTaskVector.push_back(pScene[0]);
		FutureTaskVector.push_back(pScene[1]);
		FutureTaskVector.push_back(pScene[2]);

		nUnsolvedSceneAmount += 3;
	}

	nSceneAmount = nUnsolvedSceneAmount;

	return !bAllVariantFailed;
}

//------------------------------------------------------------------------------

void CCalcBase::LoadFirstList()
{
	// delete all failed scenes
	for (unsigned int i = 0; i < CurrentTaskVector.size(); i++)
	{
		// this is failed game
		if (CurrentTaskVector[i]->GetMovementResult() == 3)
		{
			// delete this scene
			CurrentTaskVector[i]->DeleteScene();
			delete CurrentTaskVector[i];
			CurrentTaskVector[i] = NULL;
			// we must here also delete it form list and decriment ScenAmount,
			// but next line we delete all list
		}
	}

	CurrentTaskVector.clear();
	CurrentTaskVector = FutureTaskVector; // COPY
	FutureTaskVector.clear();
}

//------------------------------------------------------------------------------

void __fastcall CCalcBase::GetStrInNorthSouthNotation(const UnicodeString & sUpDownNotation, UnicodeString & sNorthSouthNotation)
{
	sNorthSouthNotation = "";
	char cSymbol;
	for (int i = 1; i <= sUpDownNotation.Length(); i++)
	{
		switch (sUpDownNotation[i])
		{
			case 'U': cSymbol = 'N'; break;
			case 'R': cSymbol = 'E'; break;
			case 'D': cSymbol = 'S'; break;
			case 'L': cSymbol = 'W'; break;

			default: cSymbol = sUpDownNotation[i]; break;
		}
		sNorthSouthNotation += cSymbol;
	}
}

//------------------------------------------------------------------------------

void __fastcall CCalcBase::PrintSuccessfulResultsInMemo(TMemo * pOutpMemo)
{
	if (pOutpMemo != NULL)
	{
		char cResult, cSymbol;
		CScene * pScene;
		UnicodeString sUpDownNotation, sNorthSouthNotation;
		pOutpMemo->Lines->Add(" ");
		pOutpMemo->Lines->Add("Game was won!");
		int nSceneCounter = 0;
		for (int i = 0; i < nSceneAmount; i++)
		{
			pScene = CurrentTaskVector[i];
			cResult = pScene->GetMovementResult(); // 0 - can't move, 1 - successful move (some ball could win), 2 - win, 3 - game failed
			if (cResult == 2) // WIN
			{
				sUpDownNotation = "";
				for (unsigned int j = 0; j < pScene->GetMovementCounter(); j++)
				{
					cSymbol = pScene->GetMovement(j);
					sUpDownNotation += cSymbol;
				}
				nSceneCounter++;
				pOutpMemo->Lines->Add(" = " + IntToStr(nSceneCounter) + " =");
				pOutpMemo->Lines->Add("Successful path is: " + sUpDownNotation + "  (Up-Down notation)");
				GetStrInNorthSouthNotation(sUpDownNotation, sNorthSouthNotation);
				pOutpMemo->Lines->Add("Successful path is: " + sNorthSouthNotation + "  (North-South notation)");
			}
		}
	}
}

//------------------------------------------------------------------------------

void CCalcBase::SetStartTime()
{
	nStartTickCount = GetTickCount(); // Start TIME
	nElapsedTicks = 0;
}

//------------------------------------------------------------------------------

int CCalcBase::GetBallAmount()
{
	int nBallAmount = 0;
	if (CurrentTaskVector.size() > 0)
	{
		if (CurrentTaskVector[0] != NULL)
		{
			nBallAmount = CurrentTaskVector[0]->GetFirstSceneBallsAmount();
		}
	}

	return nBallAmount;
}

//------------------------------------------------------------------------------

int CCalcBase::GetBoardSize()
{
	int nBoardSize = 0;
	if (CurrentTaskVector.size() > 0)
	{
		if (CurrentTaskVector[0] != NULL)
		{
			nBoardSize = CurrentTaskVector[0]->GetBoardSize();
		}
	}

	return nBoardSize;
}

//------------------------------------------------------------------------------