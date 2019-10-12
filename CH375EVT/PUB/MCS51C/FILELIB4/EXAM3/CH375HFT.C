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
/* �ó���U���е�/C51/CH375HFT.C�ļ��е�ÿ512�ֽڵ�ǰ100���ַ���ʾ����,
   ����Ҳ���ԭ�ļ�CH375HFT.C, ��ô�ó�����ʾC51��Ŀ¼��������CH375��ͷ���ļ���,
   ����Ҳ���C51��Ŀ¼, ��ô�ó�����ʾ��Ŀ¼�µ������ļ���,
   ��󽫳���ROM��ǰ2KB����(4������)д���½����ļ�"����ռ�.BIN"��(MCS51��Ƭ���ĳ������Ķ�����Ŀ������)
*/
/* CH375��INT#���Ų��ò�ѯ��ʽ����, ���ݸ��Ʒ�ʽΪ"˫DPTR����",
   ����ֻʹ��512�ֽڵ��ⲿRAM, ͬʱ��Ϊ�������ݻ��������ļ����ݻ�����, ��ʾû���ⲿRAM���ǵ�Ƭ��������RAM����768�ֽڵ�Ӧ�� */


/* C51   CH375HFT.C */
/* LX51  CH375HFT.OBJ , CH375HF4.LIB    �����CH375HF4����CH375HF6�Ϳ���֧��FAT32 */
/* OHX51 CH375HFT */

#include <reg52.h>
#include <stdio.h>

/* ���¶������ϸ˵���뿴CH375HF6.H�ļ� */
#define LIB_CFG_DISK_IO			2		/* ���̶�д�����ݵĸ��Ʒ�ʽ,1Ϊ"��DPTR����",2Ϊ"˫DPTR����",3Ϊ"��DPTR��P2+R0����" */
#define LIB_CFG_FILE_IO			2		/* �ļ���д�����ݵĸ��Ʒ�ʽ,0Ϊ"�ⲿ�ӳ���",1Ϊ"��DPTR����",2Ϊ"˫DPTR����",3Ϊ"��DPTR��P2+R0����" */
#define LIB_CFG_INT_EN			0		/* CH375��INT#�������ӷ�ʽ,0Ϊ"��ѯ��ʽ",1Ϊ"�жϷ�ʽ" */
/*#define LIB_CFG_FILE_IO_DEFAULT	1*/		/* ʹ��CH375HF6.H�ṩ��Ĭ��"�ⲿ�ӳ���" */

#define CH375_CMD_PORT_ADDR		0xBDF1	/* CH375����˿ڵ�I/O��ַ */
#define CH375_DAT_PORT_ADDR		0xBCF0	/* CH375���ݶ˿ڵ�I/O��ַ */
/* ֻʹ�õ�Ƭ�����õ�1KB�ⲿRAM: 0000H-01FFH Ϊ���̶�д������, ͬʱ�����ļ����ݻ����� */
#define	DISK_BASE_BUF_ADDR		0x0000	/* �ⲿRAM�Ĵ������ݻ���������ʼ��ַ,�Ӹõ�Ԫ��ʼ�Ļ���������ΪSECTOR_SIZE */
#define FILE_DATA_BUF_ADDR		0x0000	/* �ⲿRAM���ļ����ݻ���������ʼ��ַ,���������Ȳ�С��һ�ζ�д�����ݳ��� */
/* ���ڵ�Ƭ�����õ��ⲿRAMֻ��1KB, ��Щ��Ƭ����Ҫȥ��256�ֽ��ڲ�RAM, ֻʣ��768�ֽڵ��ⲿRAM,
   ����ǰ512�ֽ���CH375�ӳ������ڴ������ݻ���, �ڵ���CH375DirtyBuffer�ӳ����ͬʱҲ�����ļ���д���� */
#define FILE_DATA_BUF_LEN		0x0200	/* �ⲿRAM���ļ����ݻ�����,���������Ȳ�С��һ�ζ�д�����ݳ��� */

#define CH375_INT_WIRE			INT0	/* P3.2, INT0, CH375���ж���INT#����,����CH375��INT#����,���ڲ�ѯ�ж�״̬ */

