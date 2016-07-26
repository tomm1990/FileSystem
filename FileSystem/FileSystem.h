#if !defined(AFX_FileSystem_H__22277677_AF00_470C_A191_3DCCE6C77D95__INCLUDED_)
#define AFX_FileSystem_H__22277677_AF00_470C_A191_3DCCE6C77D95__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//Singlton class
class CFileSystem
{
	static CFileSystem cl_FileSystem ;//Singlton object
	int		i_Option ;//Option for instruction
	
	CFileSystem();
	~CFileSystem();

	// Stop the compiler generating methods of copy the object
	CFileSystem(CFileSystem const& copy);
	CFileSystem& operator=(CFileSystem const& copy);

	void fnViewContentOfFileInConsole( const CString& nclarCSFilePath ) ;
public:
	static CFileSystem& fnGetInstance() ;

	//Setting /Getting option
	void fnSetOption( int iaOption ) { i_Option = iaOption ; } ;
	int fnGetOption( )const {return i_Option ;} ;

	void fnPrintInstruction();
	int fnFollowInstruction() ;

	CString fnGetFullPathForRelativePath( const CString& nclarCSName ) ;
	CString fnGetDataFromConsole( bool baIgnore = true ) ;

	//Various option handling functions
	void fnCreateLibraryName() ;
	void fnCreateFileName() ;
	void fnDuplicateLibraryOrFile() ;
	void fnDeleteLibraryOrFile() ;
	void fnViewContentOfLibraryOrFile() ;
	void fnPrintLibraryOrFile() ;
};


#endif // !defined(AFX_FileSystem_H__22277677_AF00_470C_A191_3DCCE6C77D95__INCLUDED_)

