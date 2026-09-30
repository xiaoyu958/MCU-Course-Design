#include "stm32f10x.h"
#include "BSP_LCD.h"
#include "BSP_USART1.h"
#include "BSP_KEY.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// ==================== 汉字索引定义（基于你的字库顺序，0-52） ====================
#define POS_ZI         0
#define POS_DONG       1
#define POS_HUA        2
#define POS_XUE        3
#define POS_YUAN       4
#define POS_DIAN       5
#define POS_ZI2        6
#define POS_XIN        7
#define POS_XI         8
#define POS_BAN        9
#define POS_HAO       10
#define POS_XING      11
#define POS_MING      12
#define POS_ZHONG     13
#define POS_NAN       14
#define POS_DA        15
#define POS_YUE       16
#define POS_LU        17
#define POS_SHAN      18
#define POS_XIAO      19
#define POS_QU        20
#define POS_SHENG     21
#define POS_HUA2      22
#define POS_HOU       23
#define POS_LOU       24
#define POS_BAO       25
#define POS_GAO       26
#define POS_TI        27
#define POS_MU        28
#define POS_SHI       29
#define POS_DI        30
#define POS_DIAN2     31
#define POS_CHOU      32
#define POS_XIANG     33
#define POS_JIAO      34
#define POS_SHOU      35
#define POS_SHU       36
#define POS_XING2     37
#define POS_QI        38
#define POS_ER        39
#define POS_KAI       40
#define POS_SHI2      41
#define POS_HUI       42
#define POS_YI        43
#define POS_SHI3      44
#define POS_DE        45
#define POS_GUO       46
#define POS_JI        47
#define POS_ZHAI      48
#define POS_YAO       49
#define POS_FAN       50
#define POS_YU        51
#define POS_HONG      52

// ==================== 硬件定义 ====================
#define LED_BLUE_PIN   GPIO_Pin_1
#define LED_BLUE_PORT  GPIOB

// ==================== 全局变量 ====================
uint8_t current_mode = 1;
uint8_t last_mode = 0;
uint8_t mode1_dirty = 1;
uint8_t auto_play = 0;
uint32_t auto_timer = 0;

// 模式2
static int16_t scroll_x = 240;
static uint8_t scroll_init = 1;
static uint32_t last_move_time = 0;

// 模式3
static uint8_t mode3_initialized = 0;
static uint8_t report_page = 0;
static uint32_t page_deadline = 0;

// 模式4
static uint8_t pic_init = 1;

// 模式5
static int16_t rect_x, rect_y, rect_w, rect_h;
static uint8_t rect_active = 0;
static uint32_t rect_last_time = 0;

// 模式6
#define PARTICLE_COUNT 100
typedef struct {
    int16_t x, y;
    uint16_t color;
} Particle;
static Particle particles[PARTICLE_COUNT];
static uint8_t mode6_init = 1;
static uint32_t particle_last_time = 0;

// 模式7
static uint8_t mode7_game_active = 0;
static int16_t target_x = -1, target_y = -1;
static uint32_t target_appear_time = 0;
static uint32_t game_start_time = 0;
static int16_t score = 0;
static uint8_t mode7_game_ended = 0;   // 游戏结束标志

// 模式8
#define METEOR_COUNT 5
typedef struct {
    int16_t x, y;
    int8_t dx, dy;
    uint8_t active;
} Meteor;
static Meteor meteors[METEOR_COUNT];
static uint8_t mode8_init = 1;
static uint32_t meteor_last_time = 0;
static uint32_t led_toggle_time = 0;   // LED闪烁计时
static uint8_t led_state = 0;

// 模式9
static uint32_t clock_start_ms = 0;
static uint32_t clock_last_second = 0;

// 模式10
#define GRAVITY 0.5f
#define BOUNCE_DAMP 0.7f
#define STOP_SPEED 1.0f
#define BALL_RADIUS 10
static float ball_x = 120, ball_y = 20;
static float ball_vx = 0, ball_vy = 0;
static uint8_t mode10_init = 1;
static uint32_t ball_last_time = 0;
static uint8_t ball_stopped = 0;

