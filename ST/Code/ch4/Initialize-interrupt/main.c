#include <stm32f091xc.h>

#define SW1_POS (1<<13)
#define SW2_POS (1<<7)

//#define SW1_POS GPIO_PIN_13 // (1<<13)
//#define SW2_POS GPIO_PIN_7 // (1<<7)


#define W_DELAY_SLOW 4
#define W_DELAY_FAST 2
#define RGB_DELAY_SLOW 40
#define RGB_DELAY_FAST 10
#define SWITCH_PRESSED(pin) (GPIOC->IDR & (pin))

volatile uint8_t g_flash_LED = 0;
volatile uint32_t g_w_delay = W_DELAY_SLOW;
volatile uint32_t g_RGB_delay = RGB_DELAY_SLOW;

void Init_GPIO(void) {
	// Enable peripheral clock of GPIOC
	RCC->AHBENR |= RCC_AHBENR_GPIOCEN;

	// Configure PC13 and PC7 in input mode
	GPIOC->MODER &= ~GPIO_MODER_MODER13;
	GPIOC->MODER &= ~GPIO_MODER_MODER7;

	// Enable pull-ups
	GPIOC->PUPDR &= ~GPIO_PUPDR_PUPDR13;
	GPIOC->PUPDR &= ~GPIO_PUPDR_PUPDR7; 
	GPIOC->PUPDR |= GPIO_PUPDR_PUPDR13_0;
	GPIOC->PUPDR |=  GPIO_PUPDR_PUPDR7_0; 
}

void Init_Interrupt(void) {
	  /* Listing 4.3 */
		EXTI->IMR |= EXTI_IMR_MR13 | EXTI_IMR_MR7; // Set interrupt mask register bits
		EXTI->FTSR |= EXTI_FTSR_TR13 | EXTI_FTSR_TR7; // Enable falling trigger 
		EXTI->RTSR |= EXTI_RTSR_TR13 | EXTI_RTSR_TR7; // Enable rising trigger 
	
	  /* Listing 4.4 */
    // Enable the clock of SYSCFG
	 	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
		// Connect PC7 to the external interrupt line
		SYSCFG->EXTICR[1] |= SYSCFG_EXTICR2_EXTI7_PC;
		// Connect PC13 to the external interrupt line
		SYSCFG->EXTICR[3] |= SYSCFG_EXTICR4_EXTI13_PC;
	
		// Note: added to test
		NVIC_SetPriority(EXTI4_15_IRQn, 3);
    NVIC_ClearPendingIRQ(EXTI4_15_IRQn);
    NVIC_EnableIRQ(EXTI4_15_IRQn); 
    
    /* Optional: Configure PRIMASK in case interrupts were disabled */
    __enable_irq();
}

void EXTI4_15_IRQHandler(void) {
    if ((EXTI->PR & SW1_POS) != 0) {
		EXTI->PR = SW1_POS;  // clear pending request
        if (SWITCH_PRESSED(SW1_POS)) {
            g_flash_LED = 1; // Flash white
        } else {
            g_flash_LED = 0; // RGB sequence
				}
    } 
	if ((EXTI->PR & SW2_POS) != 0) {
		EXTI->PR = SW2_POS; // clear pending request
		if (SWITCH_PRESSED(SW2_POS)) {
			// Short delays
			g_w_delay = W_DELAY_FAST;
			g_RGB_delay = RGB_DELAY_FAST;
		} else {
			// Long delays
			g_w_delay = W_DELAY_SLOW;
			g_RGB_delay = RGB_DELAY_SLOW;
		}
	}
	EXTI->PR = ~0; // Clear all other pending interrupt requests for this handler
}

int main(void) {
    Init_GPIO();
	  Init_Interrupt();
	  while (1);
}
