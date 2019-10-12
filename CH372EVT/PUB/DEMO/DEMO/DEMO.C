// 2003.09.08, 2003.12.28, 2004.12.10, 2004.12.28
//****************************************
//**  Copyright  (C)  W.ch  1999-2004   **
//**  Web:  http://www.winchiphead.com  **
//****************************************
//**  EXE for USB interface chip CH375  **
//**  C, VC5.0                          **
//****************************************
//
// USB���߽ӿ�оƬCH375��Ӧ�ò���ʾ���� V1.2
// �Ͼ��ߺ�������޹�˾  ����: W.ch 2004.12
// CH375-EXE  V1.2 , Support: Ctrl/Bulk/Int
// ���л���: Windows 98/ME, Windows 2000/XP
// support USB chip: CH372/CH375
//


#define		mTHIS_VERSION		0x12		// ��ǰ�汾
#define		mTHIS_VER_STR		"1.2"		// ��ǰ�汾�ַ���

#include	<windows.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	"CH375DLL.H"		// CH372/CH375�Ķ�̬���ӿ�
#pragma comment(lib,"CH375DLL")

#include	"DEMO.H"

#define		IDC_INT_PRESS		2222	// ���������¼��ź�
#define		IDC_INT_RELEASE		2223	// �����ͷ��¼��ź�
#define		IDC_INT_DEVARRIVAL	2224	// �豸�����¼��ź�
#define     IDC_INT_DEVREMOVAL  2225    // �豸�γ��¼��ź�

typedef	struct	_COMMAND_PACKET	{	// �Զ����������ṹ
	UCHAR	mCommandCode;		// ����������,������Ķ���
	UCHAR	mCommandCodeNot;	// ������ķ���,����У�������
	union	{
		UCHAR	mParameter[5];	// ����
		struct {
			UCHAR	mBufferID;  // ������ʶ����,���������MCS51��Ƭ������: 1-ר�ù��ܼĴ���SFR, 2-�ڲ�RAM, 3-�ⲿRAM, ����������ʵ��ֻ��ʾ�ڲ�RAM
			USHORT	mBufferAddr;	// ��д��������ʼ��ַ,Ѱַ��Χ��0000H-0FFFFH,���ֽ���ǰ
			USHORT	mLength;	// ���ݿ��ܳ���,���ֽ���ǰ
		};
	};
}	mCOMMAND_PACKET,	*mpCOMMAND_PACKET;

#define		CONST_CMD_LEN		0x07	// �����ĳ���
// �������������ݶ���ͨ�������´��ܵ�(USB�˵�2��OUT)�´�, Ϊ�˷�ֹ���߻���,
// ���ǿ����ڼ����Ӧ�ó����뵥Ƭ������֮��Լ��, �����ĳ�������7, �����ݿ�ĳ��ȿ϶�����7, ����64,32��
// ����, ����Լ��, ���������ֽ���������, �ȵ�
// ������Լ��������: 80H-0FFH��ͨ������,�����ڸ���Ӧ��
//                   00H-7FH��ר������,��Ը���Ӧ���ر���
// ͨ������
#define		DEF_CMD_GET_INFORM		0x90	// ��ȡ��λ����˵����Ϣ,���Ȳ�����64���ַ�,�ַ�����00H����
#define		DEF_CMD_TEST_DATA		0x91	// ��������,��λ����PC�����������������������ȡ���󷵻�
#define		DEF_CMD_CLEAR_UP		0xA0	// ���ϴ����ݿ�֮ǰ����ͬ��,ʵ��������λ������ϴ�����������������
#define		DEF_CMD_UP_DATA			0xA1	// ����λ����ָ����ַ�Ļ������ж�ȡ���ݿ�(�ϴ����ݿ�)
#define		DEF_CMD_DOWN_DATA		0xA2	// ����λ����ָ����ַ�Ļ�������д�����ݿ�(�´����ݿ�)
// ר������
#define		DEMO_CH451_CMD			0x56	// PC���������CH451,������ʾCH451�Ĺ���
// ����MCS51��Ƭ����ʹ��ͨ������ʱ,����Ҫָ��������ʶ����
#define		ACCESS_MCS51_SFR		1		// ��д51��Ƭ����SFR
#define		ACCESS_MCS51_IRAM		2		// ��д51��Ƭ�����ڲ�RAM
#define		ACCESS_MCS51_XRAM		3		// ��д51��Ƭ�����ⲿRAM

HINSTANCE   mSaveInstance;		// ʵ��
HWND        mSaveDialogMain;	// ���Ի���
HWND        mEnterDialog;	// �������ݶԻ���

UCHAR	mEnterBuffer[8];		// �������뻺����
UINT	mBufferLength = 0;		// �������ڵ���Ч���ݳ���
UINT	mShowLED = 0;			// ��Ϊ0��LED��
UINT	mKeyCode = 0xff;		// �������
UINT	mIndex = 0;				// ����ж��CH372/CH375��ָʾ���
CHAR	mCaptionInform[] = " ��Ϣ��ʾ ";
BOOL    openflag=FALSE;
BOOL    Arrivalflag=FALSE; //���豸����

