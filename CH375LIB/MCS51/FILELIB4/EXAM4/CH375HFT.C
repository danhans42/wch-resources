/* 2004.06.05
****************************************
**  Copyright  (C)  W.ch  1999-2004   **
**  Web:  http://www.winchiphead.com  **
****************************************
**  USB Host File Interface for CH375 **
**  TC2.0@PC, KC7.0@MCS51             **
****************************************
*/
/* CH375 �����ļ�ϵͳ�ӿ� */
/* ֧��: FAT12/FAT16/FAT32 */

/* MCS-51��Ƭ��C���Ե�U���ļ���дʾ������, ������ATMEL/PHILIPS/SST�Ⱦ���1KB�ڲ�RAM�Լ�˫DPTR�ĵ�Ƭ�� */
/* �ó���U���е�/C51/CH375HFT.C�ļ��е�ÿ64�ֽڵ�ǰ4���ַ���ʾ����(��ʾ4�ַ�,����60�ַ�,��ʾ4�ַ�...),
   ����Ҳ���ԭ�ļ�CH375HFT.C, ��ô�ó�����ʾC51��Ŀ¼��������CH375��ͷ���ļ���,
   ����Ҳ���C51��Ŀ¼, ��ô�ó�����ʾ��Ŀ¼�µ������ļ���,
   ����½��ļ�"����ȫ0.TXT",�ļ�����ȫ�����ַ�0,�����ADC,������ADC�����������ַ�0
*/
/* CH375��INT#���Ų��ò�ѯ��ʽ����, �������ݸ��Ʒ�ʽΪ"˫DPTR����", �ļ����ݸ��Ʒ�ʽΪ"�ⲿ�ӳ���",
   ����ʹ��512�ֽڵ��ⲿRAM��Ϊ�������ݻ�����, ��û���ļ����ݻ�����, ��ʾû���ⲿRAM���ǵ�Ƭ�����ڲ�RAM����1K�ֽڵ�Ӧ�� */


/* C51   CH375HFT.C */
/* LX51  CH375HFT.OBJ , CH375HF4.LIB    �����CH375HF4����CH375HF6�Ϳ���֧��FAT32 */
/* OHX51 CH375HFT */

#include <reg52.h>
#include <stdio.h>

/* ���¶������ϸ˵���뿴CH375HF6.H�ļ� */
#define LIB_CFG_DISK_IO			2		/* ���̶�д�����ݵĸ��Ʒ�ʽ,1Ϊ"��DPTR����",2Ϊ"˫DPTR����",3Ϊ"��DPTR��P2+R0����" */
#define LIB_CFG_FILE_IO			0		/* �ļ���д�����ݵĸ��Ʒ�ʽ,0Ϊ"�ⲿ�ӳ���",1Ϊ"��DPTR����",2Ϊ"˫DPTR����",3Ϊ"��DPTR��P2+R0����" */
#define LIB_CFG_INT_EN			0		/* CH375��INT#�������ӷ�ʽ,0Ϊ"��ѯ��ʽ",1Ϊ"�жϷ�ʽ" */
/*#define LIB_CFG_FILE_IO_DEFAULT	1*/		/* ʹ��CH375HF6.H�ṩ��Ĭ��"�ⲿ�ӳ���" */

#define CH375_CMD_PORT_ADDR		0xBDF1	/* CH375����˿ڵ�I/O��ַ */
#define CH375_DAT_PORT_ADDR		0xBCF0	/* CH375���ݶ˿ڵ�I/O��ַ */
/* ֻʹ�õ�Ƭ�����õ�1KB�ⲿRAM: 0000H-01FFH Ϊ���̶�д������, û���ļ����ݻ����� */
#define	DISK_BASE_BUF_ADDR		0x0000	/* �ⲿRAM�Ĵ������ݻ���������ʼ��ַ,�Ӹõ�Ԫ��ʼ�Ļ���������ΪSECTOR_SIZE */
/* ���ڵ�Ƭ�����õ��ⲿRAMֻ��1KB, ��Щ��Ƭ����Ҫȥ��256�ֽ��ڲ�RAM, ֻʣ��768�ֽڵ��ⲿRAM,
   ����ǰ512�ֽ���CH375�ӳ������ڴ������ݻ���, ����"�ⲿ�ӳ���"�����ļ����ݸ���, ����û���ļ����ݻ����� */
