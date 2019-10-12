/* 2004.06.05
****************************************
**  Copyright  (C)  W.ch  1999-2004   **
**  Web:  http://www.winchiphead.com  **
****************************************
**  USB Host File Interface for CH375 **
**  TC2.0@PC, KEIL_C_2.41@ARM         **
****************************************
*/
/* CH375 �����ļ�ϵͳ�ӿ� */
/* ֧��: FAT12/FAT16/FAT32 */

/* ARM��Ƭ��C���Ե�U���ļ���дʾ������ */
/* �ó���U���е�/C51/CH375HFT.C�ļ��е�Сд��ĸת�ɴ�д��ĸ��, д���½����ļ�NEWFILE.TXT��,
   ����Ҳ���ԭ�ļ�CH375HFT.C, ��ô�ó�����ʾC51��Ŀ¼��������CH375��ͷ���ļ���, ���½�NEWFILE.TXT�ļ���д����ʾ��Ϣ,
   ����Ҳ���C51��Ŀ¼, ��ô�ó�����ʾ��Ŀ¼�µ������ļ���, ���½�NEWFILE.TXT�ļ���д����ʾ��Ϣ
*/
/* CH375��INT#���Ų��ò�ѯ��ʽ����, ���ݸ��Ʒ�ʽΪ"�ڲ�����", ������������Philips LPC2114��Ƭ��, ����0��������Ϣ,9600bps */
/* ENDIAN = "little" */

/* CA CH375HFT.C ARM */
/* LA CH375HFT.OBJ, CH375HFM.LIB */
/* OHA CH375HFT */

#include <Philips\LPC21xx.H>						/* Ŀ����ר�ó�ʼ������ */

#include <string.h>
#include <stdio.h>


/* ���¶������ϸ˵���뿴CH375HFM.H�ļ� */
#define LIB_CFG_FILE_IO			1		/* �ļ���д�����ݵĸ��Ʒ�ʽ,0Ϊ"�ⲿ�ӳ���",1Ϊ"�ڲ�����" */
#define LIB_CFG_INT_EN			0		/* CH375��INT#�������ӷ�ʽ,0Ϊ"��ѯ��ʽ",1Ϊ"�жϷ�ʽ" */

/* ��Ƭ����RAM����,����CH375�ӳ�����512�ֽ�,ʣ��RAM���ֿ��������ļ���д���� */
#define FILE_DATA_BUF_LEN		0x2000	/* �ⲿRAM���ļ����ݻ�����,���������Ȳ�С��һ�ζ�д�����ݳ��� */
/* ���׼��ʹ��˫�����������д,��ô��Ҫ����FILE_DATA_BUF_LEN,�����ڲ�����ָ����������ַ,��CH375FileReadX����CH375FileRead,��CH375FileWriteX����CH375FileWrite */

#define CH375_INT_WIRE			( IO0PIN & 0x08 )	/* P0.3, CH375���ж���INT#����,����CH375��INT#����,���ڲ�ѯ�ж�״̬ */

#define NO_DEFAULT_CH375_F_ENUM		1		/* δ����CH375FileEnumer����ʽ�ֹ�Խ�Լ���� */
#define NO_DEFAULT_CH375_F_QUERY	1		/* δ����CH375FileQuery����ʽ�ֹ�Խ�Լ���� */

#include "..\CH375HFM.H"

/* ��ЩARM��Ƭ���ṩ����ϵͳ����,��ôֱ�ӽ�CH375������ϵͳ������,��8λI/O��ʽ���ж�д */
/* ����ʹ�õ�LPC2114������ϵͳ����,������I/O����ģ�����CH375�Ĳ��ڶ�дʱ�� */
/* �����е�Ӳ�����ӷ�ʽ����(ʵ��Ӧ�õ�·���Բ����޸�����3�����ڶ�д�ӳ���) */
/* LPC2114��Ƭ��������    CH375оƬ������
         P0.3                 INT#
         P0.4                 A0
         P0.7                 CS#
         P0.6                 WR#
         P0.5                 RD#
  P0.15-P0.8(8λ�˿�)         D7-D0       */