LRESULT CALLBACK mDialogMain(HWND,UINT,WPARAM,LPARAM);  //���Ի�����ó���
LRESULT CALLBACK mDialogEnter(HWND,UINT,WPARAM,LPARAM); //�������ֶԻ�����ó���
VOID SendCH452Command();  //����CH452����.//mEnterBuffer�洢������λʮ�����Ƶ������ַ�
UCHAR	mCharToBcd(UCHAR);								// ��ASCII�ַ�ת��ΪһλBCD��
VOID	mSyncKeyboard(HWND,UINT);						// ͬ��������ʾ
VOID	CALLBACK mInterruptEvent(PUCHAR);						// �жϷ������
UINT	mDownloadData( UCHAR *iBuffer, UINT iLength );	// �´����ݿ�,����ʵ�ʴ��䳤��
UINT	mUploadData( UCHAR *iBuffer, UINT iLength );	// �ϴ����ݿ�,����ʵ�ʴ��䳤��
VOID CALLBACK NotifyRoutine (ULONG	iEventStatus );  // �豸�¼�֪ͨ�ص�����
VOID Testcommunication();   //����PC���뵥Ƭ��֮���USBͨѶ
VOID devremoval();        //�豸�Ƴ���������
VOID devarrival();        //�豸���봦������


//�������
int	APIENTRY	WinMain( HINSTANCE hInstance, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nShowCmd )
{
	CHAR	*CmdLine;
	CmdLine = lpCmdLine;
	mSaveInstance = hInstance;
	if ( CmdLine != NULL && *CmdLine >= '0' && *CmdLine <= '9' ) {  // ����������ָ��CH375�豸���
// �������DEMOʱ��������ָ������0��9,���ӦCH375�豸���0-9,���û�������в�����Ĭ��ΪCH375�豸���0
// ����,ִ��"DEMO 1"��Ӧ1#CH375�豸,ִ��"DEMO 8"��Ӧ8��CH375�豸
// �����д���������ͨ��Windows����ϵͳ��"��ʼ"�е�"����"����ִ��,������DOS������ִ��
		mIndex = *CmdLine - '0';
	}
	return( DialogBox( hInstance, "IDD_MAIN", NULL, mDialogMain ) );    //�������Ի���
}

