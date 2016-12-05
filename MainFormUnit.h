

#ifndef MainFormUnitH
#define MainFormUnitH

//------------------------------------------------------------------------------

#include <System.Classes.hpp>
#include <FMX.Controls.hpp>
#include <FMX.Forms.hpp>
#include <FMX.Controls.Presentation.hpp>
#include <FMX.Edit.hpp>
#include <FMX.Memo.hpp>
#include <FMX.Objects.hpp>
#include <FMX.ScrollBox.hpp>
#include <FMX.StdCtrls.hpp>
#include <FMX.Types.hpp>
#include <FMX.ListBox.hpp>
#include <FMX.TabControl.hpp>

class CScene;  // one current state
class CCalcBase; // all calc processes

//------------------------------------------------------------------------------

class TMainForm : public TForm
{
__published:	// IDE-managed Components
	TButton *FindSolutionButton;
	TEdit *InputEdit;
	TLabel *InputLabel;
	TImage *SceneImage;
	TButton *LoadInputDataButton;
	TProgressBar *ProgrBar;
	TComboBox *InputComboBox;
	TLabel *ProgressLabel;
	TGroupBox *GroupBox1;
	TLabel *TaskAmountAndSolvedLabel;
	TLabel *MovmentsLabel;
	TLabel *ElapsedTimeLabel;
	TLabel *CPUsLabel;
	TLabel *AllSolvedVariantsLabel;
	TLabel *UsedMemoryLabel;
	TCheckBox *CalcDelayCheckBox;
	TTabControl *TabControl1;
	TTabItem *TabItem1;
	TTabItem *TabItem2;
	TMemo *OutputMemo;
	TMemo *HelpMemo;
	TComboBox *CPU_ComboBox;
	void __fastcall LoadInputDataButtonClick(TObject *Sender);
	void __fastcall FormCreate(TObject *Sender);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	void __fastcall InputComboBoxChange(TObject *Sender);
	void __fastcall FindSolutionButtonClick(TObject *Sender);

private:	// User declarations

	// THE MOST IMPORTANT 2 OBJECTS
	CScene* pStartScene;  // one current state
	CCalcBase* pCalcBase; // all calc processes

	int nAvalibleNumCPUs;
	int nUsedNumCPUs;

public:		// User declarations
	__fastcall TMainForm(TComponent* Owner);

	void __fastcall DisplayProgressBar(int nCurSceneAmount, int nResolvedSceneAmount);
	void __fastcall DisplayProgressBar();
	void __fastcall DisplayElapsedTime(const __int64 nElapsedTime);
	void __fastcall DisplayMovmentsCounter(const unsigned int nMov);
	void __fastcall DisplayAllSolvedVariants(const __int64 nAllSolvedVar);
	void __fastcall DisplayUsedMemory();

	inline double __fastcall sqr(double dNum) { return dNum*dNum; }

	static const int nMaxMovements = 64;
	static const int nMaxBallsAmount = 255;
	static const int nMaxDigitInNumber = 3;
};
//------------------------------------------------------------------------------
extern PACKAGE TMainForm *MainForm;
//------------------------------------------------------------------------------
#endif
