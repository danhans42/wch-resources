; /* 2004.06.05
; ****************************************
; **  Copyright  (C)  W.ch  1999-2004   **
; **  Web:  http://www.winchiphead.com  **
; ****************************************
; **  USB Host File Module      @CH375  **
; ****************************************
; */
; /* U���ļ���дģ��, ���ӷ�ʽ: 3���ƴ���+��ѯ */
; /* MCS-51��Ƭ��ASM����ʾ������ */
; /* ��Ϊʹ��U���ļ���дģ�������ʹ��U���ļ����ӳ����,����ռ�ý��ٵĵ�Ƭ����Դ,����ʹ��89C51��Ƭ������ */
; /* ���ֽ�Ϊ��λ����U���ļ���д,��Ƭ����RAMֻ��Ҫ��ʮ���ֽ�,����Ҫ�ⲿRAM */
; 
;$include	(REG51.INC)
$include	(..\CH375HM.INC)

; ��·���ӷ�ʽ,ֻ��Ҫ����3����,ʹ�ô���ͬ������������
;   ��Ƭ��    ģ��
;    TXD   =  SIN
;    RXD   =  SOUT
;             STA# ���ջ�Ӹߵ�ƽ
;             INT# �ӵػ�ӵ͵�ƽ
;    GND   =  GND

LED_OUT			BIT		P1.4			;P1.4 �͵�ƽ����LED��ʾ,���ڼ����ʾ����Ľ���

; ��������ṹ, ����Ϊ20��60�ֽ�
mCmdParam		DATA	20H				;Ĭ������¸ýṹ��ռ��64�ֽڵ�RAM,�����޸�MAX_PATH_LEN����,���޸�Ϊ32ʱ,ֻռ��32�ֽڵ�RAM
NEW_SIZE_HIGH	DATA	1EH				;�ļ����ȵĸ��ֽ�
NEW_SIZE_LOW	DATA	1FH				;�ļ����ȵĵ��ֽ�

				ORG		0000H
				LJMP	MAIN
;
; ����������ʼ��ַ
				ORG		0100H
STR_FILE_NAME1:	DB		'\C51\CH375HFT.C',00H	;�ļ���,���ļ���C51��Ŀ¼��
STR_FILE_DATA1:	DB		'Note: ',0DH,0AH,'������������ֽ�Ϊ��λ����U���ļ���д,',00H
STR_FILE_DATA2:	DB		'��Ƭ��ֻ��Ҫ�м�ʮ�ֽڵ�RAM�Ϳ�����',0DH,0AH,00
STR_FILE_NAME2:	DB		'\˫���Ұ�.TXT',00H		;���ļ���,�ڸ�Ŀ¼��

; �ӳ���ռ临���ַ������ڲ�RAM������,Դ�ַ���������00H��Ϊ������־,���Ȳ��ܳ���255
; ���:   DPTR ָ��Դ�ַ�������ʼ��ַ, R0 Ŀ�껺��������ʼ��ַ
; ����:   R7 �����ַ����ĳ���(���ַ���������00H)
; ʹ��:   DPTR, R0, R7
CopyString:		MOV		R7,#00H
CopyStringChar:	MOV		A,R7
				INC		R7
				MOVC	A,@A+DPTR
				MOV		@R0,A
				INC		R0
				JNZ		CopyStringChar		;�����ַ���������00H,��������
				RET
;
; ��ʱ100����,����ȷ
; ʹ��:   R6, R7
mDelay100mS:	MOV  	R7,#0C8H
mDelay100mS_1:	MOV  	R6,#0C8H
mDelay100mS_2:	NOP
				NOP
				NOP
				DJNZ 	R6,mDelay100mS_2
				DJNZ 	R7,mDelay100mS_1
				RET  	

; ����һ���ֽ����ݸ�CH375ģ��
; ���:   ACC ׼�����͵�����
mSendByte:		CLR     TI
				MOV     SBUF,A
				JNB     TI,$
				RET

; ��CH375ģ�����һ���ֽ�����
; ����:   ACC �Ѿ����յ�����
mRecvByte:		JNB     RI,$
				MOV     A,SBUF
				CLR     RI
				RET