//���Ի����¼�  
LRESULT CALLBACK mDialogMain( HWND hDialog, UINT uMessage, WPARAM wParam, LPARAM lParam )
{
	mCOMMAND_PACKET	mDemoReq;
	UINT		mLength;
	switch( uMessage )
	{
		case WM_INITDIALOG:        //��ʼ��
			mSaveDialogMain = hDialog;
// ��Ҫʹ��DLL����Ҫ�ȼ���
			CheckDlgButton(mSaveDialogMain,IDC_WordShift,BST_CHECKED);
			CheckDlgButton(mSaveDialogMain,IDC_SegUnLight,BST_CHECKED);

			if ( LoadLibrary( "CH375DLL.DLL" ) == NULL )  // ����DLLʧ��,����δ��װ��ϵͳ��
			{
				MessageBox( hDialog, "�޷�����CH372/CH375��DLL", mCaptionInform, MB_ICONSTOP | MB_OK );
				EndDialog( hDialog, 0x81 );  // �رնԻ���
				return( TRUE );
			}

/* Ϊ�˴Ӷ��CH372/CH375�豸���ҳ��Լ����豸,Ӧ�ó�����Բ�ѯUSB�豸��ID��(����ID�Ͳ�ƷID),ֱ��ID�Ƚ���ȷ,�����ڴ��豸֮ǰ����֮���ѯ
#define		MY_VENDOR_ID	0x4348
#define		MY_PRODUCT_ID	0x5537
			for ( mIndex = 0; mIndex < mCH375_MAX_NUMBER; mIndex ++ )
			{
				if ( CH375GetUsbID( mIndex ) == ( ( MY_PRODUCT_ID << 16 ) | MY_VENDOR_ID ) ) break;  // ��ȡUSB�豸ID,����������,��16λΪ����ID,��16λΪ��ƷID,����ʱ����ȫ0(��ЧID)
			}
			if ( mIndex >= mCH375_MAX_NUMBER )
			{
				MessageBox( hDialog, "�Ҳ���ָ����CH372/CH375�豸", mCaptionInform, MB_ICONSTOP | MB_OK );
				EndDialog( hDialog, 0x81 );  // �رնԻ���
				return( TRUE );
			}*/

            if(!CH375SetDeviceNotify(mIndex,NULL,NotifyRoutine)) //�����豸��μ���
				MessageBox( hDialog, "���ü���CH372/CH375�豸���ʧ��", mCaptionInform, MB_ICONSTOP | MB_OK );

			if ( CH375OpenDevice( mIndex ) == INVALID_HANDLE_VALUE )  // ���豸 
			{
				openflag=FALSE;
                NotifyRoutine(CH375_DEVICE_REMOVE);   //�豸û��,��ť����
				return ( TRUE );
				break;
			}
			else  //�򿪳ɹ�
				openflag=TRUE;			    

			NotifyRoutine(CH375_DEVICE_ARRIVAL);  //�豸��,��ť����
			CH375SetTimeout( mIndex, 3000, 3000 );  // ����USB���ݶ�д�ĳ�ʱ,����3000mSδ��ɶ�д��ǿ�Ʒ���,����һֱ�ȴ���ȥ
            Testcommunication(); // ����PC���뵥Ƭ��֮���USBͨѶ,������ʾ,����û������
/* �������´�/�ϴ����ݿ������
			{
				UCHAR mBuffer[4096];
				mBuffer[0]=data;	// ׼���´�������
				mLength = mDownloadData( mBuffer, 4096 ) ;	// �����ݿ�Ӽ�����´�����Ƭ��,����ʵ�ʴ��䳤��
				mLength = mUploadData( mBuffer, 4096 ) ;	// �ӵ�Ƭ���ϴ����ݿ鵽�����,����ʵ�ʴ��䳤��
			}*/
			CH375SetExclusive( mIndex, FALSE );  // ��ʱ���ù���ʹ�õ�ǰCH375�豸,��Ϊ���������жϷ������ʱ��Ҫʹ�õ�ǰ���CH375�豸
// ��������жϷ������,�жϷ����������Ӧ�ò�ִ�е�,���߳����ȼ���THREAD_PRIORITY_TIME_CRITICAL
// ����Ƭ��������Ҫ֪ͨ�����ʱ,������CMD_WR_USB_DATA5����д���ж���������,�������mInterruptEvent�߳̽����յ����ж���������
// Ȼ��mInterruptEvent�߳��������򷢳���Ϣ���д���,mInterruptEvent�߳��൱���жϷ������,����������ʱ��ѯ��Ƭ��
			CH375SetIntRoutine( mIndex, mInterruptEvent );  //�����ж�
			Sleep( 50 );  // ��������Ϣ50mS,�Եȴ���һ������CH375SetIntRoutine�����̲߳��ɸ��߳��ٴδ�CH375,������һ������CH375SetExclusive�����´������߳��޷���CH375
			CH375SetExclusive( mIndex, TRUE );  // ���ö�ռʹ�õ�ǰCH375�豸,�ڴ�֮������Ӧ�ó�����ͬʱʹ�õ�ǰ���CH375�豸,ʵ�����޷��ٴ�CH375�豸
			return ( TRUE );
			break;
		case WM_COMMAND:
			switch ( LOWORD( wParam ) )
			{				
				char i=0,j=0,k=0;
				char ledval[4]="",oldledval[4]="";

				case IDC_N1:  // �������ݵ���ʾ���ϵĵ�1�������,��ߵ�1��
				case IDC_N2:
				case IDC_N3:
				case IDC_N4:
				case IDC_N5:
				case IDC_N6:
				case IDC_N7:
				case IDC_N8:  // �������ݵ���ʾ���ϵĵ�8�������,�ұߵ�1��
                    if(Arrivalflag== 0){  //�������ϵ���ʾ��Ϣ���͵�������,��Ҫ����,��֤������ʾ��Ϣ��ͬ,					
						if(DialogBox( mSaveInstance, "IDD_ENTER", NULL, mDialogEnter )==IDC_CANCEL)
							break;
					}
					Arrivalflag=FALSE;  					
					if ( mBufferLength )  // �Ѿ���������
					{
						UCHAR	mBcdCode;
						UCHAR	mNumber;
						if ( mEnterBuffer[0] == ' ' ) mBcdCode = 0x10;  // �ո�
						else if ( mEnterBuffer[0] == '=' ) mBcdCode = 0x13;
						else if ( mEnterBuffer[0] == '.' ) mBcdCode = 0x1a;
						else mBcdCode = mCharToBcd( mEnterBuffer[0] );
						// �������һ���ַ���һ��С����,�����ַ����½�׷��С����
						if ( mBufferLength >= 2 && mEnterBuffer[1] == '.' ) mBcdCode |= 0x80;
						if ( mBcdCode != 0xff )  // ������ַ���Ч
						{   
							ZeroMemory(&mDemoReq,sizeof(mDemoReq));
							mNumber = LOWORD( wParam ) - IDC_N1;  // ���IDC_N1��IDC_N8����,����Ϊ0-7,�ֱ��Ӧ�ڸ�������ܰ�ť
							mNumber = 7 - mNumber;  // �����λΪ��,���λΪ��,����N1��Ӧλ7,N8��Ӧ0
							mDemoReq.mCommandCode = DEMO_CH451_CMD;
							mDemoReq.mCommandCodeNot = ~ DEMO_CH451_CMD;
							mDemoReq.mParameter[2] = 0x08 | mNumber;  // CH451����:���������ݵ�ָ�������
							mDemoReq.mParameter[1] = mBcdCode;  // ������,��Ӧ��BCD���뷽ʽ
							mLength = CONST_CMD_LEN;	// ������ĳ���
							if ( CH375WriteData( mIndex, &mDemoReq, &mLength ) )  // ͨ��CH375��������,�ɹ�
						// �������ʱmLength����64,��ɹ�����ʱ,Ϊ�˸��ӿɿ�,���Լ��ʵ�ʷ��͵ĳ����Ƿ����
							{
								if(lstrlen(mEnterBuffer)==1)
									mEnterBuffer[1] = 0;  // �ַ���������,ֻҪһ���ַ�
								else
									mEnterBuffer[2] = 0;
								SetDlgItemText( hDialog, LOWORD( wParam ), CharUpper(mEnterBuffer) );   //���ð�ť�ϵ��ַ�
							}
							else
								MessageBox( hDialog, "CH375WriteData ʧ��", mCaptionInform, MB_OK | MB_ICONERROR );
						}
						else 
							MessageBox( hDialog, "�������ַ�:\n0-9,A-F���߿ո�,С����", mCaptionInform, MB_OK | MB_ICONERROR );
					}
					else 
						MessageBox( hDialog, "����������һ���ַ�:\n0-9,A-F���߿ո�,С����", mCaptionInform, MB_OK | MB_ICONERROR );					
					break;				
				case IDC_INT_PRESS:  // ��������,�������ͷ�ԭ�����ٶ�ȡ�°���ֵ
					mSyncKeyboard( hDialog, 0xff );  // �ͷ��ϴεİ���
					mSyncKeyboard( hDialog, mKeyCode );  // ���ݼ�ֵ��ͬ����ʾ					
					break;
				case IDC_INT_RELEASE:  // �����ͷ�
					mSyncKeyboard( hDialog, 0xff );  // �ͷŰ���					
					break;				
				case IDC_CMD1:  // ��ʮ����������3���ַ���12λ����ʾ���ϵ�CH451оƬ
					if(DialogBox( mSaveInstance, "IDD_ENTER", NULL, mDialogEnter )==IDC_CANCEL)
						break;				
					SendCH452Command(); //����CH452����					
					break;
				case IDC_INT_DEVARRIVAL:  //�Ӳ��֪ͨ�жϳ����з������豸������Ϣ
					devarrival();					
					break;
                case IDC_INT_DEVREMOVAL:  //�Ӳ��֪ͨ�жϳ����з������豸�Ƴ���Ϣ
					devremoval();					
					break;												
				case IDC_LeftShift:   //����������/��ѭ��
					if(IsDlgButtonChecked(mSaveDialogMain,IDC_WordShift)==BST_CHECKED){ //����λ
						strcpy(mEnterBuffer,"300\0");//����λ����
						mBufferLength=lstrlen(mEnterBuffer);
						SendCH452Command(); //����CH452����
                        
						//������LED�������ʾֵ����
						for(i=0;i<7;i++){
							GetDlgItemText(mSaveDialogMain,IDC_N1+i+1,ledval,4);
							SetDlgItemText(mSaveDialogMain,IDC_N1+i,ledval);
						}
						SetDlgItemText(mSaveDialogMain,IDC_N1+7,"0");
					}
					else {  //��������ѭ��
						strcpy(mEnterBuffer,"301\0");  //��ѭ������
						mBufferLength=lstrlen(mEnterBuffer);
						SendCH452Command();  //����CH452����
                        GetDlgItemText(mSaveDialogMain,IDC_N1,oldledval,4);
                        //������LED�������ʾֵ��ѭ��
						for(i=0;i<7;i++){
							GetDlgItemText(mSaveDialogMain,IDC_N1+i+1,ledval,4);
							SetDlgItemText(mSaveDialogMain,IDC_N1+i,ledval);
						}
						SetDlgItemText(mSaveDialogMain,IDC_N1+i,oldledval);
					}
					break;
				case IDC_RightShift:  //����������/��ѭ��
					if(IsDlgButtonChecked(mSaveDialogMain,IDC_WordShift)==BST_CHECKED){ //����λ
						strcpy(mEnterBuffer,"302\0");//����λ����
						mBufferLength=lstrlen(mEnterBuffer);
						SendCH452Command(); //����CH452����
						//������LED�������ʾֵ����
						for(i=0;i<7;i++){
							GetDlgItemText(mSaveDialogMain,IDC_N8-i-1,ledval,4);
							SetDlgItemText(mSaveDialogMain,IDC_N8-i,ledval);
						}
						SetDlgItemText(mSaveDialogMain,IDC_N1,"0");
					}
					else {  //��ѭ��
						strcpy(mEnterBuffer,"303\0");//����λ�����ַ�
						mBufferLength=lstrlen(mEnterBuffer);
						SendCH452Command(); //����CH452����
						GetDlgItemText(mSaveDialogMain,IDC_N8,oldledval,4);
						for(i=0;i<7;i++){
							GetDlgItemText(mSaveDialogMain,IDC_N8-i-1,ledval,4);
							SetDlgItemText(mSaveDialogMain,IDC_N8-i,ledval);
						}
						SetDlgItemText(mSaveDialogMain,IDC_N1,oldledval);
					}
					break;
				case IDC_FlashLed1: //�������˸
				case IDC_FlashLed2:
				case IDC_FlashLed3:
				case IDC_FlashLed4:
				case IDC_FlashLed5:
				case IDC_FlashLed6:
				case IDC_FlashLed7:
				case IDC_FlashLed8:  //BST_UNCHECKED=0x0000 ,BST_CHECKED=0x0001

					mEnterBuffer[0]='6';
					j=0;k=0;
					for (i=0;i<4;i++){  //ȡ��˸����ܵ�ַ����λ,��Ӧ�ڴ�������LED1..LED8.Ϊ1,��˸;Ϊ0,ֹͣ��˸
						j=(IsDlgButtonChecked(mSaveDialogMain,IDC_FlashLed1+i)<<(3-i)) +j;						
						k=(IsDlgButtonChecked(mSaveDialogMain,IDC_FlashLed5+i)<<(3-i)) +k;
					}
					if(j>9)
						mEnterBuffer[1]=j-10+'A'; //��Ӧ��ֵ�ַ�
					else
						mEnterBuffer[1]=j+'0';
					if(k>9)
						mEnterBuffer[2]=k-10+'A';
					else
						mEnterBuffer[2]=k+'0';
					mBufferLength=lstrlen(mEnterBuffer);
					SendCH452Command(); //����CH452����
					break;	
				case IDC_SetSegLight:  //���ö�
					GetDlgItemText(mSaveDialogMain,IDC_SegAddr,&mEnterBuffer[1],3);					
					if((mCharToBcd(mEnterBuffer[1])<<4) +mCharToBcd(mEnterBuffer[2])>0x40){
						MessageBox( hDialog, "��������00H...40H��Χ�ڵ���λ��ֵ", mCaptionInform, MB_OK | MB_ICONERROR );
						break;
					}
					if(IsDlgButtonChecked(mSaveDialogMain,IDC_SegUnLight)==BST_CHECKED){ //��λ��0
						mEnterBuffer[0]='1';						
						mEnterBuffer[1]=mCharToBcd(mEnterBuffer[1])+8;
						if(mEnterBuffer[1]>9)
							mEnterBuffer[1]=mEnterBuffer[1]-10+'A';
						else
							mEnterBuffer[1]=mEnterBuffer[1]+'0';
					}
					else if(IsDlgButtonChecked(mSaveDialogMain,IDC_SegLight)==BST_CHECKED){ //��λ��0
						mEnterBuffer[0]='1';						
						mEnterBuffer[1]=mCharToBcd(mEnterBuffer[1])+12-10+'A';											
					}
					else if(IsDlgButtonChecked(mSaveDialogMain,IDC_SegUnLight)==BST_CHECKED){ //��λ��0
						mEnterBuffer[0]='1';						
					}
					mEnterBuffer[3]='\0';
					mBufferLength=lstrlen(mEnterBuffer);
					SendCH452Command(); //����CH452����
					break;
				case IDC_SetShowPara:
					GetDlgItemText(mSaveDialogMain,IDC_LimitVal,&mEnterBuffer[1],2);
					if(IsDlgButtonChecked(mSaveDialogMain,IDC_BCDCoding)==BST_CHECKED){
						mEnterBuffer[1]=mCharToBcd(mEnterBuffer[1])+8;
						if(mEnterBuffer[1]>9 && mEnterBuffer[1]<16)
							mEnterBuffer[1]=mEnterBuffer[1]-10+'A';
						else if(mEnterBuffer[1]<10)
							mEnterBuffer[1]=mEnterBuffer[1]+'0';
						else
							mEnterBuffer[1]='8';
					}
					else
						mEnterBuffer[1]=mCharToBcd(mEnterBuffer[1])+'0';
					mEnterBuffer[0]='5';
					mEnterBuffer[2]='0';
					mBufferLength=lstrlen(mEnterBuffer);
					SendCH452Command(); //����CH452����
					break;
               /*
				case IDC_SDP:      //ָ�������С��������
					j=0;k=0;                    
					for (i=0;i<8;i++){ //ȡ��λ��ַһ��ֻ��Ϩ��ָ����ַ��һ�������
						if(IsDlgButtonChecked(mSaveDialogMain,IDC_SetLED1+i)==BST_CHECKED)
							j=7-i;  //Dig��
						if(IsDlgButtonChecked(mSaveDialogMain,IDC_SA+i)==BST_CHECKED)
							k=i*8;  //Seg��
					}
					i=j+k;  //�ε�ַ
					mEnterBuffer[0]='1';
                    if(IsDlgButtonChecked(mSaveDialogMain,IDC_SegLight)==BST_CHECKED)
						mEnterBuffer[1]=(i/16 +12-10)+'A';  //����
					else{    //����
						mEnterBuffer[1]=(i/16+8);
						if(mEnterBuffer[1]>9)
							mEnterBuffer[1]=(mEnterBuffer[1]-10) +'A';    
					    else
							mEnterBuffer[1]=mEnterBuffer[1] +'0'; 
					}
					mEnterBuffer[2]=i%16;
					if(mEnterBuffer[2]>9)
						mEnterBuffer[2]=mEnterBuffer[2]-10 +'A';
					else
						mEnterBuffer[2]=mEnterBuffer[2] +'0';	
					mBufferLength=lstrlen(mEnterBuffer);
					SendCH452Command(); //����CH452����
					break;*/
				case WM_DESTROY:  // �˳�
					CH375CloseDevice( mIndex );  // �˳�����ǰ����ر�CH375�豸
					EndDialog( hDialog, 1 );
					PostQuitMessage(0);
					return( TRUE );
					break;
			}
			break; 
	}
	return ( FALSE );
}

