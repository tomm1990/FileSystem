#include "stdafx.h"
#include "FileHandlingServices.h"
#include <Shlwapi.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define STR_STRING_TRIMMING_CHARACTERS		_T("\n\t \r")
#define IDP_CORRUPT_FILE	_T("File '%s' is corrupetd.")
#define IDP_ERROR_FILE_NOT_FOUND _T("File '%s' not found")
#define IDP_EXCEED_NO_OF_OPEN_INSTANCES _T("The permitted number of open files was exceeded \n So can not open the file \n '%s'")
#define IDP_FILE_CANNOT_BE_ACCESSED _T("The file '%s' could not be accessed.")
#define IDP_FILE_SHARING_VIOLATION _T("Sharing violation for file '%s'. \nSo close the file for proper working.")
#define IDP_FILE_LOCK_VIOLATION _T("Lock violation for file '%s'.")
#define IDP_DISK_FULL           _T("The disk is full. \nSo can not open file '%s'.")
#define IDP_CANNOT_OPEN_FILE    _T("Can not open file '%s'.")
#define IDS_LOW_MEMORY_STRING   _T("System is on low memory.")
#define IDP_ERROR_INVALID_FILE_STRING _T("Invalid file '%s'.")
#define IDP_CAN_NOT_SEEK_FILE_STRING _T("Can not seek file '%s'.")
#define IDP_END_OF_FILE_STRING  _T("End of file '%s'.")
#define IDS_INVALID_PARAMETER_STRING _T("Invalid parameter.")
#define IDP_COULD_NOT_DELETE_FILE_STRING _T("Unable to delete file '%s'. (%s)")
#define IDP_COULD_NOT_COPY_FILE_STRING _T("Unable to copy file from '%s' to '%s'. (%s)")


FileHandlingServices FileHandlingServices::cl_SingleInstance ;
//------------------------------------------------------------------------------------
//
//
FileHandlingServices& FileHandlingServices::fnGetInstance()
{
	return cl_SingleInstance ;
}

//------------------------------------------------------------------------------------
//Getting Module(application) path
//
CString FileHandlingServices::fnGetAppDir()
{
	TCHAR pAppPathName[MAX_PATH+1]; // CAUTION: UNICODE VERSION PATH CAN BE > MAX_PATH
	DWORD dwSize = MAX_PATH+1;
	if( !GetModuleFileName(NULL, pAppPathName, dwSize) != 0)
	{
		ASSERT(FALSE ) ;
	}

	//We need only parent directory of exe file
	CString clCSModulePath  = pAppPathName ;
	int iFoundSlash = clCSModulePath.ReverseFind( _T('\\') ) ;

	if( iFoundSlash > -1 )
	{
		iFoundSlash++ ;
		clCSModulePath.Delete( iFoundSlash , clCSModulePath.GetLength() - iFoundSlash ) ;
	}
	return clCSModulePath ;
}


//------------------------------------------------------------------------------------
//For creating folder at specified location
//
bool FileHandlingServices::fnCreateFolder( CString claCSFolderPath )
{
	if( claCSFolderPath.IsEmpty() )
		return false ;

	if (PathFileExists( claCSFolderPath ) == FALSE)//checking if input path is existed or not
	{
		//If not existed then
		// create the path.
		TCHAR * pPathNew = _tcsdup(claCSFolderPath);
		
		if ( claCSFolderPath.ReverseFind ( '.' ) != -1 )
		{
			TCHAR* pPathNewOriginal = pPathNew ;
			PathRemoveFileSpec(pPathNew);		// Means the path contains file name also hence remove file name

			if ( _tcslen ( pPathNew ) < 1 )
			{
				free(pPathNewOriginal);
				return true ;
			}
		}


		//check if this is a network drive then ignore first '\\'
		LPTSTR pComp = pPathNew;
		if ( pComp[0] == '\\' && pComp[1] == '\\' ) 
			pComp = PathFindNextComponent(pComp) ;

		while((pComp = PathFindNextComponent(pComp))[0] != 0)
		{
			LPTSTR pCompNext = PathFindNextComponent(pComp);
			TCHAR cBackup = pCompNext[0];
			pCompNext[0] = 0;

			CString dir = pPathNew;

			if (PathFileExists(pPathNew) == FALSE)
			{
				if (CreateDirectory(pPathNew, NULL) == FALSE)//creating directory
				{
					free(pPathNew);
					return false ;
				}
			}
			pCompNext[0] = cBackup;
		}

		free(pPathNew);
	}

	return true ;
}

