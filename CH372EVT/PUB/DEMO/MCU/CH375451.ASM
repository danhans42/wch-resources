; CH375/CH372/CH451 EVT
; U2(AT89C51) Program
;
; Website:  http://winchiphead.com
; Email:    tech@winchiphead.com
; Author:   W.ch 2003.12
;
;****************************************************************************
;
; Ӧ�ò㶨��
CONST_CMD_LEN	EQU   07H				; �����ĳ���
; �������������ݶ���ͨ�������´��ܵ�(USB�˵�2��OUT)�´�, Ϊ�˷�ֹ���߻���,
; ���ǿ����ڼ����Ӧ�ó����뵥Ƭ������֮��Լ��, �����ĳ�������7, �����ݿ�ĳ��ȿ϶�����7, ����64,32��
; ����, ����Լ��, ���������ֽ���������, �ȵ�
DEMO_CH451_CMD	EQU   56H              ; PC���������CH451
DEMO_TEST_DATA	EQU   91H              ; ��������,��PC�����������������������ȡ���󷵻�
DEMO_UP_CLEAR	EQU   0A0H				; �����ϴ����ݿ�֮ǰ����ͬ��,ʵ�����õ�Ƭ������ϴ�����������������
DEMO_UP_DATA	EQU   0A1H				; �����ϴ����ݿ�,ָ���ⲿRAM��������ʼ��ַ,���޳���
DEMO_DOWN_DATA	EQU   0A2H				; �����´����ݿ�,ָ���ⲿRAM��������ʼ��ַ,���޳���
;
STACK           EQU    0FH             ;��ջ��ջ��(��������)
;
; CH375��������붨��
$INCLUDE			(..\..\MCU_IF1\MCS51ASM\CH375INC.ASM)
;
; ��Ҫ��������Ĳ���
CH375_CMD_PORT		EQU    0BDF1H         ;CH375����ڵĵ�ַ,��ַ������Զ�Ƭѡ
CH375_DAT_PORT		EQU    0BCF0H         ;CH375����ڵĵ�ַ,��ַ������Զ�Ƭѡ
;
; �й�CH451�Ķ���
CH451_DCLK		BIT   P1.7             ;��������ʱ��,�����ؼ���
CH451_DIN		BIT   P1.6             ;�����������,��CH451����������
CH451_LOAD		BIT   P1.5             ;�����������,�����ؼ���
CH451_DOUT		BIT   P3.3             ;INT1,�����жϺͼ�ֵ��������,��CH451���������
CH451_KEY		DATA  28H              ;��ż����ж��ж�ȡ�ļ�ֵ
LAST_KEY		DATA  29H              ;�����ϴεļ�ֵ
;
; �й�CH375�Ķ���
CH451_CMD_H		DATA  30H				;PC������CH451�ĸ�4λ����,Ϊ0FFH��������Ч
CH451_CMD_L		DATA  31H				;PC������CH451�ĵ�8λ����
BUF_ADDR_L		DATA  32H				;�������ڴ�����ⲿRAM�����ڲ�RAM�������ĵ�ַ,��8λ
BUF_ADDR_H		DATA  33H				;�������ڴ�����ⲿRAM�����ڲ�RAM�������ĵ�ַ,��8λ
BUF_LEN_L		DATA  34H				;��ǰʣ���Ҫ��������ݳ���,��8λ
BUF_LEN_H		DATA  35H				;��ǰʣ���Ҫ��������ݳ���,��8λ
CMD_BUFFER		DATA  38H				;�����������,������Լ�����������ΪCONST_CMD_LEN
CMD_BUFFER_NOT	DATA  39H				;������ķ���,����У�������
CMD_BUFFER_ID	DATA  3AH				;��������������ֽ�,������ʶ����,���MCS51��Ƭ������: 1-ר�ù��ܼĴ���SFR, 2-�ڲ�RAM, 3-�ⲿRAM, ����������ʵ��ֻ��ʾ�ڲ�RAM
CMD_BUFFER_ADRL	DATA  3BH				;�������������3�ֽ�,��������ַ��8λ
CMD_BUFFER_ADRH	DATA  3CH				;�������������4�ֽ�,��������ַ��8λ
CMD_BUFFER_LENL	DATA  3DH				;�������������5�ֽ�,�����ܳ��ȵ�8λ
CMD_BUFFER_LENH	DATA  3EH				;�������������6�ֽ�,�����ܳ��ȸ�8λ
; ����MCS-51��Ƭ����ȡ�ⲿRAM�Ķ�д�ٶȵ����ڲ�RAM, ������Ҫ�õ�DPTR, ���Զ�д�ٶȽ���, �������Բο��������޸�
;
; ����λ��־
FLAG_INT_WAIT	BIT    2FH.2          ;�жϵȴ���־,1ָʾ���ж���������CH375�еȴ�����
;
;
;****************************************************************************
;������,���������ʾCH375��CH451����
				ORG   0000H            ;��λ��Ƭ�����
				LJMP  START
				ORG   0003H            ;CH375�ж�
				LJMP  CH375_INTER
				ORG   0013H            ;CH451�ж�,�����жϺ�֪ͨPC��Ӧ�ò�
				LJMP  CH451_INTER
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
				MOV   R7,#0FFH
				CALL  DELAY_MS			;��ʱ�ȴ�CH375��ʼ�����
; ��ʼ��
				CALL  CH375_INIT       ;��ʼ��
				CALL  CH451_INIT       ;��ʼ��
;				.....
; ����CH451����ʾ�����ͼ���ɨ��
				MOV   B,#04H           ;����ϵͳ��������
				MOV   A,#03H           ;��ֹ���Ź�,ʹ�ܼ��̹���,ʹ����ʾ����
				CALL  CH451_WRITE      ;��������
				MOV   B,#05H           ;������ʾ����
				MOV   A,#8CH           ;BCD���뷽ʽ,12������
				CALL  CH451_WRITE      ;��������
				MOV   CH451_KEY,#0FFH  ;�����������,��ʹ�ܼ���ǰ,CH451��DOUT�����������
				MOV   LAST_KEY,#0FFH   ;�п��ܱ����������ж�����INT1,����Ҫ�����
				MOV   CH451_CMD_H,#0FFH	;���CH451��������
; ������CH451�������̹���ǰ�Ĵ�����������п��ܲ��������ж�,������Ҫ�ٴ�ȡ���ж�
				CLR   IE1
				SETB  EA               ;�����ж�
;				...
;
; ���P2�ĵ�5�ź͵�6��֮����϶�·�����ɵ�Ƭ��������ʾ,������PC��ͨ��USB���ƽ�����ʾ
				JB    P3.5,WAIT_USB_REQ ;T1,��PC��ͨ��USB��ʾ
;
; �ɵ�Ƭ��������ʾ,�ȴ�����,Ȼ��������ԭ��ʾ,�ټ��ϰ���ֵ
				MOV   B,#0FH
				MOV   A,#17H           ;���1������ܼ�������'H'
				CALL  CH451_WRITE      ;��������
				MOV   B,#0EH
				MOV   A,#0EH           ;���2������ܼ�������'E'
				CALL  CH451_WRITE      ;��������
				MOV   B,#0DH
				MOV   A,#18H           ;���3������ܼ�������'L'
				CALL  CH451_WRITE      ;��������
				MOV   B,#0CH
				MOV   A,#19H           ;���4������ܼ�������'P'
				CALL  CH451_WRITE      ;��������
				MOV   B,#0BH
				MOV   A,#10H           ;���5������ܼ�������' '
				CALL  CH451_WRITE      ;��������
				MOV   B,#0AH
				MOV   A,#14H           ;���6������ܼ�������'['
				CALL  CH451_WRITE      ;��������
				MOV   B,#09H
				MOV   A,#88H           ;���7������ܼ�������'8.'
				CALL  CH451_WRITE      ;��������
				MOV   B,#08H
				MOV   A,#15H           ;���8������ܼ�������']'
				CALL  CH451_WRITE      ;��������
				MOV   B,#06H           ;��˸����
				MOV   A,#30H           ;��3���͵�4���������˸
				CALL  CH451_WRITE      ;��������
;
; ������ʾΪ��ʾ��������,��������
WAIT_KEY:		MOV   A,#0FFH
				XCH   A,CH451_KEY      ;ȡ����ֵ�����ԭֵ
				MOV   R2,A
				XRL   A,#0FFH
				JZ    WAIT_KEY         ;û�а���
; ��⵽�°�������������ʾ����ʾ��ֵ
				MOV   A,#00H           ;����һλ
				MOV   B,#03H           ;��λ����
				CALL  CH451_WRITE      ;��������
				MOV   A,#00H           ;����һλ
				MOV   B,#03H           ;��λ����
				CALL  CH451_WRITE      ;��������
				MOV   A,R2
				ANL   A,#3FH           ;��ֵ0-63
				MOV   B,#0AH
				DIV   AB               ;AΪ��ֵ��ʮλ��
				MOV   B,#09H           ;���7������ܼ�������
				CALL  CH451_WRITE      ;��������
				MOV   A,R2
				ANL   A,#3FH           ;��ֵ0-63
				MOV   B,#0AH
				DIV   AB
				MOV   A,B              ;AΪ��ֵ�ĸ�λ��
				MOV   B,#08H           ;���8������ܼ�������
				CALL  CH451_WRITE      ;��������
				SJMP  WAIT_KEY
;
; ����ָ�ʼ����ѭ��,�ȴ�PC��������в���
WAIT_USB_REQ:	NOP
				NOP
				NOP						;��������
				NOP

				MOV   A,CH451_CMD_H	   ;��4λ����
				CJNE  A,#0FFH,IS_CH451_CMD	;��CH375�жϷ�������н��յ�PC������CH451������

; û��CH451������, ���¼�ⰴ��״̬
;    LAST_KEY��CH451_KEY��������,����0FFH��û�м�����
;    ��CH451_KEYΪ��Ч��ֵ���Ǽ��հ���, ���߶�����Ч��ֵ������ͬ���Ѿ�֪ͨ�������
				MOV   A,LAST_KEY
				XRL   A,#0FFH
				JNZ   IS_KEY_RELEASE	;�����Ѿ����²����Ѿ�֪ͨ��PC��,����ȥ��ⰴ���ͷ�
				MOV   A,CH451_KEY
				XRL   A,#0FFH
				JZ    WAIT_USB_REQ		;û�м�����,��ѭ��
				MOV   A,CH451_KEY
				CLR   C
				SUBB  A,#40H
				JC    WAIT_USB_REQ		;������Ч
; ��⵽�µļ���,��������Ч���ж���������,��֪ͨPC��Ӧ�ò�
				MOV   A,CH451_KEY
				MOV   LAST_KEY,A
				MOV   R0,#01H          ;�趨�ж���������01H,֪ͨPC����Ӧ�ò�,�����Ѱ���
				MOV   R7,LAST_KEY		;�ж����ݵĴ��ֽھ��Ǽ���,����PC��Ӧ�ò�ֱ�ӻ�ȡ��ֵ,�����ٶ�ȡ
				CALL  CH375_UP_INT2		;�ϴ��ж�����,���������ڼ����ж�,����ÿ��ֻ��Ҫ�������ֽ�
				LJMP  WAIT_USB_REQ		;��ѭ��
IS_KEY_RELEASE:	NOP
				CALL  CH451_READ       ;���¶�ȡ���һ����Ч�����ļ�ֵ
				MOV   CH451_KEY,A		;�����ֵ
				XRL   A,LAST_KEY
				JZ    WAIT_USB_REQ     ;������ͬ,������Ȼû���ͷ�
				MOV   R0,#02H          ;�趨�ж���������02H,֪ͨPC����Ӧ�ò�,�������ͷ�
				MOV   R7,LAST_KEY		;�ж����ݵĴ��ֽھ��Ǽ���,����PC��Ӧ�ò�ֱ�ӻ�ȡ��ֵ
				MOV   LAST_KEY,#0FFH   ;��Ҫ�����ͷ��źŸ�PC
				CALL  CH375_UP_INT2		;�ϴ��ж�����
				LJMP  WAIT_USB_REQ		;��ѭ��

; PC���������CH451,�����������е�CH451_READ���ܻᱻCH375�ĸ����ȼ��ж�,���Բ�����CH375�жϷ��������ִ��
IS_CH451_CMD:	NOP
				MOV   B,CH451_CMD_H		;��4λ����
				MOV   A,CH451_CMD_L		;��8λ����
				MOV   CH451_CMD_H,#0FFH	;���ԭ������,��ֹ�ط�
				CALL  CH451_WRITE      ;��CH451��������
				LJMP  WAIT_USB_REQ		;��ѭ��
;
;****************************************************************************
;
; ��ʼ���ӳ���
; USE:   ACC, R7, DPTR
CH375_INIT:		CLR   FLAG_INT_WAIT			;���жϵȴ���־,1ָʾ���ж���������CH375�еȴ�����
; ����CH375�Ƿ���������,��ѡ����
;				MOV   DPTR,#CH375_CMD_PORT	;����ڵ�ַ
;				MOV   A,#CMD_CHECK_EXIST
;				MOVX  @DPTR,A				;����CH375�Ƿ���������,��ѡ����
;				NOP							;���ʱ��Ƶ�ʵ���16MHz�������ָ����ʱ
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
;CH375_UPLOAD:	JB    FLAG_SEND_WAIT,CH375_UPLOAD	;������ڵȴ�����(ǰ�η�����δ���)������ȴ�
;				CLR   EX0					;Ϊ�˷�ֹ��;���ж϶�����˳��,�����Ƚ�ֹ�ж�
;				MOV   DPTR,#CH375_CMD_PORT	;����ڵ�ַ
;				MOV   A,#CMD_WR_USB_DATA7
;				MOVX  @DPTR,A				;��USB�˵�2�ķ��ͻ�����д�����ݿ�
;				NOP							;���ʱ��Ƶ�ʵ���16MHz�������ָ����ʱ
;				MOV   DPTR,#CH375_DAT_PORT	;���ݿڵ�ַ
;				MOV   A,R7
;				MOVX  @DPTR,A				;����д��������ݳ���
;CH375_UPLOAD_1:	MOV   A,@R0
;				INC   R0
;				MOVX  @DPTR,A				;��������
;				DJNZ  R7,CH375_UPLOAD_1		;������������ֱ������
;				SETB  FLAG_SEND_WAIT		;�õȴ����ͱ�־
;				SETB  EX0					;�����ж�
;				RET
;
; �ϴ��ж������ӳ���(ͨ���ж϶˵��ϴ�),��ѡ�ӳ���
; ��Ȼÿ�ο����ϴ�8���ֽ�, ���Ǳ����������ڼ����ж�, ����ÿ��ֻ��Ҫ���������ֽ�
; ENTRY: R0 �ж��������ֽ�, R7 �ж��������ֽ�
; USE:   ACC, R0, R7, DPTR
CH375_UP_INT2:	MOV   A,#0FFH
CH375_UP_INT2W:	JNB   FLAG_INT_WAIT,CH375_UP_INT2G	;ǰ���ж������Ѿ���PC��ȡ�������
				NOP							;�ȴ������ж�����
				NOP
				NOP							;���ȴ�1����,��24MHzʱ�Ӽ���
				NOP
				DJNZ  ACC,CH375_UP_INT2W	;�����ȴ�,��1�����ʱ
