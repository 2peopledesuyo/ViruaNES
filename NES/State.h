#ifndef	__STATE_INCLUDED__
#define	__STATE_INCLUDED__

#pragma pack( push, 1 )

typedef	struct	tagFILEHDR {	//  0123456789AB
	BYTE	ID[12];		// "VirtuaNES ST"
	WORD	Reserved;
	WORD	BlockVersion;
} FILEHDR, *LPFILEHDR;

// VirtuaNES version0.30�ȍ~�p
typedef	struct	tagFILEHDR2 {	//  0123456789AB
	BYTE	ID[12];		// "VirtuaNES ST"
	WORD	Reserved;
	WORD	BlockVersion;	// 0x0200

	DWORD	Ext0;		// ROM:�v���O����CRC	FDS:�v���O����ID
	WORD	Ext1;		// ROM:�Ȃ�		FDS:���[�J�[ID
	WORD	Ext2;		// ROM:�Ȃ�		FDS:�f�B�X�N����
	LONG	MovieStep;	// �ǋL(��蒼��)���[�r�[���̃t���[����
	LONG	MovieOffset;	// �ǋL(��蒼��)���[�r�[���̃t�@�C���I�t�Z�b�g
} FILEHDR2, *LPFILEHDR2;

typedef	struct	tagBLOCKHDR {
	BYTE	ID[8];
	WORD	Reserved;
	WORD	BlockVersion;
	DWORD	BlockSize;
} BLOCKHDR, *LPBLOCKHDR;

// CPU ���W�X�^
typedef	struct	tagCPUSTAT {
	WORD	PC;
	BYTE	A;
	BYTE	X;
	BYTE	Y;
	BYTE	S;
	BYTE	P;
	BYTE	I;	// Interrupt pending flag

	BYTE	FrameIRQ;
	BYTE	reserved[3];

	LONG	mod_cycles;	// ���[�r�[���ŃN���b�N���̔����Ȃ����h����
} CPUSTAT, *LPCPUSTAT;

// PPU ���W�X�^
typedef	struct	tagPPUSTAT {
	BYTE	reg0;
	BYTE	reg1;
	BYTE	reg2;
	BYTE	reg3;
	BYTE	reg7;
	BYTE	toggle56;

	WORD	loopy_t;
	WORD	loopy_v;
	WORD	loopy_x;
} PPUSTAT, *LPPPUSTAT;

// APU ���W�X�^(�g���T�E���h�܂�)
typedef	struct	tagAPUSTAT {
	BYTE	reg[0x0018];
	BYTE	ext[0x0100];
} APUSTAT, *LPAPUSTAT;

//
// ���W�X�^�f�[�^
// ID "REG DATA"
typedef	struct	tagREGSTAT {
	union	CPUREG {
		BYTE	cpudata[32];
		CPUSTAT	cpu;
	} cpureg;
	union	PPUREG {
		BYTE	ppudata[32];
		PPUSTAT	ppu;
	} ppureg;
	APUSTAT	apu;
} REGSTAT, *LPREGSTAT;

//
// ����RAM�f�[�^
// ID "RAM DATA"
typedef	struct	tagRAMSTAT {
	BYTE	RAM[2*1024];	// Internal NES RAM
	BYTE	BGPAL[16];	// BG Palette
	BYTE	SPPAL[16];	// SP Palette
	BYTE	SPRAM[256];	// Sprite RAM
} RAMSTAT, *LPRAMSTAT;

//
// MMU�f�[�^
// ID "MMU DATA"
typedef	struct	tagMMUSTAT {
	BYTE	CPU_MEM_TYPE[8];
	WORD	CPU_MEM_PAGE[8];
	BYTE	PPU_MEM_TYPE[12];
	WORD	PPU_MEM_PAGE[12];
	BYTE	CRAM_USED[8];
} MMUSTAT, *LPMMUSTAT;

//
// �}�b�p�[�f�[�^
// ID "MMC DATA"
typedef	struct	tagMMCSTAT {
	BYTE	mmcdata[256];
} MMCSTAT, *LPMMCSTAT;

//
// �R���g���[���f�[�^
// ID "CONTDATA"
typedef	struct	tagCONTSTAT {
	BYTE	type1;
	BYTE	type2;
	BYTE	shift1;
	BYTE	shift2;
	DWORD	data1;
	DWORD	data2;
} CONTSTAT, *LPCONTSTAT;

//
// �f�B�X�N�C���[�W
// Ver0.24�܂�
// ID "DSIDE 0A","DSIDE 0B","DSIDE 1A","DSIDE 1B"
typedef	struct	tagDISKSTAT {
	BYTE	DiskTouch[16];
} DISKSTAT, *LPDISKSTAT;

