#include "lcd.h"
#include "lcd_font.h"

_lcd_dev lcddev;//给结构体重命名

//寄存器函数
void LCD_WR_REG(uint16_t reg)
{   
	LCD->LCD_REG=reg;//写入要写的寄存器序号
}

//写入的值
void LCD_WR_DATA(uint16_t data)
{	 
	LCD->LCD_RAM=data;	//把data给LCD_RAM	 
}
//读LCD数据
//返回值：读到的值
uint16_t LCD_RD_DATA(void)
{
	volatile uint16_t ram;			//用volatile防止被优化
	ram=LCD->LCD_RAM;	//把值给ram（这就是读取的过程）
	return ram;	 
}

//设置起始和结束的地址
/*
    x1，x2为起始地址和结束地址
    y1，y2为起始地址和结束地址
*/
void LCD_Address_Set(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2)
{
    LCD_WR_REG(0x2a);//列地址设置0x2a
    LCD_WR_DATA(x1>>8);
    LCD_WR_DATA(x1&0xff);
    LCD_WR_DATA(x2>>8);
    LCD_WR_DATA(x2&0xff);
    LCD_WR_REG(0x2b);//行地址设置0x2b
    LCD_WR_DATA(y1>>8);
    LCD_WR_DATA(y1&0xff);
    LCD_WR_DATA(y2>>8);
    LCD_WR_DATA(y2&0xff);
    LCD_WR_REG(0x2c);//储存器写0x2c
}
//设置光标（为下一个点做准备）
/*
    函数说明：设置光标位置
    入口数据：x,y 光标位置
*/
void LCD_SetCursor(uint16_t x,uint16_t y)
{
	LCD_WR_REG(0x2a);//列地址设置
	LCD_WR_DATA(x>>8);
	LCD_WR_DATA(x&0xff);
	LCD_WR_REG(0x2b);//行地址设置
	LCD_WR_DATA(y>>8);
	LCD_WR_DATA(y&0xff);
}
//开启亮屏
void LCD_DisplayOn(void)
{
	LCD_WR_REG(0X29);	//开启显示
}
//关闭屏幕
void LCD_DisplayOff(void)
{
	LCD_WR_REG(0X28);	//关闭显示
}  
//读取个某点的颜色值
/*
    x,y:坐标
    返回值:此点的颜色
*/
uint16_t LCD_ReadPoint(uint16_t x,uint16_t y)
{
 	uint16_t rgb;
	LCD_SetCursor(x,y);
	LCD_WR_REG(0X2E);
	rgb=LCD_RD_DATA();//dummy Read
	rgb=LCD_RD_DATA();//实际坐标颜色
	return rgb;
}
//设置显示颜色
void LCD_Clear(uint16_t color)
{          
	uint16_t i,j; 
	LCD_Address_Set(0,0,lcddev.width-1,lcddev.height-1);//设置显示范围
	for(i=0;i<lcddev.width;i++)
	{													   	 	
		for(j=0;j<lcddev.height;j++)
		{
			LCD_WR_DATA(color);
		}
	} 					  	    
}

/******************************************************************************
      函数说明：在指定区域填充颜色
      入口数据：xsta,ysta   起始坐标
                xend,yend   终止坐标
								color       要填充的颜色
      返回值：  无
******************************************************************************/
void LCD_Fill(uint16_t xsta,uint16_t ysta,uint16_t xend,uint16_t yend,uint16_t color)
{          
	uint16_t i,j; 
	LCD_Address_Set(xsta,ysta,xend-1,yend-1);//设置显示范围
	for(i=ysta;i<yend;i++)
	{													   	 	
		for(j=xsta;j<xend;j++)
		{
			LCD_WR_DATA(color);
		}
	} 					  	    
}
//设置坐标点的颜色
void LCD_DrawPoint(uint16_t x,uint16_t y,uint16_t color)
{
	LCD_Address_Set(x,y,x,y);//设置光标位置
	LCD_WR_DATA(color);
} 
//画线
void LCD_DrawLine(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2,uint16_t color)
{
	uint16_t t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance;
	int incx,incy,uRow,uCol;
	delta_x=x2-x1; //计算坐标增量
	delta_y=y2-y1;
	uRow=x1;//画线起点坐标
	uCol=y1;
	if(delta_x>0)incx=1; //设置单步方向
	else if (delta_x==0)incx=0;//垂直线
	else {incx=-1;delta_x=-delta_x;}
	if(delta_y>0)incy=1;
	else if (delta_y==0)incy=0;//水平线
	else {incy=-1;delta_y=-delta_y;}
	if(delta_x>delta_y)distance=delta_x; //选取基本增量坐标轴
	else distance=delta_y;
	for(t=0;t<distance+1;t++)
	{
		LCD_DrawPoint(uRow,uCol,color);//画点
		xerr+=delta_x;
		yerr+=delta_y;
		if(xerr>distance)
		{
			xerr-=distance;
			uRow+=incx;
		}
		if(yerr>distance)
		{
			yerr-=distance;
			uCol+=incy;
		}
	}
}

