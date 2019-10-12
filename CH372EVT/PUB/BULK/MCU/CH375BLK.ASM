; CH375/CH372 Bulk Data Test
; U2(AT89C51) Program
; ������������ݴ������ȷ��, ��ͨ���շ������ݿ������Դ����ٶ�,
; ������������ʱ��Ƶ�ʲ�����24MHz�ı�׼MCS-51��Ƭ��, �������24MHz��Ӧ���ʵ�������ʱ, �������24MHz������ʵ�������ʱ
; ֻҪ��Ƭ������, ����ʹ�ýϸߵ�ʱ��, ���罫ԭ12MHz����Ϊ24MHz,
; ��ΪMCS51��Ƭ�������ٶȽ���,��д�ⲿRAM��Ҫ2�������Լ����ӵ�ѭ��ָ��,��ʹ24MHzʱ��Ҳ�޷�����500KB/S
; Ϊ�˼��ٵ�Ƭ���������ٶȲ��Ե�Ӱ��,���������ٶȲ��Բ����ڵ�Ƭ���жϷ�����������
;
; Website:  http://winchiphead.com
; Email:    tech@winchiphead.com
; Author:   W.ch 2003.09
;
;
; ��Ҫ��������Ĳ���
CH375_CMD_PORT		EQU    0BDF1H         ;CH375����ڵĵ�ַ,��ַ������Զ�Ƭѡ
CH375_DAT_PORT		EQU    0BCF0H         ;CH375����ڵĵ�ַ,��ַ������Զ�Ƭѡ
;
THIS_CMD_CODE		DATA   3EH            ;���浱ǰ������
RECV_LEN			DATA   3FH            ;�ս��յ������ݵĳ���
RECV_BUFFER			DATA   40H            ;���ݻ�����,���ڱ�����յ����´�����,����Ϊ0��64�ֽ�
; ����MCS-51��Ƭ����ȡ�ⲿRAM�Ķ�д�ٶȵ����ڲ�RAM, ������Ҫ�õ�DPTR, ���Զ�д�ٶȽ���, �������Բο��������޸�
;
; ����λ��־
FLAG_RECV_OK		BIT    2FH.0          ;���ճɹ���־,1ָʾ�ɹ����յ����ݿ�
FLAG_SEND_WAIT		BIT    2FH.1          ;���͵ȴ���־,1ָʾ�����ݿ�����CH375�еȴ�����
;
; CH375��������붨��
$INCLUDE			(..\..\MCU_IF1\MCS51ASM\CH375INC.ASM)
;
;****************************************************************************
;
; Ӧ�ò㶨��
;TEST_OTHER		EQU    00H				;�����Զ����������
TEST_START		EQU    20H				;���Թ��̿�ʼ
TEST_DATA		EQU    21H				;����������ȷ��
TEST_UPLOAD		EQU    22H				;�����ϴ����ݿ�
TEST_DOWNLOAD	EQU    23H				;�����´����ݿ�
;
STACK           EQU    0FH             ;��ջ��ջ��(��������)
;
; �й�CH451�Ķ���
CH451_DCLK		BIT   P1.7             ;��������ʱ��,�����ؼ���
CH451_DIN		BIT   P1.6             ;�����������,��CH451����������
CH451_LOAD		BIT   P1.5             ;�����������,�����ؼ���
CH451_DOUT		BIT   P3.3             ;INT1,�����жϺͼ�ֵ��������,��CH451���������
;
;****************************************************************************
;������,���������ʾCH375
				ORG   0000H            ;��λ��Ƭ�����
				LJMP  START
				ORG   0003H            ;CH375�ж�
				LJMP  CH375_INTER
;
START:
; ���³�ʼ��ָ��Ǳ�Ҫ��,����Ƭ���ָ�ΪĬ��״̬
				CLR   EA               ;���ж�
				MOV   SP,#STACK        ;�ö�ջ��ֵ
				MOV   A,#0FFH
				MOV   P0,A
				MOV   P1,A             ;��˿�״̬
				MOV   P2,A
				MOV   P3,A
				CLR   A
				MOV   IE,A             ;���ж��������ƼĴ���
				MOV   IP,A             ;���ж����ȼ����ƼĴ���
				MOV   PSW,A            ;�����״̬��
				MOV   TCON,A           ;�嶨ʱ��/���������ƼĴ���
				MOV   TMOD,A           ;�嶨ʱ��/������������ʽ�Ĵ���
				MOV   PCON,A           ;���Դ���ƼĴ���
				MOV   THIS_CMD_CODE,#00H ;���������
				MOV   R7,#0FFH
				CALL  DELAY_MS			;��ʱ�ȴ�CH375��ʼ�����
; ��ʼ��
				CALL  CH375_INIT       ;��ʼ��
				CALL  CH451_INIT       ;��ʼ��
;				.....
;
; ����CH451����ʾ����
				MOV   B,#04H           ;����ϵͳ��������
				MOV   A,#01H           ;ʹ����ʾ����
				CALL  CH451_WRITE      ;��������
				MOV   B,#05H           ;������ʾ����
				MOV   A,#88H           ;BCD���뷽ʽ,8������
				CALL  CH451_WRITE      ;��������
				MOV   R6,#08H
				MOV   R5,#08H
				MOV   R4,#01H
SHOW_INIT:		MOV   B,R5
				MOV   A,R4
				INC   R5
				INC   R4
				CALL  CH451_WRITE      ;��ʾ�ַ�
				DJNZ  R6,SHOW_INIT
;
				SETB  EA               ;�����ж�
				SETB  TR0				;�ɶ�ʱ��0���ɼ���ֵ���������
;
; ����ָ�ʼ����ѭ��,�ȴ�PC��������в���
WAIT_REQUEST:	NOP
				JNB   FLAG_RECV_OK,WAIT_REQUEST	;�ȴ��´�����
				CLR   FLAG_RECV_OK
				MOV   A,RECV_LEN			;�����´����ݲ�׼��Ӧ��
				JZ    WAIT_REQUEST			;����Ϊ0,�ڱ�������û������
;				MOV   R0,#RECV_BUFFER		;�ս��յ������ݿ�
;				MOV   R7,RECV_LEN			;�ս��յ������ݳ���
;				MOV   A,@R0					;�����ڼ�����͵�Ƭ����Ӧ�ó���֮��Լ���´����ݵ����ֽ���������
;				CJNE  A,#TEST_OTHER,WAIT_REQUEST	;�Զ����������,�������������Ѿ����жϷ�������д�������
;				NOP							;��������,������׼��Ӧ��
;				MOV   R0,#SEND_BUFFER		;ָ��׼���ϴ������ݵĻ�����
;				MOV   R7,SEND_LEN			;׼���ϴ������ݵĳ���
;				CALL  CH375_UPLOAD			;�ϴ����ݿ��ӳ���(ͨ�������˵��ϴ�)
				MOV   B,#03H           ;��λ����
				MOV   A,#00H           ;����һλ
				CALL  CH451_WRITE      ;��������ʹCH451��ʾ����һλ
				SJMP  WAIT_REQUEST			;�ȴ���һ������
;
; �Ժ���Ϊ��λ��ʱ
; ENTRY: R7 ��ʱ������
; USE:   ACC, R7
DELAY_MS:		MOV   A,#0FAH				;250*(1*6+2)*0.5=1000uS
DELAY_1MS:		NOP
				NOP
				NOP
				NOP
				NOP
				NOP
				DJNZ  ACC,DELAY_1MS			;ÿ������ʱ1����
				DJNZ  R7,DELAY_MS
				RET
;
; ��ʼ���ӳ���
CH451_INIT:		CLR   CH451_DIN        ;�ȵͺ��,���������֪ͨCH451ѡ��4�ߴ��нӿ�
				SETB  CH451_DCLK       ;��ΪĬ�ϵĸߵ�ƽ
				SETB  CH451_DIN
				SETB  CH451_LOAD
				SETB  CH451_DOUT       ;��Ϊ����
				CLR   EX1              ;��ֹ�ж�
				RET
;
;
; ��������ӳ���
; ENTRY: B-ACC ��д��12λ����, ��8λ��ACC��, ��4λ��B�ĵ�4λ��
; USE:   ACC, B, R7
CH451_WRITE:	CLR   CH451_LOAD       ;���ʼ,��������Է��ں���
				MOV   R7,#08H          ;��ACC��8λ�ͳ�
CH451_WRITE_8:	RRC   A                ;��λ��ǰ,��λ�ں�
				CLR   CH451_DCLK
				MOV   CH451_DIN,C      ;�ͳ�һλ����
				SETB  CH451_DCLK       ;����ʱ��������֪ͨCH451����λ����
				DJNZ  R7,CH451_WRITE_8 ;λ����δ�����
				MOV   A,B
				MOV   R7,#04H          ;��B��4λ�ͳ�
CH451_WRITE_4:	RRC   A                ;��λ��ǰ,��λ�ں�
				CLR   CH451_DCLK
				MOV   CH451_DIN,C      ;�ͳ�һλ����
				SETB  CH451_DCLK       ;����ʱ��������֪ͨCH451����λ����
				DJNZ  R7,CH451_WRITE_4 ;λ����δ�����
				SETB  CH451_LOAD       ;��������������֪ͨCH451������������
				RET
;
;
;****************************************************************************
;
; ��ʼ���ӳ���
; USE:   ACC, R7, DPTR
CH375_INIT:		CLR   FLAG_RECV_OK			;����ճɹ���־,1ָʾ�ɹ����յ����ݿ�
				CLR   FLAG_SEND_WAIT		;�巢�͵ȴ���־,1ָʾ�����ݿ�����CH375�еȴ�����
; ����CH375�Ƿ���������,��ѡ����
;				MOV   DPTR,#CH375_CMD_PORT	;����ڵ�ַ
;				MOV   A,#CMD_CHECK_EXIST
;				MOVX  @DPTR,A				;����CH375�Ƿ���������,��ѡ����
;				NOP							;���ʱ��Ƶ�ʵ���16MHz�������ָ����ʱ,����30MHzҪ���2��ָ��
;				MOV   DPTR,#CH375_DAT_PORT	;���ݿڵ�ַ
;				MOV   A,#55H
;				MOVX  @DPTR,A				;д���������
;				CPL   A
;				MOV   R7,A					;ȡ������
;				MOVX  A,@DPTR				;��������Ӧ���ǲ�������ȡ��
;				XRL   A,R7
;				JZ    CH375_INIT_OK			;����ͨ��
;				MOV   DPTR,#CH375_CMD_PORT	;����ڵ�ַ
;				MOV   A,#CMD_RESET_ALL
;				MOV   R7,#50H				;����ظ�������
;CH375_INIT_HR:	MOVX  @DPTR,A				;ִ��Ӳ����λ
;				DJNZ  R7,CH375_INIT_HR
;				MOV   R7,#0A0H				;160*256uS=40mS
;CH375_INIT_D2:	CLR   A
;CH375_INIT_D1:	DJNZ  ACC,CH375_INIT_D1		;��ʱ1uS
;				DJNZ  R7,CH375_INIT_D2		;��ʱ256uS
;				SJMP  CH375_INIT			;�ٴβ���
;CH375_INIT_OK:
; �����ⲿ�Զ����USB�豸VID��PID,��ѡ����,��ִ�и�������ʹ��Ĭ�ϵ�VID��PID
;				MOV   DPTR,#CH375_CMD_PORT	;����ڵ�ַ
;				MOV   A,#CMD_SET_USB_ID
;				MOVX  @DPTR,A				;�����ⲿ�Զ����USB�豸VID��PID,��ѡ����
;				NOP							;���ʱ��Ƶ�ʵ���16MHz�������ָ����ʱ
;				MOV   DPTR,#CH375_DAT_PORT	;���ݿڵ�ַ
;				MOV   A,#USB_VENDOR_ID_L
;				MOVX  @DPTR,A				;д�볧��ID�ĵ��ֽ�
;				MOV   A,#USB_VENDOR_ID_H
;				MOVX  @DPTR,A				;д�볧��ID�ĸ��ֽ�
;				MOV   A,#USB_DEVICE_ID_L
;				MOVX  @DPTR,A				;д���豸ID�ĵ��ֽ�
;				MOV   A,#USB_DEVICE_ID_H
;				MOVX  @DPTR,A				;д���豸ID�ĸ��ֽ�
;				NOP
; ����USB����ģʽ
				MOV   DPTR,#CH375_CMD_PORT	;����ڵ�ַ
				MOV   A,#CMD_SET_USB_MODE
				MOVX  @DPTR,A				;����USB����ģʽ
				NOP							;���ʱ��Ƶ�ʵ���16MHz�������ָ����ʱ
				MOV   DPTR,#CH375_DAT_PORT	;���ݿڵ�ַ
				MOV   A,#02H
				MOVX  @DPTR,A				;����Ϊʹ�����ù̼���USB�豸��ʽ
				NOP							;���ʱ��Ƶ�ʵ���16MHz�������ָ����ʱ
