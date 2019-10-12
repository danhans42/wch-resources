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

/* MCS-51��Ƭ��C���Ե�U���ļ���дʾ������ */
/* �������������MCS51��Ƭ��
   1. ����MCS51��Ƭ��,��������С��0.3uS,�����ڻ�������Ϊ12��ʱ��ʱ��ʱ��Ƶ�ʴ���40MHz
   2. ������MCS51��Ƭ��,����ͨI/O����ģ��8λ�������߶�д,��CH375֮����ò�������
   3. ��Ƭ����CH375֮����ô�������
*/
/* ������������ʾ��ADCģ���ɼ������ݱ��浽U���ļ�MY_ADC.TXT�� */
/* CH375��INT#���Ų��ò�ѯ��ʽ����,��������ͨI/O����ģ��8λ�������߶�д,ͬʱ�ṩ��������ʾ��,
   ���ֽ�Ϊ��λ��дU���ļ�,��д�ٶȽ�����ģʽ��,���������ֽ�ģʽ��д�ļ�����Ҫ�ļ����ݻ�����FILE_DATA_BUF,
   �����ܹ�ֻ��Ҫ600�ֽڵ�RAM,�����ڵ�Ƭ��Ӳ����Դ���ޡ�������С���Ҷ�д�ٶ�Ҫ�󲻸ߵ�ϵͳ */


/* C51   CH375HFT.C */
/* LX51  CH375HFT.OBJ , CH375HF5.LIB */
/* OHX51 CH375HFT */

#include <reg52.h>
#include <stdio.h>

#define	MAX_BYTE_IO				35		/* ���ֽ�Ϊ��λ���ζ�д�ļ�ʱ����󳤶�,Ĭ��ֵ��29,ֵ����ռ���ڴ��,ֵС�򳬹��ó��ȱ���ֶ�ζ�д */

/* ���¶������ϸ˵���뿴CH375HF5.H�ļ� */
#define LIB_CFG_FILE_IO			1		/* �ļ���д�����ݵĸ��Ʒ�ʽ,0Ϊ"�ⲿ�ӳ���",1Ϊ"�ڲ�����" */
#define LIB_CFG_INT_EN			0		/* CH375��INT#�������ӷ�ʽ,0Ϊ"��ѯ��ʽ",1Ϊ"�жϷ�ʽ" */
/*#define LIB_CFG_UPD_SIZE		1*/		/* ���������ݺ��Ƿ��Զ������ļ�����: 0Ϊ"������",1Ϊ"�Զ�����" */
/* Ĭ�������,���������/�ֽ�����Ϊ0��ôCH375FileWrite/CH375ByteWriteֻ����д�����ݶ����޸��ļ�����,
   �����Ҫÿ��д�����ݺ���Զ��޸�/�����ļ�����,��ô����ʹȫ�ֱ���CH375LibConfig��λ4Ϊ1,
   �����ʱ�䲻д��������Ӧ�ø����ļ�����,��ֹͻȻ�ϵ��ǰ��д����������ļ����Ȳ����,
   ���ȷ������ͻȻ�ϵ���ߺ���ܿ������ݲ���д���򲻱ظ����ļ�����,��������ٶȲ�����U�����(U���ڲ����ڴ���������,����Ƶ����д) */

/* ֻʹ�õ�Ƭ�����õ�1KB�ⲿRAM: 0000H-01FFH Ϊ���̶�д������, ���ֽ�Ϊ��λ��д�ļ�����Ҫ�ļ����ݶ�д������FILE_DATA_BUF */
#define	DISK_BASE_BUF_ADDR		0x0000	/* �ⲿRAM�Ĵ������ݻ���������ʼ��ַ,�Ӹõ�Ԫ��ʼ�Ļ���������ΪSECTOR_SIZE */
#define DISK_BASE_BUF_LEN		2048	/* Ĭ�ϵĴ������ݻ�������СΪ512�ֽ�,����ѡ��Ϊ2048����4096��֧��ĳЩ��������U��,Ϊ0���ֹ��.H�ļ��ж��建��������Ӧ�ó�����pDISK_BASE_BUF��ָ�� */
#define FILE_DATA_BUF_ADDR		0x0000	/* �ⲿRAM���ļ����ݻ���������ʼ��ַ,���������Ȳ�С��һ�ζ�д�����ݳ���,�ֽ�ģʽ���øû����� */
/* ���ڵ�Ƭ�����õ��ⲿRAMֻ��1KB, ��Щ��Ƭ����Ҫȥ��256�ֽ��ڲ�RAM, ֻʣ��768�ֽڵ��ⲿRAM, ����ǰ512�ֽ���CH375�ӳ������ڴ������ݻ��� */
#define FILE_DATA_BUF_LEN		0x0800	/* �ⲿRAM���ļ����ݻ�����,���������Ȳ�С��һ�ζ�д�����ݳ���,�ֽ�ģʽ���øû����� */

