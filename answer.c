/* ����ͷ�ļ� */
#include <ioCC2530.h>
#include "hal_defs.h"
#include <stdio.h>
#include <string.h>

/*�궨��*/

/*�������*/
uint8 flag_state = 0;//0��ʾδ��ʼ��⡢1��ʾ�ѿ�ʼ����Ҵ��ʹ����з���Ϊ������2��ʾ�ѿ�ʼ����Ҵ��ʹ����з���Ϊ������
uint8 flag_finish = 0;//0��ʾδ��ɼ�⣬1��ʾ�����1�μ��
float speed = 0;//�ٶ�ֵ

uint16 counter = 0; //ͳ�ƶ�ʱ���������
uint8 t1_l = 0;//�����洢��ʱ��T1����ֵ��8λ
uint8 t1_h = 0;//�����洢��ʱ��T1����ֵ��8λ
uint16 t1_value = 0;//�����洢��ʱ��T1����16λ����ֵ

char buff[128] = {'\0'};

/*��������*/
void InitCLK(void);//ϵͳʱ�ӳ�ʼ��������Ϊ32MHz
void InitTime1(void);//��ʱ��1��ʼ����������������������Ϊ1us�������������Ϊ50ms
void InitUart0(void);//����0��ʼ������
void UART0SendByte(char c);//UART0����һ���ֽں���
void UART0SendString(char *str);//UART0���������ַ���
void SendMsg(float value);//UART0���ʹ��ʹ��������������value�Ǵ��ʹ������ٶ�
/*���庯��*/
void InitCLK(void)
{
  CLKCONCMD &= 0x80;
  while(CLKCONSTA & 0x40);
}

void InitTime1(void)
{
  T1CC0L = 50000 & 0xff;
  T1CC0H = (50000 &0xff00)>>8;
  T1CCTL0 |= 0x04;//�趨��ʱ��1ͨ��0�Ƚ�ģʽ
  T1CTL = 0x08;//���ö�ʱ��1Ϊ32��Ƶ����ͣ����
  TIMIF &= ~0x40;//��������ʱ��1������ж�
  T1IE = 1;//ʹ�ܶ�ʱ��1�ж� 
}

void InitUart0(void)
{
  PERCFG = 0x00;	
  P0SEL = 0x3c;	
  U0CSR |= 0x80;
  U0BAUD = 216;
  U0GCR = 11;
  U0UCR |= 0x80;
  UTX0IF = 0;  // ����UART0 TX�жϱ�־ 
}

void UART0SendByte(char c)
{
  U0DBUF = c;// ��Ҫ���͵�1�ֽ�����д��U0DBUF
  while (!UTX0IF) ;// �ȴ�TX�жϱ�־����U0DBUF����
  UTX0IF = 0;// ����TX�жϱ�־
}

void UART0SendString(char *str)
{
  while(*str != '\0')
  {
    UART0SendByte(*str++);
  }
}

void SendMsg(float value)
{
  memset(buff,'\0',128);
  if(flag_state == 1)
  {
    sprintf(buff,"���ʹ��������У��ٶ�Ϊ%.3fm/s��\n",value);
  }
  if(flag_state == 2)
  {
    sprintf(buff,"���ʹ��������У��ٶ�Ϊ%.3fm/s��\n",value);
  }
  UART0SendString(buff);
}

/*������*/
void main(void)
{
  InitCLK();
  InitTime1();
  InitUart0();
  
  /*.......������1��ʼ����P1_3��P1_4������Ϊ����˿�...........*/
  P1DIR &= ~0x18;
  /*.......������1����...........*/
  
  /*.......������2��ʼ������P1_3��P1_4���ж�...........*/
  P1INP |= 0x18;//����P1_3��P1_4��Ϊ����̬��ģʽ
  PICTL |= 0x06;//����P1_3��P1_4���жϴ�����ʽΪ���½��ش���
  IEN2 |= 0x10;//ʹ��P1�˿��ж�
  P1IEN |= 0x18;//ʹ��P1_3��P1_4�˿��ж�
  /*.......������2����...........*/
  
  EA = 1;//ʹ�����ж�
  
  while(1)
  {
    if(flag_finish == 1)
    {
      /*..������3��ʼ�����㴫�ʹ��ٶȲ�ͨ�����ڷ�������..*/
      t1_l = T1CNTL;//��ȡ��ʱ��T1��������ֵ
      t1_h = T1CNTH;
      t1_value = t1_h;
      t1_value = t1_value<<8;
      t1_value |= t1_l;
      
      speed = 0.05/(0.05*counter+0.000001*t1_value);//���㴫�ʹ��ٶ�
      
      SendMsg(speed);//ͨ�����ڷ��ʹ��ʹ�״̬����
      
      flag_state = 0;//���ϵͳ�����йر�־λ
      flag_finish = 0;
      /*.......������3����...........*/
    }    
  }
}

/*�жϷ�����*/
#pragma vector = P1INT_VECTOR
__interrupt void P1_ISR(void)
{
  /*.......������4��ʼ��������䱻�ڵ�ʱ...........*/
  if(P1IF == 1)
  {
    if(P1IFG & 0x08)//�����P1_3�������жϣ����������1���ڵ�ʱ
    {
      if(flag_state == 0)//�����ǰδ��ʼ���
      {
        counter = 0;//����T1�����������
        T1CNTL = 1;//����T1������
        T1CTL |= 0x02;//������ʱ��T1��������ģģʽ
        flag_state = 1;//����flag_state��־λ����ʼ��⣬��������
      }
      
      if(flag_state == 2)//�����ǰ�����ڼ�⣬��Ϊ��������
      {
        T1CTL &= ~0x03;//ֹͣ��ʱ��T1
        flag_finish = 1;//������ɼ���־λ
      }

      P1IFG &= ~0x08;//���P1_3���жϱ�־λ
    }
    
    if(P1IFG & 0x10)//�����P1_4�������жϣ����������2���ڵ�ʱ
    {
      if(flag_state == 0)//�����ǰδ��ʼ���
      {
        counter = 0;//����T1�����������
        T1CNTL = 1;//����T1������
        T1CTL |= 0x02;//������ʱ��T1��������ģģʽ
        flag_state = 2;//����flag_state��־λ����ʼ��⣬��������
      }
      
      if(flag_state == 1)//�����ǰ�����ڼ�⣬��Ϊ��������
      {
        T1CTL &= ~0x03;//ֹͣ��ʱ��T1
        flag_finish = 1;//������ɼ���־λ
      }

      P1IFG &= ~0x10;//���P1_4���жϱ�־λ
    }
    
    P1IF = 0;//���P1���жϱ�־λ
  }
  /*.......������4����...........*/
}

#pragma vector = T1_VECTOR
__interrupt void T1_ISR(void)
{
  counter++;
  T1STAT &= ~0x01;  //���ͨ��0�жϱ�־
}