// �������ݶԻ���
LRESULT CALLBACK mDialogEnter( HWND hDialog, UINT uMessage, WPARAM wParam, LPARAM lParam )
{
	switch( uMessage )
	{
		case WM_INITDIALOG:
			mBufferLength = 0;
			mEnterDialog=hDialog;
			return ( TRUE );
			break;
		case WM_COMMAND:
			switch( LOWORD( wParam ) )
			{
				case IDC_OK:
					mBufferLength = GetDlgItemText( hDialog, IDC_EDIT1, mEnterBuffer, 4 );  // ȡ��������ַ�
					EndDialog( hDialog, IDC_OK );
					return ( TRUE );
					break;
				case IDC_CANCEL:					
				case WM_DESTROY:
					mBufferLength = 0;					
					EndDialog( hDialog, IDC_CANCEL );			
					return ( TRUE );
					break;
			}
		break;
	}
	return ( FALSE );
}

void SendCH452Command() //����CH452����.�����������Ϊ��λʮ�����Ƶ���ֵ;
                        //mEnterBuffer�洢������λʮ�����Ƶ��ַ�
						//mCharToBcd :��һλʮ�����Ƶ��ַ�תΪ��ֵ
{
	char temB[50]="",temD[50]="";
	mCOMMAND_PACKET	mDemoReq;
	UINT		mLength;
	if ( mBufferLength >= 3 )  // �Ѿ���������,���ҳ��ȳ���3λ
	{
		UCHAR	mBcdCode[3];
		mBcdCode[0] = mCharToBcd( mEnterBuffer[0] );
		mBcdCode[1] = mCharToBcd( mEnterBuffer[1] );
		mBcdCode[2] = mCharToBcd( mEnterBuffer[2] );
		if ( mBcdCode[0] != 0xff && mBcdCode[1] != 0xff && mBcdCode[2] != 0xff )  // ����������ַ�����Ч
		{
			mDemoReq.mCommandCode = DEMO_CH451_CMD;
			mDemoReq.mCommandCodeNot = ~ DEMO_CH451_CMD;
			mDemoReq.mParameter[2] = mBcdCode[0];  // CH451����:��4λ
			mDemoReq.mParameter[1] = mBcdCode[1] << 4 | mBcdCode[2];  // ��8λ���ֽ�����
			mLength = CONST_CMD_LEN;	// ������ĳ���
            
			ltoa((mBcdCode[0]<<8)+(mBcdCode[1]<<4)+(mBcdCode[2]),temD,2);
			sprintf(temB,"�ѷ���������: %012sB (%03XH)",temD,(mBcdCode[0]<<8)+(mBcdCode[1]<<4)+(mBcdCode[2]));
			SetDlgItemText(mSaveDialogMain,IDC_cmdcode,temB);

			if (! CH375WriteData( mIndex, &mDemoReq, &mLength ) )  // ͨ��CH375��������,�ɹ�
				MessageBox( mSaveDialogMain, "CH375WriteData ʧ��", mCaptionInform, MB_OK | MB_ICONERROR );
		}
		else
			MessageBox( mSaveDialogMain, "������3���ַ�0-9,A-F", mCaptionInform, MB_OK | MB_ICONERROR );
	}
	else 
		MessageBox( mSaveDialogMain, "����������3���ַ�0-9,A-F", mCaptionInform, MB_OK | MB_ICONERROR );
}
// ��ASCII�ַ�ת��ΪһλBCD��,��Ч�ַ���0-9,A-F,a-f,��Ч�ַ�����0FFH
UCHAR	mCharToBcd(
	UCHAR	iChar )  // �����ASCII�ַ�
{
	UCHAR	mBCD;
	if ( iChar >= '0' && iChar <= '9' ) mBCD = iChar -'0';
	else if ( iChar >= 'A' && iChar <= 'F' ) mBCD = iChar - 'A' + 0x0a;
	else if ( iChar >= 'a' && iChar <= 'f' ) mBCD = iChar - 'a' + 0x0a;
	else mBCD = 0xff;
	return( mBCD );
}

