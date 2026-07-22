#include "stm32f1xx_hal.h"

void _init(void) {}
void _fini(void) {}

/* ============================================================
 *  TASK 3: LED chinh (nhay 1s)
 * ============================================================ */
#define LED_MAIN_PORT       GPIOC
#define LED_MAIN_PIN         GPIO_PIN_13

/* ============================================================
 *  TASK 4: Nut nhan (co debounce)
 * ============================================================ */
#define BUTTON_PORT          GPIOA
#define BUTTON_PIN           GPIO_PIN_0

/* ============================================================
 *  TASK 5: Ma tran phim 2x2
 *      - ROW0, ROW1 : Digital OUTPUT  (chan Hang)
 *      - COL0, COL1 : Digital INPUT PULL-UP (chan Cot)
 * ============================================================ */
#define ROW0_PORT            GPIOB
#define ROW0_PIN             GPIO_PIN_5
#define ROW1_PORT            GPIOB
#define ROW1_PIN             GPIO_PIN_6

#define COL0_PORT            GPIOB
#define COL0_PIN             GPIO_PIN_7
#define COL1_PORT            GPIOB
#define COL1_PIN             GPIO_PIN_8

#define LED_KEYPAD_PORT      GPIOC
#define LED_KEYPAD_PIN       GPIO_PIN_14

void SystemClock_Config(void);
static void MX_GPIO_Init_LED(void);
static void MX_GPIO_Init_Button(void);
static void MX_GPIO_Init_Keypad(void);
static char Keypad_Scan(void);

int main(void)
{
    /* ---------- BUOC 1 (Clock) + BUOC 2 (Hardware) ----------
     * HAL_Init()/SystemClock_Config() cau hinh xung he thong.
     * Cac ham MX_GPIO_Init_xxx() ben duoi se:
     *   (1) Bat clock cho GPIOx  (__HAL_RCC_GPIOx_CLK_ENABLE)
     *   (2) Cau hinh MODE/PULL cho tung chan (Hardware)
     * Phan con lai trong while(1) la BUOC 3 (Application Logic). */
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init_LED();
    MX_GPIO_Init_Button();
    MX_GPIO_Init_Keypad();

    uint32_t blinkLastTick = 0;
    const uint32_t BLINK_INTERVAL = 500;

    GPIO_PinState buttonLastRawState = GPIO_PIN_SET;
    GPIO_PinState buttonStableState  = GPIO_PIN_SET;
    uint32_t buttonDebounceTick = 0;
    const uint32_t DEBOUNCE_DELAY = 50;

    char keypadLastRawKey = 0;
    char keypadStableKey  = 0;
    uint32_t keypadDebounceTick = 0;

    while (1)
    {
        /* ---------------- TASK 3: nhay LED 1 giay ---------------- */
        if (HAL_GetTick() - blinkLastTick >= BLINK_INTERVAL)
        {
            blinkLastTick = HAL_GetTick();
            HAL_GPIO_TogglePin(LED_MAIN_PORT, LED_MAIN_PIN);
        }

        /* ------- TASK 4: nut nhan co debounce, toggle khi nhan ------- */
        GPIO_PinState buttonRawNow = HAL_GPIO_ReadPin(BUTTON_PORT, BUTTON_PIN);
        if (buttonRawNow != buttonLastRawState)
            buttonDebounceTick = HAL_GetTick();

        if ((HAL_GetTick() - buttonDebounceTick) > DEBOUNCE_DELAY)
        {
            if (buttonRawNow != buttonStableState)
            {
                buttonStableState = buttonRawNow;
                if (buttonStableState == GPIO_PIN_RESET)
                    HAL_GPIO_TogglePin(LED_MAIN_PORT, LED_MAIN_PIN);
            }
        }
        buttonLastRawState = buttonRawNow;

        /* ---- TASK 5: quet ma tran, toggle LED phan hoi khi co phim moi ---- */
        char keyRawNow = Keypad_Scan();
        if (keyRawNow != keypadLastRawKey)
            keypadDebounceTick = HAL_GetTick();

        if ((HAL_GetTick() - keypadDebounceTick) > DEBOUNCE_DELAY)
        {
            if (keyRawNow != keypadStableKey)
            {
                keypadStableKey = keyRawNow;
                if (keypadStableKey != 0)
                    HAL_GPIO_TogglePin(LED_KEYPAD_PORT, LED_KEYPAD_PIN);
            }
        }
        keypadLastRawKey = keyRawNow;
    }
}

