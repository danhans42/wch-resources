; /* 2004.06.05
; ****************************************
; **  Copyright  (C)  W.ch  1999-2007   **
; **  Web:  http://www.winchiphead.com  **
; ****************************************
; **  USB Host File Module      @CH375  **
; ****************************************
; */
; /* U���ļ���дģ��, ���ӷ�ʽ: ����+��ѯ */
; /* MCS-51��Ƭ��ASM����ʾ������, ��������V3.0A�����ϰ汾��ģ�� */
; /* ��Ϊʹ��U���ļ���дģ�������ʹ��U���ļ����ӳ����,����ռ�ý��ٵĵ�Ƭ����Դ,����ʹ��89C51��Ƭ������ */
; 
;$include	(REG51.INC)
$include	(..\CH375HM.INC)

;   ��·���ӷ�ʽ
;   ��Ƭ��    ģ��
;    P0    =  D0-D7
;    RD    =  RD#
;    WR    =  WR#
;    ?     =  CS#   ���û���ⲿRAM,��ôCS#=P26,����г���16KB���ⲿRAM,��ôCS#=P27 & ! P26 & ...,����CS#��Ƭѡ��ַΪBXXXH
;    P20   =  A0
;    INT0  =  INT#  ��Ȼ���ӵ�INT0,���Ǳ�����ֻ�ǲ�ѯģ���INT#��״̬,���Կ�����P1�ڵ���ͨI/O���Ŵ���INT0

CH375HM_INDEX	XDATA	0BCF0H			;CH375ģ��������˿ڵ�I/O��ַ
CH375HM_DATA	XDATA	0BDF1H			;CH375ģ������ݶ˿ڵ�I/O��ַ

CH375HM_INT		BIT		INT0			;�ٶ�CH375ģ���INT#�������ӵ���Ƭ����INT0����

LED_OUT			BIT		P1.4			;P1.4 �͵�ƽ����LED��ʾ,���ڼ����ʾ����Ľ���

; �ٶ��ļ����ݻ�����: ExtRAM: 0000H-7FFFH
DATA_BUF		XDATA	00000H			;�ⲿRAM���ļ����ݻ�����,�Ӹõ�Ԫ��ʼ�Ļ��������Ȳ�С��һ�ζ�д�����ݳ���,����Ϊ512�ֽ�

; ��������ṹ, ����Ϊ20��60�ֽ�
mCmdParam		DATA	20H				;Ĭ������¸ýṹ��ռ��64�ֽڵ�RAM,�����޸�MAX_PATH_LEN����,���޸�Ϊ32ʱ,ֻռ��32�ֽڵ�RAM
SECTOR_COUNT	DATA	1DH				;�ļ�ռ��������
NEW_SIZE_HIGH	DATA	1EH				;�ļ����ȵĸ��ֽ�
NEW_SIZE_LOW	DATA	1FH				;�ļ����ȵĵ��ֽ�

				ORG		0000H
				LJMP	MAIN
;
; ����������ʼ��ַ
				ORG		0100H
STR_FILE_NAME1:	DB		'\C51\CH375HFT.C',00H	;�ļ���,���ļ���C51��Ŀ¼��
STR_FILE_DATA:	DB		'Note: ',0DH,0AH
				DB      'ԭ���Ǵ��㽫/C51/CH375HFT.C�ļ��е�Сд��ĸת�ɴ�д��д���µ��ļ�,�����Ҳ�������ļ�',0DH,0AH,00H
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

; ִ������
; ����������������������,���ز���״̬��,��������ͷ��ز�������CMD_PARAM�ṹ��
; ���:   R7 ������, R5 ��������, ���ڶ�д�ļ���������, DPTR Ϊ�ⲿRAM��������ʼ��ַ
; ����:   R7 ״̬��, ���ڶ�д�ļ���������, DPTR Ϊ������ֹ��ַ
; ʹ��:   R0, R4, R5, R6, R7
ExecCommand:	MOV     R6,DPH				;���ڶ�д�ļ���������,�����ⲿRAM��������ʼ��ַ
				MOV     R4,DPL
				MOV  	DPTR,#CH375HM_INDEX
				MOV     A,#PARA_COMMAND_ADDR
				MOVX 	@DPTR,A				;������ַΪPARA_COMMAND_ADDR
				MOV  	DPTR,#CH375HM_DATA
				MOV  	A,R7
				MOVX 	@DPTR,A				;��������ַPARA_COMMAND_ADDRд��������
				MOV  	A,R5
				MOV     R7,A				;���������ĳ���
				JZ      ExecCommand_W0		;û�в���
				MOV  	DPTR,#CH375HM_INDEX
				MOV     A,#PARA_BUFFER_ADDR
				MOVX 	@DPTR,A				;ָ�򻺳���
				MOV     R0,#mCmdParam		;ָ�������������ʼ��ַ
				MOV  	DPTR,#CH375HM_DATA
