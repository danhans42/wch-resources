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
/* ������������ʾ��ADCģ���ɼ������ݱ��浽U���ļ�MY_ADC.TXT�� */
/* CH375��INT#���Ų��ò�ѯ��ʽ����, ���ݸ��Ʒ�ʽΪ"��DPTR��P2+R0����",�ٶ����,���ǲ������ڴ�������XRAM�ĵ�Ƭ��,
   ����������V1.6�����ϰ汾��CH375�ӳ����,������Ϊ��λ��дU���ļ�,��д�ٶȽ��ֽ�ģʽ��,
   ��������ģʽ������Ϊ������λ,������Ҫ�����������ݵ�Ӧ��,�����ֽ�ģʽ����,
   ��������ʾ������ģʽ�´�����������,ͬʱ��˲�������ͽϸ��ٶ�,��Ҫ�ļ����ݻ�����FILE_DATA_BUF */


/* C51   CH375HFT.C */
/* LX51  CH375HFT.OBJ , CH375HF4.LIB    �����CH375HF4����CH375HF6�Ϳ���֧��FAT32 */
/* OHX51 CH375HFT */

#include <reg52.h>
#include <stdio.h>
#include <string.h>

/* ���¶������ϸ˵���뿴CH375HF6.H�ļ� */
#define LIB_CFG_DISK_IO			3		/* ���̶�д�����ݵĸ��Ʒ�ʽ,1Ϊ"��DPTR����",2Ϊ"˫DPTR����",3Ϊ"��DPTR��P2+R0����" */
#define LIB_CFG_FILE_IO			3		/* �ļ���д�����ݵĸ��Ʒ�ʽ,0Ϊ"�ⲿ�ӳ���",1Ϊ"��DPTR����",2Ϊ"˫DPTR����",3Ϊ"��DPTR��P2+R0����" */
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

UINT16	total;	/* ��¼��ǰ������FILE_DATA_BUF�е����ݳ��� */