//画矩形
/*
      函数说明：画矩形
      入口数据：x1,y1   起始坐标
                x2,y2   终止坐标
                color   矩形的颜色
      返回值：  无
*/
void LCD_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,uint16_t color)
{
	LCD_DrawLine(x1,y1,x2,y1,color);
	LCD_DrawLine(x1,y1,x1,y2,color);
	LCD_DrawLine(x1,y2,x2,y2,color);
	LCD_DrawLine(x2,y1,x2,y2,color);
}

//画圆
/*
      函数说明：画圆
      入口数据：x0,y0   圆心坐标
                r       半径
                color   圆的颜色
      返回值：  无
*/
void Draw_Circle(uint16_t x0,uint16_t y0,uint8_t r,uint16_t color)
{
	int a,b;
	a=0;b=r;	  
	while(a<=b)
	{
		LCD_DrawPoint(x0-b,y0-a,color);             //3           
		LCD_DrawPoint(x0+b,y0-a,color);             //0           
		LCD_DrawPoint(x0-a,y0+b,color);             //1                
		LCD_DrawPoint(x0-a,y0-b,color);             //2             
		LCD_DrawPoint(x0+b,y0+a,color);             //4               
		LCD_DrawPoint(x0+a,y0-b,color);             //5
		LCD_DrawPoint(x0+a,y0+b,color);             //6 
		LCD_DrawPoint(x0-b,y0+a,color);             //7
		a++;
		if((a*a+b*b)>(r*r))//判断要画的点是否过远
		{
			b--;
		}
	}
}
//显示中文字符串
/*
      函数说明：显示汉字串
      入口数据：x,y显示坐标
                *s 要显示的汉字串
                fc 字的颜色
                bc 字的背景色
                sizey 字号 可选 16 24 32
                mode:  0非叠加模式  1叠加模式
      返回值：  无
*/
void LCD_ShowChinese(uint16_t x,uint16_t y,uint8_t *s,uint16_t fc,uint16_t bc,uint8_t sizey,uint8_t mode)
{
	while(*s!=0)
	{
		if(sizey==12) LCD_ShowChinese12x12(x,y,s,fc,bc,sizey,mode);
		else if(sizey==16) LCD_ShowChinese16x16(x,y,s,fc,bc,sizey,mode);
		else if(sizey==24) LCD_ShowChinese24x24(x,y,s,fc,bc,sizey,mode);
		else if(sizey==32) LCD_ShowChinese32x32(x,y,s,fc,bc,sizey,mode);
		else return;
		if((*s & 0x80) == 0) s+=1; else s+=2;  // GB2312: ASCII=1字节, 汉字=2字节
		x+=sizey;
	}
}
//显示单汉字
/*
      函数说明：显示单个12x12汉字
      入口数据：x,y显示坐标
                *s 要显示的汉字
                fc 字的颜色
                bc 字的背景色
                sizey 字号
                mode:  0非叠加模式  1叠加模式
      返回值：  无
*/
void LCD_ShowChinese12x12(uint16_t x,uint16_t y,uint8_t *s,uint16_t fc,uint16_t bc,uint8_t sizey,uint8_t mode)//12x12
{
	uint8_t i,j,m=0;
	uint16_t k;
	uint16_t HZnum;//汉字数目
	uint16_t TypefaceNum;//一个字符所占字节大小
	uint16_t x0=x;
	TypefaceNum=(sizey/8+((sizey%8)?1:0))*sizey;
	                         
	HZnum=sizeof(tfont12)/sizeof(typFNT_GB12);	//统计汉字数目
	for(k=0;k<HZnum;k++) 
	{
		if((tfont12[k].Index[0]==*(s))&&(tfont12[k].Index[1]==*(s+1)))
		{ 	
			LCD_Address_Set(x,y,x+sizey-1,y+sizey-1);
			for(i=0;i<TypefaceNum;i++)
			{
				for(j=0;j<8;j++)
				{	
					if(!mode)//非叠加方式
					{
						if(tfont12[k].Msk[i]&(0x01<<j))LCD_WR_DATA(fc);
						else LCD_WR_DATA(bc);
						m++;
						if(m%sizey==0)
						{
							m=0;
							break;
						}
					}
					else//叠加方式
					{
						if(tfont12[k].Msk[i]&(0x01<<j))	LCD_DrawPoint(x,y,fc);//画一个点
						x++;
						if((x-x0)==sizey)
						{
							x=x0;
							y++;
							break;
						}
					}
				}
			}
		}				  	
		continue;  //查找到对应点阵字库立即退出，防止多个汉字重复取模带来影响
	}
} 

