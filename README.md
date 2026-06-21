# 동방홍마향 ~ the Embodiment of Scarlet Devil — PSP 포트

**한국어** | [日本語](README.ja.md) | [English](README.en.md)

[GensokyoClub/th06](https://github.com/GensokyoClub/th06)의 `portable` 브랜치를
기반으로 만든 **동방홍마향 ~ the Embodiment of Scarlet Devil**(TH06)의 실험적인
PSP 홈브류 포트입니다.

이 저장소에는 소스 코드만 포함됩니다. 원본 실행 파일, DAT 아카이브, 음악을
비롯한 저작권 보호 게임 에셋은 **포함하지 않습니다**. 정식으로 구입한 일본어판
PC 게임의 데이터가 필요합니다.

## 현재 상태

- 표준 PSP 홈브류 `EBOOT.PBP`로 실행됩니다.
- 타이틀 화면, 메뉴, 게임 플레이, 적, 플레이어 탄, 효과음, WAV 음악을 구현했습니다.
- 원본 640×480 화면을 정확히 1/2 크기인 320×240으로 렌더링하고, PSP의
  480×272 화면 중앙에 검은 여백을 둡니다.
- 32 MiB PSP-1000에 맞춰 텍스처, 스트리밍, 메모리 사용량을 조정했습니다.
- PSP-1000의 333/166 MHz 클록에서 60 Hz를 목표로 스프라이트 배치, VFPU 수학,
  오디오 믹싱을 최적화했습니다.
- 화면 찢어짐을 막기 위해 VBlank에 맞춰 프레임을 표시합니다.
- PPSSPP의 PSP-1000 메모리 모델로 테스트합니다. 릴리스마다 실기 테스트를 권장합니다.

아직 실험적인 포트입니다. 세이브와 리플레이 파일은 백업해 두세요.

## 요구 사항

- PowerShell과 WSL2(Ubuntu 기본)를 사용할 수 있는 Windows 10 또는 11
- `psp-cmake`가 포함된 [PSPDEV](https://github.com/pspdev/pspdev)
- PSP용 SDL2, SDL2_image, SDL2_ttf, PSPGL
- 다음 파일을 포함한 정식 TH06 설치본
  - `CM.DAT`, `ED.DAT`, `IN.DAT`, `MD.DAT`, `ST.DAT`, `TL.DAT`
  - `bgm/th06_01.wav`부터 `bgm/th06_17.wav`까지

패키징 스크립트는 원본 Windows 실행 파일을 복사하거나 실행하지 않습니다. 파일이
있을 경우 예상하지 못한 게임 버전을 알리기 위한 해시 확인에만 사용합니다.

## 빌드 및 패키징

저장소를 복제한 뒤 PowerShell에서 패키징 스크립트를 실행합니다.

```powershell
git clone https://github.com/jeong-jimin-github/PSP_TH06.git
cd PSP_TH06
./scripts/build_psp.ps1 `
  -SourceDir "D:\Games\TH06" `
  -Pspdev "/home/your-wsl-user/pspdev"
```

PSPDEV가 `/usr/local/pspdev`에 설치되어 있으면 `-Pspdev`를 생략할 수 있습니다.
WSL 배포판 이름이 `Ubuntu`가 아니면 `-WslDistro`를 지정하세요.

완성된 메모리 스틱용 디렉터리는 다음 위치에 생성됩니다.

```text
build/psp-package/PSP/GAME/TH06/
```

`TH06` 디렉터리 전체를 `ms0:/PSP/GAME/TH06/`에 복사하거나 PPSSPP에서
`EBOOT.PBP`를 실행하세요. 생성 패키지는 약 340 MiB입니다. 원본 DAT, WAV 음악과
PSP에서 메모리 사용량이 큰 DAT 해제를 피하기 위한 추출 에셋이 들어갑니다.

첫 빌드 후에는 컴파일을 생략하고 다시 패키징할 수 있습니다.

```powershell
./scripts/build_psp.ps1 -SourceDir "D:\Games\TH06" -SkipBuild
```

## 조작법

| PSP 버튼 | 동작 |
| --- | --- |
| 방향키 / 아날로그 스틱 | 이동 / 메뉴 이동 |
| × | 사격 / 결정 |
| ○ | 봄 / 뒤로 |
| □ | 저속 이동 |
| Start | 일시 정지 |
| R 트리거 | 대화 넘기기 |

## PSP 전용 동작

- CPU/버스 클록을 333/166 MHz로 설정합니다.
- 불투명 텍스처는 RGB565, 투명도가 필요한 스프라이트는 RGBA4444를 사용하며 GPU
  업로드 뒤 임시 디코딩 데이터를 해제합니다.
- 사용하지 않는 코드와 unwind 메타데이터를 제거합니다. 동기식 스테이지 로딩 중에는
  글꼴 캐시를 잠시 해제해 PSP-1000이 `Now Loading`에서 종료되는 메모리 급증을 피합니다.
- 데스크톱용 2.25 MiB 정점 버퍼 대신 크기가 제한된 자동 flush 배치를 사용합니다.
  실기 PSPGL의 안정적인 고정 색상 경로를 유지하면서 RGBA4444 수준으로 색상값을
  양자화해 페이드 중 발생하는 불필요한 배치 분할을 줄입니다.
- 삼각함수는 PSP VFPU를 사용하고, 프레임 핵심 파일은 속도 위주로 별도 최적화합니다.
- BGM은 PCM WAV를 스트리밍하고 믹서 버퍼를 재사용합니다. PSP에서는 실기 SDL/스레드
  교착을 피하기 위해 프레임 루프에서 오디오를 큐에 넣습니다.
- PSP에서 의미 없는 MIDI, 창 모드, 색 농도 옵션은 숨깁니다. 음악은 WAV 또는 끄기를
  선택할 수 있습니다.
- 설정, 점수, 세이브, 리플레이는 EBOOT 옆에 기록되므로 디렉터리는 쓰기 가능해야 합니다.
- `loadtrace.txt`는 실행할 때마다 EBOOT 옆에 새로 만들며 마지막으로 완료된 스테이지
  로딩 단계를 기록합니다.

## 개발 빌드

WSL 안에서 PSP 타깃을 직접 빌드할 수도 있습니다.

```bash
export PSPDEV=/usr/local/pspdev
export PATH="$PSPDEV/bin:$PATH"
psp-cmake -S . -B build/psp -DCMAKE_BUILD_TYPE=Release
cmake --build build/psp -j2
```

`TH06_AUTOTEST_FRAMES`, `TH06_AUTOTEST_INPUT`, `TH06_FORCE_SIDECAR_ASSETS`는
테스트 전용 CMake 옵션이므로 릴리스 빌드에서는 꺼 두세요.

## 저장소 정책

원본 TH06 에셋, 실행 파일, 추출 데이터, 음악 또는 불법 다운로드 링크가 포함된 이슈나
Pull Request를 올리지 마세요. 빌드 결과물과 사용자가 제공한 게임 데이터는
`.gitignore`에서 제외합니다.

## 제작진 및 라이선스

- 원작: ZUN / 상하이 앨리스 환악단
- 포터블 재구현: [GensokyoClub/th06](https://github.com/GensokyoClub/th06) 기여자
- PSP 포트: 이 저장소의 기여자

소스 코드는 업스트림 프로젝트에 따라 [GNU GPL v3](LICENSE)로 배포합니다. 동방
Project, 동방홍마향과 원본 게임 에셋의 권리는 각 권리자에게 있습니다. 이 프로젝트는
비공식이며 상하이 앨리스 환악단과 제휴하거나 승인을 받지 않았습니다.