//------------------------------------------------------------------------------------
//For creating file at specified location
//
bool FileHandlingServices::fnCreateFile( CString claCSFile )
{
	//checking if parent directory is existed for specified file 
	//If not then creating parent directory
	if( fnCreateFolder ( claCSFile ) )
	{
		//Creating file at specified location
		CFile clFile ;
		if( !clFile.Open( claCSFile , CFile::modeCreate | CFile::modeWrite ) )
			return false ;
		
		clFile.Close() ;
		return true ;
	}
	return false ;
}

//------------------------------------------------------------------------------------
//Deleting specified directory path
//
bool FileHandlingServices::fnDeleteFolder( const CString& nclraFolderPath , CString& clraErrMsg )
{
	bool	bSubdirectory = false;       // Flag, indicating whether subdirectories have been found
	CString     clCSFilePath;            // Filepath
	
	WIN32_FIND_DATA FileInformation;             // File information
	CString clCSPatternStr = nclraFolderPath + _T("\\*.*");//creating pattern for getting all file and folders that are in specified folderpath
	HANDLE hFile = ::FindFirstFile(clCSPatternStr, &FileInformation);
	if(hFile != INVALID_HANDLE_VALUE)
	{
		do
		{
			if(FileInformation.cFileName[0] != _T('.'))//not parent directory
			{
				clCSFilePath.Empty() ;
				clCSFilePath = nclraFolderPath + _T("\\") + FileInformation.cFileName;

				if(FileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)//FOund name is of direcotry
				{
					//recursive call to Delete subdirectory
					if( !fnDeleteFolder(clCSFilePath, clraErrMsg) )
						return false;
				}
				else//Found name is of file
				{
					// Set file attributes
					if(::SetFileAttributes(clCSFilePath,FILE_ATTRIBUTE_NORMAL) == FALSE)
					{
						clraErrMsg = fnGetStringErrAsPerCode ( GetLastError(), clCSFilePath ) ;
						return false;
					}

					// Delete file
					if(::DeleteFile(clCSFilePath) == FALSE)
					{
						clraErrMsg = fnGetStringErrAsPerCode ( GetLastError(), clCSFilePath ) ;
						return false;
					}
				}
			}
		} while(::FindNextFile(hFile, &FileInformation) == TRUE);

		// Close handle
		::FindClose(hFile);

		DWORD dwError = ::GetLastError();
		if(dwError != ERROR_NO_MORE_FILES)
		{
			clraErrMsg = fnGetStringErrAsPerCode ( dwError, clCSFilePath ) ;
			return false;
		}
		else//Deleting current folder
		{
			// Set directory attributes
			if(::SetFileAttributes(nclraFolderPath,FILE_ATTRIBUTE_NORMAL) == FALSE)
			{
				clraErrMsg = fnGetStringErrAsPerCode ( ::GetLastError(), clCSFilePath ) ;
				return false;
			}

			// Delete directory
			if(::RemoveDirectory(nclraFolderPath) == FALSE)
			{
				clraErrMsg = fnGetStringErrAsPerCode ( ::GetLastError(), clCSFilePath ) ;
				return false;
			}
		}
	}

	return true;
}

//------------------------------------------------------------------------------------
// Deleting specified file 
//
bool FileHandlingServices::fnDeleteFile(CString claCSFileToDelete, CString * clpaErrString )
{
	if(claCSFileToDelete.IsEmpty())
	{
		*clpaErrString = IDS_INVALID_PARAMETER_STRING ;
		return false;
	}

	ASSERT( PathFileExists(claCSFileToDelete) ) ;

	//Deleting file
	if( DeleteFile(claCSFileToDelete) )
		return true;

	DWORD dwLastErr = GetLastError();

	if ( ( 2 == dwLastErr ) || ( 3 == dwLastErr )  )// system was unable to find the file OR system cannot find the path specified
		return true;

	// If we were unable to delete the file hence check its attribute
	// If file is readonly then we remove read only parameter
	bool bFileReadOnly = false ;;
	if(fnGetReadOnlyAttribute(claCSFileToDelete, bFileReadOnly , clpaErrString ) && bFileReadOnly)
	{
		if(fnSetReadOnlyAttribute(claCSFileToDelete, false , clpaErrString ))
		{
			if(DeleteFile(claCSFileToDelete))
				return true;
			dwLastErr = GetLastError();
		}
	}

	if(clpaErrString)
	{
		CString clCSWinError ;
		clCSWinError.Empty() ;
		clCSWinError = fnGetStringErrAsPerCode ( dwLastErr , claCSFileToDelete );
		clpaErrString->Format( IDP_COULD_NOT_DELETE_FILE_STRING , claCSFileToDelete , clCSWinError );
	}
	
	return false;
}

