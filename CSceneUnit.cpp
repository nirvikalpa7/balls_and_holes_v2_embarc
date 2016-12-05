//------------------------------------------------------------------------------
// (c) Dmitry Sidelnikov, Nov 2016, nirvikalpa@ya.ru
//------------------------------------------------------------------------------

#include "CSceneUnit.h"

//==============================================================================
// IMPLEMENTATION of CScene class
//==============================================================================

bool CScene::bCalcContinue = true; // to stop calculating process

// main pinters for work
TApplication* CScene::pApp = NULL;
TMainForm* CScene::pMForm = NULL;
CCalcBase* CScene::pCalcBase = NULL;

//------------------------------------------------------------------------------
void CScene::ZeroInit()
{
	ppcBalls = NULL;         // presence matrix
	ppcHoles = NULL;         //
	pcMovementLines = NULL;     // Lines to move, size = BorderSize
	pcMovements = NULL;         // Directions of movments, size = nMaxMovements, 'U','R','D','L'
	pBallCoord = NULL;          // Coordinates
	bSceneCreated = false; 		// is scene allocated in memory?
	nBallsAmount = 0;           // amount of balls and holes (the same value)
	nFirstSceneBallsAmount = 0;
	nBoardSize = 0;             // size of board (nBoardSize x nBoardSize)
	nMovementCounter = 0;
	nLastMovementResult = 0;    // 0 - can't move, 1 - successful move (some ball could win), 2 - win, 3 - game failed
}
//------------------------------------------------------------------------------

CScene::CScene()
{
	ZeroInit();
};

//------------------------------------------------------------------------------

CScene::CScene(const CScene * pScene)
{
	if (pScene != NULL && pScene->bSceneCreated)
	{
		// this instance should be also initicalized by 0
		ZeroInit();

		if (CreateScene(pScene->nBoardSize, pScene->nBallsAmount, pScene->nFirstSceneBallsAmount))
		{
			//------------------------------------------------------------------
			// we should copy all data
			//------------------------------------------------------------------
			unsigned int i;
			unsigned int nSize = sizeof(char)*nBoardSize;
			for (i = 0; i < nBoardSize; i++)
			{
				memcpy(ppcBalls[i], pScene->ppcBalls[i], nSize);
			}

			for (i = 0; i < nBoardSize; i++)
			{
				memcpy(ppcHoles[i], pScene->ppcHoles[i], nSize);
			}

			nFirstSceneBallsAmount = pScene->nFirstSceneBallsAmount;
			for (i = 0; i < nFirstSceneBallsAmount; i++) // create new ptr array to balls
			{
				pBallCoord[i] = pScene->pBallCoord[i];
			}

			memcpy(pcMovements, pScene->pcMovements, sizeof(char)*pMForm->nMaxMovements); // size = nMaxMovements

			nMovementCounter = pScene->nMovementCounter;
			nLastMovementResult = 0; // 1 - game isn't finished, 2 - fuccessful finish, 3 - failed game
			bSceneCreated = true;
		}
	}
}

//------------------------------------------------------------------------------

