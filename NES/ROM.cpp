//////////////////////////////////////////////////////////////////////////
//                                                                      //
//      NES ROM Cartridge class                                         //
//                                                           Norix      //
//                                               written     2001/02/20 //
//                                               last modify ----/--/-- //
//////////////////////////////////////////////////////////////////////////
#define	WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <mbstring.h>

#include "typedef.h"
#include "macro.h"

#include "VirtuaNESres.h"

#include "DebugOut.h"
#include "App.h"
#include "Plugin.h"
#include "Pathlib.h"
#include "Crclib.h"

#include "rom.h"
#include "mmu.h"

ROMDATABASE	romdatabase;

//
// Original:NesterJ arc.cpp arc.h by Mikami Kana
//
#include <time.h>

#pragma	pack(1) // 1바이트 단위로 정렬한다
#define	FNAME_MAX32	512

typedef struct {
	DWORD 	dwOriginalSize;
 	DWORD 	dwCompressedSize;
	DWORD	dwCRC;
	UINT	uFlag;
	UINT	uOSType;
	WORD	wRatio;
	WORD	wDate;
	WORD 	wTime;
	char	szFileName[FNAME_MAX32 + 1];
	char	dummy1[3];
	char	szAttribute[8];
	char	szMode[8];
} INDIVIDUALINFO, *LPINDIVIDUALINFO;
#pragma pack()

// Un??? use function
typedef	int(WINAPI *EXECUTECOMMAND)(HWND,LPCSTR,LPSTR,const DWORD);
typedef	BOOL(WINAPI *CHECKARCHIVE)(LPCSTR,const int);
typedef	int(WINAPI *EXTRACTMEM)(HWND,LPCSTR,LPBYTE,const DWORD,time_t,LPWORD,LPDWORD);
typedef	HGLOBAL(WINAPI *OPENARCHIVE)(HWND,LPCSTR,const DWORD);
typedef	int(WINAPI *CLOSEARCHIVE)(HGLOBAL);
typedef	int(WINAPI *FINDFIRST)(HGLOBAL,LPCSTR,INDIVIDUALINFO*);

static	LPCSTR	pszArchiver[] = {
	"UNLHA32",
	"UNZIP32",
	"UNRAR32",
	"CAB32",
	NULL
};

static	LPCSTR	pszFuncPrefix[] = {
	"Unlha",
	"UnZip",
	"Unrar",
	"Cab",
};

static	LPCSTR	pszCommand[] = {
	NULL,
	NULL,
	"-e -u \"%s\" \"%s\" \"%s\"",
	"-x -j \"%s\" \"%s\" \"%s\"",
};

static	LPCSTR	pszExtension[] = {
	"*.nes",
	"*.fds",
	"*.nsf",
	NULL
};

static	BOOL	bFileMatching[] = {
	FALSE,
	TRUE,
	FALSE,
	FALSE,
};

#define	FREEDLL(h)	if( h ) { FreeLibrary(h);h=NULL; }

#define M_ERROR_MESSAGE_OFF		0x00800000L