// ADC
volatile uint16_t adc_value = 0;
uint32_t scroll_delay_ms = 100;

// ==================== 系统时钟 ====================
volatile uint32_t sys_ms = 0;
void SysTick_Handler(void) { sys_ms++; }
void delay_ms(uint32_t ms) {
    uint32_t start = sys_ms;
    while((sys_ms - start) < ms);
}

// ==================== LED 初始化 ====================
void LED_Init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    GPIO_InitStructure.GPIO_Pin = LED_BLUE_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(LED_BLUE_PORT, &GPIO_InitStructure);
    GPIO_SetBits(LED_BLUE_PORT, LED_BLUE_PIN);
}
void LED_On(void) { GPIO_ResetBits(LED_BLUE_PORT, LED_BLUE_PIN); }
void LED_Off(void) { GPIO_SetBits(LED_BLUE_PORT, LED_BLUE_PIN); }
void LED_Toggle(void) {
    if(GPIO_ReadOutputDataBit(LED_BLUE_PORT, LED_BLUE_PIN))
        GPIO_ResetBits(LED_BLUE_PORT, LED_BLUE_PIN);
    else
        GPIO_SetBits(LED_BLUE_PORT, LED_BLUE_PIN);
}

// ==================== ADC 初始化 ====================
void MyADC_Init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    ADC_InitTypeDef ADC_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_ADC1, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    ADC_DeInit(ADC1);
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &ADC_InitStructure);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_55Cycles5);
    ADC_Cmd(ADC1, ENABLE);
    ADC_ResetCalibration(ADC1);
    while(ADC_GetResetCalibrationStatus(ADC1));
    ADC_StartCalibration(ADC1);
    while(ADC_GetCalibrationStatus(ADC1));
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}
void UpdateScrollSpeed(void) {
    adc_value = ADC_GetConversionValue(ADC1);
    if(adc_value < 1365) scroll_delay_ms = 100;
    else if(adc_value < 2730) scroll_delay_ms = 60;
    else scroll_delay_ms = 30;
}

// ==================== 辅助函数 ====================
void ShowChinese(uint16_t x, uint16_t y, const uint8_t *indexs, uint8_t len, uint16_t color, uint16_t bg) {
    uint16_t x0 = x;
    for(uint8_t i=0; i<len; i++) {
        LcdDispHanzi1616(x0, y, indexs[i], color, bg);
        x0 += 16;
    }
}
void DrawCircle(int16_t cx, int16_t cy, int16_t radius, uint16_t color) {
    LcdFill(cx - radius, cy - radius, radius*2, radius*2, color);
}

// ==================== 模式1 ====================
void Display_Mode1(void) {
    if(mode1_dirty) {
        LcdFill(0, 0, 240, 320, BLACK);
        uint8_t line1[] = {0,1,2,3,4};
        ShowChinese(20, 20, line1, 5, WHITE, BLACK);
        uint8_t line2_han[] = {5,6,7,8};
        ShowChinese(20, 50, line2_han, 4, WHITE, BLACK);
        LcdDispEncharString(20+64, 50, "2403", 1608, WHITE, BLACK);
        LcdDispHanzi1616(20+64+32, 50, 9, WHITE, BLACK);
        LcdDispHanzi1616(20, 80, 3, WHITE, BLACK);
        LcdDispHanzi1616(20+16, 80, 10, WHITE, BLACK);
        LcdDispEncharString(20+32, 80, "8207241011", 1608, WHITE, BLACK);
        LcdDispHanzi1616(20, 110, 50, WHITE, BLACK);
        LcdDispHanzi1616(20+16, 110, 51, WHITE, BLACK);
        LcdDispHanzi1616(20+32, 110, 52, WHITE, BLACK);
        mode1_dirty = 0;
    }
}

