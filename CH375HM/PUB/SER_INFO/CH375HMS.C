/* 2004.06.05
****************************************
**  Copyright  (C)  W.ch  1999-2004   **
**  Web:  http://www.winchiphead.com  **
****************************************
**  USB Host File Module      @CH375  **
**  TC2.0@PC, KC7.0@MCS51             **
****************************************
*/
/* U���ļ���дģ��, ���ӷ�ʽ: 3���ƴ���+��ѯ */
/* MCS-51��Ƭ��C����ʾ������ */
/* ��Ϊʹ��U���ļ���дģ�������ʹ��U���ļ����ӳ����,����ռ�ý��ٵĵ�Ƭ����Դ,����ʹ��89C51��Ƭ������ */
/* ������������ʾ�����ļ�Ŀ¼��,����:�޸��ļ���,�����ļ��Ĵ������ں�ʱ��� */

#include <reg51.h>
#include <absacc.h>
#include <string.h>
#include <stdio.h>

#define MAX_PATH_LEN			40		/* ���·������,������б�ָܷ�����С���������Լ�·��������00H,CH375ģ��֧�ֵ����ֵ��62,��Сֵ��13 */
/* Ϊ�˴����ļ�Ŀ¼��,MAX_PATH_LEN����Ϊ36,sizeof( mCmdParam.FileDirInfo ) */
#include "..\CH375HM.H"

/* ��·���ӷ�ʽ,ֻ��Ҫ����3����,ʹ�ô���ͬ������������
   ��Ƭ��    ģ��
    TXD   =  SIN
    RXD   =  SOUT
             STA# ���ջ�Ӹߵ�ƽ
             INT# �ӵػ�ӵ͵�ƽ
    GND   =  GND
*/
sbit	P15					=	P1^5;

CMD_PARAM	idata	mCmdParam;			/* Ĭ������¸ýṹ��ռ��60�ֽڵ�RAM,�����޸�MAX_PATH_LEN����,���޸�Ϊ32ʱ,ֻռ��32�ֽڵ�RAM */

sbit	LED_OUT		=	P1^4;			/* P1.4 �͵�ƽ����LED��ʾ,���ڼ����ʾ����Ľ��� */

/* �Ժ���Ϊ��λ��ʱ,������24MHzʱ�� */
void	mDelaymS( unsigned char delay )
{
	unsigned char	i, j, c;
	for ( i = delay; i != 0; i -- ) {
		for ( j = 200; j != 0; j -- ) c += 3;  /* ��24MHzʱ������ʱ500uS */
		for ( j = 200; j != 0; j -- ) c += 3;  /* ��24MHzʱ������ʱ500uS */
	}
}

/* ����һ���ֽ����ݸ�CH375ģ�� */
void	mSendByte( unsigned char c )
{
	TI = 0;
	SBUF = c;
	while ( TI == 0 );
}

/* ��CH375ģ�����һ���ֽ����� */
unsigned char	mRecvByte( )
{
	unsigned char	c;
	while ( RI == 0 );
	c = SBUF;
	RI = 0;
	return( c );
}

/* ִ������ */
unsigned char	ExecCommand( unsigned char cmd, unsigned char len )
/* ����������������������,���ز���״̬��,��������ͷ��ز�������CMD_PARAM�ṹ�� */
{
	unsigned char		i, j, status;
	mSendByte( SER_SYNC_CODE1 );  /* ���ʹ���ͬ����֪ͨģ��,˵�������뿪ʼ����,����ʼִ������ */
	mSendByte( SER_SYNC_CODE2 );  /* ����������ͬ�������STA#���½��� */
/* ������������ͬ����Ӧ����������,���������,��ô���ʱ�䲻�ܳ���20mS,����������Ч */
	RI = 0;
	mSendByte( cmd );  /* д�������� */
	mSendByte( len );  /* д����������ĳ��� */
	if ( len ) {  /* �в��� */
		for ( i = 0; i != len; i ++ ) mSendByte( mCmdParam.Other.mBuffer[ i ] );  /* ����д����� */
	}
	while ( 1 ) {  /* �������ݴ���,ֱ��������ɲ��˳� */
		status = mRecvByte( );  /* �ȴ�ģ����ɲ��������ز���״̬ */
		if ( status == ERR_SUCCESS ) {  /* �����ɹ� */
			i = mRecvByte( );  /* ���ؽ�����ݵĳ��� */
			if ( i ) {  /* �н������ */
				j = 0;
				do {  /* ʹ��do+while�ṹ����Ϊ��Ч�ʸ���for */
					mCmdParam.Other.mBuffer[ j ] = mRecvByte( );  /* ���ս�����ݲ����浽�����ṹ�� */
					j ++;
				} while ( -- i );
			}
			break;  /* �����ɹ����� */
		}
		else if ( status == USB_INT_DISK_READ || status == USB_INT_DISK_WRITE || status == USB_INT_DISK_RETRY ) {  /* ���ڴ�U�̶����ݿ�,�������ݶ���,������U��д���ݿ�,��������д��,��д���ݿ�ʧ������ */
			break;  /* ������ֻʹ�����ֽ�Ϊ��λ���ļ���д�ӳ���,������������²����յ���״̬��,����ʧ�ܷ��� */
		}
		else {  /* ����ʧ�� */
			if ( status == ERR_DISK_DISCON || status == ERR_USB_CONNECT ) mDelaymS( 100 );  /* U�̸ո����ӻ��߶Ͽ�,Ӧ����ʱ��ʮ�����ٲ��� */
			break;  /* ����ʧ�ܷ��� */
		}
	}
	return( status );
}