; ���δ����������,���߼�����е�Ӧ�ó���δ����,�����ֳ�ʱ,���򲻳���1�����ж����ݾ�Ӧ�ñ�PC��ȡ��
CH375_UP_INT2G:	CLR   EX0					;Ϊ�˷�ֹ��;���ж϶�����˳��,�����Ƚ�ֹ�ж�
				MOV   DPTR,#CH375_CMD_PORT	;����ڵ�ַ
				MOV   A,#CMD_WR_USB_DATA5
				MOVX  @DPTR,A				;��USB�˵�1�ķ��ͻ�����д�����ݿ�
				NOP							;���ʱ��Ƶ�ʵ���16MHz�������ָ����ʱ
				MOV   DPTR,#CH375_DAT_PORT	;���ݿڵ�ַ
				MOV   A,#02H
				MOVX  @DPTR,A				;����д��������ݳ���
				MOV   A,R0
				MOVX  @DPTR,A				;�����ж�����1
				MOV   A,R7
				MOVX  @DPTR,A				;�����ж�����2
				SETB  FLAG_INT_WAIT			;���жϵȴ���־
				SETB  EX0					;�����ж�
				RET
;
; �ϴ��ж������ӳ���(ͨ���ж϶˵��ϴ�),��ѡ�ӳ���
; ENTRY: R0 ָ������׼���ϴ����ݵĻ�����, R7 ׼���ϴ������ݳ���, ��Ч��ֵ��0��8
; USE:   ACC, R0, R7, DPTR
;CH375_UP_INT:	CLR   EX0					;Ϊ�˷�ֹ��;���ж϶�����˳��,�����Ƚ�ֹ�ж�
;				MOV   DPTR,#CH375_CMD_PORT	;����ڵ�ַ
;				MOV   A,#CMD_WR_USB_DATA5
;				MOVX  @DPTR,A				;��USB�˵�1�ķ��ͻ�����д�����ݿ�
;				NOP							;���ʱ��Ƶ�ʵ���16MHz�������ָ����ʱ
;				MOV   DPTR,#CH375_DAT_PORT	;���ݿڵ�ַ
;				MOV   A,R7
;				MOVX  @DPTR,A				;����д��������ݳ���
;CH375_UP_INT_1:	MOV   A,@R0
;				INC   R0
;				MOVX  @DPTR,A				;�����ж�����
;				DJNZ  R7,CH375_UP_INT_1		;������������ֱ������
;				SETB  EX0					;�����ж�
;				RET
;
; �жϷ����ӳ���
; USE:   ��ջ6�ֽ�, �����Ĵ�����1��R0,R5,R6,R7
CH375_INTER:	PUSH  PSW					;�ֳ�����
				PUSH  ACC
				PUSH  DPL
				PUSH  DPH
				SETB  RS0                   ;PSW.3,�л��������Ĵ�����1
				MOV   DPTR,#CH375_CMD_PORT	;����ڵ�ַ
				MOV   A,#CMD_GET_STATUS
				MOVX  @DPTR,A				;��ȡ�ж�״̬��ȡ���ж�����
				NOP							;���ʱ��Ƶ�ʵ���16MHz�������ָ����ʱ,����30MHzҪ���2��ָ��
				NOP
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
CH375_INT_RET:	POP   DPH
				POP   DPL
				POP   ACC
				POP   PSW					;�ָ��Ĵ���
				RETI						;�жϷ���

; USB�������ݽ��ճɹ�
CH375_DOWN_OK:	MOV   DPTR,#CH375_CMD_PORT	;����ڵ�ַ
				MOV   A,#CMD_RD_USB_DATA
				MOVX  @DPTR,A				;�ӵ�ǰUSB�жϵĶ˵㻺������ȡ���ݿ�,���ͷŻ�����
				NOP							;���ʱ��Ƶ�ʵ���16MHz�������ָ����ʱ,����30MHzҪ���2��ָ��
				NOP
				MOV   DPTR,#CH375_DAT_PORT	;���ݿڵ�ַ
				MOVX  A,@DPTR				;���ȶ�ȡ�������ݳ���
				MOV   R7,A
				JZ    CH375_INT_RET			;����Ϊ0,û��������ֱ���˳�,��ĳЩӦ����Ҳ���Խ�����0����Ϊһ����������
				CJNE  A,#CONST_CMD_LEN,CH375_INT_DOWN	;���������
				LJMP  CH375_INT_CMD			;����鳤������CONST_CMD_LEN
; �����´������ݿ�,ÿ�����ݵĳ��Ȳ�����64�ֽ�,�����ܳ���150���ֽ�,���1��͵�2�����64�ֽ�,��3����ʣ�೤��22�ֽ�
; Ϊ�˽����ݿ�����������ֿ���,ʣ�೤�Ȳ��ܵ���CONST_CMD_LEN,������������ݵİ취������ͬ
CH375_INT_DOWN:	LJMP  CH375_INT_DN1			;�������ݿ�

