port = "COM7";
baud = 115200;
FRAME_N = 1000;
BYTES   = 2 * FRAME_N;  % uint16 * 1000
Fs = 1000;

s = serialport(port, baud);
flush(s);

disp("▶ 데이터 수신 대기 중...");

while true
    % 1. 헤더 동기화
    if read(s, 1, "uint8") ~= 0xAA, continue; end
    if read(s, 1, "uint8") ~= 0x55, continue; end

    % 2. 데이터 수신
    raw = read(s, BYTES, "uint8");
    y = typecast(uint8(raw), "uint16");  % 1 x 1000

    % 3. DC 제거 + FFT
    x = double(y) - mean(y);
    Nfft = 2^nextpow2(FRAME_N);
    X = fft(x .* hamming(FRAME_N)', Nfft);
    f = (0:Nfft/2-1) * Fs / Nfft;

    % 4. 그래프 출력
    figure(1); clf;
    subplot(2,1,1);
    plot((0:FRAME_N-1)/Fs, x);
    title("Time Domain"); xlabel("Time (s)"); ylabel("Amplitude");

    subplot(2,1,2);
    plot(f, abs(X(1:Nfft/2)));
    title("Frequency Domain"); xlabel("Frequency (Hz)"); ylabel("Magnitude");

    drawnow;
end
