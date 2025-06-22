#include <avr/io.h>
#include <avr/interrupt.h>

// 1ms마다 1개씩 샘플링하면 약 3초간의 데이터를 저장할 수 있는 링버퍼입니다.
const uint16_t BUFFER_SIZE = 3000;            // 링버퍼 크기 = 3000 (≈3초 데이터)
// ADC(아날로그 디지털 변환기)로 읽은 값을 저장할 배열
volatile uint16_t buffer[BUFFER_SIZE];        // ADC 측정값 저장 버퍼 (0~1023)
// 데이터를 버퍼에 쓸 때 위치(인덱스)
volatile uint16_t writeIdx   = 0;             // 버퍼 쓰기 인덱스
// 한 바퀴를 돌았는지(=데이터가 가득 찼는지) 알려주는 플래그입니다.
volatile bool     bufferFull = false;         // 링버퍼 오버런 플래그

void setup() {
  Serial.begin(115200);                       // 시리얼 통신 시작

  // === ADC 수동 변환 모드 (Timer 트리거 사용) ===
  // REFS0 → 기준 전압은 AVcc 사용 (보통 5V)
  // ADATE 비활성화 → Free-Run 아님
  ADMUX  = _BV(REFS0);                        // AVcc 참조, 채널0 (A0)
  // ADEN: ADC 활성화, ADIE: 변환 완료 인터럽트 허용,
  // 분주 128: 약 125kHz ADC 클럭
  ADCSRA = _BV(ADEN)
         | _BV(ADIE)
         | _BV(ADPS2)|_BV(ADPS1)|_BV(ADPS0);
  ADCSRB = 0;                                 // ADATE=0: 수동 변환 모드

  // === Timer2 CTC: 1 kHz 인터럽트 설정 ===
  // 타이머2를 1kHz로 설정해서 1ms마다 인터럽트를 발생
  TCCR2A = _BV(WGM21);                        // CTC 모드
  TCCR2B = _BV(CS22);                         // 분주 64 → 250kHz 타이머 클럭
  OCR2A  = 249;                               // 250kHz/(249+1) = 1kHz => 1ms
  TIMSK2 = _BV(OCIE2A);                       // 비교 일치 인터럽트 허용

  sei();                                      // 전역 인터럽트 허용
}

// 1ms마다 호출: Timer2 인터럽트에서 ADC 변환 시작
ISR(TIMER2_COMPA_vect) {
  ADCSRA |= _BV(ADSC);    // ADC 변환 시작 (ADSC=1)
}

// ADC 변환 완료 시 호출: 정확히 1ms 후 약 104µs 뒤 실행
ISR(ADC_vect) {
  buffer[writeIdx++] = ADC;           // ADC 레지스터 값을 링버퍼에 저장
  if (writeIdx >= BUFFER_SIZE) {      // 인덱스 래핑 & 오버런 플래그 설정
    writeIdx   = 0;
    bufferFull = true;
  }
}

void loop() {
  static uint16_t readIdx = 0;

  // 읽을 데이터가 있으면 시리얼 전송
  // readIdx != writeIdx → 새 데이터가 들어왔다는 뜻
  // bufferFull == true → 버퍼가 한 바퀴 돌았던 상태
  if (readIdx != writeIdx || bufferFull) {
    Serial.println(buffer[readIdx++]);        // ASCII로 한 줄씩 출력
    if (readIdx >= BUFFER_SIZE) {             // 래핑 시 플래그 리셋
      readIdx    = 0;
      bufferFull = false;
    }
  }
}