BOOL	Uncompress( LPCSTR fname, LPBYTE* ppBuf, LPDWORD lpdwSize )
{
HMODULE		hDLL;
INDIVIDUALINFO	idvinfo;

	hDLL = NULL;
	for( INT i = 0; pszArchiver[i]; i++ ) {
		// DLLアンロード
		FREEDLL( hDLL );

		// DLLロード
		if( !(hDLL = LoadLibrary( pszArchiver[i] )) )
			continue;

		CHAR	szTemp[256];
		sprintf( szTemp, "%sCheckArchive", pszFuncPrefix[i] );
		CHECKARCHIVE	CheckArchive;
		if( !(CheckArchive = (CHECKARCHIVE)GetProcAddress( hDLL, szTemp )) )
			continue;
		// 対応するアーカイブかチェックする
		if( !CheckArchive( fname, 1 ) )
			continue;

		// アーカイブ内に対応するファイルがあるかのチェック
		OPENARCHIVE	OpenArchive;
		CLOSEARCHIVE	CloseArchive;
		FINDFIRST	FindFirst;

		sprintf( szTemp, "%sOpenArchive", pszFuncPrefix[i] );
		OpenArchive = (OPENARCHIVE)GetProcAddress( hDLL, szTemp );
		sprintf( szTemp, "%sFindFirst", pszFuncPrefix[i] );
		FindFirst = (FINDFIRST)GetProcAddress( hDLL, szTemp );
		sprintf( szTemp, "%sCloseArchive", pszFuncPrefix[i] );
		CloseArchive = (CLOSEARCHIVE)GetProcAddress( hDLL, szTemp );

		HGLOBAL		hARC;
		BOOL	bFound = FALSE;
		for( INT j = 0; pszExtension[j]; j++ ) {
			if( !(hARC = OpenArchive( NULL, fname, M_ERROR_MESSAGE_OFF ) ) ) {
				CloseArchive( hARC );
				break;
			}
			INT	ret = FindFirst( hARC, pszExtension[j], &idvinfo );
			CloseArchive( hARC );
			if( ret == 0 ) {		// Found!!
				bFound = TRUE;
				break;
			} else if( ret == -1 ) {	// Not found.
			} else {			// 異常終了
				break;
			}
		}
		if( !bFound )
			continue;

		if( !pszCommand[i] ) {
		// メモリ解凍あり(UNLHA32,UNZIP32)
			*lpdwSize = idvinfo.dwOriginalSize;
			*ppBuf = (LPBYTE)malloc( *lpdwSize );

			CHAR	szCmd [256];
			CHAR	szFunc[256];

			if( !bFileMatching[i] ) {
				sprintf( szCmd, "\"%s\" \"%s\"", fname, idvinfo.szFileName );
			} else {
			// UNZIP32 only
				BYTE	szFile[FNAME_MAX32+1];
				LPBYTE	lpF0, lpF1;

				// 正規表現を切るオプションが欲しかった....
				lpF0 = (LPBYTE)idvinfo.szFileName;
				lpF1 = szFile;
				while( *lpF0 ) {
					if( *lpF0 == '[' || *lpF0 == ']' ) {
						*lpF1++ = '\\';
					}
					_mbsncpy( lpF1, lpF0, 1 );
					lpF0 = _mbsinc( lpF0 );
					lpF1 = _mbsinc( lpF1 );
				}
				*lpF1 = '\0';

				sprintf( szCmd, "\"%s\" \"%s\"", fname, szFile );
			}
			sprintf( szFunc, "%sExtractMem", pszFuncPrefix[i] );

			EXTRACTMEM	ExtractMem;
			ExtractMem = (EXTRACTMEM)GetProcAddress( hDLL, szFunc );
			INT ret = ExtractMem( NULL, szCmd, (LPBYTE)(*ppBuf), *lpdwSize, NULL, NULL, NULL );
			FREEDLL( hDLL );
			if( ret == 0 )
				return TRUE;
		} else {
		// メモリ解凍が無い場合
			CHAR	szCmd [256];
			CHAR	szTempPath[_MAX_PATH];
			EXECUTECOMMAND	ExecuteCommand;

			GetTempPath( _MAX_PATH, szTempPath );
//DEBUGOUT( "TempPath:%s\n", szTempPath );

			sprintf( szCmd, pszCommand[i], fname, szTempPath, idvinfo.szFileName );
			ExecuteCommand = (EXECUTECOMMAND)GetProcAddress( hDLL, pszFuncPrefix[i] );
			ExecuteCommand( NULL, szCmd, NULL, 0 );
			FREEDLL( hDLL );

			string	FileName = CPathlib::MakePath( szTempPath, idvinfo.szFileName );

			FILE *fp = NULL;
			if( (fp = fopen( FileName.c_str(), "rb" )) ) {
				// ファイルサイズ取得
				fseek( fp, 0, SEEK_END );
				*lpdwSize = ftell( fp );
				fseek( fp, 0, SEEK_SET );
				if( *lpdwSize < 17 ) {
					// ファイルサイズが小さすぎます
					throw	CApp::GetErrorString( IDS_ERROR_SMALLFILE );
				}

				// テンポラリメモリ確保
				if( !(*ppBuf = (LPBYTE)malloc( *lpdwSize )) ) {
					FCLOSE( fp );
					// メモリを確保出来ません
					throw	CApp::GetErrorString( IDS_ERROR_OUTOFMEMORY );
				}
				// サイズ分読み込み
				if( fread( *ppBuf, *lpdwSize, 1, fp ) != 1 ) {
					FCLOSE( fp );
					FREE( *ppBuf );
					// ファイルの読み込みに失敗しました
					throw	CApp::GetErrorString( IDS_ERROR_READ );
				}
				FCLOSE( fp );
				DeleteFile( FileName.c_str() );
			} else {
				// xxx ファイルを開けません
				LPCSTR	szErrStr = CApp::GetErrorString( IDS_ERROR_OPEN );
				sprintf( szErrorString, szErrStr, fname );
				throw	szErrorString;
			}
			return	TRUE;
		}
	}
	FREEDLL( hDLL );

	return	FALSE;
}
// Archive

