#include <avr/io.h>
#include <avr/interrupt.h>

const uint16_t BUFFER_SIZE = 3000;        // 약 3초치 샘플
volatile uint16_t buffer[BUFFER_SIZE];
volatile uint16_t writeIdx = 0;
volatile bool     bufferFull = false;

void setup() {
  Serial.begin(115200);

  // ADC 수동 변환 설정 (AVcc 참조, 채널0, 분주128, 변환 완료 인터럽트)
  ADMUX  = _BV(REFS0);
  ADCSRA = _BV(ADEN) | _BV(ADIE) | _BV(ADPS2)|_BV(ADPS1)|_BV(ADPS0);

  // Timer2 CTC 모드: 1kHz 인터럽트 발생
  TCCR2A = _BV(WGM21);
  TCCR2B = _BV(CS22);   // 16MHz/64 = 250kHz
  OCR2A  = 249;         // 250kHz / 250 = 1kHz
  TIMSK2 = _BV(OCIE2A);

  sei();  // 전역 인터럽트 허용
}

// 1ms마다 ADC 변환 시작
ISR(TIMER2_COMPA_vect) {
  ADCSRA |= _BV(ADSC);
}

// 변환 완료 시 링버퍼에 저장
ISR(ADC_vect) {
  buffer[writeIdx++] = ADC;
  if (writeIdx >= BUFFER_SIZE) {
    writeIdx   = 0;
    bufferFull = true;
  }
}

void loop() {
  static uint16_t readIdx = 0;
  // 새 데이터가 있으면 시리얼 전송
  if (readIdx != writeIdx || bufferFull) {
    Serial.println(buffer[readIdx++]);
    if (readIdx >= BUFFER_SIZE) {
      readIdx    = 0;
      bufferFull = false;
    }
  }
}