bool __fastcall CScene::CreateScene(unsigned int nNewBoardSize, unsigned int nNewBallsAmount, unsigned int nNewFirstSceneBallsAmount)
{
	if (bSceneCreated)
	{
		return false;
	}

	nBallsAmount = nNewBallsAmount;   // amount of balls and holes (the same value)
	nBoardSize = nNewBoardSize;       // size of board (nBoardSize x nBoardSize)
	nFirstSceneBallsAmount = nNewFirstSceneBallsAmount;

	unsigned int i;
	ppcBalls = new char * [nBoardSize];
	if (ppcBalls == NULL)
	{
		return false;
	}
	for (i = 0; i < nBoardSize; i++)
	{
		ppcBalls[i] = new char [nBoardSize];
		if (ppcBalls[i] == NULL)
		{
			return false;
		}
		setmem(ppcBalls[i], sizeof(char)*nBoardSize, 0);
	}

	ppcHoles = new char * [nBoardSize];
	if (ppcHoles == NULL)
	{
		return false;
	}
	for (i = 0; i < nBoardSize; i++)
	{
		ppcHoles[i] = new char [nBoardSize];
		if (ppcHoles[i] == NULL)
		{
			return false;
		}
		setmem(ppcHoles[i], sizeof(char)*nBoardSize, 0);
	}

	pcMovementLines = new char [nBoardSize]; // Lines for movment on a board
	if (pcMovementLines == NULL)
	{
		return false;
	}

	unsigned int nSize = pMForm->nMaxMovements;
	pcMovements = new char [nSize]; // size = nMaxMovements
	if (pcMovements == NULL)
	{
		return false;
	}
	setmem(pcMovements, sizeof(char)*nSize, 0);

	pBallCoord = new CBallCoord [nFirstSceneBallsAmount]; // COORD
	if (pBallCoord == NULL)
	{
		return false;
	}
	for (i = 0; i < nFirstSceneBallsAmount; i++)
	{
		pBallCoord[i].bExist = false;
		pBallCoord[i].Ball.x = pBallCoord[i].Ball.y = 0;
		pBallCoord[i].Ball.number = 0;
	}

	nMovementCounter = 0;
	nLastMovementResult = 0; // 1 - game isn't finished, 2 - fuccessful finish, 3 - failed game
	bSceneCreated = true;

	return true;
}

//------------------------------------------------------------------------------

bool CScene::DeleteScene()
{
	if (!bSceneCreated)
	{
		return false;
	}

	unsigned int i;
	if (ppcBalls != NULL)
	{
		for (i = 0; i < nBoardSize; i++)
		{
			if (ppcBalls[i] != NULL)
			{
				delete [] ppcBalls[i];
			}
		}
		delete [] ppcBalls;
		ppcBalls = NULL;
	}

	if (ppcHoles != NULL)
	{
		for (i = 0; i < nBoardSize; i++)
		{
			if (ppcHoles[i] != NULL)
			{
				delete [] ppcHoles[i];
			}
		}
		delete [] ppcHoles;
		ppcHoles = NULL;
	}

	if (pcMovementLines != NULL)
	{
		delete [] pcMovementLines;
		pcMovementLines = NULL;
	}

	if (pcMovements != NULL)
	{
		delete [] pcMovements;
		pcMovements = NULL;
	}

	if (pBallCoord != NULL)
	{
		delete [] pBallCoord;
		pBallCoord = NULL;
	}

	nBallsAmount = nBoardSize = 0;
	nMovementCounter = 0;
	nLastMovementResult = 0;

	bSceneCreated = false;
	return true;
}

//------------------------------------------------------------------------------

CScene::~CScene()
{
	DeleteScene();
};

//------------------------------------------------------------------------------

unsigned int CScene::GetMovementCounter()
{
	return nMovementCounter;
}

//------------------------------------------------------------------------------

int CScene::GetFirstSceneBallsAmount()
{
	return nFirstSceneBallsAmount;
};

//------------------------------------------------------------------------------

int CScene::GetBoardSize()
{
	return nBoardSize;
}

//------------------------------------------------------------------------------
// 0 - can't move, 1 - successful move (some ball could win), 2 - win, 3 - game failed
char CScene::GetMovementResult()
{
	return (char)nLastMovementResult;
}

//------------------------------------------------------------------------------

void CScene::ResetMovementResult()
{
	nLastMovementResult = 4;
}

//------------------------------------------------------------------------------

bool CScene::IsCreated()
{
	return bSceneCreated;
}

//------------------------------------------------------------------------------

void __fastcall CScene::SetMainPointers(TApplication* pNewApp, TMainForm* pNewMForm, CCalcBase* pNewCalcBase)
{
	if (pNewApp != NULL)
	{
		pApp = pNewApp;
	}

	if (pNewMForm != NULL)
	{
		pMForm = pNewMForm;
	}

	if (pNewCalcBase != NULL)
	{
		pCalcBase = pNewCalcBase;
	}
}

//------------------------------------------------------------------------------