; ִ������
; ����������������������,���ز���״̬��,��������ͷ��ز�������CMD_PARAM�ṹ��
; ���:   R7 ������, R5 ��������
; ����:   R7 ״̬��, R5 ���ؽ���ĳ���
; ʹ��:   R0, R5, R6, R7
ExecCommand:	MOV     A,#SER_SYNC_CODE1	;���ʹ���ͬ����֪ͨģ��,˵�������뿪ʼ����,����ʼִ������
				LCALL   mSendByte
				MOV     A,#SER_SYNC_CODE2	;����������ͬ�������STA#���½���
				LCALL   mSendByte			;������������ͬ����Ӧ����������,���������,��ô���ʱ�䲻�ܳ���20mS,����������Ч
				CLR     RI
				MOV     A,R7
				LCALL   mSendByte			;д��������
				MOV     A,R5
				LCALL   mSendByte			;д����������ĳ���
				MOV     A,R5
				JZ      ExecCommand_Wait	;û�в���
				MOV     R0,#mCmdParam		;ָ�������������ʼ��ַ
ExecCmdParam:	MOV     A,@R0
				LCALL   mSendByte			;����д�����
				INC     R0
				DJNZ    R5,ExecCmdParam
ExecCommand_Wait:							;�������ݴ���,ֱ��������ɲ��˳�
				LCALL   mRecvByte			;�ȴ�ģ����ɲ��������ز���״̬
				MOV     R7,A				;״̬��
				XRL     A,#ERR_SUCCESS
				JNZ		ExecCmdStatus1		;���ǲ����ɹ�״̬,��Ҫ��һ������
				LCALL   mRecvByte			;�����ɹ�,�򷵻ؽ�����ݵĳ���
				MOV     R5,A				;������ݵĳ���
				JZ      ExecCmdRet			;û�н������,�����ɹ�����
				MOV     R6,A
				MOV     R0,#mCmdParam		;ָ�������������ʼ��ַ
ExecCmdResult:	LCALL   mRecvByte			;���ս�����ݲ����浽�����ṹ��
				MOV     @R0,A
				INC     R0
				DJNZ    R6,ExecCmdResult
				SJMP    ExecCmdRet			;�н������,�����ɹ�����
ExecCmdStatus1:	CJNE	R7,#USB_INT_DISK_READ,ExecCmdStatus2	;���ڴ�U�̶����ݿ�,�������ݶ���
				SJMP    ExecCmdRet			;������ֻʹ�����ֽ�Ϊ��λ���ļ���д�ӳ���,������������²����յ���״̬��,��������ʧ�ܷ���
ExecCmdStatus2:	CJNE	R7,#USB_INT_DISK_WRITE,ExecCmdStatus3	;������U��д���ݿ�,��������д��
				SJMP    ExecCmdRet			;������ֻʹ�����ֽ�Ϊ��λ���ļ���д�ӳ���,������������²����յ���״̬��,��������ʧ�ܷ���
ExecCmdStatus3:	CJNE	R7,#USB_INT_DISK_RETRY,ExecCmdStatus4	;��д���ݿ�ʧ������
				SJMP    ExecCmdRet			;������ֻʹ�����ֽ�Ϊ��λ���ļ���д�ӳ���,������������²����յ���״̬��,��������ʧ�ܷ���
ExecCmdStatus4:	CJNE    R7,#ERR_USB_CONNECT,ExecCmdStatus5
				LCALL   mDelay100mS			;U�̸ո����ӻ��߶Ͽ�,Ӧ����ʱ��ʮ�����ٲ���
				MOV     R7,#ERR_USB_CONNECT
ExecCmdStatus5:	SJMP    ExecCmdRet			;����ʧ��
ExecCmdRet:		RET
; END OF ExecCommand

; ������״̬,���������ͣ��
; ����:   R7 Ϊ����״̬��
mStopIfError:	MOV		A,R7
				JNZ		mStopIfError_LED	;״̬���Ǵ���
				RET
mStopIfError_LED:							;LED��˸
				CLR		A
				MOV		C,LED_OUT
				MOV		ACC.0,C
				XRL		A,#01H
				MOV		C,ACC.0
				MOV		LED_OUT,C
				LCALL	mDelay100mS
				SJMP	mStopIfError_LED
;
; ������
MAIN:			CLR		A
				MOV		PSW,A
				MOV		IE,A
				MOV		SP,#60H
				MOV		A,#0FFH
				MOV		P0,A
				MOV		P1,A
				MOV		P2,A
				MOV		P3,A
				CLR  	LED_OUT				;������LED��һ����ʾ����
				LCALL	mDelay100mS			;��ʱ100����,CH375ģ���ϵ����Ҫ100�������ҵĸ�λʱ��
				LCALL	mDelay100mS
				SETB 	LED_OUT