//------------------------------------------------------------------------------------
//Copying file 
//
bool FileHandlingServices::fnCopyFile( CString claCSExistingFilePath , CString claCSFilePathToCopyTo, CString * clpaErrString /*= NULL*/)
{
	bool bRetVal = false;
	DWORD dwLastError = 0;
	if( fnCreateFolder( claCSFilePathToCopyTo ) )//if destination path is not exist then create path
	{
		//copy source file
		bRetVal = ( CopyFile ( claCSExistingFilePath , claCSFilePathToCopyTo , FALSE ) > 0)?true:false; 
		dwLastError = GetLastError();
		
		if ( !bRetVal )
		{
			//if destination file is having readonly attribute then we first remove readonly then copy content of source file to destination
			bool bIsReadOnly = false ;
			if(fnGetReadOnlyAttribute(claCSFilePathToCopyTo, bIsReadOnly , clpaErrString) && bIsReadOnly)
			{
				VERIFY(fnSetReadOnlyAttribute(claCSFilePathToCopyTo, false, clpaErrString));
				bRetVal = ( CopyFile ( claCSExistingFilePath , claCSFilePathToCopyTo, FALSE ) )?true:false; 
				dwLastError = GetLastError();
				VERIFY(fnSetReadOnlyAttribute( claCSFilePathToCopyTo , true , clpaErrString ) );//reset attribute
				if ( !bRetVal )
				{
					CString clCSWinError = fnGetStringErrAsPerCode( dwLastError , claCSExistingFilePath );
					clpaErrString->Format( IDP_COULD_NOT_COPY_FILE_STRING , claCSExistingFilePath, claCSFilePathToCopyTo , clCSWinError );
				}
			}
		}
	}

	return bRetVal ;
}


//------------------------------------------------------------------------------------
//Copy folder
//
bool FileHandlingServices::fnCopyFolder( CString claCSExistingFolderPath , CString claCSFolderPathToMoveTo, CString * clpaErrString )
{
	// create the dest folder
	if( !fnCreateFolder( claCSFolderPathToMoveTo ) )
	{
		return false;
	}
	
	// first get current path
	TCHAR cpOldPath[MAX_PATH];
	::GetCurrentDirectory(MAX_PATH, cpOldPath);

	//set current directory as specified source path
	if(!::SetCurrentDirectory (claCSExistingFolderPath))
	{
		DWORD dwLastError = GetLastError();
		*clpaErrString = fnGetStringErrAsPerCode ( dwLastError , claCSExistingFolderPath );
		return false;
	}

	//Now copying all the resource from source to destination
	WIN32_FIND_DATA fd;
	HANDLE hFind = ::FindFirstFile (_T("*"), &fd);

	if (hFind != INVALID_HANDLE_VALUE) 
	{
		do 
		{
			if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				CString clFolderName = fd.cFileName;
				// reject "." and ".." as they are not useful here
				if((clFolderName != _T(".")) && (clFolderName != _T("..")))
				{
					// its directory to copy
					CString clNewSourcePath, clNewDestPath;
					clNewSourcePath = claCSExistingFolderPath + _T("\\") + fd.cFileName;
					clNewDestPath   = claCSFolderPathToMoveTo + _T("\\") + fd.cFileName;
					if(claCSFolderPathToMoveTo.Compare(clNewSourcePath) != 0)
					{
						// here only call if the new dest folder is not same as the source folder
						if(!fnCopyFolder(clNewSourcePath, clNewDestPath, clpaErrString))
						{
							// error is already set
							// restore the old path
							::SetCurrentDirectory (cpOldPath);
							return false;
						}
						// also restore the current path
						// it is not necessary as it has been already reset by the function
						if(!::SetCurrentDirectory (claCSExistingFolderPath))
						{
							DWORD dwLastError = GetLastError();
							*clpaErrString = fnGetStringErrAsPerCode ( dwLastError , claCSExistingFolderPath );
							return false;
						}
					}
				}
			}
			else
			{
				// its file
				// just copy the file
				CString clNewSourcePath, clNewDestPath;
				clNewSourcePath = claCSExistingFolderPath + _T("\\") + fd.cFileName;
				clNewDestPath   = claCSFolderPathToMoveTo + _T("\\") + fd.cFileName;
				if(!fnCopyFile(clNewSourcePath, clNewDestPath, clpaErrString))
				{
					// restore the old path
					::SetCurrentDirectory (cpOldPath);
					return false;
				}
			}
		} while (::FindNextFile (hFind, &fd));
		::FindClose (hFind);
	}
	// restore the old path
	::SetCurrentDirectory (cpOldPath);
	return true;
}