CH375_INIT_WT:	MOVX  A,@DPTR				;���ز���״̬
				XRL   A,#CMD_RET_SUCCESS
				JNZ   CH375_INIT_WT			;�ȴ������ɹ�,ͨ����Ҫ�ȴ�10uS-20uS
; ��������ָ�����������ж�
				CLR   IT0					;���ⲿ�ź�Ϊ�͵�ƽ����
				SETB  PX0					;�ø����ȼ�
				CLR   IE0					;���жϱ�־
				SETB  EX0					;����CH375�ж�
				RET
;
; �ϴ����ݿ��ӳ���(ͨ�������˵��ϴ�)
; ENTRY: R0 ָ������׼���ϴ����ݵĻ�����, R7 ׼���ϴ������ݳ���, ��Ч��ֵ��0��64
; USE:   ACC, R0, R7, DPTR
CH375_UPLOAD:	JB    FLAG_SEND_WAIT,CH375_UPLOAD	;������ڵȴ�����(ǰ�η�����δ���)������ȴ�
				CLR   EX0					;Ϊ�˷�ֹ��;���ж϶�����˳��,�����Ƚ�ֹ�ж�
				MOV   DPTR,#CH375_CMD_PORT	;����ڵ�ַ
				MOV   A,#CMD_WR_USB_DATA7
				MOVX  @DPTR,A				;��USB�˵�2�ķ��ͻ�����д�����ݿ�
				NOP							;���ʱ��Ƶ�ʵ���16MHz�������ָ����ʱ
				MOV   DPTR,#CH375_DAT_PORT	;���ݿڵ�ַ
				MOV   A,R7
				MOVX  @DPTR,A				;����д��������ݳ���
CH375_UPLOAD_1:	MOV   A,@R0
				INC   R0
				MOVX  @DPTR,A				;��������
				DJNZ  R7,CH375_UPLOAD_1		;������������ֱ������
				SETB  FLAG_SEND_WAIT		;�õȴ����ͱ�־
				SETB  EX0					;�����ж�
				RET
;
; �ϴ��ж������ӳ���(ͨ���ж϶˵��ϴ�),��ѡ�ӳ���
; ENTRY: R0 ָ������׼���ϴ����ݵĻ�����, R7 ׼���ϴ������ݳ���, ��Ч��ֵ��0��8
; USE:   ACC, R0, R7, DPTR
CH375_UP_INT:	CLR   EX0					;Ϊ�˷�ֹ��;���ж϶�����˳��,�����Ƚ�ֹ�ж�
				MOV   DPTR,#CH375_CMD_PORT	;����ڵ�ַ
				MOV   A,#CMD_WR_USB_DATA5
				MOVX  @DPTR,A				;��USB�˵�1�ķ��ͻ�����д�����ݿ�
				NOP							;���ʱ��Ƶ�ʵ���16MHz�������ָ����ʱ
				MOV   DPTR,#CH375_DAT_PORT	;���ݿڵ�ַ
				MOV   A,R7
				MOVX  @DPTR,A				;����д��������ݳ���
CH375_UP_INT_1:	MOV   A,@R0
				INC   R0
				MOVX  @DPTR,A				;�����ж�����
				DJNZ  R7,CH375_UP_INT_1		;������������ֱ������
				SETB  EX0					;�����ж�
				RET
;
; �жϷ����ӳ���
; USE:   ��ջ8�ֽ�
CH375_INTER:	PUSH  PSW					;�ֳ�����
				PUSH  ACC
				PUSH  DPL
				PUSH  DPH
