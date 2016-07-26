// FileSystem.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include "FileHandlingServices.h"
#include <fstream>
#include "FileSystem.h"

using namespace std;

#ifdef _UNICODE
    #define tcout wcout
	#define tcin  wcin
	#define tifstream wifstream
	#define tofstream wofstream
#else
    #define tcout cout
	#define tcin  wcin
	#define tifstream ifstream
	#define tofstream ofstream
#endif

CFileSystem CFileSystem::cl_FileSystem ;

CFileSystem::CFileSystem()
{
	i_Option = -1 ;
}

CFileSystem::~CFileSystem()
{
	i_Option = 0 ;
}

//------------------------------------------------------------------------------------
//Getting singlton instance
//
CFileSystem& CFileSystem::fnGetInstance()
{
	return cl_FileSystem ;
}

//------------------------------------------------------------------------------------
//Print instruction on console
//
void CFileSystem::fnPrintInstruction()
{
	tcout << _T("***********************File System***********************\n") ;
	tcout << _T("0 - Exit program\n") ;
	tcout << _T("1 - Library creation\n") ;
	tcout <<  _T("2 - File creation\n") ;
	tcout <<  _T("3 - Library\\file duplication\n") ;
	tcout <<  _T("4 - Library\\file deletion\n") ;
	tcout <<  _T("5 - Library\\file view content\n") ;
	tcout <<  _T("6 - Print library\\file content.\n") ;
	tcout << _T("********************************************************\n\n") ;
}

//------------------------------------------------------------------------------------
//Getting relative to exe path for input path
//
CString CFileSystem::fnGetFullPathForRelativePath( const CString& nclarCSName )
{
	return FileHandlingServices::fnGetInstance().fnGetAppDir() + nclarCSName ;
}

//------------------------------------------------------------------------------------
//take input data from console
//
CString CFileSystem::fnGetDataFromConsole( bool baIgnore )
{
	TCHAR tcpName[MAX_PATH] ;
	if(baIgnore )
		tcin.ignore() ;
	tcin.getline( tcpName, MAX_PATH , _T('\n') );

	return tcpName ;
}

//------------------------------------------------------------------------------------
//1st option = Create library
//
void CFileSystem::fnCreateLibraryName()
{
	tcout << _T("Enter library name : ") ;
	CString clCSLibrary = fnGetFullPathForRelativePath( fnGetDataFromConsole() ) ;

	//Create library
	if( !FileHandlingServices::fnGetInstance().fnCreateFolder ( clCSLibrary ) )
	{
		tcout << _T("Error : Can't create library\n") ;
	}
}

//------------------------------------------------------------------------------------
//2 option = Create file
//
void CFileSystem::fnCreateFileName()
{
	tcout << _T("Enter File name : ") ;		
	CString clCSFile = fnGetFullPathForRelativePath( fnGetDataFromConsole() ) ;

	//If file extension is not specified we add default .txt extension
	if( FileHandlingServices::fnGetInstance().fnGetFileExtension( clCSFile ).IsEmpty() )
		clCSFile += _T(".txt") ;

	if( !FileHandlingServices::fnGetInstance().fnCreateFile ( clCSFile ) )
	{
		tcout << _T("Error : Can't create file\n") ;
	}
}


//------------------------------------------------------------------------------------
//3 option = Duplicating library\file
//
void CFileSystem::fnDuplicateLibraryOrFile()
{
	tcout << _T("Enter source File\\library name that you want to duplicate : ") ;	
	CString clCSSourceName = fnGetFullPathForRelativePath(fnGetDataFromConsole()) ;

	tcout << _T("Please provide new name : ") ;
	CString clCSNewName = fnGetDataFromConsole(false) ;

	tcout << _T("Please provide location to save : ") ;	
	CString clCSDestination = fnGetFullPathForRelativePath( fnGetDataFromConsole(false) ) ;
	clCSDestination.TrimRight(_T("\\")) ;
	clCSDestination = clCSDestination + _T("\\") + clCSNewName ;

	CString clCSErrMsg ;
	bool bErr = false ;
	CString clCSSourceFileExt = FileHandlingServices::fnGetInstance().fnGetFileExtension( clCSSourceName ) ;
	if( !clCSSourceFileExt.IsEmpty() )//File clone
	{
		//If extension is not specified we add source file extension
		if( FileHandlingServices::fnGetInstance().fnGetFileExtension(clCSDestination).IsEmpty() )
			clCSDestination = clCSDestination + _T(".") + clCSSourceFileExt ;

		bErr = FileHandlingServices::fnGetInstance().fnCopyFile ( clCSSourceName , clCSDestination , &clCSErrMsg ) ;
	}
	else//Library
	{
		bErr = FileHandlingServices::fnGetInstance().fnCopyFolder( clCSSourceName , clCSDestination, &clCSErrMsg ) ;
	}

	//If error
	if( !bErr )
	{
		if( clCSErrMsg.IsEmpty() )
			clCSErrMsg = _T("Error in copying.") ;
		tcout << _T("Error : ") << (LPCTSTR)clCSErrMsg << _T("\n") ;
	}
}