;������·��ʼ��
;������CH375ģ��ͨѶ�Ĵ���
				MOV     SCON,#50H
				MOV     PCON,#80H
				MOV     TMOD,#20H
				MOV     TH1,#0E6H			;24MHz����, 4800bps
				SETB    TR1
;��ʼ�����
				NOP
MAIN_LOOP:									;��ѭ��
;�����ڴ����дU��ʱ�ٲ�ѯ,û�б�Ҫһֱ������ͣ�ز�ѯ,�����õ�Ƭ����������,û�¿�������ʱ�ȴ�һ���ٲ�ѯ
				MOV		R5,#00H				;û���������
				MOV		R7,#CMD_QueryStatus	;ʹ�ò�ѯ��ʽ��U���Ƿ�����
				LCALL	ExecCommand			;��ѯ��ǰģ���״̬
				LCALL	mStopIfError		;������ͣ��
				MOV		A,mCmdParam+1		;Status.mDiskStatus
				CLR     C
				SUBB	A,#DISK_CONNECT
				JNC		MAIN_CONNECT		;U���Ѿ�����
				LCALL	mDelay100mS			;�����õ�Ƭ����������,û�¿�������ʱ�ȴ�һ���ٲ�ѯ
				LCALL	mDelay100mS
				SJMP	MAIN_LOOP
MAIN_CONNECT:	LCALL	mDelay100mS			;U���Ѿ�����,��ʱ,��ѡ����,�е�USB�洢����Ҫ��ʮ�������ʱ
				LCALL	mDelay100mS
				CLR  	LED_OUT				;LED��˵��U������
; ���U���Ƿ�׼����,�����U�̲���Ҫ��һ��,����ĳЩU�̱���Ҫִ����һ�����ܹ���
				MOV		R3,#05H
WAIT_READY:		LCALL	mDelay100mS
				MOV     R5,#00H
				MOV     R7,#CMD_DiskReady
				LCALL   ExecCommand			;��ѯ�����Ƿ�׼����
				MOV     A,R7
				JZ		DISK_IS_READY
				DJNZ	R3,WAIT_READY		;U����δ׼����
DISK_IS_READY:
;��ȡԭ�ļ�
				MOV		DPTR,#STR_FILE_NAME1
				MOV		R0,#mCmdParam		;Open.mPathName
				LCALL	CopyString			;�����ļ���
				MOV		A,R7
				MOV		R5,A				;��������Ϊ�ļ�������
				MOV		R7,#CMD_FileOpen	;���ļ�
				LCALL	ExecCommand			;ִ�д��ļ�����
				CJNE	R7,#ERR_MISS_DIR,MAIN_OPEN_J1
				SJMP	MAIN_OPEN_MISS		;ERR_MISS_DIR˵��û���ҵ�C51��Ŀ¼
MAIN_OPEN_J1:	CJNE	R7,#ERR_MISS_FILE,MAIN_OPEN_J2
MAIN_OPEN_MISS:								;ERR_MISS_FILE˵��û���ҵ��ļ�
				LJMP	MAIN_CREATE_NEW		;�������ļ�
MAIN_OPEN_J2:	LCALL	mStopIfError		;Դ�ļ��ɹ���
;���ֽ�Ϊ��λ��ȡԭ�ļ�
READ_FILE_BYTE:	MOV		R2,#10H				;�������16�ֽ�����, ���ζ�д�ĳ��Ȳ��ܳ��� sizeof( mCmdParam.ByteWrite.mByteBuffer )
				MOV		mCmdParam+0,R2		;ByteRead.mByteCount
				MOV		R5,#01H				;ֻ��һ���������
				MOV		R7,#CMD_ByteRead	;���ļ����ֽ�Ϊ��λ��ȡ���ݿ�
				LCALL	ExecCommand			;���ļ���ȡ����,����ļ��Ƚϴ�,һ�ζ�����,������������CMD_ByteRead������ȡ,�ļ�ָ���Զ�����ƶ�
				LCALL	mStopIfError
; ��mCmdParam+0��Ԫ��ʵ�ʶ��������ݳ���,��mCmdParam+1��Ԫ��ʼ�Ƕ��������ݿ�
				MOV     A,mCmdParam+0		;ByteRead.mByteCount
				JZ      READ_FILE_END		;ʵ�ʶ��������ݳ���С����������ĳ�����˵���ļ�����
				MOV     R7,A
				MOV     R0,#mCmdParam+1		;ByteRead.mByteBuffer,��mCmdParam+1��Ԫ��ʼ�Ƕ��������ݿ�
GET_READ_BYTE:	MOV     A,@R0
;				MOV     ?,A					;�����ն���������
				INC     R0
				DJNZ    R7,GET_READ_BYTE
				MOV     A,mCmdParam+0		;ʵ�ʶ��������ݳ���
				CLR     C
				SUBB    A,R2
				JC      READ_FILE_END		;ʵ�ʶ��������ݳ���С����������ĳ�����˵���ļ�����
				SJMP    READ_FILE_BYTE		;�ļ�δ����,������������
READ_FILE_END:	MOV		A,#00H
				MOV		mCmdParam+0,A		;Close.mUpdateLen
				MOV		R5,#01H				;ֻ��һ���������
				MOV		R7,#CMD_FileClose	;�ر��ļ�
				LCALL	ExecCommand			;�ر��ļ�
				LCALL	mStopIfError
;�������ļ�
MAIN_CREATE_NEW:
				MOV		DPTR,#STR_FILE_NAME2
				MOV		R0,#mCmdParam		;Create.mPathName
				LCALL	CopyString			;�����ļ���
				MOV		A,R7
				MOV		R5,A				;��������Ϊ�ļ�������
				MOV		R7,#CMD_FileCreate
				LCALL	ExecCommand			;�½��ļ�����,����ļ��Ѿ���������ɾ�������½�
				LCALL	mStopIfError
;���ֽ�Ϊ��λд���һ�����ݿ�
				MOV		DPTR,#STR_FILE_DATA1
				MOV		R0,#mCmdParam+1		;ByteWrite.mByteBuffer
				LCALL	CopyString			;�������ݿ�
				MOV     A,R7
				MOV		mCmdParam+0,A		;ByteWrite.mByteCount
				INC     A
				MOV		R5,A				;��������ĳ���Ϊд�����ݿ�ĳ��ȼ�һ�����ȵ�Ԫ
				MOV		R7,#CMD_ByteWrite	;���ֽ�Ϊ��λ���ļ�д������
				LCALL	ExecCommand			;���ļ�д������,����ļ��Ƚϴ�,һ��д����,������������CMD_ByteWrite����д��,�ļ�ָ���Զ�����ƶ�
				LCALL	mStopIfError
;���ֽ�Ϊ��λд��ڶ������ݿ�
				MOV		DPTR,#STR_FILE_DATA2
				MOV		R0,#mCmdParam+1		;ByteWrite.mByteBuffer
				LCALL	CopyString			;�������ݿ�
				MOV     A,R7
				MOV		mCmdParam+0,A		;ByteWrite.mByteCount
				INC     A
				MOV		R5,A				;��������ĳ���Ϊд�����ݿ�ĳ��ȼ�һ�����ȵ�Ԫ
				MOV		R7,#CMD_ByteWrite	;���ֽ�Ϊ��λ���ļ�д������
				LCALL	ExecCommand			;���ļ�д������,����ļ��Ƚϴ�,һ��д����,������������CMD_ByteWrite����д��,�ļ�ָ���Զ�����ƶ�
				LCALL	mStopIfError
				MOV		A,#01H				;����ģ���Զ������ļ�����
				MOV		mCmdParam+0,A		;Close.mUpdateLen
				MOV		R5,#01H				;ֻ��һ���������
				MOV		R7,#CMD_FileClose	;�ر��ļ�
				LCALL	ExecCommand			;�ر��ļ�
				LCALL	mStopIfError
;�ȴ�U�̶Ͽ�,������ʾ,ʵ��Ӧ���в��ؿ���U���Ƿ�Ͽ�
MAIN_TAKE_OUT:	MOV		R5,#00H				;û���������
				MOV		R7,#CMD_QueryStatus	;ʹ�ò�ѯ��ʽ��U���Ƿ�Ͽ�
				LCALL	ExecCommand			;��ѯ��ǰģ���״̬
				LCALL	mStopIfError		;������ͣ��
				MOV		A,mCmdParam+1		;Status.mDiskStatus
				XRL		A,#DISK_DISCONNECT
				JZ		MAIN_DISCONNECT		;U���Ѿ��Ͽ�
				LCALL	mDelay100mS			;��ʱ�ȴ�һ���ٲ�ѯ
				LCALL	mDelay100mS
				SJMP	MAIN_TAKE_OUT
MAIN_DISCONNECT:	SETB	LED_OUT			;LED��
				LJMP	MAIN_LOOP			;�ȴ���һ��U������
; END main
;
END