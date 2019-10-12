// 2003.09.08, 2003.12.28, 2005.02.23
//****************************************
//**  Copyright  (C)  W.ch  1999-2004   **
//**  Web:  http://www.winchiphead.com  **
//****************************************
//**  DLL for USB interface chip CH375  **
//**  C, VC5.0                          **
//****************************************
//
// USB���߽ӿ�оƬCH375�����ݿ���Գ��� V1.2
// �Ͼ��ߺ�������޹�˾  ����: W.ch 2003.12
// CH375-BLK V1.2 for speed testing
// ���л���: Windows 98/ME, Windows 2000/XP
// support USB chip: CH372/CH375
//

// ����CH372����CH375����������USB����ʱ�����ݴ����ٶ�
// ��Windows XP SP2��, CH372A����CH375A���´��ٶ�ԼΪ341K�ֽ�/��, �����ϴ�ģʽ���ϴ��ٶ�ԼΪ447K�ֽ�/��, ֱ���ϴ�ģʽ���ϴ��ٶ�ԼΪ372K�ֽ�/��
// ��ͬ�Ĳ���ϵͳ���߲�ͬ�ĵ�Ƭ���ٶȶ���Ӱ���ٶȲ��Խ��

// ������ʹ�û����ϴ�ģʽ, �ص���, �����������Զ������ϴ����ݱ������ڲ���������, ��Ӧ�ó�����ʱ��ȡ, �����ٶȿ�, ��������������ʽ, ��������WINDOWS 98/ME
// �ڻ����ϴ�ģʽ��, Ӧ�ó������CH375ReadData�󽫻���������, ����ڲ���������û�������򷵻�0��������, ������ʱ����CH375QueryBufUpload��ѯ���������Ƿ�������

// Ĭ�Ϲ�����ֱ���ϴ�ģʽ, ��ҪӦ�ó������CH375SetBufUpload���û����ϴ�ģʽ
// ʹ��ֱ���ϴ�ģʽ, �ص���, ֻ��Ӧ�ó�����Ҫʱ����API�ϴ�����, �ϴ���������ͬ��, �����������Ӧ��ʽ
// ��ֱ���ϴ�ģʽ��, Ӧ�ó������CH375ReadData��ֻ�ڵȽ��յ�����Ҫ�����ݲŻ᷵��, �����ʱû���ϴ�������ôӦ�ó��򽫹���, �������ö���ʱ����ʱ����


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
	unsigned long	index=0;  // 0#�豸
	unsigned char	mBuffer[4100];
	unsigned char	mReadBuf[100];
	unsigned long	i, mLength, mTestCount, mErrCnt, mStep, mTotal;
	double          speed;

	printf( "\nCH372/CH375 Bulk Data Test Program V1.2 ,   Copyright (C) W.ch 2004.12\n" );
	printf( "test data correctness and USB speed, use buffer upload mode\n" );