void __fastcall CScene::AddMovment(const char cMoveDirection)
{
	pcMovements[nMovementCounter++] = cMoveDirection;
}

//------------------------------------------------------------------------------

char __fastcall CScene::GetMovement(const unsigned int i)
{
	if (i < nMovementCounter)
	{
		return pcMovements[i];
	}
	else
	{
		return '0';
	}
}

//------------------------------------------------------------------------------
int __fastcall CScene::ReadNumber(const UnicodeString & str, const int nStrLen, int & nStart, bool & bError)
{
	UnicodeString digit = "";
	unsigned short int nCounter = 0;
	while (nStart <= nStrLen && str[nStart] != ' ')
	{
		digit += str[nStart++];
		if (++nCounter > pMForm->nMaxDigitInNumber)
		{
			bError = true;
			return 0;
		}
	}

	return digit.ToInt();
}
//------------------------------------------------------------------------------

bool __fastcall CScene::InputDataParser(UnicodeString & str, CCalcBase* pCalcBase)
{
	if (!str.IsEmpty())
	{
		int n = str.Length();
		int i, j;

		// checking for invalid symbols
		for (i = 1; i <= n; i++)
		{
			if (((str[i] >= '0') && (str[i] <= '9')) || (str[i] == ' '))
			{
				continue;
			}
			else
			{
				return false;
			}
		}

		// removing duble space charachters
		for (i = 1; i <= n-1; i++)
		{
			if (str[i] == ' ' && str[i+1] == ' ')
			{
				for (int j = i; j <= str.Length()-1; j++)
				{
					str[j] = str[j+1];
				}
				str.SetLength(str.Length() - 1);
				n--;
				i--;
			}
		}

		// removing first and end spaces
		if (str[1] == ' ')
		{
			for (i = 1; i <= str.Length()-1; i++)
			{
				str[i] = str[i+1];
			}
			str.SetLength(str.Length()-1);
		}
		if (str[str.Length()] == ' ')
		{
			str.SetLength(str.Length()-1);
		}

		// clalculating amount of digits in string via spaces character amount
		unsigned int nDigitsCounter = 0;
		n = str.Length();
		for (i = 1; i <= n; i++)
		{
			if (str[i] == ' ')
			{
				nDigitsCounter++;
			}
		}
		nDigitsCounter++; // the last digit

		bool bError = false;

		// reading first 3 numbers
		j = 1;
		n = str.Length();
		int nNewBoardSize = ReadNumber(str, n, j, bError);    // board size
		j++;
		int nNewBallsAmount = ReadNumber(str, n, j, bError);  // amount of balls and holes (the same value)
		j++;
		int nNewBorderAmount = ReadNumber(str, n, j, bError); // amount of borders on a board
		j++;

		if (bError) // too long number
		{
			return false;
		}
		if (nNewBoardSize <= 0 || nNewBallsAmount <= 0 || nNewBorderAmount < 0)
			return false;
		if (nNewBoardSize < 2) // board is too small
			return false;
		if (nNewBallsAmount > pMForm->nMaxBallsAmount)
			return false;

		// check amount of digits in string (real and math estimation)
		unsigned int nMathDigitsEstim = 3 + nNewBallsAmount*2*2 + nNewBorderAmount*4;
		if (nDigitsCounter != nMathDigitsEstim)
		{
			return false;
		}

		// create arrays for balls and holes
		if (!CreateScene(nNewBoardSize, nNewBallsAmount, nNewBallsAmount))
		{
			return false;
		}
		// create arrays for borders
		if (!pCalcBase->CreateBase(nBoardSize, nNewBorderAmount))
		{
			return false;
		}

		unsigned int nStrLen = str.Length();
		int nNumber = 0;

		// reading balls coordinates
		CBall TempBall;
		for (i = 0; i < nBallsAmount; i++)
		{
			// x coord
			nNumber = ReadNumber(str, nStrLen, j, bError);
			if (nNumber <= 0)
			{
				DeleteScene();
				return false;
			}
			TempBall.x = nNumber - 1; // in C++ format [0..BoardSize-1]
			j++;
			// y coord
			nNumber = ReadNumber(str, nStrLen, j, bError);
			if (nNumber <= 0)
			{
				DeleteScene();
				return false;
			}
			TempBall.y = nNumber - 1; // in C++ format [0..BoardSize-1]
			j++;

			if (TempBall.x >= nBoardSize || TempBall.y >= nBoardSize)
			{
				DeleteScene();
				return false;
			}

			TempBall.number = i+1;     			       // save ball  number
			ppcBalls[TempBall.y][TempBall.x] = i+1;    // save number of ball
			pBallCoord[i].Ball = TempBall;         // Coord
			pBallCoord[i].bExist = true;           //
		}

		// reading holes coordinates
		unsigned int HolesCoordX, HolesCoordY;
		for (i = 0; i < nBallsAmount; i++)
		{
			// x coord
			nNumber = ReadNumber(str, nStrLen, j, bError);
			if (nNumber <= 0)
			{
				DeleteScene();
				return false;
			}
			HolesCoordX = nNumber - 1; // size of board (nBoardSize x nBoardSize)
			j++;
			// y coord
			nNumber = ReadNumber(str, nStrLen, j, bError);
			if (nNumber <= 0)
			{
				DeleteScene();
				return false;
			}
			HolesCoordY = nNumber - 1; // size of board (nBoardSize x nBoardSize)
			j++;

			if (HolesCoordX >= nBoardSize || HolesCoordY >= nBoardSize)
			{
				DeleteScene();
				return false;
			}

			ppcHoles[HolesCoordY][HolesCoordX] = i + 1; // save number of holes
		}

		if (bError) // too long number
		{
			return false;
		}

		// reading borders (4 coord)
		unsigned int BorderCoordX, BorderCoordY;
		bool SecondPoint = false;
		CBorder TempBorder;
		unsigned int nCurrentBorder = 0;
		float fBorderLen = 0;
		for (i = 0; i < nNewBorderAmount * 2; i++)
		{
			// x coord
			nNumber = ReadNumber(str, nStrLen, j, bError);
			if (nNumber <= 0)
			{
				DeleteScene();
				return false;
			}
			BorderCoordX = nNumber - 1; // size of board (nBoardSize x nBoardSize)
			j++;
			// y coord
			nNumber = ReadNumber(str, nStrLen, j, bError);
			if (nNumber <= 0)
			{
				DeleteScene();
				return false;
			}
			BorderCoordY = nNumber - 1; // size of board (nBoardSize x nBoardSize)
			j++;

			if (BorderCoordX >= nBoardSize || BorderCoordY >= nBoardSize)
			{
				DeleteScene();
				return false;
			}

			// to array
			pCalcBase->ppcBorders[BorderCoordY][BorderCoordX] = 1;
			// save coordinates
			if (!SecondPoint)
			{
				TempBorder.p1.x = BorderCoordX;
				TempBorder.p1.y = BorderCoordY;
				SecondPoint = true;
			}
			else
			{
				TempBorder.p2.x = BorderCoordX;
				TempBorder.p2.y = BorderCoordY;
				// cheking for lenght from p1 to p2 (must be = 1)
				fBorderLen = sqrt((float)pMForm->sqr(TempBorder.p2.x - TempBorder.p1.x) + pMForm->sqr(TempBorder.p2.y - TempBorder.p1.y));
				if (fabs(fBorderLen - 1) > 0.005)
				{
					return false;
				}
				pCalcBase->pBorderCoord[nCurrentBorder++] = TempBorder;
				SecondPoint = false;
			}
		}

		if (bError) // too long number
		{
			return false;
		}

		return true;
	}

	return false;
}