#define FILE_DATA_BUF_LEN		0x0200	/* �ⲿRAM���ļ����ݻ�����,���������Ȳ�С��һ�ζ�д�����ݳ���,ʹ��"�ⲿ�ӳ���"ʱ�ö���û������ */
/* ���׼��ʹ��˫�����������д,��ô��Ҫ����FILE_DATA_BUF_LEN,�����ڲ�����ָ����������ַ,��CH375FileReadX����CH375FileRead,��CH375FileWriteX����CH375FileWrite */

#define CH375_INT_WIRE			INT0	/* P3.2, INT0, CH375���ж���INT#����,����CH375��INT#����,���ڲ�ѯ�ж�״̬ */

#define NO_DEFAULT_CH375_F_ENUM		1		/* δ����CH375FileEnumer����ʽ�ֹ�Խ�Լ���� */
#define NO_DEFAULT_CH375_F_QUERY	1		/* δ����CH375FileQuery����ʽ�ֹ�Խ�Լ���� */

#include "..\CH375HF6.H"				/* �������Ҫ֧��FAT32,��ô��ѡ��CH375HF4.H */

UINT8X	mFileReadBuffer[ 64 ]	_at_ 0x0200;	/* ����CH375���ļ���һ�����ݴ���,����ΪCH375_MAX_DATA_LEN */

void xWriteToExtBuf( UINT8 mLength )	/* ���ӳ�����CH375���ӳ�������,���ڴ�CH375��ȡ�ļ����ݵ��ⲿ������,��CH375FileRead���� */
{
	UINT8	i;
	PUINT8X	p;
	if ( mLength == 64 ) {
		p = mFileReadBuffer;
		do {  /* ���ݳ��ȶ�ȡ����,ʵ���ϳ�������CH375_MAX_DATA_LEN,Ҳ����64 */
			*p = CH375_DAT_PORT;  /* ��U�̶��������ݴ浽�ڲ�RAM������,���������ַ�ʽ���ļ����ݱ��浽��Ƭ���ĸ��ִ��д洢���� */
			p ++;
		} while ( -- mLength );  /* �����������ݵ���ʱ�䲻�ó���2mS */
/*	PutDataToDAC( mFileReadBuffer );  ��mFileReadBuffer�������е����������DAC */
		for ( i = 0; i != 4; i ++ ) { while( TI == 0 ); TI = 0; SBUF = mFileReadBuffer[ i ]; }  /* ��ʾǰ4���ַ� */
	}
	else if ( mLength == 0 ) {  /* ����,�ָ���������ַ,������ļ����ݶ�д�Ļ������ĵ�ǰָ�����mCmdParam.ReadX.mDataBuffer����ᱻ�Զ��ָ�,������������г��� */
//		(UINT16)mCmdParam.ReadX.mDataBuffer;  /* mDataBuffer��Ϊ��ֵ,���Ե��ֽ��� */
	}
}

UINT8 GetDataFromADC( )  /* û��ADC,��������ģ�ⷵ������30H */
{
/*	while ( RI == 0 ); RI = 0; return( SBUF );  ͨ�����ڻ�����ݻ��ߴӸ��ִ��д洢����ȡ������ */
	return( '0' );
}