ExecCommand_WP:	MOV		A,@R0
				MOVX	@DPTR,A				;��������ַPARA_BUFFER_ADDR��ʼ,д�����
				INC     R0
				DJNZ    R7,ExecCommand_WP
ExecCommand_W0:	MOV  	DPTR,#CH375HM_INDEX
				MOV     A,#PARA_CMD_LEN_ADDR
				MOVX    @DPTR,A				;������ַPARA_CMD_LEN_ADDR
				MOV  	DPTR,#CH375HM_DATA
				MOV     A,R5
				ORL     A,#PARA_CMD_BIT_ACT	;���λ֪ͨģ��,˵��������Ѿ�д��,����ʼִ������
				MOVX    @DPTR,A				;��������ַPARA_CMD_LEN_ADDRд����������ĳ���
ExecCommand_Wait:

$IF 1
				JB		CH375HM_INT,$		;�ȴ�ģ����ɲ��������͵�ƽ�ж�,�����ز���״̬
$ELSE
				MOV  	DPTR,#CH375HM_INDEX	;�������Ҫ������ʽ��д,��ô���Բ�ѯģ��������뵥Ԫ�����ѯģ��INT#����
				MOV     A,#PARA_COMMAND_ADDR
				MOVX    @DPTR,A
				MOV  	DPTR,#CH375HM_DATA
				MOVX    A,@DPTR				;ģ��������ʱ��ֵ����0,�������ڷ�������ʽ��д
				JNZ     ExecCommand_Wait	;�ȴ��ж�
$ENDIF

;��Ѽ�ⷽʽ�Ƕ�ģ���INT#�źŽ����½��ر��ؼ��
				MOV  	DPTR,#CH375HM_INDEX
				MOV  	A,#PARA_STATUS_ADDR
				MOVX 	@DPTR,A				;д��������ַPARA_STATUS_ADDR
				MOV  	DPTR,#CH375HM_DATA
				MOVX 	A,@DPTR				;��������ַPARA_STATUS_ADDR��ȡ�ж�״̬
				MOV		R7,A				;����״̬��
				MOV  	DPTR,#CH375HM_INDEX
				MOV     A,#PARA_CMD_LEN_ADDR
				MOVX    @DPTR,A				;���������ĵ�Ԫ��ַ
				MOV  	DPTR,#CH375HM_DATA
				MOV     A,#PARA_CMD_BIT_INACT
				MOVX    @DPTR,A				;�ж�Ӧ��,ȡ������ģ����ж�����
;��Ϊģ�����յ��ж�Ӧ���3uS֮�ڲų����ж�����,����,����ǲ�ѯINT#�źŵĵ͵�ƽ,��ô�ڷ����ж�Ӧ���3uS֮�ڲ�Ӧ���ٲ�ѯINT#�źŵ�״̬
;��������51��Ƭ������,����Ĵ���ʱ���Ѿ�����3uS,���Բ���������ʱ�ȴ�ģ�鳷���ж�����
				CJNE    R7,#ERR_SUCCESS,ExecCommand_J0	;���ǲ����ɹ���״̬��
				MOV  	DPTR,#CH375HM_INDEX	;�����ɹ�
				MOV		A,#PARA_STS_LEN_ADDR
				MOVX 	@DPTR,A
				MOV  	DPTR,#CH375HM_DATA
				MOVX 	A,@DPTR				;��������ַPARA_STS_LEN_ADDR��ȡ���ؽ�����ݵĳ���
				MOV  	R7,A
				JZ      ExecCommand_R0		;û�н������
				MOV  	DPTR,#CH375HM_INDEX
				MOV     A,#PARA_BUFFER_ADDR
				MOVX 	@DPTR,A				;ָ�򻺳���
				MOV     R0,#mCmdParam		;ָ�������������ʼ��ַ
				MOV  	DPTR,#CH375HM_DATA	;��������ַPARA_BUFFER_ADDR��ʼ,��ȡ���
ExecCommand_RP:	MOVX	A,@DPTR
				MOV		@R0,A
				INC		R0
				DJNZ 	R7,ExecCommand_RP
ExecCommand_R0:	LJMP	ExecCommand_Ret
ExecCommand_J0:	CJNE    R7,#USB_INT_DISK_READ,ExecCommand_J1
				MOV  	DPTR,#CH375HM_INDEX	;���ڴ�U�̶����ݿ�,�������ݶ���
				MOV		A,#PARA_BUFFER_ADDR
				MOVX 	@DPTR,A				;ָ�򻺳���
				MOV  	R7,#040H			;һ�ζ�ȡ64���ֽ�
RD_SEC_BYTE:	MOV     DPTR,#CH375HM_DATA
				MOVX	A,@DPTR				;��������64�ֽڵ�����
				MOV     DPH,R6
				MOV     DPL,R4				;��������ַ
				MOVX    @DPTR,A				;��ȡ�����ݱ��浽�ⲿ������
				INC     DPTR
				MOV     R6,DPH
				MOV     R4,DPL
				DJNZ    R7,RD_SEC_BYTE		;��������ַ0��63���ζ���64�ֽڵ�����
				MOV  	DPTR,#CH375HM_INDEX
				MOV     A,#PARA_CMD_LEN_ADDR
				MOVX    @DPTR,A				;���������ĵ�Ԫ��ַ
				MOV  	DPTR,#CH375HM_DATA
				MOV     A,#PARA_CMD_BIT_ACT
				MOVX    @DPTR,A				;֪ͨģ�����,˵��64�ֽ������Ѿ���ȡ���
				LJMP    ExecCommand_Wait
ExecCommand_J1:	CJNE    R7,#USB_INT_DISK_WRITE,ExecCommand_J2
				MOV  	DPTR,#CH375HM_INDEX	;������U��д���ݿ�,��������д��
				MOV		A,#PARA_BUFFER_ADDR
				MOVX 	@DPTR,A				;ָ�򻺳���
				MOV  	R7,#040H			;һ��д��64���ֽ�
WR_SEC_BYTE:	MOV     DPH,R6
				MOV     DPL,R4				;��������ַ
				MOVX    A,@DPTR				;д������������ⲿ������
				INC     DPTR
				MOV     R6,DPH
				MOV     R4,DPL
				MOV     DPTR,#CH375HM_DATA
				MOVX    @DPTR,A				;�������64�ֽڵ�����
				DJNZ    R7,WR_SEC_BYTE		;��������ַ0��63����д��64�ֽڵ�����
				MOV  	DPTR,#CH375HM_INDEX
				MOV     A,#PARA_CMD_LEN_ADDR
				MOVX    @DPTR,A				;���������ĵ�Ԫ��ַ
				MOV  	DPTR,#CH375HM_DATA
				MOV     A,#PARA_CMD_BIT_ACT
				MOVX    @DPTR,A				;֪ͨģ�����,˵��64�ֽ������Ѿ�д�����
				LJMP	ExecCommand_Wait
ExecCommand_J2:	CJNE    R7,#USB_INT_DISK_RETRY,ExecCommand_J3
				MOV  	DPTR,#CH375HM_INDEX	;��д���ݿ�ʧ������,Ӧ������޸Ļ�����ָ��
				MOV		A,#PARA_BUFFER_ADDR
				MOVX 	@DPTR,A				;ָ�򻺳���
				MOV		DPTR,#CH375HM_DATA
				MOVX	A,@DPTR				;���ģʽ��Ϊ�ظ�ָ���ֽ����ĸ�8λ,�����С��ģʽ��ô���յ����ǻظ�ָ���ֽ����ĵ�8λ
				MOV		R7,A
				MOVX	A,@DPTR				;���ģʽ��Ϊ�ظ�ָ���ֽ����ĵ�8λ,�����С��ģʽ��ô���յ����ǻظ�ָ���ֽ����ĸ�8λ
				CLR		C
				XCH		A,R4
				SUBB	A,R4
				MOV		R4,A				;���Ǵ��ģʽ�µĻظ�ָ��
				MOV		A,R6
				SUBB	A,R7
				MOV		R6,A
				MOV  	DPTR,#CH375HM_INDEX
				MOV     A,#PARA_CMD_LEN_ADDR
				MOVX    @DPTR,A				;���������ĵ�Ԫ��ַ
				MOV  	DPTR,#CH375HM_DATA
				MOV     A,#PARA_CMD_BIT_ACT
				MOVX    @DPTR,A				;֪ͨģ�����,˵������״̬���Ѿ��������
				LJMP	ExecCommand_Wait