//------------------------------------------------------------------------------

void __fastcall CScene::PrintArraysToMemo(TMemo * pMemo)
{
	if (pMemo != NULL)
	{
		UnicodeString sOutputStr;
		// write balls & holes on a board
		pMemo->Lines->Add("Board with balls (B) and holes (H):");
		for (int i = 0; i < nBoardSize; i++)
		{
			sOutputStr = "[";
			for (int j = 0; j < nBoardSize; j++)
			{
				if (j > 0)
				{
					sOutputStr += " ";
				}

				if (ppcBalls[i][j] != 0) // ball
				{
					sOutputStr += "B";
				}
				else
				if (ppcHoles[i][j] != 0) // hole
				{
					sOutputStr += "H";
				}
				else
				{
					sOutputStr += "o"; // empty field
				}
			}
			sOutputStr += "]";
			pMemo->Lines->Add(sOutputStr);
		}
		pMemo->Lines->Add(" ");
		// write borders
		pMemo->Lines->Add("Border matrix:");
		for (int i = 0; i < nBoardSize; i++)
		{
			sOutputStr = "[";
			for (int j = 0; j < nBoardSize; j++)
			{
				if (j > 0)
				{
					sOutputStr += " ";
				}

				if (pCalcBase->ppcBorders[i][j] == 1) // thes cell is used in some border
				{
					sOutputStr += "1";
				}
				else
				{
					sOutputStr += "o"; // empty field
				}
			}
			sOutputStr += "]";
			pMemo->Lines->Add(sOutputStr);
		}

		// movments
		pMemo->Lines->Add(" ");
		pMemo->Lines->Add("Movments:");
		pMemo->Lines->Add("   Counter: " + IntToStr((int)nMovementCounter));
		if	((int)nMovementCounter > 0)
		{
			pMemo->Lines->Add("   String: " + UnicodeString(pcMovements));
		}
	}
}

//------------------------------------------------------------------------------

void __fastcall CScene::DrawSceneOnImage(TImage * pImage)
{
	float nW = pImage->Width;
	float nH = pImage->Height;
	unsigned int i = 0, j = 0;
	float fCellWidth = nW / nBoardSize, fCellHeight = nH / nBoardSize;
	TRectF CellRect(0, 0, 0, 0);

	pImage->Bitmap->Canvas->BeginScene();
	pImage->Bitmap->Canvas->Clear(claWhite);

	if (nBoardSize >= 20)
	{
		CellRect.init(1, 1, nW - 1, nH - 1);
		pImage->Bitmap->Canvas->Fill->Color = 0xFF000000;
		pImage->Bitmap->Canvas->FillText(CellRect, "Scene too big to be displayed here!", false, 100 , TFillTextFlags() << TFillTextFlag::RightToLeft, TTextAlign::Center, TTextAlign::Center);

		// outer border
		pImage->Bitmap->Canvas->Stroke->Color = 0xFF555555;
		pImage->Bitmap->Canvas->Stroke->Thickness = 2;
		pImage->Bitmap->Canvas->DrawRect(TRectF(0, 0, nW, nH), 0, 0, AllCorners, 1);
		pImage->Bitmap->Canvas->EndScene();
		return;
	}

	// drawing cells
	pImage->Bitmap->Canvas->Stroke->Color = 0xFF888888;
	pImage->Bitmap->Canvas->Stroke->Thickness = 1;
	for (i = 0; i < nBoardSize; i++)
	{
		for (j = 0; j < nBoardSize; j++)
		{
			CellRect.init(i*fCellWidth, j*fCellHeight, (i+1)*fCellWidth+1, (j+1)*fCellHeight+1);
			pImage->Bitmap->Canvas->DrawRect(CellRect,0,0, AllCorners, 1);
		}
	}

	// drawing balls and holes
	pImage->Bitmap->Canvas->Stroke->Color = 0xFF000000;
	UnicodeString str;
	pImage->Bitmap->Canvas->Font->Size = 11;
	int nReservedSpace;
	if (nBoardSize < 10)
	{
		nReservedSpace = 6;
	}
	else
	{
		nReservedSpace = 3;
	}
	for (i = 0; i < nBoardSize; i++)
	{
		for (j = 0; j < nBoardSize; j++)
		{
			CellRect.init(i*fCellWidth + 1 + nReservedSpace, j*fCellHeight + 1 + nReservedSpace, (i+1)*fCellWidth - 1 - nReservedSpace, (j+1)*fCellHeight - 1 - nReservedSpace);
			if (ppcBalls[j][i] != 0)
			{
				pImage->Bitmap->Canvas->Fill->Color = 0xFF000000;
				pImage->Bitmap->Canvas->FillEllipse(CellRect, 1);
				if (nBoardSize >= 12)
				{
				   str = IntToStr(ppcBalls[j][i]);
				}
				else
				if (nBoardSize >= 8)
				{
					str = "B" + IntToStr(ppcBalls[j][i]);
				}
				else
				if (nBoardSize < 8)
				{
					str = "ball №" + IntToStr(ppcBalls[j][i]);
				}
				pImage->Bitmap->Canvas->Fill->Color = 0xFFFFFFFF;
				pImage->Bitmap->Canvas->FillText(CellRect, str, false, 100 , TFillTextFlags() << TFillTextFlag::RightToLeft, TTextAlign::Center, TTextAlign::Center);
			}
			else
			if (ppcHoles[j][i] != 0)
			{
				pImage->Bitmap->Canvas->Fill->Color = 0xFFBBBBBB;
				pImage->Bitmap->Canvas->FillEllipse(CellRect, 1);
				if (nBoardSize >= 12)
				{
				   str = IntToStr(ppcHoles[j][i]);
				}
				else
				if (nBoardSize >= 8)
				{
					str = "H" + IntToStr(ppcHoles[j][i]);
				}
				else
				if (nBoardSize < 8)
				{
					str = "hole №" + IntToStr(ppcHoles[j][i]);
				}
				pImage->Bitmap->Canvas->Fill->Color = 0xFF000000;
				pImage->Bitmap->Canvas->FillText(CellRect, str, false, 100 , TFillTextFlags() << TFillTextFlag::RightToLeft, TTextAlign::Center, TTextAlign::Center);
			}
		}
	}

	// drawing of borders
	// horisontal borders
	CBorder TempBorder;
	for (i = 0; i < nBoardSize; i++)
	{
		for (j = 0; j < nBoardSize-1; j++)
		{
			if (pCalcBase->ppcBorders[j][i] == 1 && pCalcBase->ppcBorders[j+1][i] == 1)
			{
				TempBorder.p1.SetCoord(i, j);
				TempBorder.p2.SetCoord(i, j + 1);
				if (pCalcBase->IsItRealBorder(TempBorder))
				{
					CellRect.init(i*fCellWidth + 1, (j+1)*fCellHeight + 1 - nReservedSpace, (i+1)*fCellWidth, (j+1)*fCellHeight + nReservedSpace);
					pImage->Bitmap->Canvas->Fill->Color = 0xFF770000;
					pImage->Bitmap->Canvas->FillRect(CellRect, 0,0, AllCorners, 1);
				}
			}
		}
	}
	// vertical borders
	for (j = 0; j < nBoardSize; j++)
	{
		for (i = 0; i < nBoardSize-1; i++)
		{
			if (pCalcBase->ppcBorders[j][i] == 1 && pCalcBase->ppcBorders[j][i+1] == 1)
			{
				TempBorder.p1.SetCoord(i, j);
				TempBorder.p2.SetCoord(i + 1, j);
				if (pCalcBase->IsItRealBorder(TempBorder))
				{
					CellRect.init((i+1)*fCellWidth + 1 - nReservedSpace, j*fCellHeight + 1, (i+1)*fCellWidth + nReservedSpace, (j+1)*fCellHeight);
					pImage->Bitmap->Canvas->Fill->Color = 0xFF770000;
					pImage->Bitmap->Canvas->FillRect(CellRect, 0,0, AllCorners, 1);
				}
			}
		}
	}

	// outer border
	pImage->Bitmap->Canvas->Stroke->Color = 0xFF555555;
	pImage->Bitmap->Canvas->Stroke->Thickness = 2;
	pImage->Bitmap->Canvas->DrawRect(TRectF(0, 0, nW, nH), 0, 0, AllCorners, 1);
	pImage->Bitmap->Canvas->EndScene();
}