#include "..\CH375HF6.H"				/* �������Ҫ֧��FAT32,��ô��ѡ��CH375HF4.H */

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
	UINT16	NewSize, count;  /* ��Ϊ��ʾ���RAM����ֻ��32KB,����NewSize����Ϊ16λ,ʵ��������ļ�����32256�ֽ�,Ӧ�÷ּ��ζ�д���ҽ�NewSize��ΪUINT32�Ա��ۼ� */
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
		while ( CH375DiskStatus != DISK_CONNECT ) xQueryInterrupt( );  /* ��ѯCH375�жϲ������ж�״̬,�ȴ�U�̲��� */
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
		}
		else {  /* �ҵ��ļ����߳��� */
			mStopIfError( i );
/*			printf( "Query\n" );
			i = CH375FileQuery( );  ��ѯ��ǰ�ļ�����Ϣ
			mStopIfError( i );*/
			printf( "Read\n" );
			if ( CH375vFileSize > CH375vSectorSize * 3 ) {  /* �����õ�Ƭ�����õ�1KB�ⲿRAM��ʾ,ÿ��ֻ�ܶ�ȡһ������,�ٶ���ʾ��ȡ���3������ */
				SecCount = 3;  /* ��ʾ��ȡ���3������, ��3��, ÿ�ζ�ȡһ������������ */
				NewSize = CH375vSectorSize * 3;
			}
			else {  /* ���ԭ�ļ���С,��ôʹ��ԭ���� */
				SecCount = ( CH375vFileSize + CH375vSectorSize - 1 ) / CH375vSectorSize;  /* �����ļ���������,��Ϊ��д��������Ϊ��λ��,�ȼ�CH375vSectorSize-1��Ϊ�˶����ļ�β������1�������Ĳ��� */
				NewSize = (UINT16)CH375vFileSize;  /* ԭ�ļ��ĳ��� */
			}
			printf( "Size=%ld, Len=%d, Sec=%d\n", CH375vFileSize, NewSize, (UINT16)SecCount );
			CH375vFileSize += CH375vSectorSize -1 ;  /* Ĭ�������,��������ʽ��ȡ����ʱ,�޷������ļ�β������1�������Ĳ���,���Ա�����ʱ�Ӵ��ļ������Զ�ȡβ����ͷ */
			while( SecCount ) {  /* �ֶ�ζ�ȡ�ļ����� */
				mCmdParam.Read.mSectorCount = 1;  /* ����RAM����������,����ֻ��ȡ1������ */
/*				current_buffer = & FILE_DATA_BUF[0];  ����ļ���д�����ݵĸ��Ʒ�ʽΪ"�ⲿ�ӳ���",��ô��Ҫ���ô�����ݵĻ���������ʼ��ַ */
				i = CH375FileRead( );  /* ���ļ���ȡ���� */
				mStopIfError( i );
				CH375DirtyBuffer( );  /* ��Ϊ�ļ���д��������������ݻ������ص�,������CH375FileRead���ļ������CH375FileWriteд�ļ�ǰ����������̻����� */
				i = FILE_DATA_BUF[100];
				FILE_DATA_BUF[100] = 0;  /* ���ַ���������־,�����ʾ100���ַ� */
				printf( "Line 1: %s\n", FILE_DATA_BUF );  /* ����ʾ���ݴ�������ݵĴ��� */
				FILE_DATA_BUF[100] = i;  /* �ָ�ԭ�ַ� */
				SecCount --;
			}
			CH375vFileSize -= CH375vSectorSize - 1;  /* �ָ�ԭ�ļ����� */
/*
		����ļ��Ƚϴ�,һ�ζ�����,�����ٵ���CH375FileRead������ȡ,�ļ�ָ���Զ�����ƶ�
		while ( 1 ) {
			mCmdParam.Read.mSectorCount = 1;   ָ����ȡ��������
			CH375FileRead();   ������ļ�ָ���Զ�����
			CH375DirtyBuffer( );  ��Ϊ�ļ���д��������������ݻ������ص�,������CH375FileRead���ļ������CH375FileWriteд�ļ�ǰ����������̻�����
			�����Ѷ�����CH375vSectorSize�ֽ�����,��ɺ������ȡ��һ������
			if ( mCmdParam.Read.mSectorCount != 1 ) break;   ʵ�ʶ�������������С��˵���ļ��Ѿ�����
		}

	    ���ϣ����ָ��λ�ÿ�ʼ��д,�����ƶ��ļ�ָ��
		mCmdParam.Locate.mSectorOffset = 3;  �����ļ���ǰ3��������ʼ��д
		i = CH375FileLocate( );
		mCmdParam.Read.mSectorCount = 1;
		CH375FileRead();   ֱ�Ӷ�ȡ���ļ��ĵ�(CH375vSectorSize*3)���ֽڿ�ʼ������,ǰ3������������
		CH375DirtyBuffer( );  ��Ϊ�ļ���д��������������ݻ������ص�,������CH375FileRead���ļ������CH375FileWriteд�ļ�ǰ����������̻�����

	    ���ϣ�������������ӵ�ԭ�ļ���β��,�����ƶ��ļ�ָ��
		CH375FileOpen( );
		CH375FileQuery( );
		OldSize = mCmdParam.Modify.mFileSize;
		mCmdParam.Locate.mSectorOffset = 0xffffffff;  �Ƶ��ļ���β��,������Ϊ��λ,���ԭ�ļ���3�ֽ�,���CH375vSectorSize�ֽڿ�ʼ����
		CH375FileLocate( );
		for ( i=0; i!=����������; i++ ) {   Ϊ�������ļ��ռ��д����Ч����
			mCmdParam.Write.mSectorCount = 1;
			CH375FileWrite( );
		}
		mCmdParam.Locate.mSectorOffset = (OldSize+CH375vSectorSize-1)/CH375vSectorSize;  �Ƶ��ļ���ԭβ��,������Ϊ��λ,���ԭ�ļ���3�ֽ�,���CH375vSectorSize�ֽڿ�ʼ����
		CH375FileLocate( );
		for ( i=0; i!=����������; i++ ) {   �ֶ��д���������ļ�����
			CH375DirtyBuffer( );  ��Ϊ�ļ���д��������������ݻ������ص�,������CH375FileRead���ļ������CH375FileWriteд�ļ�ǰ����������̻�����
  ��׼��д���ļ������ݸ��Ƶ��ļ����ݻ�����,����ļ����ݸ��Ʒ�ʽΪ"�ⲿ�ӳ���",��ô������xReadFromExtBuf������ֱ���͸�CH375оƬ���������ļ����ݻ�����
			mCmdParam.Write.mSectorCount = 1;
			CH375FileWrite();   ��ԭ�ļ��ĺ�����������
		}
*/
			printf( "Close\n" );
			i = CH375FileClose( );  /* �ر��ļ� */
			mStopIfError( i );
		}

#ifdef EN_DISK_WRITE  /* �ӳ����֧��д���� */
/* �������ļ� */
		LED_WR_ACT( );  /* д���� */
		NewSize = CH375vSectorSize * 4;  /* ���ļ��ĳ��� */
		SecCount = 4;  /* (NewSize+CH375vSectorSize-1)/CH375vSectorSize, �����ļ���������,��Ϊ��д��������Ϊ��λ�� */
		printf( "Create\n" );
		mCopyCodeStringToIRAM( mCmdParam.Create.mPathName, "\\����ռ�.BIN" );  /* ���ļ���,�ڸ�Ŀ¼��,�����ļ��� */
		i = CH375FileCreate( );  /* �½��ļ�����,����ļ��Ѿ���������ɾ�������½� */
		mStopIfError( i );
		printf( "PreWrite\n" );
/* �½��ļ��ĳ���Ϊ1, ռ��һ����, �������׼��д��������ܳ��ȳ���һ����, ����Ҫ��CH375FileWrite�������Զ������ļ��ռ�,
   �������ļ��ռ�Ĺ�����Ҫ�õ��������ݻ�����, ���ڱ�����RAM̫��ʹ�������ݻ��������ļ����ݻ���������,
   ����CH375FileWrite�������Զ������ļ��ռ�ᵼ���ļ����ݻ������е�������Ч, ����ʵ�������ʱ��д�������û������,
   �������֪���������ݵ��ܳ��Ȳ�����һ����(����Windows�г�Ϊ"���䵥Ԫ"�Ĵ�С), ��ô�����������Ϊ�������ļ��ռ��д����Ч���ݵĲ��� */
		for ( i = 0; i != SecCount; i ++ ) {
			mCmdParam.Write.mSectorCount = 1;  /* д��1������������ */
/*			current_buffer = & FILE_DATA_BUF[0];  ����ļ���д�����ݵĸ��Ʒ�ʽΪ"�ⲿ�ӳ���",��ô��Ҫ���ô�����ݵĻ���������ʼ��ַ */
			mStopIfError( CH375FileWrite( ) );  /* ���ļ�д������,ֻ��Ϊ�������ļ��ռ�,ʵ��д�������û������ */
		}
		printf( "Locate head\n" );
		mCmdParam.Locate.mSectorOffset = 0;  /* ��Ϊǰ�������ļ��ռ�ʹ�ļ�ָ�봦��β��,������д������������ǰ��Ҫ���ļ�ָ��ص�ͷ�� */
		i = CH375FileLocate( );  /* �ƶ��ļ�ָ�뵽�ļ�ͷ�� */
		mStopIfError( i );
		printf( "Write\n" );
		pCodeStr = 0;  /* �ӳ���ռ����ʼ��ַ��ʼȡ���� */
		while( SecCount ) {  /* �ֶ��д���������ļ����� */
			CH375DirtyBuffer( );  /* ��Ϊ�ļ���д��������������ݻ������ص�,������CH375FileRead���ļ������CH375FileWriteд�ļ�ǰ����������̻����� */
/* ��׼��д���ļ������ݸ��Ƶ��ļ����ݻ�����,����ļ����ݸ��Ʒ�ʽΪ"�ⲿ�ӳ���",
   ��ô�������Լ������"�ⲿ�ӳ���"xReadFromExtBuf������ֱ���͸�CH375оƬ���������ļ����ݻ�����,���ӳ���CH375FileWrite�ӳ������ */
			for ( count = 0; count != CH375vSectorSize; count ++ ) {  /* �����ǽ�����ռ�����ݸ��Ƶ��ļ����ݻ�������д���ļ� */
				FILE_DATA_BUF[ count ] = *pCodeStr;  /* ʵ��Ӧ����,���ݿ��������ⲿADC������־���� */
				pCodeStr ++;
			}
			mCmdParam.Write.mSectorCount = 1;  /* д��1������������ */
/*			current_buffer = & FILE_DATA_BUF[0];  ����ļ���д�����ݵĸ��Ʒ�ʽΪ"�ⲿ�ӳ���",��ô��Ҫ���ô�����ݵĻ���������ʼ��ַ */
			i = CH375FileWrite( );  /* ���ļ�д������ */
			mStopIfError( i );
			SecCount --;
		}
