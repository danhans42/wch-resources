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
/* ������������ʾ�г�ָ��Ŀ¼�µ������ļ����Լ���������/ö���ļ��� */
/* CH375��INT#���Ų��ò�ѯ��ʽ����, ���ݸ��Ʒ�ʽΪ"��DPTR����", �����ٶȽ���, ����������MCS51��Ƭ��
   ����������V3.0�����ϰ汾������V2.8�����ϰ汾��CH375�ӳ���� */


/* C51   CH375HFT.C */
/* LX51  CH375HFT.OBJ , CH375HF4.LIB    �����CH375HF4����CH375HF6�Ϳ���֧��FAT32 */
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
/* �����Ҫ���ô������ݻ������Խ�ԼRAM,��ô�ɽ�DISK_BASE_BUF_LEN����Ϊ0�Խ�ֹ��.H�ļ��ж��建����,����Ӧ�ó����ڵ���CH375Init֮ǰ��������������õĻ�������ʼ��ַ����pDISK_BASE_BUF���� */
#define FILE_DATA_BUF_ADDR		0x1000	/* �ⲿRAM���ļ����ݻ���������ʼ��ַ,���������Ȳ�С��һ�ζ�д�����ݳ���,�ֽ�ģʽ���øû����� */
#define FILE_DATA_BUF_LEN		4096

#define CH375_INT_WIRE			INT0	/* P3.2, INT0, CH375���ж���INT#����,����CH375��INT#����,���ڲ�ѯ�ж�״̬ */

#define NO_DEFAULT_CH375_F_ENUM		1		/* δ����CH375FileEnumer����ʽ�ֹ�Խ�Լ���� */
#define NO_DEFAULT_CH375_F_QUERY	1		/* δ����CH375FileQuery����ʽ�ֹ�Խ�Լ���� */

#define NO_DEFAULT_FILE_ENUMER		1		/* ��ֹĬ�ϵ��ļ���ö�ٻص�����,���������б�д�ĳ�������� */

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

/* ������״̬,�����������ʾ������벢ͣ��,Ӧ���滻Ϊʵ�ʵĴ�����ʩ */
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

#if CH375_LIB_VER	>= 0x30
/* V3.0�����ϰ汾���ӳ����,ֻ��V3.0�����ϰ汾���ӳ�������֧��xFileNameEnumer�ص����� */

typedef struct _FILE_NAME {
	UINT32	DirStartClust;				/* �ļ�����Ŀ¼����ʼ�غ� */
//	UINT32	Size;						/* �ļ����� */
	UINT8	Name[8+3+1+1];				/* �ļ���,��8+3�ֽ�,�ָ���,������,��Ϊδ����Ŀ¼�����������·�� */
	UINT8	Attr;						/* �ļ����� */
} FILE_NAME;
#define		MAX_FILE_COUNT		200
FILE_NAME	xdata	FileNameBuffer[ MAX_FILE_COUNT ];	/* �ļ����ṹ */
UINT16	FileCount;
UINT32	CurrentDirStartClust;			/* ���浱ǰĿ¼����ʼ�غ�,���ڼӿ��ļ�ö�ٺʹ��ٶ� */