//------------------------------------------------------------------------------
// find all lines to move
//------------------------------------------------------------------------------
void __fastcall CScene::FindMovementLines(const char cDirection)
{
	unsigned int i = 0;
	setmem(pcMovementLines, nBoardSize, 0); // clear
	if (cDirection == 'U' || cDirection == 'D')
	{
		for (i = 0; i < nFirstSceneBallsAmount; i++)
		{
			if (pBallCoord[i].bExist)
			{
				pcMovementLines[pBallCoord[i].Ball.x] += 1; // this is active line
			}
		}
	}
	else // R or L
	{
		for (i = 0; i < nFirstSceneBallsAmount; i++)
		{
			if (pBallCoord[i].bExist)
			{
				pcMovementLines[pBallCoord[i].Ball.y] += 1; // this is active line
			}
		}
	}
}

//------------------------------------------------------------------------------

bool __fastcall CScene::CanMoveThere(const CBall* pBall, const CBall* pNewPosition)
{
	CBorder Border(*pBall, *pNewPosition);

	// is ball out of board?
	if (pNewPosition->x < 0 || pNewPosition->x >= nBoardSize)
	{
		return false;
	}

	if (pNewPosition->y < 0 || pNewPosition->y >= nBoardSize)
	{
		return false;
	}

	if (pCalcBase->IsItRealBorder(Border) ||             // there is a border there
	   ppcBalls[pNewPosition->y][pNewPosition->x] != 0) // finish cell is not empty
	{
		return false;
	}
	else
	{
		return true;  // you can move there
	}
}

