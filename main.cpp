#include "stm32f446xx.h"
#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>

// 115200 baud
// picocom -b 115200 /dev/ttyACM0
void enable_uart2() {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
    (void)RCC->APB1ENR;

    GPIOA->MODER &= ~((3u << 4) | (3u << 6));
    GPIOA->MODER |=  ((2u << 4) | (2u << 6));
    GPIOA->AFR[0] &= ~((0xFu << 8) | (0xFu << 12));
    GPIOA->AFR[0] |=  ((0x7u << 8) | (0x7u << 12));

    USART2->BRR = 139;
    USART2->CR1 = (1u << 3) | (1u << 2) | (1u << 13);
}

extern "C" {
  int _write(int file, char *ptr, int len) {
      (void)file;
      for (int i = 0; i < len; i++) {
          while (!(USART2->SR & (1u << 7)));
          USART2->DR = (uint8_t)ptr[i];
      }
      return len;
  }
  int _read(int file, char *ptr, int len) { (void)file; (void)ptr; (void)len; return 0; }
  int _close(int file) { (void)file; return -1; }
  int _fstat(int file, struct stat *st) { (void)file; st->st_mode = S_IFCHR; return 0; }
  int _isatty(int file) { (void)file; return 1; }
  int _lseek(int file, int ptr, int dir) { (void)file; (void)ptr; (void)dir; return 0; }
  int _kill(int pid, int sig) { (void)pid; (void)sig; errno = EINVAL; return -1; }
  int _getpid(void) { return 1; }
  void _exit(int status) { (void)status; while (1) { } }
  extern char _end;
  void *_sbrk(int incr) {
      static char *heap_end = &_end;
      char *prev = heap_end;
      heap_end += incr;
      return prev;
  }
}

constexpr uint32_t bit_shift_for_mode_gpioa = 10;
constexpr uint32_t output_data_register_shift = 5;
constexpr uint32_t PC4_MODE_SHIFT = 8;

void enable_port_a() { RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN; }
void enable_port_c() { RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN; }
void enable_syscfg_clock() { RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN; }

void set_LED_port_to_correct_mode() {
  GPIOA->MODER &= ~(3u << bit_shift_for_mode_gpioa);
  GPIOA->MODER |=  (1u << bit_shift_for_mode_gpioa);
}

void enable_LED2()  { GPIOA->ODR |=  (1u << output_data_register_shift); }
void disable_LED2() { GPIOA->ODR &= ~(1u << output_data_register_shift); }

void enable_external_button_input() { GPIOC->MODER &= ~(3u << PC4_MODE_SHIFT); }
void set_external_button_pull_up() {
    GPIOC->PUPDR &= ~(3u << 8);
    GPIOC->PUPDR |=  (1u << 8);
}

void set_external_interrupt_configuration_register_4() {
    SYSCFG->EXTICR[1] &= ~(15u);
    SYSCFG->EXTICR[1] |= 2u;
}
void set_exti_interrupt_mask() { EXTI->IMR  |= (1u << 4); }
void set_falling_trigger()     { EXTI->FTSR |= (1u << 4); }
void enable_nvic_exti4()       { NVIC_EnableIRQ(EXTI4_IRQn); }

void enable_timer2_us() {
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    (void)RCC->APB1ENR;
    TIM2->PSC = (16000000 / 1000000) - 1;
    TIM2->ARR = 0xFFFFFFFF;
    TIM2->EGR = (1u << 0);
    TIM2->CR1 |= (1u << 0);
}

uint32_t micros(void) { return TIM2->CNT; }

enum Gamestate { IDLE, WAIT_DELAY, LED_ON };
volatile Gamestate current_state = IDLE;
volatile uint32_t  t_start_us    = 0;
volatile uint32_t  score         = 0;

// EXTI4 interrupt handler for the button
extern "C" {
  void EXTI4_IRQHandler(void) {
    // We are in a state that expects a button press
    if (current_state == WAIT_DELAY) {
        score = micros() - t_start_us;
        current_state = LED_ON;
    }
    EXTI->PR = (1u << 4);
  }
}

void setup_interrupts_for_PC4() {
  enable_syscfg_clock();
  set_external_interrupt_configuration_register_4();
  set_exti_interrupt_mask();
  set_falling_trigger();
  enable_nvic_exti4();
}

int main() {
  enable_port_a();
  enable_port_c();
  set_LED_port_to_correct_mode();
  enable_external_button_input();
  set_external_button_pull_up();
  setup_interrupts_for_PC4();
  enable_uart2();
  enable_timer2_us();              // must come before any micros() call
  enable_LED2();                   // LED on

  printf("\r\n=== Reaction Game ===\r\n");
  printf("clock = %lu Hz\r\n", (unsigned long)SystemCoreClock);
  printf("Press the button as soon as you see the LED turn off!.\r\n\r\n");

  uint32_t rounds = 0;
  uint32_t best_us = 0xFFFFFFFFu;

  while (true) {
    // If we are waiting for the LED to turn off
    if (current_state == IDLE) {
      // Random delay between 500 ms and 2000 ms
      uint32_t target_delay_us = 500000 + (micros() % 1500000);
      uint32_t delay_start_us  = micros();

      // Blocking wait for the delay to expire
      while (micros() - delay_start_us < target_delay_us) { }

      // Ensure the ISR sees a consistent
      // (state, t_start_us) pair before the LED goes off
      __disable_irq();
      current_state = WAIT_DELAY;
      t_start_us    = micros();
      disable_LED2();
      __enable_irq();

      printf("Press!\r\n");
    }
    else if (current_state == LED_ON) {

      enable_LED2();              // back to "get ready"
      rounds++;
      if (score < best_us) best_us = score;

      printf("Reaction: %lu us  (%lu ms)   best: %lu ms   round %lu\r\n",
             (unsigned long)score,
             (unsigned long)(score / 1000),
             (unsigned long)(best_us / 1000),
             (unsigned long)rounds);

      // 1.5 s pause before the next round
      uint32_t pause_end = micros() + 1500000;
      while ((int32_t)(micros() - pause_end) < 0) { }
      printf("Get ready for the next round!\r\n");
      current_state = IDLE;
    }
  }
}