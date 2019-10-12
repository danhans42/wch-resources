/* 2004.06.05
****************************************
**  Copyright  (C)  W.ch  1999-2005   **
**  Web:  http://www.winchiphead.com  **
****************************************
**  USB Host File Interface for CH375 **
**  TC2.0@PC, KC7.0@MCS51             **
****************************************
*/
/* CH375 �����ļ�ϵͳ�ӿ� */
/* ֧��: FAT12/FAT16/FAT32 */

/* MCS-51��Ƭ��C���Ե�U���ļ���дʾ������, ������89C52���߸������ռ�ĵ�Ƭ�� */
/* ������������ʾ����/��/ɾ�����ļ����ļ� */
/* CH375��INT#���Ų��ò�ѯ��ʽ����, ���ݸ��Ʒ�ʽΪ"��DPTR����", �����ٶȽ���, ����������MCS51��Ƭ��
   ����������V2.8�����ϰ汾��CH375�ӳ���� */

/* ������FAT�ļ�ϵͳ�ͳ��ļ�����ؼ���֪ʶ�������û��ο�,֧��Сд��ĸ���ߺ��ֵȲ�����256���ַ����ļ���,
   �Ը���֪ʶ���˽���û����鲻Ҫ�漰���ļ���,����ֻʹ���ֳɵĶ��ļ���,���ļ���֧��8+3��ʽ,��д��ĸ���ߺ��� */

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
#define FILE_DATA_BUF_ADDR		0x0200	/* �ⲿRAM���ļ����ݻ���������ʼ��ַ,���������Ȳ�С��һ�ζ�д�����ݳ��� */
/* ������ʾ���õ�62256ֻ��32K�ֽ�,����CH375�ӳ�����512�ֽ�,�����ⲿRAMʣ�೤��Ϊ32256�ֽ� */
#define FILE_DATA_BUF_LEN		0x200	/* �ⲿRAM���ļ����ݻ�����,���������Ȳ�С��һ�ζ�д�����ݳ���,����Ҫ��С��0x400���� */
#define FILE_DATA_BUF_ADDR1		0x400	
unsigned char xdata FileDataBuf1[FILE_DATA_BUF_LEN] _at_ FILE_DATA_BUF_ADDR1 ;
#define CH375_INT_WIRE			INT0	/* P3.2, INT0, CH375���ж���INT#����,����CH375��INT#����,���ڲ�ѯ�ж�״̬ */

#define NO_DEFAULT_CH375_F_ENUM		1		/* δ����CH375FileEnumer����ʽ�ֹ�Խ�Լ���� */
#define NO_DEFAULT_CH375_F_QUERY	1		/* δ����CH375FileQuery����ʽ�ֹ�Խ�Լ���� */

#include "..\CH375HF6.H"				/* �������Ҫ֧��FAT32,��ô��ѡ��CH375HF4.H */
#include    "FILELONG.H"
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

/* ������ռ���ַ������Ƶ��ڲ�RAM��,�����ַ������� */
UINT8	mCopyCodeStringToIRAM1( UINT8 idata *iDestination, UINT8 xdata *iSource )
{
	UINT8	i = 0;
	while ( *iDestination = *iSource ) {
		iDestination ++;
		iSource ++;
		i ++;
	}
	return( i );
}
/* ������ռ���ַ������Ƶ��ڲ�RAM��,�����ַ������� */
UINT8	mCopyCodeStringToXRAM( UINT8 xdata *iDestination, UINT8 code *iSource )
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

/*������ļ�����У��ͣ�*/
unsigned char ChkSum (P_FAT_DIR_INFO pDir1)
{
	unsigned char FcbNameLen;
	unsigned char Sum;
	Sum = 0;
	for (FcbNameLen=0; FcbNameLen!=11; FcbNameLen++) {
		//if(pDir1->DIR_Name[FcbNameLen]==0x20)continue;
		Sum = ((Sum & 1) ? 0x80 : 0) + (Sum >> 1) + pDir1->DIR_Name[FcbNameLen];
	}
	return (Sum);
}

/*������������Ŀ¼��ͳ��ļ����Ƿ���ͬ,����00-15Ϊ�ҵ����ļ�����ͬ���ļ�00-15��ʾ��Ӧ���ļ�����Ŀ¼
���λ��,����0X80-8F��ʾ������Ŀ¼��Ľ�β,�Ժ���δ�õ�Ŀ¼��,����0FF��ʾ��������ƥ���Ŀ¼��*/
UINT8  mLDirCheck(P_FAT_DIR_INFO pDir2,F_LONG_NAME xdata *pLdir1){
	UINT8 i,j,k,sum,nodir,nodir1;
	F_LONG_NAME xdata *pLdir2;
	unsigned int  xdata *pLName;	
	unsigned char  data1;		
	for(i=0;i!=16;i++){						
		if(pDir2->DIR_Name[0]==0xe5){pDir2+=1;continue;}		/*������ɾ������������һĿ¼*/			/*�Ǳ�ɾ�����ļ���������*/
		if(pDir2->DIR_Name[0]==0x00){return i|0x80;}			/*���������¿ռ�û���ļ��������˳�*/
		if((pDir2->DIR_Attr==0x0f)|(pDir2->DIR_Attr==0x8)){pDir2+=1;continue;}  /*����ҵ����Ǿ�����߳��ļ�������*/	
		/*�ҵ�һ�����ļ���*/
			k=i-1;					/*���ļ�����Ӧ�ڶ��ļ�������*/
			if(i==0){				/*����˶��ļ����ڱ�������һ��*/
				pLdir2=pLdir1;			/*���ļ���Ӧ����һ���������һ��*/
				k=15;				/*��¼���ļ���λ��*/
				pLdir2+=15;			/*ƫ�Ƶ���β*/
			}	
			else pLdir2=(F_LONG_NAME xdata *)(pDir2-1);	/*ȡ���ļ���Ŀ¼��*/
		    	sum=ChkSum(pDir2); 				/*�����ۼӺ�*/
			pLName=LongFileName;				/*ָ��ָ���ĳ��ļ���*/
		 	nodir=0;					/*��ʼ����־*/
		 	nodir1=1;
		 	while(1){
				if((pLdir2->LDIR_Ord!=0xe5)&(pLdir2->LDIR_Attr==ATTR_LONG_NAME)& (pLdir2->LDIR_Chksum==sum)){  /*�ҵ�һ�����ļ���*/
					for(j=0;j!=5;j++){		
						if((pLdir2->LDIR_Name1[j]==0x00)|(*pLName==0))continue;	/*���������ļ�����β*/
						if((pLdir2->LDIR_Name1[j]==0xff)|(*pLName==0))continue;    /*���������ļ�����β*/
						if(pLdir2->LDIR_Name1[j]!=*pLName){		/*���������ñ�־*/	
							nodir=1;
							break;
						}
						pLName++;
					}
					if(nodir==1)break;								/*�ļ�����ͬ�˳�*/
					for(j=0;j!=6;j++){		
						if((pLdir2->LDIR_Name2[j]==0x00)|(*pLName==0))continue;
						if((pLdir2->LDIR_Name2[j]==0xff)|(*pLName==0))continue;
						if(*pLName!=pLdir2->LDIR_Name2[j]){nodir=1;break;}
						pLName++;
					}
					if(nodir==1)break;								/*�ļ�����ͬ�˳�*/
					for(j=0;j!=2;j++){		
						if((pLdir2->LDIR_Name3[j]==0x00)|(*pLName==0))continue;
						if((pLdir2->LDIR_Name3[j]==0xff)|(*pLName==0))continue;
						if(*pLName!=pLdir2->LDIR_Name3[j]){nodir=1;break;}
						pLName++;
					}
					if(nodir==1)break;								/*�ļ�����ͬ�˳�*/				
				 	if((data1=pLdir2->LDIR_Ord&0x40)==0x40){nodir1=0;break;} /*�ҵ����ļ��������ұȽϽ���*/	
			  	}
				else break;							/*����������Ӧ�ĳ��ļ����˳�*/
				if(k==0){
					pLdir2=pLdir1;
					pLdir2+=15;
					k=15;
			    }
				else {
					k=k-1;
					pLdir2-=1;
				}
		    }	
	    if(nodir1==0)	return i;			/*��ʾ�ҵ����ļ��������ض��ļ����ڵ�Ŀ¼��*/
		pDir2+=1;
    }	
	return 0xff;				/*ָ��������һ������û�ҵ���Ӧ�ĳ��ļ���*/
}