/*		printf( "Modify\n" );
		mCmdParam.Modify.mFileAttr = 0xff;   �������: �µ��ļ�����,Ϊ0FFH���޸�
		mCmdParam.Modify.mFileTime = 0xffff;   �������: �µ��ļ�ʱ��,Ϊ0FFFFH���޸�,ʹ���½��ļ�������Ĭ��ʱ��
		mCmdParam.Modify.mFileDate = MAKE_FILE_DATE( 2004, 5, 18 );  �������: �µ��ļ�����: 2004.05.18
		mCmdParam.Modify.mFileSize = NewSize;   �������: ���ԭ�ļ���С,��ô�µ��ļ�������ԭ�ļ�һ����,����RAM����,����ļ����ȴ���64KB,��ôNewSize����ΪUINT32
		i = CH375FileModify( );   �޸ĵ�ǰ�ļ�����Ϣ,�޸����ںͳ���
		mStopIfError( i );
*/
		printf( "Close\n" );
		mCmdParam.Close.mUpdateLen = 1;  /* �Զ������ļ�����,����Զ�����,��ô�ó�������CH375vSectorSize�ı��� */
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
		while ( CH375DiskStatus != DISK_DISCONNECT ) xQueryInterrupt( );  /* ��ѯCH375�жϲ������ж�״̬,�ȴ�U�̰γ� */
		LED_OUT_INACT( );  /* LED�� */
		mDelay100mS( );
		mDelay100mS( );
	}
}