#define CH375_INT_WIRE			INT0	/* P3.2, INT0, CH375���ж���INT#����,����CH375��INT#����,���ڲ�ѯ�ж�״̬ */

#define NO_DEFAULT_CH375_F_ENUM		1		/* δ����CH375FileEnumer����ʽ�ֹ�Խ�Լ���� */
#define NO_DEFAULT_CH375_F_QUERY	1		/* δ����CH375FileQuery����ʽ�ֹ�Խ�Լ���� */

#include "..\CH375HF5.H"

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

/* ������I/O����ģ�����CH375�Ĳ��ڶ�дʱ�� */
/* �����е�Ӳ�����ӷ�ʽ����(ʵ��Ӧ�õ�·���Բ����޸�����3�����ڶ�д�ӳ���) */
/* ��Ƭ��������    CH375оƬ������
      P3.2                 INT#
      P1.0                 A0
      P1.1                 CS#    ���ģ����Ĳ�����ֻ��CH375,��ôCS#����ֱ�ӽӵ͵�ƽ,ǿ��Ƭѡ
      P1.2                 WR#
      P1.3                 RD#
      P0(8λ�˿�)         D7-D0       */
sbit	CH375_A0	=	P1^0;
sbit	CH375_CS	=	P1^1;
sbit	CH375_WR	=	P1^2;
sbit	CH375_RD	=	P1^3;

void mDelay1_2uS( )  /* ������ʱ1.2uS,���ݵ�Ƭ����Ƶ���� */
{
	return;
}

void CH375_PORT_INIT( )  /* ����ʹ��ͨ��I/Oģ�鲢�ڶ�дʱ��,���Խ��г�ʼ�� */
{
	CH375_CS = 1;
	CH375_WR = 1;
	CH375_RD = 1;
	CH375_A0 = 0;
	P0 = 0xFF;  /* �������� */
}

void xWriteCH375Cmd( UINT8 mCmd )		/* �ⲿ����ı�CH375�������õ��ӳ���,��CH375д���� */
{
	mDelay1_2uS( ); mDelay1_2uS( );  /* ������ʱ1uS */
	P0 = mCmd;  /* ��CH375�Ĳ���������� */
	CH375_A0 = 1;
	CH375_CS = 0;
	CH375_WR = 0;  /* �����Чд�����ź�, дCH375оƬ������˿� */
	CH375_CS = 0;  /* �ò���������,������ʱ,CH375Ҫ���д������ȴ���100nS */
	CH375_WR = 1;  /* �����Ч�Ŀ����ź�, ��ɲ���CH375оƬ */
	CH375_CS = 1;
	CH375_A0 = 0;
	P0 = 0xFF;  /* ��ֹ������� */
	mDelay1_2uS( ); mDelay1_2uS( );  /* ������ʱ2uS */
}

void xWriteCH375Data( UINT8 mData )		/* �ⲿ����ı�CH375�������õ��ӳ���,��CH375д���� */
{
	P0 = mData;  /* ��CH375�Ĳ���������� */
	CH375_A0 = 0;
	CH375_CS = 0;
	CH375_WR = 0;  /* �����Чд�����ź�, дCH375оƬ�����ݶ˿� */
	CH375_CS = 0;  /* �ò���������,������ʱ,CH375Ҫ���д������ȴ���100nS */
	CH375_WR = 1;  /* �����Ч�Ŀ����ź�, ��ɲ���CH375оƬ */
	CH375_CS = 1;
	P0 = 0xFF;  /* ��ֹ������� */
	mDelay1_2uS( );  /* ������ʱ1.2uS */
}