//------------------------------------------------------------------------------------
//search all file names in specified directory
//
bool FileHandlingServices::fnSearchDirectory(const CString& nclarCSDirectory , CStringArray* clpaCSFilePathArray )
{
	CString clCSFilePath; // Filepath
	
	WIN32_FIND_DATA FileInformation; // File information

	CString clCSPattern; // Pattern for finding all files that are in specified directory
	clCSPattern.Format ( _T("%s\\*.*") , nclarCSDirectory ) ;

	HANDLE hFile = ::FindFirstFile(clCSPattern, &FileInformation);
	if(hFile != INVALID_HANDLE_VALUE)
	{
		do
		{
			if(FileInformation.cFileName[0] != _T('.') )
			{
				clCSFilePath.Empty() ;
				clCSFilePath.Format ( _T("%s\\%s") , nclarCSDirectory , FileInformation.cFileName );

				if(FileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)//Directory
				{
					// Search subdirectory
					if( !fnSearchDirectory( clCSFilePath, clpaCSFilePathArray ) )
						return false ;
				}
				else//File
				{
					// Save filename
					clpaCSFilePathArray->Add( clCSFilePath ) ;
				}
			}
		} while(::FindNextFile(hFile, &FileInformation) == TRUE );

		// Close handle
		::FindClose(hFile);
			
		DWORD dwError = ::GetLastError();
		if(dwError != ERROR_NO_MORE_FILES)
			return false;
	}

	return true;
}

//------------------------------------------------------------------------------------
//making error message in string from error code
//
CString FileHandlingServices::fnGetStringErrAsPerCode( DWORD dwaLastError , const CString& nclarCSFilePath)
{
	CString clCSErrMsg = fnFormatedFileReadWriteMSError ( nclarCSFilePath , dwaLastError ) ;
	if ( clCSErrMsg.IsEmpty() )//If error message is empty
	{
		CString clCSRetStr ;

		LPVOID lpMsgBuf;
		FormatMessage( 
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			dwaLastError,
			MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), // Default language
			(LPTSTR)&lpMsgBuf,
			0,
			NULL );

		clCSRetStr = CString( (TCHAR*)lpMsgBuf );
		clCSRetStr.TrimRight( STR_STRING_TRIMMING_CHARACTERS ) ;

		// Free the buffer.
		LocalFree( lpMsgBuf );

		clCSErrMsg = clCSRetStr ;
	}

	return clCSErrMsg;
}

//------------------------------------------------------------------------------------
//Checking file is readonly or not
//
bool FileHandlingServices::fnGetReadOnlyAttribute(CString claCSFilePath, bool & barIsReadOnly , CString * clpaErrString )
{
	if( !PathFileExists(claCSFilePath))//File path not found
	{
		*clpaErrString = fnGetStringErrAsPerCode ( ERROR_FILE_NOT_FOUND , claCSFilePath );
		return false;
	}

	//Get file attribute
	DWORD dwOrgAttribute = GetFileAttributes ( claCSFilePath ) ;

	if ( 0xFFFFFFFF == dwOrgAttribute )
	{
		DWORD dwLastError = GetLastError();
		*clpaErrString = fnGetStringErrAsPerCode ( dwLastError , claCSFilePath );
		return false;
	}

	barIsReadOnly = (dwOrgAttribute & FILE_ATTRIBUTE_READONLY);

	return true;
}

