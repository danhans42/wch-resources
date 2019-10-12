unit CH375DLL;

interface
// 2003.09.08, 2003.12.28, 2004.10.15, 2004.12.05, 2004.12.10, 2005.01.20, 2005.02.23, 2005.07.15
//****************************************
//**  Copyright  (C)  W.ch  1999-2005   **
//**  Web:  http://www.winchiphead.com  **
//****************************************
//**  DLL for USB interface chip CH375  **
//**  C, VC5.0                          **
//****************************************
//
// USB���߽ӿ�оƬCH375��Ӧ�ò�ӿڿ�  V2.1
// �Ͼ��ߺ�������޹�˾  ����: W.ch 2005.07
// CH375-DLL  V2.1 , Support: Ctrl/Bulk/Int
// ���л���: Windows 98/ME, Windows 2000/XP
// support USB chip: CH372/CH375
//
uses main,SysUtils,unit1;
Const
   mCH375_PACKET_LENGTH = 64;         // CH375֧�ֵ����ݰ��ĳ���
   mCH375_MAX_NUMBER = 16;            // ���ͬʱ���ӵ�CH375��
   mMAX_BUFFER_LENGTH = $1000;        // ���ݻ�������󳤶�
   mDEFAULT_BUFFER_LEN = $400;        // ���ݻ�����Ĭ�ϳ���1024

// CH375�˵��ַ
   mCH375_ENDP_INTER_UP = $81;        // CH375���ж������ϴ��˵�ĵ�ַ
   mCH375_ENDP_INTER_DOWN = $1;       // CH375���ж������´��˵�ĵ�ַ
   mCH375_ENDP_DATA_UP = $82;         // CH375�����ݿ��ϴ��˵�ĵ�ַ
   mCH375_ENDP_DATA_DOWN = $2;        //CH375�����ݿ��´��˵�ĵ�ַ

// �豸��ӿ��ṩ�Ĺܵ���������
   mPipeDeviceCtrl = $4;              // CH375���ۺϿ��ƹܵ�;
   mPipeInterUp = $5;                 //CH375���ж������ϴ��ܵ�;
   mPipeDataUp = $6;                  //CH375�����ݿ��ϴ��ܵ�
   mPipeDataDown = $7;                //CH375�����ݿ��´��ܵ�
   mPipeAuxDown =$8;	              // CH375�ĸ��������´��ܵ�

// Ӧ�ò�ӿڵĹ��ܴ���
   mFuncNoOperation = $H0;            // �޲���
   mFuncGetVersion = $1;              // ��ȡ��������汾��
   mFuncGetConfig = $2;               // ��ȡUSB�豸����������
   mFuncSetExclusive = $b;	      // ���ö�ռʹ��
   mFuncResetDevice = $C;             // ��λUSB�豸
   mFuncResetPipe = $D;               // ��λUSB�ܵ�
   mFuncAbortPipe = $E;               // ȡ��USB�ܵ�����������
   mFuncSetTimeout = $0f;	      // ����USBͨѶ��ʱ
   mFuncBufferMode = $10;			// �趨�����ϴ�ģʽ����ѯ�������е����ݳ���

// USB�豸��׼�������
   mUSB_CLR_FEATURE = $1;
   mUSB_SET_FEATURE = $3;
   mUSB_GET_STATUS = $0 ;
   mUSB_SET_ADDRESS = $5;
   mUSB_GET_DESCR = $6;
   mUSB_SET_DESCR = $7;
   mUSB_GET_CONFIG = $8;
   mUSB_SET_CONFIG = $9;
   mUSB_GET_INTERF = $A;
   mUSB_SET_INTERF = $B;
   mUSB_SYNC_FRAME = $C;

// CH375���ƴ���ĳ���ר����������
   mCH375_VENDOR_READ = $C0;          //ͨ�����ƴ���ʵ�ֵ�CH375��Ӧ��ר�ö�����
   mCH375_VENDOR_WRITE = $40;         //ͨ�����ƴ���ʵ�ֵ�CH375��Ӧ��ר��д����

// CH375���ƴ���Ĺ�Ӧ��ר���������
   mCH375_SET_CONTROL = $51;          // ��������ź�
   mCH375_GET_STATUS = $52;           // ����״̬�ź�

// �Ĵ�����λ����
   mBitInputRxd = $2;                 // ֻ��,RXD#��������״̬,1:�ߵ�ƽ,0:�͵�ƽ
   mBitInputReq = $4;                 // ֻ��,REQ#��������״̬,1:�ߵ�ƽ,0:�͵�ƽ

// ֱ�������״̬�źŵ�λ����
   mStateRXD = $200;                  // RXD#��������״̬,1:�ߵ�ƽ,0:�͵�ƽ
   mStateREQ = $400;                  // REQ#��������״̬,1:�ߵ�ƽ,0:�͵�ƽ

type
  PVOID =Pointer;
  plong=pcardinal;
  TiIntRoutine=procedure(mbuffer:pbytearray);stdcall;

Type
        mUspValue=record
        mUspValueLow : Byte;
        mUspValueHigh : Byte;
End;
Type
        mUspIndex=record
        mUspIndexLow : Byte;
        mUspIndexHigh  : Byte;
End ;
Type
    USB_SETUP_PKT=record
    mUspReqType : Byte;
    mUspRequest : Byte;
    mUspValue : mUspValue;
    mUspIndex : mUspIndex;
    mLength : Integer;
End ;
Type
   WIN32_COMMAND=record               //����WIN32����ӿڽṹ
   mFunction : cardinal;              //����ʱָ�����ܴ�����߹ܵ���
                                      //���ʱ���ز���״̬
   mLength : cardinal;                //��ȡ����,���غ������ݵĳ���
   mBuffer:array[0..(mCH375_PACKET_LENGTH-1)] of Byte;         //���ݻ�����,����Ϊ0��255B                                           '���ݻ�����,����Ϊ0��255B
End ;

var
   mUSB_SETUP_PKT :USB_SETUP_PKT;
   mWIN32_COMMAND : WIN32_COMMAND;
   mm:procedure(mbuffer:pbytearray);stdcall;

   Function CH375OpenDevice(iIndex :cardinal):cardinal ;Stdcall; external 'CH375DLL.DLL' ;
//��CH375�豸,���ؾ��,��������Ч. ָ��CH375�豸���,0��Ӧ��һ���豸

   procedure CH375CloseDevice(iIndex :cardinal) ;Stdcall; external 'CH375DLL.DLL';
//�ر�CH375�豸,ָ��CH375�豸���

   Function CH375GetVersion():cardinal  ;Stdcall; external 'CH375DLL.DLL';
//���DLL�汾��,���ذ汾��

   Function CH375DriverCommand (           // ֱ�Ӵ����������������,�����򷵻�0,���򷵻����ݳ���
                 iIndex:cardinal;          // ָ��CH375�豸���,V1.6����DLLҲ�������豸�򿪺�ľ��
                 ioCommand:WIN32_COMMAND   // ����ṹ��ָ��
                 ):cardinal;Stdcall;external 'CH375DLL.DLL';