//
// コンストラクタ
//
ROM::ROM( const char* fname )
{
FILE	*fp = NULL;
LPBYTE	temp = NULL;
LPBYTE	bios = NULL;
LONG	FileSize;

	ZeroMemory( &header, sizeof(header) );
	ZeroMemory( path, sizeof(path) );
	ZeroMemory( name, sizeof(name) );

	bNSF = FALSE;
	NSF_PAGE_SIZE = 0;

	lpPRG = lpCHR = lpTrainer = lpDiskBios = lpDisk = NULL;

	crc = crcall = 0;
	mapper = 0;
	diskno = 0;

#ifdef	_DATATRACE
	// For dis...
	PROM_ACCESS = NULL;
#endif
	try {
		if( !(fp = ::fopen( fname, "rb" )) ) {
			// xxx ファイルを開けません
			LPCSTR	szErrStr = CApp::GetErrorString( IDS_ERROR_OPEN );
			::wsprintf( szErrorString, szErrStr, fname );
			throw	szErrorString;
		}

		// ファイルサイズ取得
		::fseek( fp, 0, SEEK_END );
		FileSize = ::ftell( fp );
		::fseek( fp, 0, SEEK_SET );
		// ファイルサイズチェック(NESヘッダ+1バイト以上か？)
		if( FileSize < 17 ) {
			// ファイルサイズが小さすぎます
			throw	CApp::GetErrorString( IDS_ERROR_SMALLFILE );
		}

		// テンポラリメモリ確保
		if( !(temp = (LPBYTE)::malloc( FileSize )) ) {
			// メモリを確保出来ません
			throw	CApp::GetErrorString( IDS_ERROR_OUTOFMEMORY );
		}

		// サイズ分読み込み
		if( ::fread( temp, FileSize, 1, fp ) != 1 ) {
			// ファイルの読み込みに失敗しました
			throw	CApp::GetErrorString( IDS_ERROR_READ );
		}

		FCLOSE( fp );

		// ヘッダコピー
		::memcpy( &header, temp, sizeof(NESHEADER) );

		if( header.ID[0] == 'N' && header.ID[1] == 'E'
		 && header.ID[2] == 'S' && header.ID[3] == 0x1A ) {
			// ヘッダコピー
			memcpy( &header, temp, sizeof(NESHEADER) );
		} else if( header.ID[0] == 'F' && header.ID[1] == 'D'
			&& header.ID[2] == 'S' && header.ID[3] == 0x1A ) {
			// ヘッダコピー
			memcpy( &header, temp, sizeof(NESHEADER) );
		} else if( header.ID[0] == 'N' && header.ID[1] == 'E'
			&& header.ID[2] == 'S' && header.ID[3] == 'M') {
			// ヘッダコピー
			memcpy( &header, temp, sizeof(NESHEADER) );
		} else {
			FREE( temp );

			if( !Uncompress( fname, &temp, (LPDWORD)&FileSize ) ) {
				// 未対応形式です
				throw	CApp::GetErrorString( IDS_ERROR_UNSUPPORTFORMAT );
			}
			// ヘッダコピー
			memcpy( &header, temp, sizeof(NESHEADER) );
		}

		DWORD	PRGoffset, CHRoffset;
		LONG	PRGsize, CHRsize;

		if( header.ID[0] == 'N' && header.ID[1] == 'E'
		 && header.ID[2] == 'S' && header.ID[3] == 0x1A ) {
		// 普通のNESファイル
			PRGsize = (LONG)header.PRG_PAGE_SIZE*0x4000;
			CHRsize = (LONG)header.CHR_PAGE_SIZE*0x2000;
			PRGoffset = sizeof(NESHEADER);
			CHRoffset = PRGoffset + PRGsize;

			if( IsTRAINER() ) {
				PRGoffset += 512;
				CHRoffset += 512;
			}

			if( PRGsize <= 0 ) {
				// NESヘッダが異常です
				throw	CApp::GetErrorString( IDS_ERROR_INVALIDNESHEADER );
			}

			// PRG BANK
			if( !(lpPRG = (LPBYTE)malloc( PRGsize )) ) {
				// メモリを確保出来ません
				throw	CApp::GetErrorString( IDS_ERROR_OUTOFMEMORY );
			}

			memcpy( lpPRG, temp+PRGoffset, PRGsize );

			// CHR BANK
			if( CHRsize > 0 ) {
				if( !(lpCHR = (LPBYTE)malloc( CHRsize )) ) {
					// メモリを確保出来ません
					throw	CApp::GetErrorString( IDS_ERROR_OUTOFMEMORY );
				}

				if( FileSize >= CHRoffset+CHRsize ) {
					memcpy( lpCHR, temp+CHRoffset, CHRsize );
				} else {
					// CHRバンク少ない…
					CHRsize	-= (CHRoffset+CHRsize - FileSize);
					memcpy( lpCHR, temp+CHRoffset, CHRsize );
				}
			} else {
				lpCHR = NULL;
			}

			// Trainer
			if( IsTRAINER() ) {
				if( !(lpTrainer = (LPBYTE)malloc( 512 )) ) {
					// メモリを確保出来ません
					throw	CApp::GetErrorString( IDS_ERROR_OUTOFMEMORY );
				}

				memcpy( lpTrainer, temp+sizeof(NESHEADER), 512 );
			} else {
				lpTrainer = NULL;
			}

			// For dis....
#ifdef	_DATATRACE
			if( !(PROM_ACCESS = (LPBYTE)malloc( PRGsize )) ) {
				// メモリを確保出来ません
				throw	CApp::GetErrorString( IDS_ERROR_OUTOFMEMORY );
			}
			ZeroMemory( PROM_ACCESS, PRGsize );
#endif
		} else if( header.ID[0] == 'F' && header.ID[1] == 'D'
			&& header.ID[2] == 'S' && header.ID[3] == 0x1A ) {
		// FDS(Nintendo Disk System)
			// ディスクサイズ
			diskno = header.PRG_PAGE_SIZE;

			if( FileSize < (16+65500*diskno) ) {
				// ディスクサイズが異常です
				throw	CApp::GetErrorString( IDS_ERROR_ILLEGALDISKSIZE );
			}
			if( diskno > 4 ) {
				// 4面より多いディスクは対応していません
				throw	CApp::GetErrorString( IDS_ERROR_UNSUPPORTDISK );
			}

			ZeroMemory( &header, sizeof(NESHEADER) );

			// ダミーヘッダを作る
			header.ID[0] = 'N';
			header.ID[1] = 'E';
			header.ID[2] = 'S';
			header.ID[3] = 0x1A;
			header.PRG_PAGE_SIZE = (BYTE)diskno*4;
			header.CHR_PAGE_SIZE = 0;
			header.control1 = 0x40;
			header.control2 = 0x10;

			PRGsize = sizeof(NESHEADER)+65500*(LONG)diskno;
			// PRG BANK
			if( !(lpPRG = (LPBYTE)malloc( PRGsize )) ) {
				// メモリを確保出来ません
				throw	CApp::GetErrorString( IDS_ERROR_OUTOFMEMORY );
			}
			// データのバックアップ用
			if( !(lpDisk = (LPBYTE)malloc( PRGsize )) ) {
				// メモリを確保出来ません
				throw	CApp::GetErrorString( IDS_ERROR_OUTOFMEMORY );
			}
			// CHR BANK
			lpCHR = NULL;

			memcpy( lpPRG, &header, sizeof(NESHEADER) );
			memcpy( lpPRG+sizeof(NESHEADER), temp+sizeof(NESHEADER), PRGsize-sizeof(NESHEADER) );
			// データの書き換え場所特定用
			::ZeroMemory( lpDisk, PRGsize );
//			memcpy( lpDisk, &header, sizeof(NESHEADER) );
//			memcpy( lpDisk+sizeof(NESHEADER), temp+sizeof(NESHEADER), PRGsize-sizeof(NESHEADER) );

			lpPRG[0] = 'F';
			lpPRG[1] = 'D';
			lpPRG[2] = 'S';
			lpPRG[3] = 0x1A;
			lpPRG[4] = (BYTE)diskno;

			// DISKSYSTEM BIOSのロード
			string	Path = CPathlib::MakePathExt( CApp::GetModulePath(), "DISKSYS", "ROM" );

			if( !(fp = fopen( Path.c_str(), "rb" )) ) {
				// DISKSYS.ROMがありません
				throw	CApp::GetErrorString( IDS_ERROR_NODISKBIOS );
			}

			fseek( fp, 0, SEEK_END );
			FileSize = ftell( fp );
			fseek( fp, 0, SEEK_SET );
			if( FileSize < 17 ) {
				// ファイルサイズが小さすぎます
				throw	CApp::GetErrorString( IDS_ERROR_SMALLFILE );
			}
			if( !(bios = (LPBYTE)malloc( FileSize )) ) {
				// メモリを確保出来ません
				throw	CApp::GetErrorString( IDS_ERROR_OUTOFMEMORY );
			}
			if( fread( bios, FileSize, 1, fp ) != 1 ) {
				// ファイルの読み込みに失敗しました
				throw	CApp::GetErrorString( IDS_ERROR_READ );
			}
			FCLOSE( fp );

			if( !(lpDiskBios = (LPBYTE)malloc( 8*1024 )) ) {
				// メモリを確保出来ません
				throw	CApp::GetErrorString( IDS_ERROR_OUTOFMEMORY );
			}

			if( bios[0] == 'N' && bios[1] == 'E' && bios[2] == 'S' && bios[3] == 0x1A ) {
			// NES形式BIOS
				memcpy( lpDiskBios, bios+0x6010, 8*1024 );
			} else {
			// 生BIOS
				memcpy( lpDiskBios, bios, 8*1024 );
			}
			FREE( bios );
		} else if( header.ID[0] == 'N' && header.ID[1] == 'E'
			&& header.ID[2] == 'S' && header.ID[3] == 'M') {
		// NSF
			bNSF = TRUE;
			ZeroMemory( &header, sizeof(NESHEADER) );

			// ヘッダコピー
			memcpy( &nsfheader, temp, sizeof(NSFHEADER) );

			PRGsize = FileSize-sizeof(NSFHEADER);
			PRGsize = (PRGsize+0x0FFF)&~0x0FFF;
			if( !(lpPRG = (LPBYTE)malloc( PRGsize )) ) {
				// メモリを確保出来ません
				throw	CApp::GetErrorString( IDS_ERROR_OUTOFMEMORY );
				throw	szErrorString;
			}
			ZeroMemory( lpPRG, PRGsize );
			memcpy( lpPRG, temp+sizeof(NSFHEADER), FileSize-sizeof(NSFHEADER) );

			NSF_PAGE_SIZE = PRGsize>>12;
		} else {
			// 未対応形式です
			throw	CApp::GetErrorString( IDS_ERROR_UNSUPPORTFORMAT );
		}

		// パス/ファイル名取得
		{
		string	tempstr;
		tempstr = CPathlib::SplitPath( fname );
		::strcpy( path, tempstr.c_str() );
		tempstr = CPathlib::SplitFname( fname );
		::strcpy( name, tempstr.c_str() );
		}

		// マッパ設定
		if( !bNSF ) {
			mapper = (header.control1>>4)|(header.control2&0xF0);

			if( mapper != 20 ) {
				// PRG crcの計算(NesToyのPRG CRCと同じ)
				if( IsTRAINER() ) {
					crcall = CRC::CrcRev( 512+PRGsize+CHRsize, temp+sizeof(NESHEADER) );
					crc    = CRC::CrcRev( 512+PRGsize, temp+sizeof(NESHEADER) );
				} else {
					crcall = CRC::CrcRev( PRGsize+CHRsize, temp+sizeof(NESHEADER) );
					crc    = CRC::CrcRev( PRGsize, temp+sizeof(NESHEADER) );
				}

				romdatabase.HeaderCorrect( header, crcall, crc );

#include "ROM_Patch.cpp"
				fdsmakerID = fdsgameID = 0;
			} else {
				crc = crcall = 0;

				fdsmakerID = lpPRG[0x1F];
				fdsgameID  = (lpPRG[0x20]<<24)|(lpPRG[0x21]<<16)|(lpPRG[0x22]<<8)|(lpPRG[0x23]<<0);
			}
		} else {
		// NSF
			mapper = 0x0100;	// Private mapper
			crc = crcall = 0;
			fdsmakerID = fdsgameID = 0;
		}

		FREE( temp );
	} catch( CHAR* str ) {
		// 原因がわかっているエラー処理
		FCLOSE( fp );
		FREE( temp );
		FREE( bios );

		FREE( lpPRG );
		FREE( lpCHR );
		FREE( lpTrainer );
		FREE( lpDiskBios );
		FREE( lpDisk );

#ifdef	_DATATRACE
		// For dis...
		FREE( PROM_ACCESS );
#endif

		throw	str;
#ifndef	_DEBUG
	} catch(...) {
		// 一般保護エラーとか出したく無いので...(^^;
		FCLOSE( fp );
		FREE( temp );
		FREE( bios );

		FREE( lpPRG );
		FREE( lpCHR );
		FREE( lpTrainer );
		FREE( lpDiskBios );
		FREE( lpDisk );

#ifdef	_DATATRACE
		// For dis...
		FREE( PROM_ACCESS );
#endif

		// 不明なエラーが発生しました
		throw	CApp::GetErrorString( IDS_ERROR_UNKNOWN );
#endif	// !_DEBUG
	}
}

