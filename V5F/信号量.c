#include "rtthread.h"
#include "debug.h"

//创建一个信号量(用于控制小灯)
static rt_sem_t led_sem = RT_NULL;

//创建点灯线程
static void led_thread_entry(void *parameter){
    (void)parameter;
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    RCC_HB2PeriphClockCmd(RCC_HB2Periph_GPIOC,ENABLE);
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Very_High;
    GPIO_Init(GPIOC,&GPIO_InitStructure);

    while(1)
    {
        rt_sem_take(led_sem,RT_WAITING_FOREVER);
        GPIO_ResetBits(GPIOC,GPIO_Pin_7);
        rt_thread_mdelay(500);
        GPIO_SetBits(GPIOC,GPIO_Pin_7);
    }
}   
//创建心跳打印线程
static void heartbeat_thread_entry(void *paramter){
    (void)paramter;
    while(1){
        rt_tick_t tick = rt_tick_get();

        rt_kprintf("系统时间: %d\r\n",tick);
        rt_thread_mdelay(1000);
    }
}
//创建按键控制线程
static void key_thread_entry(void *parameter){
    (void)parameter;
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    RCC_HB2PeriphClockCmd(RCC_HB2Periph_GPIOA,ENABLE);
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Very_High;
    GPIO_Init(GPIOA,&GPIO_InitStructure);

    //按键检测逻辑
    while (1) {
        if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_8) == RESET){
            rt_kprintf("按键按下\r\n");
            //释放信号量发送给led控制
            rt_sem_release(led_sem);

            while (GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_8) == RESET) {
                rt_thread_mdelay(10);
            }
        }
        //50ms扫描一次按键状态
        rt_thread_mdelay(50);
    }
}
//宏定义栈大小
#define LED_THREAD_STACK_SIZE 512
#define HEARTBEAT_THREAD_STACK_SIZE 512
#define KEY_THREAD_STACK_SIZE 512
//定义线程优先级
#define LED_THREAD_PRIORITY 12
#define HEARTBEAT_THREAD_PRIORITY 13
#define KEY_THREAD_PRIORITY 14
//线程控制块
static rt_thread_t led_thread = RT_NULL;
static rt_thread_t heartbeat_thread = RT_NULL;
static rt_thread_t key_thread = RT_NULL;
//int函数定义（初始化）
static int rt_example_init(void){
    rt_kprintf("==========================");
    rt_kprintf("  RT-Thread Basic Example\r\n");
    rt_kprintf("  Priority levels: %d\r\n", RT_THREAD_PRIORITY_MAX);
    rt_kprintf("  Tick: %d Hz (1 tick = %d ms)\r\n",
               RT_TICK_PER_SECOND, 1000 / RT_TICK_PER_SECOND);
    rt_kprintf("========================================\r\n");

    /*--------------
     创建一个信号量
    */
    led_sem = rt_sem_create("led_sem",0,RT_IPC_FLAG_FIFO);

    if (led_sem == RT_NULL)
    {
        rt_kprintf("Failed to create semaphore!\r\n");
        return -1;
    }
    //led线程
    led_thread = rt_thread_create("led",
                                  led_thread_entry,
                                  RT_NULL,
                                  LED_THREAD_STACK_SIZE,
                                  LED_THREAD_PRIORITY,
                                  10);
    if(led_thread != RT_NULL){
        rt_thread_startup(led_thread);
        rt_kprintf("led_thread_ok!\r\n");
    }       
    //心跳线程
    heartbeat_thread = rt_thread_create("heartbeat",
                                        heartbeat_thread_entry,
                                        RT_NULL,
                                        HEARTBEAT_THREAD_STACK_SIZE,
                                        HEARTBEAT_THREAD_PRIORITY,
                                        10);
    if(heartbeat_thread != RT_NULL){
        rt_thread_startup(heartbeat_thread);
        rt_kprintf("heartbeat_thread_ok!\r\n");
    } 
    //key线程
    key_thread = rt_thread_create("key_contral",
                                  key_thread_entry,
                                  RT_NULL,
                                  HEARTBEAT_THREAD_STACK_SIZE,
                                  HEARTBEAT_THREAD_PRIORITY,
                                  10);           
    if(key_thread != RT_NULL){
        rt_thread_startup(key_thread);
        rt_kprintf("key_thread_ok");
    }          
    return 0;                                     
}
//RTT初始化宏，系统启动自动调用__函数
    INIT_APP_EXPORT(rt_example_init);

//main函数可以什么都不写，因为在__函数可以直接做完
int main(){

    return 0;
}