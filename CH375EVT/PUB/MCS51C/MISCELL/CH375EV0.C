/* 2004.03.05
  �޸ļ�¼��
    2007.08֧�ִ�����, ������CH375BоƬ, �޸���: mInitDisk, mReadSector, mWriteSector, �����˽�: mClearError
****************************************
**  Copyright  (C)  W.ch  1999-2004   **
**  Web:  http://www.winchiphead.com  **
****************************************
**  USB 1.1 Host Examples for CH375   **
**  KC7.0@MCS-51                      **
****************************************
*/
/* CH375��ΪUSB�����ӿڵĳ���ʾ�� */

/* MCS-51��Ƭ��C���Ե�ʾ������, U�����ݶ�д */

#include <reg51.h>
#include <string.h>
#include <stdio.h>

#define		MAX_SECTOR_SIZE		4096	/* ��512�ֽ�ÿ����Ϊ��,������2K�ֽ�ÿ����,���Ϊ4K�ֽ� */

/* ����CH375������뼰����״̬ */
#include "CH375INC.H"
/* CH375���� */
#define CH375_BLOCK_SIZE		64		/* CH375 maximum data block size */

/* ���¶���������MCS-51��Ƭ��,������Ƭ�������޸�,Ϊ���ṩC���Ե��ٶ���Ҫ�Ա���������Ż� */
#include <reg51.h>
unsigned char volatile xdata	CH375_CMD_PORT _at_ 0xBDF1;	/* CH375����˿ڵ�I/O��ַ */
unsigned char volatile xdata	CH375_DAT_PORT _at_ 0xBCF0;	/* CH375���ݶ˿ڵ�I/O��ַ */
unsigned char xdata				DATA_BUFFER[ MAX_SECTOR_SIZE ]    _at_ 0x0000;	/* �ⲿRAM���ݻ���������ʼ��ַ,���Ȳ�����һ�ζ�д�����ݳ��� */
sbit	CH375_INT_WIRE	=		0xB0^2;	/* P3.2, INT0, ����CH375��INT#����,���ڲ�ѯ�ж�״̬ */

unsigned short  BytePerSector;		/* ÿ�����ֽ�����������С */
unsigned char   BlockPerSector;		/* ÿ����������ָCH375��дʱ�Ŀ� BlockPerSector=BytePerSector/CH375_BLOCK_SIZE */

/* ��P1.4����һ��LED���ڼ����ʾ����Ľ���,�͵�ƽLED��,��U�̲������ */
sbit P1_4  = P1^4;
#define LED_OUT_ACT( )		{ P1_4 = 0; }	/* P1.4 �͵�ƽ����LED��ʾ */
#define LED_OUT_INACT( )	{ P1_4 = 1; }	/* P1.4 �͵�ƽ����LED��ʾ */


/* ��ʱ2΢��,����ȷ */
void	delay2us( )
{
	unsigned char i;
	for ( i = 2; i != 0; i -- );
}

/* ��ʱ1΢��,����ȷ */
void	delay1us( )
{
	unsigned char i;
	for ( i = 1; i != 0; i -- );
}

/* ��ʱ����,����ȷ */
void	mDelaymS( unsigned char cnt )
{
	unsigned char	i, c;
	while ( cnt -- ) {
		for ( i = 250; i != 0; i -- ) c+=3;
	}
}

/* �������� */

void CH375_WR_CMD_PORT( unsigned char cmd ) {  /* ��CH375������˿�д������,���ڲ�С��4uS,�����Ƭ���Ͽ�����ʱ */
	delay2us();
	CH375_CMD_PORT=cmd;
	delay2us();
}

void CH375_WR_DAT_PORT( unsigned char dat ) {  /* ��CH375�����ݶ˿�д������,���ڲ�С��1.5uS,�����Ƭ���Ͽ�����ʱ */
	CH375_DAT_PORT=dat;
	delay1us();  /* ��ΪMCS51��Ƭ����������ʵ����������ʱ */
}