//
// デストラクタ
//
ROM::~ROM()
{
	FREE( lpPRG );
	FREE( lpCHR );
	FREE( lpTrainer );
	FREE( lpDiskBios );
	FREE( lpDisk );

#ifdef	_DATATRACE
// _DATATRACE
// やっつけ仕事（お

	if( PROM_ACCESS ) {
		string	temp, path, path2;
		temp = CPathlib::SplitPath( CApp::GetModulePath() );
		path = CPathlib::MakePath( temp.c_str(), "DataAccess.lst" );

		FILE*	fp = NULL;

		if( (fp = fopen( path.c_str(), "w" )) ) {
			INT	start;
			INT	size = 0x4000*(INT)header.PRG_PAGE_SIZE;
			for( INT j = 0; j < size/0x4000; j++ ) {
				fprintf( fp, "; Page:%2d  ($%08X-$%08X)\n", j, j*0x4000, j*0x4000+0x3FFF );
				start = -1;
				for( INT i = 0; i < 0x4000; i++ ) {
					if( PROM_ACCESS[ j*0x4000+i ] ) {
						if( start == -1 )
							start = i;
					} else if( start != -1 ) {
						if( start == i-1 ) {
							fprintf( fp, "-db%04X:0000,8,$%04X\n", start, start );
//							fprintf( fp, "$%04X\n", start );
						} else {
							fprintf( fp, "-db%04X:%04X,8,$%04X-$%04X\n", start, (i-1)-start, start, i-1 );
//							fprintf( fp, "$%04X-$%04X\n", start, i-1 );
						}
						start = -1;
					}
				}
				if( start != -1 ) {
					fprintf( fp, "-db%04X:%04X,8,$%04X\n", start, (i-1)-start );
//					fprintf( fp, "$%04X - $%04X\n", start, 0x3FFF );
				}
				printf( "\n" );
			}

			FCLOSE( fp );
		}
#if	0
		{
			CHAR	szPath[_MAX_PATH];

			INT	start;
			INT	size = 0x4000*(INT)header.PRG_PAGE_SIZE;
			for( INT j = 0; j < size/0x4000; j++ ) {
				sprintf( szPath, "md6502_p%02d.lst", j );
				path2 = CPathlib::MakePath( temp.c_str(), szPath );
				if( (fp = fopen( path2.c_str(), "w" )) ) {
					fprintf( fp, "; Page:%2d ($%08X-$%08X)\n", j, j*0x4000, j*0x4000+0x3FFF );
					start = -1;
					for( INT i = 0; i < 0x4000; i++ ) {
						if( PROM_ACCESS[ j*0x4000+i ] ) {
							if( start == -1 )
								start = i;
						} else if( start != -1 ) {
							if( start == i-1 ) {
								fprintf( fp, "-db%04x:1,1\n", start );
							} else {
								fprintf( fp, "-db%04x-%04x,8\n", start, i-1 );
							}
							start = -1;
						}
					}
					if( start != -1 ) {
						fprintf( fp, "-db%04x-%04x,8\n", start, 0x3FFF );
					}
					printf( "\n" );

					FCLOSE( fp );
				}
			}
		}
#endif
	}

	// For dis...
	FREE( PROM_ACCESS );
#endif
}