// �ó����ڵ��ú󷵻����ݳ���,������Ȼ��������ṹ,����Ƕ�����,�����ݷ���������ṹ��,
// ���ص����ݳ����ڲ���ʧ��ʱΪ0,�����ɹ�ʱΪ��������ṹ�ĳ���,�����һ���ֽ�,�򷵻�mWIN32_COMMAND_HEAD+1,
// ����ṹ�ڵ���ǰ,�ֱ��ṩ:�ܵ��Ż�������ܴ���,��ȡ���ݵĳ���(��ѡ),����(��ѡ)
// ����ṹ�ڵ��ú�,�ֱ𷵻�:����״̬����,�������ݵĳ���(��ѡ),
// ����״̬��������WINDOWS����Ĵ���,���Բο�NTSTATUS.H,
// �������ݵĳ�����ָ���������ص����ݳ���,���ݴ�������Ļ�������,����д����һ��Ϊ0

   Function CH375GetDrvVersion:cardinal;Stdcall; external'CH375DLL.DLL';
   //�����������汾��,���ذ汾��,�����򷵻�0

   Function CH375ResetDevice(iIndex:cardinal) : Boolean ;Stdcall; external 'CH375DLL.DLL';
   //��λUSB�豸, iIndex ָ��CH375�豸���

   Function CH375GetDeviceDescr (iIndex :cardinal;oBuffer:pvoid;ioLength:plong) : Boolean ;Stdcall; external'CH375DLL.DLL';
   //��ȡ�豸������
   //iIndexָ��CH375�豸���
   //oBufferָ��һ���㹻��Ļ�����,���ڱ���������
   //ioLengthָ�򳤶ȵ�Ԫ,����ʱΪ׼����ȡ�ĳ���,���غ�Ϊʵ�ʶ�ȡ�ĳ���

   Function CH375GetConfigDescr(iIndex :cardinal; oBuffer :pvoid; ioLength:plong) : Boolean ;Stdcall; external 'CH375DLL.DLL' ;
   //��ȡ����������
   //iIndexָ��CH375�豸���
   //oBufferָ��һ���㹻��Ļ�����,���ڱ���������
   //ioLength  ָ�򳤶ȵ�Ԫ,����ʱΪ׼����ȡ�ĳ���,���غ�Ϊʵ�ʶ�ȡ�ĳ���

   Function CH375SetIntRoutine (iIndex:cardinal;iIntRoutine :TiIntRoutine) :boolean ;Stdcall; external 'CH375DLL.DLL';
   //�趨�жϷ������
   //iIndexָ��CH375�豸���
   //iIntRoutineָ���жϷ������,ΪNULL��ȡ���жϷ���,�������ж�ʱ���øó���
   //"TiIntRoutine"���̴��ݲ���Ҫ�÷�Ĭ�ϵ�'Register'��ʽ����.

   Function CH375ReadInter(iIndex:cardinal;oBuffer:pvoid;ioLength :plong) :Boolean ; Stdcall;external 'CH375DLL.DLL';
   //��ȡ�ж�����
   //iIndexָ��CH375�豸���
   //oBufferָ��һ���㹻��Ļ�����,���ڱ���������
   //ioLengthָ�򳤶ȵ�Ԫ,����ʱΪ׼����ȡ�ĳ���,���غ�Ϊʵ�ʶ�ȡ�ĳ���

   Function CH375AbortInter (iIndex :cardinal) :Boolean ;Stdcall; external 'CH375DLL.DLL';
   //�����ж����ݶ�����
   //iIndex  ָ��CH375�豸���

   Function CH375ReadData (iIndex :cardinal;oBuffer :pvoid;ioLength :plong):Boolean ;Stdcall; external 'CH375DLL.DLL'; //��ȡ���ݿ�
   //iIndexָ��CH375�豸���,oBufferָ��һ���㹻��Ļ�����,���ڱ���������,ioLengthָ�򳤶ȵ�Ԫ,����ʱΪ׼����ȡ�ĳ���,���غ�Ϊʵ�ʶ�ȡ�ĳ���

   function CH375AbortRead(iIndex:cardinal):boolean ;Stdcall; external 'CH375DLL.DLL' ;
   //�������ݿ������
   //iIndex  ָ��CH375�豸���

   Function CH375WriteData(iIndex :cardinal;iBuffer :pvoid;ioLength :plong):longbool ;Stdcall; external 'CH375DLL.DLL';
   //��д����׼�����ݿ�(����,���Ȳ�����8�ֽ�),�ٶ�ȡ��׼�����ݿ�(Ӧ��,���Ȳ�����8�ֽ�)
   //iIndexָ��CH375�豸���,oBufferָ��һ���㹻��Ļ�����,���ڱ���������,ioLengthָ�򳤶ȵ�Ԫ,����ʱΪ׼����ȡ�ĳ���,���غ�Ϊʵ�ʶ�ȡ�ĳ���

   Function CH375AbortWrite (iIndex:cardinal):boolean ;Stdcall; external 'CH375DLL.DLL';
   //�������ݿ�д����
   //iIndex  ָ��CH375�豸���

   Function CH375WriteRead (iIndex:cardinal;iBuffer:byte;oBuffer :pvoid;ioLength:plong):Boolean ;Stdcall; external 'CH375DLL.DLL'; //д�����ݿ�
   //iIndexָ��CH375�豸���,iBufferָ��һ��������,����׼��д��������,���Ȳ�����mCH375_PACKET_LENGTH,oBufferָ��һ���㹻��Ļ�����,���ڱ���������,ioLengthָ�򳤶ȵ�Ԫ,����ʱΪ׼����ȡ�ĳ���,���غ�Ϊʵ�ʶ�ȡ�ĳ���

   Function CH375GetStatus(iIndex:cardinal;var iStatus:plong):boolean ;Stdcall; external 'CH375DLL.DLL';
    //ͨ��CH375ֱ���������ݺ�״̬
    //iIndex   ָ��CH375�豸���
    //iStatus  ָ��һ��˫�ֵ�Ԫ,���ڱ���״̬����
    //λ7-λ0��ӦCH375��D7-D0����,λ9��ӦCH375��RXD#����,λ10��ӦCH375��REQ#����

   Function CH375SetTimeout(         // ����USB���ݶ�д�ĳ�ʱ
	    iIndex:cardinal;         // ָ��CH375�豸���
	    iWriteTimeout:cardinal;  // ָ��USBд�����ݿ�ĳ�ʱʱ��,�Ժ���mSΪ��λ,0xFFFFFFFFָ������ʱ(Ĭ��ֵ)
	    iReadTimeout:cardinal    // ָ��USB��ȡ���ݿ�ĳ�ʱʱ��,�Ժ���mSΪ��λ,0xFFFFFFFFָ������ʱ(Ĭ��ֵ)
                            ):boolean;Stdcall; external 'CH375DLL.DLL';

   Function CH375WriteAuxData(     // д����������
	iIndex:cardinal;           // ָ��CH375�豸���
	iBuffer:pvoid;             // ָ��һ��������,����׼��д��������
	ioLength:plong             // ָ�򳤶ȵ�Ԫ,����ʱΪ׼��д���ĳ���,���غ�Ϊʵ��д���ĳ���
                              ):boolean;Stdcall; external 'CH375DLL.DLL';

   Function CH375SetExclusive(  // ���ö�ռʹ�õ�ǰCH375�豸
	iIndex:cardinal;        // ָ��CH375�豸���
	iExclusive:cardinal     // Ϊ0���豸���Թ���ʹ��,��0���ռʹ��
         ):boolean;Stdcall; external 'CH375DLL.DLL';

   Function CH375GetUsbID(  // ��ȡUSB�豸ID,����������,��16λΪ����ID,��16λΪ��ƷID,����ʱ����ȫ0(��ЧID)
	    iIndex:cardinal // ָ��CH375�豸���
                         ):cardinal;Stdcall; external 'CH375DLL.DLL';

   Function CH375GetDeviceName(  // ����ָ��CH375�豸���ƵĻ�����,�����򷵻�NULL
	    iIndex:cardinal      // ָ��CH375�豸���,0��Ӧ��һ���豸
                               ):pvoid;Stdcall; external 'CH375DLL.DLL';

  Function CH375SetBufUpload(       // �趨�ڲ������ϴ�ģʽ
       iIndex:cardinal;             // ָ��CH375�豸���,0��Ӧ��һ���豸
        iEnableOrClear:cardinal ):longbool;Stdcall; external 'CH375DLL.DLL';  // Ϊ0���ֹ�ڲ������ϴ�ģʽ,ʹ��ֱ���ϴ�,��0�������ڲ������ϴ�ģʽ������������е���������
// ��������ڲ������ϴ�ģʽ,��ôCH375�������򴴽��߳��Զ�����USB�ϴ����ݵ��ڲ�������,ͬʱ����������е���������,��Ӧ�ó������CH375ReadData���������ػ������е���������

  Function CH375QueryBufUpload(  // ��ѯ�ڲ��ϴ��������е��������ݰ�����,�ɹ��������ݰ�����,��������-1
        	iIndex:cardinal ):integer;Stdcall; external 'CH375DLL.DLL';  // ָ��CH375�豸���,0��Ӧ��һ���豸

  Function CH375SetBufDownload(  // �趨�ڲ������´�ģʽ
	iIndex:cardinal;  // ָ��CH375�豸���,0��Ӧ��һ���豸
	iEnableOrClear:cardinal ):longbool;Stdcall; external 'CH375DLL.DLL';  // Ϊ0���ֹ�ڲ������´�ģʽ,ʹ��ֱ���´�,��0�������ڲ������´�ģʽ������������е���������
// ��������ڲ������´�ģʽ,��ô��Ӧ�ó������CH375WriteData�󽫽����ǽ�USB�´����ݷŵ��ڲ�����������������,����CH375�������򴴽����߳��Զ�����ֱ�����


  Function CH375QueryBufDownload(  // ��ѯ�ڲ��´��������е�ʣ�����ݰ�����(��δ����),�ɹ��������ݰ�����,��������-1
        iIndex:cardinal ):integer;Stdcall; external 'CH375DLL.DLL';  // ָ��CH375�豸���,0��Ӧ��һ���豸


  Function CH375ResetInter(  // ��λ�ж����ݶ�����
	 iIndex:cardinal ):longbool;Stdcall; external 'CH375DLL.DLL';  // ָ��CH375�豸���


  Function CH375ResetAux(  // ��λ��������д����
	iIndex:cardinal ):longbool;Stdcall; external 'CH375DLL.DLL';  // ָ��CH375�豸���


  Function CH375ResetRead(  // ��λ���ݿ������
        iIndex:cardinal ):longbool;Stdcall; external 'CH375DLL.DLL';  // ָ��CH375�豸���


  Function CH375ResetWrite(  // ��λ���ݿ�д����
	iIndex:cardinal ):longbool;Stdcall; external 'CH375DLL.DLL';  // ָ��CH375�豸���

  type
	mPCH375_NOTIFY_ROUTINE=Procedure (iEventStatus:cardinal );stdcall;  // �豸�¼�֪ͨ�ص�����
       // �豸�¼��͵�ǰ״̬(�����ж���): 0=�豸�γ��¼�, 3=�豸�����¼�

  const	CH375_DEVICE_ARRIVAL=	3;		// �豸�����¼�,�Ѿ�����
	CH375_DEVICE_REMOVE_PEND=1;		// �豸��Ҫ�γ�
        CH375_DEVICE_REMOVE=0;		// �豸�γ��¼�,�Ѿ��γ�


  Function CH375SetDeviceNotify(  // �趨�豸�¼�֪ͨ����
	iIndex:cardinal;  // ָ��CH375�豸���,0��Ӧ��һ���豸
	iDeviceID:PCHAR;  // ��ѡ����,ָ���ַ���,ָ������ص��豸��ID,�ַ�����\0��ֹ
	iNotifyRoutine:mPCH375_NOTIFY_ROUTINE ):longbool;Stdcall; external 'CH375DLL.DLL';  // ָ���豸�¼��ص�����,ΪNULL��ȡ���¼�֪ͨ,�����ڼ�⵽�¼�ʱ���øó���

implementation

end.
