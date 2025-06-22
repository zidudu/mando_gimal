#include <avr/io.h>
#include <avr/interrupt.h>

const int FRAME_N = 1000;
volatile uint16_t samples[FRAME_N];
volatile int idx = 0;
volatile bool ready = false;

void setup() {
  Serial.begin(115200);

  // ADC 설정 (A0)
  ADMUX = (1 << REFS0);               // AVcc 기준, A0 입력
  ADCSRA = (1 << ADEN) |              // ADC 활성화
           (1 << ADIE) |              // ADC 인터럽트 허용
           (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // 분주비 128 → 125kHz

  // Timer2 설정 (CTC 모드, 1ms 간격)
  TCCR2A = (1 << WGM21);              // CTC 모드
  TCCR2B = (1 << CS22);               // 분주비 64 (16MHz / 64 = 250kHz)
  OCR2A = 249;                        // 250kHz / 250 = 1kHz → 1ms
  TIMSK2 = (1 << OCIE2A);             // 비교 일치 인터럽트 허용

  sei();  // 전역 인터럽트 허용
}

ISR(TIMER2_COMPA_vect) {
  if (idx < FRAME_N) {
    ADCSRA |= (1 << ADSC); // ADC 변환 시작
  }
}

ISR(ADC_vect) {
  samples[idx++] = ADC;  // 10비트 ADC 결과 저장
  if (idx >= FRAME_N) {
    ready = true;  // 모두 샘플링 완료 플래그
  }
}

void loop() {
  if (ready) {
    noInterrupts();  // 전송 중 인터럽트 방지
    for (int i = 0; i < FRAME_N; i++) {
      Serial.println(samples[i]);
    }
    idx = 0;
    ready = false;
    interrupts();  // 인터럽트 다시 허용
  }
}