//
// ROMファイルチェック
//
BOOL	ROM::IsRomFile( const char* fname )
{
FILE*	fp = NULL;
NESHEADER	header;

	if( !(fp = fopen( fname, "rb" )) )
		return	FALSE;

	// サイズ分読み込み
	if( fread( &header, sizeof(header), 1, fp ) != 1 ) {
		FCLOSE( fp );
		return	FALSE;
	}
	FCLOSE( fp );

	if( header.ID[0] == 'N' && header.ID[1] == 'E'
	 && header.ID[2] == 'S' && header.ID[3] == 0x1A ) {
		return	TRUE;
	} else if( header.ID[0] == 'F' && header.ID[1] == 'D'
		&& header.ID[2] == 'S' && header.ID[3] == 0x1A ) {
		return	TRUE;
	} else if( header.ID[0] == 'N' && header.ID[1] == 'E'
		&& header.ID[2] == 'S' && header.ID[3] == 'M') {
		return	TRUE;
	} else {
		LPBYTE	temp = NULL;
		LONG	size;
		if( !Uncompress( fname, &temp, (LPDWORD)&size ) )
			return	FALSE;

		memcpy( &header, temp, sizeof(NESHEADER) );
		FREE( temp );
		if( header.ID[0] == 'N' && header.ID[1] == 'E'
		 && header.ID[2] == 'S' && header.ID[3] == 0x1A ) {
			return	TRUE;
		} else if( header.ID[0] == 'F' && header.ID[1] == 'D'
			&& header.ID[2] == 'S' && header.ID[3] == 0x1A ) {
			return	TRUE;
		} else if( header.ID[0] == 'N' && header.ID[1] == 'E'
			&& header.ID[2] == 'S' && header.ID[3] == 'M') {
			return	TRUE;
		}
	}

	return	FALSE;
}

