const int   FRAME_N  = 1000;          // 한 프레임당 샘플 수
const float Fs  = 300.0;         // 샘플링 레이트 [Hz]
const unsigned long interval = 1000000UL / Fs; // 샘플링 간격 [µs] ≒ 3333µs
int samples[FRAME_N];             // 샘플 저장 버퍼
int idx = 0;
unsigned long lastMicros;

void setup() {
  Serial.begin(115200);
  pinMode(A0, INPUT);
  lastMicros = micros();
}

void loop() {
  unsigned long now = micros();

  // 지정한 interval 간격(≒3333µs)마다 한 번 샘플링
  if (now - lastMicros >= interval) {
    lastMicros += interval;               // 다음 기준 시점
    samples[idx++] = analogRead(A0);      // A0 읽어서 버퍼에 저장

    // 버퍼가 가득 차면 한 번에 전송
    if (idx >= FRAME_N) {
      for (int i = 0; i < FRAME_N; i++) {
        Serial.println(samples[i]);       // ASCII 텍스트로 1000줄 전송
      }
      idx = 0;                            // 인덱스 리셋
    }
  }
}
