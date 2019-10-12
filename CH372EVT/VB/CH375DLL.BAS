Attribute VB_Name = "CH375DLL"

' 2003.09.08, 2003.12.28, 2004.10.15, 2004.12.05, 2004.12.10, 2005.01.20, 2005.02.23, 2005.07.15
'****************************************
'**  Copyright  (C)  W.ch  1999-2005   **
'**  Web:  http:'www.winchiphead.com  **
'****************************************
'**  DLL for USB interface chip CH375  **
'**  C, VC5.0                          **
'****************************************
'
' USB���߽ӿ�оƬCH375��Ӧ�ò�ӿڿ�  V2.1
' �Ͼ��ߺ�������޹�˾  ����: W.ch 2005.07
' CH375-DLL  V2.1 , Support: Ctrl/Bulk/Int
' ���л���: Windows 98/ME, Windows 2000/XP
' support USB chip: CH372/CH375
'
'

Type mUspValue
        mUspValueLow As Byte                  ' 02H ֵ�������ֽ�
        mUspValueHigh As Byte                 ' 03H ֵ�������ֽ�
End Type
Type mUspIndex
        mUspIndexLow As Byte                  ' 04H �����������ֽ�
        mUspIndexHigh  As Byte                ' 05H �����������ֽ�
End Type
Type USB_SETUP_PKT                            ' USB���ƴ���Ľ����׶ε�����������ṹ
    mUspReqType As Byte                       ' 00H ��������
    mUspRequest As Byte                       ' 01H �������
    mUspValue As mUspValue                    ' 02H-03H ֵ����
    mUspIndex As mUspIndex                    ' 04H-05H ��������
    mLength As Integer                        ' 06H-07H ���ݽ׶ε����ݳ���
End Type

Public Const mCH375_PACKET_LENGTH = 64          ' CH375֧�ֵ����ݰ��ĳ���
Public Const mCH375_PKT_LEN_SHORT = 8           ' CH375֧�ֵĶ����ݰ��ĳ���

Type WIN32_COMMAND                              '����WIN32����ӿڽṹ
   mFunction As Long                            '����ʱָ�����ܴ�����߹ܵ���
                                                '���ʱ���ز���״̬
   mLength As Long                              '��ȡ����,���غ������ݵĳ���
   mBuffer(mCH375_PACKET_LENGTH - 1) As Byte    '���ݻ�����,����Ϊ0��255B
End Type
Public mWIN32_COMMAND As WIN32_COMMAND


' WIN32Ӧ�ò�ӿ�����
'Public Const IOCTL_CH375_COMMAND = (FILE_DEVICE_UNKNOWN * (2 ^ 16) Or FILE_ANY_ACCESS * (2 ^ 14) Or &HF37 * (2 ^ 2) Or METHOD_BUFFERED) ' ר�ýӿ�
'Public Const mWIN32_COMMAND_HEAD = mOFFSET(mWIN32_COMMAND, mBuffer)      ' WIN32����ӿڵ�ͷ����
'Public Const mCH375_MAX_NUMBER = 16                                      ' ���ͬʱ���ӵ�CH375��
'Public Const mMAX_BUFFER_LENGTH = &H1000                                 ' ���ݻ�������󳤶�4096
'Public Const mMAX_COMMAND_LENGTH = (mWIN32_COMMAND_HEAD + mMAX_BUFFER_LENGTH)        ' ������ݳ��ȼ�������ṹͷ�ĳ���
'Public Const mDEFAULT_BUFFER_LEN = &H400                                             ' ���ݻ�����Ĭ�ϳ���1024
'Public Const mDEFAULT_COMMAND_LEN = (mWIN32_COMMAND_HEAD + mDEFAULT_BUFFER_LEN)      ' Ĭ�����ݳ��ȼ�������ṹͷ�ĳ���

' CH375�˵��ַ
Public Const mCH375_ENDP_INTER_UP = &H81            ' CH375���ж������ϴ��˵�ĵ�ַ
Public Const mCH375_ENDP_AUX_DOWN = &H1             ' CH375�ĸ��������´��˵�ĵ�ַ
Public Const mCH375_ENDP_DATA_UP = &H82             ' CH375�����ݿ��ϴ��˵�ĵ�ַ
Public Const mCH375_ENDP_DATA_DOWN = &H2            ' CH375�����ݿ��´��˵�ĵ�ַ