// ==================== 模式2 ====================
void Display_Mode2(void) {
    if(scroll_init) {
        LcdFill(0, 0, 240, 320, BLACK);
        scroll_x = 240;
        scroll_init = 0;
        last_move_time = sys_ms;
    }
    if(sys_ms - last_move_time >= scroll_delay_ms) {
        last_move_time = sys_ms;
        LcdFill(scroll_x, 120, 120, 20, BLACK);
        LcdDispEncharString(scroll_x, 120, "I LOVE CSU", 1608, RED, BLACK);
        scroll_x -= 2;
        if(scroll_x < -100) {
            scroll_x = 240;
            scroll_init = 1;
        }
    }
}

// ==================== 模式3辅助 ====================
static void draw_first_screen(void) {
    LcdFill(0, 0, 240, 320, BLACK);
    LcdDispEncharString(10, 10, "Yuri Shardt", 1608, WHITE, BLACK);
    uint8_t prof[] = {34,35};
    ShowChinese(10+80, 10, prof, 2, WHITE, BLACK);
    uint8_t academic[] = {3,36};
    ShowChinese(10+112, 10, academic, 2, WHITE, BLACK);
    LcdDispEncharString(10+144, 10, "Report", 1608, WHITE, BLACK);
    uint8_t title[] = {25,26,27,28};
    ShowChinese(10, 40, title, 4, WHITE, BLACK);
    LcdDispEncharString(10+64, 40, "Artificial Intelligence and Modelling: What Can Be Done", 1608, WHITE, BLACK);
    uint8_t time[] = {29};
    ShowChinese(10, 70, time, 1, WHITE, BLACK);
    LcdDispEncharString(10+16, 70, ": 2025-02-28 (", 1608, WHITE, BLACK);
    uint8_t week[] = {37,38};
    ShowChinese(10+16+80, 70, week, 2, WHITE, BLACK);
    LcdDispEncharString(10+16+112, 70, ") 9:30 ", 1608, WHITE, BLACK);
    uint8_t start[] = {40,41};
    ShowChinese(10+16+112+48, 70, start, 2, WHITE, BLACK);
    uint8_t loc[] = {30,31};
    ShowChinese(10, 100, loc, 2, WHITE, BLACK);
    uint8_t addr1[] = {13,14,15,3,16,17,18,19,20};
    ShowChinese(10+32, 100, addr1, 9, WHITE, BLACK);
    uint8_t addr2[] = {21,22,23,24};
    ShowChinese(10+32+144, 100, addr2, 4, WHITE, BLACK);
    LcdDispEncharString(10+32+144+64, 100, "504 ", 1608, WHITE, BLACK);
    uint8_t room[] = {42,43,44};
    ShowChinese(10+32+144+64+32, 100, room, 3, WHITE, BLACK);
}

// ==================== 模式3 ====================
void Display_Mode3(void) {
    if(!mode3_initialized) {
        draw_first_screen();
        report_page = 0;
        page_deadline = sys_ms + 6000;
        mode3_initialized = 1;
        return;
    }
    if(sys_ms >= page_deadline) {
        page_deadline = sys_ms + 6000;
        report_page++;
        if(report_page >= 3) report_page = 0;
        LcdFill(0, 0, 240, 320, BLACK);
        if(report_page == 0) {
            draw_first_screen();
        } else if(report_page == 1) {
            LcdDispEncharString(10, 10, "Reporter: Yuri Shardt", 1608, WHITE, BLACK);
            uint8_t prof2[] = {34,35};
            ShowChinese(10+160, 10, prof2, 2, WHITE, BLACK);
            LcdDispEncharString(10, 40, "Technical University of Ilmenau, ", 1608, WHITE, BLACK);
            uint8_t de[] = {45,46};
            ShowChinese(10+200, 40, de, 2, WHITE, BLACK);
            LcdDispEncharString(10, 70, "Abstract:", 1608, WHITE, BLACK);
            LcdDispEncharString(10, 100, "As the ubiquity of artificial intelligence increases,", 1608, WHITE, BLACK);
            LcdDispEncharString(10, 130, "there is a need to provide a sober and realistic", 1608, WHITE, BLACK);
            LcdDispEncharString(10, 160, "assessment of its capabilities and future potential.", 1608, WHITE, BLACK);
        } else {
            LcdDispEncharString(10, 20, "In this presentation, the key terms will be examined,", 1608, WHITE, BLACK);
            LcdDispEncharString(10, 50, "some applications considered, and a perspective on", 1608, WHITE, BLACK);
            LcdDispEncharString(10, 80, "the future of AI and modelling presented.", 1608, WHITE, BLACK);
            LcdDispEncharString(10, 110, "The primary focus will be on applications to", 1608, WHITE, BLACK);
            LcdDispEncharString(10, 140, "chemical and process engineering.", 1608, WHITE, BLACK);
        }
    }
}