/* ����:�о�ָ��Ŀ¼�µ������ļ� */
UINT8	ListFile( void )
// �������mCmdParam.Open.mPathName[]ΪĿ¼���ַ���,��ʽ���ļ�����ͬ,����б���������Ŀ¼
{
	UINT8	i;
	printf( "List Directory: %s\n", mCmdParam.Open.mPathName );  /* ��ʾĿ¼�� */
//	for ( i = 0; i < MAX_PATH_LEN; i ++ ) {  /* ��Ŀ¼���Ľ����� */
//		if ( mCmdParam.Open.mPathName[i] == 0 ) break;
//	}
	i = strlen( mCmdParam.Open.mPathName );  /* ����·���ĳ���,��Ŀ¼���Ľ����� */
	if ( i && mCmdParam.Open.mPathName[i-1] == '/' ) { }  /* �Ǹ�Ŀ¼,�������Ѿ���·���ָ��� */
	else mCmdParam.Open.mPathName[i++] = '/';  /* �ڵ�ǰĿ¼�½���ö��,����Ŀ¼�ⶼ�����·��,���Ǹ�Ŀ¼���·���ָ��� */
	mCmdParam.Open.mPathName[i++] = '*';  /* ö��ͨ���,������·������"\*"����"\C51\*"����"\C51\CH375*"�� */
	mCmdParam.Open.mPathName[i] = 0xFF;  /* 0xFFָ��ö�������CH375vFileSize�� */
	CH375vFileSize = 0xFFFFFFFF;  /* ��������ö��,ÿ�ҵ�һ���ļ�����һ��xFileNameEnumer�ص��ӳ���,���ֵС��0x80000000��ÿ��ֻö��һ���ļ�̫�� */
	i = CH375FileOpen( );  /* ö��,�ɻص�����xFileNameEnumer������¼���浽�ṹ�� */
	if ( i == ERR_SUCCESS || i == ERR_FOUND_NAME || i == ERR_MISS_FILE ) {  /* �����ɹ�,ͨ�����᷵��ERR_SUCCESS,����xFileNameEnumer��ǰ�˳�ʱ�Ż᷵��ERR_FOUND_NAME */
		printf( "Success, new FileCount = %d\n", FileCount );
		return( ERR_SUCCESS );
	}
	else {
		printf( "Failed, new FileCount = %d\n", FileCount );
		return( i );
	}
}

UINT8	ListAll( void )  /* �Թ�����ȵ��㷨ö������U���е������ļ���Ŀ¼ */
{
	UINT8	i;
	UINT16	OldFileCount;
	OldFileCount = FileCount = 0;  /* ���ļ��ṹ���� */
	FileNameBuffer[ 0 ].Name[0] = '/';  /* ��Ŀ¼,������·����,����Ŀ¼�Ǿ���·��֮�ⶼ�����·�� */
	FileNameBuffer[ 0 ].Name[1] = 0;
	FileNameBuffer[ 0 ].DirStartClust = 0;  /* ��Ŀ¼��������������� */
	FileNameBuffer[ 0 ].Attr = ATTR_DIRECTORY;  /* ��Ŀ¼Ҳ��Ŀ¼,��Ϊ��һ����¼���� */

	for ( FileCount = 1; OldFileCount < FileCount; OldFileCount ++ ) {  /* ������ö�ٵ����ļ����ṹδ���з��� */
		if ( FileNameBuffer[ OldFileCount ].Attr & ATTR_DIRECTORY ) {  /* ��Ŀ¼���������������� */
			strcpy( mCmdParam.Open.mPathName, FileNameBuffer[ OldFileCount ].Name );  /* Ŀ¼��,����Ŀ¼�ⶼ�����·�� */
			CH375vStartCluster = FileNameBuffer[ OldFileCount ].DirStartClust;  /* ��ǰĿ¼���ϼ�Ŀ¼����ʼ�غ�,���������·����,������·�����ٶȿ� */
			i = CH375FileOpen( );  /* ��Ŀ¼,��Ϊ�˻�ȡĿ¼����ʼ�غ�������ٶ� */
			if ( i == ERR_SUCCESS ) return( ERR_MISS_DIR );  /* Ӧ���Ǵ���Ŀ¼,���Ƿ��ؽ���Ǵ����ļ� */
			if ( i != ERR_OPEN_DIR ) return( i );
			if ( OldFileCount ) CurrentDirStartClust = CH375vStartCluster;  /* ���Ǹ�Ŀ¼,��ȡĿ¼����ʼ�غ� */
			else {  /* �Ǹ�Ŀ¼,��ȡ��Ŀ¼����ʼ�غ� */
				if ( CH375vDiskFat == DISK_FAT32 ) CurrentDirStartClust = CH375vDiskRoot;  /* FAT32��Ŀ¼ */
				else CurrentDirStartClust = 0;  /* FAT12/FAT16��Ŀ¼ */
			}
			CH375FileClose( );  /* ���ڸ�Ŀ¼һ��Ҫ�ر� */

//			strcpy( mCmdParam.Open.mPathName, FileNameBuffer[ OldFileCount ].Name );  /* Ŀ¼��,����mPathNameδ���޸����������ٸ��� */
			CH375vStartCluster = FileNameBuffer[ OldFileCount ].DirStartClust;  /* ��ǰĿ¼���ϼ�Ŀ¼����ʼ�غ�,���������·����,������·�����ٶȿ� */
			i = ListFile( );  /* ö��Ŀ¼,�ɻص�����xFileNameEnumer������¼���浽�ṹ�� */
			if ( i != ERR_SUCCESS ) return( i );
		}
	}

// U���е��ļ���Ŀ¼ȫ��ö�����,���濪ʼ���ݽṹ��¼���δ��ļ� */
	printf( "Total file&dir = %d, Open every file:\n", FileCount );
	for ( OldFileCount = 0; OldFileCount < FileCount; OldFileCount ++ ) {
		if ( ( FileNameBuffer[ OldFileCount ].Attr & ATTR_DIRECTORY ) == 0 ) {  /* ���ļ����,Ŀ¼������ */
			printf( "Open file: %s\n", FileNameBuffer[ OldFileCount ].Name );
			strcpy( mCmdParam.Open.mPathName, FileNameBuffer[ OldFileCount ].Name );  /* ���·�� */
			CH375vStartCluster = FileNameBuffer[ OldFileCount ].DirStartClust;  /* ��ǰ�ļ����ϼ�Ŀ¼����ʼ�غ�,���������·����,������·�����ٶȿ� */
			i = CH375FileOpen( );  /* ���ļ� */
			if ( i == ERR_SUCCESS ) {  /* �ɹ������ļ� */
				mCmdParam.ReadX.mDataBuffer = 0x2000;  /* ָ���ļ����ݻ���������ʼ��ַ */
				mCmdParam.ReadX.mSectorCount = 1;  /* ��ȡ������ */
				CH375FileReadX( );
//				CH375FileClose( );  /* ����д������������ر� */
			}
		}
	}
}

