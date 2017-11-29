#include "py/nlr.h"
#include "py/obj.h"
#include "py/runtime.h"
#include "py/binary.h"
#include "portmodules.h"
#include "stm32f4xx_hal.h"

#define WIDTH 64
#define HEIGHT 64
#define CHANNELS 7 // R3G3B2, (2 ** 3 - 1) channels
#define SEL 5
#define SCAN (1 << SEL)
#define PORT GPIOE

TIM_HandleTypeDef htimer;

uint8_t pixels[HEIGHT][WIDTH];
int _display_CurrentLine;
int _display_CurrentChannel;

//#define wait() { __asm__("nop"); __asm__("nop");  __asm__("nop"); }
#define wait()

void Display_DrawLine(int line, int ch)
{
	for (int x = 0; x < WIDTH; ++x)
	{
		uint8_t p1 = pixels[line][x];
		uint8_t p2 = pixels[line + SCAN][x];
		
		// creating pixel data
		volatile uint32_t data = 0;
		data |= (((p1 >> 5) & 7) > ch);
		data |= (((p2 >> 5) & 7) > ch) << 5;
		data |= (((p1 >> 2) & 7) > ch) << 1;
		data |= (((p2 >> 2) & 7) > ch) << 6;
		data |= ((p1 & 3) > (ch >> 1)) << 2;
		data |= ((p2 & 3) > (ch >> 1)) << 7;
		data |= ((data ^ 0xe7) << 0x10);
		
		PORT->BSRR = data; // WRITE PIXEL DATA
		HAL_GPIO_WritePin(PORT, 1 << 13, GPIO_PIN_RESET);
		wait();
		HAL_GPIO_WritePin(PORT, 1 << 13, GPIO_PIN_SET);
	}
	HAL_GPIO_WritePin(PORT, 1 << 15, GPIO_PIN_SET);
	HAL_GPIO_WritePin(PORT, 1 << 14, GPIO_PIN_SET);
	wait();
	HAL_GPIO_WritePin(PORT, 1 << 14, GPIO_PIN_RESET);
	
	// select line
	uint32_t data = line << 8;
	data |= ((line ^ 0x1f) << 8 << 0x10);
	PORT->BSRR = data; // WRITE LINE DATA

	HAL_GPIO_WritePin(PORT, 1 << 15, GPIO_PIN_RESET);
}

void Display_Draw(void)
{
	for (int ch = 0; ch < CHANNELS; ++ch)
		for (int line = 0; line < SCAN; ++line)
			Display_DrawLine(line, ch);
}


void Display_Init(void)
{
	_display_CurrentLine = 0;
	_display_CurrentChannel = 0;
	for (int i = 0; i < 64; ++i)
		for (int j = 0; j < 64; ++j)
			pixels[j][i] = 0;
	GPIO_InitTypeDef GPIO_InitStruct;
	__HAL_RCC_GPIOE_CLK_ENABLE();
	HAL_GPIO_WritePin(GPIOE, 57319, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOE, 8192, GPIO_PIN_SET);
	GPIO_InitStruct.Pin = 65511;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    __HAL_RCC_TIM13_CLK_ENABLE();

	htimer.Instance = TIM13;
	htimer.Init.Prescaler = 2187;
	htimer.Init.CounterMode = TIM_COUNTERMODE_UP;
	htimer.Init.Period = 1;
	htimer.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	if (HAL_TIM_Base_Init(&htimer) != HAL_OK)
	{
	}

    HAL_NVIC_SetPriority(TIM8_UP_TIM13_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM8_UP_TIM13_IRQn);

	HAL_TIM_Base_Start_IT(&htimer);
}

void TIM8_UP_TIM13_IRQHandler(void)
{
	HAL_TIM_IRQHandler(&htimer);
	Display_DrawLine(_display_CurrentLine, _display_CurrentChannel);

	_display_CurrentLine++;
	if (_display_CurrentLine >= SCAN)
	{
		_display_CurrentLine = 0;
		_display_CurrentChannel++;
		if (_display_CurrentChannel >= CHANNELS)
		{
			_display_CurrentChannel = 0;
		}	
	}
}



STATIC mp_obj_t ledmatrix_init(void) {
	Display_Init();
    return mp_obj_new_int((uint32_t)pixels);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(ledmatrix_init_obj, ledmatrix_init);


STATIC const mp_map_elem_t ledmatrix_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_ledmatrix) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_init), (mp_obj_t)&ledmatrix_init_obj },
};

STATIC MP_DEFINE_CONST_DICT (
    mp_module_ledmatrix_globals,
    ledmatrix_globals_table
);

const mp_obj_module_t mp_module_ledmatrix = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_ledmatrix_globals,
};