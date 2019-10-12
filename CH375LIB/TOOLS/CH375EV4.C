/* �ó�����180��C������ܹ���ȡFAT16�ļ�ϵͳU�̵ĸ�Ŀ¼,���Կ�����Ŀ¼�µ��ļ���,������ʾ���ļ����� */
/* ע��,�ó���ܲ��Ͻ�,Ҳû���κδ�����,��U�̼����Խϲ�,ֻ�����ڼ�����,��Ϊ�ο� */
/* ��Ƭ����дU�̵ĳ����Ϊ4��: Ӳ��USB�ӿڲ�, BulkOnly����Э���, RBC/SCSI�����, FAT�ļ�ϵͳ�� */

/* ����������֧��WINDOWS��FAT16��ʽ����U��,��Ϊ���򾫼�,����ֻ���ݳ���50%���ϵ�U��Ʒ��,�����
��CH375AоƬ������Կ���ߵ�85%,��Ȼ,���ʹ�ù�˾���ӳ���������ʽ�汾��CԴ��������Ը��á�
��������U��ͨ��:�ɿ�/���Ⱦ���64M/��������128M/U160-64M/�����ռ�128M,������/������16M/������,
��ϻ��/64M,΢��/64M,���/32M/64M/128M,����/C200-64M,�¿�/256M,����/128M...,��ӭ�ṩ���Խ��
δͨ��U��:������/�ǻ۰�128M,�廪����/USB2.0-128M,��Ȼ,ʹ���ӳ�����CH375A�����Բ���ͨ�� */

#include <stdio.h>
#include "CH375INC.H"		/* ����CH375������뼰����״̬ */
#include <reg51.h>			/* ���¶���������MCS-51��Ƭ��,������Ƭ�������޸� */
#define	UINT8		unsigned char
#define	UINT16		unsigned short
#define	UINT32		unsigned long
#define	UINT8X		unsigned char xdata
#define	UINT8VX		unsigned char volatile xdata
UINT8VX		CH375_CMD_PORT _at_ 0xBDF1;	/* CH375����˿ڵ�I/O��ַ */
UINT8VX		CH375_DAT_PORT _at_ 0xBCF0;	/* CH375���ݶ˿ڵ�I/O��ַ */
#define		CH375_INT_WIRE		INT0	/* P3.2, ����CH375��INT#����,���ڲ�ѯ�ж�״̬ */
UINT8X		DISK_BUFFER[512*32] _at_ 0x0000;	/* �ⲿRAM���ݻ���������ʼ��ַ,���Ȳ�����һ�ζ�д�����ݳ��� */

UINT32	DiskStart;		/* �߼��̵���ʼ����������LBA */
UINT8	SecPerClus;		/* �߼��̵�ÿ�������� */
UINT8	RsvdSecCnt;		/* �߼��̵ı��������� */
UINT16	FATSz16;		/* FAT16�߼��̵�FAT��ռ�õ������� */

/* ********** Ӳ��USB�ӿڲ�,����������ʡ����,��Ƭ����Ҫ��CH375�ӿڰ� ************************************************************ */

void	mDelaymS( UINT8 delay ) {		/* �Ժ���Ϊ��λ��ʱ,����ȷ,������24MHzʱ��MCS51 */
	UINT8	i, j, c;
	for ( i = delay; i != 0; i -- ) {
		for ( j = 200; j != 0; j -- ) c += 3;  /* ��24MHzʱ������ʱ500uS */
		for ( j = 200; j != 0; j -- ) c += 3;  /* ��24MHzʱ������ʱ500uS */
	}
}

