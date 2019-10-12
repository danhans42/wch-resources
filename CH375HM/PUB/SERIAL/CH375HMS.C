/* 2004.06.05
****************************************
**  Copyright  (C)  W.ch  1999-2004   **
**  Web:  http://www.winchiphead.com  **
****************************************
**  USB Host File Module      @CH375  **
**  TC2.0@PC, KC7.0@MCS51             **
****************************************
*/
/* U���ļ���дģ��, ���ӷ�ʽ: ����+��ѯ */
/* MCS-51��Ƭ��C����ʾ������ */
/* ��Ϊʹ��U���ļ���дģ�������ʹ��U���ļ����ӳ����,����ռ�ý��ٵĵ�Ƭ����Դ,����ʹ��89C51��Ƭ������ */

#include <reg51.h>
#include <absacc.h>
#include <string.h>
#include <stdio.h>

#define MAX_PATH_LEN			32		/* ���·������,������б�ָܷ�����С���������Լ�·��������00H,CH375ģ��֧�ֵ����ֵ��64,��Сֵ��13 */
#include "..\CH375HM.H"

/* ��·���ӷ�ʽ
   ��Ƭ��    ģ��
    TXD   =  SIN
    RXD   =  SOUT
    P15   =  STA#
*/
sbit	P15					=	P1^5;
#define	CH375HM_STA				P15		/* �ٶ�CH375ģ���STA#�������ӵ���Ƭ����P15���� */

/* �ٶ��ļ����ݻ�����: ExtRAM: 0000H-7FFFH */
unsigned char xdata DATA_BUF[ 512 * 64 ] _at_ 0x0000;	/* �ⲿRAM���ļ����ݻ�����,�Ӹõ�Ԫ��ʼ�Ļ��������Ȳ�С��һ�ζ�д�����ݳ���,����Ϊ512�ֽ� */

unsigned char xdata *buffer;			/* ���ݻ�����ָ��,���ڶ�д���ݿ� */