//------------------------------------------------------------------------------
// 0 - no hole, 1 - successful hole, 2 - unsutable hole (game failed)
char __fastcall CScene::HaveGotToAnyHole(const CBall * pBall)
{
	int nHoleNum = ppcHoles[pBall->y][pBall->x];
	if (nHoleNum > 0)
	{
		if (nHoleNum == pBall->number)
		{
			return 1;  // correct hole
		}
		else
		{
			return 2; // incorrect hole
		}
	}

	return 0;
}

//------------------------------------------------------------------------------
// 0 - can't move, 1 - successful move, 2 - ball won, 3 - game failed
char __fastcall CScene::OneBallMovemet(CBall * pBall, const char cDirection)
{
	int nStepX; // for 'D', 'R'
	int nStepY;
	if (cDirection == 'U')
	{
		nStepY = -1;
		nStepX = 0;
	}
	else
	if (cDirection == 'R')
	{
		nStepY = 0;
		nStepX = 1;
	}
	else
	if (cDirection == 'D')
	{
		nStepY = 1;
		nStepX = 0;
	}
	else // L
	{
		nStepY = 0;
		nStepX = -1;
	}

	CBall NewPosition;
	NewPosition.x = pBall->x;
	NewPosition.y = pBall->y;
	char cBallNumber, cResult = 0, cGotToHoleResult;
	bool bContinue = true;
	int nNumber;

	do {
		NewPosition.x += nStepX;
		NewPosition.y += nStepY;
		// checing on borders and other bools
		if (CanMoveThere(pBall, &NewPosition))
		{
			// moving presence in array ppcBall
			cBallNumber = ppcBalls[pBall->y][pBall->x];
			ppcBalls[NewPosition.y][NewPosition.x] = cBallNumber;
			NewPosition.number = cBallNumber;
			ppcBalls[pBall->y][pBall->x] = 0;

			// moving a ball
			*pBall = NewPosition; // moving coordinates

			cGotToHoleResult = HaveGotToAnyHole(pBall);
			if (cGotToHoleResult > 0) // cheking on holes
			{
				if (cGotToHoleResult == 1) // successfull hole
				{
					cResult = 2;
				}
				else
				if (cGotToHoleResult == 2) // failed
				{
					cResult = 3;
				}

				nNumber = ppcBalls[pBall->y][pBall->x];

				// present matrixes
				ppcBalls[pBall->y][pBall->x] = 0;
				ppcHoles[pBall->y][pBall->x] = 0;

				// direct array
				pBallCoord[nNumber - 1].bExist = false;

				nBallsAmount--; // amount of balls and holes (the same value)
				bContinue = false;
			}
			else
			{
				cResult = 1;
			}
		}
		else
		{
			bContinue = false;
		}
	} while (bContinue);

	return cResult;
}

