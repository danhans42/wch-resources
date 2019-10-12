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

/* MCS-51��Ƭ��C���Ե�U���ļ���дʾ������, ������89C52���߸������ռ�ĵ�Ƭ�� */
/* ������������ʾ������Ŀ¼ */
/* CH375��INT#���Ų��ò�ѯ��ʽ����, ���ݸ��Ʒ�ʽΪ"��DPTR����", �����ٶȽ���, ����������MCS51��Ƭ��
   ����������V1.6�����ϰ汾��CH375�ӳ���� */


/* C51   CH375HFT.C */
/* LX51  CH375HFT.OBJ , CH375HF6 */
/* OHX51 CH375HFT */

#include <reg52.h>
#include <stdio.h>
#include <string.h>

/* ���¶������ϸ˵���뿴CH375HF6.H�ļ� */
#define LIB_CFG_DISK_IO			1		/* ���̶�д�����ݵĸ��Ʒ�ʽ,1Ϊ"��DPTR����",2Ϊ"˫DPTR����",3Ϊ"��DPTR��P2+R0����" */
#define LIB_CFG_FILE_IO			1		/* �ļ���д�����ݵĸ��Ʒ�ʽ,0Ϊ"�ⲿ�ӳ���",1Ϊ"��DPTR����",2Ϊ"˫DPTR����",3Ϊ"��DPTR��P2+R0����" */
#define LIB_CFG_INT_EN			0		/* CH375��INT#�������ӷ�ʽ,0Ϊ"��ѯ��ʽ",1Ϊ"�жϷ�ʽ" */
/*#define LIB_CFG_FILE_IO_DEFAULT	1*/		/* ʹ��CH375HF6.H�ṩ��Ĭ��"�ⲿ�ӳ���" */
/*#define LIB_CFG_UPD_SIZE		1*/		/* ���������ݺ��Ƿ��Զ������ļ�����: 0Ϊ"������",1Ϊ"�Զ�����" */
/* Ĭ�������,���������/�ֽ�����Ϊ0��ôCH375FileWrite/CH375ByteWriteֻ����д�����ݶ����޸��ļ�����,
   �����Ҫÿ��д�����ݺ���Զ��޸�/�����ļ�����,��ô����ʹȫ�ֱ���CH375LibConfig��λ4Ϊ1,
   �����ʱ�䲻д��������Ӧ�ø����ļ�����,��ֹͻȻ�ϵ��ǰ��д����������ļ����Ȳ����,
   ���ȷ������ͻȻ�ϵ���ߺ���ܿ������ݲ���д���򲻱ظ����ļ�����,��������ٶȲ�����U�����(U���ڲ����ڴ���������,����Ƶ����д) */

#define CH375_CMD_PORT_ADDR		0xBDF1	/* CH375����˿ڵ�I/O��ַ */
#define CH375_DAT_PORT_ADDR		0xBCF0	/* CH375���ݶ˿ڵ�I/O��ַ */
/* 62256�ṩ��32KB��RAM��Ϊ������: 0000H-01FFHΪ���̶�д������, 0200H-7FFFHΪ�ļ����ݻ����� */
#define	DISK_BASE_BUF_ADDR		0x0000	/* �ⲿRAM�Ĵ������ݻ���������ʼ��ַ,�Ӹõ�Ԫ��ʼ�Ļ���������ΪSECTOR_SIZE */
#define DISK_BASE_BUF_LEN		4096	/* Ĭ�ϵĴ������ݻ�������СΪ512�ֽ�,����ѡ��Ϊ2048����4096��֧��ĳЩ��������U��,Ϊ0���ֹ��.H�ļ��ж��建��������Ӧ�ó�����pDISK_BASE_BUF��ָ�� */
#define FILE_DATA_BUF_ADDR		0x1000	/* �ⲿRAM���ļ����ݻ���������ʼ��ַ,���������Ȳ�С��һ�ζ�д�����ݳ��� */
/* ������ʾ���õ�62256ֻ��32K�ֽ�,����CH375�ӳ�����512�ֽ�,�����ⲿRAMʣ�೤��Ϊ32256�ֽ� */
#define FILE_DATA_BUF_LEN		0x3E00	/* �ⲿRAM���ļ����ݻ�����,���������Ȳ�С��һ�ζ�д�����ݳ���,����Ҫ��С��0x400���� */
/* ���׼��ʹ��˫�����������д,��ô��Ҫ����FILE_DATA_BUF_LEN,�����ڲ�����ָ����������ַ,��CH375FileReadX����CH375FileRead,��CH375FileWriteX����CH375FileWrite */