// ͬ��������ʾ,��������ʱ��ʾ��,������ʾ����
void	mSyncKeyboard(
	HWND	iDialog,  // ���Ի���
	UINT	iKeyCode )  // ����İ���ֵ,00H-3FH���������,0FFH���µļ����ͷ�
{
// ���·���ֻ������IDC_K0��IDC_K63��ȫ��������
	static	UINT	mKeyNo=0;
	UCHAR	mKeyBuffer[8];

	if ( iKeyCode == 0xff )  // �ͷŸհ��µļ�
	{
		sprintf( mKeyBuffer, "%0d", mKeyNo );  // �����ַ���
		SetDlgItemText( iDialog, IDC_K0 + mKeyNo, mKeyBuffer );  // �ָ���ʾ����
	}
	else  // ��������
	{
		mKeyNo = iKeyCode & 0x3f ;  // ����0-63
		SetDlgItemText( iDialog, IDC_K0 + mKeyNo, "��" );  // 00H-3FH������������ʾ��
	}

// ��������һ��ֱ�ӷ���,����϶�
/*
	switch( iKey )
	{
		case 0x00:
			SetDlgItemText( iDialog, IDC_K0, "��" );
			break;
		case 0x01:
			SetDlgItemText( iDialog, IDC_K1, "��" );
			break;
		......
		case 0x3f:
			SetDlgItemText( iDialog, IDC_K63, "��" );
			break;
		case 0xff:
			SetDlgItemText( iDialog, ???, "??" );
			break;
	}
*/
}

