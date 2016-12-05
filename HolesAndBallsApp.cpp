//------------------------------------------------------------------------------
// (c) Dmitry Sidelnikov, Nov 2016, nirvikalpa@ya.ru
//------------------------------------------------------------------------------

#include <fmx.h>
#ifdef _WIN32
#include <tchar.h>
#endif
#pragma hdrstop
#include <System.StartUpCopy.hpp>

//------------------------------------------------------------------------------
USEFORM("MainFormUnit.cpp", MainForm);
//------------------------------------------------------------------------------
extern "C" int FMXmain()
{
	try
	{
		Application->Initialize();
		Application->CreateForm(__classid(TMainForm), &MainForm);
		Application->Run();
	}
	catch (Exception &exception)
	{
		Application->ShowException(&exception);
	}
	catch (...)
	{
		try
		{
			throw Exception("");
		}
		catch (Exception &exception)
		{
			Application->ShowException(&exception);
		}
	}
	return 0;
}
//------------------------------------------------------------------------------