void mDelay1_2uS( )  /* ������ʱ1.2uS,���ݵ�Ƭ����Ƶ���� */
{
	UINT32	i;
	for ( i = 10; i != 0; i -- );  /* ��������ģ��I/O������ֻ��������ʱ */
}

void CH375_PORT_INIT( )  /* ����ʹ��ͨ��I/Oģ�鲢�ڶ�дʱ��,���Խ��г�ʼ�� */
{
	IO0SET |= 0x000000E0;  /* ����CS,WR,RDĬ��Ϊ�ߵ�ƽ */
	IO0DIR &= 0xFFFF00F7;  /* ����8λ���ں�INT#Ϊ���� */
	IO0DIR |= 0x000000F0;  /* ����CS,WR,RD,A0Ϊ��� */
}

void xWriteCH375Cmd( UINT8 mCmd )		/* �ⲿ����ı�CH375�������õ��ӳ���,��CH375д���� */
{
	mDelay1_2uS( ); mDelay1_2uS( );  /* ������ʱ1uS */
	IO0CLR |= 0x0000FF00;  /* �岢����� */
	IO0SET |= ( (UINT32)mCmd << 8 ) | 0x00000010;  /* ��CH375�Ĳ����������, ���A0(P0.4)=1; */
	IO0DIR |= 0x0000FFF0;  /* д���������������, ����CS,WR,RD,A0Ϊ��� */
	IO0CLR |= 0x000000C0;  /* �����Чд�����ź�, дCH375оƬ������˿�, A0(P0.4)=1; CS(P0.7)=0; WR=(P0.6)=0; RD(P0.5)=1; */
	IO0DIR = IO0DIR; IO0DIR = IO0DIR;  /* �ò���������,������ʱ,CH375Ҫ���д������ȴ���100nS */
	IO0SET |= 0x000000E0;  /* �����Ч�Ŀ����ź�, ��ɲ���CH375оƬ, A0(P0.4)=1; CS(P0.7)=1; WR=(P0.6)=1; RD(P0.5)=1; */
	IO0CLR |= 0x00000010;  /* ���A0(P0.4)=0; ��ѡ���� */
	IO0DIR &= 0xFFFF00FF;  /* ��ֹ������� */
	mDelay1_2uS( ); mDelay1_2uS( );  /* ������ʱ2uS */
}

void xWriteCH375Data( UINT8 mData )		/* �ⲿ����ı�CH375�������õ��ӳ���,��CH375д���� */
{
	IO0CLR |= 0x0000FF00;  /* �岢����� */
	IO0SET |= (UINT32)mData << 8;  /* ��CH375�Ĳ���������� */
	IO0DIR |= 0x0000FF00;  /* д��������������� */
	IO0CLR |= 0x000000D0;  /* �����Чд�����ź�, дCH375оƬ�����ݶ˿�, A0(P0.4)=0; CS(P0.7)=0; WR=(P0.6)=0; RD(P0.5)=1; */
	IO0DIR = IO0DIR; IO0DIR = IO0DIR;  /* �ò���������,������ʱ,CH375Ҫ���д������ȴ���100nS */
	IO0SET |= 0x000000E0;  /* �����Ч�Ŀ����ź�, ��ɲ���CH375оƬ, A0(P0.4)=0; CS(P0.7)=1; WR=(P0.6)=1; RD(P0.5)=1; */
	IO0DIR &= 0xFFFF00FF;  /* ��ֹ������� */
	mDelay1_2uS( );  /* ������ʱ1.2uS */
}