// �жϷ����������CH375���������жϺ�ͨ��DLL��Ӧ�ò�ģ����õ�
VOID	CALLBACK	mInterruptEvent(  // �жϷ������
	PUCHAR			iBuffer )  // ָ��һ��������,�ṩ��ǰ���ж���������
{
// iBufferָ��һ��8���ֽڵĻ�����,�û������е��������ɵ�Ƭ����CMD_WR_USB_DATA5����д��CH375������
// CH375�����жϺ�,����һ����Ϣ֪ͨ������
	if ( iBuffer[0] == 1 )  // ����ж���������
	{
		mKeyCode = iBuffer[1] & 0x3f;  // ���صļ�ֵ,��Ƭ��������ж��������ݻ������ĵ�2�ֽ�
		PostMessage( mSaveDialogMain, WM_COMMAND, IDC_INT_PRESS, 0 );  // �ж���������1���������
	}
	else if ( iBuffer[0] == 2 ) PostMessage( mSaveDialogMain, WM_COMMAND, IDC_INT_RELEASE, 0 );  // �ж���������2������ͷ�

// ���ʹ���ж���������,����ݸ���������Ӧ�Ĵ���
/*
	switch( iBuffer[0] )
	{
		case 1: �ж���������Ϊ1
		case 2: �ж���������Ϊ2
		.....
		case 6: �ж���������Ϊ6
	}
*/
}