// ==================== 模式4 ====================
void Display_Mode4(void) {
    if(pic_init) {
        LcdFill(0, 0, 240, 320, BLACK);
        LcdFill(0, 0, 240, 200, 0x051F);
        LcdFill(40, 50, 30, 20, WHITE);
        LcdFill(50, 40, 20, 25, WHITE);
        LcdFill(30, 60, 25, 15, WHITE);
        LcdFill(60, 55, 20, 20, WHITE);
        LcdFill(160, 80, 35, 22, WHITE);
        LcdFill(175, 70, 25, 28, WHITE);
        LcdFill(145, 90, 28, 18, WHITE);
        LcdFill(190, 85, 22, 22, WHITE);
        LcdFill(100, 30, 25, 15, WHITE);
        LcdFill(110, 25, 20, 18, WHITE);
        LcdFill(0, 240, 240, 80, 0x07E0);
        LcdFill(80, 160, 20, 100, 0xF800);
        LcdFill(140, 160, 20, 100, 0xF800);
        LcdFill(70, 150, 100, 15, 0xF800);
        for(int i = 0; i < 60; i++) {
            int x = 100 + i;
            int y = 130 + (int)(15 * sqrt(1 - pow((i-30)/30.0, 2)));
            LCD_DrawPoint(x, y, 0xF800);
        }
        LcdFill(95, 170, 50, 20, WHITE);
        LcdDispEncharString(105, 175, "CSU", 1608, RED, WHITE);
        pic_init = 0;
    }
}

// ==================== 模式5 ====================
void Display_Mode5(void) {
    static int16_t x = 0, y = 0, w = 240, h = 320;
    static uint8_t init = 1;
    static uint32_t last_time = 0;
    if(init) {
        LcdFill(0, 0, 240, 320, BLACK);
        x = 0; y = 0; w = 240; h = 320;
        last_time = sys_ms;
        init = 0;
        LcdDrawRectangle(x, y, w, h, RED);
        return;
    }
    if(sys_ms - last_time >= 100) {
        last_time = sys_ms;
        LcdDrawRectangle(x, y, w, h, BLACK);
        x += 5; y += 5;
        w -= 10; h -= 10;
        if(w <= 0 || h <= 0 || x >= 240 || y >= 320) {
            init = 1;
            return;
        }
        if(x + w > 240) w = 240 - x;
        if(y + h > 320) h = 320 - y;
        LcdDrawRectangle(x, y, w, h, RED);
    }
}