;				PUSH  01H					;R1
;				PUSH  02H					;R2
;��������δʹ��R1/R2,���Բ�����ջ����,�Ӷ�����ٶ�
				MOV   DPTR,#CH375_CMD_PORT	;����ڵ�ַ
				MOV   A,#CMD_GET_STATUS
				MOVX  @DPTR,A				;��ȡ�ж�״̬��ȡ���ж�����
				NOP							;���ʱ��Ƶ�ʵ���16MHz�������ָ����ʱ,����30MHzҪ���2��ָ��
				NOP
; ��Ƭ����дCH375д���������,Ӧ��Ҫ��2uS���ܶ�ȡ����,����ʱ��Ƶ�ʸ���16MHzʱ��Ҫָ����ʱ
				MOV   DPTR,#CH375_DAT_PORT	;���ݿڵ�ַ
				MOVX  A,@DPTR				;���ز���״̬
				CLR   IE0					;���жϱ�־,��Ӧ��INT0�ж�
				CJNE  A,#USB_INT_EP2_OUT,CH375_INT_1	;�����˵��´��ɹ�
				LJMP  CH375_DOWN_OK			; USB�������ݽ��ճɹ�
CH375_INT_1:	CJNE  A,#USB_INT_EP2_IN,CH375_INT_2	;�����˵��ϴ��ɹ�
				LJMP  CH375_UP_OK			; USB�������ݷ��ͳɹ�
CH375_INT_2:	CJNE  A,#USB_INT_EP1_IN,CH375_INT_3	;�ж϶˵�
				LJMP  CH375_UP_INT_OK		; USB�ж����ݷ��ͳɹ�
CH375_INT_3:	SJMP  CH375_INT_RET
CH375_INT_RET:
;				POP   02H					;R2
;				POP   01H					;R1
				POP   DPH
				POP   DPL
				POP   ACC
				POP   PSW					;�ָ��Ĵ���
				RETI						;�жϷ���
;
CH375_DOWN_OK:	; USB�������ݽ��ճɹ�
				MOV   DPTR,#CH375_CMD_PORT	;����ڵ�ַ
				MOV   A,#CMD_RD_USB_DATA
				MOVX  @DPTR,A				;�ӵ�ǰUSB�жϵĶ˵㻺������ȡ���ݿ�,���ͷŻ�����
				NOP							;���ʱ��Ƶ�ʵ���16MHz�������ָ����ʱ,����30MHzҪ���һ��ָ��
				NOP
; ��Ƭ����дCH375д���������,Ӧ��Ҫ��2uS���ܶ�ȡ����,����ʱ��Ƶ�ʸ���16MHzʱ��Ҫָ����ʱ
				MOV   DPTR,#CH375_DAT_PORT	;���ݿڵ�ַ
				MOVX  A,@DPTR				;���ȶ�ȡ�������ݳ���
				MOV   R2,A
				JZ    CH375_INT_RET			;����Ϊ0,û��������ֱ���˳�
				MOVX  A,@DPTR				;��������
				MOV   THIS_CMD_CODE,A		;���浱ǰ������
				CJNE  A,#TEST_DOWNLOAD,IS_USB_CMD0
				SJMP  USB_CMD2_NEXT			;�����´��ٶ�
USB_CMD2_RECV:	MOVX  A,@DPTR				;��������,Ϊ�˲����ٶ�,��������,24MHz��MCS51ÿ��ȡһ���ֽ���Ҫ2uS
USB_CMD2_NEXT:	DJNZ  R2,USB_CMD2_RECV		;������������ֱ������
				SJMP  CH375_INT_RET
IS_USB_CMD0:	MOV   R1,#RECV_BUFFER		;���ջ�����
				MOV   RECV_LEN,R2
				SJMP  CH375_INT_RECV0
CH375_INT_RECV:	MOVX  A,@DPTR				;��������
CH375_INT_RECV0:	MOV   @R1,A
				INC   R1
				DJNZ  R2,CH375_INT_RECV		;������������ֱ������
				MOV   A,RECV_BUFFER			;���ֽ���������
				CJNE  A,#TEST_UPLOAD,IS_USB_CMD3