VOID CALLBACK NotifyRoutine (  // �豸�¼�֪ͨ�ص�����
	  ULONG	iEventStatus )  // �豸�¼��͵�ǰ״̬: 0=�豸�γ��¼�, 3=�豸�����¼�
{
	//���жϳ����ж�ϵͳ��Դ���ܲ���̫��,���Խ�����¼�����ͨ��PostMessage()�ŵ�IDC_INT_DEVARRIVAL�д���
	if(iEventStatus==CH375_DEVICE_ARRIVAL)  
		SendMessage( mSaveDialogMain, WM_COMMAND, IDC_INT_DEVARRIVAL, 0 ); 	//���豸������Ϣ���͵�������д���			
	else if(iEventStatus==CH375_DEVICE_REMOVE)	
		SendMessage( mSaveDialogMain, WM_COMMAND, IDC_INT_DEVREMOVAL, 0 ); 	//���豸�γ���Ϣ���͵�������д���		
	return;
}
VOID devarrival() //�豸���봦������
{
	char i;	
	if(!openflag)
	{
		if ( CH375OpenDevice( mIndex ) == INVALID_HANDLE_VALUE )  /* �豸������豸 */
		{
			MessageBox( mSaveDialogMain, "�޷���CH372/CH375�豸", mCaptionInform, MB_ICONSTOP | MB_OK );
			return;
		}
		else
			openflag=TRUE;

		CH375SetTimeout( mIndex, 3000, 3000 );  // ����USB���ݶ�д�ĳ�ʱ,����3000mSδ��ɶ�д��ǿ�Ʒ���,����һֱ�ȴ���ȥ
		Testcommunication();
		CH375SetExclusive( mIndex, FALSE );  // ��ʱ���ù���ʹ�õ�ǰCH375�豸,��Ϊ���������жϷ������ʱ��Ҫʹ�õ�ǰ���CH375�豸
		CH375SetIntRoutine( mIndex, mInterruptEvent );  //�����ж�
		Sleep( 50 );  // ��������Ϣ50mS,�Եȴ���һ������CH375SetIntRoutine�����̲߳��ɸ��߳��ٴδ�CH375,������һ������CH375SetExclusive�����´������߳��޷���CH375
		CH375SetExclusive( mIndex, TRUE );  // ���ö�ռʹ�õ�ǰCH375�豸,�ڴ�֮������Ӧ�ó�����ͬʱʹ�õ�ǰ���CH375�豸,ʵ�����޷��ٴ�CH375�豸
	}
    //�豸��,���밴ť����,�豸û��,���밴ť����
	for(i=0;i<8;i++){
		Arrivalflag=TRUE;  //������������Ի���
		mBufferLength = GetDlgItemText( mSaveDialogMain, IDC_N1+i, mEnterBuffer, 4 );  // ȡLED��ť����ʾ���ַ�
		//������EVT�������ʾͬ��
		SendMessage(mSaveDialogMain,WM_COMMAND,IDC_N1+i,0);	//���͵�ǰ��������ܰ�ťIDC_N1..IDC_N8����ʾֵ
		//���밴ť����
		EnableWindow(GetDlgItem(mSaveDialogMain,IDC_N1+i),TRUE);
	    EnableWindow(GetDlgItem(mSaveDialogMain,IDC_FlashLed1+i),TRUE); //��˸��ť		
	}
	SendMessage(mSaveDialogMain,WM_COMMAND,IDC_FlashLed1,0); //���͵�ǰ�����������˸����״̬

	EnableWindow(GetDlgItem(mSaveDialogMain,IDC_SetSegLight),TRUE); //���ö�λ��ʾ��ť
	EnableWindow(GetDlgItem(mSaveDialogMain,IDC_SetShowPara),TRUE); //������ʾ������ť	
	EnableWindow(GetDlgItem(mSaveDialogMain,IDC_LeftShift),TRUE);   //�����ư�ť
	EnableWindow(GetDlgItem(mSaveDialogMain,IDC_RightShift),TRUE);  //�����ư�ť
	EnableWindow(GetDlgItem(mSaveDialogMain,IDC_CMD1),TRUE);        //���������밴ť		
	SetDlgItemText(mSaveDialogMain,IDC_devstatue,"**CH372/CH375�豸�Ѳ���");  //�豸���״̬��ʾ
}
VOID devremoval() //�豸�γ���������
{
	char i;	
	if(openflag)
	{
		CH375CloseDevice( mIndex );  // �˳�����ǰ����ر�CH375�豸
		openflag=FALSE;
	}
	for(i=0;i<8;i++){
		EnableWindow(GetDlgItem(mSaveDialogMain,IDC_N1+i),FALSE); //LED 
	    EnableWindow(GetDlgItem(mSaveDialogMain,IDC_FlashLed1+i),FALSE); //��˸��ť
 	}
	EnableWindow(GetDlgItem(mSaveDialogMain,IDC_SetSegLight),FALSE);  //���ö�λ��ʾ��ť
	EnableWindow(GetDlgItem(mSaveDialogMain,IDC_SetShowPara),FALSE);  //������ʾ������ť
	EnableWindow(GetDlgItem(mSaveDialogMain,IDC_LeftShift),FALSE);    //�����ư�ť
	EnableWindow(GetDlgItem(mSaveDialogMain,IDC_RightShift),FALSE);   //�����ư�ť
	EnableWindow(GetDlgItem(mSaveDialogMain,IDC_CMD1),FALSE);         //���������밴ť
	SetDlgItemText(mSaveDialogMain,IDC_devstatue,"**CH372/CH375�豸�Ѱγ�");//�豸���״̬��ʾ	
}

VOID Testcommunication()
{
	mCOMMAND_PACKET	mDemoReq;
	UINT		mLength;

	mDemoReq.mCommandCode = DEF_CMD_TEST_DATA;  // ��������,��PC����������������ȡ���󷵻�
	mDemoReq.mCommandCodeNot = ~ DEF_CMD_TEST_DATA;
	mDemoReq.mParameter[0] = 0x5a;  // ����Ĳ�������,���غ󽫰�λȡ��
	mDemoReq.mParameter[1] = 0x96;  // ����Ĳ�������,���غ󽫰�λȡ��
	mDemoReq.mParameter[2] = 0xf3;  // ����Ĳ�������,���غ󽫰�λȡ��
	mDemoReq.mParameter[3] = 0x4c;  // ����Ĳ�������,���غ󽫰�λȡ��
	mDemoReq.mParameter[4] = 0x39;  // ����Ĳ�������,���غ󽫰�λȡ��

	mLength = CONST_CMD_LEN;	// ������ĳ���
	if ( CH375WriteData( mIndex, &mDemoReq, &mLength ) )  // ͨ��CH375������������,�ɹ�
	{
		mLength = mCH375_PACKET_LENGTH;
		if ( CH375ReadData( mIndex, &mDemoReq, &mLength ) )  // ͨ��CH375����Ӧ������,�ɹ�
		{
			if ( mLength == CONST_CMD_LEN ) 
			{
				if ( mDemoReq.mCommandCode != (UCHAR)~ DEF_CMD_TEST_DATA || mDemoReq.mParameter[0] != (UCHAR)~ 0x5a || mDemoReq.mParameter[1] != (UCHAR)~ 0x96 )
					MessageBox( mSaveDialogMain, "ͨ��USB����������д���", mCaptionInform, MB_OK | MB_ICONERROR );
			}
			else MessageBox( mSaveDialogMain, "CH375���ݲ��Է��صĳ��ȴ���", mCaptionInform, MB_OK | MB_ICONERROR );
		}
		else 
			MessageBox( mSaveDialogMain, "CH375ReadData ʧ��", mCaptionInform, MB_OK | MB_ICONERROR );
	}
	else
		MessageBox( mSaveDialogMain, "CH375WriteData ʧ��", mCaptionInform, MB_OK | MB_ICONERROR );
}

