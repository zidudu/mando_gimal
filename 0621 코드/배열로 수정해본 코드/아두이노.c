const int FRAME_N = 1000;       // 한 프레임당 샘플 수
int   samples[FRAME_N];         // 샘플 저장 배열
int   idx = 0;                  // 현재 저장 인덱스

void setup() {
  Serial.begin(115200);         // 시리얼 통신 시작
}

void loop() {
  // 1) 샘플 수집
  samples[idx++] = analogRead(A0);  
  delay(1);                       // 대략 1 ms 대기 → 약 1 kHz 샘플링

  // 2) 배열이 가득 찼으면 전송
  if (idx >= FRAME_N) {
    for (int i = 0; i < FRAME_N; i++) {
      Serial.println(samples[i]); // 한 줄씩 숫자 전송
    }
    idx = 0;                      // 인덱스 초기화하고 다음 프레임 준비
  }
}
