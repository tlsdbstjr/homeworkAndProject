# introduce

이 프로젝트는 24학년도 1학기 김태석 교수님의 임베디드 소프트웨어 설계 과목에서 진행한 개인 프로젝트입니다.

uC/OS-II의 Win32 포트를 사용해 자동차 보조장치 시뮬레이터를 구현했으며, 속도, RPM, 기어 상태를 주기적으로 계산해 화면에 표시합니다. 사용자는 키보드로 브레이크, 가속 페달, 기어를 조작할 수 있고, 시스템은 급발진처럼 보이는 상황을 감지하면 긴급 제동과 경고 메시지를 수행합니다.

# calculation

이 프로젝트는 `rpm`, `gear_ratio`, `break_pad`, `accel_pad` 값을 이용해 차량 상태를 단순 모델로 계산합니다. 목적은 실제 차량 물리를 정밀하게 재현하는 것보다, RTOS 환경에서 입력 처리, 계산, 동기화, 경고 전달 흐름을 분리해 보는 데 있습니다.

RPM과 속도는 아래 순서로 반복 갱신됩니다.

1. 가속 페달

   `rpmhandler()`는 가속 입력을 RPM 증가량으로 반영합니다.

   ```cpp
   rpm += (5 - (gear_state >> 8)) * accel_pad * 3;
   ```

   내부 단수 인덱스가 낮을수록 같은 가속 입력에서 RPM이 더 크게 증가합니다. RPM은 최대 7000, 최소 600으로 제한됩니다.

2. 자연 감속

   가속 입력이 없거나 작을 때는 RPM이 자연스럽게 감소하도록 보정합니다.

   ```cpp
   rpm -= (short)round(exp((double)rpm / 1000)) + gear_ratio[gear_state >> 8];
   ```

   지수 함수 기반 감쇠를 사용하기 때문에 RPM이 높을수록 감소량도 커집니다.

3. 기어비 기반 속도 계산

   `gearhandler()`는 현재 기어 상태에 맞는 기어비로 속도를 계산합니다.

   | 단수 | 기준 rpm | 속도(km/h) | 기어비(speed / rpm) |
   | --- | --- | --- | --- |
   | 1단 | 2000 | 15 | 0.0075 |
   | 2단 | 2000 | 30 | 0.015 |
   | 3단 | 2000 | 50 | 0.025 |
   | 4단 | 2000 | 80 | 0.04 |
   | 5단 | 2000 | 115 | 0.0575 |

   ```cpp
   speed = rpm * gear_ratio[gear_state >> 8];
   ```

   `D` 모드에서는 `rpmLimit` 배열을 기준으로 자동 변속하고, `M` 모드에서는 `+`, `-` 입력으로 수동 변속합니다. `R`은 1단 기어비를 사용하고 `P`에서는 속도를 0으로 둡니다.

4. 브레이크

   `speedhandler()`는 브레이크 입력을 속도 감소에 반영합니다.

   ```cpp
   speed -= break_pad;
   ```

   브레이크 단계가 클수록 속도가 더 빨리 감소하며, 속도는 0 아래로 내려가지 않습니다.

5. 속도와 RPM 동기화

   브레이크 등으로 속도가 바뀌면 현재 기어 상태에 맞춰 RPM도 다시 계산합니다.

   ```cpp
   rpm = speed / gear_ratio[gear_state >> 8];
   ```

   이 동기화는 `R`, `D`, `M` 상태에서 수행되며, 동기화 이후에도 RPM은 최소 600으로 유지됩니다.

# structure

## data flow

프로그램은 여러 태스크가 전역 상태를 공유하는 구조로 동작합니다.

- `speed`: 현재 속도
- `rpm`: 현재 엔진 RPM
- `break_pad`: 브레이크 입력 단계(0~9)
- `accel_pad`: 가속 입력 단계(0~9)
- `gear_in`: 최근 기어 입력 값
- `gear_state`: 현재 기어 문자(`P`, `R`, `N`, `D`, `M`)와 내부 단수 인덱스를 함께 담는 상태값

키 입력 관련 상태는 `inputSem` 세마포어로 보호합니다. 급발진 경고는 `msgQueue` 메시지 큐를 통해 전달되고, 출력 태스크가 이를 받아 경고 문구를 표시합니다.

