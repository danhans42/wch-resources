/* ���³����ѡ��LPC2114�����׼� 2004.07 */
/* lpc21xx�������ֵ�ARM��Ŀ�������Ĵ��룬�����쳣���������Ŀ����ʼ������ */

#include "LPC2294.H"

/*     �����ӵ�����             */
/* ϵͳ����, Fosc��Fcclk��Fcco��Fpclk���붨��*/
#define Fosc            11059200                    //����Ƶ��,10MHz~25MHz��Ӧ����ʵ��һ��
#define Fcclk           (Fosc * 4)                  //ϵͳƵ�ʣ�����ΪFosc��������(1~32)����<=60MHZ
#define Fcco            (Fcclk * 4)                 //CCOƵ�ʣ�����ΪFcclk��2��4��8��16������ΧΪ156MHz~320MHz
#define Fpclk           (Fcclk / 4) * 1             //VPBʱ��Ƶ�ʣ�ֻ��Ϊ(Fcclk / 4)��1��2��4��


void	__irq	IRQ_Exception( void )				/* �ж��쳣���������û�������Ҫ�Լ��ı���� */
{
    while(1);                   // ��һ���滻Ϊ�Լ��Ĵ���
}

void	FIQ_Exception( void )						/* �����ж��쳣���������û�������Ҫ�Լ��ı���� */
{
    while(1);                   // ��һ���滻Ϊ�Լ��Ĵ���
}

void	TargetResetInit( void )						/* ����main����ǰĿ����ʼ�����룬������Ҫ�ı䣬����ɾ�� */
{
    MEMMAP = 0x1;                   //remap
    /* ����ϵͳ������ʱ�� */
    PLLCON = 1;
#if (Fpclk / (Fcclk / 4)) == 1
    VPBDIV = 0;
#endif
#if (Fpclk / (Fcclk / 4)) == 2
    VPBDIV = 2;
#endif
#if (Fpclk / (Fcclk / 4)) == 4
    VPBDIV = 1;
#endif
#if (Fcco / Fcclk) == 2
    PLLCFG = ((Fcclk / Fosc) - 1) | (0 << 5);
#endif
#if (Fcco / Fcclk) == 4
    PLLCFG = ((Fcclk / Fosc) - 1) | (1 << 5);
#endif
#if (Fcco / Fcclk) == 8
    PLLCFG = ((Fcclk / Fosc) - 1) | (2 << 5);
#endif
#if (Fcco / Fcclk) == 16
    PLLCFG = ((Fcclk / Fosc) - 1) | (3 << 5);
#endif
    PLLFEED = 0xaa;
    PLLFEED = 0x55;
    while((PLLSTAT & (1 << 10)) == 0);
    PLLCON = 3;
    PLLFEED = 0xaa;
    PLLFEED = 0x55;
    /* ���ô洢������ģ�� */
    MAMCR = 0;
#if Fcclk < 20000000
    MAMTIM = 1;
#else
#if Fcclk < 40000000
    MAMTIM = 2;
#else
    MAMTIM = 3;
#endif
#endif
    MAMCR = 2;  
    /* ��ʼ��VIC */
    VICIntEnClr = 0xffffffff;
    VICVectAddr = 0;
    VICIntSelect = 0;
    /* �����Լ��Ĵ��� */
}