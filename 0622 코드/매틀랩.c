%% 설정
port     = "COM5";          
baud     = 115200;
FRAME_N  = 1000;            
Fs       = 1000;            

% 시리얼 열기
%% 시리얼 포트를 열고, 줄바꿈 기준을 설정한 후 이전 데이터는 초기화합니다.
s = serialport(port, baud);
configureTerminator(s,"LF"); % 아두이노가 한 줄을 보냈다는 기준을 설정한다
flush(s); % 기존에 쌓여 있던 시리얼 버퍼 데이터를 모두 비워줍니다.
pause(0.2); % 0.2초 동안 잠시 대기하면서 안정적으로 연결될 시간을 줍니다.

% 시간축 미리 계산
t = (0:FRAME_N-1)/Fs; % 시간 영역 신호 
% "샘플이 찍힌 시각을 계산해서 시간축을 만든다."

% === Figure & Plot 핸들러 생성 ===
figure('Name','Real-time','NumberTitle','off');

% — 시간영역
subplot(2,1,1); % 위쪽에는 시간 영역 신호를 그립니다.
%처음에는 0으로 채운 빈 신호로 초기화하고 (zeros(size(t))),
%그래프 색은 파랑('b'), 선 두께는 1
hTime = plot(t, zeros(size(t)), 'b','LineWidth',1);
xlabel("Time (s)"); ylabel("Amplitude");
title("Filtered Signal");
grid on;
% y축 초기 범위 넉넉히 잡아 두면 깜빡임 줄어듭니다
ylim([0 1023]);

% — 주파수영역 FFT
subplot(2,1,2); % FFT를 계산한 주파수 스펙트럼을 표시
%초기에는 데이터가 없어서 nan으로 초기화함 (그래프 오류 방지용)
hFreq = plot(nan, nan, 'r','LineWidth',1); % 색은 빨강 ('r'), 선 두께는 1
xlabel("Frequency (Hz)"); ylabel("Magnitude");
title("Spectrum");
grid on;
xlim([0 Fs/2]); %x축 범위는 0 ~ Fs/2 (나이퀴스트 주파수까지)

%지금까지 설정한 그래프를 한 번 화면에 그려줍니다.
drawnow;

%% 실시간 루프
%아두이노에서 들어오는 1000개의 데이터를 실시간으로 읽고,
%간단한 이동 평균 필터를 적용해 신호를 부드럽게 만드는 과정
while true
    % 1) 한 프레임(1000줄) 읽기
    x = zeros(1,FRAME_N);
    for i=1:FRAME_N
        % 문자열로 받은 숫자를 실제 숫자(double)로 변환
        %→ 변환된 숫자를 x 배열에 하나씩 저장
        x(i) = str2double(readline(s)); %  시리얼 포트 s에서 한 줄 읽음
    end

    % 2) 간단 필터(예: 5포인트 이동평균)
    % ones(1,5)/5 → [1/5 1/5 1/5 1/5 1/5]
    % → 5개 데이터를 평균 내는 필터 커널
    % MATLAB의 디지털 필터 함수
  x_filt = filter(ones(1,5)/5, 1, x); % MATLAB의 디지털 필터 함수
 % => 매 5개의 값을 평균 내서, 신호를 부드럽게 만든다. 노이즈를 줄이는 간단한 필터다

% filtfilt 사용 시 초기 과도 크게 완화됨
%x_filt = filtfilt(ones(1,5)/5, 1, x);


    % 3) FFT 계산
    %FFT는 2의 거듭제곱 크기일 때 속도가 가장 빠르기 때문에 이렇게 설정합니다
    Nfft = 2^nextpow2(FRAME_N); %  1000 → 2의 거듭제곱으로 보정
    %x_filt 신호에 대해 **FFT(고속 푸리에 변환)**을 수행합니다.
    %이 결과는 복소수 배열이며, 진폭 + 위상 정보가 들어있습니다.
    X    = fft(x_filt, Nfft);
    %복소수 결과를 abs()로 절댓값(=진폭)만 남깁니다.
    P2   = abs(X/FRAME_N);
    %FFT는 양쪽 대칭(미러) 구조이므로 절반만 사용
    %가운데 빼고는 절반만 취하므로 2배 보정
    P1   = P2(1:Nfft/2+1);
    P1(2:end-1) = 2*P1(2:end-1);
    %각 점에 해당하는 **주파수 축(f)**을 계산합니다.
    f    = Fs*(0:(Nfft/2))/Nfft;

    % 4) 시간영역 갱신
    set(hTime, "YData", x_filt);
    % y축 스케일을 실제 데이터 범위에 맞춰 자동으로 조정
    ylim([min(x_filt)-10, max(x_filt)+10]);
 %즉, 이제 f와 P1을 함께 쓰면
%"몇 Hz에 어떤 에너지가 있는지" 주파수 분석이 가능합니다!
 
% 5) 주파수영역 갱신
    set(hFreq, "XData", f, "YData", P1);
    % 필요하면 y축도 tight
    % ylim([0, max(P1)*1.1]);

    drawnow limitrate;
end