unsigned char CH375_RD_DAT_PORT() {  /* ��CH375�����ݶ˿ڶ�������,���ڲ�С��1.5uS,�����Ƭ���Ͽ�����ʱ */
	delay1us();  /* ��ΪMCS51��Ƭ����������ʵ����������ʱ */
	return( CH375_DAT_PORT );
}

/* �ȴ�CH375�жϲ���ȡ״̬ */
unsigned char mWaitInterrupt() {  /* �����˵ȴ��������, ���ز���״̬ */
	while( CH375_INT_WIRE );  /* ��ѯ�ȴ�CH375��������ж�(INT#�͵�ƽ) */
	CH375_WR_CMD_PORT( CMD_GET_STATUS );  /* ������������ж�, ��ȡ�ж�״̬ */
	return( CH375_RD_DAT_PORT( ) );
/*	c = CH375_RD_DAT_PORT( );   �����ж�״̬ */
/*	if ( c == USB_INT_DISCONNECT ) ?;   ��⵽USB�豸�Ͽ��¼� */
/*	else if ( c == USB_INT_CONNECT ) ?;   ��⵽USB�豸�����¼� */
}

/* ����CH375ΪUSB������ʽ */
unsigned char	mCH375Init( )
{
	unsigned char	i;
#ifdef	TEST_CH375_PORT
	unsigned char	c;
	CH375_WR_CMD_PORT( CMD_CHECK_EXIST );  /* ���Թ���״̬ */
	CH375_WR_DAT_PORT( 0x55 );  /* �������� */
	c = CH375_RD_DAT_PORT( );  /* ��������Ӧ���ǲ�������ȡ�� */
	if ( c != 0xaa ) {  /* CH375���� */
		for ( i = 100; i != 0; i -- ) {  /* ǿ������ͬ�� */
			CH375_WR_CMD_PORT( CMD_RESET_ALL );  /* CH375ִ��Ӳ����λ */
			c = CH375_RD_DAT_PORT( );  /* ��ʱ */
		}
		mDelaymS( 50 );  /* ��ʱ����30mS */
	}
#endif
	CH375_WR_CMD_PORT( CMD_SET_USB_MODE );  /* ����USB����ģʽ */
	CH375_WR_DAT_PORT( 6 );  /* ģʽ����,�Զ����USB�豸���� */
	for ( i = 0xff; i != 0; i -- )  /* �ȴ������ɹ�,ͨ����Ҫ�ȴ�10uS-20uS */
		if ( CH375_RD_DAT_PORT( ) == CMD_RET_SUCCESS ) break;  /* �����ɹ� */
	if ( i != 0 ) return( 0 );  /* �����ɹ� */
	else return( 0xff );  /* CH375����,����оƬ�ͺŴ����ߴ��ڴ��ڷ�ʽ���߲�֧�� */
}