// ��Ҫʹ��DLL����Ҫ�ȼ���
	printf( "*** Load DLL: CH375DLL.DLL \n" );
	if ( LoadLibrary( "CH375DLL.DLL" ) == NULL ) return;  // ����DLLʧ��,����δ��װ��ϵͳ��

	printf( "*** CH375OpenDevice: 0# \n" );
	if ( CH375OpenDevice( index ) == INVALID_HANDLE_VALUE ) return;  /* ʹ��֮ǰ������豸 */

	CH375SetTimeout( index, 2000, 2000 );  // ����USB���ݶ�д��ʱ,����2000mSδ��ɶ�д��ǿ�Ʒ���,����һֱ�ȴ���ȥ

	CH375SetBufUpload( index, 1 );  // ʹ�û����ϴ�ģʽ, ֪ͨ�������������ڲ��̶߳�USB�ϴ����ݽ��л���

	mErrCnt=0;

	printf( "*** Step-0: notice MCU start test \n" );
	mBuffer[0]=TEST_START;  // �����ڼ�����͵�Ƭ����Ӧ�ó���֮��Լ���´����ݵ����ֽ���������
	mTotal=1;
	if ( CH375WriteData( index, &mBuffer, &mTotal ) )  // ���ͳɹ�
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
	if ( CH375WriteData( index, &mBuffer, &mTotal ) )  // ���ͳɹ�
	{
	}
	else {  // д����ʧ��
		mErrCnt++;
		printf( "S0-T1 CH375WriteData return error, length=1\n" );
	}
	Sleep(200);

	CH375SetBufUpload( index, 1 );  // ���������Ϊ������ڲ��ϴ�������,���ڻ����ϴ�,Ӧ������ʽ�ϴ�����֮ǰˢ��USB���ջ�����,���򻺳����п��ܻ���֮ǰ����Ч����

	printf( "*** Step-1: test data correctness: 1000 times, random length and data\n" );
	for ( mTestCount=0; mTestCount<1000; ++mTestCount )  // ѭ������
	{
		mStep=mTestCount&0x03;
		switch( mStep )
		{
			case 0: memset( mBuffer, 0x00, 64 );  // ����ȫ00H/����ȫFFH
					break;
			case 1: memset( mBuffer, 0xff, 64 );  // ����ȫFFH/����ȫ00H
					break;
			default: for ( i=0; i<64; i++ ) mBuffer[i]=(unsigned char)(rand());  // ���������/����λ����
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
				CH375SetTimeout( index, 0xffffffff, 0xffffffff );  // ȡ����ʱ
				CH375SetBufUpload( index, 0 );  // �ָ�ֱ���ϴ�ģʽ
				CH375CloseDevice( index );
				exit(2);
				break;
			}
		}
		if ( CH375WriteData( index, &mBuffer, &mTotal ) )  // ���ͳɹ�
		{
RetryGetData:
			mLength = 64;
			if ( CH375ReadData( index, &mReadBuf, &mLength ) )  // ���ճɹ�,ʵ���ǵ���������ȡ����
			{	// ���ڻ����ϴ���ʽ,CH375ReadData������������,����Ҫ�ϸ��鷵�س����Ƿ�����
				if ( mLength == 0 ) {  // �ڻ�������������ʱ�п��ܷ��س���Ϊ0,ֻ�ܵȴ�һ����ٲ�
					Sleep(20);  // �ȴ��ϴ�����,������������
					goto RetryGetData;  // ��δ��������,�����ȴ�
				}
				if ( mLength != mTotal || mLength==0 ) {
					mErrCnt++;
					printf( "S1-T%0ld-C%ld return length error: %ld (%ld)\n", mStep, mTestCount, mLength, mTotal );
				}
				else {
					for ( i=0; i<mLength; ++i ) {
						if ( (unsigned char)(mReadBuf[i]^0xff)!=mBuffer[i] ) {
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
		if ( CH375WriteData( index, &mBuffer, &mLength ) )  // ���ͳɹ�
		{
			if ( mLength != 4096 ) printf( "S2-C%ld CH375WriteData actual length short %ld (4096)\n", mTestCount, mLength );
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
	if ( CH375WriteData( index, &mBuffer, &mLength ) )  // ���ͳɹ�
	{
		mTotal=0;
		mStep=GetTickCount();
		for ( mTestCount=0; mTotal < 500 * 4096L; ++mTestCount )  // ѭ������
		{
			mLength = 4096;  // ���ڻ����ϴ���ʽ,������0��4096֮���������64Ϊ�����ĳ���,���Ȼ�����Ӱ���ٶ�;����ֱ���ϴ���ʽ,������0��4096֮���������64Ϊ�����ĳ���,���ȴ����ٶȿ�
			if ( CH375ReadData( index, &mBuffer, &mLength ) )  // ���ճɹ�
			{  // ���ڻ����ϴ���ʽ,CH375ReadData������������,����Ҫ�ϸ��鷵�س����Ƿ�����
				if ( mLength != 4096 ) {  // �������ݳ��Ȳ���������ĳ���,˵���ڲ�����������,������ʱ���ٶ�
					Sleep(10);  // �ȴ��ϴ�����,������������
					if ( mLength > 4096 ) printf( "S3-C%ld CH375ReadData actual length long %ld (4096)\n", mTestCount, mLength );
				}
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
		printf( "*** upload speed = %7.1f Bytes/Sec, total=%ld bytes, time=%ld mS, %d\n", speed, mTotal, mLength, mTestCount);
	}
	else {  // д����ʧ��
		mErrCnt++;
		printf( "S3 CH375WriteData return error\n" );
	}
	Sleep(100);

// �ر�CH375�豸,���������ʹ�������ر��豸,�����д��Ӳ���е������ļ���Ҫ�ر�һ��
	printf( "*** Total error = %ld \n", mErrCnt );
	printf( "*** CH375CloseDevice: 0 \n" );
	CH375SetTimeout( index, 0xffffffff, 0xffffffff );  // ȡ����ʱ
	CH375SetBufUpload( index, 0 );  // �ָ�ֱ���ϴ�ģʽ
	CH375CloseDevice( index );

	printf( "\nExit.\n" );
	getch();
}