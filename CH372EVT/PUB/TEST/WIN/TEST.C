// 2003.09.08, 2003.12.28
//****************************************
//**  Copyright  (C)  W.ch  1999-2005   **
//**  Web:  http://www.winchiphead.com  **
//****************************************
//**  DLL for USB interface chip CH375  **
//**  C, VC5.0                          **
//****************************************
//
// USB���߽ӿ�оƬCH375�����ݿ���Գ��� V1.0
// �Ͼ��ߺ�������޹�˾  ����: W.ch 2003.12
// CH375-BLK  V1.0
// ���л���: Windows 98/ME, Windows 2000/XP
// support USB chip: CH372/CH375
//

// ������������ݴ������ȷ��,�������ڳ�ʱ����������,��Ӧ�ĵ�Ƭ���˵Ĳ��Գ���ΪTEST.C
// ����: �´�������ȵ�������ݰ�,����Ƭ�����ղ������ݰ�λȡ���󷵻�,�����ɼ����������պ�Ƚ������Ƿ���ȷ


#include	<windows.h>
#include	<stdlib.h>
#include	<stdio.h>
#include	<conio.h>
#include	<winioctl.h>

#include	"CH375DLL.H"				// CH375�Ķ�̬���ӿ�

// �����ڼ�����͵�Ƭ����Ӧ�ó���֮��Լ���´����ݵ����ֽ���������
#define		TEST_DATA			0x21	// ����������ȷ��


//�������
void main ( )
{
	int				key;
	unsigned char	mBuffer[100];
	unsigned char	mReadBuf[100];
	unsigned long	i, mLength, mTestCount, mErrCnt, mStep, mTotal;

	printf( "\nCH372/CH375 Bulk Data Test Program V1.1 ,   Copyright (C) W.ch 2004.12\n" );
	printf( "test data correctness \n" );

// ��Ҫʹ��DLL����Ҫ�ȼ���,û�д˾�����Զ�����
	printf( "*** Load DLL: CH375DLL.DLL \n" );
	if ( LoadLibrary( "CH375DLL.DLL" ) == NULL ) return;  // ����DLLʧ��,����δ��װ��ϵͳ��

	printf( "*** CH375OpenDevice: 0# \n" );
	if ( CH375OpenDevice( 0 ) == INVALID_HANDLE_VALUE ) return;  /* ʹ��֮ǰ������豸 */

	CH375SetTimeout( 0, 5000, 5000 );  // ����USB���ݶ�д�ĳ�ʱ,����5000mSδ��ɶ�д��ǿ�Ʒ���,����һֱ�ȴ���ȥ

	mErrCnt=0;

	printf( "*** Step-1: test data correctness: 10000000 times, random length and data\n" );
	for ( mTestCount=0; mTestCount<10000000; ++mTestCount )  // ѭ������
	{
		mStep=mTestCount&0x03;
		switch( mStep )
		{
			case 0: memset( mBuffer, 0x00, 64 );  // ����ȫ00H/����ȫFFH
					break;
			case 1: memset( mBuffer, 0xff, 64 );  // ����ȫFFH/����ȫ00H
					break;
			default: for ( i=0; i<64; i++ ) mBuffer[i]=(unsigned char)( rand() );  // ���������/����λ����
					break;
		}
		mBuffer[0]=TEST_DATA;
		mTotal=rand();  // ���������
		mTotal=mTotal%100;
		if ( mTotal == 0 || mTotal > 64 ) mTotal=64;
		printf( "Cnt=%4ld, Len=%2ld, Data: %02XH,%02XH,%02XH,%02XH,%02XH,%02XH...\xd", mTestCount, mTotal, mBuffer[0], mBuffer[1], mBuffer[2], mBuffer[3], mBuffer[4], mBuffer[5] );
		if ( kbhit() )
		{
			key = getch( );
			if ( key == 0x1b || key == 0x20 ) {  // ESC�����߿ո����ֹѭ��
				printf( "*** CH375CloseDevice by break: 0              \n" );
				CH375CloseDevice( 0 );
				exit(2);
				break;
			}
		}
		if ( CH375WriteData( 0, mBuffer, &mTotal ) )  // ���ͳɹ�
		{
			mLength = 64;
			if ( mTestCount == 0 ) Sleep( 200 );  // ���ǵ�֮ǰ��Ƭ��׼���ϴ������ݿ���δ�������ȡ��,�����״λش��п���ֱ�Ӷ���֮ǰ�����ݶ����Ǳ������ݵ�ȡ��,�����״λش��ȵȴ���Ƭ��׼����ȡ������
			if ( CH375ReadData( 0, mReadBuf, &mLength ) )  // ���ճɹ�
			{
				if ( mLength != mTotal || mLength==0 ) {  // ���ȴ���
					mErrCnt++;
					printf( "S1-T%0ld-C%ld return length error: %ld (%ld)\n", mStep, mTestCount, mLength, mTotal );
				}
				else {
					for ( i=0; i<mLength; ++i ) {
						if ( (mReadBuf[i]^0xff)!=mBuffer[i] ) {  // ȡ��ֵ�Ƚϴ���
							mErrCnt++;
							printf( "S1-T%0ld-C%ld return data error at %ld: %02XH (%02XH)\n", mStep, mTestCount, i, mReadBuf[i], mBuffer[i] );
						}
					}
				}
			}
			else {  // ������ʧ��
				mErrCnt++;
				printf( "S1-T%0ld-C%ld CH375ReadData return error, length=%ld\n", mStep, mTestCount, mTotal );
			}
		}
		else {  // д����ʧ��
			mErrCnt++;
			printf( "S1-T%0ld-C%ld CH375WriteData return error, length=%ld\n", mStep, mTestCount, mTotal );
		}
	}
	if ( mErrCnt==0 ) printf( "*** passed                                             \n" );
// �ر�CH375�豸,���������ʹ�������ر��豸,�����д��Ӳ���е������ļ���Ҫ�ر�һ��
	printf( "*** Total error = %ld \n", mErrCnt );
	printf( "*** CH375CloseDevice: 0 \n" );
	CH375CloseDevice( 0 );

	printf( "\nExit.\n" );
	getch();
}