void LCD_ShowChinese16x16(uint16_t x,uint16_t y,uint8_t *s,uint16_t fc,uint16_t bc,uint8_t sizey,uint8_t mode)//16x16
{
	uint8_t i,j,m=0;
	uint16_t k;
	uint16_t HZnum;//汉字数目
	uint16_t TypefaceNum;//一个字符所占字节大小
	uint16_t x0=x;
    TypefaceNum=(sizey/8+((sizey%8)?1:0))*sizey;
	HZnum=sizeof(tfont16)/sizeof(typFNT_GB16);	//统计汉字数目
	for(k=0;k<HZnum;k++) 
	{
		if ((tfont16[k].Index[0]==*(s))&&(tfont16[k].Index[1]==*(s+1)))
		{ 	
			LCD_Address_Set(x,y,x+sizey-1,y+sizey-1);
			for(i=0;i<TypefaceNum;i++)
			{
				for(j=0;j<8;j++)
				{	
					if(!mode)//非叠加方式
					{
						if(tfont16[k].Msk[i]&(0x01<<j))LCD_WR_DATA(fc);
						else LCD_WR_DATA(bc);
						m++;
						if(m%sizey==0)
						{
							m=0;
							break;
						}
					}
					else//叠加方式
					{
						if(tfont16[k].Msk[i]&(0x01<<j))	LCD_DrawPoint(x,y,fc);//画一个点
						x++;
						if((x-x0)==sizey)
						{
							x=x0;
							y++;
							break;
						}
					}
				}
			}
		}				  	
		continue;  //查找到对应点阵字库立即退出，防止多个汉字重复取模带来影响
	}
} 

void LCD_ShowChinese24x24(uint16_t x,uint16_t y,uint8_t *s,uint16_t fc,uint16_t bc,uint8_t sizey,uint8_t mode)//24x24
{
	uint8_t i,j,m=0;
	uint16_t k;
	uint16_t HZnum;//汉字数目
	uint16_t TypefaceNum;//一个字符所占字节大小
	uint16_t x0=x;
	TypefaceNum=(sizey/8+((sizey%8)?1:0))*sizey;
	HZnum=sizeof(tfont24)/sizeof(typFNT_GB24);	//统计汉字数目
	for(k=0;k<HZnum;k++) 
	{
		if ((tfont24[k].Index[0]==*(s))&&(tfont24[k].Index[1]==*(s+1)))
		{ 	
			LCD_Address_Set(x,y,x+sizey-1,y+sizey-1);
			for(i=0;i<TypefaceNum;i++)
			{
				for(j=0;j<8;j++)
				{	
					if(!mode)//非叠加方式
					{
						if(tfont24[k].Msk[i]&(0x01<<j))LCD_WR_DATA(fc);
						else LCD_WR_DATA(bc);
						m++;
						if(m%sizey==0)
						{
							m=0;
							break;
						}
					}
					else//叠加方式
					{
						if(tfont24[k].Msk[i]&(0x01<<j))	LCD_DrawPoint(x,y,fc);//画一个点
						x++;
						if((x-x0)==sizey)
						{
							x=x0;
							y++;
							break;
						}
					}
				}
			}
		}				  	
		continue;  //查找到对应点阵字库立即退出，防止多个汉字重复取模带来影响
	}
} 

