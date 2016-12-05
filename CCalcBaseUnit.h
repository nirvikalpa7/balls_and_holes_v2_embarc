
#ifndef CCalcBaseUnitH
#define CCalcBaseUnitH

//---------------------------------------------------------------------------

#include <fmx.h>

#include "MainFormUnit.h"
#include "BasicDataTypesUnit.h"
#include "CSceneUnit.h"

//------------------------------------------------------------------------------
// Class for all calculations. (2nd important class)
//------------------------------------------------------------------------------

class CCalcBase {
public:
	CCalcBase();
	~CCalcBase();

	static void __fastcall SetMainPointers(TApplication* pNewApp, TMainForm* pNewMForm);

	bool __fastcall CreateBase(unsigned int nNewBoardSize, unsigned int nNewBorderAmount);
	bool DeleteBase();

	void __fastcall FirstLoad(const CScene* pStartScene);
	bool __fastcall ThreadGlobalCalc(const char cMode);
	void CalcCurrentList1(); // 1 thread   cThreadsMode - 1, 2, 4
	void CalcCurrentList2(); // 2 threads  cThreadsMode - 2, 4
	void CalcCurrentList3(); // 4 threads  cThreadsMode - 4
	void CalcCurrentList4(); // 4 threads  cThreadsMode - 4
	bool GenNewVariantsToFutureList();
	void LoadFirstList();

	void __fastcall GetStrInNorthSouthNotation(const UnicodeString & sUpDownNotation, UnicodeString & sNorthSouthNotation);
	void __fastcall PrintSuccessfulResultsInMemo(TMemo * pOutpMemo);

	bool IsCreated();
	bool __fastcall IsItRealBorder(const CBorder & Border);

	int GetElapsedTicks();
	int GetUnsolvedSceneAmount();
	int GetResolvedSceneAmount();
	int GetSceneAmount();
	int GetBoardSize();
	int GetBallAmount();

	void SetStartTime();
	void ResetAllSolvedVariants();

	void __fastcall ClearCalcVectorFull(std::vector<CScene *> & Vector);

	bool bCalcContinue;

private:

	// for multi threads support
	bool bThreadFinished[4];          // flags, from 0 ... 3
	__int64 nThreadSolvedVariants[4]; // solved variants

	// main pinters for work
	static TApplication* pApp;
	static TMainForm* pMForm;

	std::vector<CScene *> CurrentTaskVector;
	unsigned __int64 nUnsolvedSceneAmount;
	unsigned __int64 nSceneAmount;
	std::vector<CScene *> FutureTaskVector;

	unsigned int nBorderAmount; // amount of borders on a board
	char** ppcBorders;          // border array
	CBorder* pBorderCoord;     // Borders coordinates (to find real borders from unreal)
	unsigned int nBoardSize;

	char cThreadsMode; // mode for multi threads calculation (1, 2, 4)
	bool bBaseCreated;
	unsigned int nSuccessfulResultAmount; // which results are successful?

	friend CScene; // the class CScene will save borders to this class private member ppcBorders

	unsigned __int64 nStartTickCount;
	unsigned __int64 nElapsedTicks;
	unsigned __int64 nAllSolvedVariants;

}; // CCalcBase class

//------------------------------------------------------------------------------
#endif