; ����ͨ��USB���յ��������,��������CONST_CMD_LEN,���ֽ�Ϊ����,����Ϊ��ѡ�Ĳ���
CH375_INT_CMD:	MOV   R0,#CMD_BUFFER		;�����������
CH375_INT_CMD0:	MOVX  A,@DPTR				;���������������
				MOV   @R0,A
				INC   R0
				DJNZ  R7,CH375_INT_CMD0		;�����������������ֱ������
; ����ͨ��USB���յ��������,��������CONST_CMD_LEN,���ֽ�Ϊ����,����Ϊ��ѡ�Ĳ���
; �������ֽ�Ϊ����Ľṹ���ɵ�Ƭ���ͼ����Ӧ�ò�֮�����ж����
				MOV   A,CMD_BUFFER			;ȡ���������
				CPL   A
				XRL   A,CMD_BUFFER_NOT
				JNZ   CH375_INT_RET			;���������У�����
				MOV   A,CMD_BUFFER			;������
				CJNE  A,#DEMO_UP_CLEAR,CH375_INT_CMD1
; �����ϴ����ݿ�֮ǰ����ͬ��,ʵ�����õ�Ƭ������ϴ�����������������
; �����һ�ν��������ϴ�ʱ,�������ǰ�����ϴ�,��ô�п������ϴ�������������������,�����ڵڶ����ϴ�ǰ��Ҫ����ϴ�������
				MOV   DPTR,#CH375_CMD_PORT	;����ڵ�ַ
				MOV   A,#CMD_SET_ENDP7
				MOVX  @DPTR,A				;����USB�˵�2��IN,Ҳ���������ϴ��˵�
				NOP							;���ʱ��Ƶ�ʵ���16MHz�������ָ����ʱ
				MOV   DPTR,#CH375_DAT_PORT	;���ݿڵ�ַ
				MOV   A,#0EH				;ͬ������λ����
				MOVX  @DPTR,A				;����USB�˵�2��IN��æ,����NAK,Ҳ���ǻ�������û������
				LJMP  CH375_INT_RET

CH375_INT_CMD1:	CJNE  A,#DEMO_DOWN_DATA,CH375_INT_CMD2
; �����´����ݿ�
; �ٶ�����Լ��, �������ڵ�3�ֽں͵�4�ֽ�ָ�����潫�´�������Ӧ�ô�����ⲿRAM����ʼ��ַ
				MOV   A,CMD_BUFFER_ADRL
				MOV   BUF_ADDR_L,A			;���������´����ⲿRAM�������ĵ�ַ
				MOV   A,CMD_BUFFER_ADRH
				MOV   BUF_ADDR_H,A
				MOV   A,CMD_BUFFER_LENL
				MOV   BUF_LEN_L,A			;�ٶ�����Լ����5�͵�6�ֽ��ǽ�Ҫ�´������ݳ���
				MOV   A,CMD_BUFFER_LENH
				MOV   BUF_LEN_H,A
				LJMP  CH375_INT_RET

CH375_INT_CMD2:	CJNE  A,#DEMO_UP_DATA,CH375_INT_CMD3
; �����ϴ����ݿ�
; �ٶ�����Լ��, �������ڵ�3�ֽں͵�4�ֽ�ָ�����潫�ϴ�������Ӧ�������ⲿRAM����ʼ��ַ
				MOV   A,CMD_BUFFER_ADRL
				MOV   BUF_ADDR_L,A			;���������ϴ����ⲿRAM�������ĵ�ַ
				MOV   A,CMD_BUFFER_ADRH
				MOV   BUF_ADDR_H,A
				MOV   A,CMD_BUFFER_LENL
				MOV   BUF_LEN_L,A			;�ٶ�����Լ����5�͵�6�ֽ��ǽ�Ҫ�´������ݳ���
				MOV   A,CMD_BUFFER_LENH
				MOV   BUF_LEN_H,A
; �����ϴ�,׼����һ������,ÿ�����ݵĳ��Ȳ�����64�ֽ�
				CLR   C
				MOV   A,BUF_LEN_L
				SUBB  A,#40H				;���ʣ�೤��
				MOV   R5,A
				MOV   A,BUF_LEN_H
				SUBB  A,#00H
				MOV   R6,A
				JC    CH375_UP_SHORT		;ʣ�೤�Ȳ���һ�����ݰ�����󳤶�64
				MOV   R7,#40H
				MOV   BUF_LEN_L,R5
				MOV   BUF_LEN_H,R6
				SJMP  CH375_UP_FIRST
CH375_UP_SHORT:	MOV   R7,BUF_LEN_L
				MOV   BUF_LEN_L,#00H
				MOV   BUF_LEN_H,#00H
CH375_UP_FIRST:	MOV   DPTR,#CH375_CMD_PORT	;����ڵ�ַ
				MOV   A,#CMD_WR_USB_DATA7
				MOVX  @DPTR,A				;��USB�˵�2�ķ��ͻ�����д�����ݿ�
				NOP							;���ʱ��Ƶ�ʵ���16MHz�������ָ����ʱ
				MOV   DPTR,#CH375_DAT_PORT	;���ݿڵ�ַ
				MOV   A,R7					;ÿ������ϴ�64���ֽڵ�����
				MOVX  @DPTR,A				;����д��������ݳ���
; �����Ǵ�MCS51��Ƭ�����ڲ�RAM��ȡ���ݲ��ϴ�,�ٶ����ⲿRAM��3��
				MOV   R0,BUF_ADDR_L			;������ʾ����û���ⲿRAM,�����ⲿRAM��Ч�ʺܵ�,�������ڲ�RAMʾ��
CH375_INT_UP0:	MOV   A,@R0					;���ڲ�RAM������ȡ��׼���ϴ�������
				INC   R0					;��12MHzʱ�������ַ���ÿ����1�ֽڴ����Ҫ6uS
				MOVX  @DPTR,A				;������д�������˵�ķ��ͻ�����
				DJNZ  R7,CH375_INT_UP0		;����д������ֱ������
				LJMP  CH375_INT_RET
; �����MCS51��Ƭ�����ⲿRAM��ȡ���ݲ��ϴ�,������ýϸ�ʱ�Ӳ��Ҿ���˫DPTR�ĵ�Ƭ�����ٶȿ��Կ�Щ
;CH375_INT_UP0:	MOV   DPL,BUF_ADDR_L		;����MCS51��Ƭ����ȡ�ⲿRAM��Ч�ʺܵ�
;				MOV   DPH,BUF_ADDR_H		;������12MHzʱ�������ַ���ÿ����1�ֽڴ����Ҫ18uS
;				MOVX  A,@DPTR				;���ⲿRAM������ȡ��׼���ϴ�������
;				INC   DPTR					;��������ַ����
;				MOV   BUF_ADDR_L,DPL
;				MOV   BUF_ADDR_H,DPH
;				MOV   DPTR,#CH375_DAT_PORT	;���ݿڵ�ַ
;				MOVX  @DPTR,A				;������д�������˵�ķ��ͻ�����
;				DJNZ  R7,CH375_INT_UP0		;����д������ֱ������
;				LJMP  CH375_INT_RET

CH375_INT_CMD3:	CJNE  A,#DEMO_TEST_DATA,CH375_INT_CMD4
; �������ݴ�������,��PC�������ĵ�ǰ��������������ݰ�λȡ���󷵻�
				MOV   DPTR,#CH375_CMD_PORT	;����ڵ�ַ
				MOV   A,#CMD_WR_USB_DATA7
				MOVX  @DPTR,A				;�������˵�ķ��ͻ�����д�����ݿ�
				NOP							;���ʱ��Ƶ�ʵ���16MHz�������ָ����ʱ
				MOV   DPTR,#CH375_DAT_PORT	;���ݿڵ�ַ
				MOV   A,#CONST_CMD_LEN
				MOV   R7,A
				MOVX  @DPTR,A				;����д��������ݳ���,Ҳ����������ĳ���
				MOV   R0,#CMD_BUFFER		;�����������
CH375_TEST_DAT:	MOV   A,@R0
				CPL   A                     ;���ݰ�λȡ��
				MOVX  @DPTR,A
				INC   R0
				DJNZ  R7,CH375_TEST_DAT     ;����������ݰ�λȡ��д�������˵�ķ��ͻ�����
				MOV   BUF_LEN_L,#00H		;ʣ�����ݳ���Ϊ0,ֻ����һ������
				MOV   BUF_LEN_H,#00H
				LJMP  CH375_INT_RET

CH375_INT_CMD4:	CJNE  A,#DEMO_CH451_CMD,CH375_INT_CMD5
; PC���������CH451
				MOV   A,CMD_BUFFER_ADRH
				MOV   CH451_CMD_H,A         ;��4λ����
				MOV   A,CMD_BUFFER_ADRL     ;��8λ����
				MOV   CH451_CMD_L,A
; Ϊ�˷�ֹ��CH375�жϷ���������������е�CH451_READ��ִ��CH451_WRITE��������
; �����ڴ˱���CH451�����������������ڿ���ʱ����CH451
				LJMP  CH375_INT_RET

CH375_INT_CMD5:	NOP							;δ�����������,��֧��,����
				LJMP  CH375_INT_RET

; �����´������ݿ�,ÿ�����ݵĳ��Ȳ�����64�ֽ�
CH375_INT_DN1:
; ��������MCS51��Ƭ�����ڲ�RAM�����´�������,�ٶ����ⲿRAM��3��
				MOV   R0,BUF_ADDR_L			;������ʾ����û���ⲿRAM,�����ⲿRAM��Ч�ʺܵ�,�������ڲ�RAMʾ��
				MOV   R0,#40H				;Ϊ�˷�ֹ������ʱ���������������00H-3FH,��ʱָ��ר�û�����
CH375_INT_DN0:	MOVX  A,@DPTR				;�������˵�Ľ��ջ�������ȡ����
				MOV   @R0,A					;���ڲ�RAM������������յ����´�����
				INC   R0					;��12MHzʱ�������ַ���ÿ����1�ֽڴ����Ҫ6uS
				DJNZ  R7,CH375_INT_DN0		;����ȡ������ֱ������
				LJMP  CH375_INT_RET
; �����MCS51��Ƭ�����ⲿRAM��ȡ���ݲ��ϴ�,������ýϸ�ʱ�Ӳ��Ҿ���˫DPTR�ĵ�Ƭ�����ٶȿ��Կ�Щ
;CH375_INT_DN0:	MOV   DPTR,#CH375_DAT_PORT	;���ݿڵ�ַ
;				MOVX  A,@DPTR				;�������˵�Ľ��ջ�������ȡ����
;				MOV   DPL,BUF_ADDR_L		;����MCS51��Ƭ����ȡ�ⲿRAM��Ч�ʺܵ�
;				MOV   DPH,BUF_ADDR_H		;������12MHzʱ�������ַ���ÿ����1�ֽڴ����Ҫ18uS
;				MOVX  @DPTR,A				;���ⲿRAM������������յ����´�����
;				INC   DPTR					;��������ַ����
;				MOV   BUF_ADDR_L,DPL
;				MOV   BUF_ADDR_H,DPH
;				DJNZ  R7,CH375_INT_DN0		;����ȡ������ֱ������
;				LJMP  CH375_INT_RET

; USB�������ݷ��ͳɹ�
CH375_UP_OK:
; �ٶ����������ָ�����ܹ���Ҫ�ϴ������ݳ���,����ݸó����ϴ�����
				MOV   A,BUF_LEN_L			;���ʣ�೤��
				ORL   A,BUF_LEN_H
				JZ    CH375_INT_UNLK		;ʣ�����ݳ���Ϊ0,û��������Ҫ����,������ֱ���˳�
				CLR   C
				MOV   A,BUF_LEN_L
				SUBB  A,#40H				;���ʣ�೤��
				MOV   R5,A
				MOV   A,BUF_LEN_H
				SUBB  A,#00H
				MOV   R6,A
				JC    CH375_UP_END			;ʣ�೤�Ȳ���һ�����ݰ�����󳤶�64
				MOV   R7,#40H
				MOV   BUF_LEN_L,R5
				MOV   BUF_LEN_H,R6
				SJMP  CH375_UP_NEXT
CH375_UP_END:	MOV   R7,BUF_LEN_L
				MOV   BUF_LEN_L,#00H
				MOV   BUF_LEN_H,#00H
CH375_UP_NEXT:	MOV   DPTR,#CH375_CMD_PORT	;����ڵ�ַ
				MOV   A,#CMD_WR_USB_DATA7
				MOVX  @DPTR,A				;��USB�˵�2�ķ��ͻ�����д�����ݿ�
				NOP							;���ʱ��Ƶ�ʵ���16MHz�������ָ����ʱ
				MOV   DPTR,#CH375_DAT_PORT	;���ݿڵ�ַ
				MOV   A,R7					;ÿ������ϴ�64���ֽڵ�����
				MOVX  @DPTR,A				;����д��������ݳ���
; �����Ǵ�MCS51��Ƭ�����ڲ�RAM��ȡ���ݲ��ϴ�,�ٶ����ⲿRAM��3��
				MOV   R0,BUF_ADDR_L			;������ʾ����û���ⲿRAM,�����ⲿRAM��Ч�ʺܵ�,�������ڲ�RAMʾ��
CH375_INT_UP1:	MOV   A,@R0					;���ڲ�RAM������ȡ��׼���ϴ�������
				INC   R0					;��12MHzʱ�������ַ���ÿ����1�ֽڴ����Ҫ6uS
				MOVX  @DPTR,A				;������д�������˵�ķ��ͻ�����
				DJNZ  R7,CH375_INT_UP1		;����д������ֱ������
				LJMP  CH375_INT_UNLK
; �����MCS51��Ƭ�����ⲿRAM��ȡ���ݲ��ϴ�,������ýϸ�ʱ�Ӳ��Ҿ���˫DPTR�ĵ�Ƭ�����ٶȿ��Կ�Щ
;CH375_INT_UP1:	MOV   DPL,BUF_ADDR_L		;����MCS51��Ƭ����ȡ�ⲿRAM��Ч�ʺܵ�
;				MOV   DPH,BUF_ADDR_H		;������12MHzʱ�������ַ���ÿ����1�ֽڴ����Ҫ18uS
;				MOVX  A,@DPTR				;���ⲿRAM������ȡ��׼���ϴ�������
;				INC   DPTR					;��������ַ����
;				MOV   BUF_ADDR_L,DPL
;				MOV   BUF_ADDR_H,DPH
;				MOV   DPTR,#CH375_DAT_PORT	;���ݿڵ�ַ
;				MOVX  @DPTR,A				;������д�������˵�ķ��ͻ�����
;				DJNZ  R7,CH375_INT_UP1		;����д������ֱ������
;				LJMP  CH375_INT_UNLK

