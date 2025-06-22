%% === 설정 ===
port    = "COM5";          
baud    = 115200;
FRAME_N = 3000;            
Fs      = 1000;            

% 시리얼 포트 열기
s = serialport(port, baud);
configureTerminator(s,"LF");
flush(s); pause(0.2);

% 시간축 미리 계산
t = (0:FRAME_N-1)/Fs;

%% === Figure & Plot 핸들러 생성 ===
figure('Name','Real-time','NumberTitle','off');

subplot(2,1,1);
hTime = plot(t, zeros(size(t)),'b','LineWidth',1);
xlabel("Time (s)"); ylabel("Amplitude");
title("Filtered Signal"); grid on; ylim([0 1023]);

subplot(2,1,2);
hStem = stem(nan,nan,'r','filled','LineWidth',1);
xlabel("Frequency (Hz)"); ylabel("Magnitude");
title("Spectrum (Top 3 Peaks)"); grid on;
xlim([0 20]); xticks(0:0.5:20);

drawnow;

%% === 실시간 루프 ===
while true
  % 1) 데이터 수신
  x = zeros(1,FRAME_N);
  for i=1:FRAME_N
    x(i) = str2double(readline(s));
  end

  % 2) 간단 필터 + DC 제거
  x_filt = filter(ones(1,5)/5, 1, x);
  x_filt = x_filt - mean(x_filt);  % 평균 빼기

  % 3) 시간영역 갱신
  set(hTime,'YData',x_filt);
  ylim([min(x_filt)-10, max(x_filt)+10]);

  % 4) FFT 계산
  Nfft = 2^nextpow2(FRAME_N);
  X    = fft(x_filt, Nfft);
  P2   = abs(X/FRAME_N);
  P1   = P2(1:Nfft/2+1);
  P1(2:end-1) = 2*P1(2:end-1);
  f    = Fs*(0:(Nfft/2))/Nfft;

  % 5) DC 성분 확실히 제거
  P1(1) = 0;

  % 6) 상위 3개 피크만 골라 Ppeak 생성
  % 관심 대역 0~20Hz
  idxROI = f<=20;
  fR     = f(idxROI);
  PR     = P1(idxROI);

  % 로컬 최대 검출
  locs = find( PR(2:end-1)>PR(1:end-2) & PR(2:end-1)>PR(3:end) ) + 1;
  if isempty(locs)
    Ppeak = zeros(size(P1));
  else
    % 진폭 내림차순 정렬하여 상위 3개 인덱스 선택
    [~,ord]   = sort(PR(locs),'descend');
    topLocs   = locs(ord(1:min(3,numel(ord))));
    PpeakROI  = zeros(size(PR));
    PpeakROI(topLocs) = PR(topLocs);

    % 전체 크기로 복원
    Ppeak = zeros(size(P1));
    Ppeak(idxROI) = PpeakROI;
  end

  % 7) stem 플롯 갱신
  set(hStem,'XData',f,'YData',Ppeak);

  drawnow limitrate;
end
