#include <avr/io.h>
#include <avr/interrupt.h>

const uint16_t BUFFER_SIZE = 3000;
volatile uint16_t buffer[BUFFER_SIZE];
volatile uint16_t writeIdx   = 0;
volatile bool     bufferFull = false;

void setup() {
  // 시리얼
  Serial.begin(115200);

  // 디버그용 PB5(LED) 설정
  DDRB |= _BV(DDB5);

  // === ADC Free-Run 모드 ===
  ADMUX  = _BV(REFS0);                              // AVcc, 채널0
  ADCSRA = _BV(ADEN)|_BV(ADPS2)|_BV(ADPS1)|_BV(ADPS0) // 분주128
         | _BV(ADATE);                             // 자동 트리거
  ADCSRB = 0;                                      // Free-Run
  ADCSRA |= _BV(ADSC);                             // 첫 변환 시작

  // === Timer2 CTC: 1 kHz ===
  TCCR2A = _BV(WGM21);                             // CTC 모드
  TCCR2B = _BV(CS22);                              // 분주64 → 250 kHz
  OCR2A  = 249;                                    // 250 kHz/250=1 kHz
  TIMSK2 = _BV(OCIE2A);                            // 비교 일치 인터럽트

  sei();                                           // 전역 인터럽트 허용
}

// 1 ms마다 호출 → 최신 ADC 결과를 buffer에 저장
ISR(TIMER2_COMPA_vect) {
  PORTB ^= _BV(PB5);             // (옵션) 디버그용 토글
  buffer[writeIdx] = ADC;        // ADC 레지스터에서 즉시 읽기
  writeIdx++;
  if (writeIdx >= BUFFER_SIZE) {
    writeIdx   = 0;
    bufferFull = true;
  }
}

void loop() {
  static uint16_t readIdx = 0;

  // 버퍼에 읽을 샘플이 있으면 바로 전송
  // (bufferFull 일 때 wrap-around도 처리)
  if (readIdx != writeIdx || bufferFull) {
    uint16_t v = buffer[readIdx];
    Serial.println(v);          // ASCII로 한 줄씩 출력
    readIdx++;
    if (readIdx >= BUFFER_SIZE) {
      readIdx = 0;
      bufferFull = false;
    }
  }
}