ExecCommand_J3:								;R7Ϊ�������״̬,�������ִ���
				CJNE    R7,#ERR_DISK_DISCON,ExecCommand_J4	;���U�̶Ͽ��¼�
				SJMP    ExecCommandEv
ExecCommand_J4:	CJNE    R7,#ERR_USB_CONNECT,ExecCommand_J5	;���U�̲����¼�
ExecCommandEv:	LCALL	mDelay100mS			;U�̸ո����ӻ��߶Ͽ�,Ӧ����ʱ��ʮ�����ٲ���
				JB		CH375HM_INT,ExecCommand_Ret	;û���ж��򷵻�
				LJMP    ExecCommand_Wait	;�����Ȼ���ж�����˵��֮ǰ���ж���U�̲��֪ͨ�ж�,�����ٴ�����������ж�
ExecCommand_J5:	SJMP    ExecCommand_Ret
ExecCommand_Ret:
;				JNB		CH375HM_INT,$		;�ȴ�ģ����жϳ���
				MOV     DPH,R6				;���ڶ�д�ļ���������
				MOV     DPL,R4				;��DPTR�����µĻ�������ַ
				RET
; END OF ExecCommand

; ������״̬,���������ͣ��,Ӧ���滻Ϊʵ�ʵĴ�����ʩ
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
				LCALL	mDelay100mS
				LCALL	mDelay100mS
				SETB 	LED_OUT
;������·��ʼ��
;��ʼ�����
				NOP
MAIN_LOOP:									;��ѭ��
;�����ڴ����дU��ʱ�ٲ�ѯ,û�б�Ҫһֱ������ͣ�ز�ѯ,�����õ�Ƭ����������,û�¿�������ʱ�ȴ�һ���ٲ�ѯ
				MOV		R5,#00H				;û���������
				MOV		R7,#CMD_QueryStatus	;ʹ�ò�ѯ��ʽ��U���Ƿ�����
				LCALL	ExecCommand			;��ѯ��ǰģ���״̬
				LCALL	mStopIfError		;������ͣ��
				MOV		A,mCmdParam+1		;Status.mDiskStatus
				XRL		A,#DISK_CONNECT
				JZ		MAIN_CONNECT		;U���Ѿ�����
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
;û��ԭ�ļ������в�������
				MOV		DPTR,#DATA_BUF		;��Ϊû���ҵ�Դ�ļ�,�������в�������д�����ļ�
				MOV		R5,#00H
MAIN_COPY_STR:	PUSH	DPH
				PUSH    DPL
				MOV		DPTR,#STR_FILE_DATA
				MOV		A,R5
				INC		R5
				MOVC	A,@A+DPTR			;�ӳ���ռ临���ַ�����Ϊ���ļ�������
				POP     DPL
				POP     DPH
				MOVX	@DPTR,A
				INC     DPTR
				JNZ		MAIN_COPY_STR		;δ���ַ���������00H
MAIN_FILL_STR:	MOV		A,#24H				;'$'
				MOVX	@DPTR,A
				INC		DPTR
				MOV		A,DPH
				CJNE	A,#HIGH (DATA_BUF) + 2,MAIN_FILL_STR	;�����512�ֽ�(һ������)
				MOV		NEW_SIZE_HIGH,#02H
				MOV		NEW_SIZE_LOW,#00H	;���ļ��ĳ���,512�ֽ�
				MOV		SECTOR_COUNT,#01H	;���ļ��ĳ���ֻ��һ������,�����ļ���������,��Ϊ��д��������Ϊ��λ��
				LJMP	MAIN_CREATE_NEW		;�������ļ�
MAIN_OPEN_J2:	LCALL	mStopIfError		;Դ�ļ��ɹ���
;��ѯ��ǰ�ļ�����Ϣ�����㳤��
				MOV		R5,#00H
				MOV		R7,#CMD_FileQuery	;��ѯ��ǰ�ļ�����Ϣ,û���������
				LCALL	ExecCommand
				LCALL	mStopIfError
				MOV		NEW_SIZE_HIGH,#10H
				MOV		NEW_SIZE_LOW,#00H	;���ļ��ĳ��Ȳ�����4096,ʵ����ģ�鱾�������Ƴ���
				MOV		R0,#mCmdParam
				MOV		A,@R0				;Modify.mFileSize.31-24,���ֽ���ǰ
				JNZ		MAIN_QUERY_J1		;�ļ�̫��
				INC		R0
				MOV		A,@R0				;Modify.mFileSize.23-16
				JNZ		MAIN_QUERY_J1		;�ļ�̫��
				INC		R0
				MOV		A,@R0				;Modify.mFileSize.15-8
				CLR		C
				SUBB	A,#10H				;��ʾ�������ļ�����������4096
				JNC		MAIN_QUERY_J1		;�ļ����ȳ���4096
				MOV		A,@R0				;Modify.mFileSize.15-8
				MOV		NEW_SIZE_HIGH,A
				INC		R0
				MOV		A,@R0				;Modify.mFileSize.7-0
				MOV		NEW_SIZE_LOW,A		;���ļ��ĳ���
MAIN_QUERY_J1:	MOV		A,NEW_SIZE_LOW
				ADD		A,#0FFH
				MOV		A,NEW_SIZE_HIGH
				ADDC	A,#01H
				CLR		C
				RRC		A					;ÿ��������512�ֽ�,���Ը�8λ����2
				MOV		SECTOR_COUNT,A		;�����ļ���������,��Ϊ��д��������Ϊ��λ��
				JNZ		MAIN_QUERY_J3
				INC		SECTOR_COUNT		;����һ������
MAIN_QUERY_J3:
;��ȡԭ�ļ�
				MOV		A,SECTOR_COUNT
				MOV		mCmdParam+0,A		;Read.mSectorCount
				MOV		R5,#01H				;ֻ��һ���������
				MOV		R7,#CMD_FileRead	;���ļ���ȡ����
				MOV		DPTR,#DATA_BUF		;���ڶ�д�ļ����ݵĲ���,ָ���ⲿRAM������,������ݵĻ���������ʼ��ַ
				LCALL	ExecCommand			;���ļ���ȡ����,����ļ��Ƚϴ�,һ�ζ�����,������������CMD_FileRead������ȡ,�ļ�ָ���Զ�����ƶ�
				LCALL	mStopIfError
				MOV		A,#00H
				MOV		mCmdParam+0,A		;Close.mUpdateLen
				MOV		R5,#01H				;ֻ��һ���������
				MOV		R7,#CMD_FileClose	;�ر��ļ�
				LCALL	ExecCommand			;�ر��ļ�
				LCALL	mStopIfError
;��Դ�ļ��е�Сд�ַ�ת��Ϊ��д,׼��д�����ļ�
				MOV  	DPTR,#DATA_BUF
				MOV		R7,NEW_SIZE_HIGH
				MOV		R6,NEW_SIZE_LOW
MAIN_CHAR_CASE:	CJNE	R6,#00H,MAIN_CHAR_CNT1
				CJNE	R7,#00H,MAIN_CHAR_CNT2
				SJMP	MAIN_CHAR_END
MAIN_CHAR_CNT2:	DEC		R7
MAIN_CHAR_CNT1:	DEC		R6					;����
				MOVX 	A,@DPTR
				MOV		R5,A
				CLR  	C
				SUBB 	A,#61H				;'a'
				JC   	MAIN_CHAR_SKIP		;����Сд��ĸ
				MOV  	A,R5
				CLR		C
				SUBB 	A,#7BH				;'z'+1
				JNC  	MAIN_CHAR_SKIP		;����Сд��ĸ
				MOV		A,R5
				ANL		A,#0DFH				;Сдת��д��ĸ
				MOVX	@DPTR,A
MAIN_CHAR_SKIP:	INC		DPTR
				SJMP	MAIN_CHAR_CASE
MAIN_CHAR_END:	NOP							;�Ѿ���Сд��ĸת��д
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
				MOV		A,SECTOR_COUNT
				MOV		mCmdParam+0,A		;Write.mSectorCount
				MOV		R5,#01H				;ֻ��һ���������
				MOV		R7,#CMD_FileWrite	;���ļ�д������
				MOV		DPTR,#DATA_BUF		;���ڶ�д�ļ����ݵĲ���,ָ���ⲿRAM������,������ݵĻ���������ʼ��ַ
				LCALL	ExecCommand			;���ļ�д������,����ļ��Ƚϴ�,һ��д����,������������CMD_FileWrite����д��,�ļ�ָ���Զ�����ƶ�
				LCALL	mStopIfError
				MOV		A,#01H				;�Զ������ļ�����,��������512�ı���
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