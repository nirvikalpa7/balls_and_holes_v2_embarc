
#ifndef CSceneUnitH
#define CSceneUnitH

//------------------------------------------------------------------------------

#include <fmx.h>
#include <vector>

#include "MainFormUnit.h"
#include "CCalcBaseUnit.h"

//------------------------------------------------------------------------------
// One state on a board. (1st important class)
//------------------------------------------------------------------------------
class CScene {
public:

	CScene();
	CScene(const CScene * pScene);
	~CScene();
	void ZeroInit();
	bool __fastcall CreateScene(unsigned int nNewBoardSize, unsigned int nNewBallsAmount, unsigned int nNewFirstSceneBallsAmount);
	bool DeleteScene();

	static void __fastcall SetMainPointers(TApplication* pApp, TMainForm* pForm, CCalcBase* pCalcBase);

	bool __fastcall InputDataParser(UnicodeString & str, CCalcBase * pCalcBase);
	void __fastcall PrintArraysToMemo(TMemo * pMemo);
	void __fastcall DrawSceneOnImage(TImage * pImage);

	char __fastcall MoveScene(const char cDirection); // 0 - can't move, 1 - usual move (maybe partly succesful), 2 - game failed

	bool IsCreated();

	void __fastcall AddMovment(const char cMoveDirection);
	char __fastcall GetMovement(const unsigned int i);
	unsigned int GetMovementCounter();
	char GetTheLastMovement();
	int GetFirstSceneBallsAmount();
	int GetBoardSize();
	char GetMovementResult(); // 0 - can't move, 1 - successful move (some ball could win), 2 - win, 3 - game failed
	void ResetMovementResult();

	static bool bCalcContinue; // to stop calculating process

private:

	int __fastcall ReadNumber(const UnicodeString & str, const int nLen, int & nStart, bool & bError);

	void __fastcall FindMovementLines(const char cDirection); // find all lines
	char __fastcall CommitMovment(const char cDirection); // 0 - can't move, 1 - successful move (some ball could win), 2 - win, 3 - game failed
	char __fastcall OneBallMovemet(CBall * pBall, const char cDirection);
	bool __fastcall CanMoveThere(const CBall* pBall, const CBall* pNewPosition);
	char __fastcall HaveGotToAnyHole(const CBall * pBall); // 0 - no hole, 1 - successful hole, 2 - unsutable hole (game failed)

	// main pinters for work
	static TApplication* pApp;
	static TMainForm* pMForm;
	static CCalcBase* pCalcBase;

	bool bSceneCreated; 		// is scene allocated in memory?
	unsigned int nBallsAmount;  // amount of balls and holes (the same value)
	unsigned int nFirstSceneBallsAmount; // amount of balls in FIRST SCENE!
	unsigned int nBoardSize;    // size of board (nBoardSize x nBoardSize)

	char** ppcBalls;  // presence matrix
	char** ppcHoles;  //

	CBallCoord* pBallCoord; // pointers to BallCoordinates by index (number);

	char* pcMovementLines; // Lines to move, size = BorderSize

	char* pcMovements;     // Directions of movments, size = nMaxMovements, 'U','R','D','L'
	unsigned int nMovementCounter;

	unsigned short int nLastMovementResult; // 0 - can't move, 1 - successful move (some ball could win), 2 - win, 3 - game failed

}; // CSene class

//------------------------------------------------------------------------------
#endif