//------------------------------------------------------------------------------------
//4 option = Deleting library\file
//
void CFileSystem::fnDeleteLibraryOrFile()
{
	tcout << _T("Enter Library\\File name to delete : ") ;
	CString clCSName = fnGetFullPathForRelativePath( fnGetDataFromConsole() ) ;
			
	CString clCSErrMsg ;
	bool bErr = false ;
	if( !FileHandlingServices::fnGetInstance().fnGetFileExtension( clCSName ).IsEmpty() )//File
	{
		bErr = FileHandlingServices::fnGetInstance().fnDeleteFile ( clCSName , &clCSErrMsg ) ;
	}
	else//Library
	{
		bErr = FileHandlingServices::fnGetInstance().fnDeleteFolder( clCSName , clCSErrMsg ) ;
	}

	//If error
	if( !bErr )
	{
		if( clCSErrMsg.IsEmpty() )
			clCSErrMsg = _T("Error in deleting.") ;
		tcout << _T("Error : ") << (LPCTSTR)clCSErrMsg << _T("\n") ;
	}
}

//------------------------------------------------------------------------------------
//view content of file
//
void CFileSystem::fnViewContentOfFileInConsole( const CString& nclarCSFilePath )
{
	tifstream inFile;

	inFile.open( nclarCSFilePath );
	if (!inFile.is_open() ) 
	{
		tcout << _T("Unable to open file");
		return ; // terminate with error
	}
		
	//Finding size
	int iBegin = (int)inFile.tellg() ;
	inFile.seekg (0, ios::end);
	int iEnd = (int)inFile.tellg();
	inFile.seekg (0, ios::beg);
	int ilength = iEnd - iBegin ;

	//Writing content to console
	if( ilength > 0 )
		tcout << inFile.rdbuf();
		
	inFile.close();
}

//------------------------------------------------------------------------------------
//5 option = view content of file\library in console
//
void CFileSystem::fnViewContentOfLibraryOrFile()
{
	tcout << _T("Enter Library\\File name to view : ") ;
	CString clCSName = fnGetFullPathForRelativePath( fnGetDataFromConsole() ) ;
			
	if( !FileHandlingServices::fnGetInstance().fnGetFileExtension( clCSName ).IsEmpty() )//File
	{
		fnViewContentOfFileInConsole( clCSName ) ;
	}
	else//Library
	{
		CStringArray clCSArray ;
		FileHandlingServices::fnGetInstance().fnSearchDirectory( clCSName , &clCSArray ) ;
		for(int iI = 0 ; iI < clCSArray.GetSize() ; iI++ ) 
			tcout << (LPCTSTR)clCSArray[iI] << _T("\n") ;
	}
}


//------------------------------------------------------------------------------------
//6 option = Print content of file\library in printable file
//
void CFileSystem::fnPrintLibraryOrFile()
{
	tcout << _T("Enter Library\\File name to be printed : ") ;
	CString clCSSource = fnGetFullPathForRelativePath(fnGetDataFromConsole()) ;

	tcout << _T("Pleae provide location to store printable file : ");
	CString clCSDirName = fnGetFullPathForRelativePath( fnGetDataFromConsole(false) );
	clCSDirName.TrimRight( _T('\\') ) ;
	clCSDirName += _T("\\Print.txt") ;//Printable file  name

	if( !FileHandlingServices::fnGetInstance().fnGetFileExtension( clCSSource ).IsEmpty() )//File
	{
		CString clCSErrMsg ;
		if( !FileHandlingServices::fnGetInstance().fnCopyFile( clCSSource , clCSDirName, &clCSErrMsg) )
		{
			if( clCSErrMsg.IsEmpty() )
				clCSErrMsg = _T("Error in printing.") ;
			tcout << _T("Error : ") << (LPCTSTR)clCSErrMsg ; 
		}
	}
	else//Library
	{
		tofstream myfile (clCSDirName);
		if (myfile.is_open())
		{
			CStringArray clCSArray ;
			FileHandlingServices::fnGetInstance().fnSearchDirectory( clCSSource , &clCSArray ) ;
			for(int iI = 0 ; iI < clCSArray.GetSize() ; iI++ ) 
				myfile << (LPCTSTR)clCSArray[iI] << _T("\n") ;
					
			myfile.close();
		}
		else 
			tcout << _T("Unable to open library");
	}
}


//------------------------------------------------------------------------------------
//following user insturction
//
int CFileSystem::fnFollowInstruction( )
{
	switch( i_Option )
	{
	case 0:
		return 0 ;
	case 1:
		{
			fnCreateLibraryName() ;
		}
		break ;
	case 2:
		{
			fnCreateFileName() ;
		}
		break ;
	case 3:
		{
			fnDuplicateLibraryOrFile() ;
		}
		break ;
	case 4:
		{
			fnDeleteLibraryOrFile() ;
		}
		break ;
	case 5:
		{
			fnViewContentOfLibraryOrFile() ;
		}
		break ;
	case 6:
		{
			fnPrintLibraryOrFile() ;
		}
		break ;
	default :
		tcout << _T("Invalid option selected.") ;
		break ;
	}
	return 1 ;
}

//Entry function
int _tmain(int argc, _TCHAR* argv[])
{
	//Getting singlton object
	CFileSystem* clpFileSystem = &CFileSystem::fnGetInstance() ;
	if(!clpFileSystem)
	{
		ASSERT(FALSE);
		return 0 ;
	}

	//Showing instructions and handle those instructions
	for( ; ; )
	{
		//Showing instruction
		clpFileSystem->fnPrintInstruction() ;

		//Getting user input
		int iSelOption = -1 ;
		tcin >> iSelOption ;
		clpFileSystem->fnSetOption(iSelOption) ;

		//Handling instruction
		if( 0 == clpFileSystem->fnFollowInstruction( ) )
			break ;

		tcout << _T("\n\n") ;
	}
	return 0;
}