' �豸��ӿ��ṩ�Ĺܵ���������
Public Const mPipeDeviceCtrl = &H4                  ' CH375���ۺϿ��ƹܵ�
Public Const mPipeInterUp = &H5                     ' CH375���ж������ϴ��ܵ�
Public Const mPipeDataUp = &H6                      ' CH375�����ݿ��ϴ��ܵ�
Public Const mPipeDataDown = &H7                    ' CH375�����ݿ��´��ܵ�
Public Const mPipeAuxDown = &H8                     ' CH375�ĸ��������´��ܵ�

' Ӧ�ò�ӿڵĹ��ܴ���
Public Const mFuncNoOperation = &H0                 ' �޲���
Public Const mFuncGetVersion = &H1                  ' ��ȡ��������汾��
Public Const mFuncGetConfig = &H2                   ' ��ȡUSB�豸����������
Public Const mFuncSetExclusive = &HB                ' ���ö�ռʹ��
Public Const mFuncResetDevice = &HC                 ' ��λUSB�豸
Public Const mFuncResetPipe = &HD                   ' ��λUSB�ܵ�
Public Const mFuncAbortPipe = &HE                   ' ȡ��USB�ܵ�����������
Public Const mFuncSetTimeout = &HF                  ' ����USBͨѶ��ʱ
Public Const mFuncBufferMode = &H10                 ' �趨�����ϴ�ģʽ����ѯ�������е����ݳ���

' USB�豸��׼�������
Public Const mUSB_CLR_FEATURE = &H1
Public Const mUSB_SET_FEATURE = &H3
Public Const mUSB_GET_STATUS = &H0
Public Const mUSB_SET_ADDRESS = &H5
Public Const mUSB_GET_DESCR = &H6
Public Const mUSB_SET_DESCR = &H7
Public Const mUSB_GET_CONFIG = &H8
Public Const mUSB_SET_CONFIG = &H9
Public Const mUSB_GET_INTERF = &HA
Public Const mUSB_SET_INTERF = &HB
Public Const mUSB_SYNC_FRAME = &HC

' CH375���ƴ���ĳ���ר����������
Public Const mCH375_VENDOR_READ = &HC0              ' ͨ�����ƴ���ʵ�ֵ�CH375����ר�ö�����
Public Const mCH375_VENDOR_WRITE = &H40             ' ͨ�����ƴ���ʵ�ֵ�CH375����ר��д����

' CH375���ƴ���ĳ���ר���������
Public Const mCH375_SET_CONTROL = &H51              ' ��������ź�
Public Const mCH375_GET_STATUS = &H52               ' ����״̬�ź�

' �Ĵ�����λ����
Public Const mBitInputRxd = &H2                     ' ֻ��,RXD#��������״̬,1:�ߵ�ƽ,0:�͵�ƽ
Public Const mBitInputReq = &H4                     ' ֻ��,REQ#��������״̬,1:�ߵ�ƽ,0:�͵�ƽ


' ֱ�������״̬�źŵ�λ����
Public Const mStateRXD = &H200                      ' RXD#��������״̬,1:�ߵ�ƽ,0:�͵�ƽ
Public Const mStateREQ = &H400                      ' REQ#��������״̬,1:�ߵ�ƽ,0:�͵�ƽ

Declare Function CH375OpenDevice Lib "CH375DLL.DLL" (ByVal iIndex As Long) As Long
'��CH375�豸,���ؾ��,��������Ч. ָ��CH375�豸���,0��Ӧ��һ���豸

Declare Sub CH375CloseDevice Lib "CH375DLL.DLL" (ByVal iIndex As Long)
'�ر�CH375�豸,ָ��CH375�豸���

Declare Function CH375GetVersion Lib "CH375DLL.DLL" ()
'���DLL�汾��,���ذ汾��

Declare Function CH375DriverCommand Lib "CH375DLL.DLL" (ByVal iIndex As Long, ByRef ioCommand As WIN32_COMMAND) As Long
' ֱ�Ӵ����������������,�����򷵻�0,���򷵻����ݳ���
' iIndex ָ��CH375�豸���
' WIN32_COMMAND ����ṹ��ָ��
' �ó����ڵ��ú󷵻����ݳ���,������Ȼ��������ṹ,����Ƕ�����,�����ݷ���������ṹ��,
' ���ص����ݳ����ڲ���ʧ��ʱΪ0,�����ɹ�ʱΪ��������ṹ�ĳ���,�����һ���ֽ�,�򷵻�mWIN32_COMMAND_HEAD+1,
' ����ṹ�ڵ���ǰ,�ֱ��ṩ:�ܵ��Ż�������ܴ���,��ȡ���ݵĳ���(��ѡ),����(��ѡ)
' ����ṹ�ڵ��ú�,�ֱ𷵻�:����״̬����,�������ݵĳ���(��ѡ),
'   ����״̬��������WINDOWS����Ĵ���,���Բο�NTSTATUS.H,
'   �������ݵĳ�����ָ���������ص����ݳ���,���ݴ�������Ļ�������,����д����һ��Ϊ0