/* ������״̬,�����������ʾ������벢ͣ��,Ӧ���滻Ϊʵ�ʵĴ�����ʩ */
void	mStopIfError( unsigned char iError )
{
	unsigned char	led;
	if ( iError == ERR_SUCCESS ) return;  /* �����ɹ� */
/*	printf( "Error: %02X\n", (unsigned short)iError );*/  /* ��ʾ���� */
	led=0;
	while ( 1 ) {
		LED_OUT = led&1;  /* LED��˸ */
		mDelaymS( 100 );
		led^=1;
	}
}

/* �����С�˸�ʽ�����ݴ��� */
unsigned short	SwapUINT16( unsigned short d )
{
	return( ( d << 8 ) & 0xFF00 | ( d >> 8 ) & 0xFF );
}

main( ) {
	unsigned char	i;
	unsigned short	FileCreateDate, FileCreateTime;
	unsigned char	*name;
	LED_OUT = 0;  /* ������LED��һ����ʾ���� */
	mDelaymS( 100 );  /* ��ʱ100����,CH375ģ���ϵ����Ҫ100�������ҵĸ�λʱ�� */
	mDelaymS( 100 );
	LED_OUT = 1;
/* ������CH375ģ��ͨѶ�Ĵ��� */
	SCON = 0x50;
	PCON = 0x80;
	TMOD = 0x20;
	TH1 = 0xE6;  /* 24MHz����, 4800bps */
	TR1 = 1;

/*	printf( "Start\n" );*/
	while ( 1 ) {  /* ��ѭ�� */
/*		printf( "Wait\n" );*/
		while ( 1 ) {  /* ʹ�ò�ѯ��ʽ��U���Ƿ����� */
			i = ExecCommand( CMD_QueryStatus, 0 );  /* ��ѯ��ǰģ���״̬ */
			mStopIfError( i );
			if ( mCmdParam.Status.mDiskStatus >= DISK_CONNECT ) break;  /* U���Ѿ����� */
			mDelaymS( 100 );  /* �����ڴ����дU��ʱ�ٲ�ѯ,û�б�Ҫһֱ������ͣ�ز�ѯ,�����õ�Ƭ����������,û�¿�������ʱ�ȴ�һ���ٲ�ѯ */
		}
		mDelaymS( 200 );  /* ��ʱ,��ѡ����,�е�USB�洢����Ҫ��ʮ�������ʱ */
		LED_OUT = 0;  /* LED�� */
/* ���U���Ƿ�׼����,ĳЩU�̱���Ҫִ����һ�����ܹ��� */
		for ( i = 0; i < 3; i ++ ) {
			mDelaymS( 100 );
//			printf( "Ready ?\n" );
			if ( ExecCommand( CMD_DiskReady, 0 ) == ERR_SUCCESS ) break;  /* ��ѯ�����Ƿ�׼���� */
		}
/* ��MY_ADC.TXT�ļ����޸�ΪWY_ADC.C,�����ô����ļ������ں�ʱ��,���ȴ�ԭ�ļ� */
		name = "/MY_ADC.TXT";  /* �ļ���,б��˵���ǴӸ�Ŀ¼��ʼ */
/*		printf( "Open\n" );*/
		strcpy( mCmdParam.Open.mPathName, name );  /* ԭ�ļ��� */
		i = ExecCommand( CMD_FileOpen, MAX_PATH_LEN );  /* ���ļ�,���������Ϊ���ֵ,ʡ���ټ���������� */
		if ( i == ERR_MISS_FILE ) mStopIfError( i );  /* �ļ�������,��Ȼ�޷��޸��ļ�Ŀ¼��Ϣ */
		mStopIfError( i );
		/* �ļ���д������... */

/*		printf( "Get file directory information\n" );*/
		mCmdParam.FileDirInfo.mAccessMode = 0;  /* ��ȡ�ļ�Ŀ¼��Ϣ */
		mCmdParam.FileDirInfo.mReserved[0] = mCmdParam.FileDirInfo.mReserved[1] = mCmdParam.FileDirInfo.mReserved[2] = 0;  /* ������Ԫ */
		i = ExecCommand( CMD_FileDirInfo, 4 );  /* ��ȡ��ǰ�Ѵ��ļ���Ŀ¼��Ϣ */
		mStopIfError( i );

/* �����޸��ļ�Ŀ¼��Ϣ�е��ļ��� */
		mCmdParam.FileDirInfo.mDir.DIR_Name[0] = 'W';  /* �޸��ļ������ֽ�ΪW */
		mCmdParam.FileDirInfo.mDir.DIR_Name[8] = 'C';  /* �޸��ļ���չ��ΪC */
		mCmdParam.FileDirInfo.mDir.DIR_Name[9] = ' ';
		mCmdParam.FileDirInfo.mDir.DIR_Name[10] = ' ';

/* �����޸��ļ�Ŀ¼��Ϣ�е��ļ�����ʱ��,DIR_CrtTime�Ǵ���ʱ��,DIR_WrtTime���޸�ʱ�� */
		FileCreateTime = MAKE_FILE_TIME( 16, 49, 28 );  /* �����ļ�����ʱ����16ʱ49��28�� */
//		mCmdParam.FileDirInfo.mDir.DIR_CrtTime = FileCreateTime;  /* �ļ�������ʱ��,������С�˸�ʽ */
		mCmdParam.FileDirInfo.mDir.DIR_CrtTime = SwapUINT16( FileCreateTime );  /* MCS51��Ƭ��C�����Ǵ�˸�ʽ,���Ա���ת������� */
		FileCreateDate = MAKE_FILE_DATE( 2004, 12, 8 );  /* �����ļ�����������2004��12��8�� */
//		mCmdParam.FileDirInfo.mDir.DIR_CrtDate = FileCreateDate;  /* �ļ�����������,������С�˸�ʽ */
		mCmdParam.FileDirInfo.mDir.DIR_CrtDate = SwapUINT16( FileCreateDate );  /* MCS51��Ƭ��C�����Ǵ�˸�ʽ,���Ա���ת������� */

//		mCmdParam.FileDirInfo.mDir.DIR_WrtTime = SwapUINT16( MAKE_FILE_TIME( ʱ, ��, �� ) );  /* �ļ��޸�ʱ�� */
//		mCmdParam.FileDirInfo.mDir.DIR_LstAccDate = SwapUINT16( MAKE_FILE_DATE( ��, ��, �� ) );  /* ���һ�δ�ȡ���������� */

/* ���½��޸Ĺ�����������ˢ�µ�U���� */
/*		printf( "Save new file directory information\n" );*/
		mCmdParam.FileDirInfo.mAccessMode = 0xF0;  /* д��/�����ļ�Ŀ¼��Ϣ */
		i = ExecCommand( CMD_FileDirInfo, sizeof( mCmdParam.FileDirInfo ) );  /* ��ȡ��ǰ�Ѵ��ļ���Ŀ¼��Ϣ */
		mStopIfError( i );

		/* �ļ���д������... */
/*		printf( "Close\n" );*/
		mCmdParam.Close.mUpdateLen = 0;
		i = ExecCommand( CMD_FileClose, 1 );  /* �ر��ļ� */
		mStopIfError( i );

/* �ȴ�U�̶Ͽ� */
/*		printf( "Take_out\n" );*/
		while ( 1 ) {  /* ʹ�ò�ѯ��ʽ��U���Ƿ�Ͽ� */
			i = ExecCommand( CMD_QueryStatus, 0 );  /* ��ѯ��ǰģ���״̬ */
			mStopIfError( i );
			if ( mCmdParam.Status.mDiskStatus <= DISK_DISCONNECT ) break;  /* U���Ѿ��Ͽ� */
			mDelaymS( 100 );  /* û�б�Ҫһֱ������ͣ�ز�ѯ,�����õ�Ƭ����������,û�¿�������ʱ�ȴ�һ���ٲ�ѯ */
		}
		LED_OUT = 1;  /* LED�� */
	}
}