//------------------------------------------------------------------------------------
//Set file as read only
//
bool FileHandlingServices::fnSetReadOnlyAttribute(CString claCSFilePath, bool baSetReadOnly, CString * clpaErrString /*= NULL */)
{
	//Checking for valid path
	if( !PathFileExists(claCSFilePath))
	{
		if( clpaErrString )
			*clpaErrString = fnGetStringErrAsPerCode ( ERROR_FILE_NOT_FOUND , claCSFilePath );
		return false;
	}

	//Gettng attribute of file
	DWORD dwOrgAttribute = GetFileAttributes ( claCSFilePath ) ;

	if ( 0xFFFFFFFF == dwOrgAttribute )
	{
		DWORD dwLastError = GetLastError();
		if( clpaErrString )
			*clpaErrString = fnGetStringErrAsPerCode ( dwLastError , claCSFilePath );
		return false;
	}

	DWORD dwNewAttribute = baSetReadOnly ?	( dwOrgAttribute | FILE_ATTRIBUTE_READONLY ) :
											( dwOrgAttribute & (~FILE_ATTRIBUTE_READONLY) );

	if ( !SetFileAttributes ( claCSFilePath , dwNewAttribute ) )
	{
		DWORD dwLastError = GetLastError();
		if( clpaErrString )
			*clpaErrString = fnGetStringErrAsPerCode ( dwLastError , claCSFilePath );
		return false;
	}

	ASSERT(GetFileAttributes ( claCSFilePath ) == dwNewAttribute);

	return true;
}


//------------------------------------------------------------------------------------
CString FileHandlingServices::fnGetFileExtension( const CString& nclarCSFilePath )
{
	CString strName = nclarCSFilePath;
	const int ciDotPos = strName.ReverseFind(_T('.'));

	if (ciDotPos != -1)
	{
		strName = strName.Right(strName.GetLength() - ciDotPos - 1);
		return strName;
	}
	return _T("");
}


