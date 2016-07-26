#if !defined(AFX_FileHandlingServices_H__22277677_AF00_470C_A191_3DCCE6C77D95__INCLUDED_)
#define AFX_FileHandlingServices_H__22277677_AF00_470C_A191_3DCCE6C77D95__INCLUDED_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//Singlton class
class FileHandlingServices  
{
	static FileHandlingServices cl_SingleInstance ;//Singlton object

	FileHandlingServices(){};
	~FileHandlingServices(){};
	
	// Stop the compiler generating methods of copy the object
	FileHandlingServices(FileHandlingServices const& copy);            // Not Implemented
	FileHandlingServices& operator=(FileHandlingServices const& copy); // Not Implemented


	CString fnFormatedFileReadWriteMSError ( const CString& nclarCEFilePath , DWORD dwaMSErrorCode );
public:
	static FileHandlingServices& fnGetInstance() ;
	CString fnGetAppDir() ;
	
	//Creating file/folder
	bool fnCreateFolder ( CString claCSFolderPath ) ;
	bool fnCreateFile( CString claCSFile ) ;

	//deleting file/folder
	bool fnDeleteFolder( const CString& nclraFolderPath , CString& clraErrMsg ) ;
	bool fnDeleteFile(CString claFileToDelete, CString * clpaErrString );	
	
	//Copying file/folder
	bool fnCopyFile( CString claCSExistingFilePath , CString claCSFilePathToCopyTo, CString * clpaErrString);
	bool fnCopyFolder( CString claCSExistingFolderPath , CString claCSFolderPathToMoveTo, CString * clpaErrString);
	
	//Search all the files in specified directory
	bool fnSearchDirectory(const CString& nclarCSDirectory , CStringArray* clpaCSFilePathArray ) ;

	//Get string from error code
	CString fnGetStringErrAsPerCode( DWORD dwaLastErr, const CString& clraFileName ) ;

	//Getting and setting file read only attribute
	bool fnGetReadOnlyAttribute(CString claCSFilePath, bool & barIsReadOnly , CString * clpaErrString ) ;
	bool fnSetReadOnlyAttribute(CString claCSFilePath, bool baSetReadOnly, CString * clpaErrString = NULL ) ;
	
	CString fnGetFileExtension( const CString& nclarCSFilePath );
};

#endif // !defined(AFX_FileHandlingServices_H__22277677_AF00_470C_A191_3DCCE6C77D95__INCLUDED_)