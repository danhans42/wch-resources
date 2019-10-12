/* 2004.03.05, 2004.8.18
****************************************
**  Copyright  (C)  W.ch  1999-2004   **
**  Web:  http://www.winchiphead.com  **
****************************************
**  USB 1.1 Host Examples for CH375   **
**  KC7.0@MCS-51                      **
****************************************
*/
/* ��CH375+CH372����������Ƭ��ϵͳ,����豸�˲���CH37X,��ô���Բο�CH375PRT.C���������� */
/* �����˵ĳ���ʾ��,C����,CH375�ж�Ϊ��ѯ��ʽ */

/* ���¶���������MCS-51��Ƭ��,������Ƭ�������޸�,Ϊ���ṩC���Ե��ٶ���Ҫ�Ա���������Ż� */
#include <reg51.h>
unsigned char volatile xdata	CH375_CMD_PORT _at_ 0xBDF1;	/* CH375����˿ڵ�I/O��ַ */
unsigned char volatile xdata	CH375_DAT_PORT _at_ 0xBCF0;	/* CH375���ݶ˿ڵ�I/O��ַ */
sbit	CH375_INT_WIRE	=		0xB0^2;	/* P3.2, INT0, ����CH375��INT#����,���ڲ�ѯ�ж�״̬ */

/* ����Ϊͨ�õĵ�Ƭ��C���� */
#include <string.h>
#include <stdio.h>

/* ����CH375������뼰����״̬ */
#include "CH375INC.H"

/* ��ʱ2΢��,����ȷ */
void	delay2us( )
{
	unsigned char i;
	for ( i = 2; i != 0; i -- );
}

/* ��ʱ1΢��,����ȷ */
void	delay1us( )
{
	unsigned char i;
	for ( i = 1; i != 0; i -- );
}

/* �������� */

void ERROR() {
	while(1);
}

void CH375_WR_CMD_PORT( unsigned char cmd ) {  /* ��CH375������˿�д������,���ڲ�С��4uS,�����Ƭ���Ͽ�����ʱ */
	delay2us();
	CH375_CMD_PORT=cmd;
	delay2us();
}

void CH375_WR_DAT_PORT( unsigned char dat ) {  /* ��CH375�����ݶ˿�д������,���ڲ�С��1.5uS,�����Ƭ���Ͽ�����ʱ */
	CH375_DAT_PORT=dat;
	delay1us();  /* ��ΪMCS51��Ƭ����������ʵ����������ʱ */
}

unsigned char CH375_RD_DAT_PORT() {  /* ��CH375�����ݶ˿ڶ�������,���ڲ�С��1.5uS,�����Ƭ���Ͽ�����ʱ */
	delay1us();  /* ��ΪMCS51��Ƭ����������ʵ����������ʱ */
	return( CH375_DAT_PORT );
}

unsigned char wait_interrupt() {  /* �����˵ȴ��������, ���ز���״̬ */
	while( CH375_INT_WIRE );  /* ��ѯ�ȴ�CH375��������ж�(INT#�͵�ƽ) */
	CH375_WR_CMD_PORT( CMD_GET_STATUS );  /* ������������ж�, ��ȡ�ж�״̬ */
	return( CH375_RD_DAT_PORT() );
}

unsigned char endp6_mode, endp7_mode;

#define	TRUE	1
#define	FALSE	0
unsigned char set_usb_mode( unsigned char mode ) {  /* ����CH37X�Ĺ���ģʽ */
	unsigned char i;
	CH375_WR_CMD_PORT( CMD_SET_USB_MODE );
	CH375_WR_DAT_PORT( mode );
	endp6_mode=endp7_mode=0x80;  /* �����˸�λUSB����ͬ����־ */
	for( i=0; i!=100; i++ ) {  /* �ȴ�����ģʽ�������,������30uS */
		if ( CH375_RD_DAT_PORT()==CMD_RET_SUCCESS ) return( TRUE );  /* �ɹ� */
	}
	return( FALSE );  /* CH375����,����оƬ�ͺŴ����ߴ��ڴ��ڷ�ʽ���߲�֧�� */
}

/* ����ͬ�� */
/* USB������ͬ��ͨ���л�DATA0��DATA1ʵ��: ���豸��, CH372/CH375�����Զ��л�;
   ��������, ������SET_ENDP6��SET_ENDP7�������CH375�л�DATA0��DATA1.
   �����˵ĳ�����������ΪSET_ENDP6��SET_ENDP7�ֱ��ṩһ��ȫ�ֱ���,
   ��ʼֵ��Ϊ80H, ÿִ��һ�γɹ������λ6ȡ��, ÿִ��һ��ʧ��������临λΪ80H. */