// ==================== 模式6 ====================
void Display_Mode6(void) {
    if(mode6_init) {
        for(int i=0; i<PARTICLE_COUNT; i++) {
            particles[i].x = rand() % 240;
            particles[i].y = rand() % 320;
            particles[i].color = (rand() % 0xFFFF) & 0xF81F;
        }
        mode6_init = 0;
        particle_last_time = sys_ms;
        LcdFill(0, 0, 240, 320, BLACK);
        for(int i=0; i<PARTICLE_COUNT; i++) LCD_DrawPoint(particles[i].x, particles[i].y, particles[i].color);
        return;
    }
    if(sys_ms - particle_last_time >= 50) {
        particle_last_time = sys_ms;
        for(int i=0; i<PARTICLE_COUNT; i++) LCD_DrawPoint(particles[i].x, particles[i].y, BLACK);
        for(int i=0; i<PARTICLE_COUNT; i++) {
            particles[i].y += 2;
            if(particles[i].y >= 320) {
                particles[i].y = 0;
                particles[i].x = rand() % 240;
                particles[i].color = (rand() % 0xFFFF) & 0xF81F;
            }
        }
        for(int i=0; i<PARTICLE_COUNT; i++) LCD_DrawPoint(particles[i].x, particles[i].y, particles[i].color);
    }
}

// ==================== 模式7 ====================
void Display_Mode7_Static(void) {
    LcdFill(0, 0, 240, 320, BLACK);
    LcdDispEncharString(30, 140, "Reaction Test Demo", 1608, WHITE, BLACK);
    LcdDispEncharString(30, 170, "Press K2 to start", 1608, WHITE, BLACK);
}

void Display_Mode7_Dynamic(void) {
    if(mode7_game_ended) {
        LcdFill(0, 0, 240, 320, BLACK);
        LcdDispEncharString(30, 140, "Game Over", 2416, WHITE, BLACK);
        char buf[20];
        sprintf(buf, "Score: %d", score);
        LcdDispEncharString(30, 170, buf, 1608, WHITE, BLACK);
        LcdDispEncharString(30, 200, "Press K2 to next", 1608, WHITE, BLACK);
        return;
    }
    if(!mode7_game_active) {
        LcdFill(0, 0, 240, 320, BLACK);
        LcdDispEncharString(10, 10, "Score: 0", 1608, WHITE, BLACK);
        score = 0;
        target_x = -1;
        game_start_time = sys_ms;
        mode7_game_active = 1;
        target_appear_time = sys_ms;
        return;
    }
    if(sys_ms - game_start_time >= 30000) {
        mode7_game_active = 0;
        mode7_game_ended = 1;
        return;
    }
    char time_buf[20];
    int remaining = 30 - (sys_ms - game_start_time)/1000;
    sprintf(time_buf, "Time: %d", remaining);
    LcdFill(0, 0, 240, 20, BLACK);
    LcdDispEncharString(10, 10, time_buf, 1608, WHITE, BLACK);
    char score_buf[20];
    sprintf(score_buf, "Score: %d", score);
    LcdFill(150, 0, 80, 20, BLACK);
    LcdDispEncharString(150, 10, score_buf, 1608, WHITE, BLACK);
    if(target_x == -1) {
        target_x = rand() % (240 - 20);
        target_y = rand() % (320 - 20);
        LcdFill(target_x, target_y, 20, 20, RED);
        target_appear_time = sys_ms;
    } else {
        if(sys_ms - target_appear_time >= 1000) {
            LcdFill(target_x, target_y, 20, 20, BLACK);
            target_x = -1;
        }
    }
}

