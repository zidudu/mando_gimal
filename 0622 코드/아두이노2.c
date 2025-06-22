#include <avr/io.h>         // AVR 레지스터 정의용 헤더
#include <avr/interrupt.h>  // 인터럽트 사용을 위한 헤더

// === 설정값 정의 ===
const int FRAME_N = 1000;                 // 샘플 수: 한 프레임당 1000개의 데이터를 수집
volatile uint16_t samples[FRAME_N];       // 샘플 데이터를 저장할 배열 (10비트 ADC 결과 → uint16_t)
volatile int idx = 0;                     // 현재 샘플 저장 위치 인덱스
volatile bool ready = false;             // 1000개 수집 완료 여부를 나타내는 플래그
                                          // true가 되면 loop()에서 전송 시작

void setup() {
  Serial.begin(115200); // 시리얼 통신 시작 (115200 baud rate)

  // === ADC 설정 ===
  ADMUX = (1 << REFS0);  // AVcc를 기준 전압으로 사용, 입력 채널은 A0 (기본값)
  ADCSRA = (1 << ADEN) |           // ADC 활성화
           (1 << ADIE) |           // ADC 변환 완료 인터럽트 활성화
           (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // ADC 분주비 128 → 16MHz/128 = 125kHz

  // === Timer2 설정 (8비트 타이머) ===
  TCCR2A = (1 << WGM21);   // CTC 모드 (비교 일치 시 타이머 리셋)
  TCCR2B = (1 << CS22);    // 분주비 64 → 16MHz / 64 = 250kHz 타이머 주파수
  OCR2A = 249;             // 비교 일치 값 설정: 250kHz / 250 = 1kHz → 1ms 간격
  TIMSK2 = (1 << OCIE2A);  // 타이머 비교 일치 인터럽트 활성화

  sei();  // 전역 인터럽트 허용
}

// === Timer2 인터럽트 서비스 루틴 (1ms마다 호출됨) ===
ISR(TIMER2_COMPA_vect) {
  if (idx < FRAME_N) {
    ADCSRA |= (1 << ADSC); // ADC 변환 시작 (다 되면 ADC_vect 호출됨)
  }
}

// === ADC 변환 완료 인터럽트 ===
ISR(ADC_vect) {
  samples[idx++] = ADC;  // 변환된 값을 배열에 저장
  if (idx >= FRAME_N) {
    ready = true;        // 1000개 수집 완료 → loop()에서 전송하도록 플래그 설정
  }
}

// === 메인 루프 ===
void loop() {
  if (ready) {             // 데이터 수집이 완료되었을 때만 실행
    noInterrupts();        // 전송 중에는 인터럽트를 잠시 끔 (데이터 충돌 방지)
    for (int i = 0; i < FRAME_N; i++) {
      Serial.println(samples[i]); // 1000개 데이터를 한 줄씩 전송
    }
    idx = 0;               // 인덱스 리셋 → 다음 프레임 수집 준비
    ready = false;         // 플래그 초기화
    interrupts();          // 인터럽트 다시 허용
  }
}