/*����ϼ���Ŀ¼����*/
UINT8 mChkName(unsigned char data *pJ){
		UINT8 i,j;
	j = 0xFF;
	for ( i = 0; i != sizeof( mCmdParam.Create.mPathName ); i ++ ) {  /* ���Ŀ¼·�� */
		if ( mCmdParam.Create.mPathName[ i ] == 0 ) break;
		if ( mCmdParam.Create.mPathName[ i ] == PATH_SEPAR_CHAR1 || mCmdParam.Create.mPathName[ i ] == PATH_SEPAR_CHAR2 ) j = i;  /* ��¼�ϼ�Ŀ¼ */
	}
	i = ERR_SUCCESS;
	if ( j == 0 || j == 2 && mCmdParam.Create.mPathName[1] == ':' ) {  /* �ڸ�Ŀ¼�´��� */
		mCmdParam.Open.mPathName[ 0]='/';
		mCmdParam.Open.mPathName[ 1]=0;
		i=CH375FileOpen();			/*�򿪸�Ŀ¼*/
		if ( i == ERR_OPEN_DIR ) i = ERR_SUCCESS;  /* �ɹ����ϼ�Ŀ¼ */	
	}
	else {
		if ( j != 0xFF ) {  /* ���ھ���·��Ӧ�û�ȡ�ϼ�Ŀ¼����ʼ�غ� */
			mCmdParam.Create.mPathName[ j ] = 0;
			i = CH375FileOpen( );  /* ���ϼ�Ŀ¼ */
			if ( i == ERR_SUCCESS ) i = ERR_MISS_DIR;  /* ���ļ�����Ŀ¼ */
			else if ( i == ERR_OPEN_DIR ) i = ERR_SUCCESS;  /* �ɹ����ϼ�Ŀ¼ */
			mCmdParam.Create.mPathName[ j ] = PATH_SEPAR_CHAR1;  /* �ָ�Ŀ¼�ָ��� */
		}		
	}
	*pJ=j;										/*ָ���з���һ������*/
	return i;
}

/*����ָ���ĳ��ļ���������Ӧ�Ķ��ļ������ļ����ռ���볤�ļ���,���ļ����ռ����·��00��β*/
UINT8  mLoopkUpSName(){
	UINT8  BlockSer1;				/*���������������ڼ���*/
	unsigned char xdata ParData[MAX_PATH_LEN];			/**/
	UINT16	tempSec;						/*����ƫ��*/
	UINT8 i,j,k;
	F_LONG_NAME   xdata *pLDirName; 
	P_FAT_DIR_INFO  pDirName;
	bit  FBuf;
	unsigned char data *pBuf1;
	CH375DirtyBuffer();
	for(k=0;k!=MAX_PATH_LEN;k++)ParData[k]=mCmdParam.Other.mBuffer[k];	/*���浱ǰ·��*/
	i=mChkName(&j);										
	if ( i == ERR_SUCCESS ) {  /* �ɹ���ȡ�ϼ�Ŀ¼����ʼ�غ� */
		BlockSer1=0;
		FBuf=0;					/*��ʼ��*/	
		tempSec=0;
		FileDataBuf1[0]=0xe5;	/*��һ���ã���Ч������*/
		k=0xff;
		while(1){							/*�����Ƕ�ȡ������Ŀ¼��*/			
			pDirName=FBuf?FILE_DATA_BUF_ADDR1:FILE_DATA_BUF_ADDR;		/*���ļ���ָ��ָ�򻺳���*/		
			pLDirName=FBuf?FILE_DATA_BUF_ADDR:FILE_DATA_BUF_ADDR1;
															/*��ǰ�������ļ�������*/																/*����ʹ��˫�򻺳�����ȥ�����ļ���*/
			mCmdParam.ReadX.mSectorCount=1;				/*��ȡһ��������*/
			mCmdParam.ReadX.mDataBuffer=FBuf?FILE_DATA_BUF_ADDR1:FILE_DATA_BUF_ADDR;
			FBuf=!FBuf;												/*��������־��ת*/
			i=CH375FileReadX( );
			if(mCmdParam.ReadX.mSectorCount==0){k=0xff;break;}
				k=mLDirCheck(pDirName,pLDirName);
				if(k!=0x0ff){break;}			/*�ҵ��ļ������ҵ��ļ���β�˳�*/
			}
			if(k<16){
						pDirName+=k;		/*���ҵ��ļ����ļ����ڴ�Ŀ¼��*/
			    		if(j!=0xff){	
					  		 for(k=0;k!=j+1;k++)mCmdParam.Other.mBuffer[k]=ParData[k];	
						}
						pBuf1=&mCmdParam.Other.mBuffer[j+1];	/*ȡ�ļ����ĵ�ַ*/
						for(i=0;i!=8;i++){
							if(pDirName->DIR_Name[i]==0x20)continue;
					    	else{
						 		*pBuf1=pDirName->DIR_Name[i];
						 		pBuf1++;
			 				}
						}
						if(pDirName->DIR_Name[i]!=0x20){
							*pBuf1='.';
						 	pBuf1++;					/*����Ŀ¼������չ��*/
						}
						for(;i!=11;i++){
							if(pDirName->DIR_Name[i]==0x20)continue;
							else {
								*pBuf1=pDirName->DIR_Name[i];
						 		pBuf1++;
			 				}
						}
						*pBuf1=00;
						 for(k=0;k!=j+1;k++)ParData[k]=mCmdParam.Other.mBuffer[k];
								/*���ƶ��ļ���*/
					i=CH375FileClose( );
					for(k=0;k!=j+1;k++)mCmdParam.Other.mBuffer[k]=ParData[k];			
		   	}
			else 	k=0xff;				/*����û�ҵ�ָ���ĳ��ļ����ļ���������������*/	 
			i=CH375FileClose( );
	  }
	  else {k=i;};
	  return k;	
}

