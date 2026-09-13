# 신윤석 | Computer Engineering Projects

광운대학교에서 컴퓨터공학을 공부하며 수행한 과제와 개인 프로젝트를 정리한 저장소입니다. 각 설명은 현재 저장된 코드와 보고서에서 확인되는 구현 범위와 결과를 기준으로 작성했습니다.

## Projects

| 분야 | 프로젝트 | 구현 내용 | 사용 기술 |
| --- | --- | --- | --- |
| 임베디드 시스템 | [ZYNQ-Jetson Nano SPI 통신](./spi-communication/) | 조이스틱 입력 전달, LED·7-segment 제어, SPI 수신 오류 보정 | ZYNQ, AXI, SPI, C, Python, Linux device driver |
| RTOS | [uC/OS-II 차량 속도 시뮬레이터](./uCOS_car_speed_estimate/) | 7개 태스크로 차량 상태 계산과 비정상 가속 감지·제동 처리 | C, uC/OS-II, semaphore, message queue |
| 운영체제 | [System Call I/O Tracer](./systemCall_ftrace/) | 사용자 정의 시스템 콜과 커널 모듈로 프로세스의 파일 I/O 기록 | Linux kernel 4.19.67, C, system call hooking |
| 디지털 설계 | [64-bit Radix-4 Booth Multiplier](./booths_multiplier/) | 부호 있는 64-bit 곱셈기와 FSM·테스트벤치 설계 | Verilog HDL, Quartus Prime, Cyclone V |
| 어셈블리 | [Floating-point Convolution](./convolution_with_floating_point_number/) | IEEE 754 연산 루틴과 1차원 합성곱을 ARM Assembly로 구현 | ARM Assembly, Keil µVision, IEEE 754 |
| 영상 처리·수치해석 | [Adaptive Interpolation](./adaptive_interpolation/) | 방향성·활동성 분류와 최소제곱 기반 7×7 보간 필터 생성 | C, least squares, Lloyd-Max, PSNR |
| 영상 처리·수치해석 | [Image Interpolation](./ineterpolation/) | 네 가지 보간법을 직접 구현하고 동일 데이터셋에서 비교 | C, nearest neighbor, bilinear, bicubic, six-tap |

## Repository Structure

각 프로젝트 폴더에는 다음 자료가 포함되어 있습니다.

- 구현 소스와 프로젝트 설정 파일
- 구현 과정과 검증 결과를 기록한 보고서
- 핵심 구조, 실행 결과, 재현 조건을 정리한 README

> 이 저장소의 결과는 수업 과제와 개인 프로젝트 환경에서 얻은 값입니다. 각 프로젝트의 제약과 재현에 필요한 환경은 해당 README에 별도로 적었습니다.
