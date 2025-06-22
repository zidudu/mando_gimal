#include <avr/io.h>
#include <avr/interrupt.h>

const uint16_t BUFFER_SIZE = 3000;            // 링버퍼 크기 = 3000 (≈3초 데이터)
volatile uint16_t buffer[BUFFER_SIZE];        // ADC 측정값 저장 버퍼 (0~1023)
volatile uint16_t writeIdx   = 0;             // 버퍼 쓰기 인덱스
volatile bool     bufferFull = false;         // 링버퍼 오버런 플래그

void setup() {
  Serial.begin(115200);                       // 시리얼 통신 시작

  // === ADC Free-Run 모드 ===
  ADMUX  = _BV(REFS0);                        // AVcc 참조, 채널0 (A0)
  ADCSRA = _BV(ADEN)                          // ADC 활성화
         | _BV(ADPS2)|_BV(ADPS1)|_BV(ADPS0)   // 분주 128 (≈125kHz ADC 클럭)
         | _BV(ADATE);                       // 자동 트리거 (Free-Run)
  ADCSRB = 0;                                 // Free-Run 모드 선택
  ADCSRA |= _BV(ADSC);                        // 첫 변환 시작

  // === Timer2 CTC: 1 kHz 인터럽트 설정 ===
  TCCR2A = _BV(WGM21);                        // CTC 모드
  TCCR2B = _BV(CS22);                         // 분주 64 → 250kHz 타이머 클럭
  OCR2A  = 249;                               // 250kHz / (249+1) = 1kHz
  TIMSK2 = _BV(OCIE2A);                       // 비교 일치 인터럽트 허용

  sei();                                      // 전역 인터럽트 허용
}

// 1ms마다 호출: 최신 ADC 값을 링버퍼에 저장
ISR(TIMER2_COMPA_vect) {
  buffer[writeIdx++] = ADC;                  // ADC 레지스터 값을 저장
  if (writeIdx >= BUFFER_SIZE) {             // 인덱스 래핑 & 풀 플래그 설정
    writeIdx   = 0;
    bufferFull = true;
  }
}

void loop() {
  static uint16_t readIdx = 0;

  // 읽을 데이터가 있으면 시리얼 전송
  if (readIdx != writeIdx || bufferFull) {
    Serial.println(buffer[readIdx++]);        // ASCII로 한 줄씩 출력
    if (readIdx >= BUFFER_SIZE) {             // 래핑 시 플래그 리셋
      readIdx    = 0;
      bufferFull = false;
    }
  }
}