void xReadFromExtBuf( UINT8 mLength )	/* ���ӳ�����CH375���ӳ�������,���ڴ��ⲿ��������ȡ�ļ����ݵ�CH375,��CH375FileWrite���� */
{
	if ( mLength ) {
		while ( mLength ) {  /* ���ݳ���д������,ʵ���ϳ�������CH375_MAX_DATA_LEN,Ҳ����64 */
			CH375_DAT_PORT = GetDataFromADC( );  /* ��ADC��ȡһ�ֽ�����д��U��,���������ַ�ʽ�ӵ�Ƭ���ĸ��ִ��д洢����ȡ���ļ����� */
			mLength --;
		}  /* �����������ݵ���ʱ�䲻�ó���2mS,�������2mS��ο��������� */
	}
	else {  /* ����,�ָ���������ַ,������ļ����ݶ�д�Ļ������ĵ�ǰָ�����mCmdParam.WriteX.mDataBuffer����ᱻ�Զ��ָ�,������������г��� */
//		(UINT16)mCmdParam.WriteX.mDataBuffer;  /* mDataBuffer��Ϊ��ֵ,���Ե��ֽ��� */
	}
}

#if	0	/* ���xReadFromExtBuf�ӳ���ִ����ʱ�䳬��1mS,��ôӦ����������������CH375��ʱ��ȴ����� */
UINT8X	mFileWriteBuffer[ 64 ]	_at_ 0x0240;	/* ����CH375д�ļ���һ�����ݴ���,����ΪCH375_MAX_DATA_LEN */
void xReadFromExtBuf( UINT8 mLength )	/* ���ӳ�����CH375���ӳ�������,���ڴ��ⲿ��������ȡ�ļ����ݵ�CH375,��CH375FileWrite���� */
{
	UINT8	i;
	while ( mLength ) CH375_DAT_PORT = mLength --;  /* ����CH375�����������2mS֮�����,������д�����������ݽ���ǰһ������ */
	for ( i = 0; i != 64; i ++ ) mFileWriteBuffer[ i ] = GetDataFromADC( );  /* ��ADC��������ݴ浽��ʱ������, û��ʱ������ */
	CH375_CMD_PORT = CMD_WR_USB_DATA7;  /* ���·���д��������,��CH375�������˵㷢�ͻ�����д�����ݿ� */
	if ( CH375IntStatus ) CH375IntStatus = 0;  /* ������ʱ,������2uS */
	CH375_DAT_PORT = 64;  /* �������ݵĳ���,����дU�̱�����64 */
	for ( i = 0; i != 64; i ++ ) CH375_DAT_PORT = mFileWriteBuffer[ i ];  /* ����ʱ�������е�����д��CH375,����ǰ��д������������� */
}
#endif

/* ��P1.4����һ��LED���ڼ����ʾ����Ľ���,�͵�ƽLED��,��U�̲������ */
sbit P1_4  = P1^4;
#define LED_OUT_INIT( )		{ P1_4 = 1; }	/* P1.4 �ߵ�ƽ */
#define LED_OUT_ACT( )		{ P1_4 = 0; }	/* P1.4 �͵�ƽ����LED��ʾ */
#define LED_OUT_INACT( )	{ P1_4 = 1; }	/* P1.4 �͵�ƽ����LED��ʾ */
sbit P1_5  = P1^5;
/* ��P1.5����һ��LED���ڼ����ʾ����Ľ���,�͵�ƽLED��,����U�̲���ʱ�� */
#define LED_RUN_ACT( )		{ P1_5 = 0; }	/* P1.5 �͵�ƽ����LED��ʾ */
#define LED_RUN_INACT( )	{ P1_5 = 1; }	/* P1.5 �͵�ƽ����LED��ʾ */
sbit P1_6  = P1^6;
/* ��P1.6����һ��LED���ڼ����ʾ����Ľ���,�͵�ƽLED��,����U��д����ʱ�� */
#define LED_WR_ACT( )		{ P1_6 = 0; }	/* P1.6 �͵�ƽ����LED��ʾ */
#define LED_WR_INACT( )		{ P1_6 = 1; }	/* P1.6 �͵�ƽ����LED��ʾ */