/* ��׼��д��U�̵��������ݽ��м��л���,��ϳɴ����ݿ�ʱ��ͨ��CH375����д��U�� */
/* �������ĺô���: ����ٶ�(��Ϊ�����ݿ�д��ʱЧ�ʸ�), ����U�����(U���ڲ����ڴ���������,����Ƶ����д) */
void	mFlushBufferToDisk( UINT8 force )
/* force = 0 ���Զ�ˢ��(��黺�����е����ݳ���,����д��,��������ʱ���ڻ�������), force != 0 ��ǿ��ˢ��(���ܻ������е������ж��ٶ�д��,ͨ����ϵͳ�ػ�ǰӦ��ǿ��д��) */
{
	UINT8	i;
	UINT32	NewSize;
	if ( force ) {  /* ǿ��ˢ�� */
		mCmdParam.Write.mSectorCount = ( total + CH375vSectorSize - 1 ) / CH375vSectorSize;  /* ���������е��ֽ���ת��Ϊ������(����CH375vSectorSize),���ȼ���CH375vSectorSize-1����ȷ��д��������ͷ���� */
		i = CH375FileWrite( );  /* ������Ϊ��λ���ļ�д������,д�뻺�����е���������,��������ͷ */
		mStopIfError( i );
/* ��ЩU�̿��ܻ�Ҫ����д���ݺ�ȴ�һ����ܼ�������,����,�����ĳЩU���з������ݶ�ʧ����,������ÿ��д�����ݺ�������ʱ�ټ��� */
		mDelaymS( 1 );  /* д����ʱ,��ѡ��,�����U�̲���Ҫ */
		memcpy( FILE_DATA_BUF+0, FILE_DATA_BUF+(total & ~ ( CH375vSectorSize - 1 ) ), total & ( CH375vSectorSize - 1 ) );  /* ���ղ���д��U�̵���ͷ���ݸ��Ƶ���������ͷ�� */
		total &= CH375vSectorSize - 1;  /* ��������ֻʣ�¸ղ���д��U�̵���ͷ����,���������ڻ���������Ϊ�˷����Ժ��������׷������ */
		if ( total ) NewSize = CH375vFileSize - CH375vSectorSize + total;  /* ������Ϊ��λ,����ͷ����,������������ļ�����(��Ч���ݵĳ���) */
		else NewSize = CH375vFileSize;  /* ������Ϊ��λ,û����ͷ����,�ļ�������CH375vSectorSize�ı��� */
		mCmdParam.Modify.mFileSize = NewSize;   /* �������: �µ��ļ�����,����ģʽ���漰����ͷ���ݲ����Զ����³��� */
		mCmdParam.Modify.mFileAttr = 0xff;  /* �������: �µ��ļ�����,Ϊ0FFH���޸� */
		mCmdParam.Modify.mFileTime = 0xffff;  /* �������: �µ��ļ�ʱ��,Ϊ0FFH���޸� */
		mCmdParam.Modify.mFileDate = 0xffff;  /* �������: �µ��ļ�����,Ϊ0FFH���޸� */
		i = CH375FileModify( );   /* �޸ĵ�ǰ�ļ�����Ϣ,�޸��ļ����� */
		mStopIfError( i );
		printf( "Current file size is %ld\n", CH375vFileSize );
		mCmdParam.Locate.mSectorOffset = 0xffffffff;  /* �Ƶ��ļ���β��,������Ϊ��λ,���Ի�����ļ�β������ͷ���� */
		i = CH375FileLocate( );  /* ���»ص�ԭ�ļ���β��,���������д�����ݽ�����β����ͷ����,��������ͷ������һ�ݸ��������ڻ�������,��������� */
		mStopIfError( i );
	}
	else if ( total >= FILE_DATA_BUF_LEN - CH375vSectorSize ) {  /* �������е����ݿ�Ҫ����,����Ӧ���Ƚ��������е�ԭ������д��U�� */
		mCmdParam.Write.mSectorCount = total / CH375vSectorSize;  /* ���������е��ֽ���ת��Ϊ������(����CH375vSectorSize),������ͷ�����Ȳ��� */
/* ʹ��CH375FileWriteX()����CH375FileWrite()�������ж������ݻ���������ʼ��ַ,����
		mCmdParam.WriteX.mDataBuffer = 0x4600;  ��4600H��ʼ�Ļ������е�����д��
		i = CH375FileWriteX();   ��ָ��������4600H�е�����д�뵽�ļ��� */
		i = CH375FileWrite( );  /* ������Ϊ��λ���ļ�д������,д�뻺�����е���������,����������ͷ */
		mStopIfError( i );
		memcpy( FILE_DATA_BUF+0, FILE_DATA_BUF+(total & ~ ( CH375vSectorSize - 1 ) ), total & ( CH375vSectorSize - 1 ) );  /* ���ղ�δд��U�̵���ͷ���ݸ��Ƶ���������ͷ�� */
		total &= CH375vSectorSize - 1;  /* ��������ֻʣ�¸ղ�δд��U�̵���ͷ���� */
/*		mCmdParam.Write.mSectorCount = 0;  ���ȫ�ֱ���CH375LibConfig��λ4Ϊ0,����ָ��д��0����,����ˢ���ļ��ĳ���
		CH375FileWrite( );  ������Ϊ��λ���ļ�д������,��Ϊ��0����д��,����ֻ���ڸ����ļ��ĳ���,���׶���д�����ݺ�,���������ְ취�����ļ����� */
	}
}