/* ================================================================
 * Keypad_Scan()
 * Quy trinh: lan luot keo TUNG hang xuong 0 (hang con lai giu 1),
 * roi doc 2 chan cot. Cot nao doc duoc 0 -> phim tai giao diem
 * (hang dang keo, cot do) dang duoc nhan.
 *
 *   ROW0=0, ROW1=1  -> kiem tra COL0 ('1'), COL1 ('2')
 *   ROW0=1, ROW1=0  -> kiem tra COL0 ('3'), COL1 ('4')
 *
 * Ket thuc ham, CA HAI hang duoc dua ve muc 1 (trang thai nghi)
 * de khong lam sai lech lan quet ke tiep.
 * ================================================================ */
static char Keypad_Scan(void)
{
    char key = 0;

    /* ---- Quet HANG 0 ---- */
    HAL_GPIO_WritePin(ROW0_PORT, ROW0_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(ROW1_PORT, ROW1_PIN, GPIO_PIN_SET);

    if (HAL_GPIO_ReadPin(COL0_PORT, COL0_PIN) == GPIO_PIN_RESET) key = '1';
    else if (HAL_GPIO_ReadPin(COL1_PORT, COL1_PIN) == GPIO_PIN_RESET) key = '2';

    /* ---- Quet HANG 1 ---- */
    HAL_GPIO_WritePin(ROW0_PORT, ROW0_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(ROW1_PORT, ROW1_PIN, GPIO_PIN_RESET);

    if (key == 0)
    {
        if (HAL_GPIO_ReadPin(COL0_PORT, COL0_PIN) == GPIO_PIN_RESET) key = '3';
        else if (HAL_GPIO_ReadPin(COL1_PORT, COL1_PIN) == GPIO_PIN_RESET) key = '4';
    }

    /* ---- Dua ca 2 hang ve muc nghi (1) truoc khi ra khoi ham ---- */
    HAL_GPIO_WritePin(ROW1_PORT, ROW1_PIN, GPIO_PIN_SET);

    return key;
}

static void MX_GPIO_Init_LED(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOC_CLK_ENABLE();

    GPIO_InitStruct.Pin   = LED_MAIN_PIN;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_MAIN_PORT, &GPIO_InitStruct);
    HAL_GPIO_WritePin(LED_MAIN_PORT, LED_MAIN_PIN, GPIO_PIN_SET); // PC13 active-low -> SET = tat

    GPIO_InitStruct.Pin = LED_KEYPAD_PIN;
    HAL_GPIO_Init(LED_KEYPAD_PORT, &GPIO_InitStruct);
    HAL_GPIO_WritePin(LED_KEYPAD_PORT, LED_KEYPAD_PIN, GPIO_PIN_RESET); // LED keypad tat ban dau
}

static void MX_GPIO_Init_Button(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitStruct.Pin  = BUTTON_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(BUTTON_PORT, &GPIO_InitStruct);
}

/* Yeu cau i: 2 chan Hang = Output, 2 chan Cot = Input Pull-up */
static void MX_GPIO_Init_Keypad(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /* --- 2 chan HANG: Output Push-Pull --- */
    GPIO_InitStruct.Pin   = ROW0_PIN | ROW1_PIN;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    HAL_GPIO_WritePin(ROW0_PORT, ROW0_PIN, GPIO_PIN_SET); // trang thai nghi = muc 1
    HAL_GPIO_WritePin(ROW1_PORT, ROW1_PIN, GPIO_PIN_SET);

    /* --- 2 chan COT: Input Pull-up --- */
    GPIO_InitStruct.Pin  = COL0_PIN | COL1_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);
}

void HAL_MspInit(void) {}
void SysTick_Handler(void) { HAL_IncTick(); }