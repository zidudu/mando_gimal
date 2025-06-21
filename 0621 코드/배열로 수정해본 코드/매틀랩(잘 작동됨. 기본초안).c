%% 설정
port     = "COM7";          
baud     = 115200;
FRAME_N  = 1000;            
Fs       = 1000;            

% 시리얼 열기
s = serialport(port, baud);
configureTerminator(s,"LF");
flush(s); pause(0.2);

% 시간축 미리 계산
t = (0:FRAME_N-1)/Fs;

% === Figure & Plot 핸들러 생성 ===
figure('Name','Real-time','NumberTitle','off');

% — 시간영역
subplot(2,1,1);
hTime = plot(t, zeros(size(t)), 'b','LineWidth',1);
xlabel("Time (s)"); ylabel("Amplitude");
title("Filtered Signal");
grid on;
% y축 초기 범위 넉넉히 잡아 두면 깜빡임 줄어듭니다
ylim([0 1023]);

% — 주파수영역
subplot(2,1,2);
hFreq = plot(nan, nan, 'r','LineWidth',1);
xlabel("Frequency (Hz)"); ylabel("Magnitude");
title("Spectrum");
grid on;
xlim([0 Fs/2]);

drawnow;

%% 실시간 루프
while true
    % 1) 한 프레임(1000줄) 읽기
    x = zeros(1,FRAME_N);
    for i=1:FRAME_N
        x(i) = str2double(readline(s));
    end

    % 2) 간단 필터(예: 5포인트 이동평균)
  x_filt = filter(ones(1,5)/5, 1, x);

    % filtfilt 사용 시 초기 과도 크게 완화됨
%x_filt = filtfilt(ones(1,5)/5, 1, x);


    % 3) FFT 계산
    Nfft = 2^nextpow2(FRAME_N);
    X    = fft(x_filt, Nfft);
    P2   = abs(X/FRAME_N);
    P1   = P2(1:Nfft/2+1);
    P1(2:end-1) = 2*P1(2:end-1);
    f    = Fs*(0:(Nfft/2))/Nfft;

    % 4) 시간영역 갱신
    set(hTime, "YData", x_filt);
    % y축 스케일을 실제 데이터 범위에 맞춰 자동으로 조정
    ylim([min(x_filt)-10, max(x_filt)+10]);

    % 5) 주파수영역 갱신
    set(hFreq, "XData", f, "YData", P1);
    % 필요하면 y축도 tight
    % ylim([0, max(P1)*1.1]);

    drawnow limitrate;
end