main( ) {
	UINT8	i, month, date, hour;
	UINT16	year, adc;
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
//		while ( CH375DiskStatus != DISK_CONNECT ) xQueryInterrupt( );  /* ��ѯCH375�жϲ������ж�״̬,�ȴ�U�̲��� */
		while ( CH375DiskStatus < DISK_CONNECT ) {  /* ��ѯCH375�жϲ������ж�״̬,�ȴ�U�̲��� */
			if ( CH375DiskConnect( ) == ERR_SUCCESS ) break;  /* ���豸�����򷵻سɹ�,CH375DiskConnectͬʱ�����ȫ�ֱ���CH375DiskStatus */
			mDelaymS( 100 );
		}
		LED_OUT_ACT( );  /* LED�� */
		mDelaymS( 200 );  /* ��ʱ,��ѡ����,�е�USB�洢����Ҫ��ʮ�������ʱ */

/* ���U���Ƿ�׼����,ĳЩU�̱���Ҫִ����һ�����ܹ��� */
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
		printf( "DiskSize\n" );
		i = CH375DiskSize( );  
		mStopIfError( i );
		printf( "TotalSize = %u MB \n", (unsigned int)( mCmdParam.DiskSize.mDiskSizeSec * (CH375vSectorSize/512) / 2048 ) );  // ��ʾΪ��MBΪ��λ������
		// ԭ���㷽�� (unsigned int)( mCmdParam.DiskSize.mDiskSizeSec * CH375vSectorSize / 1000000 ) �п���ǰ����������˺������, �����޸ĳ���ʽ
		LED_RUN_ACT( );  /* ��ʼ����U�� */

/* ���MY_ADC.TXT�ļ��Ѿ��������������ݵ�β��,������������½��ļ� */
		printf( "Open\n" );
		mCopyCodeStringToIRAM( mCmdParam.Open.mPathName, "/MY_ADC.TXT" );  /* �ļ���,���ļ��ڸ�Ŀ¼�� */
		i = CH375FileOpen( );  /* ���ļ� */
		if ( i == ERR_SUCCESS ) {  /* �ļ����ڲ����Ѿ�����,�ƶ��ļ�ָ�뵽β���Ա��������� */
			printf( "File size = %ld\n", CH375vFileSize );  /* V1.5�����ӳ�����ڳɹ����ļ���,ȫ�ֱ���CH375vFileSize�����ļ���ǰ���� */
			printf( "Locate tail\n" );
			mCmdParam.Locate.mSectorOffset = 0xffffffff;  /* �Ƶ��ļ���β��,CH375�ӳ�����ڲ��ǽ��ļ����Ȱ���������CH375vSectorSize����ȡ������ */
			i = CH375FileLocate( );  /* ������Ϊ��λ�Ƶ��ļ�β��,����ļ�β���в���һ����������ͷ�����򱻺���,�������������ô��ͷ���ݽ����ܱ�д�����ݸ��� */
			mStopIfError( i );
			total = CH375vFileSize & ( CH375vSectorSize - 1 );  /* �ϴα����ļ�ʱ���β������ͷ����,��ô��ȡ����ͷ�ֽ���,�����������ȵ��������� */
			printf( "Read last tail = %d Bytes\n", total );
			CH375vFileSize += CH375vSectorSize - 1;  /* ��Ϊ�ؽ��ļ���������һ��������1,�Ա�������һ�������е���ͷ���� */
			mCmdParam.Read.mSectorCount = 1;  /* ��ȡ�ļ�β������ͷ����,�������Ϊ�����ļ�����,��ô�����ļ����Ȱ�CH375vSectorSizeȡ��,����β����ͷ�����޷����� */
/* ʹ��CH375FileReadX()����CH375FileRead()�������ж������ݻ���������ʼ��ַ,����
			mCmdParam.ReadX.mDataBuffer = 0x2C00;  �����������ݷŵ�2C00H��ʼ�Ļ�������
			i = CH375FileReadX();   ���ļ��ж�ȡ������ָ��������,��ʼ��ַΪ2C00H */
			i = CH375FileRead( );  /* ���ļ���ȡβ����ͷ����,���ԭβ��û����ͷ����,��ôʲôҲ������,����ʱmCmdParam.Read.mSectorCountΪʵ�ʶ��������� */
			mStopIfError( i );
			CH375vFileSize -= CH375vSectorSize - 1;  /* �ָ��������ļ����� */
			mCmdParam.Locate.mSectorOffset = 0xffffffff;  /* �Ƶ��ļ���β��,������Ϊ��λ,���Ի�����ļ�β������ͷ���� */
			i = CH375FileLocate( );  /* ���»ص�ԭ�ļ���β��,�������д�����ݽ�����ԭβ����ͷ����,����ԭ��ͷ���ݸղ��Ѿ��������ڴ�,��������� */
			mStopIfError( i );
		}
		else if ( i == ERR_MISS_FILE ) {  /* û���ҵ��ļ�,�����½��ļ� */
			LED_WR_ACT( );  /* д���� */
			printf( "Create\n" );
//			mCopyCodeStringToIRAM( mCmdParam.Create.mPathName, "/MY_ADC.TXT" );  /* �ļ���,���ļ��ڸ�Ŀ¼��,�ղ��Ѿ��ṩ��CH375FileOpen */
			i = CH375FileCreate( );  /* �½��ļ�����,����ļ��Ѿ���������ɾ�������½� */
			mStopIfError( i );
			total = 0;  /* ��ǰû����ͷ���� */
		}
		else mStopIfError( i );  /* ���ļ�ʱ���� */
		LED_WR_ACT( );  /* д���� */
		printf( "Write begin\n" );
		total += sprintf( FILE_DATA_BUF + total, "�ڱ�����������֮ǰ,���ļ��������ݵĳ����� %ld �ֽ�\xd\xa", CH375vFileSize );  /* �����������ӵ���������β��,�ۼƻ������ڵ����ݳ��� */
		mFlushBufferToDisk( 0 );  /* �Զ�ˢ�»�����,��黺�����Ƿ�����,����д�� */
		printf( "Write ADC data\n" );
		TR0=1;  /* �ö�ʱ��0�ļ���ֵ����ADC���� */
		for ( month = 1; month != 12; month ++ ) {  /* ��Ϊ���԰���û��ʵʱʱ��оƬ,������ѭ����ʽģ���·� */
			for ( date = 1; date != 30; date ++ ) {  /* ��Ϊ���԰���û��ʵʱʱ��оƬ,������ѭ����ʽģ������ */
				year = 2004;  /* �ٶ�Ϊ2004�� */
				hour = TL1 & 0x1F;  /* ��Ϊ���԰���û��ʵʱʱ��оƬ,�����ö�ʱ��1�ļ������������ʾ */
/*				adc = get_adc_data( ); */
				adc = ( (UINT16)TH0 << 8 ) | TL0;  /* ��Ϊ���԰���û��ADC,�����ö�ʱ��0�ļ�������ADC������ʾ */
				total += sprintf( FILE_DATA_BUF + total, "Year=%04d, Month=%02d, Date=%02d, Hour=%02d, ADC_data=%u\xd\xa", year, (UINT16)month, (UINT16)date, (UINT16)hour, adc );  /* �����������ݸ�ʽΪһ���ַ��� */
				if ( month == 6 && ( date & 0x0F ) == 0 ) mFlushBufferToDisk( 1 );  /* ǿ��ˢ�»�����,����ǿ��ˢ�»�����,������ͻȻ�ϵ����Լ������ݶ�ʧ */
				else mFlushBufferToDisk( 0 );  /* �Զ�ˢ�»�����,��黺�����Ƿ�����,����д�� */
				printf( "Current total is %d\n", total );  /* ���ڼ�ؼ�� */
			}
		}
		printf( "Write end\n" );
		total += sprintf( FILE_DATA_BUF + total, " ********************************* " );  /* �����������ӵ���������β��,�ۼƻ������ڵ����ݳ��� */
		total += sprintf( FILE_DATA_BUF + total, "��ε�ADC���ݵ��˽���,���򼴽��˳�\xd\xa" );  /* �����������ӵ���������β��,�ۼƻ������ڵ����ݳ��� */
		mFlushBufferToDisk( 1 );  /* ǿ��ˢ�»�����,��ΪϵͳҪ�˳���,���Ա���ǿ��ˢ�� */
		printf( "Close\n" );
		mCmdParam.Close.mUpdateLen = 0;  /* ��Ϊǿ��ˢ�»�����ʱ�Ѿ��������ļ�����,�������ﲻ��Ҫ�Զ������ļ����� */
		i = CH375FileClose( );  /* �ر��ļ� */
		mStopIfError( i );
		LED_WR_INACT( );
		LED_RUN_INACT( );
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