// Ver0.30�ȍ~
// ID "DISKDATA"
typedef	struct	tagDISKDATA {
	LONG	DifferentSize;
} DISKDATA, *LPDISKDATA;

// �ȉ��̓f�B�X�N�Z�[�u�C���[�W�t�@�C���Ŏg�p����
// Ver0.24�܂�
typedef	struct	tagDISKIMGFILEHDR {	//  0123456789AB
	BYTE	ID[12];		// "VirtuaNES DI"
	WORD	BlockVersion;
	WORD	DiskNumber;
} DISKIMGFILEHDR, *LPDISKIMGFILEHDR;

typedef	struct	tagDISKIMGHDR {
	BYTE	ID[6];		// ID "SIDE0A","SIDE0B","SIDE1A","SIDE1B"
	BYTE	DiskTouch[16];
} DISKIMGHDR, *LPDISKIMGHDR;

// VirtuaNES version0.30�ȍ~�p
typedef	struct	tagDISKFILEHDR {	//  0123456789AB
	BYTE	ID[12];		// "VirtuaNES DI"
	WORD	BlockVersion;	// 0x0200:0.30	0x0210:0.31
	WORD	Reserved;
	DWORD	ProgID;		// �v���O����ID
	WORD	MakerID;	// ���[�J�[ID
	WORD	DiskNo;		// �f�B�X�N��
	DWORD	DifferentSize;	// ���ᐔ
} DISKFILEHDR, *LPDISKFILEHDR;


//
// ���[�r�[�t�@�C��
//
typedef	struct	tagMOVIEIMGFILEHDR {
	BYTE	ID[12];			// "VirtuaNES MV"
	WORD	BlockVersion;
	WORD	reserved;
	LONG	StateStOffset;		// Movie start state offset
	LONG	StateEdOffset;		// Movie end state offset
	LONG	MovieOffset;		// Movie data offset
	LONG	MovieStep;		// Movie steps
} MOVIEIMGFILEHDR, *LPMOVIEIMGFILEHDR;

// VirtuaNES version0.30�ȍ~�p
typedef	struct	tagMOVIEFILEHDR {
	BYTE	ID[12];			// "VirtuaNES MV"
	WORD	BlockVersion;		// 0x0200
	WORD	reserved;
	DWORD	Control;		// �R���g���[���o�C�g
					// 76543210(Bit)
					// E---4321
					// |   |||+-- 1P�f�[�^
					// |   ||+--- 2P�f�[�^
					// |   |+---- 3P�f�[�^
					// |   +----- 4P�f�[�^
					// +--------- �ǋL�֎~
					// ���̑��R���g���[����1P�`4P(�ǂ�ł��ǂ�)�̕����L�[��
					// �S��ON�̎��C���̂S�o�C�g���R���g���[���p�f�[�^�ɂȂ�
	DWORD	Ext0;			// ROM:�v���O����CRC	FDS:�v���O����ID
	WORD	Ext1;			// ROM:�Ȃ�		FDS:���[�J�[ID
	WORD	Ext2;			// ROM:�Ȃ�		FDS:�f�B�X�N����
	DWORD	RecordTimes;		// �L�^��(��蒼����)

	LONG	StateStOffset;		// Movie start state offset
	LONG	StateEdOffset;		// Movie end state offset
	LONG	MovieOffset;		// Movie data offset
	LONG	MovieStep;		// Movie steps(Frame��)

	DWORD	CRC;			// ���̃f�[�^������CRC(�C���`�L�h�~)
} MOVIEFILEHDR, *LPMOVIEFILEHDR;

// Famtasia Movie....
typedef	struct	tagFMVHDR {
	BYTE	ID[4];			// "FMV^Z"
	BYTE	Control1;		// R???????	0:���Z�b�g�ォ��L�^�H 1:�r������L�^
	BYTE	Control2;		// OT??????	O:1P��� T:2P���
	DWORD	Unknown1;
	WORD	RecordTimes;		// �L�^��-1
	DWORD	Unknown2;
	BYTE	szEmulators[0x40];	// �L�^�����G�~�����[�^
	BYTE	szTitle    [0x40];	// �^�C�g��
} FMVHDR, *LPFMVHDR;

// Nesticle Movie....
typedef	struct	tagNMVHDR {
	BYTE	ExRAM[0x2000];
	BYTE	S_RAM[0x0800];
	WORD	PC;
	BYTE	A;
	BYTE	P;
	BYTE	X;
	BYTE	Y;
	BYTE	SP;
	BYTE	OAM[0x0100];
	BYTE	VRAM[0x4000];
	BYTE	Other[0xC9];
	DWORD	ScanlineCycles;
	DWORD	VblankScanlines;
	DWORD	FrameScanlines;
	DWORD	VirtualFPS;
} NMVHDR, *LPNMVHDR;

#pragma pack( pop )

#endif	// !__STATE_INCLUDED__
