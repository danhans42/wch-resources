// 2003.09.08, 2003.12.28
//****************************************
//**  Copyright  (C)  W.ch  1999-2004   **
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


#include	<windows.h>
#include	<stdlib.h>
#include	<stdio.h>
#include	<conio.h>
#include	<winioctl.h>

#include	"CH375DLL.H"				// CH375�Ķ�̬���ӿ�

// �����ڼ�����͵�Ƭ����Ӧ�ó���֮��Լ���´����ݵ����ֽ���������
#define		TEST_START			0x20	// ���Թ��̿�ʼ
#define		TEST_DATA			0x21	// ����������ȷ��
#define		TEST_UPLOAD			0x22	// �����ϴ����ݿ�
#define		TEST_DOWNLOAD		0x23	// �����´����ݿ�


//�������
void main ( )
{
	unsigned char	mBuffer[4100];
	unsigned char	mReadBuf[100];
	unsigned long	i, mLength, mTestCount, mErrCnt, mStep, mTotal;
	double          speed;

	printf( "\nCH372/CH375 Bulk Data Test Program V1.1 ,   Copyright (C) W.ch 2004.12\n" );
	printf( "test data correctness and USB speed \n" );

// ��Ҫʹ��DLL����Ҫ�ȼ���
	printf( "*** Load DLL: CH375DLL.DLL \n" );
	if ( LoadLibrary( "CH375DLL.DLL" ) == NULL ) return;  // ����DLLʧ��,����δ��װ��ϵͳ��

	printf( "*** CH375OpenDevice: 0# \n" );
	if ( CH375OpenDevice( 0 ) == INVALID_HANDLE_VALUE ) return;  /* ʹ��֮ǰ������豸 */

	CH375SetTimeout( 0, 2000, 2000 );  // ����USB���ݶ�д�ĳ�ʱ,����2000mSδ��ɶ�д��ǿ�Ʒ���,����һֱ�ȴ���ȥ

	mErrCnt=0;

	printf( "*** Step-0: notice MCU start test \n" );
	mBuffer[0]=TEST_START;  // �����ڼ�����͵�Ƭ����Ӧ�ó���֮��Լ���´����ݵ����ֽ���������
	mTotal=1;
	if ( CH375WriteData( 0, &mBuffer, &mTotal ) )  // ���ͳɹ�
	{
		printf( "start test now\n" );
	}
	else {  // д����ʧ��
		mErrCnt++;
		printf( "S0-T0 CH375WriteData return error, length=1\n" );
	}
	Sleep(200);
	mBuffer[0]=TEST_START;
	mTotal=1;
	if ( CH375WriteData( 0, &mBuffer, &mTotal ) )  // ���ͳɹ�
	{
	}
	else {  // д����ʧ��
		mErrCnt++;
		printf( "S0-T1 CH375WriteData return error, length=1\n" );
	}
	Sleep(200);

	printf( "*** Step-1: test data correctness: 5000 times, random length and data\n" );
	for ( mTestCount=0; mTestCount<5000; ++mTestCount )  // ѭ������
	{
		mStep=mTestCount&0x03;
		switch( mStep )
		{
			case 0: memset( mBuffer, 0x00, 64 );  // ����ȫ00H/����ȫFFH
					break;
			case 1: memset( mBuffer, 0xff, 64 );  // ����ȫFFH/����ȫ00H
					break;
			default: for ( i=0; i<64; i+=4 ) *(unsigned long *)(&mBuffer[i])=rand();  // ���������/����λ����
					break;
		}
		mBuffer[0]=TEST_DATA;
		mTotal=rand();  // ���������
		mTotal=mTotal%100;
		if ( mTotal == 0 || mTotal > 64 ) mTotal=(mTotal&0x01)?64:63;
		printf( "Cnt=%4ld, Len=%2ld, Data: %02XH,%02XH,%02XH,%02XH...\xd", mTestCount, mTotal, mBuffer[0], mBuffer[1], mBuffer[2], mBuffer[3] );
		if ( kbhit() )
		{
			if ( getch() == 0x20 ) {  // ��ֹѭ��
				printf( "*** CH375CloseDevice by break: 0              \n" );
				CH375CloseDevice( 0 );
				exit(2);
				break;
			}
		}
		if ( CH375WriteData( 0, &mBuffer, &mTotal ) )  // ���ͳɹ�
		{
			mLength = 64;
			if ( CH375ReadData( 0, &mReadBuf, &mLength ) )  // ���ճɹ�
			{
				if ( mLength != mTotal || mLength==0 ) {
					mErrCnt++;
					printf( "S1-T%0ld-C%ld return length error: %ld (%ld)\n", mStep, mTestCount, mLength, mTotal );
				}
				else {
					for ( i=0; i<mLength; ++i ) {
						if ( (mReadBuf[i]^0xff)!=mBuffer[i] ) {
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
	Sleep(100);

	printf( "*** Step-2: test speed of download data: 2048KB data \n" );
	for ( i=0; i<4096; i+=4 ) *(unsigned long *)(&mBuffer[i])=rand();  // ����������Է���
	for ( i=0; i<4096; i+=64 ) mBuffer[i]=TEST_DOWNLOAD;  // ÿ64�ֽ�Ϊһ�����ݰ�,���ֽ�Ϊ������
	mTotal=0;
	mStep=GetTickCount();
	for ( mTestCount=0; mTestCount<500; ++mTestCount )  // ѭ������
	{
		*(unsigned long *)(&mBuffer[4])=mTestCount;
		mLength = 4096;
		if ( CH375WriteData( 0, &mBuffer, &mLength ) )  // ���ͳɹ�
		{
			if ( mLength != 4096 ) printf( "S2-C%ld CH375WriteData actual length short %ld (4096)\n", mLength );
			mTotal +=mLength;  // �ۼƳ���
		}
		else {  // д����ʧ��
			mErrCnt++;
			printf( "S2-C%ld CH375WriteData return error\n", mTestCount );
		}
	}
	mLength=GetTickCount();
	mLength=mLength-mStep;
	speed=1000;
	if ( mLength !=0 ) speed=speed*mTotal/mLength;
	else speed=9999999;
	printf( "*** download speed = %7.1f Bytes/Sec, total=%ld bytes, time=%ld mS\n", speed, mTotal, mLength);
	Sleep(100);

	printf( "*** Step-3: test speed of upload data: 2048KB data\n" );
	mBuffer[0]=TEST_UPLOAD;
	mLength = 1;
	if ( CH375WriteData( 0, &mBuffer, &mLength ) )  // ���ͳɹ�
	{
		mTotal=0;
		mStep=GetTickCount();
		for ( mTestCount=0; mTestCount<500; ++mTestCount )  // ѭ������
		{
			mLength = 4096;
			if ( CH375ReadData( 0, &mBuffer, &mLength ) )  // ���ճɹ�
			{
				if ( mLength != 4096 ) printf( "S3-C%ld CH375ReadData actual length short %ld (4096)\n", mLength );
				mTotal +=mLength;  // �ۼƳ���
			}
			else {  // ������ʧ��
				mErrCnt++;
				printf( "S3-C%ld CH375ReadData return error\n", mTestCount );
			}
		}
		mLength=GetTickCount();
		mLength=mLength-mStep;
		speed=1000;
		if ( mLength !=0 ) speed=speed*mTotal/mLength;
		else speed=9999999;
		printf( "*** upload speed = %7.1f Bytes/Sec, total=%ld bytes, time=%ld mS\n", speed, mTotal, mLength);
	}
	else {  // д����ʧ��
		mErrCnt++;
		printf( "S3 CH375WriteData return error\n" );
	}
	Sleep(100);

// �ر�CH375�豸,���������ʹ�������ر��豸,�����д��Ӳ���е������ļ���Ҫ�ر�һ��
	printf( "*** Total error = %ld \n", mErrCnt );
	printf( "*** CH375CloseDevice: 0 \n" );
	CH375CloseDevice( 0 );

	printf( "\nExit.\n" );
	getch();
}