//
// ROM DATABASE (NESToy&NNNesterJ database)
//
INT	ROMDATABASE::HeaderCheck( NESHEADER& hdr, DWORD crcall, DWORD crc, ROMDB& data )
{
	if( m_DataBaseList.empty() ) {
		LoadDatabase();
	}

	if( m_DataBaseList.empty() )
		return	-2;	// データベースが無い

	for( list<ROMDB>::iterator it = m_DataBaseList.begin(); it != m_DataBaseList.end(); ) {
		if( it->crcall == crcall || (it->crc == crc && it->crc) ) {
			data = *it;
			if( hdr.control1 == it->control1 && hdr.control2 == it->control2 ) {
				return	0;	// 適合
			} else {
				return	1;	// CRCは一致したがヘッダが違う
			}
			return	TRUE;
		}
		it++;
	}

	return	-1;	// データベースに適合しない
}

BOOL	ROMDATABASE::HeaderCorrect( NESHEADER& hdr, DWORD crcall, DWORD crc )
{
	if( m_DataBaseList.empty() ) {
		LoadDatabase();
	}

	if( m_DataBaseList.empty() )
		return	FALSE;

	for( list<ROMDB>::iterator it = m_DataBaseList.begin(); it != m_DataBaseList.end(); ) {
		if( it->crcall == crcall || (it->crc == crc && it->crc) ) {
//DEBUGOUT( "ROMDATABASE::HeaderCheck it=%08X\n", it );
			hdr.control1 = it->control1;
			hdr.control2 = it->control2;
			for( INT i = 0; i < 8; i++ ) {
				hdr.reserved[i] = 0;
			}
			return	TRUE;
		}
		it++;
	}
	return	FALSE;
}