void toggle_recv() {  /* �������ճɹ���,�л�DATA0��DATA1ʵ������ͬ�� */
	CH375_WR_CMD_PORT( CMD_SET_ENDP6 );
	CH375_WR_DAT_PORT( endp6_mode );
	endp6_mode^=0x40;
	delay2us();
}

void toggle_send() {  /* �������ͳɹ���,�л�DATA0��DATA1ʵ������ͬ�� */
	CH375_WR_CMD_PORT( CMD_SET_ENDP7 );
	CH375_WR_DAT_PORT( endp7_mode );
	endp7_mode^=0x40;
	delay2us();
}

unsigned char clr_stall6() {  /* ��������ʧ�ܺ�,��λ�豸�˵�����ͬ����DATA0 */
	CH375_WR_CMD_PORT( CMD_CLR_STALL );
	CH375_WR_DAT_PORT( 2 | 0x80 );  /* ����豸�˲���CH37XоƬ,��ô��Ҫ�޸Ķ˵�� */
	endp6_mode=0x80;
	return( wait_interrupt() );
}

unsigned char clr_stall7() {  /* ��������ʧ�ܺ�,��λ�豸�˵�����ͬ����DATA0 */
	CH375_WR_CMD_PORT( CMD_CLR_STALL );
	CH375_WR_DAT_PORT( 2 );  /* ����豸�˲���CH37XоƬ,��ô��Ҫ�޸Ķ˵�� */
	endp7_mode=0x80;
	return( wait_interrupt() );
}

/* ���ݶ�д, ��Ƭ����дCH372����CH375оƬ�е����ݻ����� */

unsigned char rd_usb_data( unsigned char *buf ) {  /* ��CH37X�������ݿ� */
	unsigned char i, len;
	CH375_WR_CMD_PORT( CMD_RD_USB_DATA );  /* ��CH375�Ķ˵㻺������ȡ���յ������� */
	len=CH375_RD_DAT_PORT();  /* �������ݳ��� */
	for ( i=0; i!=len; i++ ) *buf++=CH375_RD_DAT_PORT();
	return( len );
}

void wr_usb_data( unsigned char len, unsigned char *buf ) {  /* ��CH37Xд�����ݿ� */
	CH375_WR_CMD_PORT( CMD_WR_USB_DATA7 );  /* ��CH375�Ķ˵㻺����д��׼�����͵����� */
	CH375_WR_DAT_PORT( len );  /* �������ݳ���, len���ܴ���64 */
	while( len-- ) CH375_WR_DAT_PORT( *buf++ );
}

/* �������� */

unsigned char issue_token( unsigned char endp_and_pid ) {  /* ִ��USB���� */
/* ִ����ɺ�, �������ж�֪ͨ��Ƭ��, �����USB_INT_SUCCESS��˵�������ɹ� */
	unsigned char status;
	CH375_WR_CMD_PORT( CMD_ISSUE_TOKEN );
	CH375_WR_DAT_PORT( endp_and_pid );  /* ��4λĿ�Ķ˵��, ��4λ����PID */
	status=wait_interrupt();  /* �ȴ�CH375������� */
	if ( status!=USB_INT_SUCCESS && (endp_and_pid&0xF0)==0x20 ) {  /* ����ʧ��,����豸�˲���CH37XоƬ,��ô��Ҫ�޸Ķ˵�� */
		if ( (endp_and_pid&0x0F)==DEF_USB_PID_OUT ) clr_stall7();  /* ��λ�豸�˽��� */
		else if ( (endp_and_pid&0x0F)==DEF_USB_PID_IN ) clr_stall6();  /* ��λ�豸�˷��� */
	}
	return( status );
}

void host_send( unsigned char len, unsigned char *buf ) {  /* �������� */
	wr_usb_data( len, buf );
	toggle_send();
	if ( issue_token( ( 2 << 4 ) | DEF_USB_PID_OUT )!=USB_INT_SUCCESS ) ERROR();  /* ����豸�˲���CH37XоƬ,��ô��Ҫ�޸Ķ˵�� */
}

unsigned char host_recv( unsigned char *buf ) {  /* ��������, ���س��� */
	toggle_recv();
	if ( issue_token( ( 2 << 4 ) | DEF_USB_PID_IN )!=USB_INT_SUCCESS ) ERROR();  /* ����豸�˲���CH37XоƬ,��ô��Ҫ�޸Ķ˵�� */
	return( rd_usb_data( buf ) );
}