void LCD_ShowChinese32x32(uint16_t x,uint16_t y,uint8_t *s,uint16_t fc,uint16_t bc,uint8_t sizey,uint8_t mode)//32x32
{
	uint8_t i,j,m=0;
	uint16_t k;
	uint16_t HZnum;//汉字数目
	uint16_t TypefaceNum;//一个字符所占字节大小
	uint16_t x0=x;
	TypefaceNum=(sizey/8+((sizey%8)?1:0))*sizey;
	HZnum=sizeof(tfont32)/sizeof(typFNT_GB32);	//统计汉字数目
	for(k=0;k<HZnum;k++) 
	{
		if ((tfont32[k].Index[0]==*(s))&&(tfont32[k].Index[1]==*(s+1)))
		{ 	
			LCD_Address_Set(x,y,x+sizey-1,y+sizey-1);
			for(i=0;i<TypefaceNum;i++)
			{
				for(j=0;j<8;j++)
				{	
					if(!mode)//非叠加方式
					{
						if(tfont32[k].Msk[i]&(0x01<<j))LCD_WR_DATA(fc);
						else LCD_WR_DATA(bc);
						m++;
						if(m%sizey==0)
						{
							m=0;
							break;
						}
					}
					else//叠加方式
					{
						if(tfont32[k].Msk[i]&(0x01<<j))	LCD_DrawPoint(x,y,fc);//画一个点
						x++;
						if((x-x0)==sizey)
						{
							x=x0;
							y++;
							break;
						}
					}
				}
			}
		}				  	
		continue;  //查找到对应点阵字库立即退出，防止多个汉字重复取模带来影响
	}
}
//显示单个字符
/*
      入口数据：x,y显示坐标
                num 要显示的字符
                fc 字的颜色
                bc 字的背景色
                sizey 字号
                mode:  0非叠加模式  1叠加模式
      返回值：  无
*/
void LCD_ShowChar(uint16_t x,uint16_t y,uint8_t num,uint16_t fc,uint16_t bc,uint8_t sizey,uint8_t mode)
{
	uint8_t temp,sizex,t,m=0;
	uint16_t i,TypefaceNum;//一个字符所占字节大小
	uint16_t x0=x;
	sizex=sizey/2;
	TypefaceNum=(sizex/8+((sizex%8)?1:0))*sizey;
	num=num-' ';    //得到偏移后的值
	LCD_Address_Set(x,y,x+sizex-1,y+sizey-1);  //设置光标位置
	for(i=0;i<TypefaceNum;i++)
	{ 
		if(sizey==12)temp=ascii_1206[num][i];		       //调用6x12字体
		else if(sizey==16)temp=ascii_1608[num][i];		 //调用8x16字体
		else if(sizey==24)temp=ascii_2412[num][i];		 //调用12x24字体
		else if(sizey==32)temp=ascii_3216[num][i];		 //调用16x32字体
		else return;
		for(t=0;t<8;t++)
		{
			if(!mode)//非叠加模式
			{
				if(temp&(0x01<<t))LCD_WR_DATA(fc);
				else LCD_WR_DATA(bc);
				m++;
				if(m%sizex==0)
				{
					m=0;
					break;
				}
			}
			else//叠加模式
			{
				if(temp&(0x01<<t))LCD_DrawPoint(x,y,fc);//画一个点
				x++;
				if((x-x0)==sizex)
				{
					x=x0;
					y++;
					break;
				}
			}
		}
	}   	 	  
}

//显示字符串
void LCD_ShowString(uint16_t x,uint16_t y,const uint8_t *p,uint16_t fc,uint16_t bc,uint8_t sizey,uint8_t mode)
{         
	while(*p!='\0')
	{       
		LCD_ShowChar(x,y,*p,fc,bc,sizey,mode);
		x+=sizey/2;
		p++;
	}  
}
/******************************************************************************
      函数说明：显示数字
      入口数据：m底数，n指数
      返回值：  无
******************************************************************************/
uint32_t mypow(uint8_t m,uint8_t n)
{
	uint32_t result=1;	 
	while(n--)result*=m;
	return result;
}

