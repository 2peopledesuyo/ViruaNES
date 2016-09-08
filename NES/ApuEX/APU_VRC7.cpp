//////////////////////////////////////////////////////////////////////////
//                                                                      //
//      Konami VRC7                                                     //
//                                                           Norix      //
//                                               written     2001/09/18 //
//                                               last modify ----/--/-- //
//////////////////////////////////////////////////////////////////////////
#include "APU_VRC7.h"

APU_VRC7::APU_VRC7()
{
	OPLL_init( 3579545, (uint32)22050 );	// ���̃T���v�����O���[�g
	VRC7_OPLL = OPLL_new();

	if( VRC7_OPLL ) {
		OPLL_reset( VRC7_OPLL );
		OPLL_reset_patch( VRC7_OPLL, OPLL_VRC7_TONE );
		VRC7_OPLL->masterVolume = 128;
	}

	Reset( 22050 );	// ���̎��g��
}

APU_VRC7::~APU_VRC7()
{
	if( VRC7_OPLL ) {
		OPLL_delete( VRC7_OPLL );
		VRC7_OPLL = NULL;
//		OPLL_close();	// �����Ă��ǂ�(���g����)
	}
}

void	APU_VRC7::Reset( INT nRate )
{
	if( VRC7_OPLL ) {
		OPLL_reset( VRC7_OPLL );
		OPLL_reset_patch( VRC7_OPLL, OPLL_VRC7_TONE );
		VRC7_OPLL->masterVolume = 128;
	}

	address = 0;

	Setup( nRate );
}

void	APU_VRC7::Setup( INT nRate )
{
	OPLL_setClock( 3579545, (uint32)nRate );
}

void	APU_VRC7::Write( WORD addr, BYTE data )
{
	if( VRC7_OPLL ) {
		if( addr == 0x9010 ) {
			address = data;
		} else if( addr == 0x9030 ) {
			OPLL_writeReg( VRC7_OPLL, address, data );
		}
	}
}

INT	APU_VRC7::Process( INT channel )
{
	if( VRC7_OPLL )
		return	OPLL_calc( VRC7_OPLL );

	return	0;
}

