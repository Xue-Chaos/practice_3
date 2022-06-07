/* 包含头文件 */
#include <ioCC2530.h>
#include "hal_defs.h"
#include <stdio.h>
#include <string.h>

/*宏定义*/

/*定义变量*/
uint8 flag_state = 0;//0表示未开始检测、1表示已开始检测且传送带运行方向为正方向、2表示已开始检测且传送带运行方向为反方向
uint8 flag_finish = 0;//0表示未完成检测，1表示完成了1次检测
float speed = 0;//速度值

uint16 counter = 0; //统计定时器溢出次数
uint8 t1_l = 0;//用来存储定时器T1计数值低8位
uint8 t1_h = 0;//用来存储定时器T1计数值高8位
uint16 t1_value = 0;//用来存储定时器T1整个16位计数值

char buff[128] = {'\0'};

/*声明函数*/
void InitCLK(void);//系统时钟初始化函数，为32MHz
void InitTime1(void);//定时器1初始化函数，计数器计数周期为1us，数据溢出周期为50ms
void InitUart0(void);//串口0初始化函数
void UART0SendByte(char c);//UART0发送一个字节函数
void UART0SendString(char *str);//UART0发送整个字符串
void SendMsg(float value);//UART0发送传送带运行情况，参数value是传送带运行速度
/*定义函数*/
void InitCLK(void)
{
  CLKCONCMD &= 0x80;
  while(CLKCONSTA & 0x40);
}

void InitTime1(void)
{
  T1CC0L = 50000 & 0xff;
  T1CC0H = (50000 &0xff00)>>8;
  T1CCTL0 |= 0x04;//设定定时器1通道0比较模式
  T1CTL = 0x08;//设置定时器1为32分频、暂停运行
  TIMIF &= ~0x40;//不产生定时器1的溢出中断
  T1IE = 1;//使能定时器1中断 
}

void InitUart0(void)
{
  PERCFG = 0x00;	
  P0SEL = 0x3c;	
  U0CSR |= 0x80;
  U0BAUD = 216;
  U0GCR = 11;
  U0UCR |= 0x80;
  UTX0IF = 0;  // 清零UART0 TX中断标志 
}

void UART0SendByte(char c)
{
  U0DBUF = c;// 将要发送的1字节数据写入U0DBUF
  while (!UTX0IF) ;// 等待TX中断标志，即U0DBUF就绪
  UTX0IF = 0;// 清零TX中断标志
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
    sprintf(buff,"传送带正向运行，速度为%.3fm/s。\n",value);
  }
  if(flag_state == 2)
  {
    sprintf(buff,"传送带反向运行，速度为%.3fm/s。\n",value);
  }
  UART0SendString(buff);
}

/*主函数*/
void main(void)
{
  InitCLK();
  InitTime1();
  InitUart0();
  
  /*.......答题区1开始：将P1_3和P1_4口设置为输入端口...........*/
  
  /*.......答题区1结束...........*/
  
  /*.......答题区2开始：配置P1_3和P1_4口中断...........*/
  //设置P1_3和P1_4口为“三态”模式
  //设置P1_3和P1_4口中断触发方式为：下降沿触发
  //使能P1端口中断
  //使能P1_3和P1_4端口中断
  /*.......答题区2结束...........*/
  
  EA = 1;//使能总中断
  
  while(1)
  {
    if(flag_finish == 1)
    {
      /*..答题区3开始：计算传送带速度并通过串口发送数据..*/
      //读取定时器T1计数器的值
      
      t1_value = t1_h;
      t1_value = t1_value<<8;
      t1_value |= t1_l;
      
      //计算传送带速度
      
      //通过串口发送传送带状态数据
      
      /*.......答题区3结束...........*/

      flag_state = 0;//清除系统运行有关标志位
      flag_finish = 0;
      
    }    
  }
}

/*中断服务函数*/
#pragma vector = P1INT_VECTOR
__interrupt void P1_ISR(void)
{
  /*.......答题区4开始：红外对射2被遮挡时...........*/
  if(P1IF == 1)
  {
    if(P1IFG & 0x08)//如果是P1_3触发的中断，即红外对射1被遮挡时
    {
      if(flag_state == 0)//如果当前未开始检测
      {
        //清零T1溢出次数计数
        //清零T1计数器
        //启动定时器T1，工作在模模式
        //设置flag_state标志位，开始检测，正向运行
      }
      
      if(flag_state == 2)//如果当前是正在检测，且为反向运行
      {
        //停止定时器T1
        //设置完成检测标志位
      }

      P1IFG &= ~0x08;//清除P1_3口中断标志位
    }
    
    if(P1IFG & 0x10)//如果是P1_4触发的中断，即红外对射2被遮挡时
    {
      if(flag_state == 0)//如果当前未开始检测
      {
        //清零T1溢出次数计数
        //清零T1计数器
        //启动定时器T1，工作在模模式
        //设置flag_state标志位，开始检测，反向运行
      }
      
      if(flag_state == 1)//如果当前是正在检测，且为正向运行
      {
        //停止定时器T1
        //设置完成检测标志位
      }

      P1IFG &= ~0x10;//清除P1_4口中断标志位
    }
    
    P1IF = 0;//清除P1口中断标志位
  }
  /*.......答题区4结束...........*/
}

#pragma vector = T1_VECTOR
__interrupt void T1_ISR(void)
{
  counter++;
  T1STAT &= ~0x01;  //清除通道0中断标志
}