/*������Դ����ļ��ĳ��ļ������ڶ��ļ����ռ�����·���Լ��ο����ļ������ڳ��ļ����ռ�������ļ����ļ�����UNICODE���룬
����״̬,00��ʾ�ɹ��������ڶ��ļ����ռ䷵����ʵ�Ķ��ļ���������Ϊ���ɹ�״̬*/
/*��������*/
UINT8  mCreatLName(){
	UINT8  BlockSer1;				/*���������������ڼ���*/
	unsigned char xdata ParData[MAX_PATH_LEN];			/**/
	UINT16	tempSec;						/*����ƫ��*/
	UINT8 i,j,k,x,sum,y,z;
	F_LONG_NAME   xdata *pLDirName; 
	P_FAT_DIR_INFO  pDirName,pDirName1;
	bit  FBuf;		
	unsigned char data *pBuf1;
	unsigned int xdata *pBuf;
	CH375DirtyBuffer(  );
	for(k=0;k!=MAX_PATH_LEN;k++)ParData[k]=mCmdParam.Other.mBuffer[k];			/**/
	i=mChkName(&j);
	if ( i == ERR_SUCCESS ) {  /* �ɹ���ȡ�ϼ�Ŀ¼����ʼ�غ� */
		BlockSer1=0;
		FBuf=0;					/*��ʼ��*/	
		tempSec=0;
		FileDataBuf1[0]=0xe5;	/*��Ч�ϴλ�����*/
		k=0xff;
		while(1){							/*�����Ƕ�ȡ������Ŀ¼��*/			
			pDirName=FBuf?FILE_DATA_BUF_ADDR1:FILE_DATA_BUF_ADDR;		/*���ļ���ָ��ָ�򻺳���*/		
			pLDirName=FBuf?FILE_DATA_BUF_ADDR:FILE_DATA_BUF_ADDR1;
			mCmdParam.ReadX.mSectorCount=1;				/*��ȡһ��������*/
			mCmdParam.ReadX.mDataBuffer=FBuf?FILE_DATA_BUF_ADDR1:FILE_DATA_BUF_ADDR;  /*��ǰ�������ļ�������,����ʹ��˫�򻺳�����ȥ�����ļ���*/
			FBuf=!FBuf;												/*��������־��ת*/
			i=CH375FileReadX( );
			if(mCmdParam.ReadX.mSectorCount==0){k=0xff;break;}
			tempSec+=1;
			k=mLDirCheck(pDirName,pLDirName);
			z=k;
			z&=0x0f;
			if(k!=0x0ff){break;}			/*�ҵ��ļ������ҵ��ļ���β�˳�*/
		 }
		   if(k<16){
						pDirName+=k;		/*���ҵ��ļ����ļ����ڴ�Ŀ¼��*/
			    		if(j!=0xff){	
					  	 	for(k=0;k!=j+1;k++)mCmdParam.Other.mBuffer[k]=ParData[k];	
						}
						pBuf1=&mCmdParam.Other.mBuffer[j+1];	/*ȡ�ļ����ĵ�ַ*/
						//else pBuf1=&mCmdParam.Other.mBuffer;		
						for(i=0;i!=8;i++){
							if(pDirName->DIR_Name[i]==0x20)continue;
							else {
						 		*pBuf1=pDirName->DIR_Name[i];
						 		pBuf1++;
			 				}
						}
						if(pDirName->DIR_Name[i]!=0x20){
							*pBuf1='.';
							 pBuf1++;
						}
						for(;i!=11;i++){
							if(pDirName->DIR_Name[i]==0x20)continue;
							else {
								*pBuf1=pDirName->DIR_Name[i];
						 		pBuf1++;
			 				}

						}					/*���ƶ��ļ���*/
				    i=CH375FileClose();
				    	i=CH375FileCreate();					/*�ɻ�����Ҫ��Ҫ�ָ����ս���˺���ʱ�Ĵغ�*/
						return i;		/*�����ļ�,����״̬*/
		   	}
			else {					/*��ʾĿ¼��ö�ٵ�����λ�ã�Ҫ�����ļ�*/
				if(k==0xff){z=00;tempSec+=1;}
				i=CH375FileClose();
					for(k=0;k!=MAX_PATH_LEN;k++)mCmdParam.Other.mBuffer[k]=ParData[k];		/*�Դ����ļ����ļ���*/					
					for(x=0x31;x!=0x3a;x++){					/*���ɶ��ļ���*/
						for(y=0x31;y!=0x3a;y++){
							for(i=0x31;i!=0x3a;i++){
								mCmdParam.Other.mBuffer[j+7]=i;
								mCmdParam.Other.mBuffer[j+6]='~';	
								mCmdParam.Other.mBuffer[j+5]=y;
								mCmdParam.Other.mBuffer[j+4]=x;
								if(CH375FileOpen()!=ERR_SUCCESS)goto XAA1;  /**/	
							}
						}
					
					}
					 i=0xff;
					 goto   XBB;				/*�����޷���ȷ����*/  
XAA1:
					i=CH375FileCreate();
					if(i!=ERR_SUCCESS);//{goto XCC;}			/*�������ܼ�������*/
					for(k=0;k!=MAX_PATH_LEN;k++)ParData[k]=mCmdParam.Other.mBuffer[k];		/*�Դ����ļ����ļ���*/	
					i=mChkName(&j);
					mCmdParam.Locate.mSectorOffset=tempSec-1;
					i=CH375FileLocate();
					if(i!=ERR_SUCCESS);//{goto XCC;}			/*�������ܼ�������*/
					mCmdParam.ReadX.mSectorCount=1;
					mCmdParam.ReadX.mDataBuffer=FILE_DATA_BUF_ADDR;	
					pDirName=FILE_DATA_BUF_ADDR;
					pDirName+=z;					/*ָ�򴴽��ļ�����ƫ��*/					
					i=CH375FileReadX();				/*��ȡ��һ�����������ݣ�ȡ��һ��Ŀ¼����ǸղŴ����Ķ��ļ���*/					
					if(i!=ERR_SUCCESS);//{goto XCC;}				/*����Ҫ����������*/
					for(i=0;i!=CH375_FILE_LONG_NAME;i++){ 
						if(LongFileName[i]==00)break;			/*���㳤�ļ����ĳ���*/
					}
					for(k=i+1;k!=CH375_FILE_LONG_NAME;k++){ 	/*����Ч��Ŀ¼�����*/
						LongFileName[k]=0xffff;
					}
					pBuf=FILE_DATA_BUF_ADDR1;	/**/
					*pBuf=0;						/*�建����һ���ֽ�*/														
					*pBuf=0;						/*�建����һ���ֽ�*/																	
					k=i/13;							/*ȡ���ļ�������*/
					i=i%13;
					if(i!=0)k=k+1;				/*����������һ��*/
					i=k;
					//pLDirName=(F_LONG_NAME   xdata *)pDirName;	
					k=i+z;					/*zΪ���ļ�ƫ��,z-1Ϊ���ļ�ƫ��*/
					if(k<16){
						pDirName1=FILE_DATA_BUF_ADDR;
						pDirName1+=k;
						pLDirName=FILE_DATA_BUF_ADDR;	
						pLDirName+=k-1;
					}
					else if(k==16){
					    pDirName1=FILE_DATA_BUF_ADDR1;
						pDirName1+=k-16;
						pLDirName=FILE_DATA_BUF_ADDR;
						pLDirName+=k-1;
						}
					else if(k>16){
					    pDirName1=FILE_DATA_BUF_ADDR1;
						pDirName1+=k-16;
						pLDirName=FILE_DATA_BUF_ADDR1;
						pLDirName+=k-1-16;
						}
					/*���ƶ��ļ���,�����ļ������Ƶ�ָ������*/
					pDirName1->DIR_NTRes=pDirName->DIR_NTRes;
					pDirName1->DIR_CrtTimeTenth=pDirName->DIR_CrtTimeTenth;
					pDirName1->DIR_CrtTime=pDirName->DIR_CrtTime;
					pDirName1->DIR_CrtDate=pDirName->DIR_CrtDate;
					pDirName1->DIR_LstAccDate=pDirName->DIR_LstAccDate;
					pDirName1->DIR_FstClusHI=pDirName->DIR_FstClusHI;
					pDirName1->DIR_WrtTime=pDirName->DIR_WrtTime;
					pDirName1->DIR_WrtDate=pDirName->DIR_WrtDate;
					pDirName1->DIR_FstClusLO=pDirName->DIR_FstClusLO;
					pDirName1->DIR_FileSize=pDirName->DIR_FileSize;
					pDirName1->DIR_Attr=pDirName->DIR_Attr;
					
					pDirName1->DIR_Name[0]=pDirName->DIR_Name[0];
					pDirName1->DIR_Name[1]=pDirName->DIR_Name[1];
					pDirName1->DIR_Name[2]=pDirName->DIR_Name[2];
					pDirName1->DIR_Name[3]=pDirName->DIR_Name[3];
					pDirName1->DIR_Name[4]=pDirName->DIR_Name[4];
					pDirName1->DIR_Name[5]=pDirName->DIR_Name[5];
					pDirName1->DIR_Name[6]=pDirName->DIR_Name[6];
					pDirName1->DIR_Name[7]=pDirName->DIR_Name[7];
					pDirName1->DIR_Name[8]=pDirName->DIR_Name[8];
					pDirName1->DIR_Name[9]=pDirName->DIR_Name[9];
					pDirName1->DIR_Name[10]=pDirName->DIR_Name[10];
					pDirName1->DIR_Name[10]=pDirName->DIR_Name[10];
					sum=ChkSum(pDirName1);				/*�����ۼӺ�*/
					pBuf=LongFileName;					/*ָ���ļ����ռ�*/
					y=1;
					if(k>16){
						for(i=1;i!=k-16+1;i++){					/*>?????*/
							pLDirName->LDIR_Ord=y;
							pLDirName->LDIR_Name1[0]=*pBuf;
							pBuf++;
							pLDirName->LDIR_Name1[1]=*pBuf;
							pBuf++;
							pLDirName->LDIR_Name1[2]=*pBuf;
							pBuf++;
							pLDirName->LDIR_Name1[3]=*pBuf;
							pBuf++;
							pLDirName->LDIR_Name1[4]=*pBuf;
							pBuf++;
							pLDirName->LDIR_Attr=0x0f;
							pLDirName->LDIR_Type=0;		
							pLDirName->LDIR_Chksum=sum;
							pLDirName->LDIR_Name2[0]=*pBuf;
							pBuf++;		
							pLDirName->LDIR_Name2[1]=*pBuf;
							pBuf++;
							pLDirName->LDIR_Name2[2]=*pBuf;
							pBuf++;
							pLDirName->LDIR_Name2[3]=*pBuf;
							pBuf++;
							pLDirName->LDIR_Name2[4]=*pBuf;
							pBuf++;
							pLDirName->LDIR_Name2[5]=*pBuf;
							pBuf++;
							pLDirName->LDIR_FstClusLO[0]=0;
							pLDirName->LDIR_FstClusLO[1]=0;
							pLDirName->LDIR_Name3[0]=*pBuf;
							pBuf++;
							pLDirName->LDIR_Name3[1]=*pBuf;
							pBuf++;		
							pLDirName--;
							y+=1;						
						}
						k=16;
						k=16;
						k=16;
						i=0;
						pLDirName=FILE_DATA_BUF_ADDR;
						pLDirName+=k-1;
					  }	
					  if(k>16)k=16;
							for(i=1;i!=k-z;i++){					/*>?????*/
							pLDirName->LDIR_Ord=y;
							pLDirName->LDIR_Name1[0]=*pBuf;
							pBuf++;
							pLDirName->LDIR_Name1[1]=*pBuf;
							pBuf++;
							pLDirName->LDIR_Name1[2]=*pBuf;
							pBuf++;
							pLDirName->LDIR_Name1[3]=*pBuf;
							pBuf++;
							pLDirName->LDIR_Name1[4]=*pBuf;
							pBuf++;
							pLDirName->LDIR_Attr=0x0f;
							pLDirName->LDIR_Type=0;		
							pLDirName->LDIR_Chksum=sum;
							pLDirName->LDIR_Name2[0]=*pBuf;
							pBuf++;		
							pLDirName->LDIR_Name2[1]=*pBuf;
							pBuf++;
							pLDirName->LDIR_Name2[2]=*pBuf;
							pBuf++;
							pLDirName->LDIR_Name2[3]=*pBuf;
							pBuf++;
							pLDirName->LDIR_Name2[4]=*pBuf;
							pBuf++;
							pLDirName->LDIR_Name2[5]=*pBuf;
							pBuf++;
							pLDirName->LDIR_FstClusLO[0]=0;
							pLDirName->LDIR_FstClusLO[1]=0;
							pLDirName->LDIR_Name3[0]=*pBuf;
							pBuf++;
							pLDirName->LDIR_Name3[1]=*pBuf;
							pBuf++;		
							pLDirName--;
							y+=1;
						}
						pLDirName->LDIR_Ord=y|0x40;
						pLDirName->LDIR_Name1[0]=*pBuf;
						pBuf++;
						pLDirName->LDIR_Name1[1]=*pBuf;
						pBuf++;
						pLDirName->LDIR_Name1[2]=*pBuf;
						pBuf++;
						pLDirName->LDIR_Name1[3]=*pBuf;
						pBuf++;
						pLDirName->LDIR_Name1[4]=*pBuf;
						pBuf++;
						pLDirName->LDIR_Attr=0x0f;
						pLDirName->LDIR_Type=0;		
						pLDirName->LDIR_Chksum=sum;
						pLDirName->LDIR_Name2[0]=*pBuf;
						pBuf++;		
						pLDirName->LDIR_Name2[1]=*pBuf;
						pBuf++;
						pLDirName->LDIR_Name2[2]=*pBuf;
						pBuf++;
						pLDirName->LDIR_Name2[3]=*pBuf;
						pBuf++;
						pLDirName->LDIR_Name2[4]=*pBuf;
						pBuf++;
						pLDirName->LDIR_Name2[5]=*pBuf;
						pBuf++;
						pLDirName->LDIR_FstClusLO[0]=0;
						pLDirName->LDIR_FstClusLO[1]=0;
						pLDirName->LDIR_Name3[0]=*pBuf;
						pBuf++;
						pLDirName->LDIR_Name3[1]=*pBuf;
						pBuf++;		
						pBuf=(unsigned int xdata *)pDirName1;
						pBuf+=16;			
						if(pBuf<(FILE_DATA_BUF_ADDR+0x200)){
							i=2;
							while(1){
								*pBuf=0;
								pBuf++;
								if(pBuf==FILE_DATA_BUF_ADDR+0x200)break;			
							}
							i++;
						}
						else if(pBuf<(FILE_DATA_BUF_ADDR1+0x200)){
							i=1;
							while(1){
								*pBuf=0;
								pBuf++;
								if(pBuf==(FILE_DATA_BUF_ADDR1+0x200))break;			
							}
							i++;
						}
					mCmdParam.Locate.mSectorOffset=tempSec-1;
					CH375DirtyBuffer();
					i=CH375FileLocate();
					if(i!=ERR_SUCCESS);			/*�������ܼ�������*/
					mCmdParam.ReadX.mSectorCount=1;						/*��������*/
					mCmdParam.ReadX.mDataBuffer=FILE_DATA_BUF_ADDR;	
						CH375DirtyBuffer();						
					i=CH375FileWriteX();				/*��ȡ��һ�����������ݣ�ȡ��һ��Ŀ¼����ǸղŴ����Ķ��ļ���*/					
					CH375DirtyBuffer(  );
					if(i!=ERR_SUCCESS);				/*����Ҫ����������*/
					pBuf=FILE_DATA_BUF_ADDR1;	/**/
					if(*pBuf!=0){
					 	mCmdParam.ReadX.mSectorCount=1;
						mCmdParam.ReadX.mDataBuffer=FILE_DATA_BUF_ADDR1;	
						i=CH375FileWriteX();
					CH375DirtyBuffer(  );
					}
				/*������ڸ�Ŀ¼�²���Ӧ�رո�Ŀ¼*/
				/*���滹Ҫ���ļ�*/
			   i=CH375FileClose();	
			for(k=0;k!=MAX_PATH_LEN;k++)mCmdParam.Other.mBuffer[k]=ParData[k];		/*�Դ����ļ����ļ���*/	
			  i=CH375FileOpen();					/*�򿪴������ļ�*/
			 return i;
			}
	}
XBB: {
  		return i=0xfe;
	}
}