UINT	mDownloadData( UCHAR *iBuffer, UINT iLength )
{
	mCOMMAND_PACKET	mDemoReq;
	UINT		mLength;
	if ( iLength > 4096 )
	{
		MessageBox( mSaveDialogMain, "�����´����ݳ��ȳ���4096�ֽ�", mCaptionInform, MB_OK | MB_ICONERROR );
		return( FALSE );
	}
	mDemoReq.mCommandCode = DEF_CMD_DOWN_DATA;	// �����´����ݿ�
	mDemoReq.mCommandCodeNot = ~ DEF_CMD_DOWN_DATA;
	mDemoReq.mBufferID = ACCESS_MCS51_XRAM; // ��д51��Ƭ�����ⲿRAM(����ʾ��ĵ�Ƭ��û���ⲿRAM,�����޷���ʾ)
	mDemoReq.mBufferAddr = 0x8200;	// ָ���ⲿRAM��������ʼ��ַ,�����ǽ������´�����ʼ��ַΪ0X8200���ⲿRAM
	mDemoReq.mLength = iLength;  // ���������ܳ���
	mLength = CONST_CMD_LEN;	// ������ĳ���
	if ( CH375WriteData( mIndex, &mDemoReq, &mLength ) )  // ͨ��CH375���������,�ɹ�
	{
		mLength = iLength;	// ���ݿ�ĳ���,һ���´�������4096�ֽ�
		if ( ( mLength % 64 ) == CONST_CMD_LEN ) mLength ++;  // ��ֹ���ݰ��ĳ�����������ĳ�����ͬ,�����ͬ,��෢��һ����Ч����
		if ( CH375WriteData( mIndex, iBuffer, &mLength ) )  // ͨ��CH375��������,�ɹ�
			return( mLength );
		else MessageBox( mSaveDialogMain, "CH375WriteData �´�����ʧ��", mCaptionInform, MB_OK | MB_ICONERROR );
	}
	else MessageBox( mSaveDialogMain, "CH375WriteData ��������ʧ��,DEF_CMD_DOWN_DATA", mCaptionInform, MB_OK | MB_ICONERROR );
	return( 0 );
}

UINT	mUploadData( UCHAR *iBuffer, UINT iLength )
{
	mCOMMAND_PACKET	mDemoReq;
	UINT		mLength;
	if ( iLength > 4096 )
	{
		MessageBox( mSaveDialogMain, "�����ϴ����ݳ��ȳ���4096�ֽ�", mCaptionInform, MB_OK | MB_ICONERROR );
		return( FALSE );
	}
	mDemoReq.mCommandCode = DEF_CMD_CLEAR_UP;	// �����ϴ����ݿ�֮ǰ����ͬ��,ʵ�����õ�Ƭ������ϴ�����������������
	mDemoReq.mCommandCodeNot = ~ DEF_CMD_CLEAR_UP;
	mLength = CONST_CMD_LEN;	// ������ĳ���
	if ( CH375WriteData( mIndex, &mDemoReq, &mLength ) )  // ͨ��CH375���������,�ɹ�
	{
		mDemoReq.mCommandCode = DEF_CMD_UP_DATA;	// �����ϴ����ݿ�
		mDemoReq.mCommandCodeNot = ~ DEF_CMD_UP_DATA;
		mDemoReq.mBufferID = ACCESS_MCS51_XRAM; // ��д51��Ƭ�����ⲿRAM(����ʾ��ĵ�Ƭ��û���ⲿRAM,�����޷���ʾ)
		mDemoReq.mBufferAddr = 0x8200;	// ָ���ⲿRAM��������ʼ��ַ,�����ǽ�����ʼ��ַΪ0X8200���ⲿRAM�ϴ�����
		mDemoReq.mLength = iLength;  // ���������ܳ���
		mLength = CONST_CMD_LEN;	// ������ĳ���
		if ( CH375WriteData( mIndex, &mDemoReq, &mLength ) )  // ͨ��CH375���������,�ɹ�
		{
			mLength = iLength;	// ���ݿ�ĳ���,һ���ϴ�������4096�ֽ�
			if ( CH375ReadData( mIndex, iBuffer, &mLength ) )  // ͨ��CH375��������,�ɹ�
				return( mLength );
			else MessageBox( mSaveDialogMain, "CH375ReadData �ϴ�����ʧ��", mCaptionInform, MB_OK | MB_ICONERROR );
		}
		else MessageBox( mSaveDialogMain, "CH375WriteData ��������ʧ��,DEF_CMD_UP_DATA", mCaptionInform, MB_OK | MB_ICONERROR );
	}
	else MessageBox( mSaveDialogMain, "CH375WriteData ��������ʧ��,DEF_CMD_CLEAR_UP", mCaptionInform, MB_OK | MB_ICONERROR );
	return( 0 );
}