void	ROMDATABASE::LoadDatabase()
{
FILE*	fp = NULL;
CHAR	buf[512];
const UCHAR seps[] = ";\n\0";	// セパレータ
ROMDB	db;

DEBUGOUT( "Database loading...\n" );

	string	Path = CPathlib::MakePathExt( CApp::GetModulePath(), "nesromdb", "dat" );

DEBUGOUT( "File:%s\n", Path.c_str() );

	m_DataBaseList.clear();

	if( (fp = fopen( Path.c_str(), "r" )) ) {
		while( fgets( buf, 512, fp ) ) {
			if( buf[0] == ';' ) {	// コメントフィールドとみなす
				continue;
			}

			CHAR*	pToken;

			// ALL CRC
			if( !(pToken = (CHAR*)_mbstok( (UCHAR*)buf, seps )) )
				continue;
			db.crcall = strtoul( pToken, NULL, 16 );
			// PRG CRC
			if( !(pToken = (CHAR*)_mbstok( NULL, seps )) )
				continue;
			db.crc = strtoul( pToken, NULL, 16 );

			// Title
			if( !(pToken = (CHAR*)_mbstok( NULL, seps )) )
				continue;
			db.title = pToken;

			// Control 1
			if( !(pToken = (CHAR*)_mbstok( NULL, seps )) )
				continue;
			db.control1 = atoi( pToken );
			// Control 2
			if( !(pToken = (CHAR*)_mbstok( NULL, seps )) )
				continue;
			db.control2 = atoi( pToken );

			// PRG SIZE
			if( !(pToken = (CHAR*)_mbstok( NULL, seps )) )
				continue;
			db.prg_size = atoi( pToken );
			// CHR SIZE
			if( !(pToken = (CHAR*)_mbstok( NULL, seps )) )
				continue;
			db.chr_size = atoi( pToken );

			// Country
			if( !(pToken = (CHAR*)_mbstok( NULL, seps )) )
				continue;
			db.country = pToken;

			db.bNTSC = TRUE;
			// Europe (PAL???)
			if( strcmp( pToken, "E"   ) == 0
			 || strcmp( pToken, "Fra" ) == 0
			 || strcmp( pToken, "Ger" ) == 0
			 || strcmp( pToken, "Spa" ) == 0
			 || strcmp( pToken, "Swe" ) == 0
			 || strcmp( pToken, "Ita" ) == 0
			 || strcmp( pToken, "Aus" ) == 0 ) {
				db.bNTSC = FALSE;
			}

			// Manufacturer
			if( pToken = (CHAR*)_mbstok( NULL, seps ) ) {
				db.manufacturer = pToken;
			} else {
				db.manufacturer.erase( db.manufacturer.begin(), db.manufacturer.end() );
			}

			// Sale date
			if( pToken = (CHAR*)_mbstok( NULL, seps ) ) {
				db.saledate = pToken;
			} else {
				db.saledate.erase( db.saledate.begin(), db.saledate.end() );
			}

			// Price
			if( pToken = (CHAR*)_mbstok( NULL, seps ) ) {
				db.price = pToken;
			} else {
				db.price.erase( db.price.begin(), db.price.end() );
			}

			// Genre
			if( pToken = (CHAR*)_mbstok( NULL, seps ) ) {
				db.genre = pToken;
			} else {
				db.genre.erase( db.genre.begin(), db.genre.end() );
			}

			m_DataBaseList.push_back( db );
		}
		FCLOSE( fp );
	} else {
DEBUGOUT( "Database file not found.\n" );
	}
}