#define CH375_INT_WIRE			INT0	/* P3.2, INT0, CH375���ж���INT#����,����CH375��INT#����,���ڲ�ѯ�ж�״̬ */

#define NO_DEFAULT_CH375_F_ENUM		1		/* δ����CH375FileEnumer����ʽ�ֹ�Խ�Լ���� */
#define NO_DEFAULT_CH375_F_QUERY	1		/* δ����CH375FileQuery����ʽ�ֹ�Խ�Լ���� */

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

/* �Ժ���Ϊ��λ��ʱ,����ȷ,������24MHzʱ�� */
void	mDelaymS( unsigned char delay )
{
	unsigned char	i, j, c;
	for ( i = delay; i != 0; i -- ) {
		for ( j = 200; j != 0; j -- ) c += 3;  /* ��24MHzʱ������ʱ500uS */
		for ( j = 200; j != 0; j -- ) c += 3;  /* ��24MHzʱ������ʱ500uS */
	}
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
		mDelaymS( 200 );
		LED_OUT_INACT( );
		mDelaymS( 200 );
	}
}

/* Ϊprintf��getkey���������ʼ������ */
void	mInitSTDIO( )
{
	SCON = 0x50;
	PCON = 0x80;
	TMOD = 0x21;
	TH1 = 0xf3;  /* 24MHz����, 9600bps */
	TR1 = 1;
	TI = 1;
}

/* �½�Ŀ¼����,���Ŀ¼�Ѿ�������ֱ�Ӵ� */
/* �������:   Ŀ¼����mCmdParam.Create.mPathName��,���ļ���������ͬ */
/* ����״̬��: ERR_SUCCESS = ��Ŀ¼�ɹ����ߴ���Ŀ¼�ɹ�,
               ERR_FOUND_NAME = �Ѿ�����ͬ���ļ�,
               ERR_MISS_DIR = ·������Ч�����ϼ�Ŀ¼������,
               ����״̬��ο�CH375HF?.H */