//----------------------------------------------------------------------------------------------------------------------------------
// This function will return error string according to error code
//
CString FileHandlingServices::fnFormatedFileReadWriteMSError ( const CString& nclarCEFilePath , DWORD dwaMSErrorCode )
{
	CString clCSReturn = _T("");
	// NT Error codes
	switch ((UINT)dwaMSErrorCode)
	{
	case NO_ERROR:
		break;
	case ERROR_FILE_NOT_FOUND: //CFileException::fileNotFound;
	case ERROR_INVALID_HANDLE:
	case ERROR_NO_MORE_FILES:
	case ERROR_DISK_CHANGE:
	case ERROR_PATH_NOT_FOUND: //CFileException::badPath; Here start as badPath
	case ERROR_INVALID_DRIVE:
	case ERROR_NOT_SAME_DEVICE:
	case ERROR_WRONG_DISK:
	case ERROR_DUP_NAME:
	case ERROR_BAD_NETPATH:
	case ERROR_DEV_NOT_EXIST:
	case ERROR_BAD_NET_NAME:
	case ERROR_SHARING_PAUSED:
	case ERROR_ALREADY_ASSIGNED:
	case ERROR_BUFFER_OVERFLOW:
	case ERROR_INVALID_NAME:
	case ERROR_INVALID_LEVEL:
	case ERROR_NO_VOLUME_LABEL:
	case ERROR_DIR_NOT_ROOT:
	case ERROR_LABEL_TOO_LONG:
	case ERROR_BAD_PATHNAME:
	case ERROR_FILENAME_EXCED_RANGE:
	case ERROR_META_EXPANSION_TOO_LONG:
	case ERROR_DIRECTORY:
		clCSReturn.Format ( IDP_ERROR_FILE_NOT_FOUND , nclarCEFilePath );
		break;
	case ERROR_TOO_MANY_OPEN_FILES://CFileException::tooManyOpenFiles;
	case ERROR_SHARING_BUFFER_EXCEEDED:
	case ERROR_TOO_MANY_NAMES:
	case ERROR_NO_MORE_SEARCH_HANDLES:
		clCSReturn.Format ( IDP_EXCEED_NO_OF_OPEN_INSTANCES , nclarCEFilePath );
		break;
	case ERROR_FILE_CORRUPT:
		clCSReturn.Format( IDP_CORRUPT_FILE, nclarCEFilePath ) ;
		break ;
	case ERROR_ACCESS_DENIED: //CFileException::accessDenied;
	case ERROR_INVALID_ACCESS:
	case ERROR_WRITE_PROTECT:
	case ERROR_WRITE_FAULT:
	case ERROR_NETWORK_BUSY:
	case ERROR_BAD_NET_RESP:
	case ERROR_NETNAME_DELETED:
	case ERROR_NETWORK_ACCESS_DENIED:
	case ERROR_REQ_NOT_ACCEP:
	case ERROR_FILE_EXISTS:
	case ERROR_CANNOT_MAKE:
	case ERROR_INVALID_PASSWORD:
	case ERROR_BUSY:
	case ERROR_ALREADY_EXISTS:
	case ERROR_SWAPERROR:
		clCSReturn.Format ( IDP_FILE_CANNOT_BE_ACCESSED , nclarCEFilePath );
		break;
	case ERROR_BAD_FORMAT: //CFileException::invalidFile;
	case ERROR_NOT_DOS_DISK:
	case ERROR_BAD_REM_ADAP:
	case ERROR_BAD_DEV_TYPE:
	case ERROR_INVALID_TARGET_HANDLE:
	case ERROR_INVALID_ORDINAL:
	case ERROR_INVALID_EXE_SIGNATURE:
	case ERROR_BAD_EXE_FORMAT:
		clCSReturn.Format ( IDP_ERROR_INVALID_FILE_STRING , nclarCEFilePath );
		break;
	case ERROR_CURRENT_DIRECTORY: //CFileException::removeCurrentDir;		
	case ERROR_DIR_NOT_EMPTY:
		clCSReturn.Format ( IDP_ERROR_FILE_NOT_FOUND , nclarCEFilePath );
		break;
	case ERROR_BAD_UNIT: //CFileException::hardIO;
	case ERROR_NOT_READY:
	case ERROR_BAD_COMMAND:
	case ERROR_CRC:
	case ERROR_ADAP_HDW_ERR:
	case ERROR_UNEXP_NET_ERR:
	case ERROR_NET_WRITE_FAULT:
	case ERROR_INVALID_CATEGORY:
	case ERROR_OPERATION_ABORTED:
	case ERROR_IO_INCOMPLETE:
	case ERROR_IO_PENDING:
		clCSReturn.Format ( IDP_CANNOT_OPEN_FILE , nclarCEFilePath );
		break;
	case ERROR_BAD_LENGTH: //CFileException::badSeek;		
	case ERROR_SEEK:
	case ERROR_SECTOR_NOT_FOUND:
	case ERROR_READ_FAULT:
	case ERROR_NEGATIVE_SEEK:
	case ERROR_SEEK_ON_DEVICE:
		clCSReturn.Format ( IDP_CAN_NOT_SEEK_FILE_STRING , nclarCEFilePath );
		break;
	case ERROR_SHARING_VIOLATION://CFileException::sharingViolation;
		clCSReturn.Format ( IDP_FILE_SHARING_VIOLATION , nclarCEFilePath );
		break;
	case ERROR_LOCK_VIOLATION://CFileException::lockViolation;		
	case ERROR_DRIVE_LOCKED:
	case ERROR_LOCK_FAILED:
		clCSReturn.Format ( IDP_FILE_LOCK_VIOLATION , nclarCEFilePath );
		break;
	case ERROR_HANDLE_EOF: //CFileException::endOfFile;
		clCSReturn.Format ( IDP_END_OF_FILE_STRING , nclarCEFilePath );
		break;
	case ERROR_HANDLE_DISK_FULL: //CFileException::diskFull;
	case ERROR_DISK_FULL:
		clCSReturn.Format ( IDP_DISK_FULL , nclarCEFilePath );
		break;
	case ERROR_OUTOFMEMORY:
		clCSReturn= IDS_LOW_MEMORY_STRING;
		break;
	}

	return clCSReturn;
}
