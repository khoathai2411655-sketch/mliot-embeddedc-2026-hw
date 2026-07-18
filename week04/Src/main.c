#include "stm32f1xx_hal.h"

void _init(void) {}
void _fini(void) {}

#define LED_MAIN_PORT       GPIOC
#define LED_MAIN_PIN         GPIO_PIN_13

#define BUTTON_PORT          GPIOA
#define BUTTON_PIN           GPIO_PIN_0

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
        // TASK 3: nháy LED 1 giây
        if (HAL_GetTick() - blinkLastTick >= BLINK_INTERVAL)
        {
            blinkLastTick = HAL_GetTick();
            HAL_GPIO_TogglePin(LED_MAIN_PORT, LED_MAIN_PIN);
        }

        // TASK 4: nút nhấn có debounce, TOGGLE khi nhấn xuống
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

        // TASK 5: quét ma trận, TOGGLE LED phản hồi khi có phím mới
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

static char Keypad_Scan(void)
{
    HAL_GPIO_WritePin(ROW0_PORT, ROW0_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(ROW1_PORT, ROW1_PIN, GPIO_PIN_SET);
    if (HAL_GPIO_ReadPin(COL0_PORT, COL0_PIN) == GPIO_PIN_RESET) return '1';
    if (HAL_GPIO_ReadPin(COL1_PORT, COL1_PIN) == GPIO_PIN_RESET) return '2';

    HAL_GPIO_WritePin(ROW0_PORT, ROW0_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(ROW1_PORT, ROW1_PIN, GPIO_PIN_RESET);
    if (HAL_GPIO_ReadPin(COL0_PORT, COL0_PIN) == GPIO_PIN_RESET) return '3';
    if (HAL_GPIO_ReadPin(COL1_PORT, COL1_PIN) == GPIO_PIN_RESET) return '4';

    HAL_GPIO_WritePin(ROW1_PORT, ROW1_PIN, GPIO_PIN_SET);
    return 0;
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
    HAL_GPIO_WritePin(LED_MAIN_PORT, LED_MAIN_PIN, GPIO_PIN_SET);

    GPIO_InitStruct.Pin = LED_KEYPAD_PIN;
    HAL_GPIO_Init(LED_KEYPAD_PORT, &GPIO_InitStruct);
    HAL_GPIO_WritePin(LED_KEYPAD_PORT, LED_KEYPAD_PIN, GPIO_PIN_RESET);
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

static void MX_GPIO_Init_Keypad(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIO_InitStruct.Pin   = ROW0_PIN | ROW1_PIN;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    HAL_GPIO_WritePin(ROW0_PORT, ROW0_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(ROW1_PORT, ROW1_PIN, GPIO_PIN_SET);

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