UINT8	CreateDirectory( void )
{
	UINT8	i, j;
	UINT16	count;
	UINT32	UpDirCluster;
	PUINT8X	DirXramBuf;
	UINT8 code *DirConstData;
	j = 0xFF;
	for ( i = 0; i != sizeof( mCmdParam.Create.mPathName ); i ++ ) {  /* ���Ŀ¼·�� */
		if ( mCmdParam.Create.mPathName[ i ] == 0 ) break;
		if ( mCmdParam.Create.mPathName[ i ] == PATH_SEPAR_CHAR1 || mCmdParam.Create.mPathName[ i ] == PATH_SEPAR_CHAR2 ) j = i;  /* ��¼�ϼ�Ŀ¼ */
	}
	i = ERR_SUCCESS;
	if ( j == 0 || j == 2 && mCmdParam.Create.mPathName[1] == ':' ) UpDirCluster = 0;  /* �ڸ�Ŀ¼�´�����Ŀ¼ */
	else {
		if ( j != 0xFF ) {  /* ���ھ���·��Ӧ�û�ȡ�ϼ�Ŀ¼����ʼ�غ� */
			mCmdParam.Create.mPathName[ j ] = 0;
			i = CH375FileOpen( );  /* ���ϼ�Ŀ¼ */
			if ( i == ERR_SUCCESS ) i = ERR_MISS_DIR;  /* ���ļ�����Ŀ¼ */
			else if ( i == ERR_OPEN_DIR ) i = ERR_SUCCESS;  /* �ɹ����ϼ�Ŀ¼ */
			mCmdParam.Create.mPathName[ j ] = PATH_SEPAR_CHAR1;  /* �ָ�Ŀ¼�ָ��� */
		}
		UpDirCluster = CH375vStartCluster;  /* �����ϼ�Ŀ¼����ʼ�غ� */
	}
	if ( i == ERR_SUCCESS ) {  /* �ɹ���ȡ�ϼ�Ŀ¼����ʼ�غ� */
		i = CH375FileOpen( );  /* �򿪱�����Ŀ¼ */
		if ( i == ERR_SUCCESS ) i = ERR_FOUND_NAME;  /* ���ļ�����Ŀ¼ */
		else if ( i == ERR_OPEN_DIR ) i = ERR_SUCCESS;  /* Ŀ¼�Ѿ����� */
		else if ( i == ERR_MISS_FILE ) {  /* Ŀ¼������,�����½� */
			i = CH375FileCreate( );  /* �Դ����ļ��ķ�������Ŀ¼ */
			if ( i == ERR_SUCCESS ) {
//				if ( &FILE_DATA_BUF[0] == &DISK_BASE_BUF[0] ) CH375DirtyBuffer( );  /* ���FILE_DATA_BUF��DISK_BASE_BUF���������������̻����� */
				DirXramBuf = &FILE_DATA_BUF[0];  /* �ļ����ݻ����� */
				DirConstData = ".          \x10\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x21\x30\x0\x0\x0\x0\x0\x0..         \x10\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x21\x30\x0\x0\x0\x0\x0\x0";
				for ( i = 0x40; i != 0; i -- ) {  /* Ŀ¼�ı�����Ԫ,�ֱ�ָ���������ϼ�Ŀ¼ */
					*DirXramBuf = *DirConstData;
					DirXramBuf ++;
					DirConstData ++;
				}
#ifdef __C51__	// ���MCS51����
				FILE_DATA_BUF[0x1A] = ( (PUINT8I)&CH375vStartCluster )[3];  /* ��������ʼ�غ� */
				FILE_DATA_BUF[0x1B] = ( (PUINT8I)&CH375vStartCluster )[2];
				FILE_DATA_BUF[0x14] = ( (PUINT8I)&CH375vStartCluster )[1];
				FILE_DATA_BUF[0x15] = ( (PUINT8I)&CH375vStartCluster )[0];
				FILE_DATA_BUF[0x20+0x1A] = ( (PUINT8I)&UpDirCluster )[3];  /* �ϼ�Ŀ¼����ʼ�غ� */
				FILE_DATA_BUF[0x20+0x1B] = ( (PUINT8I)&UpDirCluster )[2];
				FILE_DATA_BUF[0x20+0x14] = ( (PUINT8I)&UpDirCluster )[1];
				FILE_DATA_BUF[0x20+0x15] = ( (PUINT8I)&UpDirCluster )[0];
#else  // ������Ƭ��
				FILE_DATA_BUF[0x1A] = (UINT8)CH375vStartCluster;  /* ��������ʼ�غ� */
				FILE_DATA_BUF[0x1B] = (UINT8)(CH375vStartCluster>>8);
				FILE_DATA_BUF[0x14] = (UINT8)(CH375vStartCluster>>16);
				FILE_DATA_BUF[0x15] = (UINT8)(CH375vStartCluster>>24);
				FILE_DATA_BUF[0x20+0x1A] = (UINT8)UpDirCluster;  /* �ϼ�Ŀ¼����ʼ�غ� */
				FILE_DATA_BUF[0x20+0x1B] = (UINT8)(UpDirCluster>>8);
				FILE_DATA_BUF[0x20+0x14] = (UINT8)(UpDirCluster>>16);
				FILE_DATA_BUF[0x20+0x15] = (UINT8)(UpDirCluster>>24);
#endif
				for ( count = 0x40; count != CH375vSectorSize; count ++ ) {  /* ���Ŀ¼��ʣ�ಿ�� */
					*DirXramBuf = 0;
					DirXramBuf ++;
				}
				mCmdParam.Write.mSectorCount = 1;
				i = CH375FileWrite( );  /* дĿ¼�ĵ�һ������ */
				if ( i == ERR_SUCCESS ) {
					DirXramBuf = &FILE_DATA_BUF[0];
					for ( i = 0x40; i != 0; i -- ) {  /* ���Ŀ¼�� */
						*DirXramBuf = 0;
						DirXramBuf ++;
					}
					for ( j = 1; j != CH375vSecPerClus; j ++ ) {
//						if ( &FILE_DATA_BUF[0] == &DISK_BASE_BUF[0] ) CH375DirtyBuffer( );  /* ���FILE_DATA_BUF��DISK_BASE_BUF���������������̻����� */
						mCmdParam.Write.mSectorCount = 1;
						i = CH375FileWrite( );  /* ���Ŀ¼��ʣ������ */
						if ( i != ERR_SUCCESS ) break;
					}
					if ( j == CH375vSecPerClus ) {  /* �ɹ����Ŀ¼ */
						mCmdParam.Modify.mFileSize = 0;  /* Ŀ¼�ĳ�������0 */
						mCmdParam.Modify.mFileDate = 0xFFFF;
						mCmdParam.Modify.mFileTime = 0xFFFF;
						mCmdParam.Modify.mFileAttr = 0x10;  /* ��Ŀ¼���� */
						i = CH375FileModify( );  /* ���ļ���Ϣ�޸�ΪĿ¼ */
					}
				}
			}
		}
	}
	return( i );
}