UINT8 xReadCH375Data( void )			/* �ⲿ����ı�CH375�������õ��ӳ���,��CH375������ */
{
	UINT8	mData;
	mDelay1_2uS( );  /* ������ʱ1.2uS */
	P0 = 0xFF;  /* ���� */
	CH375_A0 = 0;
	CH375_CS = 0;
	CH375_RD = 0;  /* �����Чд�����ź�, ��CH375оƬ�����ݶ˿� */
	CH375_CS = 0;  /* �ò���������,������ʱ,CH375Ҫ���д������ȴ���100nS */
	mData = P0;  /* ��CH375�Ĳ����������� */
	CH375_RD = 1;  /* �����Ч�Ŀ����ź�, ��ɲ���CH375оƬ */
	CH375_CS = 1;
	return( mData );
}

/* �����Ƭ����CH375�Ǵ�������,��ô�ο�������ӳ���
void CH375_PORT_INIT( ) {
	SCON = 0xD0;  ���ô���Ϊ9λ����
���ò����ʺ��������ڲ���
}

void xWriteCH375Cmd( UINT8 mCmd ) {
	TI = 0;
	TB8 = 1;
	SBUF = mCmd;
	while ( TI == 0 );
}

void xWriteCH375Data( UINT8 mData ) {
	TI = 0;
	TB8 = 0;
	SBUF = mData;
	while ( TI == 0 );
}

UINT8 xReadCH375Data( void ) {
	while ( RI == 0 );
	RI = 0;
	return( SBUF );
}
*/

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
	UINT8	i, month, hour;
	UINT16	date, adc, len;
	CH375_PORT_INIT( );  /* ����ʹ��ͨ��I/Oģ�鲢�ڶ�дʱ��,���Խ��г�ʼ�� */
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

/* ���MY_ADC.TXT�ļ��Ѿ��������������ݵ�β��,������������½��ļ� */
		printf( "Open\n" );
		mCopyCodeStringToIRAM( mCmdParam.Open.mPathName, "/MY_ADC.TXT" );  /* �ļ���,���ļ��ڸ�Ŀ¼�� */
		i = CH375FileOpen( );  /* ���ļ� */
		if ( i == ERR_SUCCESS ) {  /* �ļ����ڲ����Ѿ�����,�ƶ��ļ�ָ�뵽β���Ա��������� */
			printf( "File size = %ld\n", CH375vFileSize );  /* V1.5�����ӳ�����ڳɹ����ļ���,ȫ�ֱ���CH375vFileSize�����ļ���ǰ���� */
			printf( "Locate tail\n" );
			mCmdParam.ByteLocate.mByteOffset = 0xffffffff;  /* �Ƶ��ļ���β�� */
			i = CH375ByteLocate( );
			mStopIfError( i );
		}
		else if ( i == ERR_MISS_FILE ) {  /* û���ҵ��ļ�,�����½��ļ� */
			LED_WR_ACT( );  /* д���� */
			printf( "Create\n" );
//			mCopyCodeStringToIRAM( mCmdParam.Create.mPathName, "/MY_ADC.TXT" );  /* �ļ���,���ļ��ڸ�Ŀ¼��,�ղ��Ѿ��ṩ��CH375FileOpen */
			i = CH375FileCreate( );  /* �½��ļ�����,����ļ��Ѿ���������ɾ�������½� */
			mStopIfError( i );
		}
		else mStopIfError( i );  /* ���ļ�ʱ���� */
		LED_WR_ACT( );  /* д���� */
		printf( "Write begin\n" );
		i = sprintf( mCmdParam.ByteWrite.mByteBuffer, "��ǰ�ļ�����= %ld �ֽ�\xd\xa", CH375vFileSize );
		mCmdParam.ByteWrite.mByteCount = i;  /* ָ������д����ֽ���,���ζ�д�ĳ��Ȳ��ܳ���MAX_BYTE_IO */
		i = CH375ByteWrite( );  /* ���ֽ�Ϊ��λ���ļ�д������,���ζ�д�ĳ��Ȳ��ܳ���MAX_BYTE_IO */
		mStopIfError( i );
		printf( "Write ADC data\n" );
		for ( hour = 8; hour != 20; hour ++  ) {  /* ��ѭ����ʽ����12������ */
			TR0=1;  /* �ö�ʱ��0�ļ���ֵ����ADC���� */
			month = 5;  /* �ٶ���5�� */
			date = TL1 & 0x1F;  /* ��Ϊ���԰���û��ʵʱʱ��оƬ,�����ö�ʱ��1�ļ������������ʾ */
/*			adc = get_adc_data( ); */
			adc = ( (UINT16)TH0 << 8 ) | TL0;  /* ��Ϊ���԰���û��ADC,�����ö�ʱ��0�ļ�������ADC������ʾ */
			len = sprintf( mCmdParam.ByteWrite.mByteBuffer, "%02d.%02d.%02d ADC=%u\xd\xa", (UINT16)month, date, (UINT16)hour, adc );  /* �����������ݸ�ʽΪһ���ַ��� */
			mCmdParam.ByteWrite.mByteCount = (unsigned char)len;  /* ָ������д����ֽ���,���ܳ���MAX_BYTE_IO,�������û������ֶ��д�� */
			i = CH375ByteWrite( );  /* ���ֽ�Ϊ��λ���ļ�д������,���ζ�д�ĳ��Ȳ��ܳ���MAX_BYTE_IO */
			mStopIfError( i );
		}