unsigned char get_descr( unsigned char type ) {  /* ���豸�˻�ȡ������ */
	unsigned char status;
	CH375_WR_CMD_PORT( CMD_GET_DESCR );
	CH375_WR_DAT_PORT( type );  /* ����������, ֻ֧��1(�豸)����2(����) */
	status=wait_interrupt();  /* �ȴ�CH375������� */
	if ( status==USB_INT_SUCCESS ) {  /* �����ɹ� */
		unsigned char buffer[64];
		unsigned char i, len;
		len=rd_usb_data( buffer );
		printf( "%s��������:", type==1?"�豸":"����" );
		for ( i=0; i!=len; i++ ) printf( "%02X ", buffer[i] );
		printf( "\n" );
	}
	return( status );
}

unsigned char set_addr( unsigned char addr ) {  /* �����豸�˵�USB��ַ */
	unsigned char status;
	CH375_WR_CMD_PORT( CMD_SET_ADDRESS );  /* ����USB�豸�˵�USB��ַ */
	CH375_WR_DAT_PORT( addr );  /* ��ַ, ��1��127֮�������ֵ, ����2��20 */
	status=wait_interrupt();  /* �ȴ�CH375������� */
	if ( status==USB_INT_SUCCESS ) {  /* �����ɹ� */
		CH375_WR_CMD_PORT( CMD_SET_USB_ADDR );  /* ����USB�����˵�USB��ַ */
		CH375_WR_DAT_PORT( addr );  /* ��Ŀ��USB�豸�ĵ�ַ�ɹ��޸ĺ�,Ӧ��ͬ���޸������˵�USB��ַ */
	}
	return( status );
}

unsigned char set_config( unsigned char cfg ) {  /* �����豸�˵�USB���� */
	endp6_mode=endp7_mode=0x80;  /* ��λUSB����ͬ����־ */
	CH375_WR_CMD_PORT( CMD_SET_CONFIG );  /* ����USB�豸�˵�����ֵ */
	CH375_WR_DAT_PORT( cfg );  /* ��ֵȡ��USB�豸�������������� */
	return( wait_interrupt() );  /* �ȴ�CH375������� */
}

/* �����˵��������ʾ�� */
main() {
	unsigned char xdata data_to_send[250], data_by_recv[250];  /* �շ������� */
	unsigned char i, len;
	set_usb_mode( 6 );  /* ����USB����ģʽ, ����豸����CH37X, ��ô5��6���� */
	while ( wait_interrupt()!=USB_INT_CONNECT );  /* �ȴ��豸���������� */

#ifdef DEVICE_NOT_CH37X
/* ����豸����CH37X,��ô���²����ǿ�ѡ��,
   ���������USBоƬ,��ô��Ҫִ�����²���,����Ҫ�������������������ݻ������ֵ�Լ��˵��,���޸ı������еĶ˵��,
   ������η���������������ο�CH375PRT.C�ļ� */
#define USB_RESET_FIRST	1  /* USB�淶��δҪ����USB�豸�������븴λ���豸,���Ǽ������WINDOWS����������,������ЩUSB�豸ҲҪ���ڲ��������ȸ�λ���ܹ��� */
#ifdef USB_RESET_FIRST
	set_usb_mode( 7 );  /* ��λUSB�豸,CH375��USB�ź��ߵ�D+��D-����͵�ƽ */
/* �����Ƭ����CH375��INT#���Ų����жϷ�ʽ�����ǲ�ѯ��ʽ,��ôӦ���ڸ���USB�豸�ڼ��ֹCH375�ж�,��USB�豸��λ��ɺ����CH375�жϱ�־�������ж� */
	for ( i=0; i<250; i++ ) { delay2us(); delay2us(); delay2us(); delay2us(); }  /* ��λʱ�䲻����1mS,����Ϊ10mS */
	set_usb_mode( 6 );  /* ������λ */
	while ( wait_interrupt()!=USB_INT_CONNECT );  /* �ȴ���λ֮����豸���ٴ��������� */
	for ( i=0; i<250; i++ ) delay2us();  /* ��ЩUSB�豸Ҫ����ʱ���ٺ������ܹ��� */
#endif
	get_descr(1);  /* ��ȡUSB�豸���豸������ */
	set_addr(5);  /* ����USB�豸�ĵ�ַ,��Ϊֻ��һ��USB�豸,���Կ��Է���1��126֮�������ֵ */
	get_descr(2);  /* ��ȡUSB�豸������������ */
	set_config(1);  /* ����USB����ֵ,����ֵ����USB�豸������������ */
#endif

	for ( i=0; i<250; i+=64 ) host_send( 64, &data_to_send[i] );  /* ����256�ֽڵ����ݸ��豸�� */
	host_send( 0, NULL );  /* �ٶ�, ���Ϳ����ݸ��豸�˾���֪ͨ�豸�˷������� */
	for ( i=0; i<250; i+=len ) len=host_recv( &data_by_recv[i] );  /* ���豸�˽���256�ֽڵ����� */
	/* �����ٴμ����������ݻ��߽������� */
	while(1);
}