main( ) {
	UINT8	i;
	LED_OUT_INIT( );
	LED_OUT_ACT( );  /* ������LED��һ����ʾ���� */
	mDelaymS( 100 );  /* ��ʱ100���� */
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
		mDelaymS( 200 );  /* ��ʱ,��ѡ����,�е�USB�洢����Ҫ��ʮ�������ʱ */

/* ���U���Ƿ�׼����,����ĳЩU�̱���Ҫִ����һ�����ܹ��� */
		for ( i = 0; i < 5; i ++ ) {  /* �е�U�����Ƿ���δ׼����,�������Ա����� */
			mDelaymS( 100 );
			printf( "Ready ?\n" );
			if ( CH375DiskReady( ) == ERR_SUCCESS ) break;  /* ��ѯ�����Ƿ�׼���� */
		}
#if DISK_BASE_BUF_LEN
		if ( DISK_BASE_BUF_LEN < CH375vSectorSize ) {  /* ���������ݻ������Ƿ��㹻��,CH375vSectorSize��U�̵�ʵ��������С */
			printf( "Too large sector size\n" );
			while ( CH375DiskConnect( ) == ERR_SUCCESS ) mDelaymS( 100 );
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

		printf( "Create Level 1 Directory /YEAR2004 \n" );
		mCopyCodeStringToIRAM( mCmdParam.Create.mPathName, "/YEAR2004" );  /* Ŀ¼��,��Ŀ¼���ڸ�Ŀ¼�� */
		LED_WR_ACT( );  /* д���� */
		i = CreateDirectory( );  /* �½����ߴ�Ŀ¼ */
		mStopIfError( i );
/* Ŀ¼�½����ߴ򿪳ɹ�,�����������Ŀ¼���½�һ����ʾ�ļ� */
		printf( "Create New File /YEAR2004/DEMO2004.TXT \n" );
		mCopyCodeStringToIRAM( mCmdParam.Create.mPathName, "/YEAR2004/DEMO2004.TXT" );  /* �ļ��� */
		i = CH375FileCreate( );  /* �½��ļ�����,����ļ��Ѿ���������ɾ�������½� */
		mStopIfError( i );
		printf( "Write some data to file DEMO2004.TXT \n" );
		i = mCopyCodeStringToIRAM( mCmdParam.ByteWrite.mByteBuffer, "��ʾ����\xd\xa" );
		mCmdParam.ByteWrite.mByteCount = i;  /* ָ������д����ֽ���,���ζ�д�ĳ��Ȳ��ܳ���MAX_BYTE_IO */
		i = CH375ByteWrite( );  /* ���ֽ�Ϊ��λ���ļ�д������,���ζ�д�ĳ��Ȳ��ܳ���MAX_BYTE_IO */
		mStopIfError( i );
		printf( "Close file DEMO2004.TXT \n" );
		mCmdParam.Close.mUpdateLen = 1;  /* �Զ������ļ����� */
		i = CH375FileClose( );  /* �ر��ļ� */
		mStopIfError( i );
/* �����½�������Ŀ¼,������ǰ���һ����Ŀ¼��ȫ��ͬ */
		printf( "Create Level 2 Directory /YEAR2004/MONTH05 \n" );
		mCopyCodeStringToIRAM( mCmdParam.Create.mPathName, "/YEAR2004/MONTH05" );  /* Ŀ¼��,��Ŀ¼����YEAR2004��Ŀ¼��,YEAR2004Ŀ¼�������ȴ��� */
		i = CreateDirectory( );  /* �½����ߴ�Ŀ¼ */
		mStopIfError( i );
		printf( "Close\n" );
		mCmdParam.Close.mUpdateLen = 0;  /* ����Ŀ¼����Ҫ�Զ������ļ����� */
		i = CH375FileClose( );  /* �ر�Ŀ¼,Ŀ¼����Ҫ�ر�,�ر�ֻ��Ϊ�˷�ֹ��������� */
		LED_WR_INACT( );
		LED_RUN_INACT( );
		printf( "Take out\n" );
		while ( CH375DiskStatus != DISK_DISCONNECT ) xQueryInterrupt( );  /* ��ѯCH375�жϲ������ж�״̬,�ȴ�U�̰γ� */
		LED_OUT_INACT( );  /* LED�� */
		mDelaymS( 200 );
	}
}