//显示整数变量
/*
      函数说明：显示整数变量
      入口数据：x,y显示坐标
                num 要显示整数变量
                len 要显示的位数
                fc 字的颜色
                bc 字的背景色
                sizey 字号
      返回值：  无
*/
void LCD_ShowIntNum(uint16_t x,uint16_t y,uint16_t num,uint8_t len,uint16_t fc,uint16_t bc,uint8_t sizey)
{         	
	uint8_t t,temp;
	uint8_t enshow=0;
	uint8_t sizex=sizey/2;
	for(t=0;t<len;t++)
	{
		temp=(num/mypow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1))
		{
			if(temp==0)
			{
				LCD_ShowChar(x+t*sizex,y,' ',fc,bc,sizey,0);
				continue;
			}else enshow=1; 
		 	 
		}
	 	LCD_ShowChar(x+t*sizex,y,temp+48,fc,bc,sizey,0);
	}
} 

/******************************************************************************
      函数说明：显示两位小数变量
      入口数据：x,y显示坐标
                num 要显示小数变量
                len 要显示的位数
                fc 字的颜色
                bc 字的背景色
                sizey 字号
      返回值：  无
******************************************************************************/
void LCD_ShowFloatNum1(uint16_t x,uint16_t y,float num,uint8_t len,uint16_t fc,uint16_t bc,uint8_t sizey)
{         	
	uint8_t t,temp,sizex;
	uint16_t num1;
	sizex=sizey/2;
	num1=num*100;
	for(t=0;t<len;t++)
	{
		temp=(num1/mypow(10,len-t-1))%10;
		if(t==(len-2))
		{
			LCD_ShowChar(x+(len-2)*sizex,y,'.',fc,bc,sizey,0);
			t++;
			len+=1;
		}
	 	LCD_ShowChar(x+t*sizex,y,temp+48,fc,bc,sizey,0);
	}
}

/******************************************************************************
      函数说明：显示图片
      入口数据：x,y起点坐标
                length 图片长度
                width  图片宽度
                pic[]  图片数组
      返回值：  无
******************************************************************************/
void LCD_ShowPicture(uint16_t x,uint16_t y,uint16_t length,uint16_t width,const uint8_t pic[])
{
	uint8_t picH,picL;
	uint16_t i,j;
	uint32_t k=0;
	LCD_Address_Set(x,y,x+length-1,y+width-1);
	for(i=0;i<length;i++)
	{
		for(j=0;j<width;j++)
		{
			picH=pic[k*2];
			picL=pic[k*2+1];
			LCD_WR_DATA(picH<<8|picL);
			k++;
		}
	}			
}