데이터 흐름은 아래와 같습니다.

1. 사용자 입력이 `break_pad`, `accel_pad`, `gear_in`에 반영됩니다.
2. `rpmhandler()`가 가속 입력과 자연 감속을 반영해 RPM을 갱신합니다.
3. `gearhandler()`가 현재 기어 상태에 맞춰 속도를 계산하고 자동 또는 수동 변속을 처리합니다.
4. `speedhandler()`가 브레이크 입력으로 속도를 줄이고, 변경된 속도에 맞춰 RPM을 다시 동기화합니다.
5. `eventhandler()`가 급발진 패턴을 감지하면 `eventWriter()`를 호출하고 메시지 큐로 경고를 전송합니다.
6. `monitorPrinter()`가 속도, RPM, 페달 상태, 기어, 경고 메시지를 화면에 출력합니다.

## task flow

`main()`에서는 총 7개의 태스크를 생성합니다. uC/OS-II에서는 숫자가 작을수록 우선순위가 높습니다.

- `inputhandler()` priority 30: 키보드 입력을 읽어 브레이크, 가속, 기어 변경 요청을 반영합니다.
- `SUAhandler()` priority 29: `1` 입력 시 급발진 시나리오를 시작하고, `2` 입력 전까지 RPM을 강제로 올립니다.
- `monitorPrinter()` priority 20: 현재 상태를 텍스트 UI로 출력합니다.
- `eventhandler()` priority 19: 가속 페달 입력이 0인데 속도가 3주기 연속 증가하면 급발진으로 판단하고 긴급 제동을 트리거합니다.
- `rpmhandler()` priority 10: 가속 입력과 자연 감속을 반영해 RPM을 계산합니다.
- `gearhandler()` priority 11: 기어 상태를 갱신하고 기어비에 따라 속도를 계산합니다.
- `speedhandler()` priority 12: 브레이크 입력을 반영해 속도를 낮추고 RPM을 다시 맞춥니다.

급발진 이벤트가 감지되면 `eventWriter()`가 호출되어 `break_pad = 9`로 설정됩니다. 이후 `eventhandler()`는 메시지 큐에 경고를 넣고, `monitorPrinter()`는 이를 받아 화면 하단에 `Emergency break opened!`를 표시합니다.

`eventWriter()` 주석에는 외부 저장소 기록이 언급되어 있지만, 현재 구현에서는 파일 기록 없이 긴급 제동 트리거 역할만 수행합니다.

# execution

이 프로젝트는 uC/OS-II Win32 포트 환경을 전제로 합니다. `source/makefile` 기준 빌드 전제조건은 다음과 같습니다.

- `C:\SOFTWARE\uCOS-II\SOURCE`
- `C:\SOFTWARE\uCOS-II\Ports\80x86\WIN32\VC\src`
- Microsoft Visual C++ 컴파일러 `cl`
- 링크 라이브러리 `winmm.lib`, `user32.lib`

빌드는 `source` 디렉터리에서 아래 명령으로 수행합니다.

```make
all:
	@cl -nologo /MD /I$(UCOS_SRC) /I$(UCOS_PORT_SRC) /I$(UCOS_PORT_EX) $(EXAMPLE) $(UCOS_SRC)\ucos_ii.c $(UCOS_PORT_SRC)\pc.c $(UCOS_PORT_SRC)\os_cpu_c.c winmm.lib user32.lib
```

실행 중 키 조작은 아래와 같습니다.

- `R`, `F`: 브레이크 단계 증가, 감소
- `T`, `G`: 가속 단계 증가, 감소
- `7`: Parking
- `Y`: Reverse
- `H`: Neutral
- `N`: Drive
- `J`: 수동 모드에서 기어 증가
- `M`: 수동 모드에서 기어 감소
- `1`: 급발진 시나리오 시작
- `2`: 급발진 시나리오 종료
- `Esc`: 프로그램 종료

화면에는 속도 바, RPM 바, 브레이크 및 가속 상태, 현재 기어 문자와 단수, 그리고 급발진 감지 시 경고 메시지가 표시됩니다.

`source/main.exe`와 `source/*.obj`는 빌드 산출물입니다. 구현 확인은 `main.c`, `os_cfg.h`, `makefile`을 기준으로 보는 것이 좋습니다.