USB_CMD3_NEXT:	;�����ϴ��ٶ�
				MOV   DPTR,#CH375_CMD_PORT	;����ڵ�ַ
				MOV   A,#CMD_WR_USB_DATA7
				MOVX  @DPTR,A				;��USB�˵�2�ķ��ͻ�����д�����ݿ�
				NOP							;���ʱ��Ƶ�ʵ���16MHz�������ָ����ʱ
				MOV   DPTR,#CH375_DAT_PORT	;���ݿڵ�ַ
				MOV   A,#40H
				MOVX  @DPTR,A				;����д��������ݳ���
				MOV   R7,A
				MOV   A,TL0					;�����
USB_CMD3_SEND:	MOVX  @DPTR,A				;��������,Ϊ�˲����ٶ�,������Ч,24MHz��MCS51ÿд��һ���ֽ���Ҫ2uS
				DJNZ  R7,USB_CMD3_SEND		;������������ֱ������
				SJMP  CH375_INT_UNLK
IS_USB_CMD3:	CJNE  A,#TEST_START,IS_USB_CMD1
				NOP							;���Թ��̿�ʼ
; ������һ�β��������ϴ��ٶ�ʱ�������ϴ�������������������, �����ڵڶ��β���ǰ��Ҫ����ϴ�������
				MOV   DPTR,#CH375_CMD_PORT	;����ڵ�ַ
				MOV   A,#CMD_SET_ENDP7
				MOVX  @DPTR,A				;����USB�˵�2��IN
				NOP							;���ʱ��Ƶ�ʵ���16MHz�������ָ����ʱ
				MOV   DPTR,#CH375_DAT_PORT	;���ݿڵ�ַ
				MOV   A,#0EH				;ͬ������λ����
				MOVX  @DPTR,A				;����USB�˵�2��IN��æ,����NAK
				CLR   FLAG_SEND_WAIT		;������͵ȴ���־,֪ͨӦ�ó�����Լ�����������
				LJMP  CH375_INT_RET
IS_USB_CMD1:	CJNE  A,#TEST_DATA,IS_USB_CMD9
				NOP							;����������ȷ��
				MOV   DPTR,#CH375_CMD_PORT	;����ڵ�ַ
				MOV   A,#CMD_WR_USB_DATA7
				MOVX  @DPTR,A				;��USB�˵�2�ķ��ͻ�����д�����ݿ�
				NOP							;���ʱ��Ƶ�ʵ���16MHz�������ָ����ʱ
				MOV   DPTR,#CH375_DAT_PORT	;���ݿڵ�ַ
				MOV   A,RECV_LEN
				MOVX  @DPTR,A				;����д��������ݳ���
				MOV   R2,A					;�ս��յ������ݳ���
				MOV   R1,#RECV_BUFFER		;�ս��յ������ݿ�
USB_CMD1_NEXT:	MOV   A,@R1
				CPL   A						;����ȡ���󷵻�,�ɼ����Ӧ�ó�����������Ƿ���ȷ
				MOVX  @DPTR,A				;��������
				INC   R1
				DJNZ  R2,USB_CMD1_NEXT
				LJMP  CH375_INT_RET
IS_USB_CMD9:	SETB  FLAG_RECV_OK			;��������,���ý��ճɹ���־,֪ͨӦ�ó���ȡ�������ٷ���
				LJMP  CH375_INT_RET
;
CH375_UP_OK:	; USB�������ݷ��ͳɹ�
				MOV   A,THIS_CMD_CODE
				CJNE  A,#TEST_UPLOAD,CH375_INT_UNLK
				SJMP  USB_CMD3_NEXT			;�����ϴ��ٶ�,����׼���ϴ�����
CH375_INT_UNLK:	MOV   DPTR,#CH375_CMD_PORT	;����ڵ�ַ
				MOV   A,#CMD_UNLOCK_USB
				MOVX  @DPTR,A				;�ͷŵ�ǰUSB������
				CLR   FLAG_SEND_WAIT		;������͵ȴ���־,֪ͨӦ�ó�����Լ�����������
				LJMP  CH375_INT_RET
;
CH375_UP_INT_OK:	; USB�ж����ݷ��ͳɹ�
				MOV   DPTR,#CH375_CMD_PORT	;����ڵ�ַ
				MOV   A,#CMD_UNLOCK_USB
				MOVX  @DPTR,A				;�ͷŵ�ǰUSB������
				LJMP  CH375_INT_RET
;
;
END
;