void LCD_Init(void)
{                   
    GPIO_InitTypeDef          GPIO_InitStructure = {0};
    FMC_NORSRAMInitTypeDef    FMC_NORSRAMInitStructure = {0};
    FMC_NORSRAMTimingInitTypeDef  readWriteTiming = {0};
    FMC_NORSRAMTimingInitTypeDef  writeTiming = {0};

    // 1. 开启时钟
    RCC_HBPeriphClockCmd(RCC_HBPeriph_FMC, ENABLE);
    RCC_HB2PeriphClockCmd(RCC_HB2Periph_AFIO | RCC_HB2Periph_GPIOC | RCC_HB2Periph_GPIOD | RCC_HB2Periph_GPIOE | RCC_HB2Periph_GPIOF, ENABLE);

    // 2. LCD 背光引脚配置 (PD3)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Very_High;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    GPIO_ResetBits(GPIOD, GPIO_Pin_3); // 初始时先关闭背光，防止看到白屏/闪烁

    // 3. LCD 复位引脚配置 (PF0)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Very_High;
    GPIO_Init(GPIOF, &GPIO_InitStructure);
    GPIO_SetBits(GPIOF, GPIO_Pin_0);

    // 4. LCD 片选引脚配置 (PC12)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Very_High;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    GPIO_ResetBits(GPIOC, GPIO_Pin_12); // 选中LCD芯片

    // 5. FMC GPIO 复用配置 (PC0-PC3, PC6)
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource0, GPIO_AF4);
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource1, GPIO_AF4);
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource2, GPIO_AF4);
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource3, GPIO_AF4);
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF4);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Very_High;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    // 6. FMC GPIO 复用配置 (PD0, PD1, PD4, PD5, PD8-PD11, PD14, PD15)
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource0,  GPIO_AF12);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource1,  GPIO_AF12);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource4,  GPIO_AF12);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource5,  GPIO_AF12);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource8,  GPIO_AF12);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource9,  GPIO_AF12);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource10, GPIO_AF12);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource11, GPIO_AF12);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource14, GPIO_AF12);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource15, GPIO_AF12);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5 | 
                                  GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | 
                                  GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    // 7. FMC GPIO 复用配置 (PE12-PE15)
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource12, GPIO_AF12);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource13, GPIO_AF12);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource14, GPIO_AF12);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource15, GPIO_AF12);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    // 8. FMC 读时序配置
    readWriteTiming.FMC_AddressSetupTime       = 0x02; // 略微增加地址建立时间，提高高频稳定性
    readWriteTiming.FMC_AddressHoldTime        = 0x00;
    readWriteTiming.FMC_DataSetupTime          = 0x0F;
    readWriteTiming.FMC_BusTurnAroundDuration  = 0x00;
    readWriteTiming.FMC_CLKDivision            = 0x00;
    readWriteTiming.FMC_DataLatency            = 0x00;
    readWriteTiming.FMC_AccessMode             = FMC_AccessMode_A;

    // 9. FMC 写时序配置
    writeTiming.FMC_AddressSetupTime      = 0x01; 
    writeTiming.FMC_AddressHoldTime       = 0x00;
    writeTiming.FMC_DataSetupTime         = 0x05; // 稍作放宽（原0x03），防刷屏太快导致的硬件概率性丢步
    writeTiming.FMC_BusTurnAroundDuration = 0x00;
    writeTiming.FMC_CLKDivision           = 0x00;
    writeTiming.FMC_DataLatency           = 0x00;
    writeTiming.FMC_AccessMode            = FMC_AccessMode_A;

    // 10. FMC 初始化
    FMC_NORSRAMInitStructure.FMC_Bank                     = FMC_Bank1_NORSRAM1;
    FMC_NORSRAMInitStructure.FMC_DataAddressMux          = FMC_DataAddressMux_Disable;
    FMC_NORSRAMInitStructure.FMC_MemoryType              = FMC_MemoryType_SRAM;
    FMC_NORSRAMInitStructure.FMC_MemoryDataWidth         = FMC_MemoryDataWidth_16b;
    FMC_NORSRAMInitStructure.FMC_BurstAccessMode         = FMC_BurstAccessMode_Disable;
    FMC_NORSRAMInitStructure.FMC_WaitSignalPolarity      = FMC_WaitSignalPolarity_Low;
    FMC_NORSRAMInitStructure.FMC_AsynchronousWait        = FMC_AsynchronousWait_Disable;
    FMC_NORSRAMInitStructure.FMC_WaitSignalActive        = FMC_WaitSignalActive_BeforeWaitState;
    FMC_NORSRAMInitStructure.FMC_WriteOperation          = FMC_WriteOperation_Enable;
    FMC_NORSRAMInitStructure.FMC_WaitSignal              = FMC_WaitSignal_Disable;
    FMC_NORSRAMInitStructure.FMC_ExtendedMode            = FMC_ExtendedMode_Enable;
    FMC_NORSRAMInitStructure.FMC_WriteBurst              = FMC_WriteBurst_Disable;
    FMC_NORSRAMInitStructure.FMC_ReadWriteTimingStruct   = &readWriteTiming;
    FMC_NORSRAMInitStructure.FMC_WriteTimingStruct       = &writeTiming;
    FMC_NORSRAMInitStructure.FMC_CPSIZE                  = FMC_CPSIZE_None;
    FMC_NORSRAMInitStructure.FMC_BMP                     = FMC_BMP_Mode0;

    FMC_NORSRAMInit(&FMC_NORSRAMInitStructure);
    FMC_NORSRAMCmd(FMC_Bank1_NORSRAM1, ENABLE);

    // 11. 优化后的 LCD 硬件复位时序（节省约 50ms）
    GPIO_ResetBits(GPIOF, GPIO_Pin_0);
    Delay_Ms(2);  // ST7796 Reset low pulse max require 10us, 2ms is fully enough
    GPIO_SetBits(GPIOF, GPIO_Pin_0);
    Delay_Ms(10); // Wait for reset stable

    // 12. 读取 LCD ID
    Set_Dir(DFT_SCAN_DIR);
    LCD_WR_REG(0XD3);
    lcddev.id = LCD_RD_DATA();   // Dummy read
    lcddev.id = LCD_RD_DATA();   // Read 0x00
    lcddev.id = LCD_RD_DATA();   // Read 0x77
    lcddev.id <<= 8;
    lcddev.id |= LCD_RD_DATA();  // Read 0x96

    printf("LCD ID = 0x%x\r\n", lcddev.id);

    // 13. ST7796 寄存器配置序列
    LCD_WR_REG(0x11);            // Sleep out
    Delay_Ms(120);           // 此处必须等待 120ms 以让电源稳定

    LCD_WR_REG(0Xf0); LCD_WR_DATA(0xc3);
    LCD_WR_REG(0Xf0); LCD_WR_DATA(0x96);
    
    LCD_WR_REG(0x36);            // Memory Access Control
    LCD_WR_DATA(0x08 | DFT_SCAN_DIR);

    LCD_WR_REG(0x3A); LCD_WR_DATA(0x05); // 16-bit color (RGB565)

    LCD_WR_REG(0Xe8);
    LCD_WR_DATA(0x40); LCD_WR_DATA(0x82); LCD_WR_DATA(0x07); LCD_WR_DATA(0x18);
    LCD_WR_DATA(0x27); LCD_WR_DATA(0x0a); LCD_WR_DATA(0xb6); LCD_WR_DATA(0x33);

    LCD_WR_REG(0Xc5); LCD_WR_DATA(0x27);
    LCD_WR_REG(0Xc2); LCD_WR_DATA(0xa7);

    // Gamma 伽马校正设置
    LCD_WR_REG(0Xe0);
    LCD_WR_DATA(0xf0); LCD_WR_DATA(0x01); LCD_WR_DATA(0x06); LCD_WR_DATA(0x0f);
    LCD_WR_DATA(0x12); LCD_WR_DATA(0x1d); LCD_WR_DATA(0x36); LCD_WR_DATA(0x54);
    LCD_WR_DATA(0x44); LCD_WR_DATA(0x0c); LCD_WR_DATA(0x18); LCD_WR_DATA(0x16);
    LCD_WR_DATA(0x13); LCD_WR_DATA(0x15);

    LCD_WR_REG(0Xe1);
    LCD_WR_DATA(0xf0); LCD_WR_DATA(0x01); LCD_WR_DATA(0x05); LCD_WR_DATA(0x0a);
    LCD_WR_DATA(0x0b); LCD_WR_DATA(0x07); LCD_WR_DATA(0x32); LCD_WR_DATA(0x44);
    LCD_WR_DATA(0x44); LCD_WR_DATA(0x0c); LCD_WR_DATA(0x18); LCD_WR_DATA(0x17);
    LCD_WR_DATA(0x13); LCD_WR_DATA(0x16);

    LCD_WR_REG(0Xf0); LCD_WR_DATA(0x3c);
    LCD_WR_REG(0Xf0); LCD_WR_DATA(0x69);

    LCD_WR_REG(0X29);            // Display ON
    
    // 14. 视觉优化：先清屏填充背景色，再开背光，避免开机白屏瞬闪
    LCD_Clear(RED);
    GPIO_SetBits(GPIOD, GPIO_Pin_3); // 正式开启背光
}
void Set_Dir(uint8_t dir)
{
	if((dir>>4)%4)
	{
		lcddev.width=480;
		lcddev.height=320;
	}else
	{
		lcddev.width=320;
		lcddev.height=480;
	}
}
