#include <avr/io.h>
#include <avr/interrupt.h>

volatile uint16_t rawSample = 0;   // 필터링된 출력 저장
volatile bool     ready     = false; // 출력 준비 플래그
float             yFilt     = 0;    // IIR 필터 내부 상태
const float       alpha     = 0.1;  // 필터 계수 (작을수록 더 부드러움)

// Timer1 CTC 모드: OCR1A 일치 시 1 kHz (1 ms 주기)
ISR(TIMER1_COMPA_vect) {
  ADCSRA |= _BV(ADSC);  // ADC 변환 시작
}

// ADC 변환 완료 인터럽트
ISR(ADC_vect) {
  uint16_t v = ADC;         // 10비트 ADC 값
  // IIR 1차 필터 적용
  yFilt = alpha * v + (1.0f - alpha) * yFilt;
  rawSample = (uint16_t)yFilt;
  ready = true;
}

void setup() {
  Serial.begin(115200);     // 시리얼 속도 설정

  // ADC 설정 (AVcc 기준, A0 입력)
  ADMUX  = _BV(REFS0);      
  ADCSRA = _BV(ADEN)        // ADC 활성화
         | _BV(ADIE)        // ADC 완료 인터럽트 허용
         | _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0); // 분주비 128

  // Timer1: 1ms 간격 설정
  noInterrupts();
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;
  OCR1A  = 249;              // 16MHz / 64 = 250kHz → 250 ticks = 1ms
  TCCR1B |= _BV(WGM12);      // CTC 모드
  TCCR1B |= _BV(CS11) | _BV(CS10); // 분주비 64
  TIMSK1 |= _BV(OCIE1A);     // 인터럽트 허용
  interrupts();
}

void loop() {
  if (ready) {
    ready = false;
    Serial.println(rawSample);  // 필터링된 값 출력
  }
}