void CH375_WR_CMD_PORT( UINT8 cmd ) {	/* ��CH375������˿�д������,���ڲ�С��4uS,�����Ƭ���Ͽ�����ʱ */
	CH375_CMD_PORT=cmd;
	for ( cmd = 2; cmd != 0; cmd -- );	/* ����������ǰ��Ӧ�ø���ʱ2uS,����MCS51���Բ���Ҫ��ʱ */
}
void CH375_WR_DAT_PORT( UINT8 dat ) {	/* ��CH375�����ݶ˿�д������,���ڲ�С��1.5uS,�����Ƭ���Ͽ�����ʱ */
	CH375_DAT_PORT=dat;					/* ��ΪMCS51��Ƭ����������ʵ����������ʱ */
}
UINT8 CH375_RD_DAT_PORT( void ) {		/* ��CH375�����ݶ˿ڶ�������,���ڲ�С��1.5uS,�����Ƭ���Ͽ�����ʱ */
	return( CH375_DAT_PORT );			/* ��ΪMCS51��Ƭ����������ʵ����������ʱ */
}
UINT8 mWaitInterrupt( void ) {	/* �ȴ�CH375�жϲ���ȡ״̬,�����˵ȴ��������,���ز���״̬ */
	while( CH375_INT_WIRE );  /* ��ѯ�ȴ�CH375��������ж�(INT#�͵�ƽ) */
	CH375_WR_CMD_PORT( CMD_GET_STATUS );  /* ������������ж�,��ȡ�ж�״̬ */
	return( CH375_RD_DAT_PORT( ) );
}

/* ********** BulkOnly����Э���,��CH375������,�����д��Ƭ������ ************************************************************ */

/* ********** RBC/SCSI�����,��Ȼ��CH375������,����Ҫд���򷢳�����շ����� ************************************************************ */

UINT8	mInitDisk( void ) {	/* ��ʼ������ */
	UINT8 Status;
	CH375_WR_CMD_PORT( CMD_GET_STATUS );  /* ������������ж�, ��ȡ�ж�״̬ */
	Status = CH375_RD_DAT_PORT( );
	if ( Status == USB_INT_DISCONNECT ) return( Status );  /* USB�豸�Ͽ� */
	CH375_WR_CMD_PORT( CMD_DISK_INIT );  /* ��ʼ��USB�洢�� */
	Status = mWaitInterrupt( );  /* �ȴ��жϲ���ȡ״̬ */
	if ( Status != USB_INT_SUCCESS ) return( Status );  /* ���ִ��� */
	CH375_WR_CMD_PORT( CMD_DISK_SIZE );  /* ��ȡUSB�洢�������� */
	Status = mWaitInterrupt( );  /* �ȴ��жϲ���ȡ״̬ */
	if ( Status != USB_INT_SUCCESS ) {  /* �������� */
/* ����CH375AоƬ,�����ڴ�ִ��һ��CMD_DISK_R_SENSE���� */
		mDelaymS( 250 );
		CH375_WR_CMD_PORT( CMD_DISK_SIZE );  /* ��ȡUSB�洢�������� */
		Status = mWaitInterrupt( );  /* �ȴ��жϲ���ȡ״̬ */
	}
	if ( Status != USB_INT_SUCCESS ) return( Status );  /* ���ִ��� */
	return( 0 );  /* U���Ѿ��ɹ���ʼ�� */
}

UINT8	mReadSector( UINT32 iLbaStart, UINT8 iSectorCount, UINT8X *oDataBuffer ) {		/* ��U�̶�ȡ���ݿ鵽������ */
/* iLbaStart ��ʼ������, iSectorCount ������, oDataBuffer ��������ַ */
	UINT16	mBlockCount;
	UINT8	c;
	CH375_WR_CMD_PORT( CMD_DISK_READ );  /* ��USB�洢�������ݿ� */
	CH375_WR_DAT_PORT( (UINT8)iLbaStart );  /* LBA�����8λ */
	CH375_WR_DAT_PORT( (UINT8)( iLbaStart >> 8 ) );
	CH375_WR_DAT_PORT( (UINT8)( iLbaStart >> 16 ) );
	CH375_WR_DAT_PORT( (UINT8)( iLbaStart >> 24 ) );  /* LBA�����8λ */
	CH375_WR_DAT_PORT( iSectorCount );  /* ������ */
	for ( mBlockCount = iSectorCount * 8; mBlockCount != 0; mBlockCount -- ) {  /* ���ݿ���� */
		c = mWaitInterrupt( );  /* �ȴ��жϲ���ȡ״̬ */
		if ( c == USB_INT_DISK_READ ) {  /* �ȴ��жϲ���ȡ״̬,USB�洢�������ݿ�,�������ݶ��� */
			CH375_WR_CMD_PORT( CMD_RD_USB_DATA );  /* ��CH375��������ȡ���ݿ� */
			c = CH375_RD_DAT_PORT( );  /* �������ݵĳ��� */
			while ( c -- ) *oDataBuffer++ = CH375_RD_DAT_PORT( );  /* ���ݳ��ȶ�ȡ���ݲ����� */
			CH375_WR_CMD_PORT( CMD_DISK_RD_GO );  /* ����ִ��USB�洢���Ķ����� */
		}
		else break;  /* ���ش���״̬ */
	}
	if ( mBlockCount == 0 ) {
		c = mWaitInterrupt( );  /* �ȴ��жϲ���ȡ״̬ */
		if ( c== USB_INT_SUCCESS ) return( 0 );  /* �����ɹ� */
	}
	return( c );  /* ����ʧ�� */
}