// ==================== 模式8 ====================
void Display_Mode8(void) {
    // LED闪烁：每2秒翻转一次
    if(sys_ms - led_toggle_time >= 2000) {
        led_toggle_time = sys_ms;
        LED_Toggle();
    }
    if(mode8_init) {
        for(int i=0; i<METEOR_COUNT; i++) meteors[i].active = 0;
        mode8_init = 0;
        meteor_last_time = sys_ms;
        LcdFill(0, 0, 240, 320, BLACK);
        return;
    }
    if(sys_ms - meteor_last_time >= 100) {
        meteor_last_time = sys_ms;
        for(int i=0; i<METEOR_COUNT; i++) {
            if(meteors[i].active) {
                LCD_DrawPoint(meteors[i].x, meteors[i].y, BLACK);
                LCD_DrawPoint(meteors[i].x - meteors[i].dx, meteors[i].y - meteors[i].dy, BLACK);
            }
        }
        for(int i=0; i<METEOR_COUNT; i++) {
            if(meteors[i].active) {
                meteors[i].x += meteors[i].dx;
                meteors[i].y += meteors[i].dy;
                if(meteors[i].x < 0 || meteors[i].x >= 240 || meteors[i].y < 0 || meteors[i].y >= 320) {
                    meteors[i].active = 0;
                }
            }
        }
        if(rand() % 10 < 3) {
            for(int i=0; i<METEOR_COUNT; i++) {
                if(!meteors[i].active) {
                    meteors[i].x = rand() % 240;
                    meteors[i].y = 0;
                    meteors[i].dx = (rand() % 3) + 1;
                    meteors[i].dy = (rand() % 3) + 1;
                    meteors[i].active = 1;
                    break;
                }
            }
        }
        for(int i=0; i<METEOR_COUNT; i++) {
            if(meteors[i].active) {
                LCD_DrawPoint(meteors[i].x, meteors[i].y, WHITE);
                LCD_DrawPoint(meteors[i].x - meteors[i].dx, meteors[i].y - meteors[i].dy, WHITE);
            }
        }
    }
}

// ==================== 模式9 ====================
void Display_Mode9_Static(void) {
    LcdFill(0, 0, 240, 320, BLACK);
    LcdDispEncharString(30, 140, "Clock Demo", 1608, WHITE, BLACK);
    LcdDispEncharString(30, 170, "00:00", 2416, WHITE, BLACK);
}

void Display_Mode9_Dynamic(void) {
    if(clock_start_ms == 0) {
        clock_start_ms = sys_ms;
        clock_last_second = sys_ms;
        LcdFill(0, 0, 240, 320, BLACK);
        LcdDispEncharString(10, 10, "Clock", 1608, WHITE, BLACK);
        LcdDispEncharString(10, 40, "Press K2 to reset", 1608, WHITE, BLACK);
        LcdDispEncharString(10, 70, "00:00", 2416, WHITE, BLACK);
    }
    if(sys_ms - clock_last_second >= 1000) {
        clock_last_second = sys_ms;
        uint32_t elapsed = (sys_ms - clock_start_ms) / 1000;
        uint8_t minutes = elapsed / 60;
        uint8_t seconds = elapsed % 60;
        char time_str[20];
        sprintf(time_str, "%02d:%02d", minutes, seconds);
        LcdFill(10, 70, 100, 30, BLACK);
        LcdDispEncharString(10, 70, time_str, 2416, WHITE, BLACK);
    }
}

// ==================== 模式10 ====================
void Display_Mode10_Static(void) {
    LcdFill(0, 0, 240, 320, BLACK);
    LcdDispEncharString(30, 140, "Gravity Ball Demo", 1608, WHITE, BLACK);
    LcdDispEncharString(30, 170, "Press K2 to reset", 1608, WHITE, BLACK);
}