UINT8 xReadCH375Data( void )			/* �ⲿ����ı�CH375�������õ��ӳ���,��CH375������ */
{
	UINT8	mData;
	mDelay1_2uS( );  /* ������ʱ1.2uS */
	IO0DIR &= 0xFFFF00FF;  /* ������������������ */
	IO0CLR |= 0x000000B0;  /* �����Ч�������ź�, ��CH375оƬ�����ݶ˿�, A0(P0.4)=0; CS(P0.7)=0; WR=(P0.6)=1; RD(P0.5)=0; */
	IO0DIR = IO0DIR; IO0DIR = IO0DIR;  /* �ò���������,������ʱ,CH375Ҫ���д������ȴ���100nS */
	mData = (UINT8)( IO0PIN >> 8 );  /* ��CH375�Ĳ���P0.15-P0.8�������� */
	IO0SET |= 0x000000E0;  /* �����Ч�Ŀ����ź�, ��ɲ���CH375оƬ, A0(P0.4)=0; CS(P0.7)=1; WR=(P0.6)=1; RD(P0.5)=1; */
	return( mData );
}

/* ��P0.2����һ��LED���ڼ����ʾ����Ľ���,�͵�ƽLED�� */
#define LED_OUT_INIT( )		{ IO0DIR |= 0x04; }	/* P0.2 �ߵ�ƽΪ������� */
#define LED_OUT_ACT( )		{ IO0CLR |= 0x04; }	/* P0.2 �͵�ƽ����LED��ʾ */
#define LED_OUT_INACT( )	{ IO0SET |= 0x04; }	/* P0.2 �͵�ƽ����LED��ʾ */

/* ��ʱָ������ʱ��,���ݵ�Ƭ����Ƶ����,����ȷ */
void	mDelaymS( UINT32 ms )
{
	UINT32	i;
	while ( ms -- ) for ( i = 25000; i != 0; i -- );
}

/* ������״̬,�����������ʾ������벢ͣ�� */
void	mStopIfError( UINT8 iError )
{
	if ( iError == ERR_SUCCESS ) return;  /* �����ɹ� */
	printf( "Error: %02X\n", (UINT16)iError );  /* ��ʾ���� */
	while ( 1 ) {
		LED_OUT_ACT( );  /* LED��˸ */
		mDelaymS( 100 );
		LED_OUT_INACT( );
		mDelaymS( 100 );
	}
}

/* Ϊprintf��getkey���������ʼ������ */
/*     �����ӵ�����             */
/* ϵͳ����, Fosc��Fcclk��Fcco��Fpclk���붨��*/
#define Fosc            11059200                    //����Ƶ��,10MHz~25MHz��Ӧ����ʵ��һ��
#define Fcclk           (Fosc * 4)                  //ϵͳƵ�ʣ�����ΪFosc��������(1~32)����<=60MHZ
#define Fcco            (Fcclk * 4)                 //CCOƵ�ʣ�����ΪFcclk��2��4��8��16������ΧΪ156MHz~320MHz
#define Fpclk           (Fcclk / 4) * 1             //VPBʱ��Ƶ�ʣ�ֻ��Ϊ(Fcclk / 4)��1��2��4��
void	mInitSTDIO( )
{
	UINT32	x;
	PINSEL0 = PINSEL0 & 0xFFFFFFF0 | 0x00000005;  // ����I/O���ӵ�UART0
	U0LCR = 0x80;                    // DLABλ��1
	x = (Fpclk>>4)/9600;             // 9600bps
	U0DLM = x>>8;
	U0DLL = x&0xff;
	U0LCR = 0x03;                    // 8λ����λ,1λֹͣλ,����żУ��
	U0FCR = 0x01;
}

/* ͨ��������������Ϣ */
/* ����KEIL C 2.41�����⣬�޷�ʵ��PUTCHAR�Ĵ��棬����KEIL�ṩ��HELLO���Ӿ��޷�������
   ���Ա������޷�ʵ�֣���������ѯKEIL������Ա����KEIL�����������������PUTCHAR�ӳ���ȥ��ע��
int		putchar( int ch )
{
	U0THR = ch;                           // ��������
	while( ( U0LSR & 0x20 ) == 0 );       // �ȴ����ݷ���
	return( ch );
}*/