/* ********** FAT�ļ�ϵͳ��,��������ʵ�ʽϴ�,����,�ó������ʾ���򵥵Ĺ���,���Ծ��� ************************************************************ */

UINT16	mGetPointWord( UINT8X *iAddr ) {	/* ��ȡ������,��ΪMCS51�Ǵ�˸�ʽ,U��FATͨ����С�˸�ʽ,����ת�� */
	return( iAddr[0] | (UINT16)iAddr[1] << 8 );
}

UINT8	mIdenDisk( void ) {		/* ʶ�������ǰ�߼��� */
	UINT8	Status;
	DiskStart = 0;  /* �����Ƿǳ��򵥵�FAT�ļ�ϵͳ�ķ���,��ʽӦ�þ��Բ�Ӧ����˼�,��������Ժ��ݴ��Բ� */
	Status = mReadSector( 0, 1, DISK_BUFFER );  /* ��ȡ�߼���������Ϣ */
	if ( Status != 0 ) return( Status );
	if ( DISK_BUFFER[0] != 0xEB && DISK_BUFFER[0] != 0xE9 ) {  /* �����߼��������� */
		DiskStart = DISK_BUFFER[0x1C6] | (UINT16)DISK_BUFFER[0x1C7] << 8 | (UINT32)DISK_BUFFER[0x1C8] << 16 | (UINT32)DISK_BUFFER[0x1C9] << 24;
		Status = mReadSector( DiskStart, 1, DISK_BUFFER );  /* �����µ���ʼ�����Ŷ�ȡ�߼���������Ϣ */
		if ( Status != 0 ) return( Status );
	}
	SecPerClus = DISK_BUFFER[0x0D];  /* ÿ�������� */
	RsvdSecCnt = DISK_BUFFER[0x0E];  /* �߼��̵ı��������� */
	FATSz16 = mGetPointWord( &DISK_BUFFER[0x16] );  /* FAT��ռ�������� */
	return( 0 );  /* �ɹ� */
}

UINT16	mLinkCluster( UINT16 iCluster ) {	/* ���ָ���غŵ����Ӵ� */
/* ����: iCluster ��ǰ�غ�, ����: ԭ���Ӵغ�, ���Ϊ0��˵������ */
	UINT8	Status;
	Status = mReadSector( DiskStart + RsvdSecCnt + iCluster / 256, 1, DISK_BUFFER );  /* ��ȡ�غ����ڵ�FAT���� */
	if ( Status != 0 ) return( 0 );  /* ���� */
	return( mGetPointWord( &DISK_BUFFER[ ( iCluster + iCluster ) & 0x01FF ] ) );  /* ȡԭ������ */
}

UINT32	mClusterToLba( UINT16 iCluster ) {	/* ���غ�ת��Ϊ����LBA������ַ */
	return( DiskStart + RsvdSecCnt + FATSz16 + FATSz16 + 32 + ( iCluster - 2 ) * SecPerClus );  /* ���غ�ת��ΪLBA,�õ�ǰ��������ʼLBA */
}