/*ɾ��ָ�����ļ������ļ���ͬʱɾ����Ӧ�ĳ��ļ���������״̬�ͷǳ��ļ�������ͬ*/
UINT8  mdeleteFile(){
	UINT8  BlockSer1;				/*���������������ڼ���*/
	unsigned char xdata ParData[MAX_PATH_LEN];	/*����һ������ļ����Ļ�����*/
	UINT16	tempSec;						/*����ƫ��*/
	UINT8 a,i,j,k,x,sum;
	F_LONG_NAME   xdata *pLDirName;		/*���ļ���ָ��*/ 
	P_FAT_DIR_INFO  pDirName;			/*���ļ���ָ��*/	
	bit  FBuf;							/*����һ���ļ��������ķ�תλ*/
	unsigned char xdata *pBuf;			/*ָ�򻺳�����ָ��*/
	for(k=0;k!=MAX_PATH_LEN;k++)ParData[k]=mCmdParam.Other.mBuffer[k];			/**/
	i=mChkName(&j);
	if ( i == ERR_SUCCESS ) {  /* �ɹ���ȡ�ϼ�Ŀ¼����ʼ�غ� */
		BlockSer1=0;
		FBuf=0;					/*��ʼ��*/	
		tempSec=0;				
	    while(1){							/*�����Ƕ�ȡ������Ŀ¼��*/
				pDirName=FBuf?FILE_DATA_BUF_ADDR1:FILE_DATA_BUF_ADDR;		/*���ļ���ָ��ָ�򻺳���*/		
				mCmdParam.ReadX.mSectorCount=1;				/*��ȡһ��������*/
				mCmdParam.ReadX.mDataBuffer=FBuf?FILE_DATA_BUF_ADDR1:FILE_DATA_BUF_ADDR;  /*��ǰ�������ļ�������,����ʹ��˫�򻺳�����ȥ�����ļ���*/
				FBuf=!FBuf;												/*��������־��ת*/
				i=CH375FileReadX( );
				if(i!=ERR_SUCCESS)goto XLL;				
				if(mCmdParam.ReadX.mSectorCount==0){k=16;break;}			/*��ʾû�����ݶ���*/
				tempSec+=1;												/*����������һ*/
				for(k=0;k!=16;k++){																	
				pBuf=&ParData[j+1];						
				if(pDirName->DIR_Name[0]==0){k=15;a=1;continue;}		/*��һ���ֽ�Ϊ0����ʾ�Ժ�û����Ч��Ŀ¼����*/
				if(pDirName->DIR_Name[0]==0xe5){pDirName++;continue;}			/*��һ���ֽ�Ϊ0XE5��ʾ���ɾ��*/
				if(pDirName->DIR_Attr==ATTR_VOLUME_ID){pDirName++;continue;}		/*Ϊ���꣬������*/
				if(pDirName->DIR_Attr==ATTR_LONG_NAME){pDirName++;continue;}		/*Ϊ���ļ�����������*/
				for(i=0;i!=8;i++){									/*�����ļ����Ƿ���ͬ*/
					if(pDirName->DIR_Name[i]==0x20)continue;		/*Ϊ20������*/        
					if(pDirName->DIR_Name[i]!=*pBuf)break;
					else pBuf++;
				}
				if(i!=8){pDirName++;continue;}						/*û���ҵ�ƥ��Ķ��ļ���*/
				if(*pBuf=='.')pBuf++;
				for(;i!=11;i++){
					if(pDirName->DIR_Name[i]==0x20)continue;				
					if(pDirName->DIR_Name[i]!=*pBuf)break;		
					else pBuf++;
				}
				if(i==11){break;}								/*��ʾ�ҵ��ļ���*/
				pDirName++;
			}
			if(i==11)break;								/*�ҵ�*/
			if(a==1){k=16;break;}
		}
		if(k!=16){
				 x=0;      /*�����Ҷ��ļ���Ȼ��ɾ��*/
				 sum=ChkSum(pDirName); 					       /*�����*/				
				 pLDirName=FBuf?FILE_DATA_BUF_ADDR:FILE_DATA_BUF_ADDR1;	/*���ļ���ָ��ָ�򻺳���*/
				 pLDirName+=k-1;
				 if(k==0){pLDirName=FBuf?FILE_DATA_BUF_ADDR1:FILE_DATA_BUF_ADDR;  /*������ļ����ǴԵ�һ�鿪ʼ�ģ����ļ�����Ҫ�����ƶ�*/
						pLDirName+=15;						
						k=15;					
					}
				while(1){	
					if((pLDirName->LDIR_Attr==0x0f)&(pLDirName->LDIR_Chksum==sum)&(pLDirName->LDIR_Ord!=0xe5)){
						pLDirName->LDIR_Ord=0xe5;				/*ɾ�����ļ���*/
				   		x++;
						k=k-1;
					}
					else break;							/*û�г��ļ���������*/
					if(x==15)break;						/*������Ƴ��ļ���Ϊ16*13�ֽ�*/
					if(k==0){								/*�����ƶ��ļ�ָ��*/					
						pLDirName=FBuf?FILE_DATA_BUF_ADDR1:FILE_DATA_BUF_ADDR;
						pLDirName+=15;														
					}
					else {pLDirName-=1; k-=1;}
				}
					if(tempSec!=0){
						tempSec-=1;
						mCmdParam.Locate.mSectorOffset=(unsigned long)tempSec;		/*����������Ŀ¼������д��*/
						i=CH375FileLocate();
						if(i!=ERR_SUCCESS)goto XLL;	
						mCmdParam.ReadX.mSectorCount=1;
						mCmdParam.ReadX.mDataBuffer=FBuf?FILE_DATA_BUF_ADDR:FILE_DATA_BUF_ADDR1;
						i=CH375FileWriteX( );
						if(i!=ERR_SUCCESS)goto XLL;
					}
				if(tempSec!=0){
					tempSec-=1;
					mCmdParam.Locate.mSectorOffset=(unsigned long)tempSec;		/*����������Ŀ¼������д��*/
					i=CH375FileLocate();
					if(i!=ERR_SUCCESS)goto XLL;	
					mCmdParam.ReadX.mSectorCount=1;
					mCmdParam.ReadX.mDataBuffer=FBuf?FILE_DATA_BUF_ADDR1:FILE_DATA_BUF_ADDR;
					i=CH375FileWriteX( );
					if(i!=ERR_SUCCESS)goto XLL;
				}
			}
	}		
	CH375DirtyBuffer(  );		
	i=CH375FileClose( );		
	CH375DirtyBuffer(  );						/*����̻�����*/
	for(k=0;k!=MAX_PATH_LEN;k++)mCmdParam.Other.mBuffer[k]=ParData[k];			/**/	
	i=CH375FileErase( );
	return i;
XLL:  return i;								/*����ʱ�����ش�����Ϣ*/						
}