CMD_PARAM		mCmdParam;				/* Ĭ������¸ýṹ��ռ��64�ֽڵ�RAM,�����޸�MAX_PATH_LEN����,���޸�Ϊ32ʱ,ֻռ��32�ֽڵ�RAM */

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
	CH375HM_STA = 0;  /* �����½���֪ͨģ��,˵�������뿪ʼ����,����ʼִ������ */
	CH375HM_STA = 0;  /* ������ʱ,�͵�ƽ���Ȳ�С��1uS */
	RI = 0;
	CH375HM_STA = 1;
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
		else if ( status == USB_INT_DISK_READ ) {  /* ���ڴ�U�̶����ݿ�,�������ݶ��� */
			i = 64;
			do {
				*buffer = mRecvByte( );  /* ���ν���64�ֽڵ����� */
				buffer ++;  /* ���յ����ݱ��浽�ⲿ������ */
			} while ( -- i );
		}
		else if ( status == USB_INT_DISK_WRITE ) {  /* ������U��д���ݿ�,��������д�� */
			i = 64;
			do {
				mSendByte( *buffer );  /* ���η���64�ֽڵ����� */
				buffer ++;  /* ���͵����������ⲿ������ */
			} while ( -- i );
		}
		else if ( status == USB_INT_DISK_RETRY ) {  /* ��д���ݿ�ʧ������,Ӧ������޸Ļ�����ָ�� */
			i = mRecvByte( );  /* ���ģʽ��Ϊ�ظ�ָ���ֽ����ĸ�8λ,�����С��ģʽ��ô���յ����ǻظ�ָ���ֽ����ĵ�8λ */
			status = mRecvByte( );  /* ���ģʽ��Ϊ�ظ�ָ���ֽ����ĵ�8λ,�����С��ģʽ��ô���յ����ǻظ�ָ���ֽ����ĸ�8λ */
			buffer -= ( (unsigned short)i << 8 ) + status;  /* ���Ǵ��ģʽ�µĻظ�ָ��,����С��ģʽ,Ӧ����( (unsigned short)status << 8 ) + i */
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

main( ) {
	unsigned char	i, c, SecCount;
	unsigned long	OldSize;
	unsigned short	NewSize, count;
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
/* ����4800bps����,����������������޸�Ϊ9600bps */
	mCmdParam.BaudRate.mDivisor = 18432000/32/9600;  /* �������: ͨѶ�����ʳ���,�ٶ�ģ��ľ���X2��Ƶ��Ϊ18.432MHz */
	i = ExecCommand( CMD_BaudRate, 1 );  /* ���ô���ͨѶ������ */
	mStopIfError( i );
	TH1 = 0xF3;  /* 24MHz����, ���������ڵ�ͨѶ�����ʵ�����9600bps */
	mDelaymS( 5 );  /* ��ʱ5����,ȷ��CH375ģ���л������趨��ͨѶ������ */
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
/* ���U���Ƿ�׼����,�����U�̲���Ҫ��һ��,����ĳЩU�̱���Ҫִ����һ�����ܹ��� */
		for ( i = 0; i < 5; i ++ ) {
			mDelaymS( 100 );
//			printf( "Ready ?\n" );
			if ( ExecCommand( CMD_DiskReady, 0 ) == ERR_SUCCESS ) break;  /* ��ѯ�����Ƿ�׼���� */
		}
/* ��ȡԭ�ļ� */
/*		printf( "Open\n" );*/
		memcpy( mCmdParam.Open.mPathName, "\\C51\\CH375HFT.C", MAX_PATH_LEN );  /* �ļ���,���ļ���C51��Ŀ¼�� */
		i = ExecCommand( CMD_FileOpen, MAX_PATH_LEN );  /* ���ļ�,���������Ϊ���ֵ,ʡ���ټ���������� */
		if ( i == ERR_MISS_DIR || i == ERR_MISS_FILE ) {  /* ERR_MISS_DIR˵��û���ҵ�C51��Ŀ¼,ERR_MISS_FILE˵��û���ҵ��ļ� */
/* �г���Ŀ¼�µ��ļ� */
/*			printf( "List file \\*\n" );*/
			for ( c = 0; c < 255; c ++ ) {  /* �������ǰ255���ļ� */
/*				memcpy( mCmdParam.Enumer.mPathName, "\\C51\\CH375*", MAX_PATH_LEN );*/  /* ����C51��Ŀ¼����CH375��ͷ���ļ���,*Ϊͨ��� */
				memcpy( mCmdParam.Enumer.mPathName, "\\*", MAX_PATH_LEN );  /* �����ļ���,*Ϊͨ���,�����������ļ�������Ŀ¼ */
/*				i = strlen( mCmdParam.Enumer.mPathName );*/  /* �����ļ����ĳ��� */
				for ( i = 0; i < MAX_PATH_LEN - 1; i ++ ) if ( mCmdParam.Enumer.mPathName[ i ] == 0 ) break;  /* ָ�������ļ����Ľ����� */
				mCmdParam.Enumer.mPathName[ i ] = c;  /* ���������滻Ϊ���������,��0��255 */
				i = ExecCommand( CMD_FileEnumer, i+1 );  /* ö���ļ�,����ļ����к���ͨ���*,��Ϊ�����ļ�������,��������ĳ��Ⱥܺü��� */
				if ( i == ERR_MISS_FILE ) break;  /* ��Ҳ��������ƥ����ļ�,�Ѿ�û��ƥ����ļ��� */
				if ( i == ERR_SUCCESS || i == ERR_FOUND_NAME ) {  /* ��������ͨ�����ƥ����ļ���,�ļ�����������·������������� */
/*					printf( "  match file %03d#: %s\n", (unsigned int)c, mCmdParam.Enumer.mPathName );*/  /* ��ʾ��ź���������ƥ���ļ���������Ŀ¼�� */
					continue;  /* ����������һ��ƥ����ļ���,�´�����ʱ��Ż��1 */
				}
				else {  /* ���� */
					mStopIfError( i );
					break;
				}
			}
			strcpy( DATA_BUF, "Note: \nԭ���Ǵ��㽫/C51/CH375HFT.C�ļ��е�Сд��ĸת�ɴ�д��д���µ��ļ�,�����Ҳ�������ļ�\n" );
			OldSize = 0;
			NewSize = strlen( DATA_BUF );  /* ���ļ��ĳ��� */
			SecCount = ( NewSize + 511 ) >> 9;  /* (NewSize+511)/512, �����ļ���������,��Ϊ��д��������Ϊ��λ�� */
		}
		else {  /* �ҵ��ļ�\C51\CH375HFT.C���߳��� */
			mStopIfError( i );
/*			printf( "Query\n" );*/
			i = ExecCommand( CMD_FileQuery, 0 );  /* ��ѯ��ǰ�ļ�����Ϣ,û��������� */
			mStopIfError( i );
/*			printf( "Read\n" );*/
			OldSize = mCmdParam.Modify.mFileSize;  /* ԭ�ļ��ĳ��� */
			if ( OldSize > (unsigned long)(64*512) ) {  /* ��ʾ���õ�62256ֻ��32K�ֽ� */
				SecCount = 64;  /* ������ʾ���õ�62256ֻ��32K�ֽ�,����ֻ��ȡ������64������,Ҳ���ǲ�����32768�ֽ� */
				NewSize = 64*512;  /* ����RAM�����������Ƴ��� */
			}
			else {  /* ���ԭ�ļ���С,��ôʹ��ԭ���� */
				SecCount = ( OldSize + 511 ) >> 9;  /* (OldSize+511)/512, �����ļ���������,��Ϊ��д��������Ϊ��λ�� */
				NewSize = (unsigned short)OldSize;  /* ԭ���� */
			}
/*			printf( "Size=%ld, Len=%d, Sec=%d\n", OldSize, NewSize, (unsigned short)SecCount );*/
			mCmdParam.Read.mSectorCount = SecCount;  /* ��ȡȫ������,�������60��������ֻ��ȡ60������ */
			buffer = & DATA_BUF;  /* ������ݵĻ���������ʼ��ַ,��CH375ģ���жϷ��������������� */
			i = ExecCommand( CMD_FileRead, 1 );  /* ���ļ���ȡ���� */
			mStopIfError( i );
/*
			����ļ��Ƚϴ�,һ�ζ�����,������������CMD_FileRead������ȡ,�ļ�ָ���Զ�����ƶ�
			while ( ʣ��δ���� ) {
				mCmdParam.Read.mSectorCount = 32;
				ExecCommand( CMD_FileRead, 1 );   ������ļ�ָ���Զ�����
				TotalLength += 32*512;  �ۼ��ļ��ܳ���
			}

		    ���ϣ����ָ��λ�ÿ�ʼ��д,�����ƶ��ļ�ָ��
			mCmdParam.Locate.mSectorOffset = 3;  �����ļ���ǰ3��������ʼ��д
			i = ExecCommand( CMD_FileLocate, 4 );  ��������ĳ���4��sizeof( mCmdParam.Locate.mSectorOffset )
			mCmdParam.Read.mSectorCount = 10;
			ExecCommand( CMD_FileRead, 1 );   ֱ�Ӷ�ȡ���ļ��ĵ�(512*3)���ֽڿ�ʼ������,ǰ3������������

			���ϣ�������������ӵ�ԭ�ļ���β��,�����ƶ��ļ�ָ��
			i = ExecCommand( CMD_FileOpen, (unsigned char)( strlen( mCmdParam.Open.mPathName ) + 1 ) );
			mCmdParam.Locate.mSectorOffset = 0xffffffff;  �Ƶ��ļ���β��,������Ϊ��λ,���ԭ�ļ���3�ֽ�,���512�ֽڿ�ʼ����
			i = ExecCommand( CMD_FileLocate, sizeof( mCmdParam.Locate.mSectorOffset ) );
			mCmdParam.Write.mSectorCount = 10;
			ExecCommand( CMD_FileWrite, 1 );   ��ԭ�ļ��ĺ�����������
*/
/*			printf( "Close\n" );*/
			mCmdParam.Close.mUpdateLen = 0;
			i = ExecCommand( CMD_FileClose, 1 );  /* �ر��ļ� */
			mStopIfError( i );

/*			i = DATA_BUF[200];*/
/*			DATA_BUF[200] = 0;  ���ַ���������־,�����ʾ200���ַ� */
/*			printf( "Line 1: %s\n", DATA_BUF );*/
/*			DATA_BUF[200] = i;  �ָ�ԭ�ַ� */
			for ( count=0; count < NewSize; count ++ ) {  /* ���ļ��е�Сд�ַ�ת��Ϊ��д */
				c = DATA_BUF[ count ];
				if ( c >= 'a' && c <= 'z' ) DATA_BUF[ count ] = c - ( 'a' - 'A' );
			}
		}
/* �������ļ� */
/*		printf( "Create\n" );*/
/*		memcpy( mCmdParam.Create.mPathName, "\\NEWFILE.TXT", MAX_PATH_LEN );*/
		memcpy( mCmdParam.Create.mPathName, "\\˫���Ұ�.TXT", MAX_PATH_LEN );  /* ���ļ���,�ڸ�Ŀ¼�� */
		i = ExecCommand( CMD_FileCreate, MAX_PATH_LEN );  /* �½��ļ�����,����ļ��Ѿ���������ɾ�������½� */
		mStopIfError( i );
/*		printf( "Write\n" );*/
		mCmdParam.Write.mSectorCount = 0x1;  /* д��һ������512�ֽ� */
		buffer = & DATA_BUF;  /* ������ݵĻ���������ʼ��ַ,��CH375ģ���жϷ��������д������ */
		i = ExecCommand( CMD_FileWrite, 1 );  /* ���ļ�д������ */
		mStopIfError( i );
		if ( SecCount > 1 ) {  /* ��Ϊ���ݲ�����255������,��������ܹ�һ��д��,����Ϊ����ʾ,���������д�� */
			mCmdParam.Write.mSectorCount = SecCount - 1;
/*	buffer = & DATA_BUF + 512;  ���Ÿղŵ�д,�������û���������ʼ��ַ */
			i = ExecCommand( CMD_FileWrite, 1 );  /* ���ļ�д������ */
			mStopIfError( i );
		}
/*		printf( "Modify\n" );*/
		mCmdParam.Modify.mFileAttr = 0xff;  /* �������: �µ��ļ�����,Ϊ0FFH���޸� */
		mCmdParam.Modify.mFileTime = 0xffff;  /* �������: �µ��ļ�ʱ��,Ϊ0FFFFH���޸�,ʹ���½��ļ�������Ĭ��ʱ�� */
		mCmdParam.Modify.mFileDate = ( (2004-1980)<<9 ) + ( 5<<5 ) + 18;  /* �������: �µ��ļ�����: 2004.05.18 */
		mCmdParam.Modify.mFileSize = NewSize;  /* �������: ���ԭ�ļ���С,��ô�µ��ļ�������ԭ�ļ�һ����,����RAM���� */
		i = ExecCommand( CMD_FileModify, 4+2+2+1 );  /* �޸ĵ�ǰ�ļ�����Ϣ,�޸����ںͳ���,��������Ϊsizeof(mCmdParam.Modify.mFileSize)+... */
		mStopIfError( i );
/*		printf( "Close\n" );*/
		mCmdParam.Close.mUpdateLen = 0;  /* ��Ҫ�Զ������ļ�����,����Զ�����,��ô�ó�������512�ı��� */
		i = ExecCommand( CMD_FileClose, 1 );
		mStopIfError( i );

/* ɾ��ĳ�ļ� */
/*		printf( "Erase\n" );*/
		memcpy( mCmdParam.Create.mPathName, "\\OLD", MAX_PATH_LEN );  /* ����ɾ�����ļ���,�ڸ�Ŀ¼�� */
		i = ExecCommand( CMD_FileErase, MAX_PATH_LEN );  /* ɾ���ļ����ر� */
/*		mStopIfError( i );*/

/* ��ѯ������Ϣ */
/*		printf( "Disk\n" );
		i = ExecCommand( CMD_DiskQuery, 0 );
		mStopIfError( i );
		i = mCmdParam.Query.mDiskFat;
		if ( i == 1 ) i = 12;
		else if ( i == 2 ) i = 16;
		else if ( i == 3 ) i = 32;
		printf( "FatCode=FAT%d, TotalSector=%ld, FreeSector=%ld\n", (unsigned short)i, mCmdParam.Query.mTotalSector, mCmdParam.Query.mFreeSector );*/
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