void Display_Mode10_Dynamic(void) {
    if(mode10_init) {
        ball_x = 120; ball_y = 20;
        ball_vx = 0; ball_vy = 0;
        ball_stopped = 0;
        mode10_init = 0;
        ball_last_time = sys_ms;
        LcdFill(0, 0, 240, 320, BLACK);
        DrawCircle((int)ball_x, (int)ball_y, BALL_RADIUS, WHITE);
        return;
    }
    if(ball_stopped) return;
    if(sys_ms - ball_last_time >= 20) {
        ball_last_time = sys_ms;
        DrawCircle((int)ball_x, (int)ball_y, BALL_RADIUS, BLACK);
        ball_vy += GRAVITY;
        ball_x += ball_vx;
        ball_y += ball_vy;
        if(ball_x - BALL_RADIUS <= 0) {
            ball_x = BALL_RADIUS;
            ball_vx = -ball_vx * BOUNCE_DAMP;
        } else if(ball_x + BALL_RADIUS >= 240) {
            ball_x = 240 - BALL_RADIUS;
            ball_vx = -ball_vx * BOUNCE_DAMP;
        }
        if(ball_y + BALL_RADIUS >= 320) {
            ball_y = 320 - BALL_RADIUS;
            ball_vy = -ball_vy * BOUNCE_DAMP;
            if(fabs(ball_vy) < STOP_SPEED && fabs(ball_vx) < STOP_SPEED) ball_stopped = 1;
        }
        if(ball_y - BALL_RADIUS <= 0) {
            ball_y = BALL_RADIUS;
            ball_vy = -ball_vy * BOUNCE_DAMP;
        }
        DrawCircle((int)ball_x, (int)ball_y, BALL_RADIUS, WHITE);
    }
}

// ==================== 串口反馈 ====================
void ReportStatus(void) {
    printf("Mode:%d, Auto:%d, ADC:%d\r\n", current_mode, auto_play, adc_value);
}