/*		mCmdParam.ByteWrite.mByteCount = 0;  ���ȫ�ֱ���CH375LibConfig��λ4Ϊ0,����ָ��д��0�ֽ�,����ˢ���ļ��ĳ���,
		CH375ByteWrite( );  ���ֽ�Ϊ��λ���ļ�д������,��Ϊ��0�ֽ�д��,����ֻ���ڸ����ļ��ĳ���,���׶���д�����ݺ�,���������ְ취�����ļ�����
        ���ȫ�ֱ���CH375LibConfig��λ4Ϊ1,��ÿ��д�����ݺ���Զ��޸�/�����ļ�����,�Ӷ���������������д0�ֽ�����: CH375LibConfig |= 0x10; */
		printf( "Write end\n" );
		i = mCopyCodeStringToIRAM( mCmdParam.ByteWrite.mByteBuffer, "�����ADC���ݵ��˽���\xd\xa" );
		mCmdParam.ByteWrite.mByteCount = i;  /* ָ������д����ֽ���,���ζ�д�ĳ��Ȳ��ܳ���MAX_BYTE_IO */
		i = CH375ByteWrite( );  /* ���ֽ�Ϊ��λ���ļ�д������,���ζ�д�ĳ��Ȳ��ܳ���MAX_BYTE_IO */
		mStopIfError( i );
/*		printf( "Modify\n" );  ���ʵ�ʲ�Ʒ����ʵʱʱ��,���Ը�����Ҫ���ļ������ں�ʱ���޸�Ϊʵ��ֵ
		mCmdParam.Modify.mFileAttr = 0xff;   �������: �µ��ļ�����,Ϊ0FFH���޸�
		mCmdParam.Modify.mFileTime = MAKE_FILE_TIME( 16, 32, 09 );   �������: �µ��ļ�ʱ��: 16:32:09
		mCmdParam.Modify.mFileDate = MAKE_FILE_DATE( 2004, 5, 18 );  �������: �µ��ļ�����: 2004.05.18
		mCmdParam.Modify.mFileSize = 0xffffffff;   �������: �µ��ļ�����,���ֽ�Ϊ��λд�ļ�Ӧ���ɳ����ر��ļ�ʱ�Զ����³���,���Դ˴����޸�
		i = CH375FileModify( );   �޸ĵ�ǰ�ļ�����Ϣ,�޸�����
		mStopIfError( i );
*/
		printf( "Close\n" );
		mCmdParam.Close.mUpdateLen = 1;  /* �Զ������ļ�����,���ֽ�Ϊ��λд�ļ�,�����ó����ر��ļ��Ա��Զ������ļ����� */
		i = CH375FileClose( );  /* �ر��ļ� */
		mStopIfError( i );
		LED_WR_INACT( );
		LED_RUN_INACT( );
		printf( "Take out\n" );
		while ( CH375DiskStatus != DISK_DISCONNECT ) xQueryInterrupt( );  /* ��ѯCH375�жϲ������ж�״̬,�ȴ�U�̰γ� */
		LED_OUT_INACT( );  /* LED�� */
		mDelay100mS( );
		mDelay100mS( );
	}
}