void xFileNameEnumer( void )			/* �ļ���ö�ٻص��ӳ���,�ο�CH375HF6.H�ļ��е����� */
{  /* ÿ������һ���ļ�FileOpen������ñ��ص�����xFileNameEnumer���غ�FileOpen�ݼ�CH375vFileSize������ö��ֱ�����������ļ�����Ŀ¼ */
	UINT8			i, c;
	P_FAT_DIR_INFO	pFileDir;
	PUINT8X			pNameBuf;
	pFileDir = (P_FAT_DIR_INFO)( (PUINT8X)(&DISK_BASE_BUF[0]) + CH375vFdtOffset );  /* ��ǰFDT����ʼ��ַ */
	if ( pFileDir -> DIR_Name[0] == '.' ) return;  /* �Ǳ��������ϼ�Ŀ¼��,���붪�������� */
	if ( ( pFileDir -> DIR_Attr & ATTR_DIRECTORY ) == 0 ) {  /* �ж����ļ��� */
		if ( pFileDir -> DIR_Name[8] == 'H' && pFileDir -> DIR_Name[9] == ' '  /* �����ļ�����չ��,��".H"�ļ�,����,���Ǽǲ����� */
			|| pFileDir -> DIR_Name[8] == 'E' && pFileDir -> DIR_Name[9] == 'X' && pFileDir -> DIR_Name[10] == 'E' ) return;  /* ��չ����".EXE"���ļ�,���� */
	}
	pNameBuf = & FileNameBuffer[ FileCount ].Name;  /* �ļ����ṹ�е��ļ��������� */
	for ( i = 0; i < 11; i ++ ) {  /* �����ļ���,����Ϊ11���ַ� */
		c = pFileDir -> DIR_Name[ i ];
		if ( i == 0 && c == 0x05 ) c = 0xE5;  /* �����ַ� */
		if ( c != 0x20 ) {  /* ��Ч�ַ� */
			if ( i == 8 ) {  /* ������չ�� */
				*pNameBuf = '.';  /* �ָ��� */
				pNameBuf ++;
			}
			*pNameBuf = c;  /* �����ļ�����һ���ַ� */
			pNameBuf ++;
		}
	}
	*pNameBuf = 0;  /* ��ǰ�ļ�������·���Ľ����� */
	FileNameBuffer[ FileCount ].DirStartClust = CurrentDirStartClust;  /* ��¼��ǰĿ¼����ʼ�غ�,���ڼӿ��ļ����ٶ� */
	FileNameBuffer[ FileCount ].Attr = pFileDir -> DIR_Attr;  /* �ļ����� */
	if ( pFileDir -> DIR_Attr & ATTR_DIRECTORY ) {  /* �ж���Ŀ¼�� */
		printf( "Dir %4d#: %s\n", FileCount, FileNameBuffer[ FileCount ].Name );
	}
	else {  /* �ж����ļ��� */
		printf( "File%4d#: %s\n", FileCount, FileNameBuffer[ FileCount ].Name );
	}
	FileCount ++;  /* �ļ����� */
	if ( FileCount >= MAX_FILE_COUNT ) {  /* �ļ����ṹ������̫С,�ṹ�������� */
		CH375vFileSize = 1;  /* ǿ����ǰ����ö��,����FileOpen�����ٻص�xFileNameEnumer������ǰ����,��ֹ��������� */
		printf( "FileName Structure Full\n" );
	}
}