/* ��ʼ������ */
unsigned char	mInitDisk( )
{
	unsigned char mIntStatus, i;
	CH375_WR_CMD_PORT( CMD_GET_STATUS );  /* ������������ж�, ��ȡ�ж�״̬ */
	mIntStatus = CH375_RD_DAT_PORT( );
	if ( mIntStatus == USB_INT_DISCONNECT ) return( mIntStatus );  /* USB�豸�Ͽ� */
	CH375_WR_CMD_PORT( CMD_DISK_INIT );  /* ��ʼ��USB�洢�� */
	mIntStatus = mWaitInterrupt( );  /* �ȴ��жϲ���ȡ״̬ */
	if ( mIntStatus != USB_INT_SUCCESS ) return( mIntStatus );  /* ���ִ��� */
	CH375_WR_CMD_PORT( CMD_DISK_SIZE );  /* ��ȡUSB�洢�������� */
	mIntStatus = mWaitInterrupt( );  /* �ȴ��жϲ���ȡ״̬ */
	if ( mIntStatus != USB_INT_SUCCESS ) {  /* �������� */
		mDelaymS( 200 );
		CH375_WR_CMD_PORT( CMD_DISK_SIZE );  /* ��ȡUSB�洢�������� */
		mIntStatus = mWaitInterrupt( );  /* �ȴ��жϲ���ȡ״̬ */
	}
	if ( mIntStatus != USB_INT_SUCCESS ) return( mIntStatus );  /* ���ִ��� */

/* ������CMD_RD_USB_DATA����������ݶ���,����ÿ�����ֽ��� */
	CH375_WR_CMD_PORT( CMD_RD_USB_DATA );  /* ��CH375��������ȡ���ݿ� */
	i = CH375_RD_DAT_PORT( );  /* �������ݵĳ��� */
	if ( i != 8 ) return( USB_INT_DISK_ERR );  /* �쳣 */
	for ( i = 0; i != 8; i ++ ) {  /* ���ݳ��ȶ�ȡ���� */
		DATA_BUFFER[ i ] = CH375_RD_DAT_PORT( );  /* �������ݲ����� */
	}
	i = DATA_BUFFER[ 6 ];  /* U�����������е�ÿ�����ֽ���,��˸�ʽ */
	if ( i == 0x04 ) BlockPerSector = 1024/CH375_BLOCK_SIZE;  /* ���̵�����������1K�ֽ� */
	else if ( i == 0x08 ) BlockPerSector = 2048/CH375_BLOCK_SIZE;  /* ���̵�����������2K�ֽ� */
	else if ( i == 0x10 ) BlockPerSector = 4096/CH375_BLOCK_SIZE;  /* ���̵�����������4K�ֽ� */
	else BlockPerSector = 512/CH375_BLOCK_SIZE;  /* Ĭ�ϵĴ��̵�����������512�ֽ� */
	BytePerSector = BlockPerSector*CH375_BLOCK_SIZE;  /* �������̵�������С */
	CH375_WR_CMD_PORT( CMD_SET_PKT_P_SEC );  /* ����USB�洢����ÿ�������ݰ����� */
	CH375_WR_DAT_PORT( 0x39 );
	CH375_WR_DAT_PORT( BlockPerSector );  /* ����ÿ�������ݰ����� */
	return( 0 );  /* U���Ѿ��ɹ���ʼ�� */
}

/* ���U�̴����Ա����� */
void	mClearError( void )
{
	mDelaymS( 10 );  /* ��ʱ10mS */
	CH375_WR_CMD_PORT( CMD_DISK_R_SENSE );  /* ���USB�洢������ */
	mDelaymS( 10 );  /* ��ʱ10mS */
	mWaitInterrupt( );  /* �ȴ��жϲ���ȡ״̬ */
}

/* ��U�̶�ȡ������������ݿ鵽������ */
unsigned char	mReadSector( unsigned long iLbaStart, unsigned char iSectorCount )
/* iLbaStart ��׼����ȡ��������ʼ������, iSectorCount ��׼����ȡ�������� */
{
	unsigned char mIntStatus;
	unsigned char *mBufferPoint;
	unsigned int  mBlockCount;
	unsigned char mLength;
	CH375_WR_CMD_PORT( CMD_DISK_READ );  /* ��USB�洢�������ݿ� */
	CH375_WR_DAT_PORT( (unsigned char)iLbaStart );  /* LBA�����8λ */
	CH375_WR_DAT_PORT( (unsigned char)( iLbaStart >> 8 ) );
	CH375_WR_DAT_PORT( (unsigned char)( iLbaStart >> 16 ) );
	CH375_WR_DAT_PORT( (unsigned char)( iLbaStart >> 24 ) );  /* LBA�����8λ */
	CH375_WR_DAT_PORT( iSectorCount );  /* ������ */
	mBufferPoint = DATA_BUFFER;  /* ָ�򻺳�����ʼ��ַ */
	for ( mBlockCount = iSectorCount * BlockPerSector; mBlockCount != 0; mBlockCount -- ) {  /* ���ݿ���� */
		mIntStatus = mWaitInterrupt( );  /* �ȴ��жϲ���ȡ״̬ */
		if ( mIntStatus == USB_INT_DISK_READ ) {  /* USB�洢�������ݿ�,�������ݶ��� */
			CH375_WR_CMD_PORT( CMD_RD_USB_DATA );  /* ��CH375��������ȡ���ݿ� */
			mLength = CH375_RD_DAT_PORT( );  /* �������ݵĳ��� */
			while ( mLength ) {  /* ���ݳ��ȶ�ȡ���� */
				*mBufferPoint = CH375_RD_DAT_PORT( );  /* �������ݲ����� */
				mBufferPoint ++;
				mLength --;
			}
			CH375_WR_CMD_PORT( CMD_DISK_RD_GO );  /* ����ִ��USB�洢���Ķ����� */
		}
		else break;  /* ���ش���״̬ */
	}
	if ( mBlockCount == 0 ) {
		mIntStatus = mWaitInterrupt( );  /* �ȴ��жϲ���ȡ״̬ */
		if ( mIntStatus == USB_INT_SUCCESS ) return( 0 );  /* �����ɹ� */
	}
//	if ( mIntStatus == USB_INT_DISCONNECT ) return( mIntStatus );  /* U�̶Ͽ� */
	mClearError( );  /* ���U�̴����Ա����� */
	return( mIntStatus );  /* ����ʧ�� */
}