; �յ��ϴ��ɹ��жϺ�,���˳�֮ǰ�������USB������,�Ա�����շ�����
CH375_INT_UNLK:	MOV   DPTR,#CH375_CMD_PORT	;����ڵ�ַ
				MOV   A,#CMD_UNLOCK_USB
				MOVX  @DPTR,A				;�ͷŵ�ǰUSB������
				LJMP  CH375_INT_RET

; USB�ж����ݷ��ͳɹ�
CH375_UP_INT_OK:	MOV   DPTR,#CH375_CMD_PORT	;����ڵ�ַ
				MOV   A,#CMD_UNLOCK_USB
				MOVX  @DPTR,A				;�ͷŵ�ǰUSB������
				CLR   FLAG_INT_WAIT			;���жϵȴ���־,֪ͨӦ�ó�����Լ��������ж�����
				LJMP  CH375_INT_RET
;
;****************************************************************************
;
; ��ʼ���ӳ���
CH451_INIT:		CLR   CH451_DIN        ;�ȵͺ��,���������֪ͨCH451ѡ��4�ߴ��нӿ�
				SETB  CH451_DCLK       ;��ΪĬ�ϵĸߵ�ƽ
				SETB  CH451_DIN
				SETB  CH451_LOAD
				SETB  CH451_DOUT       ;��Ϊ����
; ��������ָ�����ڼ����ж�
				CLR   IT1              ;���ⲿ�ź�Ϊ�͵�ƽ����
				CLR   IE1              ;���жϱ�־
				CLR   PX1              ;�õ����ȼ�
				SETB  EX1              ;���������ж�
				RET
;
;
; ��������ӳ���
; ENTRY: B-ACC ��д��12λ����, ��8λ��ACC��, ��4λ��B�ĵ�4λ��
; USE:   ACC, B, R7
CH451_WRITE:	CLR   EX1              ;��ֹ�����ж�
				CLR   CH451_LOAD       ;���ʼ,��������Է��ں���
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
				SETB  EX1              ;���������ж�
				RET
;
;
; �����ֵ�ӳ���
; EXIT:  ACC ��ֵ����
; USE:   ACC, R7
CH451_READ:		CLR   EX1              ;��ֹ�����ж�
				CLR   CH451_LOAD       ;���ʼ
				MOV   A,#07H           ;��ȡ��ֵ����ĸ�4λ0111B
				MOV   R7,#04H          ;����12λ����ĵ�8λ
CH451_READ_4:	RRC   A                ;��λ��ǰ,��λ�ں�
				CLR   CH451_DCLK
				MOV   CH451_DIN,C      ;�ͳ�һλ����
				SETB  CH451_DCLK       ;����ʱ����������֪ͨCH451����λ����
				DJNZ  R7,CH451_READ_4  ;λ����δ�����
				SETB  CH451_LOAD       ;��������������֪ͨCH451������������
				CLR   A                ;�������ֵ��Ԫ�Ա���λ
				MOV   R7,#07H          ;����7λ��ֵ
CH451_READ_7:	MOV   C,CH451_DOUT     ;����һλ����
				CLR   CH451_DCLK       ;����ʱ��������֪ͨCH451�����һλ
				RLC   A                ;��������ACC,��λ��ǰ,��λ�ں�
				SETB  CH451_DCLK
				DJNZ  R7,CH451_READ_7  ;λ����δ�����
				CLR   IE1              ;���жϱ�־,�������������е͵�ƽ����
				SETB  EX1              ;���������ж�
				RET
;
;
; �����ж��ӳ���
; USE:   ��ջ6�ֽ�
CH451_INTER:	PUSH  PSW              ;�ֳ�����
				PUSH  ACC
				CLR   CH451_LOAD       ;���ʼ
				MOV   A,#0F7H          ;����ֵ����,����12λ����ĵ�8λ,��4λ����������־
CH451_INTER_4:	SETB  C                ;�ڸ�λ��0�Լ��λ���ݽ���
				RRC   A                ;��λ��ǰ,��λ�ں�
				CLR   CH451_DCLK
				MOV   CH451_DIN,C      ;�ͳ�һλ����
				SETB  CH451_DCLK       ;����ʱ����������֪ͨCH451����λ����
				CJNE  A,#0FFH,CH451_INTER_4 ;λ����δ�����,��4λ,��ɺ�Ϊ0FFH
				SETB  CH451_LOAD       ;��������������֪ͨCH451������������
				MOV   A,#0FCH          ;���������Լ��λ���ݽ���
CH451_INTER_7:	MOV   C,CH451_DOUT     ;����һλ����
				CLR   CH451_DCLK       ;����ʱ��������֪ͨCH451�����һλ
				RLC   A                ;��������ACC,��λ��ǰ,��λ�ں�
				SETB  CH451_DCLK
				JC    CH451_INTER_7    ;λ����δ�����,��7λ,��ɺ���Ƴ�0
				MOV   CH451_KEY,A      ;�����ֵ
				POP   ACC
				POP   PSW
				CLR   IE1              ;���жϱ�־,��ָ�������ʵ��������޸�
				RETI                   ;�жϷ���
;
;*********************************************************************************************************************
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
;*********************************************************************************************************************
;
END
;