void	mInitSTDIO( void ) {	/* �����ڵ�����;����ʾ���ݵ�PC��,��ó�������ȫ�޹�,Ϊprintf��getkey���������ʼ������ */
	SCON = 0x50; PCON = 0x80; TMOD = 0x20; TH1 = 0xf3; TR1 = 1; TI = 1;  /* 24MHz����, 9600bps */
}
void	mStopIfError( UINT8 iErrCode ) {	/* ���������ֹͣ���в���ʾ����״̬,��ʽӦ�û���Ҫ�������� */
	if ( iErrCode == 0 ) return;
	printf( "Error status, %02X\n", (UINT16)iErrCode );
}

main( ) {
	UINT8	Status;
	UINT8X	*CurrentDir;
	UINT16	Cluster;
	mDelaymS( 200 );  /* ��ʱ200���� */
	mInitSTDIO( );
	CH375_WR_CMD_PORT( CMD_SET_USB_MODE );  /* ��ʼ��CH375,����USB����ģʽ */
	CH375_WR_DAT_PORT( 6 );  /* ģʽ����,�Զ����USB�豸���� */
	while ( 1 ) {
		printf( "Insert USB disk\n" );
		while ( mWaitInterrupt( ) != USB_INT_CONNECT );  /* �ȴ�U������ */
		mDelaymS( 250 );  /* ��ʱ�ȴ�U�̽�����������״̬ */
		Status = mInitDisk( );  /* ��ʼ��U��,ʵ����ʶ��U�̵�����,��Ӱ��U���е�����,�����ж�д����֮ǰ������д˲��� */
		mStopIfError( Status );
		Status = mIdenDisk( );  /* ʶ�����U���ļ�ϵͳ,��Ҫ���� */
		mStopIfError( Status );
		Status = mReadSector( DiskStart + RsvdSecCnt + FATSz16 + FATSz16, 32, DISK_BUFFER );  /* ��ȡFAT16�߼��̵ĸ�Ŀ¼,ͨ����Ŀ¼ռ��32������ */
		mStopIfError( Status );
		for ( CurrentDir = DISK_BUFFER; CurrentDir[0] != 0; CurrentDir += 32 ) {  /* ���������Ѿ������ڴ��еĸ�Ŀ¼ */
			if ( ( CurrentDir[0x0B] & 0x08 ) == 0 && CurrentDir[0] != 0xE5 ) {  /* ���ļ�������Ŀ¼��,���Ǳ�ɾ���� */
				CurrentDir[0x0B] = 0;  /* Ϊ�˱�����ʾ,�����ļ�������Ŀ¼���Ľ�����־,�����һֱ��ʾ��ȥ,ʵ��Ӧ�ò�����0 */
				printf( "Name: %s\n", CurrentDir );  /* ͨ�����������ʾ,��Ȼ�ڷ������п���ֱ�ӿ��ڴ� */
			}
		}  /* ������ʾ��Ŀ¼�µ������ļ���,���´򿪵�һ���ļ�,�����C�ļ��Ļ� */
		if ( ( DISK_BUFFER[0x0B] & 0x08 ) == 0 && DISK_BUFFER[0] != 0xE5 && DISK_BUFFER[8] == 'C' ) {  /* ��һ���ļ���C�ļ� */
			Cluster = mGetPointWord( &DISK_BUFFER[0x1A] );  /* �ļ����״� */
			while ( Cluster < 0xFFF8 ) {  /* �ļ���δ���� */
				if ( Cluster == 0 ) mStopIfError( 0x8F );  /* �����״�,������0�����ļ����Ǵ���,�������Ӵ�,�����Ǵ��� */
				Status = mReadSector( mClusterToLba( Cluster ), SecPerClus, DISK_BUFFER );  /* ��ȡ�״ص������� */
				mStopIfError( Status );
				DISK_BUFFER[30] = 0; printf( "Data: %s\n", DISK_BUFFER );  /* ��ǿ���ַ�������,��ʾ�صĵ�һ���ַ� */
				Cluster = mLinkCluster( Cluster );  /* ��ȡ���Ӵ�,����0˵������ */
			}
		}
		while ( mWaitInterrupt( ) != USB_INT_DISCONNECT );  /* �ȴ�U�̰γ� */
		mDelaymS( 250 );
	}
}