/*��ȡָ�����ļ����ĳ��ļ���,���س��ļ����ڳ��ļ����ռ�*/
UINT8  mLookUpLName(){
	UINT8  BlockSer1;				/*���������������ڼ���*/
	unsigned char xdata ParData[MAX_PATH_LEN];			/**/
	UINT16	tempSec;						/*����ƫ��*/
	UINT8 a,i,j,x,k,sum;
	F_LONG_NAME   xdata *pLDirName; 
	P_FAT_DIR_INFO  pDirName;
	bit  FBuf;
	unsigned char xdata *pBuf;
	unsigned int xdata *pBuf1;
	for(k=0;k!=MAX_PATH_LEN;k++)ParData[k]=mCmdParam.Other.mBuffer[k];			/**/
		i=mChkName(&j);	
	if ( i == ERR_SUCCESS ) {  /* �ɹ���ȡ�ϼ�Ŀ¼����ʼ�غ� */
		BlockSer1=0;
		FBuf=0;					/*��ʼ��*/	
		tempSec=0;				
		while(1){							/*�����Ƕ�ȡ������Ŀ¼��*/
			pDirName=FBuf?FILE_DATA_BUF_ADDR1:FILE_DATA_BUF_ADDR;		/*���ļ���ָ��ָ�򻺳���*/		
			mCmdParam.ReadX.mSectorCount=1;				/*��ȡһ��������*/
			mCmdParam.ReadX.mDataBuffer=FBuf?FILE_DATA_BUF_ADDR1:FILE_DATA_BUF_ADDR;  /*��ǰ�������ļ�������,����ʹ��˫�򻺳�����ȥ�����ļ���*/
			FBuf=!FBuf;												/*��������־��ת*/
			i=CH375FileReadX( );
			if(i!=ERR_SUCCESS)goto XFF;
			if(mCmdParam.ReadX.mSectorCount==0){k=16;break;}			/*��ʾû�����ݶ���*/
			tempSec+=1;												/*����������һ*/
			for(k=0;k!=16;k++){																	
				pBuf=&ParData[j+1];						
				if(pDirName->DIR_Name[i]==0){k=15;a=1;continue;}		/*��һ���ֽ�Ϊ0����ʾ�Ժ�û����Ч��Ŀ¼����*/
				if(pDirName->DIR_Name[i]==0xe5){pDirName++;continue;}			/*��һ���ֽ�Ϊ0XE5��ʾ���ɾ��*/
				if(pDirName->DIR_Attr==ATTR_VOLUME_ID){pDirName++;continue;}		/*Ϊ���꣬������*/
				if(pDirName->DIR_Attr==ATTR_LONG_NAME){pDirName++;continue;}		/*Ϊ���ļ�����������*/
				for(i=0;i!=8;i++){									/*�����ļ����Ƿ���ͬ*/
				if(pDirName->DIR_Name[i]==0x20)continue;		/*Ϊ20������*/        
				if(pDirName->DIR_Name[i]!=*pBuf)break;
				else pBuf++;
			}
			if(i!=8){pDirName++;continue;}						/*û���ҵ�ƥ��Ķ��ļ���*/
			if(*pBuf=='.')pBuf++;
			for(;i!=11;i++){
				if(pDirName->DIR_Name[i]==0x20)continue;				
				if(pDirName->DIR_Name[i]!=*pBuf)break;		
				else pBuf++;
			}
			if(i==11){break;}								/*��ʾ�ҵ��ļ���*/
			pDirName++;
			}
			if(i==11)break;								/*�ҵ�*/
			if(a==1){k=16;break;}
		}
		if(k!=16){
					pBuf1=LongFileName;
						 x=0;
				 sum=ChkSum(pDirName); 					       /*�����*/				
				 pLDirName=FBuf?FILE_DATA_BUF_ADDR:FILE_DATA_BUF_ADDR1;	/*���ļ���ָ��ָ�򻺳���*/
				 pLDirName+=k-1;
				 if(k==0){pLDirName=FBuf?FILE_DATA_BUF_ADDR1:FILE_DATA_BUF_ADDR;  /*������ļ����ǴԵ�һ�鿪ʼ�ģ����ļ�����Ҫ�����ƶ�*/
						pLDirName+=15;						
						k=15;					
					}
				while(1){	
					if(pLDirName->LDIR_Attr==0x0f&pLDirName->LDIR_Chksum==sum&pLDirName->LDIR_Ord!=0xe5){
						for(j=0;j!=5;j++){	
						*pBuf1=pLDirName->LDIR_Name1[j];
						pBuf1++;
						}
						for(;j!=11;j++){	
						*pBuf1=pLDirName->LDIR_Name2[j-5];
						pBuf1++;
						}
							for(;j!=13;j++){	
						*pBuf1=pLDirName->LDIR_Name3[j-11];
						pBuf1++;
						}
						/*���ｫ���ļ������Ƴ�ȥ�����16*13�����ļ���*/					
				   		x++;	 
					}
					else break;							/*û�г��ļ���������*/
					if(x==15)break;						/*������Ƴ��ļ���Ϊ16*13�ֽ�*/
					if(k==0){						/*�����ƶ��ļ�ָ��*/					
						pLDirName=FBuf?FILE_DATA_BUF_ADDR1:FILE_DATA_BUF_ADDR;
						pLDirName+=15;														
					}
					else {pLDirName-=1; k-=1;}
				}
			}					
	}
	*pBuf1=0;
	pBuf1++;
	i=CH375FileClose( );	
	return 0;					
XFF:  return i;
}