// ==================== 主函数 ====================
int main(void) {
    if(SysTick_Config(SystemCoreClock / 1000)) while(1);
    USART1Init(9600);
    LCD_Init();
    LCD_ScanDir(6);
    BSP_KEY_Init();
    MyADC_Init();
    LED_Init();
    srand(sys_ms);
    printf("System Ready\r\n");
    mode1_dirty = 1;
    Display_Mode1();
    last_mode = 1;

    static uint32_t mode3_start_time = 0;
    static uint8_t mode3_entered = 0;

    while(1) {
        UpdateScrollSpeed();

        // K1处理：在手动模式下模式7或模式9按K1直接下一个模式，其他情况切换自动/手动
        if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == 1) {
            delay_ms(20);
            if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == 1) {
                if(!auto_play && (current_mode == 7 || current_mode == 9)) {
                    if(current_mode == 7) current_mode = 8;
                    else if(current_mode == 9) current_mode = 10;
                    printf("Force switch to Mode %d\r\n", current_mode);
                    ReportStatus();
                    if(current_mode == 8) mode8_init = 1;
                    if(current_mode == 10) mode10_init = 1;
                } else {
                    auto_play = !auto_play;
                    printf("Auto play %s\r\n", auto_play?"ON":"OFF");
                    ReportStatus();
                    if(auto_play) auto_timer = sys_ms;
                }
                while(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == 1);
            }
        }

        // K2处理（手动模式下）
        if(!auto_play) {
            if(current_mode == 7) {
                if(mode7_game_ended) {
                    // 结束页面按K2切换到模式8
                    if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13) == 1) {
                        delay_ms(20);
                        if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13) == 1) {
                            current_mode = 8;
                            printf("Switch to Mode 8 from Mode7 ended\r\n");
                            ReportStatus();
                            mode8_init = 1;
                            mode7_game_ended = 0;
                            mode7_game_active = 0;
                            while(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13) == 1);
                        }
                    }
                } else {
                    // 游戏中按K2击中目标
                    if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13) == 1) {
                        delay_ms(20);
                        if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13) == 1) {
                            if(mode7_game_active && target_x != -1) {
                                LcdFill(target_x, target_y, 20, 20, BLACK);
                                target_x = -1;
                                score++;
                                LcdFill(170, 30, 30, 20, BLACK);
                                LcdDispEncharString(170, 30, "+1", 1608, GREEN, BLACK);
                            }
                            while(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13) == 1);
                        }
                    }
                }
            } else if(current_mode == 9) {
                // 模式9：短按K2重置计时
                if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13) == 1) {
                    delay_ms(20);
                    if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13) == 1) {
                        clock_start_ms = sys_ms;
                        clock_last_second = sys_ms;
                        LcdFill(0, 0, 240, 320, BLACK);
                        LcdDispEncharString(10, 10, "Clock", 1608, WHITE, BLACK);
                        LcdDispEncharString(10, 40, "Press K2 to reset", 1608, WHITE, BLACK);
                        LcdDispEncharString(10, 70, "00:00", 2416, WHITE, BLACK);
                        while(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13) == 1);
                    }
                }
            } else if(current_mode == 10) {
                // 模式10：按K2重置小球
                if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13) == 1) {
                    delay_ms(20);
                    if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13) == 1) {
                        ball_x = 120; ball_y = 20;
                        ball_vx = 0; ball_vy = 0;
                        ball_stopped = 0;
                        mode10_init = 1;
                        while(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13) == 1);
                    }
                }
            } else {
                // 其他模式短按切换模式
                if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13) == 1) {
                    delay_ms(20);
                    if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13) == 1) {
                        current_mode++;
                        if(current_mode > 10) current_mode = 1;
                        printf("Manual switch to Mode %d\r\n", current_mode);
                        ReportStatus();
                        mode7_game_active = 0; mode7_game_ended = 0;
                        clock_start_ms = 0;
                        mode8_init = 1; mode10_init = 1;
                        while(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13) == 1);
                    }
                }
            }
        } else {
            // 自动轮播模式下K2手动切换
            if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13) == 1) {
                delay_ms(20);
                if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13) == 1) {
                    current_mode++;
                    if(current_mode > 10) current_mode = 1;
                    auto_timer = sys_ms;
                    printf("Manual override to Mode %d (auto still ON)\r\n", current_mode);
                    ReportStatus();
                    mode7_game_active = 0; mode7_game_ended = 0;
                    clock_start_ms = 0;
                    mode8_init = 1; mode10_init = 1;
                    while(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13) == 1);
                }
            }
        }

        // 自动轮播计时
        if(auto_play) {
            if(current_mode == 3) {
                if(!mode3_entered) {
                    mode3_start_time = sys_ms;
                    mode3_entered = 1;
                }
                if(sys_ms - mode3_start_time >= 18000) {
                    current_mode++;
                    if(current_mode > 10) current_mode = 1;
                    auto_timer = sys_ms;
                    mode3_entered = 0;
                    printf("Auto switch from Mode3 to Mode%d\r\n", current_mode);
                    ReportStatus();
                    mode8_init = 1; mode10_init = 1;
                }
            } else {
                if(sys_ms - auto_timer >= 2000) {
                    auto_timer = sys_ms;
                    current_mode++;
                    if(current_mode > 10) current_mode = 1;
                    printf("Auto switch to Mode %d\r\n", current_mode);
                    ReportStatus();
                    mode3_entered = 0;
                    mode7_game_active = 0; mode7_game_ended = 0;
                    clock_start_ms = 0;
                    mode8_init = 1; mode10_init = 1;
                }
            }
        }

        // 模式切换清屏
        if(current_mode != last_mode) {
            LcdFill(0, 0, 240, 320, BLACK);
            scroll_init = 1;
            mode3_initialized = 0;
            pic_init = 1;
            mode1_dirty = 1;
            mode6_init = 1;
            // 关闭LED（模式8会自己开启）
            LED_Off();
            last_mode = current_mode;
        }

        // 执行模式
        switch(current_mode) {
            case 1: Display_Mode1(); break;
            case 2: Display_Mode2(); break;
            case 3: Display_Mode3(); break;
            case 4: Display_Mode4(); break;
            case 5: Display_Mode5(); break;
            case 6: Display_Mode6(); break;
            case 7:
                if(auto_play) Display_Mode7_Static();
                else Display_Mode7_Dynamic();
                break;
            case 8: Display_Mode8(); break;
            case 9:
                if(auto_play) Display_Mode9_Static();
                else Display_Mode9_Dynamic();
                break;
            case 10:
                if(auto_play) Display_Mode10_Static();
                else Display_Mode10_Dynamic();
                break;
        }
        delay_ms(1);
    }
}