/* ��ʱ100����,����ȷ */
void	mDelay100mS( )
{
	UINT8	i, j, c;
	for ( i = 200; i != 0; i -- ) for ( j = 200; j != 0; j -- ) c+=3;
}

/* ������ռ���ַ������Ƶ��ڲ�RAM��,�����ַ������� */
UINT8	mCopyCodeStringToIRAM( UINT8 idata *iDestination, UINT8 code *iSource )
{
	UINT8	i = 0;
	while ( *iDestination = *iSource ) {
		iDestination ++;
		iSource ++;
		i ++;
	}
	return( i );
}

/* ������״̬,�����������ʾ������벢ͣ�� */
void	mStopIfError( UINT8 iError )
{
	if ( iError == ERR_SUCCESS ) return;  /* �����ɹ� */
	printf( "Error: %02X\n", (UINT16)iError );  /* ��ʾ���� */
	while ( 1 ) {
		LED_OUT_ACT( );  /* LED��˸ */
		mDelay100mS( );
		LED_OUT_INACT( );
		mDelay100mS( );
	}
}

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
	UINT8	i, c, SecCount;
	UINT16	NewSize;  /* ��Ϊ��ʾ���RAM����ֻ��32KB,����NewSize����Ϊ16λ,ʵ��������ļ�����32256�ֽ�,Ӧ�÷ּ��ζ�д���ҽ�NewSize��ΪUINT32�Ա��ۼ� */
	UINT8	code *pCodeStr;
	LED_OUT_INIT( );
	LED_OUT_ACT( );  /* ������LED��һ����ʾ���� */
	mDelay100mS( );  /* ��ʱ100���� */
	LED_OUT_INACT( );
	mInitSTDIO( );  /* Ϊ���ü����ͨ�����ڼ����ʾ���� */
	printf( "Start\n" );

#if DISK_BASE_BUF_LEN == 0
	pDISK_BASE_BUF = &my_buffer[0];  /* ����.H�ļ��ж���CH375��ר�û�����,�����û�����ָ��ָ������Ӧ�ó���Ļ��������ں����Խ�ԼRAM */
#endif

	i = CH375LibInit( );  /* ��ʼ��CH375������CH375оƬ,�����ɹ�����0 */
	mStopIfError( i );