main( ) {
	UINT8	i,k;
	UINT16 X;
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
	//	printf( "Wait Udisk\n" );
		while ( CH375DiskStatus != DISK_CONNECT ) xQueryInterrupt( );  /* ��ѯCH375�жϲ������ж�״̬,�ȴ�U�̲��� */
		LED_OUT_ACT( );  /* LED�� */
		mDelaymS( 200 );  /* ��ʱ,��ѡ����,�е�USB�洢����Ҫ��ʮ�������ʱ */

/* ���U���Ƿ�׼����,��ЩU�̲���Ҫ��һ��,����ĳЩU�̱���Ҫִ����һ�����ܹ��� */
		for ( i = 0; i < 5; i ++ ) {  /* �е�U�����Ƿ���δ׼����,�������Ա����� */
			mDelaymS( 100 );
	//		printf( "Ready ?\n" );
			if ( CH375DiskReady( ) == ERR_SUCCESS ) break;  /* ��ѯ�����Ƿ�׼���� */
		}
/* ��ѯ������������ */
//		printf( "DiskSize\n" );
//		i = CH375DiskSize( );
//		mStopIfError( i );
//		printf( "TotalSize = %u MB \n", (unsigned int)( mCmdParam.DiskSize.mDiskSizeSec >> 11 ) );  /* ��ʾΪ��MBΪ��λ������ */
		LED_RUN_ACT( );  /* ��ʼ����U�� */

		X=0X4100;
		for(k=0;k!=0x0F;k++){		/*��ʾ�������ļ���*/
			mCmdParam.Erase.mPathName[0]='/';   /*�����ο����ļ���*/
			mCmdParam.Erase.mPathName[1]='C';
			mCmdParam.Erase.mPathName[2]='/';
			mCmdParam.Erase.mPathName[3]='A';
			mCmdParam.Erase.mPathName[4]='B';
			mCmdParam.Erase.mPathName[5]='C';
			mCmdParam.Erase.mPathName[6]='D';		
			mCmdParam.Erase.mPathName[7]='A';
			mCmdParam.Erase.mPathName[8]='~';
			mCmdParam.Erase.mPathName[9]='1';
			mCmdParam.Erase.mPathName[10]='2';
			mCmdParam.Erase.mPathName[11]='.';
			mCmdParam.Erase.mPathName[12]='C';
			mCmdParam.Erase.mPathName[13]=00;
			LongFileName[0]=0X4100;					/*����UNICODE�ĳ��ļ���*/						
			LongFileName[1]=0X4200;    /* ����С�����ݸ�ʽ�ĵ�Ƭ��,����AVR/MSP430/ARM����0x0042,��ͬ */
			LongFileName[2]=0X4300;
			LongFileName[3]=0X4400;
			LongFileName[4]=0X4500;
			LongFileName[5]=0X4600;
			LongFileName[6]=0X4700;
			LongFileName[7]=0X4800;
			LongFileName[8]=0X4100;
			LongFileName[9]=0X6300;
			LongFileName[10]=0X6200;
			LongFileName[11]=0X6100;
			LongFileName[12]=X;
			LongFileName[13]=X;
			LongFileName[14]=0X2e00;
			LongFileName[15]=0X4300;
			LongFileName[16]=0X0000;
	
			i=mCreatLName();				/*�������ļ���*/			
		X+=0X100;
	}

		X=0X4100;
		for(k=0;k!=0x0F;k++){		/*��ʾ�������ļ���*/
		mCmdParam.Erase.mPathName[0]='/';   /*�����ο����ļ���*/
		mCmdParam.Erase.mPathName[1]='A';
		mCmdParam.Erase.mPathName[2]='B';
		mCmdParam.Erase.mPathName[3]='C';
		mCmdParam.Erase.mPathName[4]='D';		
		mCmdParam.Erase.mPathName[5]='A';
		mCmdParam.Erase.mPathName[6]='~';
		mCmdParam.Erase.mPathName[7]='1';
		mCmdParam.Erase.mPathName[8]='1';
		mCmdParam.Erase.mPathName[9]='.';
		mCmdParam.Erase.mPathName[10]='C';
		mCmdParam.Erase.mPathName[11]=00;

		LongFileName[0]=0X6100;					/*����UNICODE�ĳ��ļ���*/						
		LongFileName[1]=0X4200;
		LongFileName[2]=0X6300;
		LongFileName[3]=0X4400;
		LongFileName[4]=0X4500;
		LongFileName[5]=0X6500;
		LongFileName[6]=0X4700;
		LongFileName[7]=0X4800;
		LongFileName[8]=0X4100;
		LongFileName[9]=0X6300;
		LongFileName[10]=0X6200;
		LongFileName[11]=0X6100;
		LongFileName[12]=X;
		LongFileName[13]=X;
		LongFileName[14]=0X2e00;
		LongFileName[15]=0X4300;
		LongFileName[16]=0X0000;
	
			i=mCreatLName();				/*�������ļ���*/			
		X+=0X100;
	}
	X=0X4100;

for(k=0;k!=0x4;k++){	    /*���Ҳ�ɾ��*/
		mCmdParam.Erase.mPathName[0]='/';		/*����·��*/
		mCmdParam.Erase.mPathName[1]='C';
		mCmdParam.Erase.mPathName[2]='/';
		mCmdParam.Erase.mPathName[3]=00;
		LongFileName[0]=0X4100;				/*�������ļ���*/					
		LongFileName[1]=0X4200;
		LongFileName[2]=0X4300;
		LongFileName[3]=0X4400;
		LongFileName[4]=0X4500;
		LongFileName[5]=0X4600;
		LongFileName[6]=0X4700;
		LongFileName[7]=0X4800;
		LongFileName[8]=0X4100;
		LongFileName[9]=0X6300;
		LongFileName[10]=0X6200;
		LongFileName[11]=0X6100;
		LongFileName[12]=X;
		LongFileName[13]=X;
		LongFileName[14]=0X2e00;
		LongFileName[15]=0X4300;
		LongFileName[16]=0X0000;

		i=mLoopkUpSName();			/*���Ҷ��ļ���*/
		i=mdeleteFile();			/*ɾ���ļ�*/	
		X+=0X100;
	}

		X=0;
	while(1)
	{	
		mCmdParam.Erase.mPathName[0]='/';		/*����·��*/
		mCmdParam.Erase.mPathName[1]='C';
		mCmdParam.Erase.mPathName[2]='/';
		mCmdParam.Erase.mPathName[3]='*';
		mCmdParam.Erase.mPathName[4]=X;
		mCmdParam.Erase.mPathName[5]=00;
		mCmdParam.Erase.mPathName[6]=00;
		i=CH375FileOpen();
		if ( i == ERR_MISS_FILE ) break;  /* ��Ҳ��������ƥ����ļ�,�Ѿ�û��ƥ����ļ��� */
		if ( i == ERR_FOUND_NAME ) {  /* ��������ͨ�����ƥ����ļ���,�ļ�����������·������������� */
		   i=mLookUpLName();		/*������Ӧ�ĳ��ļ���*/
		 }
		X++;
		if(X==0xfE)break;
	}		
		LED_WR_INACT( );
		LED_RUN_INACT( );
		printf( "Take out\n" );
		while ( CH375DiskStatus != DISK_DISCONNECT ) xQueryInterrupt( );  /* ��ѯCH375�жϲ������ж�״̬,�ȴ�U�̰γ� */
		LED_OUT_INACT( );  /* LED�� */
		mDelaymS( 200 );
	}
}