#else
/* V2.8, V2.9, V3.0�����ϰ汾���ӳ���� */

/* ����:�о�ָ��Ŀ¼�µ������ļ� */
UINT8	ListFile( void )
// �������mCmdParam.Open.mPathName[]ΪĿ¼���ַ���,��ʽ���ļ�����ͬ,����б���������Ŀ¼
{
	UINT16			ListCount=0;  /* �����ڼ��� */
	UINT8			status, i, c;
	P_FAT_DIR_INFO	mFileDir;
	printf( "List Directory: %s\n", mCmdParam.Open.mPathName );  /* ��ʾĿ¼�� */
	status = CH375FileOpen( );  /* ��Ŀ¼ */
	if ( status == ERR_SUCCESS ) {  /* �ɹ����ļ� */
		printf( "This is a file, not directory\n" );
		status = ERR_MISS_DIR;
	}
	else if ( status == ERR_OPEN_DIR ) {  /* �ɹ���Ŀ¼ */
		while ( 1 ) {  /* ���δ��� */
			mCmdParam.ReadX.mSectorCount = 1;  /* ����һ������ */
			mCmdParam.ReadX.mDataBuffer = FILE_DATA_BUF_ADDR;  /* ������ */
			status = CH375FileReadX( );  /* ��ȡ��ǰĿ¼�����ڵ����� */
			if ( status != ERR_SUCCESS ) break;  /* ����ʧ�� */
			if ( mCmdParam.Read.mSectorCount == 0 ) break;  /* Ŀ¼���� */
			mFileDir = FILE_DATA_BUF_ADDR;
			for ( i = mCmdParam.Read.mSectorCount * CH375vSectorSize / sizeof( FAT_DIR_INFO ); i != 0; i --, mFileDir ++ ) {  /* ����FDT������ */
				c = mFileDir -> DIR_Name[0];  /* ����ļ������ֽ� */
				if ( c == 0 ) break;  /* �ļ�Ŀ¼����� */
				else if ( c == 0xE5 ) continue;  /* ��Ŀ¼���ѱ�ɾ�������� */
				else if ( c == 0x05 ) mFileDir -> DIR_Name[0] = i = 0xE5;
				if ( ( mFileDir -> DIR_Attr & ATTR_VOLUME_ID ) == 0 ) {  /* ���Ǿ�����(�п����ǳ��ļ�����һ����) */
					ListCount++;
					if ( mFileDir -> DIR_Attr & ATTR_DIRECTORY ) {  /* Ŀ¼,���ܽ��еݹ鴦��,���Ǳ��ݺܶ��ڲ����� */
						printf( "Found %4d# directory, name: ", ListCount );  /* ��ʾ���� */
					}
					else {  /* �ļ� */
						printf( "Found %4d# file, name: ", ListCount );  /* ��ʾ���� */
					}
					for ( c = 0; c != 11; c ++ ) {  /* ��ʾ���������ļ���,ǰ11���ַ�Ϊ�ļ��� */
						printf( "%c", mFileDir -> DIR_Name[ c ] );
					}
					printf( "\n" );
				}
			}
		}
	}
	CH375FileClose( );  /* ��Ŀ¼��������ر� */
	return( status );  /* Ŀ¼�����򷵻�ERR_SUCCESS�����߳���ʱ���س������� */
}