//------------------------------------------------------------------------------
// Commit movement for ALL LINES and ALL BALLS
// 0 - can't move, 1 - successful move (some ball could win), 2 - win, 3 - game failed
//------------------------------------------------------------------------------
char __fastcall CScene::CommitMovment(const char cDirection)
{
	int i, j, nStep, nStart, nBallNumber;
	CBall* pBall;
	char cResult;
	bool bSceneChanged = false;
	for	(i = 0; (i < nBoardSize) && (bCalcContinue); i++) // for ALL LINES
	{
		if (pcMovementLines[i] == 0) // no line in this coord
		{
			continue;
		}
		else
		{
			if (cDirection == 'U' || cDirection == 'D')
			{
				if (cDirection == 'U')
				{
					nStep = 1;  // go from UP to DOWN
					nStart = 1; // we can not move 0 to UP
				} else
				if (cDirection == 'D')
				{
					nStep = -1;
					nStart = nBoardSize-2; // we can not move the last to down
				}

				// for ALL bolls in thi LINE
				for (j = nStart; (j < nBoardSize) && (j >= 0) && (bCalcContinue); j += nStep)
				{
				   if (ppcBalls[j][i] > 0)
				   {
						nBallNumber = ppcBalls[j][i];                // get Number
						pBall = &(pBallCoord[nBallNumber - 1].Ball); // get coord

						// 0 - can't move, 1 - successful move, 2 - ball won, 3 - game failed
						cResult = OneBallMovemet(pBall, cDirection);

						if (cResult != 0)
						{
							bSceneChanged = true;
						}

						if (cResult == 3) // fail
						{
							break;
						}
				   }
				}
			}
			else
			if (cDirection == 'L' || cDirection == 'R')
			{
				if (cDirection == 'L')
				{
					nStep = 1;  // go from LEFT to RIGHT
					nStart = 1; // we can not move 0 to LEFT
				} else
				if (cDirection == 'R')
				{
					nStep = -1;
					nStart = nBoardSize-2; // we can not move the last to RIGHT
				}

				// for ALL bolls in this LINE
				for (j = nStart; (j < nBoardSize) && (j >= 0) && (bCalcContinue); j += nStep)
				{
				   if (ppcBalls[i][j] > 0)
				   {
						nBallNumber = ppcBalls[i][j];                // get Number
						pBall = &(pBallCoord[nBallNumber - 1].Ball); // get coord

						// 0 - can't move, 1 - successful move, 2 - ball won, 3 - game failed
						cResult = OneBallMovemet(pBall, cDirection);

						if (cResult != 0)
						{
							bSceneChanged = true;
						}

						if (cResult == 3) // fail
						{
							break;
						}
				   }
				}
			}

			if (cResult == 2 && nBallsAmount == 0) // win
			{
				return 2;
			} else
			if (cResult == 3) // game is failed
			{
				return 3;
			}
		}
	}

	if (!bSceneChanged)
	{
		return 3; // IF SCENE was not changed we consider this like fail (тупиковая ветвь игры)
	}

	return 0;
}

//------------------------------------------------------------------------------
// 0 - can't move, 1 - successful move (some ball could win), 2 - win, 3 - game failed
//------------------------------------------------------------------------------
char __fastcall CScene::MoveScene(const char cDirection)
{
	// find all lines
	FindMovementLines(cDirection);

	// commit movement for all lines
	char cResult = CommitMovment(cDirection);

	nLastMovementResult = cResult;

	return cResult;
}

//------------------------------------------------------------------------------

char CScene::GetTheLastMovement()
{
	if (nMovementCounter > 0)
	{
		return pcMovements[nMovementCounter - 1];
	}
	else
	{
		return '0';
	}
}

//------------------------------------------------------------------------------