int		main( ) {
	UINT8	i, c, SecCount;
	UINT16	NewSize, count;  /* ��ΪRAM��������,����NewSize����Ϊ16λ,ʵ��������ļ��ϴ�,Ӧ�÷ּ��ζ�д���ҽ�NewSize��ΪUINT32�Ա��ۼ� */
	UINT8	*pCodeStr;
	CH375_PORT_INIT( );
	LED_OUT_INIT( );
	LED_OUT_ACT( );  /* ������LED��һ����ʾ���� */
	mDelaymS( 100 );  /* ��ʱ100���� */
	LED_OUT_INACT( );
	mInitSTDIO( );  /* Ϊ���ü����ͨ�����ڼ����ʾ���� */
	printf( "Start\n" );

	i = CH375LibInit( );  /* ��ʼ��CH375������CH375оƬ,�����ɹ�����0 */
	mStopIfError( i );
/* ������·��ʼ�� */

	while ( 1 ) {
		printf( "Wait Udisk\n" );
//		while ( CH375DiskStatus != DISK_CONNECT ) xQueryInterrupt( );  /* ��ѯCH375�жϲ������ж�״̬,�ȴ�U�̲��� */
		while ( CH375DiskStatus < DISK_CONNECT ) {  /* ��ѯCH375�жϲ������ж�״̬,�ȴ�U�̲��� */
			if ( CH375DiskConnect( ) == ERR_SUCCESS ) break;  /* ���豸�����򷵻سɹ�,CH375DiskConnectͬʱ�����ȫ�ֱ���CH375DiskStatus */
			mDelaymS( 100 );
		}
		LED_OUT_ACT( );  /* LED�� */
		mDelaymS( 200 );  /* ��ʱ,��ѡ����,�е�USB�洢����Ҫ��ʮ�������ʱ */

/* ���U���Ƿ�׼����,��ЩU�̲���Ҫ��һ��,����ĳЩU�̱���Ҫִ����һ�����ܹ��� */
		for ( i = 0; i < 10; i ++ ) {  /* �е�U�����Ƿ���δ׼����,�������Ա����� */
			mDelaymS( 100 );
			printf( "Ready ?\n" );
			if ( CH375DiskReady( ) == ERR_SUCCESS ) break;  /* ��ѯ�����Ƿ�׼���� */
		}
/* ��ѯ������������ */
/*		printf( "DiskSize\n" );
		i = CH375DiskSize( );  
		mStopIfError( i );
		printf( "TotalSize = %d MB \n", (unsigned int)( mCmdParam.DiskSize.mDiskSizeSec >> 11 ) );  ��ʾΪ��MBΪ��λ������
*/

/* ��ȡԭ�ļ� */
		printf( "Open\n" );
		strcpy( (char *)mCmdParam.Open.mPathName, "\\C51\\CH375HFT.C" );  /* �ļ���,���ļ���C51��Ŀ¼�� */
		i = CH375FileOpen( );  /* ���ļ� */
		if ( i == ERR_MISS_DIR || i == ERR_MISS_FILE ) {  /* û���ҵ��ļ� */
/* �г��ļ� */
			if ( i == ERR_MISS_DIR ) pCodeStr = (UINT8 *)"\\*";  /* C51��Ŀ¼���������г���Ŀ¼�µ��ļ� */
			else pCodeStr = (UINT8 *)"\\C51\\CH375*";  /* CH375HFT.C�ļ����������г�\C51��Ŀ¼�µ���CH375��ͷ���ļ� */
			printf( "List file %s\n", pCodeStr );
			for ( c = 0; c < 255; c ++ ) {  /* �������ǰ255���ļ� */
				strcpy( (char *)mCmdParam.Open.mPathName, (char *)pCodeStr );  /* �����ļ���,*Ϊͨ���,�����������ļ�������Ŀ¼ */
				i = strlen( (char const *)mCmdParam.Open.mPathName );  /* �����ļ�������,�Դ����ļ��������� */
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
			pCodeStr = (UINT8 *)"�Ҳ���/C51/CH375HFT.C�ļ�\xd\n";
			for ( i = 0; i != 255; i ++ ) {
				if ( ( FILE_DATA_BUF[i] = *pCodeStr ) == 0 ) break;
				pCodeStr++;
			}
			NewSize = i;  /* ���ļ��ĳ��� */
			SecCount = 1;  /* (NewSize+511)/512, �����ļ���������,��Ϊ��д��������Ϊ��λ�� */
		}
		else {  /* �ҵ��ļ����߳��� */
			mStopIfError( i );
/*			printf( "Query\n" );
			i = CH375FileQuery( );  ��ѯ��ǰ�ļ�����Ϣ
			mStopIfError( i );*/
			printf( "Read\n" );
			if ( CH375vFileSize > FILE_DATA_BUF_LEN ) {  /* ������ʾ���õ�62256ֻ��32K�ֽ�,����CH375�ӳ�����512�ֽ�,����ֻ��ȡ������63������,Ҳ���ǲ�����32256�ֽ� */
				SecCount = FILE_DATA_BUF_LEN / 512;  /* ������ʾ���õ�62256ֻ��32K�ֽ�,����CH375�ӳ�����512�ֽ�,����ֻ��ȡ������63������,Ҳ���ǲ�����32256�ֽ� */
				NewSize = FILE_DATA_BUF_LEN;  /* ����RAM�����������Ƴ��� */
			}
			else {  /* ���ԭ�ļ���С,��ôʹ��ԭ���� */
				SecCount = ( CH375vFileSize + 511 ) >> 9;  /* (CH375vFileSize+511)/512, �����ļ���������,��Ϊ��д��������Ϊ��λ��,�ȼ�511��Ϊ�˶����ļ�β������1�������Ĳ��� */
				NewSize = (UINT16)CH375vFileSize;  /* ԭ�ļ��ĳ��� */
			}
			printf( "Size=%ld, Len=%d, Sec=%d\n", CH375vFileSize, NewSize, (UINT16)SecCount );
			mCmdParam.Read.mSectorCount = SecCount;  /* ��ȡȫ������,�������60��������ֻ��ȡ60������ */
/*			current_buffer = & FILE_DATA_BUF[0];  ����ļ���д�����ݵĸ��Ʒ�ʽΪ"�ⲿ�ӳ���",��ô��Ҫ���ô�����ݵĻ���������ʼ��ַ */
			CH375vFileSize += 511;  /* Ĭ�������,��������ʽ��ȡ����ʱ,�޷������ļ�β������1�������Ĳ���,���Ա�����ʱ�Ӵ��ļ������Զ�ȡβ����ͷ */
			i = CH375FileRead( );  /* ���ļ���ȡ���� */
			CH375vFileSize -= 511;  /* �ָ�ԭ�ļ����� */
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
		CH375FileRead();   ֱ�Ӷ�ȡ���ļ��ĵ�(512*3)���ֽڿ�ʼ������,ǰ3������������

	    ���ϣ�������������ӵ�ԭ�ļ���β��,�����ƶ��ļ�ָ��
		i = CH375FileOpen( );
		mCmdParam.Locate.mSectorOffset = 0xffffffff;  �Ƶ��ļ���β��,������Ϊ��λ,���ԭ�ļ���3�ֽ�,���512�ֽڿ�ʼ����
		i = CH375FileLocate( );
		mCmdParam.Write.mSectorCount = 10;
		CH375FileWrite();   ��ԭ�ļ��ĺ�����������

ʹ��CH375FileReadX�������ж������ݻ���������ʼ��ַ
		mCmdParam.ReadX.mSectorCount = 2;
		mCmdParam.ReadX.mDataBuffer = 0x2000;  �����������ݷŵ�2000H��ʼ�Ļ�������
		CH375FileReadX();   ���ļ��ж�ȡ2��������ָ��������

ʹ��CH375FileWriteX�������ж������ݻ���������ʼ��ַ
		mCmdParam.WiiteX.mSectorCount = 2;
		mCmdParam.WriteX.mDataBuffer = 0x4600;  ��4600H��ʼ�Ļ������е�����д��
		CH375FileWriteX();   ��ָ���������е�����д��2���������ļ���
*/
			printf( "Close\n" );
			i = CH375FileClose( );  /* �ر��ļ� */
			mStopIfError( i );

			i = FILE_DATA_BUF[100];
			FILE_DATA_BUF[100] = 0;  /* ���ַ���������־,�����ʾ100���ַ� */
			printf( "Line 1: %s\n", FILE_DATA_BUF );
			FILE_DATA_BUF[100] = i;  /* �ָ�ԭ�ַ� */
			for ( count=0; count < NewSize; count ++ ) {  /* ���ļ��е�Сд�ַ�ת��Ϊ��д */
				c = FILE_DATA_BUF[ count ];
				if ( c >= 'a' && c <= 'z' ) FILE_DATA_BUF[ count ] = c - ( 'a' - 'A' );
			}
		}

#ifdef EN_DISK_WRITE  /* �ӳ����֧��д���� */
/* �������ļ� */
		printf( "Create\n" );
		strcpy( (char *)mCmdParam.Create.mPathName, "\\NEWFILE.TXT" );  /* ���ļ���,�ڸ�Ŀ¼�� */
		i = CH375FileCreate( );  /* �½��ļ�����,����ļ��Ѿ���������ɾ�������½� */
		mStopIfError( i );
		printf( "Write\n" );
		mCmdParam.Write.mSectorCount = SecCount;  /* д���������������� */
/*		current_buffer = & FILE_DATA_BUF[0];  ����ļ���д�����ݵĸ��Ʒ�ʽΪ"�ⲿ�ӳ���",��ô��Ҫ���ô�����ݵĻ���������ʼ��ַ */
		i = CH375FileWrite( );  /* ���ļ�д������ */
		mStopIfError( i );
		printf( "Modify\n" );
		mCmdParam.Modify.mFileAttr = 0xff;  /* �������: �µ��ļ�����,Ϊ0FFH���޸� */
		mCmdParam.Modify.mFileTime = 0xffff;  /* �������: �µ��ļ�ʱ��,Ϊ0FFFFH���޸�,ʹ���½��ļ�������Ĭ��ʱ�� */
		mCmdParam.Modify.mFileDate = MAKE_FILE_DATE( 2004, 5, 18 );  /* �������: �µ��ļ�����: 2004.05.18 */
		mCmdParam.Modify.mFileSize = NewSize;  /* �������: ���ԭ�ļ���С,��ô�µ��ļ�������ԭ�ļ�һ����,����RAM����,����ļ����ȴ���64KB,��ôNewSize����ΪUINT32 */
		i = CH375FileModify( );  /* �޸ĵ�ǰ�ļ�����Ϣ,�޸����ںͳ��� */
		mStopIfError( i );
		printf( "Close\n" );
		mCmdParam.Close.mUpdateLen = 0;  /* ��Ҫ�Զ������ļ�����,����Զ�����,��ô�ó�������512�ı��� */
		i = CH375FileClose( );
		mStopIfError( i );

/* ɾ��ĳ�ļ� */
/*		printf( "Erase\n" );
		strcpy( (char *)mCmdParam.Create.mPathName, "\\OLD" );  ����ɾ�����ļ���,�ڸ�Ŀ¼��
		i = CH375FileErase( );  ɾ���ļ����ر�
		if ( i != ERR_SUCCESS ) printf( "Error: %02X\n", (UINT16)i );  ��ʾ����
*/

/* ��ѯ������Ϣ */
/*		printf( "Disk\n" );
		i = CH375DiskQuery( );
		mStopIfError( i );
		printf( "Fat=%d, Total=%ld, Free=%ld\n", (UINT16)mCmdParam.Query.mDiskFat, mCmdParam.Query.mTotalSector, mCmdParam.Query.mFreeSector );
*/
#endif
		printf( "Take out\n" );
//		while ( CH375DiskStatus != DISK_DISCONNECT ) xQueryInterrupt( );  /* ��ѯCH375�жϲ������ж�״̬,�ȴ�U�̰γ� */
		while ( CH375DiskStatus >= DISK_CONNECT ) {  /* ��ѯCH375�жϲ������ж�״̬,�ȴ�U�̰γ� */
			if ( CH375DiskConnect( ) != ERR_SUCCESS ) break;
			mDelaymS( 100 );
		}
		LED_OUT_INACT( );  /* LED�� */
		mDelaymS( 200 );
	}
}