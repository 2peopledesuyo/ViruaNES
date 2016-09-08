//////////////////////////////////////////////////////////////////////////
// Mapper189  Street Fighter 2/Yoko version                             //
//////////////////////////////////////////////////////////////////////////
class	Mapper189 : public Mapper
{
public:
	Mapper189( NES* parent ) : Mapper(parent) {}

	void	Reset();
	void	WriteLow( WORD addr, BYTE data );
	void	Write( WORD addr, BYTE data );
	void	HSync( INT scanline );

	void	SetBank_PPU();

	// For state save
	BOOL	IsStateSave() { return TRUE; }
	void	SaveState( LPBYTE p );
	void	LoadState( LPBYTE p );

protected:
	BYTE	reg[2];
	BYTE	chr01, chr23, chr4, chr5, chr6, chr7;

	BYTE	irq_enable;
	BYTE	irq_counter;
	BYTE	irq_latch;

private:
};
