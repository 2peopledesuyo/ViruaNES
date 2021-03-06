//
// DirectSound class
//
#ifndef	__DIRECTSOUND_INCLUDED__
#define	__DIRECTSOUND_INCLUDED__

#define	WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include <dsound.h>

#include "typedef.h"
#include "macro.h"

class	CDirectSound
{
public:
	// パブリックメンバ変数
	typedef	struct	tagSAMPLERATE {
		DWORD	Rate;
		DWORD	Bits;
	} SAMPLERATE, *LPSAMPLERATE;

	SAMPLERATE		m_SampleRate;		// 現在サンプリングレ?ト
	INT			m_BufferSize;		// バッフ?サイズ(フレ??数)
	static	SAMPLERATE	m_SampleRateTable[];	// サンプリングレ?トテ?ブル
	static	INT		m_BufferSizeTable[];	// バッフ?サイズテ?ブル

	// パブリックメンバ関数
	CDirectSound();
	virtual ~CDirectSound();

	BOOL	InitialDSound( HWND hWnd );
	void	ReleaseDSound();

	BOOL	InitialBuffer();
	void	ReleaseBuffer();

	BOOL	SetSamplingRate( DWORD rate, DWORD bits );
	void	GetSamplingRate( DWORD& rate, DWORD& bits );

	void	SetBufferSize( INT nSize ) { m_BufferSize = nSize; }
	INT	GetBufferSize()		   { return m_BufferSize; }

	BOOL	IsStreamPlaying() { return (m_bStreamPlay&&!m_bStreamPause); }
	void	StreamPlay();
	void	StreamStop();
	void	StreamPause();
	void	StreamResume();

	BOOL	GetStreamLockPosition( LPDWORD lpdwStart, LPDWORD lpdwSize );
	BOOL	StreamLock( DWORD dwWriteCursor, DWORD dwWriteBytes, LPVOID* lplpvPtr1, LPDWORD lpdwBytes1, LPVOID* lplpvPtr2, LPDWORD lpdwBytes2, DWORD dwFlags );
	BOOL	StreamUnlock( LPVOID lpvPtr1, DWORD dwBytes1, LPVOID lpvPtr2, DWORD dwBytes2 );

	BOOL	IsStreamPlay() { return m_bStreamPlay; }
	BOOL	IsStreamPause() { return m_bStreamPause; }

protected:
	// プロテクトメンバ変数
	HWND	m_hWnd;		// Window handle

	LPDIRECTSOUND		m_lpDS;		// DirectSoundオブジェクト
	LPDIRECTSOUNDBUFFER	m_lpDSPrimary;	// プライ?リオブジェクト

	LPDIRECTSOUNDBUFFER	m_lpDSStream;		// ストリ??ングオブジェクト

	DWORD			m_dwDSBufferSize;
	DWORD			m_dwDSBlockSize;	// 1ブロックのサイズ
	DWORD			m_dwDSBlockNum;		// ブロックの数
	DWORD			m_dwDSLastBlock;	// 最後に書き込んだブロック位置

	volatile BOOL		m_bStreamPlay;	// ストリ??再生中フラグ
	volatile BOOL		m_bStreamPause;	// ストリ????ズフラグ

	// プロテクトメンバ関数
private:
	// プライベ?トメンバ変数
	// プライベ?トメンバ関数
};

extern	CDirectSound	DirectSound;

#endif	// !__DIRECTSOUND_INCLUDED__
