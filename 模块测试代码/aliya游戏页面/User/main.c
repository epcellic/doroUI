#include "stm32f10x.h"
#include "Delay.h"
#include "OLED.h"
#include "OLED_Data.h" 
#include <stdlib.h>
#include <math.h>

// --- 参数配置 ---
#define BIG_FONT_W  27       // 字体宽
#define BIG_FONT_H  38       // 字体高
#define TAIL_LEN    51       // 拖尾长度
#define SCAN_SPEED  4        // 扫描速度

// --- 心率波形数据 (Y坐标, 0-63) ---
// 修改说明：中间设计了多个连续的剧烈抖动(QRS波群)，两端保持绝对平直
const uint8_t HeartBeatPath[128] = {
    // 0-39: 前段绝对平直 (基准线 32) - 40个点
    32,32,32,32,32,32,32,32,32,32, 32,32,32,32,32,32,32,32,32,32,
    32,32,32,32,32,32,32,32,32,32, 32,32,32,32,32,32,32,32,32,32,
    
    // 40-89: 中间复杂抖动区 - 50个点
    // 模拟连续的P-QRS-T复合波形，制造多次起伏
    32,30,25,15, 5, 2, 5,15,25,32, // 第一次大抽动 (下潜->冲高->回落)
    45,58,62,58,45,35,32,25,15,10, // 第二次大抽动 (上浮->下潜)
    15,25,32,40,48,40,32,28,22,18, // 第三次中抽动
    22,28,32,35,40,35,32,30,32,32, // 第四次小抽动并回归
    32,32,32,32,32,32,32,32,32,32, // 过渡填充
    
    // 90-127: 后段绝对平直 - 38个点
    32,32,32,32,32,32,32,32,32,32, 32,32,32,32,32,32,32,32,32,32,
    32,32,32,32,32,32,32,32,32,32, 32,32,32,32,32,32,32,32
};
// --- 算法：生成平滑心率 (不变) ---
int Get_Next_BPM(int current_val)
{
    int change = 0;
    int direction = rand() % 100; 
    if (current_val >= 175) change = -1;
    else if (current_val <= 45) change = 1;
    else {
        if (direction < 10) change = 1;
        else if (direction < 20) change = -1;
        else change = 0;
    }
    return current_val + change;
}

int main(void)
{
    // 变量初始化
    int current_bpm = 75;       
    int16_t scan_x = 0;         
    uint8_t num_x_pos = 0;      
    uint8_t num_len = 0;        
    
    OLED_Init();
    OLED_Clear();
    OLED_Update();
    srand(0);

    // 初始计算位置
    num_len = (current_bpm >= 100) ? 3 : 2;
    num_x_pos = (128 - (num_len * BIG_FONT_W)) / 2;

    while (1)
    {
        // 1. 清空缓存
        OLED_Clear();

        // ==========================================
        // 2. 绘制数字 (现在是底层/背景层)
        // ==========================================
        // 先把数字画上去，占好位置
        OLED_ShowBigNum(num_x_pos, 13, current_bpm, num_len);

        // ==========================================
        // 3. 绘制波形线 (现在是顶层/遮罩层)
        // ==========================================
        // OLED_DrawLine使用的是“或”运算(|=)，
        // 所以线画在数字上时，会叠加显示，不会被数字的黑色背景擦除
        for (int i = 0; i < TAIL_LEN; i++) 
        {
            int curr_idx = scan_x - i;
            int prev_idx = scan_x - i - 1;

            if (prev_idx >= 0 && curr_idx < 128) 
            {
                OLED_DrawLine(prev_idx, HeartBeatPath[prev_idx], 
                              curr_idx, HeartBeatPath[curr_idx]);
            }
        }

        // 4. 刷新屏幕 & 状态更新
        OLED_Update(); 

        scan_x += SCAN_SPEED;
        
        // 周期结束处理
        if (scan_x >= 128 + TAIL_LEN) 
        {
            scan_x = 0;
            current_bpm = Get_Next_BPM(current_bpm);
            
            if (current_bpm >= 100) {
                num_len = 3;
                num_x_pos = (128 - (3 * BIG_FONT_W)) / 2; 
            } else {
                num_len = 2;
                num_x_pos = (128 - (2 * BIG_FONT_W)) / 2; 
            }
        }
        
        Delay_ms(2); 
    }
}