/* ���������еĶ�����������ݿ�д��U�� */
unsigned char	mWriteSector( unsigned long iLbaStart, unsigned char iSectorCount )
/* iLbaStart ��д�������ʼ��������, iSectorCount ��д��������� */
{
	unsigned char mIntStatus;
	unsigned char *mBufferPoint;
	unsigned int  mBlockCount;
	unsigned char mLength;
	CH375_WR_CMD_PORT( CMD_DISK_WRITE );  /* ��USB�洢��д���ݿ� */
	CH375_WR_DAT_PORT( (unsigned char)iLbaStart );  /* LBA�����8λ */
	CH375_WR_DAT_PORT( (unsigned char)( iLbaStart >> 8 ) );
	CH375_WR_DAT_PORT( (unsigned char)( iLbaStart >> 16 ) );
	CH375_WR_DAT_PORT( (unsigned char)( iLbaStart >> 24 ) );  /* LBA�����8λ */
	CH375_WR_DAT_PORT( iSectorCount );  /* ������ */
	mBufferPoint = DATA_BUFFER;  /* ָ�򻺳�����ʼ��ַ */
	for ( mBlockCount = iSectorCount * BlockPerSector; mBlockCount != 0; mBlockCount -- ) {  /* ���ݿ���� */
		mIntStatus = mWaitInterrupt( );  /* �ȴ��жϲ���ȡ״̬ */
		if ( mIntStatus == USB_INT_DISK_WRITE ) {  /* USB�洢��д���ݿ�,��������д�� */
			CH375_WR_CMD_PORT( CMD_WR_USB_DATA7 );  /* ��CH375������д�����ݿ� */
			mLength = CH375_BLOCK_SIZE;
			CH375_WR_DAT_PORT( mLength );  /* �������ݵĳ��� */
			while ( mLength ) {  /* ���ݳ���д������ */
				CH375_WR_DAT_PORT( *mBufferPoint );  /* ������д�� */
				mBufferPoint ++;
				mLength --;
			}
/*			do { ����C51,���DO+WHILE�ṹ�������WHILEЧ�ʸ�,�ٶȿ�
				CH375_WR_DAT_PORT( *mBufferPoint );
				mBufferPoint ++;
			} while ( -- mLength );*/
			CH375_WR_CMD_PORT( CMD_DISK_WR_GO );  /* ����ִ��USB�洢����д���� */
		}
		else break;  /* ���ش���״̬ */
	}
	if ( mBlockCount == 0 ) {
		mIntStatus = mWaitInterrupt( );  /* �ȴ��жϲ���ȡ״̬ */
		if ( mIntStatus == USB_INT_SUCCESS ) return( 0 );  /* �����ɹ� */
	}
//	if ( mIntStatus == USB_INT_DISCONNECT ) return( mIntStatus );  /* U�̶Ͽ� */
	mClearError( );  /* ���U�̴����Ա����� */
	return( mIntStatus );  /* ����ʧ�� */
}