/* ������·��ʼ�� */

	while ( 1 ) {
		printf( "Wait Udisk\n" );
//		while ( CH375DiskStatus != DISK_CONNECT ) xQueryInterrupt( );  /* ��ѯCH375�жϲ������ж�״̬,�ȴ�U�̲��� */
		while ( CH375DiskStatus < DISK_CONNECT ) {  /* ��ѯCH375�жϲ������ж�״̬,�ȴ�U�̲��� */
			if ( CH375DiskConnect( ) == ERR_SUCCESS ) break;  /* ���豸�����򷵻سɹ�,CH375DiskConnectͬʱ�����ȫ�ֱ���CH375DiskStatus */
			mDelay100mS( );
		}
		LED_OUT_ACT( );  /* LED�� */
		mDelay100mS( );  /* ��ʱ,��ѡ����,�е�USB�洢����Ҫ��ʮ�������ʱ */
		mDelay100mS( );

/* ���U���Ƿ�׼����,��ЩU�̲���Ҫ��һ��,����ĳЩU�̱���Ҫִ����һ�����ܹ��� */
		for ( i = 0; i < 5; i ++ ) {  /* �е�U�����Ƿ���δ׼����,�������Ա����� */
			mDelay100mS( );
			printf( "Ready ?\n" );
			if ( CH375DiskReady( ) == ERR_SUCCESS ) break;  /* ��ѯ�����Ƿ�׼���� */
		}
#if DISK_BASE_BUF_LEN
		if ( DISK_BASE_BUF_LEN < CH375vSectorSize ) {  /* ���������ݻ������Ƿ��㹻��,CH375vSectorSize��U�̵�ʵ��������С */
			printf( "Too large sector size\n" );
			while ( CH375DiskConnect( ) == ERR_SUCCESS ) mDelay100mS( );
			continue;
		}
#endif
/* ��ѯ������������ */
/*		printf( "DiskSize\n" );
		i = CH375DiskSize( );  
		mStopIfError( i );
		printf( "TotalSize = %u MB \n", (unsigned int)( mCmdParam.DiskSize.mDiskSizeSec * (CH375vSectorSize/512) / 2048 ) );  // ��ʾΪ��MBΪ��λ������
		// ԭ���㷽�� (unsigned int)( mCmdParam.DiskSize.mDiskSizeSec * CH375vSectorSize / 1000000 ) �п���ǰ����������˺������, �����޸ĳ���ʽ
*/
		LED_RUN_ACT( );  /* ��ʼ����U�� */

/* ��ȡԭ�ļ� */
		printf( "Open\n" );
		mCopyCodeStringToIRAM( mCmdParam.Open.mPathName, "\\C51\\CH375HFT.C" );  /* �ļ���,���ļ���C51��Ŀ¼�� */
		i = CH375FileOpen( );  /* ���ļ� */
		if ( i == ERR_MISS_DIR || i == ERR_MISS_FILE ) {  /* û���ҵ��ļ� */
/* �г��ļ� */
			if ( i == ERR_MISS_DIR ) pCodeStr = "\\*";  /* C51��Ŀ¼���������г���Ŀ¼�µ��ļ� */
			else pCodeStr = "\\C51\\CH375*";  /* CH375HFT.C�ļ����������г�\C51��Ŀ¼�µ���CH375��ͷ���ļ� */
			printf( "List file %s\n", pCodeStr );
			for ( c = 0; c < 255; c ++ ) {  /* �������ǰ255���ļ� */
				i = mCopyCodeStringToIRAM( mCmdParam.Open.mPathName, pCodeStr );  /* �����ļ���,*Ϊͨ���,�����������ļ�������Ŀ¼ */
				mCmdParam.Open.mPathName[ i ] = c;  /* �����ַ������Ƚ��������滻Ϊ���������,��0��255 */
				i = CH375FileOpen( );  /* ���ļ�,����ļ����к���ͨ���*,��Ϊ�����ļ������� */
				if ( i == ERR_MISS_FILE ) break;  /* ��Ҳ��������ƥ����ļ�,�Ѿ�û��ƥ����ļ��� */
				if ( i == ERR_FOUND_NAME ) {  /* ��������ͨ�����ƥ����ļ���,�ļ�����������·������������� */
					printf( "  match file %03d#: %s\n", (unsigned int)c, mCmdParam.Open.mPathName );  /* ��ʾ��ź���������ƥ���ļ���������Ŀ¼�� */
					continue;  /* ����������һ��ƥ����ļ���,�´�����ʱ��Ż��1 */
				}
				else {  /* ���� */
					mStopIfError( i );
					break;
				}
			}
			NewSize = CH375vSectorSize;  /* ���ļ��ĳ��� */
			SecCount = 1;  /* (NewSize+CH375vSectorSize-1)/CH375vSectorSize, �����ļ���������,��Ϊ��д��������Ϊ��λ�� */
		}
		else {  /* �ҵ��ļ����߳��� */
			mStopIfError( i );
/*			printf( "Query\n" );
			i = CH375FileQuery( );  ��ѯ��ǰ�ļ�����Ϣ
			mStopIfError( i );*/
			printf( "Read\n" );
			if ( CH375vFileSize > CH375vSectorSize * 100  ) {  /* ��Ϊÿ����64�ֽھͱ�xWriteToExtBuf������,���Բ���Ҫ�ļ���д���ݻ�����,��Ȼ��д���Ȳ��� */
				SecCount = 100;  /* ��ʾ��ȡ���100������, �ڶ�д���������ӳ���xWriteToExtBuf�������� */
				NewSize = CH375vSectorSize * 100;
			}
			else {  /* ���ԭ�ļ���С,��ôʹ��ԭ���� */
				SecCount = ( CH375vFileSize + CH375vSectorSize - 1 ) / CH375vSectorSize;  /* �����ļ���������,��Ϊ��д��������Ϊ��λ��,�ȼ�CH375vSectorSize-1��Ϊ�˶����ļ�β������1�������Ĳ��� */
				NewSize = (UINT16)CH375vFileSize;  /* ԭ�ļ��ĳ��� */
			}
			printf( "Size=%ld, Len=%d, Sec=%d\n", CH375vFileSize, NewSize, (UINT16)SecCount );
			mCmdParam.Read.mSectorCount = SecCount;  /* ��ȡ������ȫ������ */
/*			current_buffer = & FILE_DATA_BUF[0];  ����ļ����ݵĸ��Ʒ�ʽΪ"�ⲿ�ӳ���"����LIB_CFG_FILE_IO_DEFAULTΪ1,��ô��Ҫ���ô�����ݵĻ���������ʼ��ַ */
			CH375vFileSize += CH375vSectorSize - 1;  /* Ĭ�������,��������ʽ��ȡ����ʱ,�޷������ļ�β������1�������Ĳ���,���Ա�����ʱ�Ӵ��ļ������Զ�ȡβ����ͷ */
			i = CH375FileRead( );  /* ���ļ���ȡ����,����ֱ����"�ⲿ�ӳ���"xWriteToExtBuf������ */
			CH375vFileSize -= CH375vSectorSize - 1;  /* �ָ�ԭ�ļ����� */
			mStopIfError( i );
/*
		����ļ��Ƚϴ�,һ�ζ�����,�����ٵ���CH375FileRead������ȡ,�ļ�ָ���Զ�����ƶ�
		while ( 1 ) {
			c = 32;   ÿ�ζ�ȡ32������
			mCmdParam.Read.mSectorCount = c;   ָ����ȡ��������
			CH375FileRead();   ������ļ�ָ���Զ�����
			��������
			if ( mCmdParam.Read.mSectorCount < c ) break;   ʵ�ʶ�������������С��˵���ļ��Ѿ�����
		}

	    ���ϣ����ָ��λ�ÿ�ʼ��д,�����ƶ��ļ�ָ��
		mCmdParam.Locate.mSectorOffset = 3;  �����ļ���ǰ3��������ʼ��д
		i = CH375FileLocate( );
		mCmdParam.Read.mSectorCount = 10;
		CH375FileRead();   ֱ�Ӷ�ȡ���ļ��ĵ�(CH375vSectorSize*3)���ֽڿ�ʼ������,ǰ3������������

	    ���ϣ�������������ӵ�ԭ�ļ���β��,�����ƶ��ļ�ָ��
		i = CH375FileOpen( );
		mCmdParam.Locate.mSectorOffset = 0xffffffff;  �Ƶ��ļ���β��,������Ϊ��λ,���ԭ�ļ���3�ֽ�,���CH375vSectorSize�ֽڿ�ʼ����
		i = CH375FileLocate( );
		mCmdParam.Write.mSectorCount = 10;
		CH375FileWrite();   ��ԭ�ļ��ĺ�����������,��ΪxReadFromExtBuf�ӳ�����ṩд���ļ�������,���Բ���Ҫ�ļ���д���ݻ�����,��Ȼ��д���Ȳ���
*/
			printf( "Close\n" );
			i = CH375FileClose( );  /* �ر��ļ� */
			mStopIfError( i );
		}

#ifdef EN_DISK_WRITE  /* �ӳ����֧��д���� */
/* �������ļ� */
		LED_WR_ACT( );  /* д���� */
		printf( "Create\n" );
		mCopyCodeStringToIRAM( mCmdParam.Create.mPathName, "\\����ȫ0.TXT" );  /* ���ļ���,�ڸ�Ŀ¼�� */
		i = CH375FileCreate( );  /* �½��ļ�����,����ļ��Ѿ���������ɾ�������½� */
		mStopIfError( i );
		printf( "Write\n" );
		mCmdParam.Write.mSectorCount = SecCount;  /* д���������������� */
/*		current_buffer = & FILE_DATA_BUF[0];  ����ļ������ݵĸ��Ʒ�ʽΪ"�ⲿ�ӳ���"����LIB_CFG_FILE_IO_DEFAULTΪ1,��ô��Ҫ���ô�����ݵĻ���������ʼ��ַ */
		i = CH375FileWrite( );  /* ���ļ�д������,ʵ��д���ļ���������"�ⲿ�ӳ���"xReadFromExtBuf�ṩ,������Ҫ��������ADC���ߴ��ڵ� */
		mStopIfError( i );
		printf( "Modify\n" );
		mCmdParam.Modify.mFileAttr = ATTR_READ_ONLY;  /* �������: �µ��ļ�����,ֻ���ļ� */
		mCmdParam.Modify.mFileTime = MAKE_FILE_TIME( 3, 7, 5 );  /* �������: �µ��ļ�ʱ��: 3ʱ7��5�� */
		mCmdParam.Modify.mFileDate = MAKE_FILE_DATE( 2004, 5, 18 );  /* �������: �µ��ļ�����: 2004.05.18 */
		mCmdParam.Modify.mFileSize = NewSize;  /* �������: ���ԭ�ļ���С,��ô�µ��ļ�������ԭ�ļ�һ����,����RAM����,����ļ����ȴ���64KB,��ôNewSize����ΪUINT32 */
		i = CH375FileModify( );  /* �޸ĵ�ǰ�ļ�����Ϣ,�޸����ںͳ��� */
		mStopIfError( i );
		printf( "Close\n" );
		mCmdParam.Close.mUpdateLen = 0;  /* ��Ҫ�Զ������ļ�����,����Զ�����,��ô�ó�������CH375vSectorSize�ı��� */
		i = CH375FileClose( );
		mStopIfError( i );

/* ɾ��ĳ�ļ� */
/*		printf( "Erase\n" );
		mCopyCodeStringToIRAM( mCmdParam.Create.mPathName, "\\OLD" );  ����ɾ�����ļ���,�ڸ�Ŀ¼��
		i = CH375FileErase( );  ɾ���ļ����ر�
		if ( i != ERR_SUCCESS ) printf( "Error: %02X\n", (UINT16)i );  ��ʾ����
*/
		LED_WR_INACT( );

/* ��ѯ������Ϣ */
/*		printf( "Disk\n" );
		i = CH375DiskQuery( );
		mStopIfError( i );
		printf( "Fat=%d, Total=%ld, Free=%ld\n", (UINT16)mCmdParam.Query.mDiskFat, mCmdParam.Query.mTotalSector, mCmdParam.Query.mFreeSector );
*/
#endif
		LED_RUN_INACT( );
		printf( "Take out\n" );
//		while ( CH375DiskStatus != DISK_DISCONNECT ) xQueryInterrupt( );  /* ��ѯCH375�жϲ������ж�״̬,�ȴ�U�̰γ� */
		while ( CH375DiskStatus >= DISK_CONNECT ) {  /* ��ѯCH375�жϲ������ж�״̬,�ȴ�U�̰γ� */
			if ( CH375DiskConnect( ) != ERR_SUCCESS ) break;
			mDelay100mS( );
		}
		LED_OUT_INACT( );  /* LED�� */
		mDelay100mS( );
		mDelay100mS( );
	}
}