#endif

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

		while ( CH375DiskStatus < DISK_CONNECT ) {  /* ��ѯCH375�жϲ������ж�״̬,�ȴ�U�̲��� */
			CH375DiskConnect( );
			mDelaymS( 100 );
		}
		LED_OUT_ACT( );  /* LED�� */
		mDelaymS( 200 );  /* ��ʱ,��ѡ����,�е�USB�洢����Ҫ��ʮ�������ʱ */

/* ���ڼ�⵽USB�豸��,���ȴ�100*50mS,��Ҫ�����ЩMP3̫��,���ڼ�⵽USB�豸��������DISK_MOUNTED��,���ȴ�5*50mS,��Ҫ���DiskReady������ */
		for ( i = 0; i < 100; i ++ ) {  /* ��ȴ�ʱ��,100*50mS */
			mDelaymS( 50 );
			printf( "Ready ?\n" );
			if ( CH375DiskReady( ) == ERR_SUCCESS ) break;  /* ��ѯ�����Ƿ�׼���� */
			if ( CH375DiskStatus < DISK_CONNECT ) break;  /* ��⵽�Ͽ�,���¼�Ⲣ��ʱ */
			if ( CH375DiskStatus >= DISK_MOUNTED && i > 5 ) break;  /* �е�U�����Ƿ���δ׼����,�������Ժ���,ֻҪ�佨������MOUNTED�ҳ���5*50mS */
		}
		if ( CH375DiskStatus < DISK_CONNECT ) {  /* ��⵽�Ͽ�,���¼�Ⲣ��ʱ */
			printf( "Device gone\n" );
			continue;
		}
		if ( CH375DiskStatus < DISK_MOUNTED ) {  /* δ֪USB�豸,����USB���̡���ӡ���� */
			printf( "Unknown device\n" );
			goto UnknownUsbDevice;
		}

#if DISK_BASE_BUF_LEN
		if ( DISK_BASE_BUF_LEN < CH375vSectorSize ) {  /* ���������ݻ������Ƿ��㹻��,CH375vSectorSize��U�̵�ʵ��������С */
			printf( "Too large sector size\n" );
			goto UnknownUsbDevice;
		}
#endif

		mDelaymS( 20 );

/* ��ѯ������������ */
/*		printf( "DiskSize\n" );
		i = CH375DiskSize( );  
		mStopIfError( i );
		printf( "TotalSize = %u MB \n", (unsigned int)( mCmdParam.DiskSize.mDiskSizeSec * (CH375vSectorSize/512) / 2048 ) );  // ��ʾΪ��MBΪ��λ������
		// ԭ���㷽�� (unsigned int)( mCmdParam.DiskSize.mDiskSizeSec * CH375vSectorSize / 1000000 ) �п���ǰ����������˺������, �����޸ĳ���ʽ
*/
		LED_RUN_ACT( );  /* ��ʼ����U�� */


#if CH375_LIB_VER	>= 0x30
		printf( "List all file \n" );
		i = ListAll( );  /* ö������U���е������ļ���Ŀ¼ */
		mStopIfError( i );
#else
		printf( "List all file under Root \n" );
		mCopyCodeStringToIRAM( mCmdParam.Open.mPathName, "/" );  /* ��Ŀ¼ */
		i = ListFile( );  /* �г�Ŀ¼�µ������ļ� */
		mStopIfError( i );

		printf( "List all file under /C51 \n" );
		mCopyCodeStringToIRAM( mCmdParam.Open.mPathName, "/C51" );  /* ��Ŀ¼ */
		i = ListFile( );  /* �г�Ŀ¼�µ������ļ� */
		if ( i == ERR_MISS_FILE ) {
			printf( "Sub-Directory not found \n" );
		}
		else mStopIfError( i );
#endif

		LED_RUN_INACT( );
UnknownUsbDevice:
		printf( "Take out\n" );
		while ( CH375DiskStatus >= DISK_CONNECT ) {  /* ��ѯCH375�жϲ������ж�״̬,�ȴ�U�̰γ� */
			CH375DiskConnect( );
			mDelaymS( 100 );
		}
		LED_OUT_INACT( );  /* LED�� */
		mDelaymS( 200 );
	}
}