struct _HD_MBR_DPT {
	unsigned char	PartState;
	unsigned char	StartHead;
	unsigned int	StartSec;
	unsigned char	PartType;
	unsigned char	EndHead;
	unsigned int	EndSec;
	unsigned long	StartSector;
	unsigned long	TotalSector;
};

/* Ϊprintf��getkey���������ʼ������ */
void	mInitSTDIO( )
{
	SCON = 0x50;
	PCON = 0x80;
	TMOD = 0x20;
	TH1 = 0xf3;  /* 24MHz����, 9600bps */
	TR1 = 1;
	TI = 1;
}

main( ) {
	unsigned char	c, mIntStatus;
	LED_OUT_ACT( );  /* ������LED��һ����ʾ���� */
	mDelaymS( 100 );  /* ��ʱ100���� */
	LED_OUT_INACT( );
	mInitSTDIO( );
	printf( "Start\n" );
	c = mCH375Init( );  /* ��ʼ��CH375 */
	if ( c ) printf( "Error @CH375Init\n" );
	printf( "Insert USB disk\n" );
	do {  /* �ȴ�U������ */
		mIntStatus = mWaitInterrupt( );  /* �ȴ��жϲ���ȡ״̬ */
	} while ( mIntStatus != USB_INT_CONNECT );  /* U��û�����ӻ����Ѿ��γ� */
	mDelaymS( 200 );  /* ��ʱ�ȴ�U�̽�����������״̬ */
	printf( "InitDisk\n" );
	c = mInitDisk( );  /* ��ʼ��U��,ʵ����ʶ��U�̵�����,��Ӱ��U���е�����,�����ж�д����֮ǰ������д˲��� */
	if ( c ) printf( "Error @InitDisk, %02X\n", c );
	LED_OUT_ACT( );

/* ���U���Ƿ�׼����,�����U�̲���Ҫ��һ��,����ĳЩU�̱���Ҫִ����һ�����ܹ��� */
//	do {
//		mDelaymS( 100 );
//		printf( "Disk Ready ?\n" );
//		i = CH375DiskReady( );  /* ��ѯ�����Ƿ�׼����,���ʡ������ӳ�����Խ�Լ����1KB�ĳ������ */
//	} while ( i != ERR_SUCCESS );
/* CH375DiskReady ��CH375��U���ļ��ӳ������,��Ϊ����϶�,���Դ˴�ʡȥ */

	printf( "ReadSector 0# to buffer\n" );
	c = mReadSector( 0, 1 );
	if ( c ) printf( "Error @ReadSector, %02X\n", c );
	if ( DATA_BUFFER[0x01FF] == 0xAA ) {  /* ���̷�����Ч */
		printf( "WriteSector 1# from buffer\n" );
		c = mWriteSector( 1, 1 );
		if ( c ) printf( "Error @WriteSector, %02X\n", c );
		memset( DATA_BUFFER, 0, sizeof(DATA_BUFFER) );  /* ������ݻ�����,����ԭ���ķ�����Ϣ */
		printf( "WriteSector 0# for clear\n" );
		c = mWriteSector( 0, 1 );
		if ( c ) printf( "Error @WriteSector, %02X\n", c );
	}
	else {
		printf( "ReadSector 1# to buffer\n" );
		c = mReadSector( 1, 1 );
		if ( c ) printf( "Error @ReadSector, %02X\n", c );
		printf( "WriteSector 0# from buffer\n" );
		c = mWriteSector( 0, 1 );
		if ( c ) printf( "Error @WriteSector, %02X\n", c );
	}
	printf( "Stop\n" );
	while ( 1 ) {
		mIntStatus = mWaitInterrupt( );  /* �ȴ��жϲ���ȡ״̬ */
		if ( mIntStatus == USB_INT_DISCONNECT ) {  /* U��û�����ӻ����Ѿ��γ� */
			printf( "Out\n" );
			LED_OUT_INACT( );
		}
		else if ( mIntStatus == USB_INT_CONNECT ) {  /* U���Ѿ����� */
			printf( "In\n" );
			LED_OUT_ACT( );
		}
	}
}