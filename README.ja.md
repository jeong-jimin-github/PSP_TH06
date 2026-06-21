# 東方紅魔郷 ～ the Embodiment of Scarlet Devil — PSPポート

[한국어](README.md) | **日本語** | [English](README.en.md)

[GensokyoClub/th06](https://github.com/GensokyoClub/th06) の `portable`
ブランチを基にした、**東方紅魔郷 ～ the Embodiment of Scarlet Devil**（TH06）の
実験的なPSP Homebrew移植です。

このリポジトリに含まれるのはソースコードのみです。オリジナルの実行ファイル、
DATアーカイブ、音楽など、著作権で保護されたゲーム素材は**含まれていません**。
正規に入手した日本語PC版のデータが必要です。

## 現在の状態

- 標準のPSP Homebrew `EBOOT.PBP`として起動します。
- タイトル画面、メニュー、ゲームプレイ、敵、自機ショット、効果音、WAV音楽を実装しています。
- 元の640×480画面を正確に1/2の320×240で描画し、PSPの480×272画面中央に
  黒い余白を配置します。
- 32 MiBのPSP-1000向けにテクスチャ、ストリーミング、メモリ使用量を調整しています。
- PSP-1000の333/166 MHzクロックで60 Hzを目標に、スプライトバッチ、VFPU演算、
  オーディオミキシングを最適化しています。
- 画面のティアリングを防ぐため、VBlankに同期して表示します。
- PPSSPPのPSP-1000メモリモデルでテストしています。リリースごとの実機確認を推奨します。

まだ実験的な移植です。セーブとリプレイはバックアップしてください。

## 必要なもの

- PowerShellとWSL2（標準はUbuntu）が使えるWindows 10または11
- `psp-cmake`を含む[PSPDEV](https://github.com/pspdev/pspdev)
- PSP用SDL2、SDL2_image、SDL2_ttf、PSPGL
- 以下を含む正規のTH06インストール
  - `CM.DAT`、`ED.DAT`、`IN.DAT`、`MD.DAT`、`ST.DAT`、`TL.DAT`
  - `bgm/th06_01.wav`から`bgm/th06_17.wav`

パッケージスクリプトは元のWindows実行ファイルをコピーも実行もしません。存在する
場合は、想定外のゲームバージョンを警告するためのハッシュ確認にのみ使用します。

## ビルドとパッケージ作成

リポジトリをクローンし、PowerShellで次を実行します。

```powershell
git clone https://github.com/jeong-jimin-github/PSP_TH06.git
cd PSP_TH06
./scripts/build_psp.ps1 `
  -SourceDir "D:\Games\TH06" `
  -Pspdev "/home/your-wsl-user/pspdev"
```

PSPDEVが`/usr/local/pspdev`にある場合、`-Pspdev`は省略できます。WSLの
ディストリビューション名が`Ubuntu`以外なら`-WslDistro`を指定してください。

完成したメモリースティック用ディレクトリは次に作成されます。

```text
build/psp-package/PSP/GAME/TH06/
```

`TH06`ディレクトリ全体を`ms0:/PSP/GAME/TH06/`へコピーするか、PPSSPPで
`EBOOT.PBP`を起動してください。生成パッケージは約340 MiBです。元のDAT、WAV音源、
PSPで負荷の高いDAT展開を避けるための抽出済み素材が含まれます。

初回ビルド後は、コンパイルを省略して再パッケージできます。

```powershell
./scripts/build_psp.ps1 -SourceDir "D:\Games\TH06" -SkipBuild
```

## 操作

| PSP操作 | 動作 |
| --- | --- |
| 方向キー / アナログパッド | 移動 / メニュー操作 |
| × | ショット / 決定 |
| ○ | ボム / 戻る |
| □ | 低速移動 |
| Start | ポーズ |
| Rトリガー | 会話スキップ |

## PSP固有の動作

- CPU/バスクロックを333/166 MHzに設定します。
- 不透明テクスチャはRGB565、透明度が必要なスプライトはRGBA4444を使用し、GPUへの
  転送後に一時デコードデータを解放します。
- 未使用コードとunwindメタデータを削除します。同期ステージ読み込み中はフォント
  キャッシュを一時解放し、PSP-1000が`Now Loading`で終了するメモリ急増を防ぎます。
- デスクトップ用2.25 MiB頂点バッファの代わりに、上限付き自動flushバッチを使用します。
  実機PSPGLで安定している固定色経路を維持しながら、変調色をRGBA4444精度へ量子化し、
  フェード中の不要なバッチ分割を減らします。
- 三角関数はPSP VFPUを使用し、フレーム処理の主要ファイルを速度優先で最適化します。
- BGMはPCM WAVをストリーミングし、ミキサーバッファを再利用します。実機のSDL/
  スレッド停止を避けるため、PSPではフレームループからオーディオをキューします。
- PSPで不要なMIDI、ウィンドウモード、色深度の設定は非表示です。音楽はWAVまたは
  Offを選べます。
- 設定、スコア、セーブ、リプレイはEBOOTと同じ場所に書き込まれます。ディレクトリを
  書き込み可能にしてください。
- `loadtrace.txt`は起動ごとにEBOOTの隣へ作り直され、最後に完了したステージ読み込み
  手順を記録します。

## 開発用ビルド

WSL内でPSPターゲットを直接ビルドすることもできます。

```bash
export PSPDEV=/usr/local/pspdev
export PATH="$PSPDEV/bin:$PATH"
psp-cmake -S . -B build/psp -DCMAKE_BUILD_TYPE=Release
cmake --build build/psp -j2
```

`TH06_AUTOTEST_FRAMES`、`TH06_AUTOTEST_INPUT`、`TH06_FORCE_SIDECAR_ASSETS`は
テスト専用CMakeオプションです。リリースビルドでは無効にしてください。

## リポジトリ方針

元のTH06素材、実行ファイル、抽出データ、音楽、または不正なダウンロード先を含む
IssueやPull Requestを投稿しないでください。ビルド結果とローカルのゲームデータは
`.gitignore`で除外されます。

## クレジットとライセンス

- 原作：ZUN / 上海アリス幻樂団
- ポータブル再実装：[GensokyoClub/th06](https://github.com/GensokyoClub/th06)の貢献者
- PSPポート：このリポジトリの貢献者

ソースコードは上流プロジェクトに従い、[GNU GPL v3](LICENSE)で配布します。東方
Project、東方紅魔郷、および元のゲーム素材の権利は各権利者に帰属します。本プロジェクトは
非公式であり、上海アリス幻樂団との提携または承認を受けたものではありません。
