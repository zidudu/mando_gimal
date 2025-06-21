port = "COM7";              % 아두이노 포트 번호 확인 후 수정
baud = 115200;
FRAME_N = 1000;             % 프레임당 샘플 수
BYTES   = 2 * FRAME_N;      % 수신 바이트 수 (2바이트 * 1000 샘플)
Fs = 1000;                  % 샘플링 주파수 1kHz

% 시리얼 포트 설정
s = serialport(port, baud);
flush(s);
configureTerminator(s, "LF");
s.Timeout = 10;

% 누적 데이터 초기화
t_all = [];
x_all = [];

% 슬라이딩 윈도우 설정 (최근 5초 유지)
maxSeconds = 5;
maxSamples = maxSeconds * Fs;

disp("▶ 실시간 데이터 수신 시작...");

%% 실시간 수신 루프
while true
    % 1. 헤더 동기화
    if read(s, 1, "uint8") ~= 0xAA, continue; end
    if read(s, 1, "uint8") ~= 0x55, continue; end

    % 2. 데이터 수신
    raw = read(s, BYTES, "uint8");
    y = typecast(uint8(raw), "uint16");
    x = double(y);  % DC 제거는 누적 후 한 번만

    % 3. 시간 누적
    t = (0:FRAME_N-1) / Fs + length(t_all) / Fs;
    t_all = [t_all, t];
    x_all = [x_all, x];

    % 4. 최근 5초만 유지 (슬라이딩 윈도우)
    if length(t_all) > maxSamples
        t_all = t_all(end-maxSamples+1:end);
        x_all = x_all(end-maxSamples+1:end);
    end

    % 5. 그래프 출력
    figure(1); clf;

    % 시간 영역
    subplot(2,1,1);
    plot(t_all, x_all - mean(x_all), 'b');  % 전체 평균 기준 DC 제거
    xlabel("Time (s)");
    ylabel("Amplitude");
    title("Time Domain (최근 5초)");
    grid on;

    % 주파수 영역 (최근 프레임 기준)
    Nfft = 2^nextpow2(FRAME_N);
    X = fft((x - mean(x)) .* hamming(FRAME_N)', Nfft);  % 개별 프레임 기준 DC 제거
    f = (0:Nfft/2-1) * Fs / Nfft;

    subplot(2,1,2);
    plot(f, abs(X(1:Nfft/2)), 'r');
    xlabel("Frequency (Hz)");
    ylabel("Magnitude");
    title("Frequency Domain (최근 프레임)");
    grid on;

    drawnow;
end