Declare Function CH375GetDrvVersion Lib "CH375DLL.DLL" () As Long
'�����������汾��,���ذ汾��,�����򷵻�0


Declare Function CH375ResetDevice Lib "CH375DLL.DLL" (ByVal iIndex As Long) As Boolean
' ��λUSB�豸, iIndex ָ��CH375�豸���

Declare Function CH375GetDeviceDescr Lib "CH375DLL.DLL" (ByVal iIndex As Long, ByRef oBuffer As Any, ByRef ioLength As Long) As Boolean
' ��ȡ�豸������
' iIndexָ��CH375�豸���
'oBufferָ��һ���㹻��Ļ�����,���ڱ���������
'ioLengthָ�򳤶ȵ�Ԫ,����ʱΪ׼����ȡ�ĳ���,���غ�Ϊʵ�ʶ�ȡ�ĳ���

Declare Function CH375GetConfigDescr Lib "CH375DLL.DLL" (ByVal iIndex As Long, ByRef oBuffer As Any, ByRef ioLength As Long) As Boolean
' ��ȡ����������
'iIndexָ��CH375�豸���
'oBufferָ��һ���㹻��Ļ�����,���ڱ���������
'ioLength  ָ�򳤶ȵ�Ԫ,����ʱΪ׼����ȡ�ĳ���,���غ�Ϊʵ�ʶ�ȡ�ĳ���

'typedef     VOID    ( CALLBACK  * mPCH375_INT_ROUTINE ) (  // �жϷ���ص�����
'    PUCHAR          iBuffer );  // ָ��һ��������,�ṩ��ǰ���ж���������

Declare Function CH375SetIntRoutine Lib "CH375DLL.DLL" (ByVal iIndex As Long, ByVal iIntRoutine As Long) As Long
'�趨�жϷ������
' iIndexָ��CH375�豸���
'iIntRoutineָ���жϷ������,ΪNULL��ȡ���жϷ���,�������ж�ʱ���øó���

Declare Function CH375ReadInter Lib "CH375DLL.DLL" (ByVal iIndex As Long, ByRef oBuffer As Any, ByRef ioLength As Long) As Boolean
' ��ȡ�ж�����
' iIndexָ��CH375�豸���
'oBufferָ��һ���㹻��Ļ�����,���ڱ���������
'ioLengthָ�򳤶ȵ�Ԫ,����ʱΪ׼����ȡ�ĳ���,���غ�Ϊʵ�ʶ�ȡ�ĳ���

Declare Function CH375AbortInter Lib "CH375DLL.DLL" (ByVal iIndex As Long) As Boolean
'�����ж����ݶ�����
'iIndex  ָ��CH375�豸���

Declare Function CH375ReadData Lib "CH375DLL.DLL" (ByVal iIndex As Long, ByRef oBuffer As Any, ByRef ioLength As Long) As Boolean '��ȡ���ݿ�
'iIndexָ��CH375�豸���,oBufferָ��һ���㹻��Ļ�����,���ڱ���������,ioLengthָ�򳤶ȵ�Ԫ,����ʱΪ׼����ȡ�ĳ���,���غ�Ϊʵ�ʶ�ȡ�ĳ���

Declare Function CH375AbortRead Lib "CH375DLL.DLL" (ByVal iIndex As Long)
'�������ݿ������
'iIndex  ָ��CH375�豸���

Public Declare Function CH375Writedata Lib "CH375DLL.DLL" Alias "CH375WriteData" (ByVal iIndex As Long, ByRef iBuffer As Any, ByRef ioLength As Long) As Boolean '��д����׼�����ݿ�(����,���Ȳ�����8�ֽ�),�ٶ�ȡ��׼�����ݿ�(Ӧ��,���Ȳ�����8�ֽ�)
'iIndexָ��CH375�豸���,
'oBufferָ��һ���㹻��Ļ�����,���ڱ���������,
'ioLengthָ�򳤶ȵ�Ԫ,����ʱΪ׼����ȡ�ĳ���,���غ�Ϊʵ�ʶ�ȡ�ĳ���

Declare Function CH375AbortWrite Lib "CH375DLL.DLL" (ByVal iIndex As Long)
'�������ݿ�д����
'iIndex  ָ��CH375�豸���
    
Declare Function CH375WriteRead Lib "CH375DLL.DLL" (ByVal iIndex As Long, ByRef iBuffer As Any, ByRef oBuffer As Any, ByRef ioLength As Long) As Boolean 'д�����ݿ�
'iIndexָ��CH375�豸���,iBufferָ��һ��������,����׼��д��������,���Ȳ�����mCH375_PACKET_LENGTH,
'oBufferָ��һ���㹻��Ļ�����,���ڱ���������,
'ioLengthָ�򳤶ȵ�Ԫ,����ʱΪ׼����ȡ�ĳ���,���غ�Ϊʵ�ʶ�ȡ�ĳ���
 
Declare Function CH375GetStatus Lib "CH375DLL.DLL" (ByVal iIndex As Long, ByRef iStatus As Long)
'ͨ��CH375ֱ���������ݺ�״̬
'iIndex   ָ��CH375�豸���
'iStatus  ָ��һ��˫�ֵ�Ԫ,���ڱ���״̬����
'λ7-λ0��ӦCH375��D7-D0����,λ9��ӦCH375��RXD#����,λ10��ӦCH375��REQ#����


Declare Function CH375SetTimeout Lib "CH375DLL.DLL" (ByVal iIndex As Long, ByVal iWriteTimeout As Long, ByVal iReadTimeout As Long) As Boolean
'����USB���ݶ�д�ĳ�ʱ
'iIndex ָ��CH375�豸���
'iWriteTimeout,  ' ָ��USBд�����ݿ�ĳ�ʱʱ��,�Ժ���mSΪ��λ,0xFFFFFFFFָ������ʱ(Ĭ��ֵ)
'iReadTimeout    ' ָ��USB��ȡ���ݿ�ĳ�ʱʱ��,�Ժ���mSΪ��λ,0xFFFFFFFFָ������ʱ(Ĭ��ֵ)
 
Declare Function CH375WriteAuxData Lib "CH375DLL.DLL" (ByVal iIndex As Long, ByRef iBuffer As Any, ByRef ioLength As Long) As Boolean
'д����������
'iIndex,   ָ��CH375�豸���
'iBuffer,  ָ��һ��������,����׼��д��������
'ioLength  ָ�򳤶ȵ�Ԫ,����ʱΪ׼��д���ĳ���,���غ�Ϊʵ��д���ĳ���

Declare Function CH375SetExclusive Lib "CH375DLL.DLL" (ByVal iIndex As Long, ByVal iExclusive As Long) As Boolean
' ���ö�ռʹ�õ�ǰCH375�豸
'iIndex      ָ��CH375�豸���
'iExclusive  Ϊ0���豸���Թ���ʹ��,��0���ռʹ��

Declare Function CH375GetUsbID Lib "CH375DLL.DLL" (ByVal iIndex As Long) As Long
'��ȡUSB�豸ID,����������,��16λΪ����ID,��16λΪ��ƷID,����ʱ����ȫ0(��ЧID)
'iIndex ָ��CH375�豸���

Declare Function CH375GetDeviceName Lib "CH375DLL.DLL" (ByVal iIndex As Long) As Long
' ����ָ��CH375�豸���ƵĻ�����,�����򷵻�NULL
' ָ��CH375�豸���,0��Ӧ��һ���豸

Declare Function CH375SetBufUpload Lib "CH375DLL.DLL" (ByVal iIndex As Long, ByVal iEnableOrClear As Long) As Boolean   ' Ϊ0���ֹ�ڲ������ϴ�ģʽ,ʹ��ֱ���ϴ�,��0�������ڲ������ϴ�ģʽ������������е���������
' �趨�ڲ������ϴ�ģʽ
'iIndex,  ָ��CH375�豸���,0��Ӧ��һ���豸
'iEnableOrClear   Ϊ0���ֹ�ڲ������ϴ�ģʽ,ʹ��ֱ���ϴ�,��0�������ڲ������ϴ�ģʽ������������е���������
' ��������ڲ������ϴ�ģʽ,��ôCH375�������򴴽��߳��Զ�����USB�ϴ����ݵ��ڲ�������,ͬʱ����������е���������,��Ӧ�ó������CH375ReadData���������ػ������е���������

Declare Function CH375QueryBufUpload Lib "CH375DLL.DLL" (ByVal iIndex As Long) As Long  ' ָ��CH375�豸���,0��Ӧ��һ���豸
' ��ѯ�ڲ��ϴ��������е��������ݰ�����,�ɹ��������ݰ�����,��������-1
'iIndex  ָ��CH375�豸���,0��Ӧ��һ���豸

Declare Function CH375SetBufDownload Lib "CH375DLL.DLL" (ByVal iIndex As Long, ByVal iEnableOrClear As Long) As Boolean     ' Ϊ0���ֹ�ڲ������´�ģʽ,ʹ��ֱ���´�,��0�������ڲ������´�ģʽ������������е���������
' CH375SetBufDownload    �趨�ڲ������´�ģʽ
' iIndex                 ָ��CH375�豸���,0��Ӧ��һ���豸
' iEnableOrClear         Ϊ0���ֹ�ڲ������´�ģʽ,ʹ��ֱ���´�,��0�������ڲ������´�ģʽ������������е���������
' ��������ڲ������´�ģʽ,��ô��Ӧ�ó������CH375WriteData�󽫽����ǽ�USB�´����ݷŵ��ڲ�����������������,����CH375�������򴴽����߳��Զ�����ֱ�����
' ��������ڲ������´�ģʽ,��ô��Ӧ�ó������CH375WriteData�󽫽����ǽ�USB�´����ݷŵ��ڲ�����������������,����CH375�������򴴽����߳��Զ�����ֱ�����


Declare Function CH375QueryBufDownload Lib "CH375DLL.DLL" (ByVal iIndex As Long) As Long
'  CH375QueryBufDownload(    ��ѯ�ڲ��´��������е�ʣ�����ݰ�����(��δ����),�ɹ��������ݰ�����,��������-1
'  iIndex                     ָ��CH375�豸���,0��Ӧ��һ���豸


Declare Function CH375ResetInter Lib "CH375DLL.DLL" (ByVal iIndex As Long) As Boolean
'  CH375ResetInter    ��λ�ж����ݶ�����
'  iIndex             ָ��CH375�豸���


Declare Function CH375ResetAux Lib "CH375DLL.DLL" (ByVal iIndex As Long) As Boolean
'  CH375ResetAux      ��λ��������д����
'  iIndex             ָ��CH375�豸���

Declare Function CH375ResetRead Lib "CH375DLL.DLL" (ByVal iIndex As Long) As Boolean
'  CH375ResetRead     ��λ���ݿ������
'  iIndex             ָ��CH375�豸���


Declare Function CH375ResetWrite Lib "CH375DLL.DLL" (ByVal iIndex As Long) As Boolean
'  CH375ResetWrite    ��λ���ݿ�д����
'  iIndex             ָ��CH375�豸���

Public Const CH375_DEVICE_ARRIVAL = 3             ' �豸�����¼�,�Ѿ�����
Public Const CH375_DEVICE_REMOVE_PEND = 1         ' �豸��Ҫ�γ�
Public Const CH375_DEVICE_REMOVE = 0              ' �豸�γ��¼�,�Ѿ��γ�

'  typedef     VOID    ( CALLBACK  * mPCH375_NOTIFY_ROUTINE ) (  // �豸�¼�֪ͨ�ص�����
'                        ULONG           iEventStatus );  // �豸�¼��͵�ǰ״̬(�����ж���): 0=�豸�γ��¼�, 3=�豸�����¼�

Declare Function CH375SetDeviceNotify Lib "CH375DLL.DLL" (ByVal iIndex As Long, ByVal iDeviceID As String, ByVal iNotifyRoutine As Long) As Boolean
'   CH375SetDeviceNotify     �趨�豸�¼�֪ͨ����
'   iIndex                    ָ��CH375�豸���,0��Ӧ��һ���豸
'   iDeviceID                 ��ѡ����,ָ���ַ���,ָ������ص��豸��ID,�ַ�����\0��ֹ
'   mPCH375_NOTIFY_ROUTINE  iNotifyRoutine    ָ���豸�¼��ص�����,ΪNULL��ȡ���¼�֪ͨ,�����ڼ�⵽�¼�ʱ���øó���


